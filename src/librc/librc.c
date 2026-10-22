/*
 * librc
 * core RC functions
 */

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#include <assert.h>
#include <helpers.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#include "queue.h"
#include "librc.h"
#include "misc.h"
#include "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = NULL;
static số_nguyên dirfds[RC_DIR_MAX];

số_nguyên rc_dirfd(enum rc_dir dir) {
	số_nguyên flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	số_nguyên dirfd;

	nếu (dir < 0 || dir >= RC_DIR_MAX)
		trả_về -1;
	nếu (dirfds[dir] <= 0) {
		switch (dir) {
		case RC_DIR_SYSCONF:
			trả_về dirfds[dir] = open(rc_sysconfdir(), flags);
		case RC_DIR_USRCONF:
			trả_về dirfds[dir] = open(rc_usrconfdir(), flags);
		case RC_DIR_RUNLEVEL:
			trả_về dirfds[dir] = open(rc_runleveldir(), flags);
		case RC_DIR_SVCDIR:
			nếu (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				trả_về -1;
			trả_về dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			nếu ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				nếu (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					trả_về -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			break;
		}
	}
	trả_về dirfds[dir];
}

void clear_dirfds(void) {
	cho (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		nếu (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(số_nguyên targetfd, const char *dir, số_nguyên options)
{
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *list = NULL;
	struct stat buf;
	size_t l;

	list = rc_stringlist_new();

	nếu (!(dp = do_opendirat(targetfd, dir)))
		trả_về list;

	trong_khi (((d = readdir(dp)) != NULL)) {
		nếu (d->d_name[0] == '.')
			continue;
		nếu (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			nếu (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			nếu (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		nếu (options & LS_DIR) {
			nếu (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	trả_về list;
}

static bool
rm_dir(số_nguyên targetfd, const char *pathname, bool top)
{
	bool retval = true;
	struct dirent *d;
	struct stat s;
	DIR *dp;

	nếu (!(dp = do_opendirat(targetfd, pathname)))
		trả_về false;

	trong_khi ((d = readdir(dp))) {
		nếu (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		nếu (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			break;
		}
		nếu (S_ISDIR(s.st_mode)) {
			nếu (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				break;
			}
		} không_thì {
			nếu (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				break;
			}
		}
	}
	closedir(dp);

	nếu (!retval)
		trả_về false;

	nếu (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		trả_về false;

	trả_về true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
static bool
file_regex(const char *file, const char *regex)
{
	FILE *fp;
	char *line = NULL;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	số_nguyên result;

	nếu (!(fp = fopen(file, "r")))
		trả_về false;

	nếu ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		char buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, sizeof(buf));
		fprintf(stderr, "file_regex: %s", buf);
		trả_về false;
	}

	trong_khi ((len = xgetline(&line, &size, fp)) != -1) {
		char *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			nếu (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			trong_khi (str < line + len && *str == '\0')
				str++;
		} trong_khi (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	trả_về retval;
}
#endif


static const char *
get_systype(void)
{
	char *systype = rc_conf_value("rc_sys");
	nếu (systype) {
		char *s = systype;
		/* Convert to uppercase */
		trong_khi (s && *s) {
			nếu (islower((unsigned char) *s))
				*s = toupper((unsigned char) *s);
			s++;
		}
	}
	trả_về systype;
}

static const char *
detect_container(const char *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	nếu (systype) {
		nếu (strcmp(systype, RC_SYS_NONE) == 0)
		       trả_về NULL;
		nếu (strcmp(systype, RC_SYS_JAIL) == 0)
			trả_về RC_SYS_JAIL;
		nếu (strcmp(systype, RC_SYS_PODMAN) == 0)
			trả_về RC_SYS_PODMAN;
	}

	số_nguyên jailed = 0;
	size_t len = sizeof(jailed);

	nếu (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		nếu (jailed == 1)
			trả_về RC_SYS_JAIL;

	nếu (access("/var/run/.containerenv", F_OK) == 0)
		trả_về RC_SYS_PODMAN;
#endif

#ifdef __linux__
	nếu (systype) {
		nếu (strcmp(systype, RC_SYS_NONE) == 0)
			trả_về NULL;
		nếu (strcmp(systype, RC_SYS_UML) == 0)
			trả_về RC_SYS_UML;
		nếu (strcmp(systype, RC_SYS_VSERVER) == 0)
			trả_về RC_SYS_VSERVER;
		nếu (strcmp(systype, RC_SYS_OPENVZ) == 0)
			trả_về RC_SYS_OPENVZ;
		nếu (strcmp(systype, RC_SYS_LXC) == 0)
			trả_về RC_SYS_LXC;
		nếu (strcmp(systype, RC_SYS_RKT) == 0)
				trả_về RC_SYS_RKT;
		nếu (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				trả_về RC_SYS_SYSTEMD_NSPAWN;
		nếu (strcmp(systype, RC_SYS_DOCKER) == 0)
				trả_về RC_SYS_DOCKER;
		nếu (strcmp(systype, RC_SYS_PODMAN) == 0)
			trả_về RC_SYS_PODMAN;
		nếu (strcmp(systype, RC_SYS_WSL) == 0)
			trả_về RC_SYS_WSL;
	}
	nếu (file_regex("/proc/cpuinfo", "UML"))
		trả_về RC_SYS_UML;
	không_thì nếu (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		trả_về RC_SYS_VSERVER;
	không_thì nếu (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		trả_về RC_SYS_OPENVZ;
	không_thì nếu (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		trả_về RC_SYS_OPENVZ; /* old test */
	không_thì nếu (file_regex("/proc/1/environ", "container=lxc"))
		trả_về RC_SYS_LXC;
	không_thì nếu (file_regex("/proc/1/environ", "container=rkt"))
		trả_về RC_SYS_RKT;
	không_thì nếu (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		trả_về RC_SYS_SYSTEMD_NSPAWN;
	không_thì nếu (access("/run/.containerenv", F_OK) == 0)
		trả_về RC_SYS_PODMAN;
	không_thì nếu (access("/.dockerenv", F_OK) == 0)
		trả_về RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	không_thì nếu (file_regex("/proc/1/environ", "container=docker"))
		trả_về RC_SYS_DOCKER;
	không_thì nếu (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		trả_về RC_SYS_WSL;
	không_thì nếu (getenv("WSL_DISTRO_NAME"))
		trả_về RC_SYS_WSL;
	không_thì nếu (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		trả_về RC_SYS_WSL;
#endif

	trả_về NULL;
}

static const char *
detect_vm(const char *systype RC_UNUSED)
{
#ifdef __NetBSD__
	nếu (systype) {
		nếu (strcmp(systype, RC_SYS_NONE) == 0)
			trả_về NULL;
		nếu (strcmp(systype, RC_SYS_XEN0) == 0)
			trả_về RC_SYS_XEN0;
		nếu (strcmp(systype, RC_SYS_XENU) == 0)
			trả_về RC_SYS_XENU;
	}
	nếu (access("/kern/xen/privcmd", F_OK) == 0)
		trả_về RC_SYS_XEN0;
	nếu (access("/kern/xen", F_OK) == 0)
		trả_về RC_SYS_XENU;
#endif

#ifdef __linux__
	nếu (systype) {
		nếu (strcmp(systype, RC_SYS_NONE) == 0)
			trả_về NULL;
		nếu (strcmp(systype, RC_SYS_XEN0) == 0)
			trả_về RC_SYS_XEN0;
		nếu (strcmp(systype, RC_SYS_XENU) == 0)
			trả_về RC_SYS_XENU;
	}
	nếu (access("/proc/xen", F_OK) == 0) {
		nếu (file_regex("/proc/xen/capabilities", "control_d"))
			trả_về RC_SYS_XEN0;
		trả_về RC_SYS_XENU;
	}
#endif

	trả_về NULL;
}

const char *
rc_sys(void)
{
	const char *systype;
	const char *sys;

	systype = get_systype();
	sys = detect_container(systype);
	nếu (!sys) {
		sys = detect_vm(systype);
	}

	trả_về sys;
}

static const char *
rc_parse_service_state(RC_SERVICE state)
{
	số_nguyên i;

	cho (i = 0; rc_service_state_names[i].name; i++) {
		nếu (rc_service_state_names[i].state == state)
			trả_về rc_service_state_names[i].name;
	}
	trả_về NULL;
}

static số_nguyên
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	cho (size_t i = 0; rc_service_state_names[i].name; i++)
		nếu (rc_service_state_names[i].state == state)
			trả_về rc_service_state_names[i].dir;
	trả_về -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
static void
get_runlevel_chain(const char *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	const char *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	nếu (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		trả_về;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	nếu (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->value;

		/* Check for loop */
		nếu (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->value);
			exit(1);
		}

		/* Add new ancestor */
		rc_stringlist_add(ancestor_list, nextlevel);

		get_runlevel_chain(nextlevel, level_list, ancestor_list);

		rc_stringlist_delete(ancestor_list, nextlevel);
	}
	rc_stringlist_free(dirs);
}

bool
rc_runlevel_starting(void)
{
	trả_về faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	trả_về faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	trả_về ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

char *
rc_runlevel_get(void)
{
	FILE *fp;
	char *runlevel = NULL;
	size_t i;

	nếu ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(sizeof(char) * PATH_MAX);
		nếu (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			nếu (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} không_thì
			*runlevel = '\0';
		fclose(fp);
	}

	nếu (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	trả_về runlevel;
}

bool
rc_runlevel_set(const char *runlevel)
{
	FILE *fp;

	nếu (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		trả_về false;

	fputs(runlevel, fp);
	fclose(fp);
	trả_về true;
}

bool
rc_runlevel_exists(const char *runlevel)
{
	struct stat buf;

	nếu (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		trả_về false;
	nếu (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		trả_về true;
	trả_về false;
}

bool
rc_runlevel_stack(const char *dst, const char *src)
{
	char *d, *s;
	số_nguyên r;

	nếu (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		trả_về false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	trả_về (r == 0);
}

bool
rc_runlevel_unstack(const char *dst, const char *src)
{
	char *path;
	số_nguyên r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	trả_về (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(const char *runlevel)
{
	RC_STRINGLIST *stack, *ancestor_list;
	stack = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, stack, ancestor_list);
	rc_stringlist_free(ancestor_list);
	trả_về stack;
}

static const char *config_path[] = { _RC_PATH, NULL };

static struct {
	bool set;
	char *svcdir, *confdir, *runleveldir;
} rc_dirs;

static struct {
	char *buffer;
	size_t count;
	số_nguyên *fds;
	const char **entries, **allocd;
} rc_path;

static void
free_rc_dirs(void)
{
#define X(ptr) ptr = (free(ptr), NULL)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = NULL;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.count = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
static void
append_path(FILE *fp, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.count++;
}

static const char *
strnsep(const char **str, size_t *size, const char sep)
{
	const char *ret = *str;

	nếu (!ret)
		trả_về NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	nếu (!**str)
		*str = NULL;
	không_thì
		*str += 1;

	trả_về ret;
}

static const char *
strnullsep(const char **str, size_t *size)
{
	const char *ret = *str;
	size_t len;

	nếu (!ret)
		trả_về NULL;

	len = strnlen(ret, *size - 1) + 1;

	nếu ((*size -= len))
		*str += len;
	không_thì
		*str = NULL;

	trả_về ret;
}

static void
append_user_path(FILE *fp)
{
	const char *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	nếu (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	không_thì trong_khi ((entry = strnsep(&xdg, &size, ':')))
		nếu (size) append_path(fp, "%.*s/rc", (số_nguyên) size, entry);

	cho (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const char *path, bool user)
{
	bool has_user = false;
	const char *entry;
	size_t size;

	trong_khi ((entry = strnsep(&path, &size, ':'))) {
		nếu (size)
			append_path(fp, "%.*s%s", (số_nguyên)size, entry, user ? "/user" : "");
		không_thì nếu (user && !has_user)
			append_user_path(fp), has_user = true;
		không_thì nếu (!user) trong_khi (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const char *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	nếu (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	nếu (!(path = getenv("RC_PATH")) && !user)
		trả_về;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	nếu (path)
		append_rc_path(fp, path, user);
	không_thì
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, sizeof(*rc_path.entries));

	path = rc_path.buffer;

	trong_khi ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	số_nguyên _is_user = -1;

	nếu (_is_user == -1) {
		char *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	trả_về _is_user;
}

void
rc_set_user(void)
{
	char *env;

	nếu (is_user)
		trả_về;

	nếu (!rc_dirs.set)
		atexit(free_rc_dirs);
	không_thì
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	nếu ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} không_thì nếu ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} không_thì {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	nếu (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

const char * const *
rc_scriptdirs(void)
{
	nếu (!rc_path.entries)
		load_dynamic_path(false);
	trả_về rc_path.entries;
}

size_t
rc_scriptdirfds(const số_nguyên **fds)
{
	static size_t len = 0;

	nếu (len == 0) {
		nếu (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * sizeof(*rc_path.fds));
		cho (size_t i = 0; i < rc_path.count; i++) {
			số_nguyên fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			nếu (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	trả_về len;
}

const char *
rc_sysconfdir(void)
{
	trả_về RC_SYSCONFDIR;
}

const char *
rc_usrconfdir(void)
{
	trả_về rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const char *
rc_runleveldir(void)
{
	trả_về rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const char *
rc_svcdir(void)
{
	static const char *svcdir;

	nếu (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	trả_về rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(số_nguyên targetfd, const char *path, char **buf, size_t bufsiz)
{
	char *buffer;
	ssize_t r;

	cho (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		nếu (r < 0) {
			free(buffer);
			trả_về -errno;
		}
		nếu ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			trả_về r;
		}
		free(buffer);
		nếu (bufsiz >= SSIZE_MAX - PATH_MAX)
			trả_về -ENOMEM;
	}
}

/* Resolve a service name to its full path */
char *
rc_service_resolve(const char *service)
{
	char *buffer;
	struct stat buf;

	nếu (!service)
		trả_về NULL;

	nếu (service[0] == '/')
		trả_về xstrdup(service);

	/* First check started services */
	nếu (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		nếu (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			trả_về buffer;
	nếu (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		nếu (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			trả_về buffer;

	cho (const char * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		char *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		nếu (stat(file, &buf) == 0)
			trả_về file;
		free(file);
	}

	trả_về NULL;
}

bool
rc_service_exists(const char *service)
{
	char *file;
	bool retval = false;
	size_t len;
	struct stat buf;

	nếu (!service) {
		errno = EINVAL;
		trả_về false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	nếu (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		trả_về false;
	}

	nếu (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		trả_về false;
	}

	nếu (stat(file, &buf) == 0) {
		nếu (buf.st_mode & S_IXUGO)
			retval = true;
		không_thì
			errno = ENOEXEC;
	}
	free(file);
	trả_về retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(const char *service)
{
	char *svc;
	char *cmd = NULL;
	char *buffer = NULL;
	size_t len = 0;
	RC_STRINGLIST *commands = NULL;
	char *token;
	char *p;
	FILE *fp;

	nếu (!(svc = rc_service_resolve(service)))
		trả_về NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	nếu (!(fp = popen(cmd, "r"))) {
		free(cmd);
		trả_về NULL;
	}

	nếu (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		trong_khi ((token = strsep(&p, " ")))
			nếu (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	trả_về commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
char *
rc_service_description(const char *service, const char *option)
{
	char *svc;
	char *cmd;
	char *desc = NULL;
	size_t size = 0;
	FILE *fp;

	nếu (!(svc = rc_service_resolve(service)))
		trả_về NULL;

	nếu (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	nếu ((fp = popen(cmd, "r"))) {
		nếu (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	trả_về desc;
}

bool
rc_service_in_runlevel(const char *service, const char *runlevel)
{
	char *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	trả_về r;
}

bool
rc_service_mark(const char *service, const RC_SERVICE state)
{
	số_nguyên i = 0;
	const char *base;
	char *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	số_nguyên serrno;

	nếu (!init)
		trả_về false;

	base = basename_c(service);
	nếu (state != RC_SERVICE_STOPPED) {
		số_nguyên state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		nếu (access(init, F_OK) != 0) {
			free(init);
			trả_về false;
		}

		unlinkat(state_dirfd, base, 0);
		nếu ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			trả_về false;
		}
	}

	nếu (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		trả_về true;
	}

	/* Remove any old states now */
	cho (i = 0; rc_service_state_names[i].name; i++) {
		số_nguyên state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		nếu (s == state)
			continue;

		switch (s) {
		case RC_SERVICE_STOPPED:
		case RC_SERVICE_HOTPLUGGED:
		case RC_SERVICE_SCHEDULED:
			continue;
		case RC_SERVICE_WASINACTIVE:
			nếu (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			nếu (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			nếu ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				nếu (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					trả_về false;
				}
				skip_wasinactive = true;
			}
			nếu (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				trả_về false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	nếu (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	nếu (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	nếu (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		nếu (dp) {
			struct dirent *d;
			trong_khi ((d = readdir(dp))) {
				char *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				free(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	free(init);
	trả_về true;
}

RC_SERVICE
rc_service_state(const char *service)
{
	const char *base = basename_c(service);
	số_nguyên state = RC_SERVICE_STOPPED;
	số_nguyên i;

	cho (i = 0; rc_service_state_names[i].name; i++) {
		nếu (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			nếu (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			không_thì
				state |= rc_service_state_names[i].state;
		}
	}

	nếu (state & RC_SERVICE_STARTED)
		nếu (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	nếu (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		nếu (dp) {
			struct dirent *d;
			trong_khi ((d = readdir(dp))) {
				nếu (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					break;
				}
			}

			closedir(dp);
		}
	}

	trả_về state;
}

char *
rc_service_value_get(const char *service, const char *option)
{
	char *buffer = NULL;
	size_t len = 0;
	char *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	free(file);

	trả_về buffer;
}

bool
rc_service_value_set(const char *service, const char *option, const char *value)
{
	bool ret = true;
	số_nguyên optdirfd;
	FILE *fp;

	nếu (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		trả_về false;

	nếu ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		trả_về false;

	nếu (!value) {
		unlinkat(optdirfd, option, 0);
	} không_thì nếu ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} không_thì {
		ret = false;
	}

	close(optdirfd);
	trả_về ret;
}

bool
rc_service_schedule_start(const char *service, const char *service_to_start)
{
	const char *base_to_start = basename_c(service_to_start);
	const char *base = basename_c(service);
	bool retval;
	số_nguyên schedfd;
	char *init;

	/* service may be a provided service, like net */
	nếu (!service || !rc_service_exists(service_to_start))
		trả_về false;

	nếu (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		trả_về false;

	nếu ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		trả_về false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	trả_về retval;
}

bool
rc_service_schedule_clear(const char *service)
{
	trả_về !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const char *runlevel)
{
	RC_STRINGLIST *list = NULL;

	nếu (!runlevel) {
		const số_nguyên *scriptdirfds;
		list = rc_stringlist_new();
		cho (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		trả_về list;
	}

	/* These special levels never contain any services */
	nếu (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		trả_về rc_stringlist_new();

	trả_về ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(const char *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *stack;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stack, stacks, entries) {
		sl = rc_services_in_runlevel(stack->value);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	trả_về list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	struct dirent *d;
	DIR *dp;

	nếu (state != RC_SERVICE_SCHEDULED)
		trả_về ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	nếu (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		trả_về list;

	trong_khi ((d = readdir(dp))) {
		nếu (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	trả_về list;
}

bool
rc_service_add(const char *runlevel, const char *service)
{
	bool retval;
	char *init;
	char *file;
	char *path;
	char *binit = NULL;
	char *i;

	nếu (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		trả_về false;
	}

	nếu (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		trả_về false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	nếu (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		nếu (path == NULL) {
			free(init);
			trả_về false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		nếu (!retval) {
			free(init);
			errno = EPERM;
			trả_về false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	nếu (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		goto out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	free(file);

out:
	free(binit);
	free(init);
	trả_về retval;
}

bool
rc_service_delete(const char *runlevel, const char *service)
{
	char *file;
	số_nguyên r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	trả_về (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const char *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	struct dirent *d;
	DIR *dp;

	nếu (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		trả_về list;

	trong_khi ((d = readdir(dp))) {
		char *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		nếu (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	trả_về list;
}

RC_STRINGLIST *
rc_services_scheduled(const char *service)
{
	trả_về ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const char *service, const char *env)
{
	size_t name_len = strcspn(env, "=");
	char var[strlen(service) + sizeof("/") + name_len];
	số_nguyên dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	nếu (env[name_len] != '=')
		trả_về errno = EINVAL, false;

	snprintf(var, sizeof(var), "%s/%.*s", service, (số_nguyên) name_len, env);

	nếu (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		trả_về false;

	nếu (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		trả_về false;

	fputs(&env[name_len + 1], fp);

	trả_về fclose(fp) == 0;
}

bool
rc_service_setenv(const char *service, const char *name, const char *value)
{
	char var[strlen(service) + sizeof("/") + strlen(name)];
	số_nguyên dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	nếu (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		trả_về false;

	nếu (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		trả_về false;

	fputs(value, fp);

	trả_về fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const char *service, const char *name, char **buf)
{
	char var[strlen(service) + sizeof("/") + strlen(name)], *value;
	số_nguyên dfd = rc_dirfd(RC_DIR_ENVIRON);
	struct stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	nếu (fstatat(dfd, var, &stat, 0) == -1)
		trả_về -1;

	nếu (!buf)
		trả_về stat.st_size;

	nếu (!(value = xrealloc(*buf, stat.st_size + 1)))
		trả_về -1;

	nếu (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		trả_về -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	trả_về count;
}
