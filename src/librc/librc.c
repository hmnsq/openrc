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
static целое dirfds[RC_DIR_MAX];

целое rc_dirfd(enum rc_dir dir) {
	целое flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	целое dirfd;

	если (dir < 0 || dir >= RC_DIR_MAX)
		вернуть -1;
	если (dirfds[dir] <= 0) {
		switch (dir) {
		case RC_DIR_SYSCONF:
			вернуть dirfds[dir] = open(rc_sysconfdir(), flags);
		case RC_DIR_USRCONF:
			вернуть dirfds[dir] = open(rc_usrconfdir(), flags);
		case RC_DIR_RUNLEVEL:
			вернуть dirfds[dir] = open(rc_runleveldir(), flags);
		case RC_DIR_SVCDIR:
			если (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				вернуть -1;
			вернуть dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			если ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				если (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					вернуть -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			break;
		}
	}
	вернуть dirfds[dir];
}

void clear_dirfds(void) {
	для (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		если (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(целое targetfd, const char *dir, целое options)
{
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *list = NULL;
	struct stat buf;
	size_t l;

	list = rc_stringlist_new();

	если (!(dp = do_opendirat(targetfd, dir)))
		вернуть list;

	пока (((d = readdir(dp)) != NULL)) {
		если (d->d_name[0] == '.')
			continue;
		если (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			если (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			если (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		если (options & LS_DIR) {
			если (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	вернуть list;
}

static bool
rm_dir(целое targetfd, const char *pathname, bool top)
{
	bool retval = true;
	struct dirent *d;
	struct stat s;
	DIR *dp;

	если (!(dp = do_opendirat(targetfd, pathname)))
		вернуть false;

	пока ((d = readdir(dp))) {
		если (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		если (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			break;
		}
		если (S_ISDIR(s.st_mode)) {
			если (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				break;
			}
		} иначе {
			если (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				break;
			}
		}
	}
	closedir(dp);

	если (!retval)
		вернуть false;

	если (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		вернуть false;

	вернуть true;
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
	целое result;

	если (!(fp = fopen(file, "r")))
		вернуть false;

	если ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		char buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, sizeof(buf));
		fprintf(stderr, "file_regex: %s", buf);
		вернуть false;
	}

	пока ((len = xgetline(&line, &size, fp)) != -1) {
		char *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			если (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			пока (str < line + len && *str == '\0')
				str++;
		} пока (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	вернуть retval;
}
#endif


static const char *
get_systype(void)
{
	char *systype = rc_conf_value("rc_sys");
	если (systype) {
		char *s = systype;
		/* Convert to uppercase */
		пока (s && *s) {
			если (islower((unsigned char) *s))
				*s = toupper((unsigned char) *s);
			s++;
		}
	}
	вернуть systype;
}

static const char *
detect_container(const char *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	если (systype) {
		если (strcmp(systype, RC_SYS_NONE) == 0)
		       вернуть NULL;
		если (strcmp(systype, RC_SYS_JAIL) == 0)
			вернуть RC_SYS_JAIL;
		если (strcmp(systype, RC_SYS_PODMAN) == 0)
			вернуть RC_SYS_PODMAN;
	}

	целое jailed = 0;
	size_t len = sizeof(jailed);

	если (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		если (jailed == 1)
			вернуть RC_SYS_JAIL;

	если (access("/var/run/.containerenv", F_OK) == 0)
		вернуть RC_SYS_PODMAN;
#endif

#ifdef __linux__
	если (systype) {
		если (strcmp(systype, RC_SYS_NONE) == 0)
			вернуть NULL;
		если (strcmp(systype, RC_SYS_UML) == 0)
			вернуть RC_SYS_UML;
		если (strcmp(systype, RC_SYS_VSERVER) == 0)
			вернуть RC_SYS_VSERVER;
		если (strcmp(systype, RC_SYS_OPENVZ) == 0)
			вернуть RC_SYS_OPENVZ;
		если (strcmp(systype, RC_SYS_LXC) == 0)
			вернуть RC_SYS_LXC;
		если (strcmp(systype, RC_SYS_RKT) == 0)
				вернуть RC_SYS_RKT;
		если (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				вернуть RC_SYS_SYSTEMD_NSPAWN;
		если (strcmp(systype, RC_SYS_DOCKER) == 0)
				вернуть RC_SYS_DOCKER;
		если (strcmp(systype, RC_SYS_PODMAN) == 0)
			вернуть RC_SYS_PODMAN;
		если (strcmp(systype, RC_SYS_WSL) == 0)
			вернуть RC_SYS_WSL;
	}
	если (file_regex("/proc/cpuinfo", "UML"))
		вернуть RC_SYS_UML;
	иначе если (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		вернуть RC_SYS_VSERVER;
	иначе если (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		вернуть RC_SYS_OPENVZ;
	иначе если (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		вернуть RC_SYS_OPENVZ; /* old test */
	иначе если (file_regex("/proc/1/environ", "container=lxc"))
		вернуть RC_SYS_LXC;
	иначе если (file_regex("/proc/1/environ", "container=rkt"))
		вернуть RC_SYS_RKT;
	иначе если (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		вернуть RC_SYS_SYSTEMD_NSPAWN;
	иначе если (access("/run/.containerenv", F_OK) == 0)
		вернуть RC_SYS_PODMAN;
	иначе если (access("/.dockerenv", F_OK) == 0)
		вернуть RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	иначе если (file_regex("/proc/1/environ", "container=docker"))
		вернуть RC_SYS_DOCKER;
	иначе если (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		вернуть RC_SYS_WSL;
	иначе если (getenv("WSL_DISTRO_NAME"))
		вернуть RC_SYS_WSL;
	иначе если (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		вернуть RC_SYS_WSL;
#endif

	вернуть NULL;
}

static const char *
detect_vm(const char *systype RC_UNUSED)
{
#ifdef __NetBSD__
	если (systype) {
		если (strcmp(systype, RC_SYS_NONE) == 0)
			вернуть NULL;
		если (strcmp(systype, RC_SYS_XEN0) == 0)
			вернуть RC_SYS_XEN0;
		если (strcmp(systype, RC_SYS_XENU) == 0)
			вернуть RC_SYS_XENU;
	}
	если (access("/kern/xen/privcmd", F_OK) == 0)
		вернуть RC_SYS_XEN0;
	если (access("/kern/xen", F_OK) == 0)
		вернуть RC_SYS_XENU;
#endif

#ifdef __linux__
	если (systype) {
		если (strcmp(systype, RC_SYS_NONE) == 0)
			вернуть NULL;
		если (strcmp(systype, RC_SYS_XEN0) == 0)
			вернуть RC_SYS_XEN0;
		если (strcmp(systype, RC_SYS_XENU) == 0)
			вернуть RC_SYS_XENU;
	}
	если (access("/proc/xen", F_OK) == 0) {
		если (file_regex("/proc/xen/capabilities", "control_d"))
			вернуть RC_SYS_XEN0;
		вернуть RC_SYS_XENU;
	}
#endif

	вернуть NULL;
}

const char *
rc_sys(void)
{
	const char *systype;
	const char *sys;

	systype = get_systype();
	sys = detect_container(systype);
	если (!sys) {
		sys = detect_vm(systype);
	}

	вернуть sys;
}

static const char *
rc_parse_service_state(RC_SERVICE state)
{
	целое i;

	для (i = 0; rc_service_state_names[i].name; i++) {
		если (rc_service_state_names[i].state == state)
			вернуть rc_service_state_names[i].name;
	}
	вернуть NULL;
}

static целое
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	для (size_t i = 0; rc_service_state_names[i].name; i++)
		если (rc_service_state_names[i].state == state)
			вернуть rc_service_state_names[i].dir;
	вернуть -1;
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
	если (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		вернуть;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	если (rc_stringlist_find(level_list, runlevel))
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
		если (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	вернуть faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	вернуть faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	вернуть ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

char *
rc_runlevel_get(void)
{
	FILE *fp;
	char *runlevel = NULL;
	size_t i;

	если ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(sizeof(char) * PATH_MAX);
		если (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			если (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} иначе
			*runlevel = '\0';
		fclose(fp);
	}

	если (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	вернуть runlevel;
}

bool
rc_runlevel_set(const char *runlevel)
{
	FILE *fp;

	если (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		вернуть false;

	fputs(runlevel, fp);
	fclose(fp);
	вернуть true;
}

bool
rc_runlevel_exists(const char *runlevel)
{
	struct stat buf;

	если (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		вернуть false;
	если (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		вернуть true;
	вернуть false;
}

bool
rc_runlevel_stack(const char *dst, const char *src)
{
	char *d, *s;
	целое r;

	если (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		вернуть false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	вернуть (r == 0);
}

bool
rc_runlevel_unstack(const char *dst, const char *src)
{
	char *path;
	целое r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	вернуть (r == 0);
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
	вернуть stack;
}

static const char *config_path[] = { _RC_PATH, NULL };

static struct {
	bool set;
	char *svcdir, *confdir, *runleveldir;
} rc_dirs;

static struct {
	char *buffer;
	size_t count;
	целое *fds;
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

	если (!ret)
		вернуть NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	если (!**str)
		*str = NULL;
	иначе
		*str += 1;

	вернуть ret;
}

static const char *
strnullsep(const char **str, size_t *size)
{
	const char *ret = *str;
	size_t len;

	если (!ret)
		вернуть NULL;

	len = strnlen(ret, *size - 1) + 1;

	если ((*size -= len))
		*str += len;
	иначе
		*str = NULL;

	вернуть ret;
}

static void
append_user_path(FILE *fp)
{
	const char *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	если (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	иначе пока ((entry = strnsep(&xdg, &size, ':')))
		если (size) append_path(fp, "%.*s/rc", (целое) size, entry);

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

	пока ((entry = strnsep(&path, &size, ':'))) {
		если (size)
			append_path(fp, "%.*s%s", (целое)size, entry, user ? "/user" : "");
		иначе если (user && !has_user)
			append_user_path(fp), has_user = true;
		иначе если (!user) пока (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const char *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	если (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	если (!(path = getenv("RC_PATH")) && !user)
		вернуть;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	если (path)
		append_rc_path(fp, path, user);
	иначе
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, sizeof(*rc_path.entries));

	path = rc_path.buffer;

	пока ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	целое _is_user = -1;

	если (_is_user == -1) {
		char *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	вернуть _is_user;
}

void
rc_set_user(void)
{
	char *env;

	если (is_user)
		вернуть;

	если (!rc_dirs.set)
		atexit(free_rc_dirs);
	иначе
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	если ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} иначе если ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} иначе {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	если (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	если (!rc_path.entries)
		load_dynamic_path(false);
	вернуть rc_path.entries;
}

size_t
rc_scriptdirfds(const целое **fds)
{
	static size_t len = 0;

	если (len == 0) {
		если (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * sizeof(*rc_path.fds));
		для (size_t i = 0; i < rc_path.count; i++) {
			целое fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			если (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	вернуть len;
}

const char *
rc_sysconfdir(void)
{
	вернуть RC_SYSCONFDIR;
}

const char *
rc_usrconfdir(void)
{
	вернуть rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const char *
rc_runleveldir(void)
{
	вернуть rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const char *
rc_svcdir(void)
{
	static const char *svcdir;

	если (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	вернуть rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(целое targetfd, const char *path, char **buf, size_t bufsiz)
{
	char *buffer;
	ssize_t r;

	для (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		если (r < 0) {
			free(buffer);
			вернуть -errno;
		}
		если ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			вернуть r;
		}
		free(buffer);
		если (bufsiz >= SSIZE_MAX - PATH_MAX)
			вернуть -ENOMEM;
	}
}

/* Resolve a service name to its full path */
char *
rc_service_resolve(const char *service)
{
	char *buffer;
	struct stat buf;

	если (!service)
		вернуть NULL;

	если (service[0] == '/')
		вернуть xstrdup(service);

	/* First check started services */
	если (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		если (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			вернуть buffer;
	если (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		если (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			вернуть buffer;

	для (const char * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		char *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		если (stat(file, &buf) == 0)
			вернуть file;
		free(file);
	}

	вернуть NULL;
}

bool
rc_service_exists(const char *service)
{
	char *file;
	bool retval = false;
	size_t len;
	struct stat buf;

	если (!service) {
		errno = EINVAL;
		вернуть false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	если (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		вернуть false;
	}

	если (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		вернуть false;
	}

	если (stat(file, &buf) == 0) {
		если (buf.st_mode & S_IXUGO)
			retval = true;
		иначе
			errno = ENOEXEC;
	}
	free(file);
	вернуть retval;
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

	если (!(svc = rc_service_resolve(service)))
		вернуть NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	если (!(fp = popen(cmd, "r"))) {
		free(cmd);
		вернуть NULL;
	}

	если (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		пока ((token = strsep(&p, " ")))
			если (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	вернуть commands;
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

	если (!(svc = rc_service_resolve(service)))
		вернуть NULL;

	если (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	если ((fp = popen(cmd, "r"))) {
		если (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	вернуть desc;
}

bool
rc_service_in_runlevel(const char *service, const char *runlevel)
{
	char *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	вернуть r;
}

bool
rc_service_mark(const char *service, const RC_SERVICE state)
{
	целое i = 0;
	const char *base;
	char *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	целое serrno;

	если (!init)
		вернуть false;

	base = basename_c(service);
	если (state != RC_SERVICE_STOPPED) {
		целое state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		если (access(init, F_OK) != 0) {
			free(init);
			вернуть false;
		}

		unlinkat(state_dirfd, base, 0);
		если ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			вернуть false;
		}
	}

	если (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		вернуть true;
	}

	/* Remove any old states now */
	для (i = 0; rc_service_state_names[i].name; i++) {
		целое state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		если (s == state)
			continue;

		switch (s) {
		case RC_SERVICE_STOPPED:
		case RC_SERVICE_HOTPLUGGED:
		case RC_SERVICE_SCHEDULED:
			continue;
		case RC_SERVICE_WASINACTIVE:
			если (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			если (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			если ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				если (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					вернуть false;
				}
				skip_wasinactive = true;
			}
			если (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				вернуть false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	если (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	если (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	если (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		если (dp) {
			struct dirent *d;
			пока ((d = readdir(dp))) {
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
	вернуть true;
}

RC_SERVICE
rc_service_state(const char *service)
{
	const char *base = basename_c(service);
	целое state = RC_SERVICE_STOPPED;
	целое i;

	для (i = 0; rc_service_state_names[i].name; i++) {
		если (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			если (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			иначе
				state |= rc_service_state_names[i].state;
		}
	}

	если (state & RC_SERVICE_STARTED)
		если (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	если (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		если (dp) {
			struct dirent *d;
			пока ((d = readdir(dp))) {
				если (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					break;
				}
			}

			closedir(dp);
		}
	}

	вернуть state;
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

	вернуть buffer;
}

bool
rc_service_value_set(const char *service, const char *option, const char *value)
{
	bool ret = true;
	целое optdirfd;
	FILE *fp;

	если (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		вернуть false;

	если ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		вернуть false;

	если (!value) {
		unlinkat(optdirfd, option, 0);
	} иначе если ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} иначе {
		ret = false;
	}

	close(optdirfd);
	вернуть ret;
}

bool
rc_service_schedule_start(const char *service, const char *service_to_start)
{
	const char *base_to_start = basename_c(service_to_start);
	const char *base = basename_c(service);
	bool retval;
	целое schedfd;
	char *init;

	/* service may be a provided service, like net */
	если (!service || !rc_service_exists(service_to_start))
		вернуть false;

	если (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		вернуть false;

	если ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		вернуть false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	вернуть retval;
}

bool
rc_service_schedule_clear(const char *service)
{
	вернуть !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const char *runlevel)
{
	RC_STRINGLIST *list = NULL;

	если (!runlevel) {
		const целое *scriptdirfds;
		list = rc_stringlist_new();
		для (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		вернуть list;
	}

	/* These special levels never contain any services */
	если (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		вернуть rc_stringlist_new();

	вернуть ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	вернуть list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	struct dirent *d;
	DIR *dp;

	если (state != RC_SERVICE_SCHEDULED)
		вернуть ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	если (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		вернуть list;

	пока ((d = readdir(dp))) {
		если (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	вернуть list;
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

	если (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		вернуть false;
	}

	если (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		вернуть false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	если (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		если (path == NULL) {
			free(init);
			вернуть false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		если (!retval) {
			free(init);
			errno = EPERM;
			вернуть false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	если (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	вернуть retval;
}

bool
rc_service_delete(const char *runlevel, const char *service)
{
	char *file;
	целое r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	вернуть (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const char *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	struct dirent *d;
	DIR *dp;

	если (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		вернуть list;

	пока ((d = readdir(dp))) {
		char *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		если (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	вернуть list;
}

RC_STRINGLIST *
rc_services_scheduled(const char *service)
{
	вернуть ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const char *service, const char *env)
{
	size_t name_len = strcspn(env, "=");
	char var[strlen(service) + sizeof("/") + name_len];
	целое dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	если (env[name_len] != '=')
		вернуть errno = EINVAL, false;

	snprintf(var, sizeof(var), "%s/%.*s", service, (целое) name_len, env);

	если (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		вернуть false;

	если (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		вернуть false;

	fputs(&env[name_len + 1], fp);

	вернуть fclose(fp) == 0;
}

bool
rc_service_setenv(const char *service, const char *name, const char *value)
{
	char var[strlen(service) + sizeof("/") + strlen(name)];
	целое dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	если (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		вернуть false;

	если (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		вернуть false;

	fputs(value, fp);

	вернуть fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const char *service, const char *name, char **buf)
{
	char var[strlen(service) + sizeof("/") + strlen(name)], *value;
	целое dfd = rc_dirfd(RC_DIR_ENVIRON);
	struct stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	если (fstatat(dfd, var, &stat, 0) == -1)
		вернуть -1;

	если (!buf)
		вернуть stat.st_size;

	если (!(value = xrealloc(*buf, stat.st_size + 1)))
		вернуть -1;

	если (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		вернуть -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	вернуть count;
}
