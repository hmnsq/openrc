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
static ចំនួនគត់ dirfds[RC_DIR_MAX];

ចំនួនគត់ rc_dirfd(ga_nanaa rc_dir dir) {
	ចំនួនគត់ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ចំនួនគត់ dirfd;

	ប្រសិនបើ (dir < 0 || dir >= RC_DIR_MAX)
		ត្រឡប់ -1;
	ប្រសិនបើ (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ត្រឡប់ dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ត្រឡប់ dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ត្រឡប់ dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			ប្រសិនបើ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ត្រឡប់ -1;
			ត្រឡប់ dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ប្រសិនបើ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ប្រសិនបើ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ត្រឡប់ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ត្រឡប់ dirfds[dir];
}

void clear_dirfds(void) {
	សម្រាប់ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ប្រសិនបើ (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(ចំនួនគត់ targetfd, const var_na *dir, ចំនួនគត់ options)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *list = NULL;
	sa_nracanaa stat buf;
	size_t l;

	list = rc_stringlist_new();

	ប្រសិនបើ (!(dp = do_opendirat(targetfd, dir)))
		ត្រឡប់ list;

	ខណៈពេល (((d = readdir(dp)) != NULL)) {
		ប្រសិនបើ (d->d_name[0] == '.')
			continue;
		ប្រសិនបើ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ប្រសិនបើ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ប្រសិនបើ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ប្រសិនបើ (options & LS_DIR) {
			ប្រសិនបើ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ត្រឡប់ list;
}

static bool
rm_dir(ចំនួនគត់ targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	ប្រសិនបើ (!(dp = do_opendirat(targetfd, pathname)))
		ត្រឡប់ false;

	ខណៈពេល ((d = readdir(dp))) {
		ប្រសិនបើ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ប្រសិនបើ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		ប្រសិនបើ (S_ISDIR(s.st_mode)) {
			ប្រសិនបើ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	ប្រសិនបើ (!retval)
		ត្រឡប់ false;

	ប្រសិនបើ (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ត្រឡប់ false;

	ត្រឡប់ true;
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
	ចំនួនគត់ result;

	ប្រសិនបើ (!(fp = fopen(file, "r")))
		ត្រឡប់ false;

	ប្រសិនបើ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ត្រឡប់ false;
	}

	ខណៈពេល ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ប្រសិនបើ (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ខណៈពេល (str < line + len && *str == '\0')
				str++;
		} ខណៈពេល (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ត្រឡប់ retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	ប្រសិនបើ (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		ខណៈពេល (s && *s) {
			ប្រសិនបើ (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ត្រឡប់ systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ប្រសិនបើ (systype) {
		ប្រសិនបើ (strcmp(systype, RC_SYS_NONE) == 0)
		       ត្រឡប់ NULL;
		ប្រសិនបើ (strcmp(systype, RC_SYS_JAIL) == 0)
			ត្រឡប់ RC_SYS_JAIL;
		ប្រសិនបើ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ត្រឡប់ RC_SYS_PODMAN;
	}

	ចំនួនគត់ jailed = 0;
	size_t len = parimaa_na(jailed);

	ប្រសិនបើ (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ប្រសិនបើ (jailed == 1)
			ត្រឡប់ RC_SYS_JAIL;

	ប្រសិនបើ (access("/var/run/.containerenv", F_OK) == 0)
		ត្រឡប់ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ប្រសិនបើ (systype) {
		ប្រសិនបើ (strcmp(systype, RC_SYS_NONE) == 0)
			ត្រឡប់ NULL;
		ប្រសិនបើ (strcmp(systype, RC_SYS_UML) == 0)
			ត្រឡប់ RC_SYS_UML;
		ប្រសិនបើ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ត្រឡប់ RC_SYS_VSERVER;
		ប្រសិនបើ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ត្រឡប់ RC_SYS_OPENVZ;
		ប្រសិនបើ (strcmp(systype, RC_SYS_LXC) == 0)
			ត្រឡប់ RC_SYS_LXC;
		ប្រសិនបើ (strcmp(systype, RC_SYS_RKT) == 0)
				ត្រឡប់ RC_SYS_RKT;
		ប្រសិនបើ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ត្រឡប់ RC_SYS_SYSTEMD_NSPAWN;
		ប្រសិនបើ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ត្រឡប់ RC_SYS_DOCKER;
		ប្រសិនបើ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ត្រឡប់ RC_SYS_PODMAN;
		ប្រសិនបើ (strcmp(systype, RC_SYS_WSL) == 0)
			ត្រឡប់ RC_SYS_WSL;
	}
	ប្រសិនបើ (file_regex("/proc/cpuinfo", "UML"))
		ត្រឡប់ RC_SYS_UML;
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ត្រឡប់ RC_SYS_VSERVER;
	បើមិនដូច្នេះ ប្រសិនបើ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ត្រឡប់ RC_SYS_OPENVZ;
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ត្រឡប់ RC_SYS_OPENVZ; /* old test */
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/1/environ", "container=lxc"))
		ត្រឡប់ RC_SYS_LXC;
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/1/environ", "container=rkt"))
		ត្រឡប់ RC_SYS_RKT;
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ត្រឡប់ RC_SYS_SYSTEMD_NSPAWN;
	បើមិនដូច្នេះ ប្រសិនបើ (access("/run/.containerenv", F_OK) == 0)
		ត្រឡប់ RC_SYS_PODMAN;
	បើមិនដូច្នេះ ប្រសិនបើ (access("/.dockerenv", F_OK) == 0)
		ត្រឡប់ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/1/environ", "container=docker"))
		ត្រឡប់ RC_SYS_DOCKER;
	បើមិនដូច្នេះ ប្រសិនបើ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ត្រឡប់ RC_SYS_WSL;
	បើមិនដូច្នេះ ប្រសិនបើ (getenv("WSL_DISTRO_NAME"))
		ត្រឡប់ RC_SYS_WSL;
	បើមិនដូច្នេះ ប្រសិនបើ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ត្រឡប់ RC_SYS_WSL;
#endif

	ត្រឡប់ NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ប្រសិនបើ (systype) {
		ប្រសិនបើ (strcmp(systype, RC_SYS_NONE) == 0)
			ត្រឡប់ NULL;
		ប្រសិនបើ (strcmp(systype, RC_SYS_XEN0) == 0)
			ត្រឡប់ RC_SYS_XEN0;
		ប្រសិនបើ (strcmp(systype, RC_SYS_XENU) == 0)
			ត្រឡប់ RC_SYS_XENU;
	}
	ប្រសិនបើ (access("/kern/xen/privcmd", F_OK) == 0)
		ត្រឡប់ RC_SYS_XEN0;
	ប្រសិនបើ (access("/kern/xen", F_OK) == 0)
		ត្រឡប់ RC_SYS_XENU;
#endif

#ifdef __linux__
	ប្រសិនបើ (systype) {
		ប្រសិនបើ (strcmp(systype, RC_SYS_NONE) == 0)
			ត្រឡប់ NULL;
		ប្រសិនបើ (strcmp(systype, RC_SYS_XEN0) == 0)
			ត្រឡប់ RC_SYS_XEN0;
		ប្រសិនបើ (strcmp(systype, RC_SYS_XENU) == 0)
			ត្រឡប់ RC_SYS_XENU;
	}
	ប្រសិនបើ (access("/proc/xen", F_OK) == 0) {
		ប្រសិនបើ (file_regex("/proc/xen/capabilities", "control_d"))
			ត្រឡប់ RC_SYS_XEN0;
		ត្រឡប់ RC_SYS_XENU;
	}
#endif

	ត្រឡប់ NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ប្រសិនបើ (!sys) {
		sys = detect_vm(systype);
	}

	ត្រឡប់ sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	ចំនួនគត់ i;

	សម្រាប់ (i = 0; rc_service_state_names[i].name; i++) {
		ប្រសិនបើ (rc_service_state_names[i].state == state)
			ត្រឡប់ rc_service_state_names[i].name;
	}
	ត្រឡប់ NULL;
}

static ចំនួនគត់
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	សម្រាប់ (size_t i = 0; rc_service_state_names[i].name; i++)
		ប្រសិនបើ (rc_service_state_names[i].state == state)
			ត្រឡប់ rc_service_state_names[i].dir;
	ត្រឡប់ -1;
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
	ប្រសិនបើ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ត្រឡប់;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ប្រសិនបើ (rc_stringlist_find(level_list, runlevel))
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
		ប្រសិនបើ (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ត្រឡប់ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ត្រឡប់ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ត្រឡប់ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	ប្រសិនបើ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		ប្រសិនបើ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ប្រសិនបើ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} បើមិនដូច្នេះ
			*runlevel = '\0';
		fclose(fp);
	}

	ប្រសិនបើ (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ត្រឡប់ runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	ប្រសិនបើ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ត្រឡប់ false;

	fputs(runlevel, fp);
	fclose(fp);
	ត្រឡប់ true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	ប្រសិនបើ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ត្រឡប់ false;
	ប្រសិនបើ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ត្រឡប់ true;
	ត្រឡប់ false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	ចំនួនគត់ r;

	ប្រសិនបើ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ត្រឡប់ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ត្រឡប់ (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	ចំនួនគត់ r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ត្រឡប់ (r == 0);
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
	ត្រឡប់ stack;
}

static const var_na *config_path[] = { _RC_PATH, NULL };

static sa_nracanaa {
	bool set;
	var_na *svcdir, *confdir, *runleveldir;
} rc_dirs;

static sa_nracanaa {
	var_na *buffer;
	size_t count;
	ចំនួនគត់ *fds;
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

	ប្រសិនបើ (!ret)
		ត្រឡប់ NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ប្រសិនបើ (!**str)
		*str = NULL;
	បើមិនដូច្នេះ
		*str += 1;

	ត្រឡប់ ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	ប្រសិនបើ (!ret)
		ត្រឡប់ NULL;

	len = strnlen(ret, *size - 1) + 1;

	ប្រសិនបើ ((*size -= len))
		*str += len;
	បើមិនដូច្នេះ
		*str = NULL;

	ត្រឡប់ ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ប្រសិនបើ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	បើមិនដូច្នេះ ខណៈពេល ((entry = strnsep(&xdg, &size, ':')))
		ប្រសិនបើ (size) append_path(fp, "%.*s/rc", (ចំនួនគត់) size, entry);

	សម្រាប់ (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	ខណៈពេល ((entry = strnsep(&path, &size, ':'))) {
		ប្រសិនបើ (size)
			append_path(fp, "%.*s%s", (ចំនួនគត់)size, entry, user ? "/user" : "");
		បើមិនដូច្នេះ ប្រសិនបើ (user && !has_user)
			append_user_path(fp), has_user = true;
		បើមិនដូច្នេះ ប្រសិនបើ (!user) ខណៈពេល (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ប្រសិនបើ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ប្រសិនបើ (!(path = getenv("RC_PATH")) && !user)
		ត្រឡប់;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ប្រសិនបើ (path)
		append_rc_path(fp, path, user);
	បើមិនដូច្នេះ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	ខណៈពេល ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	ចំនួនគត់ _is_user = -1;

	ប្រសិនបើ (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ត្រឡប់ _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	ប្រសិនបើ (is_user)
		ត្រឡប់;

	ប្រសិនបើ (!rc_dirs.set)
		atexit(free_rc_dirs);
	បើមិនដូច្នេះ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ប្រសិនបើ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} បើមិនដូច្នេះ ប្រសិនបើ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} បើមិនដូច្នេះ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ប្រសិនបើ (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ប្រសិនបើ (!rc_path.entries)
		load_dynamic_path(false);
	ត្រឡប់ rc_path.entries;
}

size_t
rc_scriptdirfds(const ចំនួនគត់ **fds)
{
	static size_t len = 0;

	ប្រសិនបើ (len == 0) {
		ប្រសិនបើ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		សម្រាប់ (size_t i = 0; i < rc_path.count; i++) {
			ចំនួនគត់ fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ប្រសិនបើ (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ត្រឡប់ len;
}

const var_na *
rc_sysconfdir(void)
{
	ត្រឡប់ RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ត្រឡប់ rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ត្រឡប់ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	ប្រសិនបើ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ត្រឡប់ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(ចំនួនគត់ targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	សម្រាប់ (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ប្រសិនបើ (r < 0) {
			free(buffer);
			ត្រឡប់ -errno;
		}
		ប្រសិនបើ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ត្រឡប់ r;
		}
		free(buffer);
		ប្រសិនបើ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ត្រឡប់ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	ប្រសិនបើ (!service)
		ត្រឡប់ NULL;

	ប្រសិនបើ (service[0] == '/')
		ត្រឡប់ xstrdup(service);

	/* First check started services */
	ប្រសិនបើ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ប្រសិនបើ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ត្រឡប់ buffer;
	ប្រសិនបើ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ប្រសិនបើ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ត្រឡប់ buffer;

	សម្រាប់ (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ប្រសិនបើ (stat(file, &buf) == 0)
			ត្រឡប់ file;
		free(file);
	}

	ត្រឡប់ NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	ប្រសិនបើ (!service) {
		errno = EINVAL;
		ត្រឡប់ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ប្រសិនបើ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ត្រឡប់ false;
	}

	ប្រសិនបើ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ត្រឡប់ false;
	}

	ប្រសិនបើ (stat(file, &buf) == 0) {
		ប្រសិនបើ (buf.st_mode & S_IXUGO)
			retval = true;
		បើមិនដូច្នេះ
			errno = ENOEXEC;
	}
	free(file);
	ត្រឡប់ retval;
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

	ប្រសិនបើ (!(svc = rc_service_resolve(service)))
		ត្រឡប់ NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ប្រសិនបើ (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ត្រឡប់ NULL;
	}

	ប្រសិនបើ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ខណៈពេល ((token = strsep(&p, " ")))
			ប្រសិនបើ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ត្រឡប់ commands;
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

	ប្រសិនបើ (!(svc = rc_service_resolve(service)))
		ត្រឡប់ NULL;

	ប្រសិនបើ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ប្រសិនបើ ((fp = popen(cmd, "r"))) {
		ប្រសិនបើ (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ត្រឡប់ desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ត្រឡប់ r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	ចំនួនគត់ i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ចំនួនគត់ serrno;

	ប្រសិនបើ (!init)
		ត្រឡប់ false;

	base = basename_c(service);
	ប្រសិនបើ (state != RC_SERVICE_STOPPED) {
		ចំនួនគត់ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ប្រសិនបើ (access(init, F_OK) != 0) {
			free(init);
			ត្រឡប់ false;
		}

		unlinkat(state_dirfd, base, 0);
		ប្រសិនបើ ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ត្រឡប់ false;
		}
	}

	ប្រសិនបើ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ត្រឡប់ true;
	}

	/* Remove any old states now */
	សម្រាប់ (i = 0; rc_service_state_names[i].name; i++) {
		ចំនួនគត់ state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ប្រសិនបើ (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			ប្រសិនបើ (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ប្រសិនបើ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ប្រសិនបើ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ប្រសិនបើ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ត្រឡប់ false;
				}
				skip_wasinactive = true;
			}
			ប្រសិនបើ (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ត្រឡប់ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ប្រសិនបើ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ប្រសិនបើ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ប្រសិនបើ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ប្រសិនបើ (dp) {
			sa_nracanaa dirent *d;
			ខណៈពេល ((d = readdir(dp))) {
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
	ត្រឡប់ true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	ចំនួនគត់ state = RC_SERVICE_STOPPED;
	ចំនួនគត់ i;

	សម្រាប់ (i = 0; rc_service_state_names[i].name; i++) {
		ប្រសិនបើ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ប្រសិនបើ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			បើមិនដូច្នេះ
				state |= rc_service_state_names[i].state;
		}
	}

	ប្រសិនបើ (state & RC_SERVICE_STARTED)
		ប្រសិនបើ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ប្រសិនបើ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ប្រសិនបើ (dp) {
			sa_nracanaa dirent *d;
			ខណៈពេល ((d = readdir(dp))) {
				ប្រសិនបើ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ត្រឡប់ state;
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

	ត្រឡប់ buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	ចំនួនគត់ optdirfd;
	FILE *fp;

	ប្រសិនបើ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ត្រឡប់ false;

	ប្រសិនបើ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ត្រឡប់ false;

	ប្រសិនបើ (!value) {
		unlinkat(optdirfd, option, 0);
	} បើមិនដូច្នេះ ប្រសិនបើ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} បើមិនដូច្នេះ {
		ret = false;
	}

	close(optdirfd);
	ត្រឡប់ ret;
}

bool
rc_service_schedule_start(const var_na *service, const var_na *service_to_start)
{
	const var_na *base_to_start = basename_c(service_to_start);
	const var_na *base = basename_c(service);
	bool retval;
	ចំនួនគត់ schedfd;
	var_na *init;

	/* service may be a provided service, like net */
	ប្រសិនបើ (!service || !rc_service_exists(service_to_start))
		ត្រឡប់ false;

	ប្រសិនបើ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ត្រឡប់ false;

	ប្រសិនបើ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ត្រឡប់ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ត្រឡប់ retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ត្រឡប់ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ប្រសិនបើ (!runlevel) {
		const ចំនួនគត់ *scriptdirfds;
		list = rc_stringlist_new();
		សម្រាប់ (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ត្រឡប់ list;
	}

	/* These special levels never contain any services */
	ប្រសិនបើ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ត្រឡប់ rc_stringlist_new();

	ត្រឡប់ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ត្រឡប់ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	ប្រសិនបើ (state != RC_SERVICE_SCHEDULED)
		ត្រឡប់ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ប្រសិនបើ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ត្រឡប់ list;

	ខណៈពេល ((d = readdir(dp))) {
		ប្រសិនបើ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ត្រឡប់ list;
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

	ប្រសិនបើ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ត្រឡប់ false;
	}

	ប្រសិនបើ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ត្រឡប់ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ប្រសិនបើ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ប្រសិនបើ (path == NULL) {
			free(init);
			ត្រឡប់ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ប្រសិនបើ (!retval) {
			free(init);
			errno = EPERM;
			ត្រឡប់ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ប្រសិនបើ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ត្រឡប់ retval;
}

bool
rc_service_delete(const var_na *runlevel, const var_na *service)
{
	var_na *file;
	ចំនួនគត់ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	ត្រឡប់ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	ប្រសិនបើ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ត្រឡប់ list;

	ខណៈពេល ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ប្រសិនបើ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ត្រឡប់ list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ត្រឡប់ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	ចំនួនគត់ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ប្រសិនបើ (env[name_len] != '=')
		ត្រឡប់ errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (ចំនួនគត់) name_len, env);

	ប្រសិនបើ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ត្រឡប់ false;

	ប្រសិនបើ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ត្រឡប់ false;

	fputs(&env[name_len + 1], fp);

	ត្រឡប់ fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	ចំនួនគត់ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ប្រសិនបើ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ត្រឡប់ false;

	ប្រសិនបើ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ត្រឡប់ false;

	fputs(value, fp);

	ត្រឡប់ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const var_na *service, const var_na *name, var_na **buf)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)], *value;
	ចំនួនគត់ dfd = rc_dirfd(RC_DIR_ENVIRON);
	sa_nracanaa stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ប្រសិនបើ (fstatat(dfd, var, &stat, 0) == -1)
		ត្រឡប់ -1;

	ប្រសិនបើ (!buf)
		ត្រឡប់ stat.st_size;

	ប្រសិនបើ (!(value = xrealloc(*buf, stat.st_size + 1)))
		ត្រឡប់ -1;

	ប្រសិនបើ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ត្រឡប់ -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ត្រឡប់ count;
}
