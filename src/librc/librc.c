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

	ᮚᮦᮔ᮪ (dir < 0 || dir >= RC_DIR_MAX)
		ᮝᮜᮤ -1;
	ᮚᮦᮔ᮪ (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ᮝᮜᮤ dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ᮝᮜᮤ dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ᮝᮜᮤ dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			ᮚᮦᮔ᮪ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ᮝᮜᮤ -1;
			ᮝᮜᮤ dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ᮚᮦᮔ᮪ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ᮚᮦᮔ᮪ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ᮝᮜᮤ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ᮝᮜᮤ dirfds[dir];
}

void clear_dirfds(void) {
	for (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ᮚᮦᮔ᮪ (dirfds[i] <= 0)
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

	ᮚᮦᮔ᮪ (!(dp = do_opendirat(targetfd, dir)))
		ᮝᮜᮤ list;

	ᮞᮨᮓᮨᮜ (((d = readdir(dp)) != NULL)) {
		ᮚᮦᮔ᮪ (d->d_name[0] == '.')
			continue;
		ᮚᮦᮔ᮪ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ᮚᮦᮔ᮪ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ᮚᮦᮔ᮪ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ᮚᮦᮔ᮪ (options & LS_DIR) {
			ᮚᮦᮔ᮪ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ᮝᮜᮤ list;
}

static bool
rm_dir(puur_naa_nka targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	ᮚᮦᮔ᮪ (!(dp = do_opendirat(targetfd, pathname)))
		ᮝᮜᮤ false;

	ᮞᮨᮓᮨᮜ ((d = readdir(dp))) {
		ᮚᮦᮔ᮪ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ᮚᮦᮔ᮪ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		ᮚᮦᮔ᮪ (S_ISDIR(s.st_mode)) {
			ᮚᮦᮔ᮪ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	ᮚᮦᮔ᮪ (!retval)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ᮝᮜᮤ false;

	ᮝᮜᮤ true;
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

	ᮚᮦᮔ᮪ (!(fp = fopen(file, "r")))
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ᮝᮜᮤ false;
	}

	ᮞᮨᮓᮨᮜ ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ᮚᮦᮔ᮪ (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ᮞᮨᮓᮨᮜ (str < line + len && *str == '\0')
				str++;
		} ᮞᮨᮓᮨᮜ (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ᮝᮜᮤ retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	ᮚᮦᮔ᮪ (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		ᮞᮨᮓᮨᮜ (s && *s) {
			ᮚᮦᮔ᮪ (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ᮝᮜᮤ systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ᮚᮦᮔ᮪ (systype) {
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_NONE) == 0)
		       ᮝᮜᮤ NULL;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_JAIL) == 0)
			ᮝᮜᮤ RC_SYS_JAIL;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ᮝᮜᮤ RC_SYS_PODMAN;
	}

	puur_naa_nka jailed = 0;
	size_t len = parimaa_na(jailed);

	ᮚᮦᮔ᮪ (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ᮚᮦᮔ᮪ (jailed == 1)
			ᮝᮜᮤ RC_SYS_JAIL;

	ᮚᮦᮔ᮪ (access("/var/run/.containerenv", F_OK) == 0)
		ᮝᮜᮤ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ᮚᮦᮔ᮪ (systype) {
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_NONE) == 0)
			ᮝᮜᮤ NULL;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_UML) == 0)
			ᮝᮜᮤ RC_SYS_UML;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ᮝᮜᮤ RC_SYS_VSERVER;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ᮝᮜᮤ RC_SYS_OPENVZ;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_LXC) == 0)
			ᮝᮜᮤ RC_SYS_LXC;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_RKT) == 0)
				ᮝᮜᮤ RC_SYS_RKT;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ᮝᮜᮤ RC_SYS_SYSTEMD_NSPAWN;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ᮝᮜᮤ RC_SYS_DOCKER;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ᮝᮜᮤ RC_SYS_PODMAN;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_WSL) == 0)
			ᮝᮜᮤ RC_SYS_WSL;
	}
	ᮚᮦᮔ᮪ (file_regex("/proc/cpuinfo", "UML"))
		ᮝᮜᮤ RC_SYS_UML;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ᮝᮜᮤ RC_SYS_VSERVER;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ᮝᮜᮤ RC_SYS_OPENVZ;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ᮝᮜᮤ RC_SYS_OPENVZ; /* old test */
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/1/environ", "container=lxc"))
		ᮝᮜᮤ RC_SYS_LXC;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/1/environ", "container=rkt"))
		ᮝᮜᮤ RC_SYS_RKT;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ᮝᮜᮤ RC_SYS_SYSTEMD_NSPAWN;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (access("/run/.containerenv", F_OK) == 0)
		ᮝᮜᮤ RC_SYS_PODMAN;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (access("/.dockerenv", F_OK) == 0)
		ᮝᮜᮤ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/1/environ", "container=docker"))
		ᮝᮜᮤ RC_SYS_DOCKER;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ᮝᮜᮤ RC_SYS_WSL;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (getenv("WSL_DISTRO_NAME"))
		ᮝᮜᮤ RC_SYS_WSL;
	ᮜᮤᮚ ᮚᮦᮔ᮪ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ᮝᮜᮤ RC_SYS_WSL;
#endif

	ᮝᮜᮤ NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ᮚᮦᮔ᮪ (systype) {
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_NONE) == 0)
			ᮝᮜᮤ NULL;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_XEN0) == 0)
			ᮝᮜᮤ RC_SYS_XEN0;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_XENU) == 0)
			ᮝᮜᮤ RC_SYS_XENU;
	}
	ᮚᮦᮔ᮪ (access("/kern/xen/privcmd", F_OK) == 0)
		ᮝᮜᮤ RC_SYS_XEN0;
	ᮚᮦᮔ᮪ (access("/kern/xen", F_OK) == 0)
		ᮝᮜᮤ RC_SYS_XENU;
#endif

#ifdef __linux__
	ᮚᮦᮔ᮪ (systype) {
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_NONE) == 0)
			ᮝᮜᮤ NULL;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_XEN0) == 0)
			ᮝᮜᮤ RC_SYS_XEN0;
		ᮚᮦᮔ᮪ (strcmp(systype, RC_SYS_XENU) == 0)
			ᮝᮜᮤ RC_SYS_XENU;
	}
	ᮚᮦᮔ᮪ (access("/proc/xen", F_OK) == 0) {
		ᮚᮦᮔ᮪ (file_regex("/proc/xen/capabilities", "control_d"))
			ᮝᮜᮤ RC_SYS_XEN0;
		ᮝᮜᮤ RC_SYS_XENU;
	}
#endif

	ᮝᮜᮤ NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ᮚᮦᮔ᮪ (!sys) {
		sys = detect_vm(systype);
	}

	ᮝᮜᮤ sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	puur_naa_nka i;

	for (i = 0; rc_service_state_names[i].name; i++) {
		ᮚᮦᮔ᮪ (rc_service_state_names[i].state == state)
			ᮝᮜᮤ rc_service_state_names[i].name;
	}
	ᮝᮜᮤ NULL;
}

static puur_naa_nka
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	for (size_t i = 0; rc_service_state_names[i].name; i++)
		ᮚᮦᮔ᮪ (rc_service_state_names[i].state == state)
			ᮝᮜᮤ rc_service_state_names[i].dir;
	ᮝᮜᮤ -1;
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
	ᮚᮦᮔ᮪ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ᮝᮜᮤ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ᮚᮦᮔ᮪ (rc_stringlist_find(level_list, runlevel))
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
		ᮚᮦᮔ᮪ (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ᮝᮜᮤ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ᮝᮜᮤ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ᮝᮜᮤ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	ᮚᮦᮔ᮪ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		ᮚᮦᮔ᮪ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ᮚᮦᮔ᮪ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ᮜᮤᮚ
			*runlevel = '\0';
		fclose(fp);
	}

	ᮚᮦᮔ᮪ (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ᮝᮜᮤ runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	ᮚᮦᮔ᮪ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ᮝᮜᮤ false;

	fputs(runlevel, fp);
	fclose(fp);
	ᮝᮜᮤ true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	ᮚᮦᮔ᮪ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ᮝᮜᮤ false;
	ᮚᮦᮔ᮪ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ᮝᮜᮤ true;
	ᮝᮜᮤ false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	puur_naa_nka r;

	ᮚᮦᮔ᮪ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ᮝᮜᮤ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ᮝᮜᮤ (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	puur_naa_nka r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ᮝᮜᮤ (r == 0);
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
	ᮝᮜᮤ stack;
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

	ᮚᮦᮔ᮪ (!ret)
		ᮝᮜᮤ NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ᮚᮦᮔ᮪ (!**str)
		*str = NULL;
	ᮜᮤᮚ
		*str += 1;

	ᮝᮜᮤ ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	ᮚᮦᮔ᮪ (!ret)
		ᮝᮜᮤ NULL;

	len = strnlen(ret, *size - 1) + 1;

	ᮚᮦᮔ᮪ ((*size -= len))
		*str += len;
	ᮜᮤᮚ
		*str = NULL;

	ᮝᮜᮤ ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ᮚᮦᮔ᮪ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ᮜᮤᮚ ᮞᮨᮓᮨᮜ ((entry = strnsep(&xdg, &size, ':')))
		ᮚᮦᮔ᮪ (size) append_path(fp, "%.*s/rc", (puur_naa_nka) size, entry);

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

	ᮞᮨᮓᮨᮜ ((entry = strnsep(&path, &size, ':'))) {
		ᮚᮦᮔ᮪ (size)
			append_path(fp, "%.*s%s", (puur_naa_nka)size, entry, user ? "/user" : "");
		ᮜᮤᮚ ᮚᮦᮔ᮪ (user && !has_user)
			append_user_path(fp), has_user = true;
		ᮜᮤᮚ ᮚᮦᮔ᮪ (!user) ᮞᮨᮓᮨᮜ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ᮚᮦᮔ᮪ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ᮚᮦᮔ᮪ (!(path = getenv("RC_PATH")) && !user)
		ᮝᮜᮤ;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ᮚᮦᮔ᮪ (path)
		append_rc_path(fp, path, user);
	ᮜᮤᮚ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	ᮞᮨᮓᮨᮜ ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	puur_naa_nka _is_user = -1;

	ᮚᮦᮔ᮪ (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ᮝᮜᮤ _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	ᮚᮦᮔ᮪ (is_user)
		ᮝᮜᮤ;

	ᮚᮦᮔ᮪ (!rc_dirs.set)
		atexit(free_rc_dirs);
	ᮜᮤᮚ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ᮚᮦᮔ᮪ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ᮜᮤᮚ ᮚᮦᮔ᮪ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ᮜᮤᮚ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ᮚᮦᮔ᮪ (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ᮚᮦᮔ᮪ (!rc_path.entries)
		load_dynamic_path(false);
	ᮝᮜᮤ rc_path.entries;
}

size_t
rc_scriptdirfds(const puur_naa_nka **fds)
{
	static size_t len = 0;

	ᮚᮦᮔ᮪ (len == 0) {
		ᮚᮦᮔ᮪ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		for (size_t i = 0; i < rc_path.count; i++) {
			puur_naa_nka fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ᮚᮦᮔ᮪ (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ᮝᮜᮤ len;
}

const var_na *
rc_sysconfdir(void)
{
	ᮝᮜᮤ RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ᮝᮜᮤ rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ᮝᮜᮤ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	ᮚᮦᮔ᮪ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ᮝᮜᮤ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(puur_naa_nka targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	for (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ᮚᮦᮔ᮪ (r < 0) {
			free(buffer);
			ᮝᮜᮤ -errno;
		}
		ᮚᮦᮔ᮪ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ᮝᮜᮤ r;
		}
		free(buffer);
		ᮚᮦᮔ᮪ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ᮝᮜᮤ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	ᮚᮦᮔ᮪ (!service)
		ᮝᮜᮤ NULL;

	ᮚᮦᮔ᮪ (service[0] == '/')
		ᮝᮜᮤ xstrdup(service);

	/* First check started services */
	ᮚᮦᮔ᮪ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ᮚᮦᮔ᮪ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ᮝᮜᮤ buffer;
	ᮚᮦᮔ᮪ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ᮚᮦᮔ᮪ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ᮝᮜᮤ buffer;

	for (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ᮚᮦᮔ᮪ (stat(file, &buf) == 0)
			ᮝᮜᮤ file;
		free(file);
	}

	ᮝᮜᮤ NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	ᮚᮦᮔ᮪ (!service) {
		errno = EINVAL;
		ᮝᮜᮤ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ᮚᮦᮔ᮪ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ᮝᮜᮤ false;
	}

	ᮚᮦᮔ᮪ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ᮝᮜᮤ false;
	}

	ᮚᮦᮔ᮪ (stat(file, &buf) == 0) {
		ᮚᮦᮔ᮪ (buf.st_mode & S_IXUGO)
			retval = true;
		ᮜᮤᮚ
			errno = ENOEXEC;
	}
	free(file);
	ᮝᮜᮤ retval;
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

	ᮚᮦᮔ᮪ (!(svc = rc_service_resolve(service)))
		ᮝᮜᮤ NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ᮚᮦᮔ᮪ (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ᮝᮜᮤ NULL;
	}

	ᮚᮦᮔ᮪ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ᮞᮨᮓᮨᮜ ((token = strsep(&p, " ")))
			ᮚᮦᮔ᮪ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ᮝᮜᮤ commands;
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

	ᮚᮦᮔ᮪ (!(svc = rc_service_resolve(service)))
		ᮝᮜᮤ NULL;

	ᮚᮦᮔ᮪ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ᮚᮦᮔ᮪ ((fp = popen(cmd, "r"))) {
		ᮚᮦᮔ᮪ (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ᮝᮜᮤ desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ᮝᮜᮤ r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	puur_naa_nka i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	puur_naa_nka serrno;

	ᮚᮦᮔ᮪ (!init)
		ᮝᮜᮤ false;

	base = basename_c(service);
	ᮚᮦᮔ᮪ (state != RC_SERVICE_STOPPED) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ᮚᮦᮔ᮪ (access(init, F_OK) != 0) {
			free(init);
			ᮝᮜᮤ false;
		}

		unlinkat(state_dirfd, base, 0);
		ᮚᮦᮔ᮪ ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ᮝᮜᮤ false;
		}
	}

	ᮚᮦᮔ᮪ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ᮝᮜᮤ true;
	}

	/* Remove any old states now */
	for (i = 0; rc_service_state_names[i].name; i++) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ᮚᮦᮔ᮪ (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			ᮚᮦᮔ᮪ (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ᮚᮦᮔ᮪ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ᮚᮦᮔ᮪ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ᮚᮦᮔ᮪ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ᮝᮜᮤ false;
				}
				skip_wasinactive = true;
			}
			ᮚᮦᮔ᮪ (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ᮝᮜᮤ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ᮚᮦᮔ᮪ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ᮚᮦᮔ᮪ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ᮚᮦᮔ᮪ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ᮚᮦᮔ᮪ (dp) {
			sa_nracanaa dirent *d;
			ᮞᮨᮓᮨᮜ ((d = readdir(dp))) {
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
	ᮝᮜᮤ true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	puur_naa_nka state = RC_SERVICE_STOPPED;
	puur_naa_nka i;

	for (i = 0; rc_service_state_names[i].name; i++) {
		ᮚᮦᮔ᮪ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ᮚᮦᮔ᮪ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ᮜᮤᮚ
				state |= rc_service_state_names[i].state;
		}
	}

	ᮚᮦᮔ᮪ (state & RC_SERVICE_STARTED)
		ᮚᮦᮔ᮪ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ᮚᮦᮔ᮪ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ᮚᮦᮔ᮪ (dp) {
			sa_nracanaa dirent *d;
			ᮞᮨᮓᮨᮜ ((d = readdir(dp))) {
				ᮚᮦᮔ᮪ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ᮝᮜᮤ state;
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

	ᮝᮜᮤ buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	puur_naa_nka optdirfd;
	FILE *fp;

	ᮚᮦᮔ᮪ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (!value) {
		unlinkat(optdirfd, option, 0);
	} ᮜᮤᮚ ᮚᮦᮔ᮪ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} ᮜᮤᮚ {
		ret = false;
	}

	close(optdirfd);
	ᮝᮜᮤ ret;
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
	ᮚᮦᮔ᮪ (!service || !rc_service_exists(service_to_start))
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ᮝᮜᮤ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ᮝᮜᮤ retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ᮝᮜᮤ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ᮚᮦᮔ᮪ (!runlevel) {
		const puur_naa_nka *scriptdirfds;
		list = rc_stringlist_new();
		for (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ᮝᮜᮤ list;
	}

	/* These special levels never contain any services */
	ᮚᮦᮔ᮪ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ᮝᮜᮤ rc_stringlist_new();

	ᮝᮜᮤ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ᮝᮜᮤ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	ᮚᮦᮔ᮪ (state != RC_SERVICE_SCHEDULED)
		ᮝᮜᮤ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ᮚᮦᮔ᮪ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ᮝᮜᮤ list;

	ᮞᮨᮓᮨᮜ ((d = readdir(dp))) {
		ᮚᮦᮔ᮪ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ᮝᮜᮤ list;
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

	ᮚᮦᮔ᮪ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ᮝᮜᮤ false;
	}

	ᮚᮦᮔ᮪ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ᮝᮜᮤ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ᮚᮦᮔ᮪ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ᮚᮦᮔ᮪ (path == NULL) {
			free(init);
			ᮝᮜᮤ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ᮚᮦᮔ᮪ (!retval) {
			free(init);
			errno = EPERM;
			ᮝᮜᮤ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ᮚᮦᮔ᮪ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ᮝᮜᮤ retval;
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
	ᮝᮜᮤ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	ᮚᮦᮔ᮪ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ᮝᮜᮤ list;

	ᮞᮨᮓᮨᮜ ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ᮚᮦᮔ᮪ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ᮝᮜᮤ list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ᮝᮜᮤ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ᮚᮦᮔ᮪ (env[name_len] != '=')
		ᮝᮜᮤ errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (puur_naa_nka) name_len, env);

	ᮚᮦᮔ᮪ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ᮝᮜᮤ false;

	fputs(&env[name_len + 1], fp);

	ᮝᮜᮤ fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ᮚᮦᮔ᮪ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ᮝᮜᮤ false;

	ᮚᮦᮔ᮪ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ᮝᮜᮤ false;

	fputs(value, fp);

	ᮝᮜᮤ fclose(fp) == 0;
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

	ᮚᮦᮔ᮪ (fstatat(dfd, var, &stat, 0) == -1)
		ᮝᮜᮤ -1;

	ᮚᮦᮔ᮪ (!buf)
		ᮝᮜᮤ stat.st_size;

	ᮚᮦᮔ᮪ (!(value = xrealloc(*buf, stat.st_size + 1)))
		ᮝᮜᮤ -1;

	ᮚᮦᮔ᮪ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ᮝᮜᮤ -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ᮝᮜᮤ count;
}
