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

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <helpers.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <libgen.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <regex.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/param.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ರಿಕ್ತ;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dirfds[RC_DIR_MAX];

ಪೂರ್ಣಾಙ್ಕ rc_dirfd(ಗಣನಾ rc_dir dir) {
	ಪೂರ್ಣಾಙ್ಕ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ಪೂರ್ಣಾಙ್ಕ dirfd;

	ಯದಿ (dir < 0 || dir >= RC_DIR_MAX)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (dirfds[dir] <= 0) {
		ವಿಕಲ್ಪ (dir) {
		ಪಕ್ಷ RC_DIR_SYSCONF:
			ಪ್ರತ್ಯಾಗಮ dirfds[dir] = open(rc_sysconfdir(), flags);
		ಪಕ್ಷ RC_DIR_USRCONF:
			ಪ್ರತ್ಯಾಗಮ dirfds[dir] = open(rc_usrconfdir(), flags);
		ಪಕ್ಷ RC_DIR_RUNLEVEL:
			ಪ್ರತ್ಯಾಗಮ dirfds[dir] = open(rc_runleveldir(), flags);
		ಪಕ್ಷ RC_DIR_SVCDIR:
			ಯದಿ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ಪ್ರತ್ಯಾಗಮ -1;
			ಪ್ರತ್ಯಾಗಮ dirfds[dir] = open(rc_svcdir(), flags);
		ಸ್ವಭಾವ:
			ಪ್ರತಿಜ್ಞಾ(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ಯದಿ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ಯದಿ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ಪ್ರತ್ಯಾಗಮ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			ಭಙ್ಗ;
		}
	}
	ಪ್ರತ್ಯಾಗಮ dirfds[dir];
}

ಶೂನ್ಯ clear_dirfds(ಶೂನ್ಯ) {
	ಪ್ರತಿ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ಯದಿ (dirfds[i] <= 0)
			ಅನುವರ್ತ;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
ಸ್ಥಿರ RC_STRINGLIST *
ls_dir(ಪೂರ್ಣಾಙ್ಕ targetfd, ಧ್ರುವ ವರ್ಣ *dir, ಪೂರ್ಣಾಙ್ಕ options)
{
	DIR *dp;
	ಸಂರಚನಾ dirent *d;
	RC_STRINGLIST *list = ರಿಕ್ತ;
	ಸಂರಚನಾ stat buf;
	size_t l;

	list = rc_stringlist_new();

	ಯದಿ (!(dp = do_opendirat(targetfd, dir)))
		ಪ್ರತ್ಯಾಗಮ list;

	ಯಾವತ್ (((d = readdir(dp)) != ರಿಕ್ತ)) {
		ಯದಿ (d->d_name[0] == '.')
			ಅನುವರ್ತ;
		ಯದಿ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ಯದಿ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				ಅನುವರ್ತ;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ಯದಿ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				ಅನುವರ್ತ;
		}
		ಯದಿ (options & LS_DIR) {
			ಯದಿ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				ಅನುವರ್ತ;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ಪ್ರತ್ಯಾಗಮ list;
}

ಸ್ಥಿರ bool
rm_dir(ಪೂರ್ಣಾಙ್ಕ targetfd, ಧ್ರುವ ವರ್ಣ *pathname, bool ತುದಿ)
{
	bool retval = true;
	ಸಂರಚನಾ dirent *d;
	ಸಂರಚನಾ stat s;
	DIR *dp;

	ಯದಿ (!(dp = do_opendirat(targetfd, pathname)))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯಾವತ್ ((d = readdir(dp))) {
		ಯದಿ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ಅನುವರ್ತ;
		ಯದಿ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			ಭಙ್ಗ;
		}
		ಯದಿ (S_ISDIR(s.st_mode)) {
			ಯದಿ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				ಭಙ್ಗ;
			}
		} ಅನ್ಯಥಾ {
			ಯದಿ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				ಭಙ್ಗ;
			}
		}
	}
	closedir(dp);

	ಯದಿ (!retval)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (ತುದಿ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ಪ್ರತ್ಯಾಗಮ false;

	ಪ್ರತ್ಯಾಗಮ true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
ಸ್ಥಿರ bool
file_regex(ಧ್ರುವ ವರ್ಣ *file, ಧ್ರುವ ವರ್ಣ *regex)
{
	FILE *fp;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	ಪೂರ್ಣಾಙ್ಕ result;

	ಯದಿ (!(fp = fopen(file, "r")))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		ವರ್ಣ buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, ಪರಿಮಾಣ(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯಾವತ್ ((len = xgetline(&ಸಾಲು, &size, fp)) != -1) {
		ವರ್ಣ *str = ಸಾಲು;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		ಕುರು {
			ಯದಿ (regexec(&re, str, 0, ರಿಕ್ತ, 0) == 0)
				ಗಚ್ಛ found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ಯಾವತ್ (str < ಸಾಲು + len && *str == '\0')
				str++;
		} ಯಾವತ್ (str < ಸಾಲು + len);
	}
	retval = false;
found:
	fclose(fp);
	ಮುಕ್ತ(ಸಾಲು);
	regfree(&re);

	ಪ್ರತ್ಯಾಗಮ retval;
}
#endif


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
get_systype(ಶೂನ್ಯ)
{
	ವರ್ಣ *systype = rc_conf_ಮೌಲ್ಯ("rc_sys");
	ಯದಿ (systype) {
		ವರ್ಣ *s = systype;
		/* Convert to uppercase */
		ಯಾವತ್ (s && *s) {
			ಯದಿ (islower((unsigned ವರ್ಣ) *s))
				*s = toupper((unsigned ವರ್ಣ) *s);
			s++;
		}
	}
	ಪ್ರತ್ಯಾಗಮ systype;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
detect_container(ಧ್ರುವ ವರ್ಣ *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ಯದಿ (systype) {
		ಯದಿ (strcmp(systype, RC_SYS_NONE) == 0)
		       ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		ಯದಿ (strcmp(systype, RC_SYS_JAIL) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_JAIL;
		ಯದಿ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_PODMAN;
	}

	ಪೂರ್ಣಾಙ್ಕ jailed = 0;
	size_t len = ಪರಿಮಾಣ(jailed);

	ಯದಿ (sysctlbyname("security.jail.jailed", &jailed, &len, ರಿಕ್ತ, 0) == 0)
		ಯದಿ (jailed == 1)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_JAIL;

	ಯದಿ (access("/var/run/.containerenv", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ಯದಿ (systype) {
		ಯದಿ (strcmp(systype, RC_SYS_NONE) == 0)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		ಯದಿ (strcmp(systype, RC_SYS_UML) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_UML;
		ಯದಿ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_VSERVER;
		ಯದಿ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_OPENVZ;
		ಯದಿ (strcmp(systype, RC_SYS_LXC) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_LXC;
		ಯದಿ (strcmp(systype, RC_SYS_RKT) == 0)
				ಪ್ರತ್ಯಾಗಮ RC_SYS_RKT;
		ಯದಿ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ಪ್ರತ್ಯಾಗಮ RC_SYS_SYSTEMD_NSPAWN;
		ಯದಿ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ಪ್ರತ್ಯಾಗಮ RC_SYS_DOCKER;
		ಯದಿ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_PODMAN;
		ಯದಿ (strcmp(systype, RC_SYS_WSL) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_WSL;
	}
	ಯದಿ (file_regex("/proc/cpuinfo", "UML"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_UML;
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_VSERVER;
	ಅನ್ಯಥಾ ಯದಿ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_OPENVZ;
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_OPENVZ; /* old test */
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/1/environ", "container=lxc"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_LXC;
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/1/environ", "container=rkt"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_RKT;
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_SYSTEMD_NSPAWN;
	ಅನ್ಯಥಾ ಯದಿ (access("/run/.containerenv", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_PODMAN;
	ಅನ್ಯಥಾ ಯದಿ (access("/.dockerenv", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/1/environ", "container=docker"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_DOCKER;
	ಅನ್ಯಥಾ ಯದಿ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_WSL;
	ಅನ್ಯಥಾ ಯದಿ (getenv("WSL_DISTRO_NAME"))
		ಪ್ರತ್ಯಾಗಮ RC_SYS_WSL;
	ಅನ್ಯಥಾ ಯದಿ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_WSL;
#endif

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
detect_vm(ಧ್ರುವ ವರ್ಣ *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ಯದಿ (systype) {
		ಯದಿ (strcmp(systype, RC_SYS_NONE) == 0)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		ಯದಿ (strcmp(systype, RC_SYS_XEN0) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_XEN0;
		ಯದಿ (strcmp(systype, RC_SYS_XENU) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_XENU;
	}
	ಯದಿ (access("/kern/xen/privcmd", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_XEN0;
	ಯದಿ (access("/kern/xen", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ RC_SYS_XENU;
#endif

#ifdef __linux__
	ಯದಿ (systype) {
		ಯದಿ (strcmp(systype, RC_SYS_NONE) == 0)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		ಯದಿ (strcmp(systype, RC_SYS_XEN0) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_XEN0;
		ಯದಿ (strcmp(systype, RC_SYS_XENU) == 0)
			ಪ್ರತ್ಯಾಗಮ RC_SYS_XENU;
	}
	ಯದಿ (access("/proc/xen", F_OK) == 0) {
		ಯದಿ (file_regex("/proc/xen/capabilities", "control_d"))
			ಪ್ರತ್ಯಾಗಮ RC_SYS_XEN0;
		ಪ್ರತ್ಯಾಗಮ RC_SYS_XENU;
	}
#endif

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಧ್ರುವ ವರ್ಣ *
rc_sys(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *systype;
	ಧ್ರುವ ವರ್ಣ *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ಯದಿ (!sys) {
		sys = detect_vm(systype);
	}

	ಪ್ರತ್ಯಾಗಮ sys;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
rc_parse_service_state(RC_SERVICE state)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; rc_service_state_names[i].name; i++) {
		ಯದಿ (rc_service_state_names[i].state == state)
			ಪ್ರತ್ಯಾಗಮ rc_service_state_names[i].name;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	ಪ್ರತಿ (size_t i = 0; rc_service_state_names[i].name; i++)
		ಯದಿ (rc_service_state_names[i].state == state)
			ಪ್ರತ್ಯಾಗಮ rc_service_state_names[i].dir;
	ಪ್ರತ್ಯಾಗಮ -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ಸ್ಥಿರ ಶೂನ್ಯ
get_runlevel_chain(ಧ್ರುವ ವರ್ಣ *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ಧ್ರುವ ವರ್ಣ *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	ಯದಿ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ಪ್ರತ್ಯಾಗಮ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ಯದಿ (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->ಮೌಲ್ಯ;

		/* Check for loop */
		ಯದಿ (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->ಮೌಲ್ಯ);
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
rc_runlevel_starting(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

ವರ್ಣ *
rc_runlevel_get(ಶೂನ್ಯ)
{
	FILE *fp;
	ವರ್ಣ *runlevel = ರಿಕ್ತ;
	size_t i;

	ಯದಿ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(ಪರಿಮಾಣ(ವರ್ಣ) * PATH_MAX);
		ಯದಿ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ಯದಿ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ಅನ್ಯಥಾ
			*runlevel = '\0';
		fclose(fp);
	}

	ಯದಿ (!runlevel || !*runlevel) {
		ಮುಕ್ತ(runlevel);
		runlevel = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ runlevel;
}

bool
rc_runlevel_set(ಧ್ರುವ ವರ್ಣ *runlevel)
{
	FILE *fp;

	ಯದಿ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ಪ್ರತ್ಯಾಗಮ false;

	fputs(runlevel, fp);
	fclose(fp);
	ಪ್ರತ್ಯಾಗಮ true;
}

bool
rc_runlevel_exists(ಧ್ರುವ ವರ್ಣ *runlevel)
{
	ಸಂರಚನಾ stat buf;

	ಯದಿ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ಪ್ರತ್ಯಾಗಮ true;
	ಪ್ರತ್ಯಾಗಮ false;
}

bool
rc_runlevel_ರಾಶಿ(ಧ್ರುವ ವರ್ಣ *dst, ಧ್ರುವ ವರ್ಣ *src)
{
	ವರ್ಣ *d, *s;
	ಪೂರ್ಣಾಙ್ಕ r;

	ಯದಿ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ಪ್ರತ್ಯಾಗಮ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	ಮುಕ್ತ(d);
	ಮುಕ್ತ(s);
	ಪ್ರತ್ಯಾಗಮ (r == 0);
}

bool
rc_runlevel_unstack(ಧ್ರುವ ವರ್ಣ *dst, ಧ್ರುವ ವರ್ಣ *src)
{
	ವರ್ಣ *path;
	ಪೂರ್ಣಾಙ್ಕ r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	ಮುಕ್ತ(path);
	ಪ್ರತ್ಯಾಗಮ (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ಧ್ರುವ ವರ್ಣ *runlevel)
{
	RC_STRINGLIST *ರಾಶಿ, *ancestor_list;
	ರಾಶಿ = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ರಾಶಿ, ancestor_list);
	rc_stringlist_free(ancestor_list);
	ಪ್ರತ್ಯಾಗಮ ರಾಶಿ;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *config_path[] = { _RC_PATH, ರಿಕ್ತ };

ಸ್ಥಿರ ಸಂರಚನಾ {
	bool set;
	ವರ್ಣ *svcdir, *confdir, *runleveldir;
} rc_dirs;

ಸ್ಥಿರ ಸಂರಚನಾ {
	ವರ್ಣ *buffer;
	size_t ಎಣಿಕೆ;
	ಪೂರ್ಣಾಙ್ಕ *fds;
	ಧ್ರುವ ವರ್ಣ **entries, **allocd;
} rc_path;

ಸ್ಥಿರ ಶೂನ್ಯ
free_rc_dirs(ಶೂನ್ಯ)
{
#define X(ptr) ptr = (ಮುಕ್ತ(ptr), ರಿಕ್ತ)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = ರಿಕ್ತ;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.ಎಣಿಕೆ = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
ಸ್ಥಿರ ಶೂನ್ಯ
append_path(FILE *fp, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.ಎಣಿಕೆ++;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
strnsep(ಧ್ರುವ ವರ್ಣ **str, size_t *size, ಧ್ರುವ ವರ್ಣ sep)
{
	ಧ್ರುವ ವರ್ಣ *ret = *str;

	ಯದಿ (!ret)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ಯದಿ (!**str)
		*str = ರಿಕ್ತ;
	ಅನ್ಯಥಾ
		*str += 1;

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *
strnullsep(ಧ್ರುವ ವರ್ಣ **str, size_t *size)
{
	ಧ್ರುವ ವರ್ಣ *ret = *str;
	size_t len;

	ಯದಿ (!ret)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	len = strnlen(ret, *size - 1) + 1;

	ಯದಿ ((*size -= len))
		*str += len;
	ಅನ್ಯಥಾ
		*str = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ
append_user_path(FILE *fp)
{
	ಧ್ರುವ ವರ್ಣ *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ಯದಿ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ಅನ್ಯಥಾ ಯಾವತ್ ((entry = strnsep(&xdg, &size, ':')))
		ಯದಿ (size) append_path(fp, "%.*s/rc", (ಪೂರ್ಣಾಙ್ಕ) size, entry);

	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

ಸ್ಥಿರ ಶೂನ್ಯ
append_rc_path(FILE *fp, ಧ್ರುವ ವರ್ಣ *path, bool user)
{
	bool has_user = false;
	ಧ್ರುವ ವರ್ಣ *entry;
	size_t size;

	ಯಾವತ್ ((entry = strnsep(&path, &size, ':'))) {
		ಯದಿ (size)
			append_path(fp, "%.*s%s", (ಪೂರ್ಣಾಙ್ಕ)size, entry, user ? "/user" : "");
		ಅನ್ಯಥಾ ಯದಿ (user && !has_user)
			append_user_path(fp), has_user = true;
		ಅನ್ಯಥಾ ಯದಿ (!user) ಯಾವತ್ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
load_dynamic_path(bool user)
{
	ಧ್ರುವ ವರ್ಣ *path, *entry;
	size_t size, ಎಣಿಕೆ = 0;
	FILE *fp;

	ಯದಿ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.ಎಣಿಕೆ = ARRAY_SIZE(config_path) - 1;

	ಯದಿ (!(path = getenv("RC_PATH")) && !user)
		ಪ್ರತ್ಯಾಗಮ;

	rc_path.ಎಣಿಕೆ = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ಯದಿ (path)
		append_rc_path(fp, path, user);
	ಅನ್ಯಥಾ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.ಎಣಿಕೆ + 1, ಪರಿಮಾಣ(*rc_path.entries));

	path = rc_path.buffer;

	ಯಾವತ್ ((entry = strnullsep(&path, &size)) && ಎಣಿಕೆ < rc_path.ಎಣಿಕೆ)
		rc_path.entries[ಎಣಿಕೆ++] = entry;
	rc_path.entries[rc_path.ಎಣಿಕೆ] = ರಿಕ್ತ;
}

ಸ್ಥಿರ bool is_user = false;

bool
rc_is_user(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ _is_user = -1;

	ಯದಿ (_is_user == -1) {
		ವರ್ಣ *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ಪ್ರತ್ಯಾಗಮ _is_user;
}

ಶೂನ್ಯ
rc_set_user(ಶೂನ್ಯ)
{
	ವರ್ಣ *env;

	ಯದಿ (is_user)
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (!rc_dirs.set)
		atexit(free_rc_dirs);
	ಅನ್ಯಥಾ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ಯದಿ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ಅನ್ಯಥಾ ಯದಿ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ಅನ್ಯಥಾ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ಯದಿ (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ *
rc_scriptdirs(ಶೂನ್ಯ)
{
	ಯದಿ (!rc_path.entries)
		load_dynamic_path(false);
	ಪ್ರತ್ಯಾಗಮ rc_path.entries;
}

size_t
rc_scriptdirfds(ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ **fds)
{
	ಸ್ಥಿರ size_t len = 0;

	ಯದಿ (len == 0) {
		ಯದಿ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.ಎಣಿಕೆ * ಪರಿಮಾಣ(*rc_path.fds));
		ಪ್ರತಿ (size_t i = 0; i < rc_path.ಎಣಿಕೆ; i++) {
			ಪೂರ್ಣಾಙ್ಕ fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ಯದಿ (fd == -1)
				ಅನುವರ್ತ;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ಪ್ರತ್ಯಾಗಮ len;
}

ಧ್ರುವ ವರ್ಣ *
rc_sysconfdir(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ RC_SYSCONFDIR;
}

ಧ್ರುವ ವರ್ಣ *
rc_usrconfdir(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ rc_dirs.confdir ? rc_dirs.confdir : ರಿಕ್ತ;
}

ಧ್ರುವ ವರ್ಣ *
rc_runleveldir(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ಧ್ರುವ ವರ್ಣ *
rc_svcdir(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *svcdir;

	ಯದಿ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ಪ್ರತ್ಯಾಗಮ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ಸ್ಥಿರ ssize_t
safe_readlink(ಪೂರ್ಣಾಙ್ಕ targetfd, ಧ್ರುವ ವರ್ಣ *path, ವರ್ಣ **buf, size_t bufsiz)
{
	ವರ್ಣ *buffer;
	ssize_t r;

	ಪ್ರತಿ (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ಯದಿ (r < 0) {
			ಮುಕ್ತ(buffer);
			ಪ್ರತ್ಯಾಗಮ -errno;
		}
		ಯದಿ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ಪ್ರತ್ಯಾಗಮ r;
		}
		ಮುಕ್ತ(buffer);
		ಯದಿ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
ವರ್ಣ *
rc_service_resolve(ಧ್ರುವ ವರ್ಣ *service)
{
	ವರ್ಣ *buffer;
	ಸಂರಚನಾ stat buf;

	ಯದಿ (!service)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (service[0] == '/')
		ಪ್ರತ್ಯಾಗಮ xstrdup(service);

	/* First check started services */
	ಯದಿ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ಯದಿ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ಪ್ರತ್ಯಾಗಮ buffer;
	ಯದಿ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ಯದಿ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ಪ್ರತ್ಯಾಗಮ buffer;

	ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		ವರ್ಣ *file = ರಿಕ್ತ;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ಯದಿ (stat(file, &buf) == 0)
			ಪ್ರತ್ಯಾಗಮ file;
		ಮುಕ್ತ(file);
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

bool
rc_service_exists(ಧ್ರುವ ವರ್ಣ *service)
{
	ವರ್ಣ *file;
	bool retval = false;
	size_t len;
	ಸಂರಚನಾ stat buf;

	ಯದಿ (!service) {
		errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ಯದಿ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (stat(file, &buf) == 0) {
		ಯದಿ (buf.st_mode & S_IXUGO)
			retval = true;
		ಅನ್ಯಥಾ
			errno = ENOEXEC;
	}
	ಮುಕ್ತ(file);
	ಪ್ರತ್ಯಾಗಮ retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ಧ್ರುವ ವರ್ಣ *service)
{
	ವರ್ಣ *svc;
	ವರ್ಣ *cmd = ರಿಕ್ತ;
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	size_t len = 0;
	RC_STRINGLIST *commands = ರಿಕ್ತ;
	ವರ್ಣ *token;
	ವರ್ಣ *p;
	FILE *fp;

	ಯದಿ (!(svc = rc_service_resolve(service)))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	xasprintf(&cmd, OPTSTR, svc);
	ಮುಕ್ತ(svc);

	ಯದಿ (!(fp = popen(cmd, "r"))) {
		ಮುಕ್ತ(cmd);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ಯಾವತ್ ((token = strsep(&p, " ")))
			ಯದಿ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	ಮುಕ್ತ(buffer);
	ಮುಕ್ತ(cmd);
	ಪ್ರತ್ಯಾಗಮ commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
ವರ್ಣ *
rc_service_description(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *option)
{
	ವರ್ಣ *svc;
	ವರ್ಣ *cmd;
	ವರ್ಣ *desc = ರಿಕ್ತ;
	size_t size = 0;
	FILE *fp;

	ಯದಿ (!(svc = rc_service_resolve(service)))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	ಮುಕ್ತ(svc);
	ಯದಿ ((fp = popen(cmd, "r"))) {
		ಯದಿ (xgetline(&desc, &size, fp) == -1) {
			ಮುಕ್ತ(desc);
			desc = ರಿಕ್ತ;
		}
		pclose(fp);
	}
	ಮುಕ್ತ(cmd);
	ಪ್ರತ್ಯಾಗಮ desc;
}

bool
rc_service_in_runlevel(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *runlevel)
{
	ವರ್ಣ *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	ಮುಕ್ತ(file);
	ಪ್ರತ್ಯಾಗಮ r;
}

bool
rc_service_mark(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ RC_SERVICE state)
{
	ಪೂರ್ಣಾಙ್ಕ i = 0;
	ಧ್ರುವ ವರ್ಣ *base;
	ವರ್ಣ *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ಪೂರ್ಣಾಙ್ಕ serrno;

	ಯದಿ (!init)
		ಪ್ರತ್ಯಾಗಮ false;

	base = basename_c(service);
	ಯದಿ (state != RC_SERVICE_STOPPED) {
		ಪೂರ್ಣಾಙ್ಕ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ಯದಿ (access(init, F_OK) != 0) {
			ಮುಕ್ತ(init);
			ಪ್ರತ್ಯಾಗಮ false;
		}

		unlinkat(state_dirfd, base, 0);
		ಯದಿ ((symlinkat(init, state_dirfd, base)) != 0) {
			ಮುಕ್ತ(init);
			ಪ್ರತ್ಯಾಗಮ false;
		}
	}

	ಯದಿ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		ಮುಕ್ತ(init);
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Remove any old states now */
	ಪ್ರತಿ (i = 0; rc_service_state_names[i].name; i++) {
		ಪೂರ್ಣಾಙ್ಕ state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ಯದಿ (s == state)
			ಅನುವರ್ತ;

		ವಿಕಲ್ಪ (s) {
		ಪಕ್ಷ RC_SERVICE_STOPPED:
		ಪಕ್ಷ RC_SERVICE_HOTPLUGGED:
		ಪಕ್ಷ RC_SERVICE_SCHEDULED:
			ಅನುವರ್ತ;
		ಪಕ್ಷ RC_SERVICE_WASINACTIVE:
			ಯದಿ (skip_wasinactive)
				ಅನುವರ್ತ;
			/* fall-through */
		ಸ್ವಭಾವ:
			ಯದಿ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				ಅನುವರ್ತ;
			ಯದಿ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ಯದಿ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					ಮುಕ್ತ(init);
					ಪ್ರತ್ಯಾಗಮ false;
				}
				skip_wasinactive = true;
			}
			ಯದಿ (unlinkat(state_dirfd, base, 0) == -1) {
				ಮುಕ್ತ(init);
				ಪ್ರತ್ಯಾಗಮ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ಯದಿ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ಯದಿ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ಯದಿ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ಯದಿ (dp) {
			ಸಂರಚನಾ dirent *d;
			ಯಾವತ್ ((d = readdir(dp))) {
				ವರ್ಣ *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				ಮುಕ್ತ(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	ಮುಕ್ತ(init);
	ಪ್ರತ್ಯಾಗಮ true;
}

RC_SERVICE
rc_service_state(ಧ್ರುವ ವರ್ಣ *service)
{
	ಧ್ರುವ ವರ್ಣ *base = basename_c(service);
	ಪೂರ್ಣಾಙ್ಕ state = RC_SERVICE_STOPPED;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; rc_service_state_names[i].name; i++) {
		ಯದಿ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ಯದಿ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ಅನ್ಯಥಾ
				state |= rc_service_state_names[i].state;
		}
	}

	ಯದಿ (state & RC_SERVICE_STARTED)
		ಯದಿ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ಯದಿ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ಯದಿ (dp) {
			ಸಂರಚನಾ dirent *d;
			ಯಾವತ್ ((d = readdir(dp))) {
				ಯದಿ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					ಭಙ್ಗ;
				}
			}

			closedir(dp);
		}
	}

	ಪ್ರತ್ಯಾಗಮ state;
}

ವರ್ಣ *
rc_service_ಮೌಲ್ಯ_get(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *option)
{
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	size_t len = 0;
	ವರ್ಣ *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	ಮುಕ್ತ(file);

	ಪ್ರತ್ಯಾಗಮ buffer;
}

bool
rc_service_ಮೌಲ್ಯ_set(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *option, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	bool ret = true;
	ಪೂರ್ಣಾಙ್ಕ optdirfd;
	FILE *fp;

	ಯದಿ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (!ಮೌಲ್ಯ) {
		unlinkat(optdirfd, option, 0);
	} ಅನ್ಯಥಾ ಯದಿ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", ಮೌಲ್ಯ);
		fclose(fp);
	} ಅನ್ಯಥಾ {
		ret = false;
	}

	close(optdirfd);
	ಪ್ರತ್ಯಾಗಮ ret;
}

bool
rc_service_schedule_start(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *service_to_start)
{
	ಧ್ರುವ ವರ್ಣ *base_to_start = basename_c(service_to_start);
	ಧ್ರುವ ವರ್ಣ *base = basename_c(service);
	bool retval;
	ಪೂರ್ಣಾಙ್ಕ schedfd;
	ವರ್ಣ *init;

	/* service may be a provided service, like net */
	ಯದಿ (!service || !rc_service_exists(service_to_start))
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ಪ್ರತ್ಯಾಗಮ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	ಮುಕ್ತ(init);
	ಪ್ರತ್ಯಾಗಮ retval;
}

bool
rc_service_schedule_clear(ಧ್ರುವ ವರ್ಣ *service)
{
	ಪ್ರತ್ಯಾಗಮ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ಧ್ರುವ ವರ್ಣ *runlevel)
{
	RC_STRINGLIST *list = ರಿಕ್ತ;

	ಯದಿ (!runlevel) {
		ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *scriptdirfds;
		list = rc_stringlist_new();
		ಪ್ರತಿ (size_t i = 0, ಎಣಿಕೆ = rc_scriptdirfds(&scriptdirfds); i < ಎಣಿಕೆ; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ಪ್ರತ್ಯಾಗಮ list;
	}

	/* These special levels never contain any services */
	ಯದಿ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ಪ್ರತ್ಯಾಗಮ rc_stringlist_new();

	ಪ್ರತ್ಯಾಗಮ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ಧ್ರುವ ವರ್ಣ *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ರಾಶಿ;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ರಾಶಿ, stacks, entries) {
		sl = rc_services_in_runlevel(ರಾಶಿ->ಮೌಲ್ಯ);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	ಪ್ರತ್ಯಾಗಮ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ಸಂರಚನಾ dirent *d;
	DIR *dp;

	ಯದಿ (state != RC_SERVICE_SCHEDULED)
		ಪ್ರತ್ಯಾಗಮ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ಯದಿ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ಪ್ರತ್ಯಾಗಮ list;

	ಯಾವತ್ ((d = readdir(dp))) {
		ಯದಿ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			ಅನುವರ್ತ;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ಪ್ರತ್ಯಾಗಮ list;
}

bool
rc_service_add(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *service)
{
	bool retval;
	ವರ್ಣ *init;
	ವರ್ಣ *file;
	ವರ್ಣ *path;
	ವರ್ಣ *binit = ರಿಕ್ತ;
	ವರ್ಣ *i;

	ಯದಿ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ಪ್ರತ್ಯಾಗಮ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ಯದಿ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), ರಿಕ್ತ);
		ಯದಿ (path == ರಿಕ್ತ) {
			ಮುಕ್ತ(init);
			ಪ್ರತ್ಯಾಗಮ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		ಮುಕ್ತ(path);
		ಯದಿ (!retval) {
			ಮುಕ್ತ(init);
			errno = EPERM;
			ಪ್ರತ್ಯಾಗಮ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ಯದಿ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		ಗಚ್ಛ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	ಮುಕ್ತ(file);

out:
	ಮುಕ್ತ(binit);
	ಮುಕ್ತ(init);
	ಪ್ರತ್ಯಾಗಮ retval;
}

bool
rc_service_delete(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *service)
{
	ವರ್ಣ *file;
	ಪೂರ್ಣಾಙ್ಕ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	ಮುಕ್ತ(file);
	ಪ್ರತ್ಯಾಗಮ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ಧ್ರುವ ವರ್ಣ *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ಸಂರಚನಾ dirent *d;
	DIR *dp;

	ಯದಿ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ಪ್ರತ್ಯಾಗಮ list;

	ಯಾವತ್ ((d = readdir(dp))) {
		ವರ್ಣ *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ಯದಿ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		ಮುಕ್ತ(file);
	}

	ಪ್ರತ್ಯಾಗಮ list;
}

RC_STRINGLIST *
rc_services_scheduled(ಧ್ರುವ ವರ್ಣ *service)
{
	ಪ್ರತ್ಯಾಗಮ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *env)
{
	size_t name_len = strcspn(env, "=");
	ವರ್ಣ var[strlen(service) + ಪರಿಮಾಣ("/") + name_len];
	ಪೂರ್ಣಾಙ್ಕ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ಯದಿ (env[name_len] != '=')
		ಪ್ರತ್ಯಾಗಮ errno = EINVAL, false;

	snprintf(var, ಪರಿಮಾಣ(var), "%s/%.*s", service, (ಪೂರ್ಣಾಙ್ಕ) name_len, env);

	ಯದಿ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ಪ್ರತ್ಯಾಗಮ false;

	fputs(&env[name_len + 1], fp);

	ಪ್ರತ್ಯಾಗಮ fclose(fp) == 0;
}

bool
rc_service_setenv(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ವರ್ಣ var[strlen(service) + ಪರಿಮಾಣ("/") + strlen(name)];
	ಪೂರ್ಣಾಙ್ಕ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, ಪರಿಮಾಣ(var), "%s/%s", service, name);

	ಯದಿ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ಪ್ರತ್ಯಾಗಮ false;

	ಯದಿ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ಪ್ರತ್ಯಾಗಮ false;

	fputs(ಮೌಲ್ಯ, fp);

	ಪ್ರತ್ಯಾಗಮ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *name, ವರ್ಣ **buf)
{
	ವರ್ಣ var[strlen(service) + ಪರಿಮಾಣ("/") + strlen(name)], *ಮೌಲ್ಯ;
	ಪೂರ್ಣಾಙ್ಕ dfd = rc_dirfd(RC_DIR_ENVIRON);
	ಸಂರಚನಾ stat stat;
	size_t ಎಣಿಕೆ;
	FILE *fp;

	snprintf(var, ಪರಿಮಾಣ(var), "%s/%s", service, name);

	ಯದಿ (fstatat(dfd, var, &stat, 0) == -1)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (!buf)
		ಪ್ರತ್ಯಾಗಮ stat.st_size;

	ಯದಿ (!(ಮೌಲ್ಯ = xrealloc(*buf, stat.st_size + 1)))
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ಪ್ರತ್ಯಾಗಮ -1;

	ಎಣಿಕೆ = fread(ಮೌಲ್ಯ, 1, stat.st_size, fp);
	ಮೌಲ್ಯ[ಎಣಿಕೆ] = '\0';

	fclose(fp);
	*buf = ಮೌಲ್ಯ;
	ಪ್ರತ್ಯಾಗಮ ಎಣಿಕೆ;
}
