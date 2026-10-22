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
static ཧྲིལ dirfds[RC_DIR_MAX];

ཧྲིལ rc_dirfd(ga_nanaa rc_dir dir) {
	ཧྲིལ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ཧྲིལ dirfd;

	གལ་སྲིད (dir < 0 || dir >= RC_DIR_MAX)
		ལོག -1;
	གལ་སྲིད (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ལོག dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ལོག dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ལོག dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			གལ་སྲིད (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ལོག -1;
			ལོག dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			གལ་སྲིད ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				གལ་སྲིད (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ལོག -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ལོག dirfds[dir];
}

void clear_dirfds(void) {
	དོན་ལུ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		གལ་སྲིད (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(ཧྲིལ targetfd, const var_na *dir, ཧྲིལ options)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *list = NULL;
	sa_nracanaa stat buf;
	size_t l;

	list = rc_stringlist_new();

	གལ་སྲིད (!(dp = do_opendirat(targetfd, dir)))
		ལོག list;

	སྐབས (((d = readdir(dp)) != NULL)) {
		གལ་སྲིད (d->d_name[0] == '.')
			continue;
		གལ་སྲིད (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			གལ་སྲིད (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			གལ་སྲིད (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		གལ་སྲིད (options & LS_DIR) {
			གལ་སྲིད (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ལོག list;
}

static bool
rm_dir(ཧྲིལ targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	གལ་སྲིད (!(dp = do_opendirat(targetfd, pathname)))
		ལོག false;

	སྐབས ((d = readdir(dp))) {
		གལ་སྲིད (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		གལ་སྲིད (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		གལ་སྲིད (S_ISDIR(s.st_mode)) {
			གལ་སྲིད (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} དེ་མིན {
			གལ་སྲིད (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	གལ་སྲིད (!retval)
		ལོག false;

	གལ་སྲིད (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ལོག false;

	ལོག true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
static bool
file_regex(const var_na *file, const var_na *regex)
{
	FILE *fp;
	var_na *line = NULL;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	ཧྲིལ result;

	གལ་སྲིད (!(fp = fopen(file, "r")))
		ལོག false;

	གལ་སྲིད ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ལོག false;
	}

	སྐབས ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			གལ་སྲིད (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			སྐབས (str < line + len && *str == '\0')
				str++;
		} སྐབས (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ལོག retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	གལ་སྲིད (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		སྐབས (s && *s) {
			གལ་སྲིད (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ལོག systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	གལ་སྲིད (systype) {
		གལ་སྲིད (strcmp(systype, RC_SYS_NONE) == 0)
		       ལོག NULL;
		གལ་སྲིད (strcmp(systype, RC_SYS_JAIL) == 0)
			ལོག RC_SYS_JAIL;
		གལ་སྲིད (strcmp(systype, RC_SYS_PODMAN) == 0)
			ལོག RC_SYS_PODMAN;
	}

	ཧྲིལ jailed = 0;
	size_t len = parimaa_na(jailed);

	གལ་སྲིད (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		གལ་སྲིད (jailed == 1)
			ལོག RC_SYS_JAIL;

	གལ་སྲིད (access("/var/run/.containerenv", F_OK) == 0)
		ལོག RC_SYS_PODMAN;
#endif

#ifdef __linux__
	གལ་སྲིད (systype) {
		གལ་སྲིད (strcmp(systype, RC_SYS_NONE) == 0)
			ལོག NULL;
		གལ་སྲིད (strcmp(systype, RC_SYS_UML) == 0)
			ལོག RC_SYS_UML;
		གལ་སྲིད (strcmp(systype, RC_SYS_VSERVER) == 0)
			ལོག RC_SYS_VSERVER;
		གལ་སྲིད (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ལོག RC_SYS_OPENVZ;
		གལ་སྲིད (strcmp(systype, RC_SYS_LXC) == 0)
			ལོག RC_SYS_LXC;
		གལ་སྲིད (strcmp(systype, RC_SYS_RKT) == 0)
				ལོག RC_SYS_RKT;
		གལ་སྲིད (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ལོག RC_SYS_SYSTEMD_NSPAWN;
		གལ་སྲིད (strcmp(systype, RC_SYS_DOCKER) == 0)
				ལོག RC_SYS_DOCKER;
		གལ་སྲིད (strcmp(systype, RC_SYS_PODMAN) == 0)
			ལོག RC_SYS_PODMAN;
		གལ་སྲིད (strcmp(systype, RC_SYS_WSL) == 0)
			ལོག RC_SYS_WSL;
	}
	གལ་སྲིད (file_regex("/proc/cpuinfo", "UML"))
		ལོག RC_SYS_UML;
	དེ་མིན གལ་སྲིད (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ལོག RC_SYS_VSERVER;
	དེ་མིན གལ་སྲིད (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ལོག RC_SYS_OPENVZ;
	དེ་མིན གལ་སྲིད (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ལོག RC_SYS_OPENVZ; /* old test */
	དེ་མིན གལ་སྲིད (file_regex("/proc/1/environ", "container=lxc"))
		ལོག RC_SYS_LXC;
	དེ་མིན གལ་སྲིད (file_regex("/proc/1/environ", "container=rkt"))
		ལོག RC_SYS_RKT;
	དེ་མིན གལ་སྲིད (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ལོག RC_SYS_SYSTEMD_NSPAWN;
	དེ་མིན གལ་སྲིད (access("/run/.containerenv", F_OK) == 0)
		ལོག RC_SYS_PODMAN;
	དེ་མིན གལ་སྲིད (access("/.dockerenv", F_OK) == 0)
		ལོག RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	དེ་མིན གལ་སྲིད (file_regex("/proc/1/environ", "container=docker"))
		ལོག RC_SYS_DOCKER;
	དེ་མིན གལ་སྲིད (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ལོག RC_SYS_WSL;
	དེ་མིན གལ་སྲིད (getenv("WSL_DISTRO_NAME"))
		ལོག RC_SYS_WSL;
	དེ་མིན གལ་སྲིད (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ལོག RC_SYS_WSL;
#endif

	ལོག NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	གལ་སྲིད (systype) {
		གལ་སྲིད (strcmp(systype, RC_SYS_NONE) == 0)
			ལོག NULL;
		གལ་སྲིད (strcmp(systype, RC_SYS_XEN0) == 0)
			ལོག RC_SYS_XEN0;
		གལ་སྲིད (strcmp(systype, RC_SYS_XENU) == 0)
			ལོག RC_SYS_XENU;
	}
	གལ་སྲིད (access("/kern/xen/privcmd", F_OK) == 0)
		ལོག RC_SYS_XEN0;
	གལ་སྲིད (access("/kern/xen", F_OK) == 0)
		ལོག RC_SYS_XENU;
#endif

#ifdef __linux__
	གལ་སྲིད (systype) {
		གལ་སྲིད (strcmp(systype, RC_SYS_NONE) == 0)
			ལོག NULL;
		གལ་སྲིད (strcmp(systype, RC_SYS_XEN0) == 0)
			ལོག RC_SYS_XEN0;
		གལ་སྲིད (strcmp(systype, RC_SYS_XENU) == 0)
			ལོག RC_SYS_XENU;
	}
	གལ་སྲིད (access("/proc/xen", F_OK) == 0) {
		གལ་སྲིད (file_regex("/proc/xen/capabilities", "control_d"))
			ལོག RC_SYS_XEN0;
		ལོག RC_SYS_XENU;
	}
#endif

	ལོག NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	གལ་སྲིད (!sys) {
		sys = detect_vm(systype);
	}

	ལོག sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	ཧྲིལ i;

	དོན་ལུ (i = 0; rc_service_state_names[i].name; i++) {
		གལ་སྲིད (rc_service_state_names[i].state == state)
			ལོག rc_service_state_names[i].name;
	}
	ལོག NULL;
}

static ཧྲིལ
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	དོན་ལུ (size_t i = 0; rc_service_state_names[i].name; i++)
		གལ་སྲིད (rc_service_state_names[i].state == state)
			ལོག rc_service_state_names[i].dir;
	ལོག -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
static void
get_runlevel_chain(const var_na *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	const var_na *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	གལ་སྲིད (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ལོག;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	གལ་སྲིད (rc_stringlist_find(level_list, runlevel))
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
		གལ་སྲིད (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ལོག faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ལོག faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ལོག ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	གལ་སྲིད ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		གལ་སྲིད (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			གལ་སྲིད (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} དེ་མིན
			*runlevel = '\0';
		fclose(fp);
	}

	གལ་སྲིད (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ལོག runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	གལ་སྲིད (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ལོག false;

	fputs(runlevel, fp);
	fclose(fp);
	ལོག true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	གལ་སྲིད (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ལོག false;
	གལ་སྲིད (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ལོག true;
	ལོག false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	ཧྲིལ r;

	གལ་སྲིད (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ལོག false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ལོག (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	ཧྲིལ r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ལོག (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(const var_na *runlevel)
{
	RC_STRINGLIST *stack, *ancestor_list;
	stack = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, stack, ancestor_list);
	rc_stringlist_free(ancestor_list);
	ལོག stack;
}

static const var_na *config_path[] = { _RC_PATH, NULL };

static sa_nracanaa {
	bool set;
	var_na *svcdir, *confdir, *runleveldir;
} rc_dirs;

static sa_nracanaa {
	var_na *buffer;
	size_t count;
	ཧྲིལ *fds;
	const var_na **entries, **allocd;
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
append_path(FILE *fp, const var_na *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.count++;
}

static const var_na *
strnsep(const var_na **str, size_t *size, const var_na sep)
{
	const var_na *ret = *str;

	གལ་སྲིད (!ret)
		ལོག NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	གལ་སྲིད (!**str)
		*str = NULL;
	དེ་མིན
		*str += 1;

	ལོག ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	གལ་སྲིད (!ret)
		ལོག NULL;

	len = strnlen(ret, *size - 1) + 1;

	གལ་སྲིད ((*size -= len))
		*str += len;
	དེ་མིན
		*str = NULL;

	ལོག ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	གལ་སྲིད (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	དེ་མིན སྐབས ((entry = strnsep(&xdg, &size, ':')))
		གལ་སྲིད (size) append_path(fp, "%.*s/rc", (ཧྲིལ) size, entry);

	དོན་ལུ (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	སྐབས ((entry = strnsep(&path, &size, ':'))) {
		གལ་སྲིད (size)
			append_path(fp, "%.*s%s", (ཧྲིལ)size, entry, user ? "/user" : "");
		དེ་མིན གལ་སྲིད (user && !has_user)
			append_user_path(fp), has_user = true;
		དེ་མིན གལ་སྲིད (!user) སྐབས (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	གལ་སྲིད (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	གལ་སྲིད (!(path = getenv("RC_PATH")) && !user)
		ལོག;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	གལ་སྲིད (path)
		append_rc_path(fp, path, user);
	དེ་མིན
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	སྐབས ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	ཧྲིལ _is_user = -1;

	གལ་སྲིད (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ལོག _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	གལ་སྲིད (is_user)
		ལོག;

	གལ་སྲིད (!rc_dirs.set)
		atexit(free_rc_dirs);
	དེ་མིན
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	གལ་སྲིད ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} དེ་མིན གལ་སྲིད ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} དེ་མིན {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	གལ་སྲིད (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

const var_na * const *
rc_scriptdirs(void)
{
	གལ་སྲིད (!rc_path.entries)
		load_dynamic_path(false);
	ལོག rc_path.entries;
}

size_t
rc_scriptdirfds(const ཧྲིལ **fds)
{
	static size_t len = 0;

	གལ་སྲིད (len == 0) {
		གལ་སྲིད (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		དོན་ལུ (size_t i = 0; i < rc_path.count; i++) {
			ཧྲིལ fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			གལ་སྲིད (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ལོག len;
}

const var_na *
rc_sysconfdir(void)
{
	ལོག RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ལོག rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ལོག rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	གལ་སྲིད (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ལོག rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(ཧྲིལ targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	དོན་ལུ (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		གལ་སྲིད (r < 0) {
			free(buffer);
			ལོག -errno;
		}
		གལ་སྲིད ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ལོག r;
		}
		free(buffer);
		གལ་སྲིད (bufsiz >= SSIZE_MAX - PATH_MAX)
			ལོག -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	གལ་སྲིད (!service)
		ལོག NULL;

	གལ་སྲིད (service[0] == '/')
		ལོག xstrdup(service);

	/* First check started services */
	གལ་སྲིད (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		གལ་སྲིད (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ལོག buffer;
	གལ་སྲིད (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		གལ་སྲིད (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ལོག buffer;

	དོན་ལུ (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		གལ་སྲིད (stat(file, &buf) == 0)
			ལོག file;
		free(file);
	}

	ལོག NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	གལ་སྲིད (!service) {
		errno = EINVAL;
		ལོག false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	གལ་སྲིད (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ལོག false;
	}

	གལ་སྲིད (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ལོག false;
	}

	གལ་སྲིད (stat(file, &buf) == 0) {
		གལ་སྲིད (buf.st_mode & S_IXUGO)
			retval = true;
		དེ་མིན
			errno = ENOEXEC;
	}
	free(file);
	ལོག retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(const var_na *service)
{
	var_na *svc;
	var_na *cmd = NULL;
	var_na *buffer = NULL;
	size_t len = 0;
	RC_STRINGLIST *commands = NULL;
	var_na *token;
	var_na *p;
	FILE *fp;

	གལ་སྲིད (!(svc = rc_service_resolve(service)))
		ལོག NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	གལ་སྲིད (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ལོག NULL;
	}

	གལ་སྲིད (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		སྐབས ((token = strsep(&p, " ")))
			གལ་སྲིད (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ལོག commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
var_na *
rc_service_description(const var_na *service, const var_na *option)
{
	var_na *svc;
	var_na *cmd;
	var_na *desc = NULL;
	size_t size = 0;
	FILE *fp;

	གལ་སྲིད (!(svc = rc_service_resolve(service)))
		ལོག NULL;

	གལ་སྲིད (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	གལ་སྲིད ((fp = popen(cmd, "r"))) {
		གལ་སྲིད (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ལོག desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ལོག r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	ཧྲིལ i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ཧྲིལ serrno;

	གལ་སྲིད (!init)
		ལོག false;

	base = basename_c(service);
	གལ་སྲིད (state != RC_SERVICE_STOPPED) {
		ཧྲིལ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		གལ་སྲིད (access(init, F_OK) != 0) {
			free(init);
			ལོག false;
		}

		unlinkat(state_dirfd, base, 0);
		གལ་སྲིད ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ལོག false;
		}
	}

	གལ་སྲིད (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ལོག true;
	}

	/* Remove any old states now */
	དོན་ལུ (i = 0; rc_service_state_names[i].name; i++) {
		ཧྲིལ state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		གལ་སྲིད (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			གལ་སྲིད (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			གལ་སྲིད (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			གལ་སྲིད ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				གལ་སྲིད (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ལོག false;
				}
				skip_wasinactive = true;
			}
			གལ་སྲིད (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ལོག false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	གལ་སྲིད (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	གལ་སྲིད (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	གལ་སྲིད (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		གལ་སྲིད (dp) {
			sa_nracanaa dirent *d;
			སྐབས ((d = readdir(dp))) {
				var_na *was;

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
	ལོག true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	ཧྲིལ state = RC_SERVICE_STOPPED;
	ཧྲིལ i;

	དོན་ལུ (i = 0; rc_service_state_names[i].name; i++) {
		གལ་སྲིད (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			གལ་སྲིད (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			དེ་མིན
				state |= rc_service_state_names[i].state;
		}
	}

	གལ་སྲིད (state & RC_SERVICE_STARTED)
		གལ་སྲིད (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	གལ་སྲིད (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		གལ་སྲིད (dp) {
			sa_nracanaa dirent *d;
			སྐབས ((d = readdir(dp))) {
				གལ་སྲིད (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ལོག state;
}

var_na *
rc_service_value_get(const var_na *service, const var_na *option)
{
	var_na *buffer = NULL;
	size_t len = 0;
	var_na *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	free(file);

	ལོག buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	ཧྲིལ optdirfd;
	FILE *fp;

	གལ་སྲིད (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ལོག false;

	གལ་སྲིད ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ལོག false;

	གལ་སྲིད (!value) {
		unlinkat(optdirfd, option, 0);
	} དེ་མིན གལ་སྲིད ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} དེ་མིན {
		ret = false;
	}

	close(optdirfd);
	ལོག ret;
}

bool
rc_service_schedule_start(const var_na *service, const var_na *service_to_start)
{
	const var_na *base_to_start = basename_c(service_to_start);
	const var_na *base = basename_c(service);
	bool retval;
	ཧྲིལ schedfd;
	var_na *init;

	/* service may be a provided service, like net */
	གལ་སྲིད (!service || !rc_service_exists(service_to_start))
		ལོག false;

	གལ་སྲིད (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ལོག false;

	གལ་སྲིད ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ལོག false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ལོག retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ལོག !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	གལ་སྲིད (!runlevel) {
		const ཧྲིལ *scriptdirfds;
		list = rc_stringlist_new();
		དོན་ལུ (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ལོག list;
	}

	/* These special levels never contain any services */
	གལ་སྲིད (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ལོག rc_stringlist_new();

	ལོག ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(const var_na *runlevel)
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
	ལོག list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	གལ་སྲིད (state != RC_SERVICE_SCHEDULED)
		ལོག ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	གལ་སྲིད (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ལོག list;

	སྐབས ((d = readdir(dp))) {
		གལ་སྲིད (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ལོག list;
}

bool
rc_service_add(const var_na *runlevel, const var_na *service)
{
	bool retval;
	var_na *init;
	var_na *file;
	var_na *path;
	var_na *binit = NULL;
	var_na *i;

	གལ་སྲིད (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ལོག false;
	}

	གལ་སྲིད (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ལོག false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	གལ་སྲིད (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		གལ་སྲིད (path == NULL) {
			free(init);
			ལོག false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		གལ་སྲིད (!retval) {
			free(init);
			errno = EPERM;
			ལོག false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	གལ་སྲིད (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ལོག retval;
}

bool
rc_service_delete(const var_na *runlevel, const var_na *service)
{
	var_na *file;
	ཧྲིལ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	ལོག (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	གལ་སྲིད (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ལོག list;

	སྐབས ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		གལ་སྲིད (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ལོག list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ལོག ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	ཧྲིལ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	གལ་སྲིད (env[name_len] != '=')
		ལོག errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (ཧྲིལ) name_len, env);

	གལ་སྲིད (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ལོག false;

	གལ་སྲིད (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ལོག false;

	fputs(&env[name_len + 1], fp);

	ལོག fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	ཧྲིལ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	གལ་སྲིད (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ལོག false;

	གལ་སྲིད (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ལོག false;

	fputs(value, fp);

	ལོག fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const var_na *service, const var_na *name, var_na **buf)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)], *value;
	ཧྲིལ dfd = rc_dirfd(RC_DIR_ENVIRON);
	sa_nracanaa stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	གལ་སྲིད (fstatat(dfd, var, &stat, 0) == -1)
		ལོག -1;

	གལ་སྲིད (!buf)
		ལོག stat.st_size;

	གལ་སྲིད (!(value = xrealloc(*buf, stat.st_size + 1)))
		ལོག -1;

	གལ་སྲིད (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ལོག -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ལོག count;
}
