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
static puur_naa_nka dirfds[RC_DIR_MAX];

puur_naa_nka rc_dirfd(ga_nanaa rc_dir dir) {
	puur_naa_nka flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	puur_naa_nka dirfd;

	ꦪꦼꦤ꧀ (dir < 0 || dir >= RC_DIR_MAX)
		ꦮꦭꦶ -1;
	ꦪꦼꦤ꧀ (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ꦮꦭꦶ dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ꦮꦭꦶ dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ꦮꦭꦶ dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			ꦪꦼꦤ꧀ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ꦮꦭꦶ -1;
			ꦮꦭꦶ dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ꦪꦼꦤ꧀ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ꦪꦼꦤ꧀ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ꦮꦭꦶ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ꦮꦭꦶ dirfds[dir];
}

void clear_dirfds(void) {
	ꦏꦁꦒꦺꦴ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ꦪꦼꦤ꧀ (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(puur_naa_nka targetfd, const var_na *dir, puur_naa_nka options)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *list = NULL;
	sa_nracanaa stat buf;
	size_t l;

	list = rc_stringlist_new();

	ꦪꦼꦤ꧀ (!(dp = do_opendirat(targetfd, dir)))
		ꦮꦭꦶ list;

	ꦱꦼꦢꦼꦭ (((d = readdir(dp)) != NULL)) {
		ꦪꦼꦤ꧀ (d->d_name[0] == '.')
			continue;
		ꦪꦼꦤ꧀ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ꦪꦼꦤ꧀ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ꦪꦼꦤ꧀ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ꦪꦼꦤ꧀ (options & LS_DIR) {
			ꦪꦼꦤ꧀ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ꦮꦭꦶ list;
}

static bool
rm_dir(puur_naa_nka targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	ꦪꦼꦤ꧀ (!(dp = do_opendirat(targetfd, pathname)))
		ꦮꦭꦶ false;

	ꦱꦼꦢꦼꦭ ((d = readdir(dp))) {
		ꦪꦼꦤ꧀ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ꦪꦼꦤ꧀ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		ꦪꦼꦤ꧀ (S_ISDIR(s.st_mode)) {
			ꦪꦼꦤ꧀ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} ꦭꦶꦪ {
			ꦪꦼꦤ꧀ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	ꦪꦼꦤ꧀ (!retval)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ꦮꦭꦶ false;

	ꦮꦭꦶ true;
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
	puur_naa_nka result;

	ꦪꦼꦤ꧀ (!(fp = fopen(file, "r")))
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ꦮꦭꦶ false;
	}

	ꦱꦼꦢꦼꦭ ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ꦪꦼꦤ꧀ (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ꦱꦼꦢꦼꦭ (str < line + len && *str == '\0')
				str++;
		} ꦱꦼꦢꦼꦭ (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ꦮꦭꦶ retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	ꦪꦼꦤ꧀ (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		ꦱꦼꦢꦼꦭ (s && *s) {
			ꦪꦼꦤ꧀ (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ꦮꦭꦶ systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ꦪꦼꦤ꧀ (systype) {
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_NONE) == 0)
		       ꦮꦭꦶ NULL;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_JAIL) == 0)
			ꦮꦭꦶ RC_SYS_JAIL;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ꦮꦭꦶ RC_SYS_PODMAN;
	}

	puur_naa_nka jailed = 0;
	size_t len = parimaa_na(jailed);

	ꦪꦼꦤ꧀ (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ꦪꦼꦤ꧀ (jailed == 1)
			ꦮꦭꦶ RC_SYS_JAIL;

	ꦪꦼꦤ꧀ (access("/var/run/.containerenv", F_OK) == 0)
		ꦮꦭꦶ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ꦪꦼꦤ꧀ (systype) {
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_NONE) == 0)
			ꦮꦭꦶ NULL;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_UML) == 0)
			ꦮꦭꦶ RC_SYS_UML;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ꦮꦭꦶ RC_SYS_VSERVER;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ꦮꦭꦶ RC_SYS_OPENVZ;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_LXC) == 0)
			ꦮꦭꦶ RC_SYS_LXC;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_RKT) == 0)
				ꦮꦭꦶ RC_SYS_RKT;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ꦮꦭꦶ RC_SYS_SYSTEMD_NSPAWN;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ꦮꦭꦶ RC_SYS_DOCKER;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ꦮꦭꦶ RC_SYS_PODMAN;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_WSL) == 0)
			ꦮꦭꦶ RC_SYS_WSL;
	}
	ꦪꦼꦤ꧀ (file_regex("/proc/cpuinfo", "UML"))
		ꦮꦭꦶ RC_SYS_UML;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ꦮꦭꦶ RC_SYS_VSERVER;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ꦮꦭꦶ RC_SYS_OPENVZ;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ꦮꦭꦶ RC_SYS_OPENVZ; /* old test */
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/1/environ", "container=lxc"))
		ꦮꦭꦶ RC_SYS_LXC;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/1/environ", "container=rkt"))
		ꦮꦭꦶ RC_SYS_RKT;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ꦮꦭꦶ RC_SYS_SYSTEMD_NSPAWN;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (access("/run/.containerenv", F_OK) == 0)
		ꦮꦭꦶ RC_SYS_PODMAN;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (access("/.dockerenv", F_OK) == 0)
		ꦮꦭꦶ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/1/environ", "container=docker"))
		ꦮꦭꦶ RC_SYS_DOCKER;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ꦮꦭꦶ RC_SYS_WSL;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (getenv("WSL_DISTRO_NAME"))
		ꦮꦭꦶ RC_SYS_WSL;
	ꦭꦶꦪ ꦪꦼꦤ꧀ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ꦮꦭꦶ RC_SYS_WSL;
#endif

	ꦮꦭꦶ NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ꦪꦼꦤ꧀ (systype) {
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_NONE) == 0)
			ꦮꦭꦶ NULL;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_XEN0) == 0)
			ꦮꦭꦶ RC_SYS_XEN0;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_XENU) == 0)
			ꦮꦭꦶ RC_SYS_XENU;
	}
	ꦪꦼꦤ꧀ (access("/kern/xen/privcmd", F_OK) == 0)
		ꦮꦭꦶ RC_SYS_XEN0;
	ꦪꦼꦤ꧀ (access("/kern/xen", F_OK) == 0)
		ꦮꦭꦶ RC_SYS_XENU;
#endif

#ifdef __linux__
	ꦪꦼꦤ꧀ (systype) {
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_NONE) == 0)
			ꦮꦭꦶ NULL;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_XEN0) == 0)
			ꦮꦭꦶ RC_SYS_XEN0;
		ꦪꦼꦤ꧀ (strcmp(systype, RC_SYS_XENU) == 0)
			ꦮꦭꦶ RC_SYS_XENU;
	}
	ꦪꦼꦤ꧀ (access("/proc/xen", F_OK) == 0) {
		ꦪꦼꦤ꧀ (file_regex("/proc/xen/capabilities", "control_d"))
			ꦮꦭꦶ RC_SYS_XEN0;
		ꦮꦭꦶ RC_SYS_XENU;
	}
#endif

	ꦮꦭꦶ NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ꦪꦼꦤ꧀ (!sys) {
		sys = detect_vm(systype);
	}

	ꦮꦭꦶ sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	puur_naa_nka i;

	ꦏꦁꦒꦺꦴ (i = 0; rc_service_state_names[i].name; i++) {
		ꦪꦼꦤ꧀ (rc_service_state_names[i].state == state)
			ꦮꦭꦶ rc_service_state_names[i].name;
	}
	ꦮꦭꦶ NULL;
}

static puur_naa_nka
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	ꦏꦁꦒꦺꦴ (size_t i = 0; rc_service_state_names[i].name; i++)
		ꦪꦼꦤ꧀ (rc_service_state_names[i].state == state)
			ꦮꦭꦶ rc_service_state_names[i].dir;
	ꦮꦭꦶ -1;
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
	ꦪꦼꦤ꧀ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ꦮꦭꦶ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ꦪꦼꦤ꧀ (rc_stringlist_find(level_list, runlevel))
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
		ꦪꦼꦤ꧀ (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ꦮꦭꦶ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ꦮꦭꦶ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ꦮꦭꦶ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	ꦪꦼꦤ꧀ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		ꦪꦼꦤ꧀ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ꦪꦼꦤ꧀ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ꦭꦶꦪ
			*runlevel = '\0';
		fclose(fp);
	}

	ꦪꦼꦤ꧀ (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ꦮꦭꦶ runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	ꦪꦼꦤ꧀ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ꦮꦭꦶ false;

	fputs(runlevel, fp);
	fclose(fp);
	ꦮꦭꦶ true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	ꦪꦼꦤ꧀ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ꦮꦭꦶ false;
	ꦪꦼꦤ꧀ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ꦮꦭꦶ true;
	ꦮꦭꦶ false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	puur_naa_nka r;

	ꦪꦼꦤ꧀ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ꦮꦭꦶ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ꦮꦭꦶ (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	puur_naa_nka r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ꦮꦭꦶ (r == 0);
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
	ꦮꦭꦶ stack;
}

static const var_na *config_path[] = { _RC_PATH, NULL };

static sa_nracanaa {
	bool set;
	var_na *svcdir, *confdir, *runleveldir;
} rc_dirs;

static sa_nracanaa {
	var_na *buffer;
	size_t count;
	puur_naa_nka *fds;
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

	ꦪꦼꦤ꧀ (!ret)
		ꦮꦭꦶ NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ꦪꦼꦤ꧀ (!**str)
		*str = NULL;
	ꦭꦶꦪ
		*str += 1;

	ꦮꦭꦶ ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	ꦪꦼꦤ꧀ (!ret)
		ꦮꦭꦶ NULL;

	len = strnlen(ret, *size - 1) + 1;

	ꦪꦼꦤ꧀ ((*size -= len))
		*str += len;
	ꦭꦶꦪ
		*str = NULL;

	ꦮꦭꦶ ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ꦪꦼꦤ꧀ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ꦭꦶꦪ ꦱꦼꦢꦼꦭ ((entry = strnsep(&xdg, &size, ':')))
		ꦪꦼꦤ꧀ (size) append_path(fp, "%.*s/rc", (puur_naa_nka) size, entry);

	ꦏꦁꦒꦺꦴ (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	ꦱꦼꦢꦼꦭ ((entry = strnsep(&path, &size, ':'))) {
		ꦪꦼꦤ꧀ (size)
			append_path(fp, "%.*s%s", (puur_naa_nka)size, entry, user ? "/user" : "");
		ꦭꦶꦪ ꦪꦼꦤ꧀ (user && !has_user)
			append_user_path(fp), has_user = true;
		ꦭꦶꦪ ꦪꦼꦤ꧀ (!user) ꦱꦼꦢꦼꦭ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ꦪꦼꦤ꧀ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ꦪꦼꦤ꧀ (!(path = getenv("RC_PATH")) && !user)
		ꦮꦭꦶ;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ꦪꦼꦤ꧀ (path)
		append_rc_path(fp, path, user);
	ꦭꦶꦪ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	ꦱꦼꦢꦼꦭ ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	puur_naa_nka _is_user = -1;

	ꦪꦼꦤ꧀ (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ꦮꦭꦶ _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	ꦪꦼꦤ꧀ (is_user)
		ꦮꦭꦶ;

	ꦪꦼꦤ꧀ (!rc_dirs.set)
		atexit(free_rc_dirs);
	ꦭꦶꦪ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ꦪꦼꦤ꧀ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ꦭꦶꦪ ꦪꦼꦤ꧀ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ꦭꦶꦪ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ꦪꦼꦤ꧀ (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ꦪꦼꦤ꧀ (!rc_path.entries)
		load_dynamic_path(false);
	ꦮꦭꦶ rc_path.entries;
}

size_t
rc_scriptdirfds(const puur_naa_nka **fds)
{
	static size_t len = 0;

	ꦪꦼꦤ꧀ (len == 0) {
		ꦪꦼꦤ꧀ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		ꦏꦁꦒꦺꦴ (size_t i = 0; i < rc_path.count; i++) {
			puur_naa_nka fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ꦪꦼꦤ꧀ (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ꦮꦭꦶ len;
}

const var_na *
rc_sysconfdir(void)
{
	ꦮꦭꦶ RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ꦮꦭꦶ rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ꦮꦭꦶ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	ꦪꦼꦤ꧀ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ꦮꦭꦶ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(puur_naa_nka targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	ꦏꦁꦒꦺꦴ (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ꦪꦼꦤ꧀ (r < 0) {
			free(buffer);
			ꦮꦭꦶ -errno;
		}
		ꦪꦼꦤ꧀ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ꦮꦭꦶ r;
		}
		free(buffer);
		ꦪꦼꦤ꧀ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ꦮꦭꦶ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	ꦪꦼꦤ꧀ (!service)
		ꦮꦭꦶ NULL;

	ꦪꦼꦤ꧀ (service[0] == '/')
		ꦮꦭꦶ xstrdup(service);

	/* First check started services */
	ꦪꦼꦤ꧀ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ꦪꦼꦤ꧀ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ꦮꦭꦶ buffer;
	ꦪꦼꦤ꧀ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ꦪꦼꦤ꧀ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ꦮꦭꦶ buffer;

	ꦏꦁꦒꦺꦴ (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ꦪꦼꦤ꧀ (stat(file, &buf) == 0)
			ꦮꦭꦶ file;
		free(file);
	}

	ꦮꦭꦶ NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	ꦪꦼꦤ꧀ (!service) {
		errno = EINVAL;
		ꦮꦭꦶ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ꦪꦼꦤ꧀ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ꦮꦭꦶ false;
	}

	ꦪꦼꦤ꧀ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ꦮꦭꦶ false;
	}

	ꦪꦼꦤ꧀ (stat(file, &buf) == 0) {
		ꦪꦼꦤ꧀ (buf.st_mode & S_IXUGO)
			retval = true;
		ꦭꦶꦪ
			errno = ENOEXEC;
	}
	free(file);
	ꦮꦭꦶ retval;
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

	ꦪꦼꦤ꧀ (!(svc = rc_service_resolve(service)))
		ꦮꦭꦶ NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ꦪꦼꦤ꧀ (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ꦮꦭꦶ NULL;
	}

	ꦪꦼꦤ꧀ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ꦱꦼꦢꦼꦭ ((token = strsep(&p, " ")))
			ꦪꦼꦤ꧀ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ꦮꦭꦶ commands;
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

	ꦪꦼꦤ꧀ (!(svc = rc_service_resolve(service)))
		ꦮꦭꦶ NULL;

	ꦪꦼꦤ꧀ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ꦪꦼꦤ꧀ ((fp = popen(cmd, "r"))) {
		ꦪꦼꦤ꧀ (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ꦮꦭꦶ desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ꦮꦭꦶ r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	puur_naa_nka i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	puur_naa_nka serrno;

	ꦪꦼꦤ꧀ (!init)
		ꦮꦭꦶ false;

	base = basename_c(service);
	ꦪꦼꦤ꧀ (state != RC_SERVICE_STOPPED) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ꦪꦼꦤ꧀ (access(init, F_OK) != 0) {
			free(init);
			ꦮꦭꦶ false;
		}

		unlinkat(state_dirfd, base, 0);
		ꦪꦼꦤ꧀ ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ꦮꦭꦶ false;
		}
	}

	ꦪꦼꦤ꧀ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ꦮꦭꦶ true;
	}

	/* Remove any old states now */
	ꦏꦁꦒꦺꦴ (i = 0; rc_service_state_names[i].name; i++) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ꦪꦼꦤ꧀ (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			ꦪꦼꦤ꧀ (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ꦪꦼꦤ꧀ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ꦪꦼꦤ꧀ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ꦪꦼꦤ꧀ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ꦮꦭꦶ false;
				}
				skip_wasinactive = true;
			}
			ꦪꦼꦤ꧀ (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ꦮꦭꦶ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ꦪꦼꦤ꧀ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ꦪꦼꦤ꧀ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ꦪꦼꦤ꧀ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ꦪꦼꦤ꧀ (dp) {
			sa_nracanaa dirent *d;
			ꦱꦼꦢꦼꦭ ((d = readdir(dp))) {
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
	ꦮꦭꦶ true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	puur_naa_nka state = RC_SERVICE_STOPPED;
	puur_naa_nka i;

	ꦏꦁꦒꦺꦴ (i = 0; rc_service_state_names[i].name; i++) {
		ꦪꦼꦤ꧀ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ꦪꦼꦤ꧀ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ꦭꦶꦪ
				state |= rc_service_state_names[i].state;
		}
	}

	ꦪꦼꦤ꧀ (state & RC_SERVICE_STARTED)
		ꦪꦼꦤ꧀ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ꦪꦼꦤ꧀ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ꦪꦼꦤ꧀ (dp) {
			sa_nracanaa dirent *d;
			ꦱꦼꦢꦼꦭ ((d = readdir(dp))) {
				ꦪꦼꦤ꧀ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ꦮꦭꦶ state;
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

	ꦮꦭꦶ buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	puur_naa_nka optdirfd;
	FILE *fp;

	ꦪꦼꦤ꧀ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (!value) {
		unlinkat(optdirfd, option, 0);
	} ꦭꦶꦪ ꦪꦼꦤ꧀ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} ꦭꦶꦪ {
		ret = false;
	}

	close(optdirfd);
	ꦮꦭꦶ ret;
}

bool
rc_service_schedule_start(const var_na *service, const var_na *service_to_start)
{
	const var_na *base_to_start = basename_c(service_to_start);
	const var_na *base = basename_c(service);
	bool retval;
	puur_naa_nka schedfd;
	var_na *init;

	/* service may be a provided service, like net */
	ꦪꦼꦤ꧀ (!service || !rc_service_exists(service_to_start))
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ꦮꦭꦶ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ꦮꦭꦶ retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ꦮꦭꦶ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ꦪꦼꦤ꧀ (!runlevel) {
		const puur_naa_nka *scriptdirfds;
		list = rc_stringlist_new();
		ꦏꦁꦒꦺꦴ (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ꦮꦭꦶ list;
	}

	/* These special levels never contain any services */
	ꦪꦼꦤ꧀ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ꦮꦭꦶ rc_stringlist_new();

	ꦮꦭꦶ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ꦮꦭꦶ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	ꦪꦼꦤ꧀ (state != RC_SERVICE_SCHEDULED)
		ꦮꦭꦶ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ꦪꦼꦤ꧀ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ꦮꦭꦶ list;

	ꦱꦼꦢꦼꦭ ((d = readdir(dp))) {
		ꦪꦼꦤ꧀ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ꦮꦭꦶ list;
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

	ꦪꦼꦤ꧀ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ꦮꦭꦶ false;
	}

	ꦪꦼꦤ꧀ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ꦮꦭꦶ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ꦪꦼꦤ꧀ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ꦪꦼꦤ꧀ (path == NULL) {
			free(init);
			ꦮꦭꦶ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ꦪꦼꦤ꧀ (!retval) {
			free(init);
			errno = EPERM;
			ꦮꦭꦶ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ꦪꦼꦤ꧀ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ꦮꦭꦶ retval;
}

bool
rc_service_delete(const var_na *runlevel, const var_na *service)
{
	var_na *file;
	puur_naa_nka r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	ꦮꦭꦶ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	ꦪꦼꦤ꧀ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ꦮꦭꦶ list;

	ꦱꦼꦢꦼꦭ ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ꦪꦼꦤ꧀ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ꦮꦭꦶ list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ꦮꦭꦶ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ꦪꦼꦤ꧀ (env[name_len] != '=')
		ꦮꦭꦶ errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (puur_naa_nka) name_len, env);

	ꦪꦼꦤ꧀ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ꦮꦭꦶ false;

	fputs(&env[name_len + 1], fp);

	ꦮꦭꦶ fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ꦪꦼꦤ꧀ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ꦮꦭꦶ false;

	ꦪꦼꦤ꧀ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ꦮꦭꦶ false;

	fputs(value, fp);

	ꦮꦭꦶ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const var_na *service, const var_na *name, var_na **buf)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)], *value;
	puur_naa_nka dfd = rc_dirfd(RC_DIR_ENVIRON);
	sa_nracanaa stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ꦪꦼꦤ꧀ (fstatat(dfd, var, &stat, 0) == -1)
		ꦮꦭꦶ -1;

	ꦪꦼꦤ꧀ (!buf)
		ꦮꦭꦶ stat.st_size;

	ꦪꦼꦤ꧀ (!(value = xrealloc(*buf, stat.st_size + 1)))
		ꦮꦭꦶ -1;

	ꦪꦼꦤ꧀ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ꦮꦭꦶ -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ꦮꦭꦶ count;
}
