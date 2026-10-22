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
static ціле dirfds[RC_DIR_MAX];

ціле rc_dirfd(enum rc_dir dir) {
	ціле flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ціле dirfd;

	якщо (dir < 0 || dir >= RC_DIR_MAX)
		повернути -1;
	якщо (dirfds[dir] <= 0) {
		switch (dir) {
		case RC_DIR_SYSCONF:
			повернути dirfds[dir] = open(rc_sysconfdir(), flags);
		case RC_DIR_USRCONF:
			повернути dirfds[dir] = open(rc_usrconfdir(), flags);
		case RC_DIR_RUNLEVEL:
			повернути dirfds[dir] = open(rc_runleveldir(), flags);
		case RC_DIR_SVCDIR:
			якщо (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				повернути -1;
			повернути dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			якщо ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				якщо (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					повернути -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			break;
		}
	}
	повернути dirfds[dir];
}

void clear_dirfds(void) {
	для (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		якщо (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(ціле targetfd, const char *dir, ціле options)
{
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *list = NULL;
	struct stat buf;
	size_t l;

	list = rc_stringlist_new();

	якщо (!(dp = do_opendirat(targetfd, dir)))
		повернути list;

	поки (((d = readdir(dp)) != NULL)) {
		якщо (d->d_name[0] == '.')
			continue;
		якщо (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			якщо (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			якщо (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		якщо (options & LS_DIR) {
			якщо (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	повернути list;
}

static bool
rm_dir(ціле targetfd, const char *pathname, bool top)
{
	bool retval = true;
	struct dirent *d;
	struct stat s;
	DIR *dp;

	якщо (!(dp = do_opendirat(targetfd, pathname)))
		повернути false;

	поки ((d = readdir(dp))) {
		якщо (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		якщо (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			break;
		}
		якщо (S_ISDIR(s.st_mode)) {
			якщо (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				break;
			}
		} інакше {
			якщо (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				break;
			}
		}
	}
	closedir(dp);

	якщо (!retval)
		повернути false;

	якщо (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		повернути false;

	повернути true;
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
	ціле result;

	якщо (!(fp = fopen(file, "r")))
		повернути false;

	якщо ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		char buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, sizeof(buf));
		fprintf(stderr, "file_regex: %s", buf);
		повернути false;
	}

	поки ((len = xgetline(&line, &size, fp)) != -1) {
		char *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			якщо (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			поки (str < line + len && *str == '\0')
				str++;
		} поки (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	повернути retval;
}
#endif


static const char *
get_systype(void)
{
	char *systype = rc_conf_value("rc_sys");
	якщо (systype) {
		char *s = systype;
		/* Convert to uppercase */
		поки (s && *s) {
			якщо (islower((unsigned char) *s))
				*s = toupper((unsigned char) *s);
			s++;
		}
	}
	повернути systype;
}

static const char *
detect_container(const char *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	якщо (systype) {
		якщо (strcmp(systype, RC_SYS_NONE) == 0)
		       повернути NULL;
		якщо (strcmp(systype, RC_SYS_JAIL) == 0)
			повернути RC_SYS_JAIL;
		якщо (strcmp(systype, RC_SYS_PODMAN) == 0)
			повернути RC_SYS_PODMAN;
	}

	ціле jailed = 0;
	size_t len = sizeof(jailed);

	якщо (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		якщо (jailed == 1)
			повернути RC_SYS_JAIL;

	якщо (access("/var/run/.containerenv", F_OK) == 0)
		повернути RC_SYS_PODMAN;
#endif

#ifdef __linux__
	якщо (systype) {
		якщо (strcmp(systype, RC_SYS_NONE) == 0)
			повернути NULL;
		якщо (strcmp(systype, RC_SYS_UML) == 0)
			повернути RC_SYS_UML;
		якщо (strcmp(systype, RC_SYS_VSERVER) == 0)
			повернути RC_SYS_VSERVER;
		якщо (strcmp(systype, RC_SYS_OPENVZ) == 0)
			повернути RC_SYS_OPENVZ;
		якщо (strcmp(systype, RC_SYS_LXC) == 0)
			повернути RC_SYS_LXC;
		якщо (strcmp(systype, RC_SYS_RKT) == 0)
				повернути RC_SYS_RKT;
		якщо (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				повернути RC_SYS_SYSTEMD_NSPAWN;
		якщо (strcmp(systype, RC_SYS_DOCKER) == 0)
				повернути RC_SYS_DOCKER;
		якщо (strcmp(systype, RC_SYS_PODMAN) == 0)
			повернути RC_SYS_PODMAN;
		якщо (strcmp(systype, RC_SYS_WSL) == 0)
			повернути RC_SYS_WSL;
	}
	якщо (file_regex("/proc/cpuinfo", "UML"))
		повернути RC_SYS_UML;
	інакше якщо (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		повернути RC_SYS_VSERVER;
	інакше якщо (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		повернути RC_SYS_OPENVZ;
	інакше якщо (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		повернути RC_SYS_OPENVZ; /* old test */
	інакше якщо (file_regex("/proc/1/environ", "container=lxc"))
		повернути RC_SYS_LXC;
	інакше якщо (file_regex("/proc/1/environ", "container=rkt"))
		повернути RC_SYS_RKT;
	інакше якщо (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		повернути RC_SYS_SYSTEMD_NSPAWN;
	інакше якщо (access("/run/.containerenv", F_OK) == 0)
		повернути RC_SYS_PODMAN;
	інакше якщо (access("/.dockerenv", F_OK) == 0)
		повернути RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	інакше якщо (file_regex("/proc/1/environ", "container=docker"))
		повернути RC_SYS_DOCKER;
	інакше якщо (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		повернути RC_SYS_WSL;
	інакше якщо (getenv("WSL_DISTRO_NAME"))
		повернути RC_SYS_WSL;
	інакше якщо (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		повернути RC_SYS_WSL;
#endif

	повернути NULL;
}

static const char *
detect_vm(const char *systype RC_UNUSED)
{
#ifdef __NetBSD__
	якщо (systype) {
		якщо (strcmp(systype, RC_SYS_NONE) == 0)
			повернути NULL;
		якщо (strcmp(systype, RC_SYS_XEN0) == 0)
			повернути RC_SYS_XEN0;
		якщо (strcmp(systype, RC_SYS_XENU) == 0)
			повернути RC_SYS_XENU;
	}
	якщо (access("/kern/xen/privcmd", F_OK) == 0)
		повернути RC_SYS_XEN0;
	якщо (access("/kern/xen", F_OK) == 0)
		повернути RC_SYS_XENU;
#endif

#ifdef __linux__
	якщо (systype) {
		якщо (strcmp(systype, RC_SYS_NONE) == 0)
			повернути NULL;
		якщо (strcmp(systype, RC_SYS_XEN0) == 0)
			повернути RC_SYS_XEN0;
		якщо (strcmp(systype, RC_SYS_XENU) == 0)
			повернути RC_SYS_XENU;
	}
	якщо (access("/proc/xen", F_OK) == 0) {
		якщо (file_regex("/proc/xen/capabilities", "control_d"))
			повернути RC_SYS_XEN0;
		повернути RC_SYS_XENU;
	}
#endif

	повернути NULL;
}

const char *
rc_sys(void)
{
	const char *systype;
	const char *sys;

	systype = get_systype();
	sys = detect_container(systype);
	якщо (!sys) {
		sys = detect_vm(systype);
	}

	повернути sys;
}

static const char *
rc_parse_service_state(RC_SERVICE state)
{
	ціле i;

	для (i = 0; rc_service_state_names[i].name; i++) {
		якщо (rc_service_state_names[i].state == state)
			повернути rc_service_state_names[i].name;
	}
	повернути NULL;
}

static ціле
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	для (size_t i = 0; rc_service_state_names[i].name; i++)
		якщо (rc_service_state_names[i].state == state)
			повернути rc_service_state_names[i].dir;
	повернути -1;
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
	якщо (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		повернути;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	якщо (rc_stringlist_find(level_list, runlevel))
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
		якщо (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	повернути faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	повернути faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	повернути ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

char *
rc_runlevel_get(void)
{
	FILE *fp;
	char *runlevel = NULL;
	size_t i;

	якщо ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(sizeof(char) * PATH_MAX);
		якщо (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			якщо (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} інакше
			*runlevel = '\0';
		fclose(fp);
	}

	якщо (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	повернути runlevel;
}

bool
rc_runlevel_set(const char *runlevel)
{
	FILE *fp;

	якщо (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		повернути false;

	fputs(runlevel, fp);
	fclose(fp);
	повернути true;
}

bool
rc_runlevel_exists(const char *runlevel)
{
	struct stat buf;

	якщо (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		повернути false;
	якщо (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		повернути true;
	повернути false;
}

bool
rc_runlevel_stack(const char *dst, const char *src)
{
	char *d, *s;
	ціле r;

	якщо (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		повернути false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	повернути (r == 0);
}

bool
rc_runlevel_unstack(const char *dst, const char *src)
{
	char *path;
	ціле r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	повернути (r == 0);
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
	повернути stack;
}

static const char *config_path[] = { _RC_PATH, NULL };

static struct {
	bool set;
	char *svcdir, *confdir, *runleveldir;
} rc_dirs;

static struct {
	char *buffer;
	size_t count;
	ціле *fds;
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

	якщо (!ret)
		повернути NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	якщо (!**str)
		*str = NULL;
	інакше
		*str += 1;

	повернути ret;
}

static const char *
strnullsep(const char **str, size_t *size)
{
	const char *ret = *str;
	size_t len;

	якщо (!ret)
		повернути NULL;

	len = strnlen(ret, *size - 1) + 1;

	якщо ((*size -= len))
		*str += len;
	інакше
		*str = NULL;

	повернути ret;
}

static void
append_user_path(FILE *fp)
{
	const char *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	якщо (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	інакше поки ((entry = strnsep(&xdg, &size, ':')))
		якщо (size) append_path(fp, "%.*s/rc", (ціле) size, entry);

	для (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const char *path, bool user)
{
	bool has_user = false;
	const char *entry;
	size_t size;

	поки ((entry = strnsep(&path, &size, ':'))) {
		якщо (size)
			append_path(fp, "%.*s%s", (ціле)size, entry, user ? "/user" : "");
		інакше якщо (user && !has_user)
			append_user_path(fp), has_user = true;
		інакше якщо (!user) поки (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const char *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	якщо (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	якщо (!(path = getenv("RC_PATH")) && !user)
		повернути;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	якщо (path)
		append_rc_path(fp, path, user);
	інакше
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, sizeof(*rc_path.entries));

	path = rc_path.buffer;

	поки ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	ціле _is_user = -1;

	якщо (_is_user == -1) {
		char *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	повернути _is_user;
}

void
rc_set_user(void)
{
	char *env;

	якщо (is_user)
		повернути;

	якщо (!rc_dirs.set)
		atexit(free_rc_dirs);
	інакше
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	якщо ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} інакше якщо ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} інакше {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	якщо (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	якщо (!rc_path.entries)
		load_dynamic_path(false);
	повернути rc_path.entries;
}

size_t
rc_scriptdirfds(const ціле **fds)
{
	static size_t len = 0;

	якщо (len == 0) {
		якщо (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * sizeof(*rc_path.fds));
		для (size_t i = 0; i < rc_path.count; i++) {
			ціле fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			якщо (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	повернути len;
}

const char *
rc_sysconfdir(void)
{
	повернути RC_SYSCONFDIR;
}

const char *
rc_usrconfdir(void)
{
	повернути rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const char *
rc_runleveldir(void)
{
	повернути rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const char *
rc_svcdir(void)
{
	static const char *svcdir;

	якщо (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	повернути rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(ціле targetfd, const char *path, char **buf, size_t bufsiz)
{
	char *buffer;
	ssize_t r;

	для (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		якщо (r < 0) {
			free(buffer);
			повернути -errno;
		}
		якщо ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			повернути r;
		}
		free(buffer);
		якщо (bufsiz >= SSIZE_MAX - PATH_MAX)
			повернути -ENOMEM;
	}
}

/* Resolve a service name to its full path */
char *
rc_service_resolve(const char *service)
{
	char *buffer;
	struct stat buf;

	якщо (!service)
		повернути NULL;

	якщо (service[0] == '/')
		повернути xstrdup(service);

	/* First check started services */
	якщо (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		якщо (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			повернути buffer;
	якщо (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		якщо (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			повернути buffer;

	для (const char * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		char *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		якщо (stat(file, &buf) == 0)
			повернути file;
		free(file);
	}

	повернути NULL;
}

bool
rc_service_exists(const char *service)
{
	char *file;
	bool retval = false;
	size_t len;
	struct stat buf;

	якщо (!service) {
		errno = EINVAL;
		повернути false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	якщо (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		повернути false;
	}

	якщо (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		повернути false;
	}

	якщо (stat(file, &buf) == 0) {
		якщо (buf.st_mode & S_IXUGO)
			retval = true;
		інакше
			errno = ENOEXEC;
	}
	free(file);
	повернути retval;
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

	якщо (!(svc = rc_service_resolve(service)))
		повернути NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	якщо (!(fp = popen(cmd, "r"))) {
		free(cmd);
		повернути NULL;
	}

	якщо (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		поки ((token = strsep(&p, " ")))
			якщо (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	повернути commands;
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

	якщо (!(svc = rc_service_resolve(service)))
		повернути NULL;

	якщо (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	якщо ((fp = popen(cmd, "r"))) {
		якщо (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	повернути desc;
}

bool
rc_service_in_runlevel(const char *service, const char *runlevel)
{
	char *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	повернути r;
}

bool
rc_service_mark(const char *service, const RC_SERVICE state)
{
	ціле i = 0;
	const char *base;
	char *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ціле serrno;

	якщо (!init)
		повернути false;

	base = basename_c(service);
	якщо (state != RC_SERVICE_STOPPED) {
		ціле state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		якщо (access(init, F_OK) != 0) {
			free(init);
			повернути false;
		}

		unlinkat(state_dirfd, base, 0);
		якщо ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			повернути false;
		}
	}

	якщо (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		повернути true;
	}

	/* Remove any old states now */
	для (i = 0; rc_service_state_names[i].name; i++) {
		ціле state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		якщо (s == state)
			continue;

		switch (s) {
		case RC_SERVICE_STOPPED:
		case RC_SERVICE_HOTPLUGGED:
		case RC_SERVICE_SCHEDULED:
			continue;
		case RC_SERVICE_WASINACTIVE:
			якщо (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			якщо (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			якщо ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				якщо (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					повернути false;
				}
				skip_wasinactive = true;
			}
			якщо (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				повернути false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	якщо (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	якщо (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	якщо (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		якщо (dp) {
			struct dirent *d;
			поки ((d = readdir(dp))) {
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
	повернути true;
}

RC_SERVICE
rc_service_state(const char *service)
{
	const char *base = basename_c(service);
	ціле state = RC_SERVICE_STOPPED;
	ціле i;

	для (i = 0; rc_service_state_names[i].name; i++) {
		якщо (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			якщо (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			інакше
				state |= rc_service_state_names[i].state;
		}
	}

	якщо (state & RC_SERVICE_STARTED)
		якщо (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	якщо (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		якщо (dp) {
			struct dirent *d;
			поки ((d = readdir(dp))) {
				якщо (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					break;
				}
			}

			closedir(dp);
		}
	}

	повернути state;
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

	повернути buffer;
}

bool
rc_service_value_set(const char *service, const char *option, const char *value)
{
	bool ret = true;
	ціле optdirfd;
	FILE *fp;

	якщо (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		повернути false;

	якщо ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		повернути false;

	якщо (!value) {
		unlinkat(optdirfd, option, 0);
	} інакше якщо ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} інакше {
		ret = false;
	}

	close(optdirfd);
	повернути ret;
}

bool
rc_service_schedule_start(const char *service, const char *service_to_start)
{
	const char *base_to_start = basename_c(service_to_start);
	const char *base = basename_c(service);
	bool retval;
	ціле schedfd;
	char *init;

	/* service may be a provided service, like net */
	якщо (!service || !rc_service_exists(service_to_start))
		повернути false;

	якщо (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		повернути false;

	якщо ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		повернути false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	повернути retval;
}

bool
rc_service_schedule_clear(const char *service)
{
	повернути !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const char *runlevel)
{
	RC_STRINGLIST *list = NULL;

	якщо (!runlevel) {
		const ціле *scriptdirfds;
		list = rc_stringlist_new();
		для (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		повернути list;
	}

	/* These special levels never contain any services */
	якщо (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		повернути rc_stringlist_new();

	повернути ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	повернути list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	struct dirent *d;
	DIR *dp;

	якщо (state != RC_SERVICE_SCHEDULED)
		повернути ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	якщо (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		повернути list;

	поки ((d = readdir(dp))) {
		якщо (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	повернути list;
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

	якщо (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		повернути false;
	}

	якщо (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		повернути false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	якщо (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		якщо (path == NULL) {
			free(init);
			повернути false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		якщо (!retval) {
			free(init);
			errno = EPERM;
			повернути false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	якщо (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	повернути retval;
}

bool
rc_service_delete(const char *runlevel, const char *service)
{
	char *file;
	ціле r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	повернути (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const char *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	struct dirent *d;
	DIR *dp;

	якщо (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		повернути list;

	поки ((d = readdir(dp))) {
		char *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		якщо (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	повернути list;
}

RC_STRINGLIST *
rc_services_scheduled(const char *service)
{
	повернути ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const char *service, const char *env)
{
	size_t name_len = strcspn(env, "=");
	char var[strlen(service) + sizeof("/") + name_len];
	ціле dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	якщо (env[name_len] != '=')
		повернути errno = EINVAL, false;

	snprintf(var, sizeof(var), "%s/%.*s", service, (ціле) name_len, env);

	якщо (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		повернути false;

	якщо (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		повернути false;

	fputs(&env[name_len + 1], fp);

	повернути fclose(fp) == 0;
}

bool
rc_service_setenv(const char *service, const char *name, const char *value)
{
	char var[strlen(service) + sizeof("/") + strlen(name)];
	ціле dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	якщо (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		повернути false;

	якщо (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		повернути false;

	fputs(value, fp);

	повернути fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const char *service, const char *name, char **buf)
{
	char var[strlen(service) + sizeof("/") + strlen(name)], *value;
	ціле dfd = rc_dirfd(RC_DIR_ENVIRON);
	struct stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	якщо (fstatat(dfd, var, &stat, 0) == -1)
		повернути -1;

	якщо (!buf)
		повернути stat.st_size;

	якщо (!(value = xrealloc(*buf, stat.st_size + 1)))
		повернути -1;

	якщо (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		повернути -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	повернути count;
}
