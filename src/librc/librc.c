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

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <helpers.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libgen.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <regex.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __FreeBSD__
#  include <sys/sysctl.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#ਜੇ_ਅੰਤ

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ਨਿਰਾ;
ਸਥਿਰ ਪੂਰਨ dirfds[RC_DIR_MAX];

ਪੂਰਨ rc_dirfd(ਗਿਣਤੀ rc_dir dir) {
	ਪੂਰਨ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ਪੂਰਨ dirfd;

	ਜੇ (dir < 0 || dir >= RC_DIR_MAX)
		ਵਾਪਸ -1;
	ਜੇ (dirfds[dir] <= 0) {
		ਚੋਣ (dir) {
		ਹਾਲਤ RC_DIR_SYSCONF:
			ਵਾਪਸ dirfds[dir] = open(rc_sysconfdir(), flags);
		ਹਾਲਤ RC_DIR_USRCONF:
			ਵਾਪਸ dirfds[dir] = open(rc_usrconfdir(), flags);
		ਹਾਲਤ RC_DIR_RUNLEVEL:
			ਵਾਪਸ dirfds[dir] = open(rc_runleveldir(), flags);
		ਹਾਲਤ RC_DIR_SVCDIR:
			ਜੇ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ਵਾਪਸ -1;
			ਵਾਪਸ dirfds[dir] = open(rc_svcdir(), flags);
		ਮੂਲ:
			ਪ੍ਰਤਿਜ੍ਞਾ(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ਜੇ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ਜੇ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ਵਾਪਸ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			ਰੋਕੋ;
		}
	}
	ਵਾਪਸ dirfds[dir];
}

ਖਾਲੀ clear_dirfds(ਖਾਲੀ) {
	ਲਈ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ਜੇ (dirfds[i] <= 0)
			ਜਾਰੀ;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LS_INITD	0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LS_DIR		0x02
ਸਥਿਰ RC_STRINGLIST *
ls_dir(ਪੂਰਨ targetfd, ਅਚਲ ਅੱਖਰ *dir, ਪੂਰਨ options)
{
	DIR *dp;
	ਬਣਤਰ dirent *d;
	RC_STRINGLIST *list = ਨਿਰਾ;
	ਬਣਤਰ stat buf;
	size_t l;

	list = rc_stringlist_new();

	ਜੇ (!(dp = do_opendirat(targetfd, dir)))
		ਵਾਪਸ list;

	ਜਦੋਂ (((d = readdir(dp)) != ਨਿਰਾ)) {
		ਜੇ (d->d_name[0] == '.')
			ਜਾਰੀ;
		ਜੇ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ਜੇ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				ਜਾਰੀ;

			/* .sh files are not init scripts */
			l = ਲੜੀ_ਲੰਬਾਈ(d->d_name);
			ਜੇ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				ਜਾਰੀ;
		}
		ਜੇ (options & LS_DIR) {
			ਜੇ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				ਜਾਰੀ;
		}
		rc_stringlist_ਜੋੜੋ(list, d->d_name);
	}
	closedir(dp);
	ਵਾਪਸ list;
}

ਸਥਿਰ bool
rm_dir(ਪੂਰਨ targetfd, ਅਚਲ ਅੱਖਰ *pathname, bool ਸਿਖਰ)
{
	bool retval = true;
	ਬਣਤਰ dirent *d;
	ਬਣਤਰ stat s;
	DIR *dp;

	ਜੇ (!(dp = do_opendirat(targetfd, pathname)))
		ਵਾਪਸ false;

	ਜਦੋਂ ((d = readdir(dp))) {
		ਜੇ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ਜਾਰੀ;
		ਜੇ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			ਰੋਕੋ;
		}
		ਜੇ (S_ISDIR(s.st_mode)) {
			ਜੇ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				ਰੋਕੋ;
			}
		} ਨਹੀਂਤਾਂ {
			ਜੇ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				ਰੋਕੋ;
			}
		}
	}
	closedir(dp);

	ਜੇ (!retval)
		ਵਾਪਸ false;

	ਜੇ (ਸਿਖਰ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ਵਾਪਸ false;

	ਵਾਪਸ true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
ਸਥਿਰ bool
file_regex(ਅਚਲ ਅੱਖਰ *file, ਅਚਲ ਅੱਖਰ *regex)
{
	FILE *fp;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	size_t ਮਾਪ = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	ਪੂਰਨ ਨਤੀਜਾ;

	ਜੇ (!(fp = fopen(file, "r")))
		ਵਾਪਸ false;

	ਜੇ ((ਨਤੀਜਾ = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		ਅੱਖਰ buf[BUFSIZ];
		fclose(fp);
		regerror(ਨਤੀਜਾ, &re, buf, ਆਕਾਰ(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ਵਾਪਸ false;
	}

	ਜਦੋਂ ((len = xgetline(&ਸਤਰ, &ਮਾਪ, fp)) != -1) {
		ਅੱਖਰ *str = ਸਤਰ;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		ਕਰੋ {
			ਜੇ (regexec(&re, str, 0, ਨਿਰਾ, 0) == 0)
				ਜਾਓ ਮਿਲਿਆ;
			str += ਲੜੀ_ਲੰਬਾਈ(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ਜਦੋਂ (str < ਸਤਰ + len && *str == '\0')
				str++;
		} ਜਦੋਂ (str < ਸਤਰ + len);
	}
	retval = false;
ਮਿਲਿਆ:
	fclose(fp);
	ਮੁਕਤ(ਸਤਰ);
	regfree(&re);

	ਵਾਪਸ retval;
}
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
get_systype(ਖਾਲੀ)
{
	ਅੱਖਰ *systype = rc_conf_ਮੁੱਲ("rc_sys");
	ਜੇ (systype) {
		ਅੱਖਰ *s = systype;
		/* Convert to uppercase */
		ਜਦੋਂ (s && *s) {
			ਜੇ (islower((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *s))
				*s = ਵੱਡਾ_ਅੱਖਰ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *s);
			s++;
		}
	}
	ਵਾਪਸ systype;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
detect_container(ਅਚਲ ਅੱਖਰ *systype RC_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __FreeBSD__
	ਜੇ (systype) {
		ਜੇ (strcmp(systype, RC_SYS_NONE) == 0)
		       ਵਾਪਸ ਨਿਰਾ;
		ਜੇ (strcmp(systype, RC_SYS_JAIL) == 0)
			ਵਾਪਸ RC_SYS_JAIL;
		ਜੇ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ਵਾਪਸ RC_SYS_PODMAN;
	}

	ਪੂਰਨ jailed = 0;
	size_t len = ਆਕਾਰ(jailed);

	ਜੇ (sysctlbyname("security.jail.jailed", &jailed, &len, ਨਿਰਾ, 0) == 0)
		ਜੇ (jailed == 1)
			ਵਾਪਸ RC_SYS_JAIL;

	ਜੇ (access("/var/run/.containerenv", F_OK) == 0)
		ਵਾਪਸ RC_SYS_PODMAN;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਜੇ (systype) {
		ਜੇ (strcmp(systype, RC_SYS_NONE) == 0)
			ਵਾਪਸ ਨਿਰਾ;
		ਜੇ (strcmp(systype, RC_SYS_UML) == 0)
			ਵਾਪਸ RC_SYS_UML;
		ਜੇ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ਵਾਪਸ RC_SYS_VSERVER;
		ਜੇ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ਵਾਪਸ RC_SYS_OPENVZ;
		ਜੇ (strcmp(systype, RC_SYS_LXC) == 0)
			ਵਾਪਸ RC_SYS_LXC;
		ਜੇ (strcmp(systype, RC_SYS_RKT) == 0)
				ਵਾਪਸ RC_SYS_RKT;
		ਜੇ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ਵਾਪਸ RC_SYS_SYSTEMD_NSPAWN;
		ਜੇ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ਵਾਪਸ RC_SYS_DOCKER;
		ਜੇ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ਵਾਪਸ RC_SYS_PODMAN;
		ਜੇ (strcmp(systype, RC_SYS_WSL) == 0)
			ਵਾਪਸ RC_SYS_WSL;
	}
	ਜੇ (file_regex("/proc/cpuinfo", "UML"))
		ਵਾਪਸ RC_SYS_UML;
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ਵਾਪਸ RC_SYS_VSERVER;
	ਨਹੀਂਤਾਂ ਜੇ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ਵਾਪਸ RC_SYS_OPENVZ;
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ਵਾਪਸ RC_SYS_OPENVZ; /* old test */
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/1/environ", "container=lxc"))
		ਵਾਪਸ RC_SYS_LXC;
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/1/environ", "container=rkt"))
		ਵਾਪਸ RC_SYS_RKT;
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ਵਾਪਸ RC_SYS_SYSTEMD_NSPAWN;
	ਨਹੀਂਤਾਂ ਜੇ (access("/run/.containerenv", F_OK) == 0)
		ਵਾਪਸ RC_SYS_PODMAN;
	ਨਹੀਂਤਾਂ ਜੇ (access("/.dockerenv", F_OK) == 0)
		ਵਾਪਸ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/1/environ", "container=docker"))
		ਵਾਪਸ RC_SYS_DOCKER;
	ਨਹੀਂਤਾਂ ਜੇ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ਵਾਪਸ RC_SYS_WSL;
	ਨਹੀਂਤਾਂ ਜੇ (getenv("WSL_DISTRO_NAME"))
		ਵਾਪਸ RC_SYS_WSL;
	ਨਹੀਂਤਾਂ ਜੇ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ਵਾਪਸ RC_SYS_WSL;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
detect_vm(ਅਚਲ ਅੱਖਰ *systype RC_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NetBSD__
	ਜੇ (systype) {
		ਜੇ (strcmp(systype, RC_SYS_NONE) == 0)
			ਵਾਪਸ ਨਿਰਾ;
		ਜੇ (strcmp(systype, RC_SYS_XEN0) == 0)
			ਵਾਪਸ RC_SYS_XEN0;
		ਜੇ (strcmp(systype, RC_SYS_XENU) == 0)
			ਵਾਪਸ RC_SYS_XENU;
	}
	ਜੇ (access("/kern/xen/privcmd", F_OK) == 0)
		ਵਾਪਸ RC_SYS_XEN0;
	ਜੇ (access("/kern/xen", F_OK) == 0)
		ਵਾਪਸ RC_SYS_XENU;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਜੇ (systype) {
		ਜੇ (strcmp(systype, RC_SYS_NONE) == 0)
			ਵਾਪਸ ਨਿਰਾ;
		ਜੇ (strcmp(systype, RC_SYS_XEN0) == 0)
			ਵਾਪਸ RC_SYS_XEN0;
		ਜੇ (strcmp(systype, RC_SYS_XENU) == 0)
			ਵਾਪਸ RC_SYS_XENU;
	}
	ਜੇ (access("/proc/xen", F_OK) == 0) {
		ਜੇ (file_regex("/proc/xen/capabilities", "control_d"))
			ਵਾਪਸ RC_SYS_XEN0;
		ਵਾਪਸ RC_SYS_XENU;
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ ਨਿਰਾ;
}

ਅਚਲ ਅੱਖਰ *
rc_sys(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *systype;
	ਅਚਲ ਅੱਖਰ *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ਜੇ (!sys) {
		sys = detect_vm(systype);
	}

	ਵਾਪਸ sys;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
rc_parse_service_state(RC_SERVICE state)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; rc_service_state_ਨਾਂ[i].name; i++) {
		ਜੇ (rc_service_state_ਨਾਂ[i].state == state)
			ਵਾਪਸ rc_service_state_ਨਾਂ[i].name;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	ਲਈ (size_t i = 0; rc_service_state_ਨਾਂ[i].name; i++)
		ਜੇ (rc_service_state_ਨਾਂ[i].state == state)
			ਵਾਪਸ rc_service_state_ਨਾਂ[i].dir;
	ਵਾਪਸ -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ਸਥਿਰ ਖਾਲੀ
get_runlevel_chain(ਅਚਲ ਅੱਖਰ *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *ਮਾਪਾ;
	ਅਚਲ ਅੱਖਰ *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	ਜੇ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ਵਾਪਸ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ਜੇ (rc_stringlist_ਲੱਭੋ(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_ਜੋੜੋ(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->ਮੁੱਲ;

		/* Check for loop */
		ਜੇ (rc_stringlist_ਲੱਭੋ(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(ਮਾਪਾ, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", ਮਾਪਾ->ਮੁੱਲ);
			ਬਾਹਰ_ਨਿਕਲੋ(1);
		}

		/* Add new ancestor */
		rc_stringlist_ਜੋੜੋ(ancestor_list, nextlevel);

		get_runlevel_chain(nextlevel, level_list, ancestor_list);

		rc_stringlist_delete(ancestor_list, nextlevel);
	}
	rc_stringlist_free(dirs);
}

bool
rc_runlevel_starting(ਖਾਲੀ)
{
	ਵਾਪਸ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(ਖਾਲੀ)
{
	ਵਾਪਸ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(ਖਾਲੀ)
{
	ਵਾਪਸ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

ਅੱਖਰ *
rc_runlevel_get(ਖਾਲੀ)
{
	FILE *fp;
	ਅੱਖਰ *runlevel = ਨਿਰਾ;
	size_t i;

	ਜੇ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(ਆਕਾਰ(ਅੱਖਰ) * PATH_MAX);
		ਜੇ (fgets(runlevel, PATH_MAX, fp)) {
			i = ਲੜੀ_ਲੰਬਾਈ(runlevel);
			ਜੇ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ਨਹੀਂਤਾਂ
			*runlevel = '\0';
		fclose(fp);
	}

	ਜੇ (!runlevel || !*runlevel) {
		ਮੁਕਤ(runlevel);
		runlevel = ਨਿਰਾ;
	}

	ਵਾਪਸ runlevel;
}

bool
rc_runlevel_ਸੈੱਟ(ਅਚਲ ਅੱਖਰ *runlevel)
{
	FILE *fp;

	ਜੇ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ਵਾਪਸ false;

	fputs(runlevel, fp);
	fclose(fp);
	ਵਾਪਸ true;
}

bool
rc_runlevel_exists(ਅਚਲ ਅੱਖਰ *runlevel)
{
	ਬਣਤਰ stat buf;

	ਜੇ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ਵਾਪਸ false;
	ਜੇ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ਵਾਪਸ true;
	ਵਾਪਸ false;
}

bool
rc_runlevel_ਢੇਰ(ਅਚਲ ਅੱਖਰ *dst, ਅਚਲ ਅੱਖਰ *src)
{
	ਅੱਖਰ *d, *s;
	ਪੂਰਨ r;

	ਜੇ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ਵਾਪਸ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	ਮੁਕਤ(d);
	ਮੁਕਤ(s);
	ਵਾਪਸ (r == 0);
}

bool
rc_runlevel_unstack(ਅਚਲ ਅੱਖਰ *dst, ਅਚਲ ਅੱਖਰ *src)
{
	ਅੱਖਰ *ਰਸਤਾ;
	ਪੂਰਨ r;

	xasprintf(&ਰਸਤਾ, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(ਰਸਤਾ);
	ਮੁਕਤ(ਰਸਤਾ);
	ਵਾਪਸ (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ਅਚਲ ਅੱਖਰ *runlevel)
{
	RC_STRINGLIST *ਢੇਰ, *ancestor_list;
	ਢੇਰ = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ਢੇਰ, ancestor_list);
	rc_stringlist_free(ancestor_list);
	ਵਾਪਸ ਢੇਰ;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *config_ਰਸਤਾ[] = { RC_ਰਸਤਾ, ਨਿਰਾ };

ਸਥਿਰ ਬਣਤਰ {
	bool ਸੈੱਟ;
	ਅੱਖਰ *svcdir, *confdir, *runleveldir;
} rc_dirs;

ਸਥਿਰ ਬਣਤਰ {
	ਅੱਖਰ *ਬਫਰ;
	size_t ਗਿਣਤੀ_2;
	ਪੂਰਨ *fds;
	ਅਚਲ ਅੱਖਰ **entries, **allocd;
} rc_ਰਸਤਾ;

ਸਥਿਰ ਖਾਲੀ
free_rc_dirs(ਖਾਲੀ)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ X(ptr) ptr = (ਮੁਕਤ(ptr), ਨਿਰਾ)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_ਰਸਤਾ.allocd);
	/* entries alias freed allocation */
	rc_ਰਸਤਾ.entries = ਨਿਰਾ;
	X(rc_ਰਸਤਾ.ਬਫਰ);
	X(rc_ਰਸਤਾ.fds);
	rc_ਰਸਤਾ.ਗਿਣਤੀ_2 = 0;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ X
}

__attribute__((__format__(__printf__, 2, 3)))
ਸਥਿਰ ਖਾਲੀ
append_ਰਸਤਾ(FILE *fp, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_ਰਸਤਾ.ਗਿਣਤੀ_2++;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
strnsep(ਅਚਲ ਅੱਖਰ **str, size_t *ਮਾਪ, ਅਚਲ ਅੱਖਰ sep)
{
	ਅਚਲ ਅੱਖਰ *ret = *str;

	ਜੇ (!ret)
		ਵਾਪਸ ਨਿਰਾ;

	*str = strchrnul(ret, sep);
	*ਮਾਪ = *str - ret;
	ਜੇ (!**str)
		*str = ਨਿਰਾ;
	ਨਹੀਂਤਾਂ
		*str += 1;

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *
strnullsep(ਅਚਲ ਅੱਖਰ **str, size_t *ਮਾਪ)
{
	ਅਚਲ ਅੱਖਰ *ret = *str;
	size_t len;

	ਜੇ (!ret)
		ਵਾਪਸ ਨਿਰਾ;

	len = strnlen(ret, *ਮਾਪ - 1) + 1;

	ਜੇ ((*ਮਾਪ -= len))
		*str += len;
	ਨਹੀਂਤਾਂ
		*str = ਨਿਰਾ;

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ
append_user_ਰਸਤਾ(FILE *fp)
{
	ਅਚਲ ਅੱਖਰ *xdg, *ਇੰਦਰਾਜ;
	size_t ਮਾਪ;

	append_ਰਸਤਾ(fp, "%s", rc_dirs.confdir);
	ਜੇ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_ਰਸਤਾ(fp, "/etc/xdg/rc");
	ਨਹੀਂਤਾਂ ਜਦੋਂ ((ਇੰਦਰਾਜ = strnsep(&xdg, &ਮਾਪ, ':')))
		ਜੇ (ਮਾਪ) append_ਰਸਤਾ(fp, "%.*s/rc", (ਪੂਰਨ) ਮਾਪ, ਇੰਦਰਾਜ);

	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(config_ਰਸਤਾ) - 2; i++)
		append_ਰਸਤਾ(fp, "%s/user", config_ਰਸਤਾ[i]);
	append_ਰਸਤਾ(fp, "%s", rc_dirs.svcdir);
}

ਸਥਿਰ ਖਾਲੀ
append_rc_ਰਸਤਾ(FILE *fp, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, bool user)
{
	bool has_user = false;
	ਅਚਲ ਅੱਖਰ *ਇੰਦਰਾਜ;
	size_t ਮਾਪ;

	ਜਦੋਂ ((ਇੰਦਰਾਜ = strnsep(&ਰਸਤਾ, &ਮਾਪ, ':'))) {
		ਜੇ (ਮਾਪ)
			append_ਰਸਤਾ(fp, "%.*s%s", (ਪੂਰਨ)ਮਾਪ, ਇੰਦਰਾਜ, user ? "/user" : "");
		ਨਹੀਂਤਾਂ ਜੇ (user && !has_user)
			append_user_ਰਸਤਾ(fp), has_user = true;
		ਨਹੀਂਤਾਂ ਜੇ (!user) ਜਦੋਂ (*rc_ਰਸਤਾ.entries)
			append_ਰਸਤਾ(fp, "%s", *rc_ਰਸਤਾ.entries++);
	}
}

ਸਥਿਰ ਖਾਲੀ
ਲੱਦੋ_dynamic_ਰਸਤਾ(bool user)
{
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, *ਇੰਦਰਾਜ;
	size_t ਮਾਪ, ਗਿਣਤੀ_2 = 0;
	FILE *fp;

	ਜੇ (!rc_dirs.ਸੈੱਟ) {
		atexit(free_rc_dirs);
		rc_dirs.ਸੈੱਟ = true;
	}

	rc_ਰਸਤਾ.entries = config_ਰਸਤਾ;
	rc_ਰਸਤਾ.ਗਿਣਤੀ_2 = ARRAY_ਮਾਪ(config_ਰਸਤਾ) - 1;

	ਜੇ (!(ਰਸਤਾ = getenv("RC_PATH")) && !user)
		ਵਾਪਸ;

	rc_ਰਸਤਾ.ਗਿਣਤੀ_2 = 0;
	fp = xopen_memstream(&rc_ਰਸਤਾ.ਬਫਰ, &ਮਾਪ);
	ਜੇ (ਰਸਤਾ)
		append_rc_ਰਸਤਾ(fp, ਰਸਤਾ, user);
	ਨਹੀਂਤਾਂ
		append_user_ਰਸਤਾ(fp);
	xclose_memstream(fp);

	rc_ਰਸਤਾ.allocd = rc_ਰਸਤਾ.entries = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(rc_ਰਸਤਾ.ਗਿਣਤੀ_2 + 1, ਆਕਾਰ(*rc_ਰਸਤਾ.entries));

	ਰਸਤਾ = rc_ਰਸਤਾ.ਬਫਰ;

	ਜਦੋਂ ((ਇੰਦਰਾਜ = strnullsep(&ਰਸਤਾ, &ਮਾਪ)) && ਗਿਣਤੀ_2 < rc_ਰਸਤਾ.ਗਿਣਤੀ_2)
		rc_ਰਸਤਾ.entries[ਗਿਣਤੀ_2++] = ਇੰਦਰਾਜ;
	rc_ਰਸਤਾ.entries[rc_ਰਸਤਾ.ਗਿਣਤੀ_2] = ਨਿਰਾ;
}

ਸਥਿਰ bool is_user = false;

bool
rc_is_user(ਖਾਲੀ)
{
	ਪੂਰਨ _is_user = -1;

	ਜੇ (_is_user == -1) {
		ਅੱਖਰ *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ਵਾਪਸ _is_user;
}

ਖਾਲੀ
rc_ਸੈੱਟ_user(ਖਾਲੀ)
{
	ਅੱਖਰ *env;

	ਜੇ (is_user)
		ਵਾਪਸ;

	ਜੇ (!rc_dirs.ਸੈੱਟ)
		atexit(free_rc_dirs);
	ਨਹੀਂਤਾਂ
		free_rc_dirs();

	is_user = true;
	rc_dirs.ਸੈੱਟ = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ਜੇ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ਨਹੀਂਤਾਂ ਜੇ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ਨਹੀਂਤਾਂ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ਜੇ (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	ਲੱਦੋ_dynamic_ਰਸਤਾ(true);

	clear_dirfds();
}

ਅਚਲ ਅੱਖਰ * ਅਚਲ *
rc_scriptdirs(ਖਾਲੀ)
{
	ਜੇ (!rc_ਰਸਤਾ.entries)
		ਲੱਦੋ_dynamic_ਰਸਤਾ(false);
	ਵਾਪਸ rc_ਰਸਤਾ.entries;
}

size_t
rc_scriptdirfds(ਅਚਲ ਪੂਰਨ **fds)
{
	ਸਥਿਰ size_t len = 0;

	ਜੇ (len == 0) {
		ਜੇ (!rc_ਰਸਤਾ.entries)
			ਲੱਦੋ_dynamic_ਰਸਤਾ(false);

		rc_ਰਸਤਾ.fds = xmalloc(rc_ਰਸਤਾ.ਗਿਣਤੀ_2 * ਆਕਾਰ(*rc_ਰਸਤਾ.fds));
		ਲਈ (size_t i = 0; i < rc_ਰਸਤਾ.ਗਿਣਤੀ_2; i++) {
			ਪੂਰਨ fd = open(rc_ਰਸਤਾ.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ਜੇ (fd == -1)
				ਜਾਰੀ;

			rc_ਰਸਤਾ.fds[len++] = fd;
		}
	}

	*fds = rc_ਰਸਤਾ.fds;
	ਵਾਪਸ len;
}

ਅਚਲ ਅੱਖਰ *
rc_sysconfdir(ਖਾਲੀ)
{
	ਵਾਪਸ RC_SYSCONFDIR;
}

ਅਚਲ ਅੱਖਰ *
rc_usrconfdir(ਖਾਲੀ)
{
	ਵਾਪਸ rc_dirs.confdir ? rc_dirs.confdir : ਨਿਰਾ;
}

ਅਚਲ ਅੱਖਰ *
rc_runleveldir(ਖਾਲੀ)
{
	ਵਾਪਸ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ਅਚਲ ਅੱਖਰ *
rc_svcdir(ਖਾਲੀ)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *svcdir;

	ਜੇ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ਵਾਪਸ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ਸਥਿਰ ssize_t
ਸੁਰੱਖਿਅਤ_readlink(ਪੂਰਨ targetfd, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅੱਖਰ **buf, size_t bufsiz)
{
	ਅੱਖਰ *ਬਫਰ;
	ssize_t r;

	ਲਈ (;; bufsiz += PATH_MAX) {
		ਬਫਰ = xmalloc(bufsiz);
		r = readlinkat(targetfd, ਰਸਤਾ, ਬਫਰ, bufsiz);
		ਜੇ (r < 0) {
			ਮੁਕਤ(ਬਫਰ);
			ਵਾਪਸ -errno;
		}
		ਜੇ ((size_t)r < bufsiz) {
			ਬਫਰ[r] = '\0';
			*buf = ਬਫਰ;
			ਵਾਪਸ r;
		}
		ਮੁਕਤ(ਬਫਰ);
		ਜੇ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ਵਾਪਸ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
ਅੱਖਰ *
rc_service_resolve(ਅਚਲ ਅੱਖਰ *service)
{
	ਅੱਖਰ *ਬਫਰ;
	ਬਣਤਰ stat buf;

	ਜੇ (!service)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (service[0] == '/')
		ਵਾਪਸ xstrdup(service);

	/* First check started services */
	ਜੇ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ਜੇ (ਸੁਰੱਖਿਅਤ_readlink(rc_dirfd(RC_DIR_STARTED), service, &ਬਫਰ, buf.st_size + 1) >= 0)
			ਵਾਪਸ ਬਫਰ;
	ਜੇ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ਜੇ (ਸੁਰੱਖਿਅਤ_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &ਬਫਰ, buf.st_size + 1) >= 0)
			ਵਾਪਸ ਬਫਰ;

	ਲਈ (ਅਚਲ ਅੱਖਰ * ਅਚਲ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		ਅੱਖਰ *file = ਨਿਰਾ;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ਜੇ (stat(file, &buf) == 0)
			ਵਾਪਸ file;
		ਮੁਕਤ(file);
	}

	ਵਾਪਸ ਨਿਰਾ;
}

bool
rc_service_exists(ਅਚਲ ਅੱਖਰ *service)
{
	ਅੱਖਰ *file;
	bool retval = false;
	size_t len;
	ਬਣਤਰ stat buf;

	ਜੇ (!service) {
		errno = EINVAL;
		ਵਾਪਸ false;
	}

	len = ਲੜੀ_ਲੰਬਾਈ(service);

	/* .sh files are not init scripts */
	ਜੇ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ਵਾਪਸ false;
	}

	ਜੇ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ਵਾਪਸ false;
	}

	ਜੇ (stat(file, &buf) == 0) {
		ਜੇ (buf.st_mode & S_IXUGO)
			retval = true;
		ਨਹੀਂਤਾਂ
			errno = ENOEXEC;
	}
	ਮੁਕਤ(file);
	ਵਾਪਸ retval;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ਅਚਲ ਅੱਖਰ *service)
{
	ਅੱਖਰ *svc;
	ਅੱਖਰ *cmd = ਨਿਰਾ;
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	size_t len = 0;
	RC_STRINGLIST *commands = ਨਿਰਾ;
	ਅੱਖਰ *token;
	ਅੱਖਰ *p;
	FILE *fp;

	ਜੇ (!(svc = rc_service_resolve(service)))
		ਵਾਪਸ ਨਿਰਾ;

	xasprintf(&cmd, OPTSTR, svc);
	ਮੁਕਤ(svc);

	ਜੇ (!(fp = popen(cmd, "r"))) {
		ਮੁਕਤ(cmd);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (xgetline(&ਬਫਰ, &len, fp) != -1) {
		p = ਬਫਰ;
		commands = rc_stringlist_new();

		ਜਦੋਂ ((token = strsep(&p, " ")))
			ਜੇ (token[0] != '\0')
				rc_stringlist_ਜੋੜੋ(commands, token);
	}

	pclose(fp);
	ਮੁਕਤ(ਬਫਰ);
	ਮੁਕਤ(cmd);
	ਵਾਪਸ commands;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DESCSTR ". '%s'; echo \"${description%s%s}\""
ਅੱਖਰ *
rc_service_description(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *option)
{
	ਅੱਖਰ *svc;
	ਅੱਖਰ *cmd;
	ਅੱਖਰ *desc = ਨਿਰਾ;
	size_t ਮਾਪ = 0;
	FILE *fp;

	ਜੇ (!(svc = rc_service_resolve(service)))
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	ਮੁਕਤ(svc);
	ਜੇ ((fp = popen(cmd, "r"))) {
		ਜੇ (xgetline(&desc, &ਮਾਪ, fp) == -1) {
			ਮੁਕਤ(desc);
			desc = ਨਿਰਾ;
		}
		pclose(fp);
	}
	ਮੁਕਤ(cmd);
	ਵਾਪਸ desc;
}

bool
rc_service_ਵਿੱਚ_runlevel(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *runlevel)
{
	ਅੱਖਰ *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	ਮੁਕਤ(file);
	ਵਾਪਸ r;
}

bool
rc_service_mark(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ RC_SERVICE state)
{
	ਪੂਰਨ i = 0;
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ;
	ਅੱਖਰ *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ਪੂਰਨ serrno;

	ਜੇ (!init)
		ਵਾਪਸ false;

	ਅਧਾਰ = basename_c(service);
	ਜੇ (state != RC_SERVICE_STOPPED) {
		ਪੂਰਨ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ਜੇ (access(init, F_OK) != 0) {
			ਮੁਕਤ(init);
			ਵਾਪਸ false;
		}

		unlinkat(state_dirfd, ਅਧਾਰ, 0);
		ਜੇ ((symlinkat(init, state_dirfd, ਅਧਾਰ)) != 0) {
			ਮੁਕਤ(init);
			ਵਾਪਸ false;
		}
	}

	ਜੇ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		ਮੁਕਤ(init);
		ਵਾਪਸ true;
	}

	/* Remove any old states now */
	ਲਈ (i = 0; rc_service_state_ਨਾਂ[i].name; i++) {
		ਪੂਰਨ state_dirfd = rc_dirfd(rc_service_state_ਨਾਂ[i].dir);
		RC_SERVICE s = rc_service_state_ਨਾਂ[i].state;

		ਜੇ (s == state)
			ਜਾਰੀ;

		ਚੋਣ (s) {
		ਹਾਲਤ RC_SERVICE_STOPPED:
		ਹਾਲਤ RC_SERVICE_HOTPLUGGED:
		ਹਾਲਤ RC_SERVICE_SCHEDULED:
			ਜਾਰੀ;
		ਹਾਲਤ RC_SERVICE_WASINACTIVE:
			ਜੇ (skip_wasinactive)
				ਜਾਰੀ;
			/* fall-through */
		ਮੂਲ:
			ਜੇ (faccessat(state_dirfd, ਅਧਾਰ, F_OK, 0) != 0)
				ਜਾਰੀ;
			ਜੇ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ਜੇ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), ਅਧਾਰ) == -1) {
					ਮੁਕਤ(init);
					ਵਾਪਸ false;
				}
				skip_wasinactive = true;
			}
			ਜੇ (unlinkat(state_dirfd, ਅਧਾਰ, 0) == -1) {
				ਮੁਕਤ(init);
				ਵਾਪਸ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ਜੇ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), ਅਧਾਰ, 0);

	/* Remove any options and daemons the service may have stored */
	ਜੇ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), ਅਧਾਰ, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), ਅਧਾਰ, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), ਅਧਾਰ, 0);
		rc_service_ਸਮਾਂ_ਸਾਰਣੀ_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ਜੇ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ਜੇ (dp) {
			ਬਣਤਰ dirent *d;
			ਜਦੋਂ ((d = readdir(dp))) {
				ਅੱਖਰ *was;

				xasprintf(&was, "%s/%s", d->d_name, ਅਧਾਰ);
				unlinkat(dirfd(dp), was, 0);
				ਮੁਕਤ(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	ਮੁਕਤ(init);
	ਵਾਪਸ true;
}

RC_SERVICE
rc_service_state(ਅਚਲ ਅੱਖਰ *service)
{
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = basename_c(service);
	ਪੂਰਨ state = RC_SERVICE_STOPPED;
	ਪੂਰਨ i;

	ਲਈ (i = 0; rc_service_state_ਨਾਂ[i].name; i++) {
		ਜੇ (faccessat(rc_dirfd(rc_service_state_ਨਾਂ[i].dir), ਅਧਾਰ, F_OK, 0) == 0) {
			ਜੇ (rc_service_state_ਨਾਂ[i].state <= 0x10)
				state = rc_service_state_ਨਾਂ[i].state;
			ਨਹੀਂਤਾਂ
				state |= rc_service_state_ਨਾਂ[i].state;
		}
	}

	ਜੇ (state & RC_SERVICE_STARTED)
		ਜੇ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ਜੇ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ਜੇ (dp) {
			ਬਣਤਰ dirent *d;
			ਜਦੋਂ ((d = readdir(dp))) {
				ਜੇ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					ਰੋਕੋ;
				}
			}

			closedir(dp);
		}
	}

	ਵਾਪਸ state;
}

ਅੱਖਰ *
rc_service_ਮੁੱਲ_get(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *option)
{
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	size_t len = 0;
	ਅੱਖਰ *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &ਬਫਰ, &len);
	ਮੁਕਤ(file);

	ਵਾਪਸ ਬਫਰ;
}

bool
rc_service_ਮੁੱਲ_ਸੈੱਟ(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *option, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	bool ret = true;
	ਪੂਰਨ optdirfd;
	FILE *fp;

	ਜੇ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ਵਾਪਸ false;

	ਜੇ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ਵਾਪਸ false;

	ਜੇ (!ਮੁੱਲ) {
		unlinkat(optdirfd, option, 0);
	} ਨਹੀਂਤਾਂ ਜੇ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", ਮੁੱਲ);
		fclose(fp);
	} ਨਹੀਂਤਾਂ {
		ret = false;
	}

	close(optdirfd);
	ਵਾਪਸ ret;
}

bool
rc_service_ਸਮਾਂ_ਸਾਰਣੀ_ਸ਼ੁਰੂ(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *service_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ)
{
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ = basename_c(service_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ);
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = basename_c(service);
	bool retval;
	ਪੂਰਨ schedfd;
	ਅੱਖਰ *init;

	/* service may be a provided service, like net */
	ਜੇ (!service || !rc_service_exists(service_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ))
		ਵਾਪਸ false;

	ਜੇ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), ਅਧਾਰ, 0755) != 0 && errno != EEXIST)
		ਵਾਪਸ false;

	ਜੇ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), ਅਧਾਰ, O_RDONLY)) == -1)
		ਵਾਪਸ false;

	init = rc_service_resolve(service_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ);

	retval = (faccessat(schedfd, ਅਧਾਰ_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ, F_OK, 0) == 0) || (symlinkat(init, schedfd, ਅਧਾਰ_ਮੰਜ਼ਿਲ_ਸ਼ੁਰੂ) == 0);

	close(schedfd);
	ਮੁਕਤ(init);
	ਵਾਪਸ retval;
}

bool
rc_service_ਸਮਾਂ_ਸਾਰਣੀ_clear(ਅਚਲ ਅੱਖਰ *service)
{
	ਵਾਪਸ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_ਵਿੱਚ_runlevel(ਅਚਲ ਅੱਖਰ *runlevel)
{
	RC_STRINGLIST *list = ਨਿਰਾ;

	ਜੇ (!runlevel) {
		ਅਚਲ ਪੂਰਨ *scriptdirfds;
		list = rc_stringlist_new();
		ਲਈ (size_t i = 0, ਗਿਣਤੀ_2 = rc_scriptdirfds(&scriptdirfds); i < ਗਿਣਤੀ_2; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ਵਾਪਸ list;
	}

	/* These special levels never contain any services */
	ਜੇ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ਵਾਪਸ rc_stringlist_new();

	ਵਾਪਸ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_ਵਿੱਚ_runlevel_stacked(ਅਚਲ ਅੱਖਰ *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ਢੇਰ;

	list = rc_services_ਵਿੱਚ_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ਢੇਰ, stacks, entries) {
		sl = rc_services_ਵਿੱਚ_runlevel(ਢੇਰ->ਮੁੱਲ);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	ਵਾਪਸ list;
}

RC_STRINGLIST *
rc_services_ਵਿੱਚ_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ਬਣਤਰ dirent *d;
	DIR *dp;

	ਜੇ (state != RC_SERVICE_SCHEDULED)
		ਵਾਪਸ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ਜੇ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ਵਾਪਸ list;

	ਜਦੋਂ ((d = readdir(dp))) {
		ਜੇ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			ਜਾਰੀ;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ਵਾਪਸ list;
}

bool
rc_service_ਜੋੜੋ(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *service)
{
	bool retval;
	ਅੱਖਰ *init;
	ਅੱਖਰ *file;
	ਅੱਖਰ *ਰਸਤਾ;
	ਅੱਖਰ *binit = ਨਿਰਾ;
	ਅੱਖਰ *i;

	ਜੇ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ਵਾਪਸ false;
	}

	ਜੇ (rc_service_ਵਿੱਚ_runlevel(service, runlevel)) {
		errno = EEXIST;
		ਵਾਪਸ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ਜੇ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		ਰਸਤਾ = realpath(dirname(init), ਨਿਰਾ);
		ਜੇ (ਰਸਤਾ == ਨਿਰਾ) {
			ਮੁਕਤ(init);
			ਵਾਪਸ false;
		}
		retval = (strcmp(ਰਸਤਾ, RC_INITDIR) == 0);
		ਮੁਕਤ(ਰਸਤਾ);
		ਜੇ (!retval) {
			ਮੁਕਤ(init);
			errno = EPERM;
			ਵਾਪਸ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ਜੇ (ਲੜੀ_ਤੁਲਨਾ(i, rc_svcdir(), ਲੜੀ_ਲੰਬਾਈ(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	ਮੁਕਤ(file);

ਬਾਹਰਲਾ:
	ਮੁਕਤ(binit);
	ਮੁਕਤ(init);
	ਵਾਪਸ retval;
}

bool
rc_service_delete(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *service)
{
	ਅੱਖਰ *file;
	ਪੂਰਨ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	ਮੁਕਤ(file);
	ਵਾਪਸ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ਅਚਲ ਅੱਖਰ *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ਬਣਤਰ dirent *d;
	DIR *dp;

	ਜੇ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ਵਾਪਸ list;

	ਜਦੋਂ ((d = readdir(dp))) {
		ਅੱਖਰ *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ਜੇ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_ਜੋੜੋ(list, d->d_name);
		ਮੁਕਤ(file);
	}

	ਵਾਪਸ list;
}

RC_STRINGLIST *
rc_services_scheduled(ਅਚਲ ਅੱਖਰ *service)
{
	ਵਾਪਸ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *env)
{
	size_t name_len = strcspn(env, "=");
	ਅੱਖਰ var[ਲੜੀ_ਲੰਬਾਈ(service) + ਆਕਾਰ("/") + name_len];
	ਪੂਰਨ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ਜੇ (env[name_len] != '=')
		ਵਾਪਸ errno = EINVAL, false;

	snprintf(var, ਆਕਾਰ(var), "%s/%.*s", service, (ਪੂਰਨ) name_len, env);

	ਜੇ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ਵਾਪਸ false;

	ਜੇ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ਵਾਪਸ false;

	fputs(&env[name_len + 1], fp);

	ਵਾਪਸ fclose(fp) == 0;
}

bool
rc_service_setenv(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਅੱਖਰ var[ਲੜੀ_ਲੰਬਾਈ(service) + ਆਕਾਰ("/") + ਲੜੀ_ਲੰਬਾਈ(name)];
	ਪੂਰਨ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, ਆਕਾਰ(var), "%s/%s", service, name);

	ਜੇ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ਵਾਪਸ false;

	ਜੇ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ਵਾਪਸ false;

	fputs(ਮੁੱਲ, fp);

	ਵਾਪਸ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *name, ਅੱਖਰ **buf)
{
	ਅੱਖਰ var[ਲੜੀ_ਲੰਬਾਈ(service) + ਆਕਾਰ("/") + ਲੜੀ_ਲੰਬਾਈ(name)], *ਮੁੱਲ;
	ਪੂਰਨ dfd = rc_dirfd(RC_DIR_ENVIRON);
	ਬਣਤਰ stat stat;
	size_t ਗਿਣਤੀ_2;
	FILE *fp;

	snprintf(var, ਆਕਾਰ(var), "%s/%s", service, name);

	ਜੇ (fstatat(dfd, var, &stat, 0) == -1)
		ਵਾਪਸ -1;

	ਜੇ (!buf)
		ਵਾਪਸ stat.st_size;

	ਜੇ (!(ਮੁੱਲ = xrealloc(*buf, stat.st_size + 1)))
		ਵਾਪਸ -1;

	ਜੇ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ਵਾਪਸ -1;

	ਗਿਣਤੀ_2 = fread(ਮੁੱਲ, 1, stat.st_size, fp);
	ਮੁੱਲ[ਗਿਣਤੀ_2] = '\0';

	fclose(fp);
	*buf = ਮੁੱਲ;
	ਵਾਪਸ ਗਿਣਤੀ_2;
}
