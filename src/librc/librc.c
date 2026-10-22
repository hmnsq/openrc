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

#అన్తర్భావ <assert.h>
#అన్తర్భావ <helpers.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <libgen.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <regex.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/param.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "queue.h"
#అన్తర్భావ "librc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "rc.h"
#నిర్వచితమైతే __FreeBSD__
#  include <sys/sysctl.h>
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#అయితే_ముగింపు

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ఖాళీ;
స్థిర పూర్ణాంకం dirfds[RC_DIR_MAX];

పూర్ణాంకం rc_dirfd(గణన rc_dir dir) {
	పూర్ణాంకం flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	పూర్ణాంకం dirfd;

	ఒకవేళ (dir < 0 || dir >= RC_DIR_MAX)
		తిరిగి -1;
	ఒకవేళ (dirfds[dir] <= 0) {
		ఎంపిక (dir) {
		స్థితి RC_DIR_SYSCONF:
			తిరిగి dirfds[dir] = open(rc_sysconfdir(), flags);
		స్థితి RC_DIR_USRCONF:
			తిరిగి dirfds[dir] = open(rc_usrconfdir(), flags);
		స్థితి RC_DIR_RUNLEVEL:
			తిరిగి dirfds[dir] = open(rc_runleveldir(), flags);
		స్థితి RC_DIR_SVCDIR:
			ఒకవేళ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				తిరిగి -1;
			తిరిగి dirfds[dir] = open(rc_svcdir(), flags);
		అప్రమేయ:
			ప్రతిజ్ఞా(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ఒకవేళ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ఒకవేళ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					తిరిగి -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			ఆపు;
		}
	}
	తిరిగి dirfds[dir];
}

శూన్య clear_dirfds(శూన్య) {
	కొరకు (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ఒకవేళ (dirfds[i] <= 0)
			కొనసాగు;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#నిర్వచించు LS_INITD	0x01
#నిర్వచించు LS_DIR		0x02
స్థిర RC_STRINGLIST *
ls_dir(పూర్ణాంకం targetfd, స్థిరరాశి అక్షరం *dir, పూర్ణాంకం options)
{
	DIR *dp;
	నిర్మాణం dirent *d;
	RC_STRINGLIST *list = ఖాళీ;
	నిర్మాణం stat buf;
	size_t l;

	list = rc_stringlist_new();

	ఒకవేళ (!(dp = do_opendirat(targetfd, dir)))
		తిరిగి list;

	అయితే (((d = readdir(dp)) != ఖాళీ)) {
		ఒకవేళ (d->d_name[0] == '.')
			కొనసాగు;
		ఒకవేళ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ఒకవేళ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				కొనసాగు;

			/* .sh files are not init scripts */
			l = అక్షరశ్రేణి_పొడవు(d->d_name);
			ఒకవేళ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				కొనసాగు;
		}
		ఒకవేళ (options & LS_DIR) {
			ఒకవేళ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				కొనసాగు;
		}
		rc_stringlist_కూడు(list, d->d_name);
	}
	closedir(dp);
	తిరిగి list;
}

స్థిర bool
rm_dir(పూర్ణాంకం targetfd, స్థిరరాశి అక్షరం *pathname, bool శిఖరం)
{
	bool retval = true;
	నిర్మాణం dirent *d;
	నిర్మాణం stat s;
	DIR *dp;

	ఒకవేళ (!(dp = do_opendirat(targetfd, pathname)))
		తిరిగి false;

	అయితే ((d = readdir(dp))) {
		ఒకవేళ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			కొనసాగు;
		ఒకవేళ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			ఆపు;
		}
		ఒకవేళ (S_ISDIR(s.st_mode)) {
			ఒకవేళ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				ఆపు;
			}
		} లేకపోతే {
			ఒకవేళ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				ఆపు;
			}
		}
	}
	closedir(dp);

	ఒకవేళ (!retval)
		తిరిగి false;

	ఒకవేళ (శిఖరం && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		తిరిగి false;

	తిరిగి true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#నిర్వచితమైతే __linux__
స్థిర bool
file_regex(స్థిరరాశి అక్షరం *file, స్థిరరాశి అక్షరం *regex)
{
	FILE *fp;
	అక్షరం *పంక్తి = ఖాళీ;
	size_t కొలత = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	పూర్ణాంకం ఫలితం;

	ఒకవేళ (!(fp = fopen(file, "r")))
		తిరిగి false;

	ఒకవేళ ((ఫలితం = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		అక్షరం buf[BUFSIZ];
		fclose(fp);
		regerror(ఫలితం, &re, buf, పరిమాణం(buf));
		fprintf(stderr, "file_regex: %s", buf);
		తిరిగి false;
	}

	అయితే ((len = xgetline(&పంక్తి, &కొలత, fp)) != -1) {
		అక్షరం *str = పంక్తి;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		చేయి {
			ఒకవేళ (regexec(&re, str, 0, ఖాళీ, 0) == 0)
				వెళ్ళు దొరికింది;
			str += అక్షరశ్రేణి_పొడవు(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			అయితే (str < పంక్తి + len && *str == '\0')
				str++;
		} అయితే (str < పంక్తి + len);
	}
	retval = false;
దొరికింది:
	fclose(fp);
	విడుదల(పంక్తి);
	regfree(&re);

	తిరిగి retval;
}
#అయితే_ముగింపు


స్థిర స్థిరరాశి అక్షరం *
get_systype(శూన్య)
{
	అక్షరం *systype = rc_conf_విలువ("rc_sys");
	ఒకవేళ (systype) {
		అక్షరం *s = systype;
		/* Convert to uppercase */
		అయితే (s && *s) {
			ఒకవేళ (islower((గుర్తులేని అక్షరం) *s))
				*s = పెద్ద_అక్షరం((గుర్తులేని అక్షరం) *s);
			s++;
		}
	}
	తిరిగి systype;
}

స్థిర స్థిరరాశి అక్షరం *
detect_container(స్థిరరాశి అక్షరం *systype RC_వాడని)
{
#నిర్వచితమైతే __FreeBSD__
	ఒకవేళ (systype) {
		ఒకవేళ (strcmp(systype, RC_SYS_NONE) == 0)
		       తిరిగి ఖాళీ;
		ఒకవేళ (strcmp(systype, RC_SYS_JAIL) == 0)
			తిరిగి RC_SYS_JAIL;
		ఒకవేళ (strcmp(systype, RC_SYS_PODMAN) == 0)
			తిరిగి RC_SYS_PODMAN;
	}

	పూర్ణాంకం jailed = 0;
	size_t len = పరిమాణం(jailed);

	ఒకవేళ (sysctlbyname("security.jail.jailed", &jailed, &len, ఖాళీ, 0) == 0)
		ఒకవేళ (jailed == 1)
			తిరిగి RC_SYS_JAIL;

	ఒకవేళ (access("/var/run/.containerenv", F_OK) == 0)
		తిరిగి RC_SYS_PODMAN;
#అయితే_ముగింపు

#నిర్వచితమైతే __linux__
	ఒకవేళ (systype) {
		ఒకవేళ (strcmp(systype, RC_SYS_NONE) == 0)
			తిరిగి ఖాళీ;
		ఒకవేళ (strcmp(systype, RC_SYS_UML) == 0)
			తిరిగి RC_SYS_UML;
		ఒకవేళ (strcmp(systype, RC_SYS_VSERVER) == 0)
			తిరిగి RC_SYS_VSERVER;
		ఒకవేళ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			తిరిగి RC_SYS_OPENVZ;
		ఒకవేళ (strcmp(systype, RC_SYS_LXC) == 0)
			తిరిగి RC_SYS_LXC;
		ఒకవేళ (strcmp(systype, RC_SYS_RKT) == 0)
				తిరిగి RC_SYS_RKT;
		ఒకవేళ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				తిరిగి RC_SYS_SYSTEMD_NSPAWN;
		ఒకవేళ (strcmp(systype, RC_SYS_DOCKER) == 0)
				తిరిగి RC_SYS_DOCKER;
		ఒకవేళ (strcmp(systype, RC_SYS_PODMAN) == 0)
			తిరిగి RC_SYS_PODMAN;
		ఒకవేళ (strcmp(systype, RC_SYS_WSL) == 0)
			తిరిగి RC_SYS_WSL;
	}
	ఒకవేళ (file_regex("/proc/cpuinfo", "UML"))
		తిరిగి RC_SYS_UML;
	లేకపోతే ఒకవేళ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		తిరిగి RC_SYS_VSERVER;
	లేకపోతే ఒకవేళ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		తిరిగి RC_SYS_OPENVZ;
	లేకపోతే ఒకవేళ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		తిరిగి RC_SYS_OPENVZ; /* old test */
	లేకపోతే ఒకవేళ (file_regex("/proc/1/environ", "container=lxc"))
		తిరిగి RC_SYS_LXC;
	లేకపోతే ఒకవేళ (file_regex("/proc/1/environ", "container=rkt"))
		తిరిగి RC_SYS_RKT;
	లేకపోతే ఒకవేళ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		తిరిగి RC_SYS_SYSTEMD_NSPAWN;
	లేకపోతే ఒకవేళ (access("/run/.containerenv", F_OK) == 0)
		తిరిగి RC_SYS_PODMAN;
	లేకపోతే ఒకవేళ (access("/.dockerenv", F_OK) == 0)
		తిరిగి RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	లేకపోతే ఒకవేళ (file_regex("/proc/1/environ", "container=docker"))
		తిరిగి RC_SYS_DOCKER;
	లేకపోతే ఒకవేళ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		తిరిగి RC_SYS_WSL;
	లేకపోతే ఒకవేళ (getenv("WSL_DISTRO_NAME"))
		తిరిగి RC_SYS_WSL;
	లేకపోతే ఒకవేళ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		తిరిగి RC_SYS_WSL;
#అయితే_ముగింపు

	తిరిగి ఖాళీ;
}

స్థిర స్థిరరాశి అక్షరం *
detect_vm(స్థిరరాశి అక్షరం *systype RC_వాడని)
{
#నిర్వచితమైతే __NetBSD__
	ఒకవేళ (systype) {
		ఒకవేళ (strcmp(systype, RC_SYS_NONE) == 0)
			తిరిగి ఖాళీ;
		ఒకవేళ (strcmp(systype, RC_SYS_XEN0) == 0)
			తిరిగి RC_SYS_XEN0;
		ఒకవేళ (strcmp(systype, RC_SYS_XENU) == 0)
			తిరిగి RC_SYS_XENU;
	}
	ఒకవేళ (access("/kern/xen/privcmd", F_OK) == 0)
		తిరిగి RC_SYS_XEN0;
	ఒకవేళ (access("/kern/xen", F_OK) == 0)
		తిరిగి RC_SYS_XENU;
#అయితే_ముగింపు

#నిర్వచితమైతే __linux__
	ఒకవేళ (systype) {
		ఒకవేళ (strcmp(systype, RC_SYS_NONE) == 0)
			తిరిగి ఖాళీ;
		ఒకవేళ (strcmp(systype, RC_SYS_XEN0) == 0)
			తిరిగి RC_SYS_XEN0;
		ఒకవేళ (strcmp(systype, RC_SYS_XENU) == 0)
			తిరిగి RC_SYS_XENU;
	}
	ఒకవేళ (access("/proc/xen", F_OK) == 0) {
		ఒకవేళ (file_regex("/proc/xen/capabilities", "control_d"))
			తిరిగి RC_SYS_XEN0;
		తిరిగి RC_SYS_XENU;
	}
#అయితే_ముగింపు

	తిరిగి ఖాళీ;
}

స్థిరరాశి అక్షరం *
rc_sys(శూన్య)
{
	స్థిరరాశి అక్షరం *systype;
	స్థిరరాశి అక్షరం *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ఒకవేళ (!sys) {
		sys = detect_vm(systype);
	}

	తిరిగి sys;
}

స్థిర స్థిరరాశి అక్షరం *
rc_parse_service_state(RC_SERVICE state)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; rc_service_state_పేర్లు[i].name; i++) {
		ఒకవేళ (rc_service_state_పేర్లు[i].state == state)
			తిరిగి rc_service_state_పేర్లు[i].name;
	}
	తిరిగి ఖాళీ;
}

స్థిర పూర్ణాంకం
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	కొరకు (size_t i = 0; rc_service_state_పేర్లు[i].name; i++)
		ఒకవేళ (rc_service_state_పేర్లు[i].state == state)
			తిరిగి rc_service_state_పేర్లు[i].dir;
	తిరిగి -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
స్థిర శూన్య
get_runlevel_chain(స్థిరరాశి అక్షరం *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *మాతృ;
	స్థిరరాశి అక్షరం *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	ఒకవేళ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		తిరిగి;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ఒకవేళ (rc_stringlist_కనుగొను(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_కూడు(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->విలువ;

		/* Check for loop */
		ఒకవేళ (rc_stringlist_కనుగొను(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(మాతృ, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", మాతృ->విలువ);
			నిష్క్రమించు(1);
		}

		/* Add new ancestor */
		rc_stringlist_కూడు(ancestor_list, nextlevel);

		get_runlevel_chain(nextlevel, level_list, ancestor_list);

		rc_stringlist_delete(ancestor_list, nextlevel);
	}
	rc_stringlist_free(dirs);
}

bool
rc_runlevel_starting(శూన్య)
{
	తిరిగి faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(శూన్య)
{
	తిరిగి faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(శూన్య)
{
	తిరిగి ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

అక్షరం *
rc_runlevel_get(శూన్య)
{
	FILE *fp;
	అక్షరం *runlevel = ఖాళీ;
	size_t i;

	ఒకవేళ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(పరిమాణం(అక్షరం) * PATH_MAX);
		ఒకవేళ (fgets(runlevel, PATH_MAX, fp)) {
			i = అక్షరశ్రేణి_పొడవు(runlevel);
			ఒకవేళ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} లేకపోతే
			*runlevel = '\0';
		fclose(fp);
	}

	ఒకవేళ (!runlevel || !*runlevel) {
		విడుదల(runlevel);
		runlevel = ఖాళీ;
	}

	తిరిగి runlevel;
}

bool
rc_runlevel_సమితి(స్థిరరాశి అక్షరం *runlevel)
{
	FILE *fp;

	ఒకవేళ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		తిరిగి false;

	fputs(runlevel, fp);
	fclose(fp);
	తిరిగి true;
}

bool
rc_runlevel_exists(స్థిరరాశి అక్షరం *runlevel)
{
	నిర్మాణం stat buf;

	ఒకవేళ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		తిరిగి false;
	ఒకవేళ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		తిరిగి true;
	తిరిగి false;
}

bool
rc_runlevel_దొంతర(స్థిరరాశి అక్షరం *dst, స్థిరరాశి అక్షరం *src)
{
	అక్షరం *d, *s;
	పూర్ణాంకం r;

	ఒకవేళ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		తిరిగి false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	విడుదల(d);
	విడుదల(s);
	తిరిగి (r == 0);
}

bool
rc_runlevel_unstack(స్థిరరాశి అక్షరం *dst, స్థిరరాశి అక్షరం *src)
{
	అక్షరం *మార్గం;
	పూర్ణాంకం r;

	xasprintf(&మార్గం, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(మార్గం);
	విడుదల(మార్గం);
	తిరిగి (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(స్థిరరాశి అక్షరం *runlevel)
{
	RC_STRINGLIST *దొంతర, *ancestor_list;
	దొంతర = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_కూడు(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, దొంతర, ancestor_list);
	rc_stringlist_free(ancestor_list);
	తిరిగి దొంతర;
}

స్థిర స్థిరరాశి అక్షరం *config_మార్గం[] = { RC_మార్గం, ఖాళీ };

స్థిర నిర్మాణం {
	bool సమితి;
	అక్షరం *svcdir, *confdir, *runleveldir;
} rc_dirs;

స్థిర నిర్మాణం {
	అక్షరం *బఫరు;
	size_t లెక్క;
	పూర్ణాంకం *fds;
	స్థిరరాశి అక్షరం **entries, **allocd;
} rc_మార్గం;

స్థిర శూన్య
free_rc_dirs(శూన్య)
{
#నిర్వచించు X(ptr) ptr = (విడుదల(ptr), ఖాళీ)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_మార్గం.allocd);
	/* entries alias freed allocation */
	rc_మార్గం.entries = ఖాళీ;
	X(rc_మార్గం.బఫరు);
	X(rc_మార్గం.fds);
	rc_మార్గం.లెక్క = 0;
#నిర్వచనం_రద్దు X
}

__attribute__((__format__(__printf__, 2, 3)))
స్థిర శూన్య
append_మార్గం(FILE *fp, స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_మార్గం.లెక్క++;
}

స్థిర స్థిరరాశి అక్షరం *
strnsep(స్థిరరాశి అక్షరం **str, size_t *కొలత, స్థిరరాశి అక్షరం sep)
{
	స్థిరరాశి అక్షరం *ret = *str;

	ఒకవేళ (!ret)
		తిరిగి ఖాళీ;

	*str = strchrnul(ret, sep);
	*కొలత = *str - ret;
	ఒకవేళ (!**str)
		*str = ఖాళీ;
	లేకపోతే
		*str += 1;

	తిరిగి ret;
}

స్థిర స్థిరరాశి అక్షరం *
strnullsep(స్థిరరాశి అక్షరం **str, size_t *కొలత)
{
	స్థిరరాశి అక్షరం *ret = *str;
	size_t len;

	ఒకవేళ (!ret)
		తిరిగి ఖాళీ;

	len = strnlen(ret, *కొలత - 1) + 1;

	ఒకవేళ ((*కొలత -= len))
		*str += len;
	లేకపోతే
		*str = ఖాళీ;

	తిరిగి ret;
}

స్థిర శూన్య
append_user_మార్గం(FILE *fp)
{
	స్థిరరాశి అక్షరం *xdg, *నమోదు_2;
	size_t కొలత;

	append_మార్గం(fp, "%s", rc_dirs.confdir);
	ఒకవేళ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_మార్గం(fp, "/etc/xdg/rc");
	లేకపోతే అయితే ((నమోదు_2 = strnsep(&xdg, &కొలత, ':')))
		ఒకవేళ (కొలత) append_మార్గం(fp, "%.*s/rc", (పూర్ణాంకం) కొలత, నమోదు_2);

	కొరకు (size_t i = 0; i < ARRAY_కొలత(config_మార్గం) - 2; i++)
		append_మార్గం(fp, "%s/user", config_మార్గం[i]);
	append_మార్గం(fp, "%s", rc_dirs.svcdir);
}

స్థిర శూన్య
append_rc_మార్గం(FILE *fp, స్థిరరాశి అక్షరం *మార్గం, bool user)
{
	bool has_user = false;
	స్థిరరాశి అక్షరం *నమోదు_2;
	size_t కొలత;

	అయితే ((నమోదు_2 = strnsep(&మార్గం, &కొలత, ':'))) {
		ఒకవేళ (కొలత)
			append_మార్గం(fp, "%.*s%s", (పూర్ణాంకం)కొలత, నమోదు_2, user ? "/user" : "");
		లేకపోతే ఒకవేళ (user && !has_user)
			append_user_మార్గం(fp), has_user = true;
		లేకపోతే ఒకవేళ (!user) అయితే (*rc_మార్గం.entries)
			append_మార్గం(fp, "%s", *rc_మార్గం.entries++);
	}
}

స్థిర శూన్య
ఎక్కించు_dynamic_మార్గం(bool user)
{
	స్థిరరాశి అక్షరం *మార్గం, *నమోదు_2;
	size_t కొలత, లెక్క = 0;
	FILE *fp;

	ఒకవేళ (!rc_dirs.సమితి) {
		atexit(free_rc_dirs);
		rc_dirs.సమితి = true;
	}

	rc_మార్గం.entries = config_మార్గం;
	rc_మార్గం.లెక్క = ARRAY_కొలత(config_మార్గం) - 1;

	ఒకవేళ (!(మార్గం = getenv("RC_PATH")) && !user)
		తిరిగి;

	rc_మార్గం.లెక్క = 0;
	fp = xopen_memstream(&rc_మార్గం.బఫరు, &కొలత);
	ఒకవేళ (మార్గం)
		append_rc_మార్గం(fp, మార్గం, user);
	లేకపోతే
		append_user_మార్గం(fp);
	xclose_memstream(fp);

	rc_మార్గం.allocd = rc_మార్గం.entries = సున్నా_జ్ఞాపకం_ఇవ్వు(rc_మార్గం.లెక్క + 1, పరిమాణం(*rc_మార్గం.entries));

	మార్గం = rc_మార్గం.బఫరు;

	అయితే ((నమోదు_2 = strnullsep(&మార్గం, &కొలత)) && లెక్క < rc_మార్గం.లెక్క)
		rc_మార్గం.entries[లెక్క++] = నమోదు_2;
	rc_మార్గం.entries[rc_మార్గం.లెక్క] = ఖాళీ;
}

స్థిర bool is_user = false;

bool
rc_is_user(శూన్య)
{
	పూర్ణాంకం _is_user = -1;

	ఒకవేళ (_is_user == -1) {
		అక్షరం *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	తిరిగి _is_user;
}

శూన్య
rc_సమితి_user(శూన్య)
{
	అక్షరం *env;

	ఒకవేళ (is_user)
		తిరిగి;

	ఒకవేళ (!rc_dirs.సమితి)
		atexit(free_rc_dirs);
	లేకపోతే
		free_rc_dirs();

	is_user = true;
	rc_dirs.సమితి = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ఒకవేళ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} లేకపోతే ఒకవేళ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} లేకపోతే {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		నిష్క్రమించు(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ఒకవేళ (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		నిష్క్రమించు(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	ఎక్కించు_dynamic_మార్గం(true);

	clear_dirfds();
}

స్థిరరాశి అక్షరం * స్థిరరాశి *
rc_scriptdirs(శూన్య)
{
	ఒకవేళ (!rc_మార్గం.entries)
		ఎక్కించు_dynamic_మార్గం(false);
	తిరిగి rc_మార్గం.entries;
}

size_t
rc_scriptdirfds(స్థిరరాశి పూర్ణాంకం **fds)
{
	స్థిర size_t len = 0;

	ఒకవేళ (len == 0) {
		ఒకవేళ (!rc_మార్గం.entries)
			ఎక్కించు_dynamic_మార్గం(false);

		rc_మార్గం.fds = xmalloc(rc_మార్గం.లెక్క * పరిమాణం(*rc_మార్గం.fds));
		కొరకు (size_t i = 0; i < rc_మార్గం.లెక్క; i++) {
			పూర్ణాంకం fd = open(rc_మార్గం.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ఒకవేళ (fd == -1)
				కొనసాగు;

			rc_మార్గం.fds[len++] = fd;
		}
	}

	*fds = rc_మార్గం.fds;
	తిరిగి len;
}

స్థిరరాశి అక్షరం *
rc_sysconfdir(శూన్య)
{
	తిరిగి RC_SYSCONFDIR;
}

స్థిరరాశి అక్షరం *
rc_usrconfdir(శూన్య)
{
	తిరిగి rc_dirs.confdir ? rc_dirs.confdir : ఖాళీ;
}

స్థిరరాశి అక్షరం *
rc_runleveldir(శూన్య)
{
	తిరిగి rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

స్థిరరాశి అక్షరం *
rc_svcdir(శూన్య)
{
	స్థిర స్థిరరాశి అక్షరం *svcdir;

	ఒకవేళ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	తిరిగి rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

స్థిర ssize_t
సురక్షిత_readlink(పూర్ణాంకం targetfd, స్థిరరాశి అక్షరం *మార్గం, అక్షరం **buf, size_t bufsiz)
{
	అక్షరం *బఫరు;
	ssize_t r;

	కొరకు (;; bufsiz += PATH_MAX) {
		బఫరు = xmalloc(bufsiz);
		r = readlinkat(targetfd, మార్గం, బఫరు, bufsiz);
		ఒకవేళ (r < 0) {
			విడుదల(బఫరు);
			తిరిగి -errno;
		}
		ఒకవేళ ((size_t)r < bufsiz) {
			బఫరు[r] = '\0';
			*buf = బఫరు;
			తిరిగి r;
		}
		విడుదల(బఫరు);
		ఒకవేళ (bufsiz >= SSIZE_MAX - PATH_MAX)
			తిరిగి -ENOMEM;
	}
}

/* Resolve a service name to its full path */
అక్షరం *
rc_service_resolve(స్థిరరాశి అక్షరం *service)
{
	అక్షరం *బఫరు;
	నిర్మాణం stat buf;

	ఒకవేళ (!service)
		తిరిగి ఖాళీ;

	ఒకవేళ (service[0] == '/')
		తిరిగి xstrdup(service);

	/* First check started services */
	ఒకవేళ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ఒకవేళ (సురక్షిత_readlink(rc_dirfd(RC_DIR_STARTED), service, &బఫరు, buf.st_size + 1) >= 0)
			తిరిగి బఫరు;
	ఒకవేళ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ఒకవేళ (సురక్షిత_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &బఫరు, buf.st_size + 1) >= 0)
			తిరిగి బఫరు;

	కొరకు (స్థిరరాశి అక్షరం * స్థిరరాశి *dirs = rc_scriptdirs(); *dirs; dirs++) {
		అక్షరం *file = ఖాళీ;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ఒకవేళ (stat(file, &buf) == 0)
			తిరిగి file;
		విడుదల(file);
	}

	తిరిగి ఖాళీ;
}

bool
rc_service_exists(స్థిరరాశి అక్షరం *service)
{
	అక్షరం *file;
	bool retval = false;
	size_t len;
	నిర్మాణం stat buf;

	ఒకవేళ (!service) {
		errno = EINVAL;
		తిరిగి false;
	}

	len = అక్షరశ్రేణి_పొడవు(service);

	/* .sh files are not init scripts */
	ఒకవేళ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		తిరిగి false;
	}

	ఒకవేళ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		తిరిగి false;
	}

	ఒకవేళ (stat(file, &buf) == 0) {
		ఒకవేళ (buf.st_mode & S_IXUGO)
			retval = true;
		లేకపోతే
			errno = ENOEXEC;
	}
	విడుదల(file);
	తిరిగి retval;
}

#నిర్వచించు OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(స్థిరరాశి అక్షరం *service)
{
	అక్షరం *svc;
	అక్షరం *cmd = ఖాళీ;
	అక్షరం *బఫరు = ఖాళీ;
	size_t len = 0;
	RC_STRINGLIST *commands = ఖాళీ;
	అక్షరం *token;
	అక్షరం *p;
	FILE *fp;

	ఒకవేళ (!(svc = rc_service_resolve(service)))
		తిరిగి ఖాళీ;

	xasprintf(&cmd, OPTSTR, svc);
	విడుదల(svc);

	ఒకవేళ (!(fp = popen(cmd, "r"))) {
		విడుదల(cmd);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (xgetline(&బఫరు, &len, fp) != -1) {
		p = బఫరు;
		commands = rc_stringlist_new();

		అయితే ((token = strsep(&p, " ")))
			ఒకవేళ (token[0] != '\0')
				rc_stringlist_కూడు(commands, token);
	}

	pclose(fp);
	విడుదల(బఫరు);
	విడుదల(cmd);
	తిరిగి commands;
}

#నిర్వచించు DESCSTR ". '%s'; echo \"${description%s%s}\""
అక్షరం *
rc_service_description(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *option)
{
	అక్షరం *svc;
	అక్షరం *cmd;
	అక్షరం *desc = ఖాళీ;
	size_t కొలత = 0;
	FILE *fp;

	ఒకవేళ (!(svc = rc_service_resolve(service)))
		తిరిగి ఖాళీ;

	ఒకవేళ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	విడుదల(svc);
	ఒకవేళ ((fp = popen(cmd, "r"))) {
		ఒకవేళ (xgetline(&desc, &కొలత, fp) == -1) {
			విడుదల(desc);
			desc = ఖాళీ;
		}
		pclose(fp);
	}
	విడుదల(cmd);
	తిరిగి desc;
}

bool
rc_service_లోపల_runlevel(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *runlevel)
{
	అక్షరం *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	విడుదల(file);
	తిరిగి r;
}

bool
rc_service_mark(స్థిరరాశి అక్షరం *service, స్థిరరాశి RC_SERVICE state)
{
	పూర్ణాంకం i = 0;
	స్థిరరాశి అక్షరం *పునాది;
	అక్షరం *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	పూర్ణాంకం serrno;

	ఒకవేళ (!init)
		తిరిగి false;

	పునాది = basename_c(service);
	ఒకవేళ (state != RC_SERVICE_STOPPED) {
		పూర్ణాంకం state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ఒకవేళ (access(init, F_OK) != 0) {
			విడుదల(init);
			తిరిగి false;
		}

		unlinkat(state_dirfd, పునాది, 0);
		ఒకవేళ ((symlinkat(init, state_dirfd, పునాది)) != 0) {
			విడుదల(init);
			తిరిగి false;
		}
	}

	ఒకవేళ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		విడుదల(init);
		తిరిగి true;
	}

	/* Remove any old states now */
	కొరకు (i = 0; rc_service_state_పేర్లు[i].name; i++) {
		పూర్ణాంకం state_dirfd = rc_dirfd(rc_service_state_పేర్లు[i].dir);
		RC_SERVICE s = rc_service_state_పేర్లు[i].state;

		ఒకవేళ (s == state)
			కొనసాగు;

		ఎంపిక (s) {
		స్థితి RC_SERVICE_STOPPED:
		స్థితి RC_SERVICE_HOTPLUGGED:
		స్థితి RC_SERVICE_SCHEDULED:
			కొనసాగు;
		స్థితి RC_SERVICE_WASINACTIVE:
			ఒకవేళ (skip_wasinactive)
				కొనసాగు;
			/* fall-through */
		అప్రమేయ:
			ఒకవేళ (faccessat(state_dirfd, పునాది, F_OK, 0) != 0)
				కొనసాగు;
			ఒకవేళ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ఒకవేళ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), పునాది) == -1) {
					విడుదల(init);
					తిరిగి false;
				}
				skip_wasinactive = true;
			}
			ఒకవేళ (unlinkat(state_dirfd, పునాది, 0) == -1) {
				విడుదల(init);
				తిరిగి false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ఒకవేళ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), పునాది, 0);

	/* Remove any options and daemons the service may have stored */
	ఒకవేళ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), పునాది, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), పునాది, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), పునాది, 0);
		rc_service_కాలపట్టిక_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ఒకవేళ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ఒకవేళ (dp) {
			నిర్మాణం dirent *d;
			అయితే ((d = readdir(dp))) {
				అక్షరం *was;

				xasprintf(&was, "%s/%s", d->d_name, పునాది);
				unlinkat(dirfd(dp), was, 0);
				విడుదల(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	విడుదల(init);
	తిరిగి true;
}

RC_SERVICE
rc_service_state(స్థిరరాశి అక్షరం *service)
{
	స్థిరరాశి అక్షరం *పునాది = basename_c(service);
	పూర్ణాంకం state = RC_SERVICE_STOPPED;
	పూర్ణాంకం i;

	కొరకు (i = 0; rc_service_state_పేర్లు[i].name; i++) {
		ఒకవేళ (faccessat(rc_dirfd(rc_service_state_పేర్లు[i].dir), పునాది, F_OK, 0) == 0) {
			ఒకవేళ (rc_service_state_పేర్లు[i].state <= 0x10)
				state = rc_service_state_పేర్లు[i].state;
			లేకపోతే
				state |= rc_service_state_పేర్లు[i].state;
		}
	}

	ఒకవేళ (state & RC_SERVICE_STARTED)
		ఒకవేళ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ఒకవేళ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ఒకవేళ (dp) {
			నిర్మాణం dirent *d;
			అయితే ((d = readdir(dp))) {
				ఒకవేళ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					ఆపు;
				}
			}

			closedir(dp);
		}
	}

	తిరిగి state;
}

అక్షరం *
rc_service_విలువ_get(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *option)
{
	అక్షరం *బఫరు = ఖాళీ;
	size_t len = 0;
	అక్షరం *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &బఫరు, &len);
	విడుదల(file);

	తిరిగి బఫరు;
}

bool
rc_service_విలువ_సమితి(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *option, స్థిరరాశి అక్షరం *విలువ)
{
	bool ret = true;
	పూర్ణాంకం optdirfd;
	FILE *fp;

	ఒకవేళ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		తిరిగి false;

	ఒకవేళ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		తిరిగి false;

	ఒకవేళ (!విలువ) {
		unlinkat(optdirfd, option, 0);
	} లేకపోతే ఒకవేళ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", విలువ);
		fclose(fp);
	} లేకపోతే {
		ret = false;
	}

	close(optdirfd);
	తిరిగి ret;
}

bool
rc_service_కాలపట్టిక_ప్రారంభం(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *service_గమ్యం_ప్రారంభం)
{
	స్థిరరాశి అక్షరం *పునాది_గమ్యం_ప్రారంభం = basename_c(service_గమ్యం_ప్రారంభం);
	స్థిరరాశి అక్షరం *పునాది = basename_c(service);
	bool retval;
	పూర్ణాంకం schedfd;
	అక్షరం *init;

	/* service may be a provided service, like net */
	ఒకవేళ (!service || !rc_service_exists(service_గమ్యం_ప్రారంభం))
		తిరిగి false;

	ఒకవేళ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), పునాది, 0755) != 0 && errno != EEXIST)
		తిరిగి false;

	ఒకవేళ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), పునాది, O_RDONLY)) == -1)
		తిరిగి false;

	init = rc_service_resolve(service_గమ్యం_ప్రారంభం);

	retval = (faccessat(schedfd, పునాది_గమ్యం_ప్రారంభం, F_OK, 0) == 0) || (symlinkat(init, schedfd, పునాది_గమ్యం_ప్రారంభం) == 0);

	close(schedfd);
	విడుదల(init);
	తిరిగి retval;
}

bool
rc_service_కాలపట్టిక_clear(స్థిరరాశి అక్షరం *service)
{
	తిరిగి !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_లోపల_runlevel(స్థిరరాశి అక్షరం *runlevel)
{
	RC_STRINGLIST *list = ఖాళీ;

	ఒకవేళ (!runlevel) {
		స్థిరరాశి పూర్ణాంకం *scriptdirfds;
		list = rc_stringlist_new();
		కొరకు (size_t i = 0, లెక్క = rc_scriptdirfds(&scriptdirfds); i < లెక్క; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		తిరిగి list;
	}

	/* These special levels never contain any services */
	ఒకవేళ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		తిరిగి rc_stringlist_new();

	తిరిగి ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_లోపల_runlevel_stacked(స్థిరరాశి అక్షరం *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *దొంతర;

	list = rc_services_లోపల_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(దొంతర, stacks, entries) {
		sl = rc_services_లోపల_runlevel(దొంతర->విలువ);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	తిరిగి list;
}

RC_STRINGLIST *
rc_services_లోపల_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	నిర్మాణం dirent *d;
	DIR *dp;

	ఒకవేళ (state != RC_SERVICE_SCHEDULED)
		తిరిగి ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ఒకవేళ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		తిరిగి list;

	అయితే ((d = readdir(dp))) {
		ఒకవేళ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			కొనసాగు;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	తిరిగి list;
}

bool
rc_service_కూడు(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *service)
{
	bool retval;
	అక్షరం *init;
	అక్షరం *file;
	అక్షరం *మార్గం;
	అక్షరం *binit = ఖాళీ;
	అక్షరం *i;

	ఒకవేళ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		తిరిగి false;
	}

	ఒకవేళ (rc_service_లోపల_runlevel(service, runlevel)) {
		errno = EEXIST;
		తిరిగి false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ఒకవేళ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		మార్గం = realpath(dirname(init), ఖాళీ);
		ఒకవేళ (మార్గం == ఖాళీ) {
			విడుదల(init);
			తిరిగి false;
		}
		retval = (strcmp(మార్గం, RC_INITDIR) == 0);
		విడుదల(మార్గం);
		ఒకవేళ (!retval) {
			విడుదల(init);
			errno = EPERM;
			తిరిగి false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ఒకవేళ (అక్షరశ్రేణి_పోలిక(i, rc_svcdir(), అక్షరశ్రేణి_పొడవు(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		వెళ్ళు బహిర్గతం;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	విడుదల(file);

బహిర్గతం:
	విడుదల(binit);
	విడుదల(init);
	తిరిగి retval;
}

bool
rc_service_delete(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *service)
{
	అక్షరం *file;
	పూర్ణాంకం r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	విడుదల(file);
	తిరిగి (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(స్థిరరాశి అక్షరం *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	నిర్మాణం dirent *d;
	DIR *dp;

	ఒకవేళ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		తిరిగి list;

	అయితే ((d = readdir(dp))) {
		అక్షరం *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ఒకవేళ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_కూడు(list, d->d_name);
		విడుదల(file);
	}

	తిరిగి list;
}

RC_STRINGLIST *
rc_services_scheduled(స్థిరరాశి అక్షరం *service)
{
	తిరిగి ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *env)
{
	size_t name_len = strcspn(env, "=");
	అక్షరం var[అక్షరశ్రేణి_పొడవు(service) + పరిమాణం("/") + name_len];
	పూర్ణాంకం dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ఒకవేళ (env[name_len] != '=')
		తిరిగి errno = EINVAL, false;

	snprintf(var, పరిమాణం(var), "%s/%.*s", service, (పూర్ణాంకం) name_len, env);

	ఒకవేళ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		తిరిగి false;

	ఒకవేళ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		తిరిగి false;

	fputs(&env[name_len + 1], fp);

	తిరిగి fclose(fp) == 0;
}

bool
rc_service_setenv(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *విలువ)
{
	అక్షరం var[అక్షరశ్రేణి_పొడవు(service) + పరిమాణం("/") + అక్షరశ్రేణి_పొడవు(name)];
	పూర్ణాంకం dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, పరిమాణం(var), "%s/%s", service, name);

	ఒకవేళ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		తిరిగి false;

	ఒకవేళ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		తిరిగి false;

	fputs(విలువ, fp);

	తిరిగి fclose(fp) == 0;
}

ssize_t
rc_service_getenv(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *name, అక్షరం **buf)
{
	అక్షరం var[అక్షరశ్రేణి_పొడవు(service) + పరిమాణం("/") + అక్షరశ్రేణి_పొడవు(name)], *విలువ;
	పూర్ణాంకం dfd = rc_dirfd(RC_DIR_ENVIRON);
	నిర్మాణం stat stat;
	size_t లెక్క;
	FILE *fp;

	snprintf(var, పరిమాణం(var), "%s/%s", service, name);

	ఒకవేళ (fstatat(dfd, var, &stat, 0) == -1)
		తిరిగి -1;

	ఒకవేళ (!buf)
		తిరిగి stat.st_size;

	ఒకవేళ (!(విలువ = xrealloc(*buf, stat.st_size + 1)))
		తిరిగి -1;

	ఒకవేళ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		తిరిగి -1;

	లెక్క = fread(విలువ, 1, stat.st_size, fp);
	విలువ[లెక్క] = '\0';

	fclose(fp);
	*buf = విలువ;
	తిరిగి లెక్క;
}
