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
static 정수 dirfds[RC_DIR_MAX];

정수 rc_dirfd(enum rc_dir dir) {
	정수 flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	정수 dirfd;

	만약 (dir < 0 || dir >= RC_DIR_MAX)
		반환 -1;
	만약 (dirfds[dir] <= 0) {
		switch (dir) {
		case RC_DIR_SYSCONF:
			반환 dirfds[dir] = open(rc_sysconfdir(), flags);
		case RC_DIR_USRCONF:
			반환 dirfds[dir] = open(rc_usrconfdir(), flags);
		case RC_DIR_RUNLEVEL:
			반환 dirfds[dir] = open(rc_runleveldir(), flags);
		case RC_DIR_SVCDIR:
			만약 (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				반환 -1;
			반환 dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			만약 ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				만약 (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					반환 -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			break;
		}
	}
	반환 dirfds[dir];
}

void clear_dirfds(void) {
	위해 (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		만약 (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(정수 targetfd, const char *dir, 정수 options)
{
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *list = NULL;
	struct stat buf;
	size_t l;

	list = rc_stringlist_new();

	만약 (!(dp = do_opendirat(targetfd, dir)))
		반환 list;

	동안 (((d = readdir(dp)) != NULL)) {
		만약 (d->d_name[0] == '.')
			continue;
		만약 (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			만약 (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			만약 (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		만약 (options & LS_DIR) {
			만약 (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	반환 list;
}

static bool
rm_dir(정수 targetfd, const char *pathname, bool top)
{
	bool retval = true;
	struct dirent *d;
	struct stat s;
	DIR *dp;

	만약 (!(dp = do_opendirat(targetfd, pathname)))
		반환 false;

	동안 ((d = readdir(dp))) {
		만약 (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		만약 (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			break;
		}
		만약 (S_ISDIR(s.st_mode)) {
			만약 (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				break;
			}
		} 아니면 {
			만약 (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				break;
			}
		}
	}
	closedir(dp);

	만약 (!retval)
		반환 false;

	만약 (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		반환 false;

	반환 true;
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
	정수 result;

	만약 (!(fp = fopen(file, "r")))
		반환 false;

	만약 ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		char buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, sizeof(buf));
		fprintf(stderr, "file_regex: %s", buf);
		반환 false;
	}

	동안 ((len = xgetline(&line, &size, fp)) != -1) {
		char *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			만약 (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			동안 (str < line + len && *str == '\0')
				str++;
		} 동안 (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	반환 retval;
}
#endif


static const char *
get_systype(void)
{
	char *systype = rc_conf_value("rc_sys");
	만약 (systype) {
		char *s = systype;
		/* Convert to uppercase */
		동안 (s && *s) {
			만약 (islower((unsigned char) *s))
				*s = toupper((unsigned char) *s);
			s++;
		}
	}
	반환 systype;
}

static const char *
detect_container(const char *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	만약 (systype) {
		만약 (strcmp(systype, RC_SYS_NONE) == 0)
		       반환 NULL;
		만약 (strcmp(systype, RC_SYS_JAIL) == 0)
			반환 RC_SYS_JAIL;
		만약 (strcmp(systype, RC_SYS_PODMAN) == 0)
			반환 RC_SYS_PODMAN;
	}

	정수 jailed = 0;
	size_t len = sizeof(jailed);

	만약 (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		만약 (jailed == 1)
			반환 RC_SYS_JAIL;

	만약 (access("/var/run/.containerenv", F_OK) == 0)
		반환 RC_SYS_PODMAN;
#endif

#ifdef __linux__
	만약 (systype) {
		만약 (strcmp(systype, RC_SYS_NONE) == 0)
			반환 NULL;
		만약 (strcmp(systype, RC_SYS_UML) == 0)
			반환 RC_SYS_UML;
		만약 (strcmp(systype, RC_SYS_VSERVER) == 0)
			반환 RC_SYS_VSERVER;
		만약 (strcmp(systype, RC_SYS_OPENVZ) == 0)
			반환 RC_SYS_OPENVZ;
		만약 (strcmp(systype, RC_SYS_LXC) == 0)
			반환 RC_SYS_LXC;
		만약 (strcmp(systype, RC_SYS_RKT) == 0)
				반환 RC_SYS_RKT;
		만약 (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				반환 RC_SYS_SYSTEMD_NSPAWN;
		만약 (strcmp(systype, RC_SYS_DOCKER) == 0)
				반환 RC_SYS_DOCKER;
		만약 (strcmp(systype, RC_SYS_PODMAN) == 0)
			반환 RC_SYS_PODMAN;
		만약 (strcmp(systype, RC_SYS_WSL) == 0)
			반환 RC_SYS_WSL;
	}
	만약 (file_regex("/proc/cpuinfo", "UML"))
		반환 RC_SYS_UML;
	아니면 만약 (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		반환 RC_SYS_VSERVER;
	아니면 만약 (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		반환 RC_SYS_OPENVZ;
	아니면 만약 (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		반환 RC_SYS_OPENVZ; /* old test */
	아니면 만약 (file_regex("/proc/1/environ", "container=lxc"))
		반환 RC_SYS_LXC;
	아니면 만약 (file_regex("/proc/1/environ", "container=rkt"))
		반환 RC_SYS_RKT;
	아니면 만약 (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		반환 RC_SYS_SYSTEMD_NSPAWN;
	아니면 만약 (access("/run/.containerenv", F_OK) == 0)
		반환 RC_SYS_PODMAN;
	아니면 만약 (access("/.dockerenv", F_OK) == 0)
		반환 RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	아니면 만약 (file_regex("/proc/1/environ", "container=docker"))
		반환 RC_SYS_DOCKER;
	아니면 만약 (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		반환 RC_SYS_WSL;
	아니면 만약 (getenv("WSL_DISTRO_NAME"))
		반환 RC_SYS_WSL;
	아니면 만약 (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		반환 RC_SYS_WSL;
#endif

	반환 NULL;
}

static const char *
detect_vm(const char *systype RC_UNUSED)
{
#ifdef __NetBSD__
	만약 (systype) {
		만약 (strcmp(systype, RC_SYS_NONE) == 0)
			반환 NULL;
		만약 (strcmp(systype, RC_SYS_XEN0) == 0)
			반환 RC_SYS_XEN0;
		만약 (strcmp(systype, RC_SYS_XENU) == 0)
			반환 RC_SYS_XENU;
	}
	만약 (access("/kern/xen/privcmd", F_OK) == 0)
		반환 RC_SYS_XEN0;
	만약 (access("/kern/xen", F_OK) == 0)
		반환 RC_SYS_XENU;
#endif

#ifdef __linux__
	만약 (systype) {
		만약 (strcmp(systype, RC_SYS_NONE) == 0)
			반환 NULL;
		만약 (strcmp(systype, RC_SYS_XEN0) == 0)
			반환 RC_SYS_XEN0;
		만약 (strcmp(systype, RC_SYS_XENU) == 0)
			반환 RC_SYS_XENU;
	}
	만약 (access("/proc/xen", F_OK) == 0) {
		만약 (file_regex("/proc/xen/capabilities", "control_d"))
			반환 RC_SYS_XEN0;
		반환 RC_SYS_XENU;
	}
#endif

	반환 NULL;
}

const char *
rc_sys(void)
{
	const char *systype;
	const char *sys;

	systype = get_systype();
	sys = detect_container(systype);
	만약 (!sys) {
		sys = detect_vm(systype);
	}

	반환 sys;
}

static const char *
rc_parse_service_state(RC_SERVICE state)
{
	정수 i;

	위해 (i = 0; rc_service_state_names[i].name; i++) {
		만약 (rc_service_state_names[i].state == state)
			반환 rc_service_state_names[i].name;
	}
	반환 NULL;
}

static 정수
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	위해 (size_t i = 0; rc_service_state_names[i].name; i++)
		만약 (rc_service_state_names[i].state == state)
			반환 rc_service_state_names[i].dir;
	반환 -1;
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
	만약 (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		반환;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	만약 (rc_stringlist_find(level_list, runlevel))
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
		만약 (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	반환 faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	반환 faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	반환 ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

char *
rc_runlevel_get(void)
{
	FILE *fp;
	char *runlevel = NULL;
	size_t i;

	만약 ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(sizeof(char) * PATH_MAX);
		만약 (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			만약 (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} 아니면
			*runlevel = '\0';
		fclose(fp);
	}

	만약 (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	반환 runlevel;
}

bool
rc_runlevel_set(const char *runlevel)
{
	FILE *fp;

	만약 (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		반환 false;

	fputs(runlevel, fp);
	fclose(fp);
	반환 true;
}

bool
rc_runlevel_exists(const char *runlevel)
{
	struct stat buf;

	만약 (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		반환 false;
	만약 (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		반환 true;
	반환 false;
}

bool
rc_runlevel_stack(const char *dst, const char *src)
{
	char *d, *s;
	정수 r;

	만약 (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		반환 false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	반환 (r == 0);
}

bool
rc_runlevel_unstack(const char *dst, const char *src)
{
	char *path;
	정수 r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	반환 (r == 0);
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
	반환 stack;
}

static const char *config_path[] = { _RC_PATH, NULL };

static struct {
	bool set;
	char *svcdir, *confdir, *runleveldir;
} rc_dirs;

static struct {
	char *buffer;
	size_t count;
	정수 *fds;
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

	만약 (!ret)
		반환 NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	만약 (!**str)
		*str = NULL;
	아니면
		*str += 1;

	반환 ret;
}

static const char *
strnullsep(const char **str, size_t *size)
{
	const char *ret = *str;
	size_t len;

	만약 (!ret)
		반환 NULL;

	len = strnlen(ret, *size - 1) + 1;

	만약 ((*size -= len))
		*str += len;
	아니면
		*str = NULL;

	반환 ret;
}

static void
append_user_path(FILE *fp)
{
	const char *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	만약 (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	아니면 동안 ((entry = strnsep(&xdg, &size, ':')))
		만약 (size) append_path(fp, "%.*s/rc", (정수) size, entry);

	위해 (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const char *path, bool user)
{
	bool has_user = false;
	const char *entry;
	size_t size;

	동안 ((entry = strnsep(&path, &size, ':'))) {
		만약 (size)
			append_path(fp, "%.*s%s", (정수)size, entry, user ? "/user" : "");
		아니면 만약 (user && !has_user)
			append_user_path(fp), has_user = true;
		아니면 만약 (!user) 동안 (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const char *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	만약 (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	만약 (!(path = getenv("RC_PATH")) && !user)
		반환;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	만약 (path)
		append_rc_path(fp, path, user);
	아니면
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, sizeof(*rc_path.entries));

	path = rc_path.buffer;

	동안 ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	정수 _is_user = -1;

	만약 (_is_user == -1) {
		char *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	반환 _is_user;
}

void
rc_set_user(void)
{
	char *env;

	만약 (is_user)
		반환;

	만약 (!rc_dirs.set)
		atexit(free_rc_dirs);
	아니면
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	만약 ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} 아니면 만약 ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} 아니면 {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	만약 (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	만약 (!rc_path.entries)
		load_dynamic_path(false);
	반환 rc_path.entries;
}

size_t
rc_scriptdirfds(const 정수 **fds)
{
	static size_t len = 0;

	만약 (len == 0) {
		만약 (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * sizeof(*rc_path.fds));
		위해 (size_t i = 0; i < rc_path.count; i++) {
			정수 fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			만약 (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	반환 len;
}

const char *
rc_sysconfdir(void)
{
	반환 RC_SYSCONFDIR;
}

const char *
rc_usrconfdir(void)
{
	반환 rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const char *
rc_runleveldir(void)
{
	반환 rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const char *
rc_svcdir(void)
{
	static const char *svcdir;

	만약 (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	반환 rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(정수 targetfd, const char *path, char **buf, size_t bufsiz)
{
	char *buffer;
	ssize_t r;

	위해 (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		만약 (r < 0) {
			free(buffer);
			반환 -errno;
		}
		만약 ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			반환 r;
		}
		free(buffer);
		만약 (bufsiz >= SSIZE_MAX - PATH_MAX)
			반환 -ENOMEM;
	}
}

/* Resolve a service name to its full path */
char *
rc_service_resolve(const char *service)
{
	char *buffer;
	struct stat buf;

	만약 (!service)
		반환 NULL;

	만약 (service[0] == '/')
		반환 xstrdup(service);

	/* First check started services */
	만약 (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		만약 (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			반환 buffer;
	만약 (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		만약 (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			반환 buffer;

	위해 (const char * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		char *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		만약 (stat(file, &buf) == 0)
			반환 file;
		free(file);
	}

	반환 NULL;
}

bool
rc_service_exists(const char *service)
{
	char *file;
	bool retval = false;
	size_t len;
	struct stat buf;

	만약 (!service) {
		errno = EINVAL;
		반환 false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	만약 (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		반환 false;
	}

	만약 (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		반환 false;
	}

	만약 (stat(file, &buf) == 0) {
		만약 (buf.st_mode & S_IXUGO)
			retval = true;
		아니면
			errno = ENOEXEC;
	}
	free(file);
	반환 retval;
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

	만약 (!(svc = rc_service_resolve(service)))
		반환 NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	만약 (!(fp = popen(cmd, "r"))) {
		free(cmd);
		반환 NULL;
	}

	만약 (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		동안 ((token = strsep(&p, " ")))
			만약 (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	반환 commands;
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

	만약 (!(svc = rc_service_resolve(service)))
		반환 NULL;

	만약 (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	만약 ((fp = popen(cmd, "r"))) {
		만약 (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	반환 desc;
}

bool
rc_service_in_runlevel(const char *service, const char *runlevel)
{
	char *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	반환 r;
}

bool
rc_service_mark(const char *service, const RC_SERVICE state)
{
	정수 i = 0;
	const char *base;
	char *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	정수 serrno;

	만약 (!init)
		반환 false;

	base = basename_c(service);
	만약 (state != RC_SERVICE_STOPPED) {
		정수 state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		만약 (access(init, F_OK) != 0) {
			free(init);
			반환 false;
		}

		unlinkat(state_dirfd, base, 0);
		만약 ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			반환 false;
		}
	}

	만약 (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		반환 true;
	}

	/* Remove any old states now */
	위해 (i = 0; rc_service_state_names[i].name; i++) {
		정수 state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		만약 (s == state)
			continue;

		switch (s) {
		case RC_SERVICE_STOPPED:
		case RC_SERVICE_HOTPLUGGED:
		case RC_SERVICE_SCHEDULED:
			continue;
		case RC_SERVICE_WASINACTIVE:
			만약 (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			만약 (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			만약 ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				만약 (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					반환 false;
				}
				skip_wasinactive = true;
			}
			만약 (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				반환 false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	만약 (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	만약 (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	만약 (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		만약 (dp) {
			struct dirent *d;
			동안 ((d = readdir(dp))) {
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
	반환 true;
}

RC_SERVICE
rc_service_state(const char *service)
{
	const char *base = basename_c(service);
	정수 state = RC_SERVICE_STOPPED;
	정수 i;

	위해 (i = 0; rc_service_state_names[i].name; i++) {
		만약 (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			만약 (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			아니면
				state |= rc_service_state_names[i].state;
		}
	}

	만약 (state & RC_SERVICE_STARTED)
		만약 (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	만약 (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		만약 (dp) {
			struct dirent *d;
			동안 ((d = readdir(dp))) {
				만약 (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					break;
				}
			}

			closedir(dp);
		}
	}

	반환 state;
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

	반환 buffer;
}

bool
rc_service_value_set(const char *service, const char *option, const char *value)
{
	bool ret = true;
	정수 optdirfd;
	FILE *fp;

	만약 (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		반환 false;

	만약 ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		반환 false;

	만약 (!value) {
		unlinkat(optdirfd, option, 0);
	} 아니면 만약 ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} 아니면 {
		ret = false;
	}

	close(optdirfd);
	반환 ret;
}

bool
rc_service_schedule_start(const char *service, const char *service_to_start)
{
	const char *base_to_start = basename_c(service_to_start);
	const char *base = basename_c(service);
	bool retval;
	정수 schedfd;
	char *init;

	/* service may be a provided service, like net */
	만약 (!service || !rc_service_exists(service_to_start))
		반환 false;

	만약 (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		반환 false;

	만약 ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		반환 false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	반환 retval;
}

bool
rc_service_schedule_clear(const char *service)
{
	반환 !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const char *runlevel)
{
	RC_STRINGLIST *list = NULL;

	만약 (!runlevel) {
		const 정수 *scriptdirfds;
		list = rc_stringlist_new();
		위해 (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		반환 list;
	}

	/* These special levels never contain any services */
	만약 (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		반환 rc_stringlist_new();

	반환 ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	반환 list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	struct dirent *d;
	DIR *dp;

	만약 (state != RC_SERVICE_SCHEDULED)
		반환 ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	만약 (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		반환 list;

	동안 ((d = readdir(dp))) {
		만약 (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	반환 list;
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

	만약 (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		반환 false;
	}

	만약 (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		반환 false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	만약 (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		만약 (path == NULL) {
			free(init);
			반환 false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		만약 (!retval) {
			free(init);
			errno = EPERM;
			반환 false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	만약 (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	반환 retval;
}

bool
rc_service_delete(const char *runlevel, const char *service)
{
	char *file;
	정수 r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	반환 (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const char *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	struct dirent *d;
	DIR *dp;

	만약 (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		반환 list;

	동안 ((d = readdir(dp))) {
		char *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		만약 (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	반환 list;
}

RC_STRINGLIST *
rc_services_scheduled(const char *service)
{
	반환 ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const char *service, const char *env)
{
	size_t name_len = strcspn(env, "=");
	char var[strlen(service) + sizeof("/") + name_len];
	정수 dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	만약 (env[name_len] != '=')
		반환 errno = EINVAL, false;

	snprintf(var, sizeof(var), "%s/%.*s", service, (정수) name_len, env);

	만약 (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		반환 false;

	만약 (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		반환 false;

	fputs(&env[name_len + 1], fp);

	반환 fclose(fp) == 0;
}

bool
rc_service_setenv(const char *service, const char *name, const char *value)
{
	char var[strlen(service) + sizeof("/") + strlen(name)];
	정수 dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	만약 (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		반환 false;

	만약 (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		반환 false;

	fputs(value, fp);

	반환 fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const char *service, const char *name, char **buf)
{
	char var[strlen(service) + sizeof("/") + strlen(name)], *value;
	정수 dfd = rc_dirfd(RC_DIR_ENVIRON);
	struct stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	만약 (fstatat(dfd, var, &stat, 0) == -1)
		반환 -1;

	만약 (!buf)
		반환 stat.st_size;

	만약 (!(value = xrealloc(*buf, stat.st_size + 1)))
		반환 -1;

	만약 (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		반환 -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	반환 count;
}
