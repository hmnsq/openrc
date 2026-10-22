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

	ᱡᱩᱫᱤ (dir < 0 || dir >= RC_DIR_MAX)
		ᱨᱩᱣᱟᱹᱲ -1;
	ᱡᱩᱫᱤ (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ᱨᱩᱣᱟᱹᱲ dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ᱨᱩᱣᱟᱹᱲ dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ᱨᱩᱣᱟᱹᱲ dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			ᱡᱩᱫᱤ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ᱨᱩᱣᱟᱹᱲ -1;
			ᱨᱩᱣᱟᱹᱲ dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ᱡᱩᱫᱤ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ᱡᱩᱫᱤ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ᱨᱩᱣᱟᱹᱲ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ᱨᱩᱣᱟᱹᱲ dirfds[dir];
}

void clear_dirfds(void) {
	for (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ᱡᱩᱫᱤ (dirfds[i] <= 0)
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

	ᱡᱩᱫᱤ (!(dp = do_opendirat(targetfd, dir)))
		ᱨᱩᱣᱟᱹᱲ list;

	ᱡᱚᱠᱷᱚᱱ (((d = readdir(dp)) != NULL)) {
		ᱡᱩᱫᱤ (d->d_name[0] == '.')
			continue;
		ᱡᱩᱫᱤ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ᱡᱩᱫᱤ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ᱡᱩᱫᱤ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ᱡᱩᱫᱤ (options & LS_DIR) {
			ᱡᱩᱫᱤ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ᱨᱩᱣᱟᱹᱲ list;
}

static bool
rm_dir(puur_naa_nka targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	ᱡᱩᱫᱤ (!(dp = do_opendirat(targetfd, pathname)))
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱚᱠᱷᱚᱱ ((d = readdir(dp))) {
		ᱡᱩᱫᱤ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ᱡᱩᱫᱤ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		ᱡᱩᱫᱤ (S_ISDIR(s.st_mode)) {
			ᱡᱩᱫᱤ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} ᱵᱟᱝ {
			ᱡᱩᱫᱤ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	ᱡᱩᱫᱤ (!retval)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱨᱩᱣᱟᱹᱲ true;
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

	ᱡᱩᱫᱤ (!(fp = fopen(file, "r")))
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ᱨᱩᱣᱟᱹᱲ false;
	}

	ᱡᱚᱠᱷᱚᱱ ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ᱡᱩᱫᱤ (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ᱡᱚᱠᱷᱚᱱ (str < line + len && *str == '\0')
				str++;
		} ᱡᱚᱠᱷᱚᱱ (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ᱨᱩᱣᱟᱹᱲ retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	ᱡᱩᱫᱤ (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		ᱡᱚᱠᱷᱚᱱ (s && *s) {
			ᱡᱩᱫᱤ (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ᱨᱩᱣᱟᱹᱲ systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ᱡᱩᱫᱤ (systype) {
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_NONE) == 0)
		       ᱨᱩᱣᱟᱹᱲ NULL;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_JAIL) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_JAIL;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_PODMAN;
	}

	puur_naa_nka jailed = 0;
	size_t len = parimaa_na(jailed);

	ᱡᱩᱫᱤ (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ᱡᱩᱫᱤ (jailed == 1)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_JAIL;

	ᱡᱩᱫᱤ (access("/var/run/.containerenv", F_OK) == 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ᱡᱩᱫᱤ (systype) {
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_NONE) == 0)
			ᱨᱩᱣᱟᱹᱲ NULL;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_UML) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_UML;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_VSERVER;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_OPENVZ;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_LXC) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_LXC;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_RKT) == 0)
				ᱨᱩᱣᱟᱹᱲ RC_SYS_RKT;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ᱨᱩᱣᱟᱹᱲ RC_SYS_SYSTEMD_NSPAWN;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ᱨᱩᱣᱟᱹᱲ RC_SYS_DOCKER;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_PODMAN;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_WSL) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_WSL;
	}
	ᱡᱩᱫᱤ (file_regex("/proc/cpuinfo", "UML"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_UML;
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_VSERVER;
	ᱵᱟᱝ ᱡᱩᱫᱤ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_OPENVZ;
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_OPENVZ; /* old test */
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/1/environ", "container=lxc"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_LXC;
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/1/environ", "container=rkt"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_RKT;
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_SYSTEMD_NSPAWN;
	ᱵᱟᱝ ᱡᱩᱫᱤ (access("/run/.containerenv", F_OK) == 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_PODMAN;
	ᱵᱟᱝ ᱡᱩᱫᱤ (access("/.dockerenv", F_OK) == 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/1/environ", "container=docker"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_DOCKER;
	ᱵᱟᱝ ᱡᱩᱫᱤ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_WSL;
	ᱵᱟᱝ ᱡᱩᱫᱤ (getenv("WSL_DISTRO_NAME"))
		ᱨᱩᱣᱟᱹᱲ RC_SYS_WSL;
	ᱵᱟᱝ ᱡᱩᱫᱤ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_WSL;
#endif

	ᱨᱩᱣᱟᱹᱲ NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ᱡᱩᱫᱤ (systype) {
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_NONE) == 0)
			ᱨᱩᱣᱟᱹᱲ NULL;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_XEN0) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_XEN0;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_XENU) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_XENU;
	}
	ᱡᱩᱫᱤ (access("/kern/xen/privcmd", F_OK) == 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_XEN0;
	ᱡᱩᱫᱤ (access("/kern/xen", F_OK) == 0)
		ᱨᱩᱣᱟᱹᱲ RC_SYS_XENU;
#endif

#ifdef __linux__
	ᱡᱩᱫᱤ (systype) {
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_NONE) == 0)
			ᱨᱩᱣᱟᱹᱲ NULL;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_XEN0) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_XEN0;
		ᱡᱩᱫᱤ (strcmp(systype, RC_SYS_XENU) == 0)
			ᱨᱩᱣᱟᱹᱲ RC_SYS_XENU;
	}
	ᱡᱩᱫᱤ (access("/proc/xen", F_OK) == 0) {
		ᱡᱩᱫᱤ (file_regex("/proc/xen/capabilities", "control_d"))
			ᱨᱩᱣᱟᱹᱲ RC_SYS_XEN0;
		ᱨᱩᱣᱟᱹᱲ RC_SYS_XENU;
	}
#endif

	ᱨᱩᱣᱟᱹᱲ NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ᱡᱩᱫᱤ (!sys) {
		sys = detect_vm(systype);
	}

	ᱨᱩᱣᱟᱹᱲ sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	puur_naa_nka i;

	for (i = 0; rc_service_state_names[i].name; i++) {
		ᱡᱩᱫᱤ (rc_service_state_names[i].state == state)
			ᱨᱩᱣᱟᱹᱲ rc_service_state_names[i].name;
	}
	ᱨᱩᱣᱟᱹᱲ NULL;
}

static puur_naa_nka
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	for (size_t i = 0; rc_service_state_names[i].name; i++)
		ᱡᱩᱫᱤ (rc_service_state_names[i].state == state)
			ᱨᱩᱣᱟᱹᱲ rc_service_state_names[i].dir;
	ᱨᱩᱣᱟᱹᱲ -1;
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
	ᱡᱩᱫᱤ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ᱨᱩᱣᱟᱹᱲ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ᱡᱩᱫᱤ (rc_stringlist_find(level_list, runlevel))
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
		ᱡᱩᱫᱤ (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ᱨᱩᱣᱟᱹᱲ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ᱨᱩᱣᱟᱹᱲ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ᱨᱩᱣᱟᱹᱲ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	ᱡᱩᱫᱤ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		ᱡᱩᱫᱤ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ᱡᱩᱫᱤ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ᱵᱟᱝ
			*runlevel = '\0';
		fclose(fp);
	}

	ᱡᱩᱫᱤ (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ᱨᱩᱣᱟᱹᱲ runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	ᱡᱩᱫᱤ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ᱨᱩᱣᱟᱹᱲ false;

	fputs(runlevel, fp);
	fclose(fp);
	ᱨᱩᱣᱟᱹᱲ true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	ᱡᱩᱫᱤ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ᱨᱩᱣᱟᱹᱲ false;
	ᱡᱩᱫᱤ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ᱨᱩᱣᱟᱹᱲ true;
	ᱨᱩᱣᱟᱹᱲ false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	puur_naa_nka r;

	ᱡᱩᱫᱤ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ᱨᱩᱣᱟᱹᱲ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ᱨᱩᱣᱟᱹᱲ (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	puur_naa_nka r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ᱨᱩᱣᱟᱹᱲ (r == 0);
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
	ᱨᱩᱣᱟᱹᱲ stack;
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

	ᱡᱩᱫᱤ (!ret)
		ᱨᱩᱣᱟᱹᱲ NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ᱡᱩᱫᱤ (!**str)
		*str = NULL;
	ᱵᱟᱝ
		*str += 1;

	ᱨᱩᱣᱟᱹᱲ ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	ᱡᱩᱫᱤ (!ret)
		ᱨᱩᱣᱟᱹᱲ NULL;

	len = strnlen(ret, *size - 1) + 1;

	ᱡᱩᱫᱤ ((*size -= len))
		*str += len;
	ᱵᱟᱝ
		*str = NULL;

	ᱨᱩᱣᱟᱹᱲ ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ᱡᱩᱫᱤ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ᱵᱟᱝ ᱡᱚᱠᱷᱚᱱ ((entry = strnsep(&xdg, &size, ':')))
		ᱡᱩᱫᱤ (size) append_path(fp, "%.*s/rc", (puur_naa_nka) size, entry);

	for (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	ᱡᱚᱠᱷᱚᱱ ((entry = strnsep(&path, &size, ':'))) {
		ᱡᱩᱫᱤ (size)
			append_path(fp, "%.*s%s", (puur_naa_nka)size, entry, user ? "/user" : "");
		ᱵᱟᱝ ᱡᱩᱫᱤ (user && !has_user)
			append_user_path(fp), has_user = true;
		ᱵᱟᱝ ᱡᱩᱫᱤ (!user) ᱡᱚᱠᱷᱚᱱ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ᱡᱩᱫᱤ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ᱡᱩᱫᱤ (!(path = getenv("RC_PATH")) && !user)
		ᱨᱩᱣᱟᱹᱲ;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ᱡᱩᱫᱤ (path)
		append_rc_path(fp, path, user);
	ᱵᱟᱝ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	ᱡᱚᱠᱷᱚᱱ ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	puur_naa_nka _is_user = -1;

	ᱡᱩᱫᱤ (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ᱨᱩᱣᱟᱹᱲ _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	ᱡᱩᱫᱤ (is_user)
		ᱨᱩᱣᱟᱹᱲ;

	ᱡᱩᱫᱤ (!rc_dirs.set)
		atexit(free_rc_dirs);
	ᱵᱟᱝ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ᱡᱩᱫᱤ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ᱵᱟᱝ ᱡᱩᱫᱤ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ᱵᱟᱝ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ᱡᱩᱫᱤ (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ᱡᱩᱫᱤ (!rc_path.entries)
		load_dynamic_path(false);
	ᱨᱩᱣᱟᱹᱲ rc_path.entries;
}

size_t
rc_scriptdirfds(const puur_naa_nka **fds)
{
	static size_t len = 0;

	ᱡᱩᱫᱤ (len == 0) {
		ᱡᱩᱫᱤ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		for (size_t i = 0; i < rc_path.count; i++) {
			puur_naa_nka fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ᱡᱩᱫᱤ (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ᱨᱩᱣᱟᱹᱲ len;
}

const var_na *
rc_sysconfdir(void)
{
	ᱨᱩᱣᱟᱹᱲ RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ᱨᱩᱣᱟᱹᱲ rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ᱨᱩᱣᱟᱹᱲ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	ᱡᱩᱫᱤ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ᱨᱩᱣᱟᱹᱲ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(puur_naa_nka targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	for (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ᱡᱩᱫᱤ (r < 0) {
			free(buffer);
			ᱨᱩᱣᱟᱹᱲ -errno;
		}
		ᱡᱩᱫᱤ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ᱨᱩᱣᱟᱹᱲ r;
		}
		free(buffer);
		ᱡᱩᱫᱤ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ᱨᱩᱣᱟᱹᱲ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	ᱡᱩᱫᱤ (!service)
		ᱨᱩᱣᱟᱹᱲ NULL;

	ᱡᱩᱫᱤ (service[0] == '/')
		ᱨᱩᱣᱟᱹᱲ xstrdup(service);

	/* First check started services */
	ᱡᱩᱫᱤ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ᱡᱩᱫᱤ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ᱨᱩᱣᱟᱹᱲ buffer;
	ᱡᱩᱫᱤ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ᱡᱩᱫᱤ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ᱨᱩᱣᱟᱹᱲ buffer;

	for (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ᱡᱩᱫᱤ (stat(file, &buf) == 0)
			ᱨᱩᱣᱟᱹᱲ file;
		free(file);
	}

	ᱨᱩᱣᱟᱹᱲ NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	ᱡᱩᱫᱤ (!service) {
		errno = EINVAL;
		ᱨᱩᱣᱟᱹᱲ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ᱡᱩᱫᱤ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ᱨᱩᱣᱟᱹᱲ false;
	}

	ᱡᱩᱫᱤ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ᱨᱩᱣᱟᱹᱲ false;
	}

	ᱡᱩᱫᱤ (stat(file, &buf) == 0) {
		ᱡᱩᱫᱤ (buf.st_mode & S_IXUGO)
			retval = true;
		ᱵᱟᱝ
			errno = ENOEXEC;
	}
	free(file);
	ᱨᱩᱣᱟᱹᱲ retval;
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

	ᱡᱩᱫᱤ (!(svc = rc_service_resolve(service)))
		ᱨᱩᱣᱟᱹᱲ NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ᱡᱩᱫᱤ (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ᱨᱩᱣᱟᱹᱲ NULL;
	}

	ᱡᱩᱫᱤ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ᱡᱚᱠᱷᱚᱱ ((token = strsep(&p, " ")))
			ᱡᱩᱫᱤ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ᱨᱩᱣᱟᱹᱲ commands;
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

	ᱡᱩᱫᱤ (!(svc = rc_service_resolve(service)))
		ᱨᱩᱣᱟᱹᱲ NULL;

	ᱡᱩᱫᱤ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ᱡᱩᱫᱤ ((fp = popen(cmd, "r"))) {
		ᱡᱩᱫᱤ (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ᱨᱩᱣᱟᱹᱲ desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ᱨᱩᱣᱟᱹᱲ r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	puur_naa_nka i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	puur_naa_nka serrno;

	ᱡᱩᱫᱤ (!init)
		ᱨᱩᱣᱟᱹᱲ false;

	base = basename_c(service);
	ᱡᱩᱫᱤ (state != RC_SERVICE_STOPPED) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ᱡᱩᱫᱤ (access(init, F_OK) != 0) {
			free(init);
			ᱨᱩᱣᱟᱹᱲ false;
		}

		unlinkat(state_dirfd, base, 0);
		ᱡᱩᱫᱤ ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ᱨᱩᱣᱟᱹᱲ false;
		}
	}

	ᱡᱩᱫᱤ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ᱨᱩᱣᱟᱹᱲ true;
	}

	/* Remove any old states now */
	for (i = 0; rc_service_state_names[i].name; i++) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ᱡᱩᱫᱤ (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			ᱡᱩᱫᱤ (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ᱡᱩᱫᱤ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ᱡᱩᱫᱤ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ᱡᱩᱫᱤ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ᱨᱩᱣᱟᱹᱲ false;
				}
				skip_wasinactive = true;
			}
			ᱡᱩᱫᱤ (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ᱨᱩᱣᱟᱹᱲ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ᱡᱩᱫᱤ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ᱡᱩᱫᱤ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ᱡᱩᱫᱤ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ᱡᱩᱫᱤ (dp) {
			sa_nracanaa dirent *d;
			ᱡᱚᱠᱷᱚᱱ ((d = readdir(dp))) {
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
	ᱨᱩᱣᱟᱹᱲ true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	puur_naa_nka state = RC_SERVICE_STOPPED;
	puur_naa_nka i;

	for (i = 0; rc_service_state_names[i].name; i++) {
		ᱡᱩᱫᱤ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ᱡᱩᱫᱤ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ᱵᱟᱝ
				state |= rc_service_state_names[i].state;
		}
	}

	ᱡᱩᱫᱤ (state & RC_SERVICE_STARTED)
		ᱡᱩᱫᱤ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ᱡᱩᱫᱤ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ᱡᱩᱫᱤ (dp) {
			sa_nracanaa dirent *d;
			ᱡᱚᱠᱷᱚᱱ ((d = readdir(dp))) {
				ᱡᱩᱫᱤ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ᱨᱩᱣᱟᱹᱲ state;
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

	ᱨᱩᱣᱟᱹᱲ buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	puur_naa_nka optdirfd;
	FILE *fp;

	ᱡᱩᱫᱤ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ (!value) {
		unlinkat(optdirfd, option, 0);
	} ᱵᱟᱝ ᱡᱩᱫᱤ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} ᱵᱟᱝ {
		ret = false;
	}

	close(optdirfd);
	ᱨᱩᱣᱟᱹᱲ ret;
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
	ᱡᱩᱫᱤ (!service || !rc_service_exists(service_to_start))
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ᱨᱩᱣᱟᱹᱲ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ᱨᱩᱣᱟᱹᱲ retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ᱨᱩᱣᱟᱹᱲ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ᱡᱩᱫᱤ (!runlevel) {
		const puur_naa_nka *scriptdirfds;
		list = rc_stringlist_new();
		for (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ᱨᱩᱣᱟᱹᱲ list;
	}

	/* These special levels never contain any services */
	ᱡᱩᱫᱤ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ᱨᱩᱣᱟᱹᱲ rc_stringlist_new();

	ᱨᱩᱣᱟᱹᱲ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ᱨᱩᱣᱟᱹᱲ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	ᱡᱩᱫᱤ (state != RC_SERVICE_SCHEDULED)
		ᱨᱩᱣᱟᱹᱲ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ᱡᱩᱫᱤ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ᱨᱩᱣᱟᱹᱲ list;

	ᱡᱚᱠᱷᱚᱱ ((d = readdir(dp))) {
		ᱡᱩᱫᱤ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ᱨᱩᱣᱟᱹᱲ list;
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

	ᱡᱩᱫᱤ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ᱨᱩᱣᱟᱹᱲ false;
	}

	ᱡᱩᱫᱤ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ᱨᱩᱣᱟᱹᱲ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ᱡᱩᱫᱤ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ᱡᱩᱫᱤ (path == NULL) {
			free(init);
			ᱨᱩᱣᱟᱹᱲ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ᱡᱩᱫᱤ (!retval) {
			free(init);
			errno = EPERM;
			ᱨᱩᱣᱟᱹᱲ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ᱡᱩᱫᱤ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ᱨᱩᱣᱟᱹᱲ retval;
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
	ᱨᱩᱣᱟᱹᱲ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	ᱡᱩᱫᱤ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ᱨᱩᱣᱟᱹᱲ list;

	ᱡᱚᱠᱷᱚᱱ ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ᱡᱩᱫᱤ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ᱨᱩᱣᱟᱹᱲ list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ᱨᱩᱣᱟᱹᱲ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ᱡᱩᱫᱤ (env[name_len] != '=')
		ᱨᱩᱣᱟᱹᱲ errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (puur_naa_nka) name_len, env);

	ᱡᱩᱫᱤ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ᱨᱩᱣᱟᱹᱲ false;

	fputs(&env[name_len + 1], fp);

	ᱨᱩᱣᱟᱹᱲ fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ᱡᱩᱫᱤ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ᱨᱩᱣᱟᱹᱲ false;

	ᱡᱩᱫᱤ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ᱨᱩᱣᱟᱹᱲ false;

	fputs(value, fp);

	ᱨᱩᱣᱟᱹᱲ fclose(fp) == 0;
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

	ᱡᱩᱫᱤ (fstatat(dfd, var, &stat, 0) == -1)
		ᱨᱩᱣᱟᱹᱲ -1;

	ᱡᱩᱫᱤ (!buf)
		ᱨᱩᱣᱟᱹᱲ stat.st_size;

	ᱡᱩᱫᱤ (!(value = xrealloc(*buf, stat.st_size + 1)))
		ᱨᱩᱣᱟᱹᱲ -1;

	ᱡᱩᱫᱤ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ᱨᱩᱣᱟᱹᱲ -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ᱨᱩᱣᱟᱹᱲ count;
}
