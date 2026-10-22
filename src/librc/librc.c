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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __FreeBSD__
#  include <sys/sysctl.h>
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#ಆದರೆ_ಅಂತ್ಯ

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ಖಾಲಿ;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dirfds[RC_DIR_MAX];

ಪೂರ್ಣಾಂಕ rc_dirfd(ಎಣಿಕೆ rc_dir dir) {
	ಪೂರ್ಣಾಂಕ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ಪೂರ್ಣಾಂಕ dirfd;

	ಆದರೆ (dir < 0 || dir >= RC_DIR_MAX)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (dirfds[dir] <= 0) {
		ಆಯ್ಕೆ (dir) {
		ಸ್ಥಿತಿ RC_DIR_SYSCONF:
			ಹಿಂತಿರುಗಿ dirfds[dir] = open(rc_sysconfdir(), flags);
		ಸ್ಥಿತಿ RC_DIR_USRCONF:
			ಹಿಂತಿರುಗಿ dirfds[dir] = open(rc_usrconfdir(), flags);
		ಸ್ಥಿತಿ RC_DIR_RUNLEVEL:
			ಹಿಂತಿರುಗಿ dirfds[dir] = open(rc_runleveldir(), flags);
		ಸ್ಥಿತಿ RC_DIR_SVCDIR:
			ಆದರೆ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ಹಿಂತಿರುಗಿ -1;
			ಹಿಂತಿರುಗಿ dirfds[dir] = open(rc_svcdir(), flags);
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಪ್ರತಿಜ್ಞಾ(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ಆದರೆ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ಆದರೆ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ಹಿಂತಿರುಗಿ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			ನಿಲ್ಲಿಸು;
		}
	}
	ಹಿಂತಿರುಗಿ dirfds[dir];
}

ಶೂನ್ಯ clear_dirfds(ಶೂನ್ಯ) {
	ಗಾಗಿ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ಆದರೆ (dirfds[i] <= 0)
			ಮುಂದುವರಿ;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#ವ್ಯಾಖ್ಯಾನಿಸು LS_INITD	0x01
#ವ್ಯಾಖ್ಯಾನಿಸು LS_DIR		0x02
ಸ್ಥಿರ RC_STRINGLIST *
ls_dir(ಪೂರ್ಣಾಂಕ targetfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir, ಪೂರ್ಣಾಂಕ options)
{
	DIR *dp;
	ರಚನೆ dirent *d;
	RC_STRINGLIST *list = ಖಾಲಿ;
	ರಚನೆ stat buf;
	size_t l;

	list = rc_stringlist_new();

	ಆದರೆ (!(dp = do_opendirat(targetfd, dir)))
		ಹಿಂತಿರುಗಿ list;

	ಆಗ (((d = readdir(dp)) != ಖಾಲಿ)) {
		ಆದರೆ (d->d_name[0] == '.')
			ಮುಂದುವರಿ;
		ಆದರೆ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ಆದರೆ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				ಮುಂದುವರಿ;

			/* .sh files are not init scripts */
			l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(d->d_name);
			ಆದರೆ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				ಮುಂದುವರಿ;
		}
		ಆದರೆ (options & LS_DIR) {
			ಆದರೆ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				ಮುಂದುವರಿ;
		}
		rc_stringlist_ಕೂಡಿಸು(list, d->d_name);
	}
	closedir(dp);
	ಹಿಂತಿರುಗಿ list;
}

ಸ್ಥಿರ bool
rm_dir(ಪೂರ್ಣಾಂಕ targetfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname, bool ತುದಿ)
{
	bool retval = true;
	ರಚನೆ dirent *d;
	ರಚನೆ stat s;
	DIR *dp;

	ಆದರೆ (!(dp = do_opendirat(targetfd, pathname)))
		ಹಿಂತಿರುಗಿ false;

	ಆಗ ((d = readdir(dp))) {
		ಆದರೆ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ಮುಂದುವರಿ;
		ಆದರೆ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (S_ISDIR(s.st_mode)) {
			ಆದರೆ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				ನಿಲ್ಲಿಸು;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				ನಿಲ್ಲಿಸು;
			}
		}
	}
	closedir(dp);

	ಆದರೆ (!retval)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (ತುದಿ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ಹಿಂತಿರುಗಿ false;

	ಹಿಂತಿರುಗಿ true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
ಸ್ಥಿರ bool
file_regex(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *file, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *regex)
{
	FILE *fp;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	size_t ಗಾತ್ರ_2 = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	ಆದರೆ (!(fp = fopen(file, "r")))
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ ((ಫಲಿತಾಂಶ = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		ಅಕ್ಷರ buf[BUFSIZ];
		fclose(fp);
		regerror(ಫಲಿತಾಂಶ, &re, buf, ಗಾತ್ರ(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ಹಿಂತಿರುಗಿ false;
	}

	ಆಗ ((len = xgetline(&ಸಾಲು, &ಗಾತ್ರ_2, fp)) != -1) {
		ಅಕ್ಷರ *str = ಸಾಲು;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		ಮಾಡು {
			ಆದರೆ (regexec(&re, str, 0, ಖಾಲಿ, 0) == 0)
				ಹೋಗು ಸಿಕ್ಕಿತು;
			str += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ಆಗ (str < ಸಾಲು + len && *str == '\0')
				str++;
		} ಆಗ (str < ಸಾಲು + len);
	}
	retval = false;
ಸಿಕ್ಕಿತು:
	fclose(fp);
	ಬಿಡುಗಡೆ(ಸಾಲು);
	regfree(&re);

	ಹಿಂತಿರುಗಿ retval;
}
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
get_systype(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *systype = rc_conf_ಮೌಲ್ಯ("rc_sys");
	ಆದರೆ (systype) {
		ಅಕ್ಷರ *s = systype;
		/* Convert to uppercase */
		ಆಗ (s && *s) {
			ಆದರೆ (islower((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *s))
				*s = ದೊಡ್ಡ_ಅಕ್ಷರ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *s);
			s++;
		}
	}
	ಹಿಂತಿರುಗಿ systype;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
detect_container(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *systype RC_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __FreeBSD__
	ಆದರೆ (systype) {
		ಆದರೆ (strcmp(systype, RC_SYS_NONE) == 0)
		       ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		ಆದರೆ (strcmp(systype, RC_SYS_JAIL) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_JAIL;
		ಆದರೆ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_PODMAN;
	}

	ಪೂರ್ಣಾಂಕ jailed = 0;
	size_t len = ಗಾತ್ರ(jailed);

	ಆದರೆ (sysctlbyname("security.jail.jailed", &jailed, &len, ಖಾಲಿ, 0) == 0)
		ಆದರೆ (jailed == 1)
			ಹಿಂತಿರುಗಿ RC_SYS_JAIL;

	ಆದರೆ (access("/var/run/.containerenv", F_OK) == 0)
		ಹಿಂತಿರುಗಿ RC_SYS_PODMAN;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಆದರೆ (systype) {
		ಆದರೆ (strcmp(systype, RC_SYS_NONE) == 0)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		ಆದರೆ (strcmp(systype, RC_SYS_UML) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_UML;
		ಆದರೆ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_VSERVER;
		ಆದರೆ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_OPENVZ;
		ಆದರೆ (strcmp(systype, RC_SYS_LXC) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_LXC;
		ಆದರೆ (strcmp(systype, RC_SYS_RKT) == 0)
				ಹಿಂತಿರುಗಿ RC_SYS_RKT;
		ಆದರೆ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ಹಿಂತಿರುಗಿ RC_SYS_SYSTEMD_NSPAWN;
		ಆದರೆ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ಹಿಂತಿರುಗಿ RC_SYS_DOCKER;
		ಆದರೆ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_PODMAN;
		ಆದರೆ (strcmp(systype, RC_SYS_WSL) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_WSL;
	}
	ಆದರೆ (file_regex("/proc/cpuinfo", "UML"))
		ಹಿಂತಿರುಗಿ RC_SYS_UML;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ಹಿಂತಿರುಗಿ RC_SYS_VSERVER;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ಹಿಂತಿರುಗಿ RC_SYS_OPENVZ;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ಹಿಂತಿರುಗಿ RC_SYS_OPENVZ; /* old test */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/1/environ", "container=lxc"))
		ಹಿಂತಿರುಗಿ RC_SYS_LXC;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/1/environ", "container=rkt"))
		ಹಿಂತಿರುಗಿ RC_SYS_RKT;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ಹಿಂತಿರುಗಿ RC_SYS_SYSTEMD_NSPAWN;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (access("/run/.containerenv", F_OK) == 0)
		ಹಿಂತಿರುಗಿ RC_SYS_PODMAN;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (access("/.dockerenv", F_OK) == 0)
		ಹಿಂತಿರುಗಿ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/1/environ", "container=docker"))
		ಹಿಂತಿರುಗಿ RC_SYS_DOCKER;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ಹಿಂತಿರುಗಿ RC_SYS_WSL;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (getenv("WSL_DISTRO_NAME"))
		ಹಿಂತಿರುಗಿ RC_SYS_WSL;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ಹಿಂತಿರುಗಿ RC_SYS_WSL;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
detect_vm(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *systype RC_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NetBSD__
	ಆದರೆ (systype) {
		ಆದರೆ (strcmp(systype, RC_SYS_NONE) == 0)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		ಆದರೆ (strcmp(systype, RC_SYS_XEN0) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_XEN0;
		ಆದರೆ (strcmp(systype, RC_SYS_XENU) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_XENU;
	}
	ಆದರೆ (access("/kern/xen/privcmd", F_OK) == 0)
		ಹಿಂತಿರುಗಿ RC_SYS_XEN0;
	ಆದರೆ (access("/kern/xen", F_OK) == 0)
		ಹಿಂತಿರುಗಿ RC_SYS_XENU;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಆದರೆ (systype) {
		ಆದರೆ (strcmp(systype, RC_SYS_NONE) == 0)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		ಆದರೆ (strcmp(systype, RC_SYS_XEN0) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_XEN0;
		ಆದರೆ (strcmp(systype, RC_SYS_XENU) == 0)
			ಹಿಂತಿರುಗಿ RC_SYS_XENU;
	}
	ಆದರೆ (access("/proc/xen", F_OK) == 0) {
		ಆದರೆ (file_regex("/proc/xen/capabilities", "control_d"))
			ಹಿಂತಿರುಗಿ RC_SYS_XEN0;
		ಹಿಂತಿರುಗಿ RC_SYS_XENU;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
rc_sys(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *systype;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ಆದರೆ (!sys) {
		sys = detect_vm(systype);
	}

	ಹಿಂತಿರುಗಿ sys;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
rc_parse_service_state(RC_SERVICE state)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; rc_service_state_ಹೆಸರುಗಳು[i].name; i++) {
		ಆದರೆ (rc_service_state_ಹೆಸರುಗಳು[i].state == state)
			ಹಿಂತಿರುಗಿ rc_service_state_ಹೆಸರುಗಳು[i].name;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	ಗಾಗಿ (size_t i = 0; rc_service_state_ಹೆಸರುಗಳು[i].name; i++)
		ಆದರೆ (rc_service_state_ಹೆಸರುಗಳು[i].state == state)
			ಹಿಂತಿರುಗಿ rc_service_state_ಹೆಸರುಗಳು[i].dir;
	ಹಿಂತಿರುಗಿ -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ಸ್ಥಿರ ಶೂನ್ಯ
get_runlevel_chain(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *ಪೋಷಕ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	ಆದರೆ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ಹಿಂತಿರುಗಿ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_ಕೂಡಿಸು(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->ಮೌಲ್ಯ;

		/* Check for loop */
		ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(ಪೋಷಕ, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", ಪೋಷಕ->ಮೌಲ್ಯ);
			ನಿರ್ಗಮಿಸು(1);
		}

		/* Add new ancestor */
		rc_stringlist_ಕೂಡಿಸು(ancestor_list, nextlevel);

		get_runlevel_chain(nextlevel, level_list, ancestor_list);

		rc_stringlist_delete(ancestor_list, nextlevel);
	}
	rc_stringlist_free(dirs);
}

bool
rc_runlevel_starting(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

ಅಕ್ಷರ *
rc_runlevel_get(ಶೂನ್ಯ)
{
	FILE *fp;
	ಅಕ್ಷರ *runlevel = ಖಾಲಿ;
	size_t i;

	ಆದರೆ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(ಗಾತ್ರ(ಅಕ್ಷರ) * PATH_MAX);
		ಆದರೆ (fgets(runlevel, PATH_MAX, fp)) {
			i = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(runlevel);
			ಆದರೆ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ಇಲ್ಲದಿದ್ದರೆ
			*runlevel = '\0';
		fclose(fp);
	}

	ಆದರೆ (!runlevel || !*runlevel) {
		ಬಿಡುಗಡೆ(runlevel);
		runlevel = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ runlevel;
}

bool
rc_runlevel_ಗಣ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	FILE *fp;

	ಆದರೆ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ಹಿಂತಿರುಗಿ false;

	fputs(runlevel, fp);
	fclose(fp);
	ಹಿಂತಿರುಗಿ true;
}

bool
rc_runlevel_exists(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	ರಚನೆ stat buf;

	ಆದರೆ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ಹಿಂತಿರುಗಿ true;
	ಹಿಂತಿರುಗಿ false;
}

bool
rc_runlevel_ರಾಶಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dst, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src)
{
	ಅಕ್ಷರ *d, *s;
	ಪೂರ್ಣಾಂಕ r;

	ಆದರೆ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ಹಿಂತಿರುಗಿ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	ಬಿಡುಗಡೆ(d);
	ಬಿಡುಗಡೆ(s);
	ಹಿಂತಿರುಗಿ (r == 0);
}

bool
rc_runlevel_unstack(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dst, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src)
{
	ಅಕ್ಷರ *ಮಾರ್ಗ;
	ಪೂರ್ಣಾಂಕ r;

	xasprintf(&ಮಾರ್ಗ, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(ಮಾರ್ಗ);
	ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
	ಹಿಂತಿರುಗಿ (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	RC_STRINGLIST *ರಾಶಿ, *ancestor_list;
	ರಾಶಿ = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ರಾಶಿ, ancestor_list);
	rc_stringlist_free(ancestor_list);
	ಹಿಂತಿರುಗಿ ರಾಶಿ;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *config_ಮಾರ್ಗ[] = { RC_ಮಾರ್ಗ, ಖಾಲಿ };

ಸ್ಥಿರ ರಚನೆ {
	bool ಗಣ;
	ಅಕ್ಷರ *svcdir, *confdir, *runleveldir;
} rc_dirs;

ಸ್ಥಿರ ರಚನೆ {
	ಅಕ್ಷರ *ಬಫರ್;
	size_t ಎಣಿಕೆ_2;
	ಪೂರ್ಣಾಂಕ *fds;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **entries, **allocd;
} rc_ಮಾರ್ಗ;

ಸ್ಥಿರ ಶೂನ್ಯ
free_rc_dirs(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿಸು X(ptr) ptr = (ಬಿಡುಗಡೆ(ptr), ಖಾಲಿ)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_ಮಾರ್ಗ.allocd);
	/* entries alias freed allocation */
	rc_ಮಾರ್ಗ.entries = ಖಾಲಿ;
	X(rc_ಮಾರ್ಗ.ಬಫರ್);
	X(rc_ಮಾರ್ಗ.fds);
	rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2 = 0;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು X
}

__attribute__((__format__(__printf__, 2, 3)))
ಸ್ಥಿರ ಶೂನ್ಯ
append_ಮಾರ್ಗ(FILE *fp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2++;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
strnsep(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **str, size_t *ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ sep)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret = *str;

	ಆದರೆ (!ret)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	*str = strchrnul(ret, sep);
	*ಗಾತ್ರ_2 = *str - ret;
	ಆದರೆ (!**str)
		*str = ಖಾಲಿ;
	ಇಲ್ಲದಿದ್ದರೆ
		*str += 1;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
strnullsep(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **str, size_t *ಗಾತ್ರ_2)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret = *str;
	size_t len;

	ಆದರೆ (!ret)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	len = strnlen(ret, *ಗಾತ್ರ_2 - 1) + 1;

	ಆದರೆ ((*ಗಾತ್ರ_2 -= len))
		*str += len;
	ಇಲ್ಲದಿದ್ದರೆ
		*str = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ
append_user_ಮಾರ್ಗ(FILE *fp)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *xdg, *ನಮೂದು;
	size_t ಗಾತ್ರ_2;

	append_ಮಾರ್ಗ(fp, "%s", rc_dirs.confdir);
	ಆದರೆ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_ಮಾರ್ಗ(fp, "/etc/xdg/rc");
	ಇಲ್ಲದಿದ್ದರೆ ಆಗ ((ನಮೂದು = strnsep(&xdg, &ಗಾತ್ರ_2, ':')))
		ಆದರೆ (ಗಾತ್ರ_2) append_ಮಾರ್ಗ(fp, "%.*s/rc", (ಪೂರ್ಣಾಂಕ) ಗಾತ್ರ_2, ನಮೂದು);

	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(config_ಮಾರ್ಗ) - 2; i++)
		append_ಮಾರ್ಗ(fp, "%s/user", config_ಮಾರ್ಗ[i]);
	append_ಮಾರ್ಗ(fp, "%s", rc_dirs.svcdir);
}

ಸ್ಥಿರ ಶೂನ್ಯ
append_rc_ಮಾರ್ಗ(FILE *fp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, bool user)
{
	bool has_user = false;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂದು;
	size_t ಗಾತ್ರ_2;

	ಆಗ ((ನಮೂದು = strnsep(&ಮಾರ್ಗ, &ಗಾತ್ರ_2, ':'))) {
		ಆದರೆ (ಗಾತ್ರ_2)
			append_ಮಾರ್ಗ(fp, "%.*s%s", (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ_2, ನಮೂದು, user ? "/user" : "");
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (user && !has_user)
			append_user_ಮಾರ್ಗ(fp), has_user = true;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!user) ಆಗ (*rc_ಮಾರ್ಗ.entries)
			append_ಮಾರ್ಗ(fp, "%s", *rc_ಮಾರ್ಗ.entries++);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
ತುಂಬು_dynamic_ಮಾರ್ಗ(bool user)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, *ನಮೂದು;
	size_t ಗಾತ್ರ_2, ಎಣಿಕೆ_2 = 0;
	FILE *fp;

	ಆದರೆ (!rc_dirs.ಗಣ) {
		atexit(free_rc_dirs);
		rc_dirs.ಗಣ = true;
	}

	rc_ಮಾರ್ಗ.entries = config_ಮಾರ್ಗ;
	rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2 = ARRAY_ಗಾತ್ರ(config_ಮಾರ್ಗ) - 1;

	ಆದರೆ (!(ಮಾರ್ಗ = getenv("RC_PATH")) && !user)
		ಹಿಂತಿರುಗಿ;

	rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2 = 0;
	fp = xopen_memstream(&rc_ಮಾರ್ಗ.ಬಫರ್, &ಗಾತ್ರ_2);
	ಆದರೆ (ಮಾರ್ಗ)
		append_rc_ಮಾರ್ಗ(fp, ಮಾರ್ಗ, user);
	ಇಲ್ಲದಿದ್ದರೆ
		append_user_ಮಾರ್ಗ(fp);
	xclose_memstream(fp);

	rc_ಮಾರ್ಗ.allocd = rc_ಮಾರ್ಗ.entries = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2 + 1, ಗಾತ್ರ(*rc_ಮಾರ್ಗ.entries));

	ಮಾರ್ಗ = rc_ಮಾರ್ಗ.ಬಫರ್;

	ಆಗ ((ನಮೂದು = strnullsep(&ಮಾರ್ಗ, &ಗಾತ್ರ_2)) && ಎಣಿಕೆ_2 < rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2)
		rc_ಮಾರ್ಗ.entries[ಎಣಿಕೆ_2++] = ನಮೂದು;
	rc_ಮಾರ್ಗ.entries[rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2] = ಖಾಲಿ;
}

ಸ್ಥಿರ bool is_user = false;

bool
rc_is_user(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ _is_user = -1;

	ಆದರೆ (_is_user == -1) {
		ಅಕ್ಷರ *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ಹಿಂತಿರುಗಿ _is_user;
}

ಶೂನ್ಯ
rc_ಗಣ_user(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *env;

	ಆದರೆ (is_user)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (!rc_dirs.ಗಣ)
		atexit(free_rc_dirs);
	ಇಲ್ಲದಿದ್ದರೆ
		free_rc_dirs();

	is_user = true;
	rc_dirs.ಗಣ = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ಆದರೆ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ಇಲ್ಲದಿದ್ದರೆ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ಆದರೆ (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	ತುಂಬು_dynamic_ಮಾರ್ಗ(true);

	clear_dirfds();
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *
rc_scriptdirs(ಶೂನ್ಯ)
{
	ಆದರೆ (!rc_ಮಾರ್ಗ.entries)
		ತುಂಬು_dynamic_ಮಾರ್ಗ(false);
	ಹಿಂತಿರುಗಿ rc_ಮಾರ್ಗ.entries;
}

size_t
rc_scriptdirfds(ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ **fds)
{
	ಸ್ಥಿರ size_t len = 0;

	ಆದರೆ (len == 0) {
		ಆದರೆ (!rc_ಮಾರ್ಗ.entries)
			ತುಂಬು_dynamic_ಮಾರ್ಗ(false);

		rc_ಮಾರ್ಗ.fds = xmalloc(rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2 * ಗಾತ್ರ(*rc_ಮಾರ್ಗ.fds));
		ಗಾಗಿ (size_t i = 0; i < rc_ಮಾರ್ಗ.ಎಣಿಕೆ_2; i++) {
			ಪೂರ್ಣಾಂಕ fd = open(rc_ಮಾರ್ಗ.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ಆದರೆ (fd == -1)
				ಮುಂದುವರಿ;

			rc_ಮಾರ್ಗ.fds[len++] = fd;
		}
	}

	*fds = rc_ಮಾರ್ಗ.fds;
	ಹಿಂತಿರುಗಿ len;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
rc_sysconfdir(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ RC_SYSCONFDIR;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
rc_usrconfdir(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ rc_dirs.confdir ? rc_dirs.confdir : ಖಾಲಿ;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
rc_runleveldir(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *
rc_svcdir(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svcdir;

	ಆದರೆ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ಹಿಂತಿರುಗಿ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ಸ್ಥಿರ ssize_t
ಸುರಕ್ಷಿತ_readlink(ಪೂರ್ಣಾಂಕ targetfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ, ಅಕ್ಷರ **buf, size_t bufsiz)
{
	ಅಕ್ಷರ *ಬಫರ್;
	ssize_t r;

	ಗಾಗಿ (;; bufsiz += PATH_MAX) {
		ಬಫರ್ = xmalloc(bufsiz);
		r = readlinkat(targetfd, ಮಾರ್ಗ, ಬಫರ್, bufsiz);
		ಆದರೆ (r < 0) {
			ಬಿಡುಗಡೆ(ಬಫರ್);
			ಹಿಂತಿರುಗಿ -errno;
		}
		ಆದರೆ ((size_t)r < bufsiz) {
			ಬಫರ್[r] = '\0';
			*buf = ಬಫರ್;
			ಹಿಂತಿರುಗಿ r;
		}
		ಬಿಡುಗಡೆ(ಬಫರ್);
		ಆದರೆ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ಹಿಂತಿರುಗಿ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
ಅಕ್ಷರ *
rc_service_resolve(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಅಕ್ಷರ *ಬಫರ್;
	ರಚನೆ stat buf;

	ಆದರೆ (!service)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (service[0] == '/')
		ಹಿಂತಿರುಗಿ xstrdup(service);

	/* First check started services */
	ಆದರೆ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ಆದರೆ (ಸುರಕ್ಷಿತ_readlink(rc_dirfd(RC_DIR_STARTED), service, &ಬಫರ್, buf.st_size + 1) >= 0)
			ಹಿಂತಿರುಗಿ ಬಫರ್;
	ಆದರೆ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ಆದರೆ (ಸುರಕ್ಷಿತ_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &ಬಫರ್, buf.st_size + 1) >= 0)
			ಹಿಂತಿರುಗಿ ಬಫರ್;

	ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		ಅಕ್ಷರ *file = ಖಾಲಿ;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ಆದರೆ (stat(file, &buf) == 0)
			ಹಿಂತಿರುಗಿ file;
		ಬಿಡುಗಡೆ(file);
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

bool
rc_service_exists(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಅಕ್ಷರ *file;
	bool retval = false;
	size_t len;
	ರಚನೆ stat buf;

	ಆದರೆ (!service) {
		errno = EINVAL;
		ಹಿಂತಿರುಗಿ false;
	}

	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(service);

	/* .sh files are not init scripts */
	ಆದರೆ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (stat(file, &buf) == 0) {
		ಆದರೆ (buf.st_mode & S_IXUGO)
			retval = true;
		ಇಲ್ಲದಿದ್ದರೆ
			errno = ENOEXEC;
	}
	ಬಿಡುಗಡೆ(file);
	ಹಿಂತಿರುಗಿ retval;
}

#ವ್ಯಾಖ್ಯಾನಿಸು OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಅಕ್ಷರ *svc;
	ಅಕ್ಷರ *cmd = ಖಾಲಿ;
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	size_t len = 0;
	RC_STRINGLIST *commands = ಖಾಲಿ;
	ಅಕ್ಷರ *token;
	ಅಕ್ಷರ *p;
	FILE *fp;

	ಆದರೆ (!(svc = rc_service_resolve(service)))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	xasprintf(&cmd, OPTSTR, svc);
	ಬಿಡುಗಡೆ(svc);

	ಆದರೆ (!(fp = popen(cmd, "r"))) {
		ಬಿಡುಗಡೆ(cmd);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (xgetline(&ಬಫರ್, &len, fp) != -1) {
		p = ಬಫರ್;
		commands = rc_stringlist_new();

		ಆಗ ((token = strsep(&p, " ")))
			ಆದರೆ (token[0] != '\0')
				rc_stringlist_ಕೂಡಿಸು(commands, token);
	}

	pclose(fp);
	ಬಿಡುಗಡೆ(ಬಫರ್);
	ಬಿಡುಗಡೆ(cmd);
	ಹಿಂತಿರುಗಿ commands;
}

#ವ್ಯಾಖ್ಯಾನಿಸು DESCSTR ". '%s'; echo \"${description%s%s}\""
ಅಕ್ಷರ *
rc_service_description(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *option)
{
	ಅಕ್ಷರ *svc;
	ಅಕ್ಷರ *cmd;
	ಅಕ್ಷರ *desc = ಖಾಲಿ;
	size_t ಗಾತ್ರ_2 = 0;
	FILE *fp;

	ಆದರೆ (!(svc = rc_service_resolve(service)))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	ಬಿಡುಗಡೆ(svc);
	ಆದರೆ ((fp = popen(cmd, "r"))) {
		ಆದರೆ (xgetline(&desc, &ಗಾತ್ರ_2, fp) == -1) {
			ಬಿಡುಗಡೆ(desc);
			desc = ಖಾಲಿ;
		}
		pclose(fp);
	}
	ಬಿಡುಗಡೆ(cmd);
	ಹಿಂತಿರುಗಿ desc;
}

bool
rc_service_ಒಳ_runlevel(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	ಅಕ್ಷರ *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	ಬಿಡುಗಡೆ(file);
	ಹಿಂತಿರುಗಿ r;
}

bool
rc_service_mark(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ RC_SERVICE state)
{
	ಪೂರ್ಣಾಂಕ i = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ;
	ಅಕ್ಷರ *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ಪೂರ್ಣಾಂಕ serrno;

	ಆದರೆ (!init)
		ಹಿಂತಿರುಗಿ false;

	ತಳ = basename_c(service);
	ಆದರೆ (state != RC_SERVICE_STOPPED) {
		ಪೂರ್ಣಾಂಕ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ಆದರೆ (access(init, F_OK) != 0) {
			ಬಿಡುಗಡೆ(init);
			ಹಿಂತಿರುಗಿ false;
		}

		unlinkat(state_dirfd, ತಳ, 0);
		ಆದರೆ ((symlinkat(init, state_dirfd, ತಳ)) != 0) {
			ಬಿಡುಗಡೆ(init);
			ಹಿಂತಿರುಗಿ false;
		}
	}

	ಆದರೆ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		ಬಿಡುಗಡೆ(init);
		ಹಿಂತಿರುಗಿ true;
	}

	/* Remove any old states now */
	ಗಾಗಿ (i = 0; rc_service_state_ಹೆಸರುಗಳು[i].name; i++) {
		ಪೂರ್ಣಾಂಕ state_dirfd = rc_dirfd(rc_service_state_ಹೆಸರುಗಳು[i].dir);
		RC_SERVICE s = rc_service_state_ಹೆಸರುಗಳು[i].state;

		ಆದರೆ (s == state)
			ಮುಂದುವರಿ;

		ಆಯ್ಕೆ (s) {
		ಸ್ಥಿತಿ RC_SERVICE_STOPPED:
		ಸ್ಥಿತಿ RC_SERVICE_HOTPLUGGED:
		ಸ್ಥಿತಿ RC_SERVICE_SCHEDULED:
			ಮುಂದುವರಿ;
		ಸ್ಥಿತಿ RC_SERVICE_WASINACTIVE:
			ಆದರೆ (skip_wasinactive)
				ಮುಂದುವರಿ;
			/* fall-through */
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (faccessat(state_dirfd, ತಳ, F_OK, 0) != 0)
				ಮುಂದುವರಿ;
			ಆದರೆ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ಆದರೆ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), ತಳ) == -1) {
					ಬಿಡುಗಡೆ(init);
					ಹಿಂತಿರುಗಿ false;
				}
				skip_wasinactive = true;
			}
			ಆದರೆ (unlinkat(state_dirfd, ತಳ, 0) == -1) {
				ಬಿಡುಗಡೆ(init);
				ಹಿಂತಿರುಗಿ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ಆದರೆ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), ತಳ, 0);

	/* Remove any options and daemons the service may have stored */
	ಆದರೆ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), ತಳ, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), ತಳ, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), ತಳ, 0);
		rc_service_ವೇಳಾಪಟ್ಟಿ_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ಆದರೆ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ಆದರೆ (dp) {
			ರಚನೆ dirent *d;
			ಆಗ ((d = readdir(dp))) {
				ಅಕ್ಷರ *was;

				xasprintf(&was, "%s/%s", d->d_name, ತಳ);
				unlinkat(dirfd(dp), was, 0);
				ಬಿಡುಗಡೆ(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	ಬಿಡುಗಡೆ(init);
	ಹಿಂತಿರುಗಿ true;
}

RC_SERVICE
rc_service_state(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = basename_c(service);
	ಪೂರ್ಣಾಂಕ state = RC_SERVICE_STOPPED;
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; rc_service_state_ಹೆಸರುಗಳು[i].name; i++) {
		ಆದರೆ (faccessat(rc_dirfd(rc_service_state_ಹೆಸರುಗಳು[i].dir), ತಳ, F_OK, 0) == 0) {
			ಆದರೆ (rc_service_state_ಹೆಸರುಗಳು[i].state <= 0x10)
				state = rc_service_state_ಹೆಸರುಗಳು[i].state;
			ಇಲ್ಲದಿದ್ದರೆ
				state |= rc_service_state_ಹೆಸರುಗಳು[i].state;
		}
	}

	ಆದರೆ (state & RC_SERVICE_STARTED)
		ಆದರೆ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ಆದರೆ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ಆದರೆ (dp) {
			ರಚನೆ dirent *d;
			ಆಗ ((d = readdir(dp))) {
				ಆದರೆ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					ನಿಲ್ಲಿಸು;
				}
			}

			closedir(dp);
		}
	}

	ಹಿಂತಿರುಗಿ state;
}

ಅಕ್ಷರ *
rc_service_ಮೌಲ್ಯ_get(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *option)
{
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	size_t len = 0;
	ಅಕ್ಷರ *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &ಬಫರ್, &len);
	ಬಿಡುಗಡೆ(file);

	ಹಿಂತಿರುಗಿ ಬಫರ್;
}

bool
rc_service_ಮೌಲ್ಯ_ಗಣ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *option, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	bool ret = true;
	ಪೂರ್ಣಾಂಕ optdirfd;
	FILE *fp;

	ಆದರೆ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (!ಮೌಲ್ಯ) {
		unlinkat(optdirfd, option, 0);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", ಮೌಲ್ಯ);
		fclose(fp);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = false;
	}

	close(optdirfd);
	ಹಿಂತಿರುಗಿ ret;
}

bool
rc_service_ವೇಳಾಪಟ್ಟಿ_ಆರಂಭ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service_ಗಮ್ಯ_ಆರಂಭ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ_ಗಮ್ಯ_ಆರಂಭ = basename_c(service_ಗಮ್ಯ_ಆರಂಭ);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = basename_c(service);
	bool retval;
	ಪೂರ್ಣಾಂಕ schedfd;
	ಅಕ್ಷರ *init;

	/* service may be a provided service, like net */
	ಆದರೆ (!service || !rc_service_exists(service_ಗಮ್ಯ_ಆರಂಭ))
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), ತಳ, 0755) != 0 && errno != EEXIST)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), ತಳ, O_RDONLY)) == -1)
		ಹಿಂತಿರುಗಿ false;

	init = rc_service_resolve(service_ಗಮ್ಯ_ಆರಂಭ);

	retval = (faccessat(schedfd, ತಳ_ಗಮ್ಯ_ಆರಂಭ, F_OK, 0) == 0) || (symlinkat(init, schedfd, ತಳ_ಗಮ್ಯ_ಆರಂಭ) == 0);

	close(schedfd);
	ಬಿಡುಗಡೆ(init);
	ಹಿಂತಿರುಗಿ retval;
}

bool
rc_service_ವೇಳಾಪಟ್ಟಿ_clear(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಹಿಂತಿರುಗಿ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_ಒಳ_runlevel(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	RC_STRINGLIST *list = ಖಾಲಿ;

	ಆದರೆ (!runlevel) {
		ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *scriptdirfds;
		list = rc_stringlist_new();
		ಗಾಗಿ (size_t i = 0, ಎಣಿಕೆ_2 = rc_scriptdirfds(&scriptdirfds); i < ಎಣಿಕೆ_2; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ಹಿಂತಿರುಗಿ list;
	}

	/* These special levels never contain any services */
	ಆದರೆ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ಹಿಂತಿರುಗಿ rc_stringlist_new();

	ಹಿಂತಿರುಗಿ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_ಒಳ_runlevel_stacked(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ರಾಶಿ;

	list = rc_services_ಒಳ_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ರಾಶಿ, stacks, entries) {
		sl = rc_services_ಒಳ_runlevel(ರಾಶಿ->ಮೌಲ್ಯ);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	ಹಿಂತಿರುಗಿ list;
}

RC_STRINGLIST *
rc_services_ಒಳ_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ರಚನೆ dirent *d;
	DIR *dp;

	ಆದರೆ (state != RC_SERVICE_SCHEDULED)
		ಹಿಂತಿರುಗಿ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ಆದರೆ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ಹಿಂತಿರುಗಿ list;

	ಆಗ ((d = readdir(dp))) {
		ಆದರೆ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			ಮುಂದುವರಿ;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ಹಿಂತಿರುಗಿ list;
}

bool
rc_service_ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	bool retval;
	ಅಕ್ಷರ *init;
	ಅಕ್ಷರ *file;
	ಅಕ್ಷರ *ಮಾರ್ಗ;
	ಅಕ್ಷರ *binit = ಖಾಲಿ;
	ಅಕ್ಷರ *i;

	ಆದರೆ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (rc_service_ಒಳ_runlevel(service, runlevel)) {
		errno = EEXIST;
		ಹಿಂತಿರುಗಿ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ಆದರೆ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		ಮಾರ್ಗ = realpath(dirname(init), ಖಾಲಿ);
		ಆದರೆ (ಮಾರ್ಗ == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(init);
			ಹಿಂತಿರುಗಿ false;
		}
		retval = (strcmp(ಮಾರ್ಗ, RC_INITDIR) == 0);
		ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
		ಆದರೆ (!retval) {
			ಬಿಡುಗಡೆ(init);
			errno = EPERM;
			ಹಿಂತಿರುಗಿ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(i, rc_svcdir(), ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	ಬಿಡುಗಡೆ(file);

ಹೊರಹೋಗು:
	ಬಿಡುಗಡೆ(binit);
	ಬಿಡುಗಡೆ(init);
	ಹಿಂತಿರುಗಿ retval;
}

bool
rc_service_delete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಅಕ್ಷರ *file;
	ಪೂರ್ಣಾಂಕ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	ಬಿಡುಗಡೆ(file);
	ಹಿಂತಿರುಗಿ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ರಚನೆ dirent *d;
	DIR *dp;

	ಆದರೆ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ಹಿಂತಿರುಗಿ list;

	ಆಗ ((d = readdir(dp))) {
		ಅಕ್ಷರ *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ಆದರೆ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_ಕೂಡಿಸು(list, d->d_name);
		ಬಿಡುಗಡೆ(file);
	}

	ಹಿಂತಿರುಗಿ list;
}

RC_STRINGLIST *
rc_services_scheduled(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಹಿಂತಿರುಗಿ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *env)
{
	size_t name_len = strcspn(env, "=");
	ಅಕ್ಷರ var[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(service) + ಗಾತ್ರ("/") + name_len];
	ಪೂರ್ಣಾಂಕ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ಆದರೆ (env[name_len] != '=')
		ಹಿಂತಿರುಗಿ errno = EINVAL, false;

	snprintf(var, ಗಾತ್ರ(var), "%s/%.*s", service, (ಪೂರ್ಣಾಂಕ) name_len, env);

	ಆದರೆ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ಹಿಂತಿರುಗಿ false;

	fputs(&env[name_len + 1], fp);

	ಹಿಂತಿರುಗಿ fclose(fp) == 0;
}

bool
rc_service_setenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಅಕ್ಷರ var[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(service) + ಗಾತ್ರ("/") + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name)];
	ಪೂರ್ಣಾಂಕ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, ಗಾತ್ರ(var), "%s/%s", service, name);

	ಆದರೆ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ಹಿಂತಿರುಗಿ false;

	ಆದರೆ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ಹಿಂತಿರುಗಿ false;

	fputs(ಮೌಲ್ಯ, fp);

	ಹಿಂತಿರುಗಿ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಅಕ್ಷರ **buf)
{
	ಅಕ್ಷರ var[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(service) + ಗಾತ್ರ("/") + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name)], *ಮೌಲ್ಯ;
	ಪೂರ್ಣಾಂಕ dfd = rc_dirfd(RC_DIR_ENVIRON);
	ರಚನೆ stat stat;
	size_t ಎಣಿಕೆ_2;
	FILE *fp;

	snprintf(var, ಗಾತ್ರ(var), "%s/%s", service, name);

	ಆದರೆ (fstatat(dfd, var, &stat, 0) == -1)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (!buf)
		ಹಿಂತಿರುಗಿ stat.st_size;

	ಆದರೆ (!(ಮೌಲ್ಯ = xrealloc(*buf, stat.st_size + 1)))
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ಹಿಂತಿರುಗಿ -1;

	ಎಣಿಕೆ_2 = fread(ಮೌಲ್ಯ, 1, stat.st_size, fp);
	ಮೌಲ್ಯ[ಎಣಿಕೆ_2] = '\0';

	fclose(fp);
	*buf = ಮೌಲ್ಯ;
	ಹಿಂತಿರುಗಿ ಎಣಿಕೆ_2;
}
