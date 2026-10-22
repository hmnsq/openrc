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
static цяло dirfds[RC_DIR_MAX];

цяло rc_dirfd(enum rc_dir dir) {
	цяло flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	цяло dirfd;

	ако (dir < 0 || dir >= RC_DIR_MAX)
		върни -1;
	ако (dirfds[dir] <= 0) {
		switch (dir) {
		case RC_DIR_SYSCONF:
			върни dirfds[dir] = open(rc_sysconfdir(), flags);
		case RC_DIR_USRCONF:
			върни dirfds[dir] = open(rc_usrconfdir(), flags);
		case RC_DIR_RUNLEVEL:
			върни dirfds[dir] = open(rc_runleveldir(), flags);
		case RC_DIR_SVCDIR:
			ако (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				върни -1;
			върни dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ако ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ако (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					върни -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			break;
		}
	}
	върни dirfds[dir];
}

void clear_dirfds(void) {
	за (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ако (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(цяло targetfd, const char *dir, цяло options)
{
	DIR *dp;
	struct dirent *d;
	RC_STRINGLIST *list = NULL;
	struct stat buf;
	size_t l;

	list = rc_stringlist_new();

	ако (!(dp = do_opendirat(targetfd, dir)))
		върни list;

	докато (((d = readdir(dp)) != NULL)) {
		ако (d->d_name[0] == '.')
			continue;
		ако (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ако (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ако (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ако (options & LS_DIR) {
			ако (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	върни list;
}

static bool
rm_dir(цяло targetfd, const char *pathname, bool top)
{
	bool retval = true;
	struct dirent *d;
	struct stat s;
	DIR *dp;

	ако (!(dp = do_opendirat(targetfd, pathname)))
		върни false;

	докато ((d = readdir(dp))) {
		ако (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ако (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			break;
		}
		ако (S_ISDIR(s.st_mode)) {
			ако (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				break;
			}
		} иначе {
			ако (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				break;
			}
		}
	}
	closedir(dp);

	ако (!retval)
		върни false;

	ако (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		върни false;

	върни true;
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
	цяло result;

	ако (!(fp = fopen(file, "r")))
		върни false;

	ако ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		char buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, sizeof(buf));
		fprintf(stderr, "file_regex: %s", buf);
		върни false;
	}

	докато ((len = xgetline(&line, &size, fp)) != -1) {
		char *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ако (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			докато (str < line + len && *str == '\0')
				str++;
		} докато (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	върни retval;
}
#endif


static const char *
get_systype(void)
{
	char *systype = rc_conf_value("rc_sys");
	ако (systype) {
		char *s = systype;
		/* Convert to uppercase */
		докато (s && *s) {
			ако (islower((unsigned char) *s))
				*s = toupper((unsigned char) *s);
			s++;
		}
	}
	върни systype;
}

static const char *
detect_container(const char *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ако (systype) {
		ако (strcmp(systype, RC_SYS_NONE) == 0)
		       върни NULL;
		ако (strcmp(systype, RC_SYS_JAIL) == 0)
			върни RC_SYS_JAIL;
		ако (strcmp(systype, RC_SYS_PODMAN) == 0)
			върни RC_SYS_PODMAN;
	}

	цяло jailed = 0;
	size_t len = sizeof(jailed);

	ако (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ако (jailed == 1)
			върни RC_SYS_JAIL;

	ако (access("/var/run/.containerenv", F_OK) == 0)
		върни RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ако (systype) {
		ако (strcmp(systype, RC_SYS_NONE) == 0)
			върни NULL;
		ако (strcmp(systype, RC_SYS_UML) == 0)
			върни RC_SYS_UML;
		ако (strcmp(systype, RC_SYS_VSERVER) == 0)
			върни RC_SYS_VSERVER;
		ако (strcmp(systype, RC_SYS_OPENVZ) == 0)
			върни RC_SYS_OPENVZ;
		ако (strcmp(systype, RC_SYS_LXC) == 0)
			върни RC_SYS_LXC;
		ако (strcmp(systype, RC_SYS_RKT) == 0)
				върни RC_SYS_RKT;
		ако (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				върни RC_SYS_SYSTEMD_NSPAWN;
		ако (strcmp(systype, RC_SYS_DOCKER) == 0)
				върни RC_SYS_DOCKER;
		ако (strcmp(systype, RC_SYS_PODMAN) == 0)
			върни RC_SYS_PODMAN;
		ако (strcmp(systype, RC_SYS_WSL) == 0)
			върни RC_SYS_WSL;
	}
	ако (file_regex("/proc/cpuinfo", "UML"))
		върни RC_SYS_UML;
	иначе ако (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		върни RC_SYS_VSERVER;
	иначе ако (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		върни RC_SYS_OPENVZ;
	иначе ако (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		върни RC_SYS_OPENVZ; /* old test */
	иначе ако (file_regex("/proc/1/environ", "container=lxc"))
		върни RC_SYS_LXC;
	иначе ако (file_regex("/proc/1/environ", "container=rkt"))
		върни RC_SYS_RKT;
	иначе ако (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		върни RC_SYS_SYSTEMD_NSPAWN;
	иначе ако (access("/run/.containerenv", F_OK) == 0)
		върни RC_SYS_PODMAN;
	иначе ако (access("/.dockerenv", F_OK) == 0)
		върни RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	иначе ако (file_regex("/proc/1/environ", "container=docker"))
		върни RC_SYS_DOCKER;
	иначе ако (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		върни RC_SYS_WSL;
	иначе ако (getenv("WSL_DISTRO_NAME"))
		върни RC_SYS_WSL;
	иначе ако (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		върни RC_SYS_WSL;
#endif

	върни NULL;
}

static const char *
detect_vm(const char *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ако (systype) {
		ако (strcmp(systype, RC_SYS_NONE) == 0)
			върни NULL;
		ако (strcmp(systype, RC_SYS_XEN0) == 0)
			върни RC_SYS_XEN0;
		ако (strcmp(systype, RC_SYS_XENU) == 0)
			върни RC_SYS_XENU;
	}
	ако (access("/kern/xen/privcmd", F_OK) == 0)
		върни RC_SYS_XEN0;
	ако (access("/kern/xen", F_OK) == 0)
		върни RC_SYS_XENU;
#endif

#ifdef __linux__
	ако (systype) {
		ако (strcmp(systype, RC_SYS_NONE) == 0)
			върни NULL;
		ако (strcmp(systype, RC_SYS_XEN0) == 0)
			върни RC_SYS_XEN0;
		ако (strcmp(systype, RC_SYS_XENU) == 0)
			върни RC_SYS_XENU;
	}
	ако (access("/proc/xen", F_OK) == 0) {
		ако (file_regex("/proc/xen/capabilities", "control_d"))
			върни RC_SYS_XEN0;
		върни RC_SYS_XENU;
	}
#endif

	върни NULL;
}

const char *
rc_sys(void)
{
	const char *systype;
	const char *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ако (!sys) {
		sys = detect_vm(systype);
	}

	върни sys;
}

static const char *
rc_parse_service_state(RC_SERVICE state)
{
	цяло i;

	за (i = 0; rc_service_state_names[i].name; i++) {
		ако (rc_service_state_names[i].state == state)
			върни rc_service_state_names[i].name;
	}
	върни NULL;
}

static цяло
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	за (size_t i = 0; rc_service_state_names[i].name; i++)
		ако (rc_service_state_names[i].state == state)
			върни rc_service_state_names[i].dir;
	върни -1;
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
	ако (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		върни;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ако (rc_stringlist_find(level_list, runlevel))
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
		ако (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	върни faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	върни faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	върни ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

char *
rc_runlevel_get(void)
{
	FILE *fp;
	char *runlevel = NULL;
	size_t i;

	ако ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(sizeof(char) * PATH_MAX);
		ако (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ако (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} иначе
			*runlevel = '\0';
		fclose(fp);
	}

	ако (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	върни runlevel;
}

bool
rc_runlevel_set(const char *runlevel)
{
	FILE *fp;

	ако (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		върни false;

	fputs(runlevel, fp);
	fclose(fp);
	върни true;
}

bool
rc_runlevel_exists(const char *runlevel)
{
	struct stat buf;

	ако (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		върни false;
	ако (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		върни true;
	върни false;
}

bool
rc_runlevel_stack(const char *dst, const char *src)
{
	char *d, *s;
	цяло r;

	ако (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		върни false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	върни (r == 0);
}

bool
rc_runlevel_unstack(const char *dst, const char *src)
{
	char *path;
	цяло r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	върни (r == 0);
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
	върни stack;
}

static const char *config_path[] = { _RC_PATH, NULL };

static struct {
	bool set;
	char *svcdir, *confdir, *runleveldir;
} rc_dirs;

static struct {
	char *buffer;
	size_t count;
	цяло *fds;
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

	ако (!ret)
		върни NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ако (!**str)
		*str = NULL;
	иначе
		*str += 1;

	върни ret;
}

static const char *
strnullsep(const char **str, size_t *size)
{
	const char *ret = *str;
	size_t len;

	ако (!ret)
		върни NULL;

	len = strnlen(ret, *size - 1) + 1;

	ако ((*size -= len))
		*str += len;
	иначе
		*str = NULL;

	върни ret;
}

static void
append_user_path(FILE *fp)
{
	const char *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ако (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	иначе докато ((entry = strnsep(&xdg, &size, ':')))
		ако (size) append_path(fp, "%.*s/rc", (цяло) size, entry);

	за (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const char *path, bool user)
{
	bool has_user = false;
	const char *entry;
	size_t size;

	докато ((entry = strnsep(&path, &size, ':'))) {
		ако (size)
			append_path(fp, "%.*s%s", (цяло)size, entry, user ? "/user" : "");
		иначе ако (user && !has_user)
			append_user_path(fp), has_user = true;
		иначе ако (!user) докато (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const char *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ако (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ако (!(path = getenv("RC_PATH")) && !user)
		върни;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ако (path)
		append_rc_path(fp, path, user);
	иначе
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, sizeof(*rc_path.entries));

	path = rc_path.buffer;

	докато ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	цяло _is_user = -1;

	ако (_is_user == -1) {
		char *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	върни _is_user;
}

void
rc_set_user(void)
{
	char *env;

	ако (is_user)
		върни;

	ако (!rc_dirs.set)
		atexit(free_rc_dirs);
	иначе
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ако ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} иначе ако ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} иначе {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ако (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ако (!rc_path.entries)
		load_dynamic_path(false);
	върни rc_path.entries;
}

size_t
rc_scriptdirfds(const цяло **fds)
{
	static size_t len = 0;

	ако (len == 0) {
		ако (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * sizeof(*rc_path.fds));
		за (size_t i = 0; i < rc_path.count; i++) {
			цяло fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ако (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	върни len;
}

const char *
rc_sysconfdir(void)
{
	върни RC_SYSCONFDIR;
}

const char *
rc_usrconfdir(void)
{
	върни rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const char *
rc_runleveldir(void)
{
	върни rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const char *
rc_svcdir(void)
{
	static const char *svcdir;

	ако (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	върни rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(цяло targetfd, const char *path, char **buf, size_t bufsiz)
{
	char *buffer;
	ssize_t r;

	за (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ако (r < 0) {
			free(buffer);
			върни -errno;
		}
		ако ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			върни r;
		}
		free(buffer);
		ако (bufsiz >= SSIZE_MAX - PATH_MAX)
			върни -ENOMEM;
	}
}

/* Resolve a service name to its full path */
char *
rc_service_resolve(const char *service)
{
	char *buffer;
	struct stat buf;

	ако (!service)
		върни NULL;

	ако (service[0] == '/')
		върни xstrdup(service);

	/* First check started services */
	ако (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ако (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			върни buffer;
	ако (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ако (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			върни buffer;

	за (const char * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		char *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ако (stat(file, &buf) == 0)
			върни file;
		free(file);
	}

	върни NULL;
}

bool
rc_service_exists(const char *service)
{
	char *file;
	bool retval = false;
	size_t len;
	struct stat buf;

	ако (!service) {
		errno = EINVAL;
		върни false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ако (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		върни false;
	}

	ако (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		върни false;
	}

	ако (stat(file, &buf) == 0) {
		ако (buf.st_mode & S_IXUGO)
			retval = true;
		иначе
			errno = ENOEXEC;
	}
	free(file);
	върни retval;
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

	ако (!(svc = rc_service_resolve(service)))
		върни NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ако (!(fp = popen(cmd, "r"))) {
		free(cmd);
		върни NULL;
	}

	ако (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		докато ((token = strsep(&p, " ")))
			ако (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	върни commands;
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

	ако (!(svc = rc_service_resolve(service)))
		върни NULL;

	ако (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ако ((fp = popen(cmd, "r"))) {
		ако (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	върни desc;
}

bool
rc_service_in_runlevel(const char *service, const char *runlevel)
{
	char *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	върни r;
}

bool
rc_service_mark(const char *service, const RC_SERVICE state)
{
	цяло i = 0;
	const char *base;
	char *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	цяло serrno;

	ако (!init)
		върни false;

	base = basename_c(service);
	ако (state != RC_SERVICE_STOPPED) {
		цяло state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ако (access(init, F_OK) != 0) {
			free(init);
			върни false;
		}

		unlinkat(state_dirfd, base, 0);
		ако ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			върни false;
		}
	}

	ако (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		върни true;
	}

	/* Remove any old states now */
	за (i = 0; rc_service_state_names[i].name; i++) {
		цяло state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ако (s == state)
			continue;

		switch (s) {
		case RC_SERVICE_STOPPED:
		case RC_SERVICE_HOTPLUGGED:
		case RC_SERVICE_SCHEDULED:
			continue;
		case RC_SERVICE_WASINACTIVE:
			ако (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ако (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ако ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ако (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					върни false;
				}
				skip_wasinactive = true;
			}
			ако (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				върни false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ако (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ако (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ако (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ако (dp) {
			struct dirent *d;
			докато ((d = readdir(dp))) {
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
	върни true;
}

RC_SERVICE
rc_service_state(const char *service)
{
	const char *base = basename_c(service);
	цяло state = RC_SERVICE_STOPPED;
	цяло i;

	за (i = 0; rc_service_state_names[i].name; i++) {
		ако (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ако (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			иначе
				state |= rc_service_state_names[i].state;
		}
	}

	ако (state & RC_SERVICE_STARTED)
		ако (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ако (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ако (dp) {
			struct dirent *d;
			докато ((d = readdir(dp))) {
				ако (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					break;
				}
			}

			closedir(dp);
		}
	}

	върни state;
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

	върни buffer;
}

bool
rc_service_value_set(const char *service, const char *option, const char *value)
{
	bool ret = true;
	цяло optdirfd;
	FILE *fp;

	ако (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		върни false;

	ако ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		върни false;

	ако (!value) {
		unlinkat(optdirfd, option, 0);
	} иначе ако ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} иначе {
		ret = false;
	}

	close(optdirfd);
	върни ret;
}

bool
rc_service_schedule_start(const char *service, const char *service_to_start)
{
	const char *base_to_start = basename_c(service_to_start);
	const char *base = basename_c(service);
	bool retval;
	цяло schedfd;
	char *init;

	/* service may be a provided service, like net */
	ако (!service || !rc_service_exists(service_to_start))
		върни false;

	ако (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		върни false;

	ако ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		върни false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	върни retval;
}

bool
rc_service_schedule_clear(const char *service)
{
	върни !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const char *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ако (!runlevel) {
		const цяло *scriptdirfds;
		list = rc_stringlist_new();
		за (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		върни list;
	}

	/* These special levels never contain any services */
	ако (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		върни rc_stringlist_new();

	върни ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	върни list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	struct dirent *d;
	DIR *dp;

	ако (state != RC_SERVICE_SCHEDULED)
		върни ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ако (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		върни list;

	докато ((d = readdir(dp))) {
		ако (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	върни list;
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

	ако (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		върни false;
	}

	ако (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		върни false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ако (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ако (path == NULL) {
			free(init);
			върни false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ако (!retval) {
			free(init);
			errno = EPERM;
			върни false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ако (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	върни retval;
}

bool
rc_service_delete(const char *runlevel, const char *service)
{
	char *file;
	цяло r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	върни (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const char *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	struct dirent *d;
	DIR *dp;

	ако (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		върни list;

	докато ((d = readdir(dp))) {
		char *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ако (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	върни list;
}

RC_STRINGLIST *
rc_services_scheduled(const char *service)
{
	върни ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const char *service, const char *env)
{
	size_t name_len = strcspn(env, "=");
	char var[strlen(service) + sizeof("/") + name_len];
	цяло dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ако (env[name_len] != '=')
		върни errno = EINVAL, false;

	snprintf(var, sizeof(var), "%s/%.*s", service, (цяло) name_len, env);

	ако (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		върни false;

	ако (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		върни false;

	fputs(&env[name_len + 1], fp);

	върни fclose(fp) == 0;
}

bool
rc_service_setenv(const char *service, const char *name, const char *value)
{
	char var[strlen(service) + sizeof("/") + strlen(name)];
	цяло dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	ако (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		върни false;

	ако (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		върни false;

	fputs(value, fp);

	върни fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const char *service, const char *name, char **buf)
{
	char var[strlen(service) + sizeof("/") + strlen(name)], *value;
	цяло dfd = rc_dirfd(RC_DIR_ENVIRON);
	struct stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, sizeof(var), "%s/%s", service, name);

	ако (fstatat(dfd, var, &stat, 0) == -1)
		върни -1;

	ако (!buf)
		върни stat.st_size;

	ако (!(value = xrealloc(*buf, stat.st_size + 1)))
		върни -1;

	ако (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		върни -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	върни count;
}
