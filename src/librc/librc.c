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

#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ <helpers.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <libgen.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <regex.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/param.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "librc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ଖାଲି;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfds[RC_DIR_MAX];

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc_dirfd(ଗଣନା rc_dir dir) {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd;

	ଯଦି (dir < 0 || dir >= RC_DIR_MAX)
		ଫେରନ୍ତୁ -1;
	ଯଦି (dirfds[dir] <= 0) {
		ବାଛ (dir) {
		ସ୍ଥିତି RC_DIR_SYSCONF:
			ଫେରନ୍ତୁ dirfds[dir] = open(rc_sysconfdir(), flags);
		ସ୍ଥିତି RC_DIR_USRCONF:
			ଫେରନ୍ତୁ dirfds[dir] = open(rc_usrconfdir(), flags);
		ସ୍ଥିତି RC_DIR_RUNLEVEL:
			ଫେରନ୍ତୁ dirfds[dir] = open(rc_runleveldir(), flags);
		ସ୍ଥିତି RC_DIR_SVCDIR:
			ଯଦି (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ଫେରନ୍ତୁ -1;
			ଫେରନ୍ତୁ dirfds[dir] = open(rc_svcdir(), flags);
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ପ୍ରତିଜ୍ଞା(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ଯଦି ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ଯଦି (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ଫେରନ୍ତୁ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			ଭାଙ୍ଗ;
		}
	}
	ଫେରନ୍ତୁ dirfds[dir];
}

ଶୂନ୍ୟ clear_dirfds(ଶୂନ୍ୟ) {
	ପାଇଁ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ଯଦି (dirfds[i] <= 0)
			ଜାରି;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
ସ୍ଥିର RC_STRINGLIST *
ls_dir(ପୂର୍ଣ୍ଣସଂଖ୍ୟା targetfd, ଅଚଳ ଅକ୍ଷର *dir, ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	DIR *dp;
	ସଂରଚନା dirent *d;
	RC_STRINGLIST *list = ଖାଲି;
	ସଂରଚନା stat buf;
	size_t l;

	list = rc_stringlist_new();

	ଯଦି (!(dp = do_opendirat(targetfd, dir)))
		ଫେରନ୍ତୁ list;

	ଯେତେବେଳେ (((d = readdir(dp)) != ଖାଲି)) {
		ଯଦି (d->d_name[0] == '.')
			ଜାରି;
		ଯଦି (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ଯଦି (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				ଜାରି;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ଯଦି (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				ଜାରି;
		}
		ଯଦି (options & LS_DIR) {
			ଯଦି (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				ଜାରି;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ଫେରନ୍ତୁ list;
}

ସ୍ଥିର bool
rm_dir(ପୂର୍ଣ୍ଣସଂଖ୍ୟା targetfd, ଅଚଳ ଅକ୍ଷର *pathname, bool ଶୀର୍ଷ)
{
	bool retval = true;
	ସଂରଚନା dirent *d;
	ସଂରଚନା stat s;
	DIR *dp;

	ଯଦି (!(dp = do_opendirat(targetfd, pathname)))
		ଫେରନ୍ତୁ false;

	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଯଦି (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ଜାରି;
		ଯଦି (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			ଭାଙ୍ଗ;
		}
		ଯଦି (S_ISDIR(s.st_mode)) {
			ଯଦି (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				ଭାଙ୍ଗ;
			}
		} ଅନ୍ୟଥା {
			ଯଦି (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				ଭାଙ୍ଗ;
			}
		}
	}
	closedir(dp);

	ଯଦି (!retval)
		ଫେରନ୍ତୁ false;

	ଯଦି (ଶୀର୍ଷ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ଫେରନ୍ତୁ false;

	ଫେରନ୍ତୁ true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
ସ୍ଥିର bool
file_regex(ଅଚଳ ଅକ୍ଷର *file, ଅଚଳ ଅକ୍ଷର *regex)
{
	FILE *fp;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	ଯଦି (!(fp = fopen(file, "r")))
		ଫେରନ୍ତୁ false;

	ଯଦି ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		ଅକ୍ଷର buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, ଆକାର(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ଫେରନ୍ତୁ false;
	}

	ଯେତେବେଳେ ((len = xgetline(&ଧାଡ଼ି, &size, fp)) != -1) {
		ଅକ୍ଷର *str = ଧାଡ଼ି;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		କର {
			ଯଦି (regexec(&re, str, 0, ଖାଲି, 0) == 0)
				ଯାଅ found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ଯେତେବେଳେ (str < ଧାଡ଼ି + len && *str == '\0')
				str++;
		} ଯେତେବେଳେ (str < ଧାଡ଼ି + len);
	}
	retval = false;
found:
	fclose(fp);
	ମୁକ୍ତ(ଧାଡ଼ି);
	regfree(&re);

	ଫେରନ୍ତୁ retval;
}
#endif


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
get_systype(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *systype = rc_conf_ମୂଲ୍ୟ("rc_sys");
	ଯଦି (systype) {
		ଅକ୍ଷର *s = systype;
		/* Convert to uppercase */
		ଯେତେବେଳେ (s && *s) {
			ଯଦି (islower((unsigned ଅକ୍ଷର) *s))
				*s = toupper((unsigned ଅକ୍ଷର) *s);
			s++;
		}
	}
	ଫେରନ୍ତୁ systype;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
detect_container(ଅଚଳ ଅକ୍ଷର *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ଯଦି (systype) {
		ଯଦି (strcmp(systype, RC_SYS_NONE) == 0)
		       ଫେରନ୍ତୁ ଖାଲି;
		ଯଦି (strcmp(systype, RC_SYS_JAIL) == 0)
			ଫେରନ୍ତୁ RC_SYS_JAIL;
		ଯଦି (strcmp(systype, RC_SYS_PODMAN) == 0)
			ଫେରନ୍ତୁ RC_SYS_PODMAN;
	}

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା jailed = 0;
	size_t len = ଆକାର(jailed);

	ଯଦି (sysctlbyname("security.jail.jailed", &jailed, &len, ଖାଲି, 0) == 0)
		ଯଦି (jailed == 1)
			ଫେରନ୍ତୁ RC_SYS_JAIL;

	ଯଦି (access("/var/run/.containerenv", F_OK) == 0)
		ଫେରନ୍ତୁ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ଯଦି (systype) {
		ଯଦି (strcmp(systype, RC_SYS_NONE) == 0)
			ଫେରନ୍ତୁ ଖାଲି;
		ଯଦି (strcmp(systype, RC_SYS_UML) == 0)
			ଫେରନ୍ତୁ RC_SYS_UML;
		ଯଦି (strcmp(systype, RC_SYS_VSERVER) == 0)
			ଫେରନ୍ତୁ RC_SYS_VSERVER;
		ଯଦି (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ଫେରନ୍ତୁ RC_SYS_OPENVZ;
		ଯଦି (strcmp(systype, RC_SYS_LXC) == 0)
			ଫେରନ୍ତୁ RC_SYS_LXC;
		ଯଦି (strcmp(systype, RC_SYS_RKT) == 0)
				ଫେରନ୍ତୁ RC_SYS_RKT;
		ଯଦି (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ଫେରନ୍ତୁ RC_SYS_SYSTEMD_NSPAWN;
		ଯଦି (strcmp(systype, RC_SYS_DOCKER) == 0)
				ଫେରନ୍ତୁ RC_SYS_DOCKER;
		ଯଦି (strcmp(systype, RC_SYS_PODMAN) == 0)
			ଫେରନ୍ତୁ RC_SYS_PODMAN;
		ଯଦି (strcmp(systype, RC_SYS_WSL) == 0)
			ଫେରନ୍ତୁ RC_SYS_WSL;
	}
	ଯଦି (file_regex("/proc/cpuinfo", "UML"))
		ଫେରନ୍ତୁ RC_SYS_UML;
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ଫେରନ୍ତୁ RC_SYS_VSERVER;
	ଅନ୍ୟଥା ଯଦି (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ଫେରନ୍ତୁ RC_SYS_OPENVZ;
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ଫେରନ୍ତୁ RC_SYS_OPENVZ; /* old test */
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/1/environ", "container=lxc"))
		ଫେରନ୍ତୁ RC_SYS_LXC;
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/1/environ", "container=rkt"))
		ଫେରନ୍ତୁ RC_SYS_RKT;
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ଫେରନ୍ତୁ RC_SYS_SYSTEMD_NSPAWN;
	ଅନ୍ୟଥା ଯଦି (access("/run/.containerenv", F_OK) == 0)
		ଫେରନ୍ତୁ RC_SYS_PODMAN;
	ଅନ୍ୟଥା ଯଦି (access("/.dockerenv", F_OK) == 0)
		ଫେରନ୍ତୁ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/1/environ", "container=docker"))
		ଫେରନ୍ତୁ RC_SYS_DOCKER;
	ଅନ୍ୟଥା ଯଦି (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ଫେରନ୍ତୁ RC_SYS_WSL;
	ଅନ୍ୟଥା ଯଦି (getenv("WSL_DISTRO_NAME"))
		ଫେରନ୍ତୁ RC_SYS_WSL;
	ଅନ୍ୟଥା ଯଦି (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ଫେରନ୍ତୁ RC_SYS_WSL;
#endif

	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
detect_vm(ଅଚଳ ଅକ୍ଷର *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ଯଦି (systype) {
		ଯଦି (strcmp(systype, RC_SYS_NONE) == 0)
			ଫେରନ୍ତୁ ଖାଲି;
		ଯଦି (strcmp(systype, RC_SYS_XEN0) == 0)
			ଫେରନ୍ତୁ RC_SYS_XEN0;
		ଯଦି (strcmp(systype, RC_SYS_XENU) == 0)
			ଫେରନ୍ତୁ RC_SYS_XENU;
	}
	ଯଦି (access("/kern/xen/privcmd", F_OK) == 0)
		ଫେରନ୍ତୁ RC_SYS_XEN0;
	ଯଦି (access("/kern/xen", F_OK) == 0)
		ଫେରନ୍ତୁ RC_SYS_XENU;
#endif

#ifdef __linux__
	ଯଦି (systype) {
		ଯଦି (strcmp(systype, RC_SYS_NONE) == 0)
			ଫେରନ୍ତୁ ଖାଲି;
		ଯଦି (strcmp(systype, RC_SYS_XEN0) == 0)
			ଫେରନ୍ତୁ RC_SYS_XEN0;
		ଯଦି (strcmp(systype, RC_SYS_XENU) == 0)
			ଫେରନ୍ତୁ RC_SYS_XENU;
	}
	ଯଦି (access("/proc/xen", F_OK) == 0) {
		ଯଦି (file_regex("/proc/xen/capabilities", "control_d"))
			ଫେରନ୍ତୁ RC_SYS_XEN0;
		ଫେରନ୍ତୁ RC_SYS_XENU;
	}
#endif

	ଫେରନ୍ତୁ ଖାଲି;
}

ଅଚଳ ଅକ୍ଷର *
rc_sys(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *systype;
	ଅଚଳ ଅକ୍ଷର *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ଯଦି (!sys) {
		sys = detect_vm(systype);
	}

	ଫେରନ୍ତୁ sys;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
rc_parse_service_state(RC_SERVICE state)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; rc_service_state_names[i].name; i++) {
		ଯଦି (rc_service_state_names[i].state == state)
			ଫେରନ୍ତୁ rc_service_state_names[i].name;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	ପାଇଁ (size_t i = 0; rc_service_state_names[i].name; i++)
		ଯଦି (rc_service_state_names[i].state == state)
			ଫେରନ୍ତୁ rc_service_state_names[i].dir;
	ଫେରନ୍ତୁ -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ସ୍ଥିର ଶୂନ୍ୟ
get_runlevel_chain(ଅଚଳ ଅକ୍ଷର *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ଅଚଳ ଅକ୍ଷର *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	ଯଦି (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ଫେରନ୍ତୁ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ଯଦି (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->ମୂଲ୍ୟ;

		/* Check for loop */
		ଯଦି (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->ମୂଲ୍ୟ);
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
rc_runlevel_starting(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

ଅକ୍ଷର *
rc_runlevel_get(ଶୂନ୍ୟ)
{
	FILE *fp;
	ଅକ୍ଷର *runlevel = ଖାଲି;
	size_t i;

	ଯଦି ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(ଆକାର(ଅକ୍ଷର) * PATH_MAX);
		ଯଦି (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ଯଦି (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ଅନ୍ୟଥା
			*runlevel = '\0';
		fclose(fp);
	}

	ଯଦି (!runlevel || !*runlevel) {
		ମୁକ୍ତ(runlevel);
		runlevel = ଖାଲି;
	}

	ଫେରନ୍ତୁ runlevel;
}

bool
rc_runlevel_set(ଅଚଳ ଅକ୍ଷର *runlevel)
{
	FILE *fp;

	ଯଦି (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ଫେରନ୍ତୁ false;

	fputs(runlevel, fp);
	fclose(fp);
	ଫେରନ୍ତୁ true;
}

bool
rc_runlevel_exists(ଅଚଳ ଅକ୍ଷର *runlevel)
{
	ସଂରଚନା stat buf;

	ଯଦି (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ଫେରନ୍ତୁ false;
	ଯଦି (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ଫେରନ୍ତୁ true;
	ଫେରନ୍ତୁ false;
}

bool
rc_runlevel_ଗଦା(ଅଚଳ ଅକ୍ଷର *dst, ଅଚଳ ଅକ୍ଷର *src)
{
	ଅକ୍ଷର *d, *s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

	ଯଦି (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ଫେରନ୍ତୁ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	ମୁକ୍ତ(d);
	ମୁକ୍ତ(s);
	ଫେରନ୍ତୁ (r == 0);
}

bool
rc_runlevel_unstack(ଅଚଳ ଅକ୍ଷର *dst, ଅଚଳ ଅକ୍ଷର *src)
{
	ଅକ୍ଷର *path;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	ମୁକ୍ତ(path);
	ଫେରନ୍ତୁ (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ଅଚଳ ଅକ୍ଷର *runlevel)
{
	RC_STRINGLIST *ଗଦା, *ancestor_list;
	ଗଦା = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ଗଦା, ancestor_list);
	rc_stringlist_free(ancestor_list);
	ଫେରନ୍ତୁ ଗଦା;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *config_path[] = { _RC_PATH, ଖାଲି };

ସ୍ଥିର ସଂରଚନା {
	bool set;
	ଅକ୍ଷର *svcdir, *confdir, *runleveldir;
} rc_dirs;

ସ୍ଥିର ସଂରଚନା {
	ଅକ୍ଷର *buffer;
	size_t ଗଣନା_2;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *fds;
	ଅଚଳ ଅକ୍ଷର **entries, **allocd;
} rc_path;

ସ୍ଥିର ଶୂନ୍ୟ
free_rc_dirs(ଶୂନ୍ୟ)
{
#define X(ptr) ptr = (ମୁକ୍ତ(ptr), ଖାଲି)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = ଖାଲି;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.ଗଣନା_2 = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
ସ୍ଥିର ଶୂନ୍ୟ
append_path(FILE *fp, ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.ଗଣନା_2++;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
strnsep(ଅଚଳ ଅକ୍ଷର **str, size_t *size, ଅଚଳ ଅକ୍ଷର sep)
{
	ଅଚଳ ଅକ୍ଷର *ret = *str;

	ଯଦି (!ret)
		ଫେରନ୍ତୁ ଖାଲି;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ଯଦି (!**str)
		*str = ଖାଲି;
	ଅନ୍ୟଥା
		*str += 1;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *
strnullsep(ଅଚଳ ଅକ୍ଷର **str, size_t *size)
{
	ଅଚଳ ଅକ୍ଷର *ret = *str;
	size_t len;

	ଯଦି (!ret)
		ଫେରନ୍ତୁ ଖାଲି;

	len = strnlen(ret, *size - 1) + 1;

	ଯଦି ((*size -= len))
		*str += len;
	ଅନ୍ୟଥା
		*str = ଖାଲି;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ
append_user_path(FILE *fp)
{
	ଅଚଳ ଅକ୍ଷର *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ଯଦି (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ଅନ୍ୟଥା ଯେତେବେଳେ ((entry = strnsep(&xdg, &size, ':')))
		ଯଦି (size) append_path(fp, "%.*s/rc", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) size, entry);

	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

ସ୍ଥିର ଶୂନ୍ୟ
append_rc_path(FILE *fp, ଅଚଳ ଅକ୍ଷର *path, bool user)
{
	bool has_user = false;
	ଅଚଳ ଅକ୍ଷର *entry;
	size_t size;

	ଯେତେବେଳେ ((entry = strnsep(&path, &size, ':'))) {
		ଯଦି (size)
			append_path(fp, "%.*s%s", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)size, entry, user ? "/user" : "");
		ଅନ୍ୟଥା ଯଦି (user && !has_user)
			append_user_path(fp), has_user = true;
		ଅନ୍ୟଥା ଯଦି (!user) ଯେତେବେଳେ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ
load_dynamic_path(bool user)
{
	ଅଚଳ ଅକ୍ଷର *path, *entry;
	size_t size, ଗଣନା_2 = 0;
	FILE *fp;

	ଯଦି (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.ଗଣନା_2 = ARRAY_SIZE(config_path) - 1;

	ଯଦି (!(path = getenv("RC_PATH")) && !user)
		ଫେରନ୍ତୁ;

	rc_path.ଗଣନା_2 = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ଯଦି (path)
		append_rc_path(fp, path, user);
	ଅନ୍ୟଥା
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.ଗଣନା_2 + 1, ଆକାର(*rc_path.entries));

	path = rc_path.buffer;

	ଯେତେବେଳେ ((entry = strnullsep(&path, &size)) && ଗଣନା_2 < rc_path.ଗଣନା_2)
		rc_path.entries[ଗଣନା_2++] = entry;
	rc_path.entries[rc_path.ଗଣନା_2] = ଖାଲି;
}

ସ୍ଥିର bool is_user = false;

bool
rc_is_user(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା _is_user = -1;

	ଯଦି (_is_user == -1) {
		ଅକ୍ଷର *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ଫେରନ୍ତୁ _is_user;
}

ଶୂନ୍ୟ
rc_set_user(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *env;

	ଯଦି (is_user)
		ଫେରନ୍ତୁ;

	ଯଦି (!rc_dirs.set)
		atexit(free_rc_dirs);
	ଅନ୍ୟଥା
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ଯଦି ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ଅନ୍ୟଥା ଯଦି ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ଅନ୍ୟଥା {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ଯଦି (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

ଅଚଳ ଅକ୍ଷର * ଅଚଳ *
rc_scriptdirs(ଶୂନ୍ୟ)
{
	ଯଦି (!rc_path.entries)
		load_dynamic_path(false);
	ଫେରନ୍ତୁ rc_path.entries;
}

size_t
rc_scriptdirfds(ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା **fds)
{
	ସ୍ଥିର size_t len = 0;

	ଯଦି (len == 0) {
		ଯଦି (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.ଗଣନା_2 * ଆକାର(*rc_path.fds));
		ପାଇଁ (size_t i = 0; i < rc_path.ଗଣନା_2; i++) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ଯଦି (fd == -1)
				ଜାରି;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ଫେରନ୍ତୁ len;
}

ଅଚଳ ଅକ୍ଷର *
rc_sysconfdir(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ RC_SYSCONFDIR;
}

ଅଚଳ ଅକ୍ଷର *
rc_usrconfdir(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ rc_dirs.confdir ? rc_dirs.confdir : ଖାଲି;
}

ଅଚଳ ଅକ୍ଷର *
rc_runleveldir(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ଅଚଳ ଅକ୍ଷର *
rc_svcdir(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *svcdir;

	ଯଦି (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ଫେରନ୍ତୁ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ସ୍ଥିର ssize_t
safe_readlink(ପୂର୍ଣ୍ଣସଂଖ୍ୟା targetfd, ଅଚଳ ଅକ୍ଷର *path, ଅକ୍ଷର **buf, size_t bufsiz)
{
	ଅକ୍ଷର *buffer;
	ssize_t r;

	ପାଇଁ (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ଯଦି (r < 0) {
			ମୁକ୍ତ(buffer);
			ଫେରନ୍ତୁ -errno;
		}
		ଯଦି ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ଫେରନ୍ତୁ r;
		}
		ମୁକ୍ତ(buffer);
		ଯଦି (bufsiz >= SSIZE_MAX - PATH_MAX)
			ଫେରନ୍ତୁ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
ଅକ୍ଷର *
rc_service_resolve(ଅଚଳ ଅକ୍ଷର *service)
{
	ଅକ୍ଷର *buffer;
	ସଂରଚନା stat buf;

	ଯଦି (!service)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (service[0] == '/')
		ଫେରନ୍ତୁ xstrdup(service);

	/* First check started services */
	ଯଦି (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ଯଦି (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ଫେରନ୍ତୁ buffer;
	ଯଦି (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ଯଦି (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ଫେରନ୍ତୁ buffer;

	ପାଇଁ (ଅଚଳ ଅକ୍ଷର * ଅଚଳ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		ଅକ୍ଷର *file = ଖାଲି;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ଯଦି (stat(file, &buf) == 0)
			ଫେରନ୍ତୁ file;
		ମୁକ୍ତ(file);
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

bool
rc_service_exists(ଅଚଳ ଅକ୍ଷର *service)
{
	ଅକ୍ଷର *file;
	bool retval = false;
	size_t len;
	ସଂରଚନା stat buf;

	ଯଦି (!service) {
		errno = EINVAL;
		ଫେରନ୍ତୁ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ଯଦି (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (stat(file, &buf) == 0) {
		ଯଦି (buf.st_mode & S_IXUGO)
			retval = true;
		ଅନ୍ୟଥା
			errno = ENOEXEC;
	}
	ମୁକ୍ତ(file);
	ଫେରନ୍ତୁ retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ଅଚଳ ଅକ୍ଷର *service)
{
	ଅକ୍ଷର *svc;
	ଅକ୍ଷର *cmd = ଖାଲି;
	ଅକ୍ଷର *buffer = ଖାଲି;
	size_t len = 0;
	RC_STRINGLIST *commands = ଖାଲି;
	ଅକ୍ଷର *token;
	ଅକ୍ଷର *p;
	FILE *fp;

	ଯଦି (!(svc = rc_service_resolve(service)))
		ଫେରନ୍ତୁ ଖାଲି;

	xasprintf(&cmd, OPTSTR, svc);
	ମୁକ୍ତ(svc);

	ଯଦି (!(fp = popen(cmd, "r"))) {
		ମୁକ୍ତ(cmd);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ଯେତେବେଳେ ((token = strsep(&p, " ")))
			ଯଦି (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	ମୁକ୍ତ(buffer);
	ମୁକ୍ତ(cmd);
	ଫେରନ୍ତୁ commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
ଅକ୍ଷର *
rc_service_description(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *option)
{
	ଅକ୍ଷର *svc;
	ଅକ୍ଷର *cmd;
	ଅକ୍ଷର *desc = ଖାଲି;
	size_t size = 0;
	FILE *fp;

	ଯଦି (!(svc = rc_service_resolve(service)))
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	ମୁକ୍ତ(svc);
	ଯଦି ((fp = popen(cmd, "r"))) {
		ଯଦି (xgetline(&desc, &size, fp) == -1) {
			ମୁକ୍ତ(desc);
			desc = ଖାଲି;
		}
		pclose(fp);
	}
	ମୁକ୍ତ(cmd);
	ଫେରନ୍ତୁ desc;
}

bool
rc_service_in_runlevel(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *runlevel)
{
	ଅକ୍ଷର *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	ମୁକ୍ତ(file);
	ଫେରନ୍ତୁ r;
}

bool
rc_service_mark(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ RC_SERVICE state)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;
	ଅଚଳ ଅକ୍ଷର *base;
	ଅକ୍ଷର *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno;

	ଯଦି (!init)
		ଫେରନ୍ତୁ false;

	base = basename_c(service);
	ଯଦି (state != RC_SERVICE_STOPPED) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ଯଦି (access(init, F_OK) != 0) {
			ମୁକ୍ତ(init);
			ଫେରନ୍ତୁ false;
		}

		unlinkat(state_dirfd, base, 0);
		ଯଦି ((symlinkat(init, state_dirfd, base)) != 0) {
			ମୁକ୍ତ(init);
			ଫେରନ୍ତୁ false;
		}
	}

	ଯଦି (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		ମୁକ୍ତ(init);
		ଫେରନ୍ତୁ true;
	}

	/* Remove any old states now */
	ପାଇଁ (i = 0; rc_service_state_names[i].name; i++) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ଯଦି (s == state)
			ଜାରି;

		ବାଛ (s) {
		ସ୍ଥିତି RC_SERVICE_STOPPED:
		ସ୍ଥିତି RC_SERVICE_HOTPLUGGED:
		ସ୍ଥିତି RC_SERVICE_SCHEDULED:
			ଜାରି;
		ସ୍ଥିତି RC_SERVICE_WASINACTIVE:
			ଯଦି (skip_wasinactive)
				ଜାରି;
			/* fall-through */
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (faccessat(state_dirfd, base, F_OK, 0) != 0)
				ଜାରି;
			ଯଦି ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ଯଦି (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					ମୁକ୍ତ(init);
					ଫେରନ୍ତୁ false;
				}
				skip_wasinactive = true;
			}
			ଯଦି (unlinkat(state_dirfd, base, 0) == -1) {
				ମୁକ୍ତ(init);
				ଫେରନ୍ତୁ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ଯଦି (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ଯଦି (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ଯଦି (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ଯଦି (dp) {
			ସଂରଚନା dirent *d;
			ଯେତେବେଳେ ((d = readdir(dp))) {
				ଅକ୍ଷର *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				ମୁକ୍ତ(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	ମୁକ୍ତ(init);
	ଫେରନ୍ତୁ true;
}

RC_SERVICE
rc_service_state(ଅଚଳ ଅକ୍ଷର *service)
{
	ଅଚଳ ଅକ୍ଷର *base = basename_c(service);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା state = RC_SERVICE_STOPPED;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; rc_service_state_names[i].name; i++) {
		ଯଦି (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ଯଦି (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ଅନ୍ୟଥା
				state |= rc_service_state_names[i].state;
		}
	}

	ଯଦି (state & RC_SERVICE_STARTED)
		ଯଦି (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ଯଦି (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ଯଦି (dp) {
			ସଂରଚନା dirent *d;
			ଯେତେବେଳେ ((d = readdir(dp))) {
				ଯଦି (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					ଭାଙ୍ଗ;
				}
			}

			closedir(dp);
		}
	}

	ଫେରନ୍ତୁ state;
}

ଅକ୍ଷର *
rc_service_ମୂଲ୍ୟ_get(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *option)
{
	ଅକ୍ଷର *buffer = ଖାଲି;
	size_t len = 0;
	ଅକ୍ଷର *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	ମୁକ୍ତ(file);

	ଫେରନ୍ତୁ buffer;
}

bool
rc_service_ମୂଲ୍ୟ_set(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *option, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	bool ret = true;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା optdirfd;
	FILE *fp;

	ଯଦି (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ଫେରନ୍ତୁ false;

	ଯଦି ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ଫେରନ୍ତୁ false;

	ଯଦି (!ମୂଲ୍ୟ) {
		unlinkat(optdirfd, option, 0);
	} ଅନ୍ୟଥା ଯଦି ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", ମୂଲ୍ୟ);
		fclose(fp);
	} ଅନ୍ୟଥା {
		ret = false;
	}

	close(optdirfd);
	ଫେରନ୍ତୁ ret;
}

bool
rc_service_schedule_start(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *service_to_start)
{
	ଅଚଳ ଅକ୍ଷର *base_to_start = basename_c(service_to_start);
	ଅଚଳ ଅକ୍ଷର *base = basename_c(service);
	bool retval;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା schedfd;
	ଅକ୍ଷର *init;

	/* service may be a provided service, like net */
	ଯଦି (!service || !rc_service_exists(service_to_start))
		ଫେରନ୍ତୁ false;

	ଯଦି (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ଫେରନ୍ତୁ false;

	ଯଦି ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ଫେରନ୍ତୁ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	ମୁକ୍ତ(init);
	ଫେରନ୍ତୁ retval;
}

bool
rc_service_schedule_clear(ଅଚଳ ଅକ୍ଷର *service)
{
	ଫେରନ୍ତୁ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ଅଚଳ ଅକ୍ଷର *runlevel)
{
	RC_STRINGLIST *list = ଖାଲି;

	ଯଦି (!runlevel) {
		ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *scriptdirfds;
		list = rc_stringlist_new();
		ପାଇଁ (size_t i = 0, ଗଣନା_2 = rc_scriptdirfds(&scriptdirfds); i < ଗଣନା_2; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ଫେରନ୍ତୁ list;
	}

	/* These special levels never contain any services */
	ଯଦି (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ଫେରନ୍ତୁ rc_stringlist_new();

	ଫେରନ୍ତୁ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ଅଚଳ ଅକ୍ଷର *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ଗଦା;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ଗଦା, stacks, entries) {
		sl = rc_services_in_runlevel(ଗଦା->ମୂଲ୍ୟ);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	ଫେରନ୍ତୁ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ସଂରଚନା dirent *d;
	DIR *dp;

	ଯଦି (state != RC_SERVICE_SCHEDULED)
		ଫେରନ୍ତୁ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ଯଦି (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ଫେରନ୍ତୁ list;

	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଯଦି (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			ଜାରି;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ଫେରନ୍ତୁ list;
}

bool
rc_service_add(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *service)
{
	bool retval;
	ଅକ୍ଷର *init;
	ଅକ୍ଷର *file;
	ଅକ୍ଷର *path;
	ଅକ୍ଷର *binit = ଖାଲି;
	ଅକ୍ଷର *i;

	ଯଦି (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ଫେରନ୍ତୁ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ଯଦି (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), ଖାଲି);
		ଯଦି (path == ଖାଲି) {
			ମୁକ୍ତ(init);
			ଫେରନ୍ତୁ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		ମୁକ୍ତ(path);
		ଯଦି (!retval) {
			ମୁକ୍ତ(init);
			errno = EPERM;
			ଫେରନ୍ତୁ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ଯଦି (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		ଯାଅ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	ମୁକ୍ତ(file);

out:
	ମୁକ୍ତ(binit);
	ମୁକ୍ତ(init);
	ଫେରନ୍ତୁ retval;
}

bool
rc_service_delete(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *service)
{
	ଅକ୍ଷର *file;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	ମୁକ୍ତ(file);
	ଫେରନ୍ତୁ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ଅଚଳ ଅକ୍ଷର *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ସଂରଚନା dirent *d;
	DIR *dp;

	ଯଦି (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ଫେରନ୍ତୁ list;

	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଅକ୍ଷର *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ଯଦି (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		ମୁକ୍ତ(file);
	}

	ଫେରନ୍ତୁ list;
}

RC_STRINGLIST *
rc_services_scheduled(ଅଚଳ ଅକ୍ଷର *service)
{
	ଫେରନ୍ତୁ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *env)
{
	size_t name_len = strcspn(env, "=");
	ଅକ୍ଷର var[strlen(service) + ଆକାର("/") + name_len];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ଯଦି (env[name_len] != '=')
		ଫେରନ୍ତୁ errno = EINVAL, false;

	snprintf(var, ଆକାର(var), "%s/%.*s", service, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) name_len, env);

	ଯଦି (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ଫେରନ୍ତୁ false;

	ଯଦି (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ଫେରନ୍ତୁ false;

	fputs(&env[name_len + 1], fp);

	ଫେରନ୍ତୁ fclose(fp) == 0;
}

bool
rc_service_setenv(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ଅକ୍ଷର var[strlen(service) + ଆକାର("/") + strlen(name)];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, ଆକାର(var), "%s/%s", service, name);

	ଯଦି (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ଫେରନ୍ତୁ false;

	ଯଦି (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ଫେରନ୍ତୁ false;

	fputs(ମୂଲ୍ୟ, fp);

	ଫେରନ୍ତୁ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *name, ଅକ୍ଷର **buf)
{
	ଅକ୍ଷର var[strlen(service) + ଆକାର("/") + strlen(name)], *ମୂଲ୍ୟ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dfd = rc_dirfd(RC_DIR_ENVIRON);
	ସଂରଚନା stat stat;
	size_t ଗଣନା_2;
	FILE *fp;

	snprintf(var, ଆକାର(var), "%s/%s", service, name);

	ଯଦି (fstatat(dfd, var, &stat, 0) == -1)
		ଫେରନ୍ତୁ -1;

	ଯଦି (!buf)
		ଫେରନ୍ତୁ stat.st_size;

	ଯଦି (!(ମୂଲ୍ୟ = xrealloc(*buf, stat.st_size + 1)))
		ଫେରନ୍ତୁ -1;

	ଯଦି (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ଫେରନ୍ତୁ -1;

	ଗଣନା_2 = fread(ମୂଲ୍ୟ, 1, stat.st_size, fp);
	ମୂଲ୍ୟ[ଗଣନା_2] = '\0';

	fclose(fp);
	*buf = ମୂଲ୍ୟ;
	ଫେରନ୍ତୁ ଗଣନା_2;
}
