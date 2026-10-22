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

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <helpers.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <libgen.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <regex.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/param.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "librc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ശൂന്യ;
സ്ഥിതം പൂർണ്ണസംഖ്യ dirfds[RC_DIR_MAX];

പൂർണ്ണസംഖ്യ rc_dirfd(എണ്ണൽ rc_dir dir) {
	പൂർണ്ണസംഖ്യ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	പൂർണ്ണസംഖ്യ dirfd;

	എങ്കിൽ (dir < 0 || dir >= RC_DIR_MAX)
		തിരികെ -1;
	എങ്കിൽ (dirfds[dir] <= 0) {
		തിരഞ്ഞെടുപ്പ് (dir) {
		അവസ്ഥ RC_DIR_SYSCONF:
			തിരികെ dirfds[dir] = open(rc_sysconfdir(), flags);
		അവസ്ഥ RC_DIR_USRCONF:
			തിരികെ dirfds[dir] = open(rc_usrconfdir(), flags);
		അവസ്ഥ RC_DIR_RUNLEVEL:
			തിരികെ dirfds[dir] = open(rc_runleveldir(), flags);
		അവസ്ഥ RC_DIR_SVCDIR:
			എങ്കിൽ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				തിരികെ -1;
			തിരികെ dirfds[dir] = open(rc_svcdir(), flags);
		സ്ഥിരസ്ഥിതി:
			പ്രതിജ്ഞാ(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			എങ്കിൽ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				എങ്കിൽ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					തിരികെ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			നിർത്തുക;
		}
	}
	തിരികെ dirfds[dir];
}

ശൂന്യം clear_dirfds(ശൂന്യം) {
	വേണ്ടി (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		എങ്കിൽ (dirfds[i] <= 0)
			തുടരുക;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
സ്ഥിതം RC_STRINGLIST *
ls_dir(പൂർണ്ണസംഖ്യ targetfd, സ്ഥിരം അക്ഷരം *dir, പൂർണ്ണസംഖ്യ options)
{
	DIR *dp;
	ഘടന dirent *d;
	RC_STRINGLIST *list = ശൂന്യ;
	ഘടന stat buf;
	size_t l;

	list = rc_stringlist_new();

	എങ്കിൽ (!(dp = do_opendirat(targetfd, dir)))
		തിരികെ list;

	ആകുമ്പോൾ (((d = readdir(dp)) != ശൂന്യ)) {
		എങ്കിൽ (d->d_name[0] == '.')
			തുടരുക;
		എങ്കിൽ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			എങ്കിൽ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				തുടരുക;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			എങ്കിൽ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				തുടരുക;
		}
		എങ്കിൽ (options & LS_DIR) {
			എങ്കിൽ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				തുടരുക;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	തിരികെ list;
}

സ്ഥിതം bool
rm_dir(പൂർണ്ണസംഖ്യ targetfd, സ്ഥിരം അക്ഷരം *pathname, bool മുകൾ)
{
	bool retval = true;
	ഘടന dirent *d;
	ഘടന stat s;
	DIR *dp;

	എങ്കിൽ (!(dp = do_opendirat(targetfd, pathname)))
		തിരികെ false;

	ആകുമ്പോൾ ((d = readdir(dp))) {
		എങ്കിൽ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			തുടരുക;
		എങ്കിൽ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			നിർത്തുക;
		}
		എങ്കിൽ (S_ISDIR(s.st_mode)) {
			എങ്കിൽ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				നിർത്തുക;
			}
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				നിർത്തുക;
			}
		}
	}
	closedir(dp);

	എങ്കിൽ (!retval)
		തിരികെ false;

	എങ്കിൽ (മുകൾ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		തിരികെ false;

	തിരികെ true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
സ്ഥിതം bool
file_regex(സ്ഥിരം അക്ഷരം *file, സ്ഥിരം അക്ഷരം *regex)
{
	FILE *fp;
	അക്ഷരം *വരി = ശൂന്യ;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	പൂർണ്ണസംഖ്യ result;

	എങ്കിൽ (!(fp = fopen(file, "r")))
		തിരികെ false;

	എങ്കിൽ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		അക്ഷരം buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, വലുപ്പം(buf));
		fprintf(stderr, "file_regex: %s", buf);
		തിരികെ false;
	}

	ആകുമ്പോൾ ((len = xgetline(&വരി, &size, fp)) != -1) {
		അക്ഷരം *str = വരി;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		ചെയ്യുക {
			എങ്കിൽ (regexec(&re, str, 0, ശൂന്യ, 0) == 0)
				പോകുക found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ആകുമ്പോൾ (str < വരി + len && *str == '\0')
				str++;
		} ആകുമ്പോൾ (str < വരി + len);
	}
	retval = false;
found:
	fclose(fp);
	സ്വതന്ത്രം(വരി);
	regfree(&re);

	തിരികെ retval;
}
#endif


സ്ഥിതം സ്ഥിരം അക്ഷരം *
get_systype(ശൂന്യം)
{
	അക്ഷരം *systype = rc_conf_മൂല്യം("rc_sys");
	എങ്കിൽ (systype) {
		അക്ഷരം *s = systype;
		/* Convert to uppercase */
		ആകുമ്പോൾ (s && *s) {
			എങ്കിൽ (islower((unsigned അക്ഷരം) *s))
				*s = toupper((unsigned അക്ഷരം) *s);
			s++;
		}
	}
	തിരികെ systype;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
detect_container(സ്ഥിരം അക്ഷരം *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	എങ്കിൽ (systype) {
		എങ്കിൽ (strcmp(systype, RC_SYS_NONE) == 0)
		       തിരികെ ശൂന്യ;
		എങ്കിൽ (strcmp(systype, RC_SYS_JAIL) == 0)
			തിരികെ RC_SYS_JAIL;
		എങ്കിൽ (strcmp(systype, RC_SYS_PODMAN) == 0)
			തിരികെ RC_SYS_PODMAN;
	}

	പൂർണ്ണസംഖ്യ jailed = 0;
	size_t len = വലുപ്പം(jailed);

	എങ്കിൽ (sysctlbyname("security.jail.jailed", &jailed, &len, ശൂന്യ, 0) == 0)
		എങ്കിൽ (jailed == 1)
			തിരികെ RC_SYS_JAIL;

	എങ്കിൽ (access("/var/run/.containerenv", F_OK) == 0)
		തിരികെ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	എങ്കിൽ (systype) {
		എങ്കിൽ (strcmp(systype, RC_SYS_NONE) == 0)
			തിരികെ ശൂന്യ;
		എങ്കിൽ (strcmp(systype, RC_SYS_UML) == 0)
			തിരികെ RC_SYS_UML;
		എങ്കിൽ (strcmp(systype, RC_SYS_VSERVER) == 0)
			തിരികെ RC_SYS_VSERVER;
		എങ്കിൽ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			തിരികെ RC_SYS_OPENVZ;
		എങ്കിൽ (strcmp(systype, RC_SYS_LXC) == 0)
			തിരികെ RC_SYS_LXC;
		എങ്കിൽ (strcmp(systype, RC_SYS_RKT) == 0)
				തിരികെ RC_SYS_RKT;
		എങ്കിൽ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				തിരികെ RC_SYS_SYSTEMD_NSPAWN;
		എങ്കിൽ (strcmp(systype, RC_SYS_DOCKER) == 0)
				തിരികെ RC_SYS_DOCKER;
		എങ്കിൽ (strcmp(systype, RC_SYS_PODMAN) == 0)
			തിരികെ RC_SYS_PODMAN;
		എങ്കിൽ (strcmp(systype, RC_SYS_WSL) == 0)
			തിരികെ RC_SYS_WSL;
	}
	എങ്കിൽ (file_regex("/proc/cpuinfo", "UML"))
		തിരികെ RC_SYS_UML;
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		തിരികെ RC_SYS_VSERVER;
	അല്ലെങ്കിൽ എങ്കിൽ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		തിരികെ RC_SYS_OPENVZ;
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		തിരികെ RC_SYS_OPENVZ; /* old test */
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/1/environ", "container=lxc"))
		തിരികെ RC_SYS_LXC;
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/1/environ", "container=rkt"))
		തിരികെ RC_SYS_RKT;
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		തിരികെ RC_SYS_SYSTEMD_NSPAWN;
	അല്ലെങ്കിൽ എങ്കിൽ (access("/run/.containerenv", F_OK) == 0)
		തിരികെ RC_SYS_PODMAN;
	അല്ലെങ്കിൽ എങ്കിൽ (access("/.dockerenv", F_OK) == 0)
		തിരികെ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/1/environ", "container=docker"))
		തിരികെ RC_SYS_DOCKER;
	അല്ലെങ്കിൽ എങ്കിൽ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		തിരികെ RC_SYS_WSL;
	അല്ലെങ്കിൽ എങ്കിൽ (getenv("WSL_DISTRO_NAME"))
		തിരികെ RC_SYS_WSL;
	അല്ലെങ്കിൽ എങ്കിൽ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		തിരികെ RC_SYS_WSL;
#endif

	തിരികെ ശൂന്യ;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
detect_vm(സ്ഥിരം അക്ഷരം *systype RC_UNUSED)
{
#ifdef __NetBSD__
	എങ്കിൽ (systype) {
		എങ്കിൽ (strcmp(systype, RC_SYS_NONE) == 0)
			തിരികെ ശൂന്യ;
		എങ്കിൽ (strcmp(systype, RC_SYS_XEN0) == 0)
			തിരികെ RC_SYS_XEN0;
		എങ്കിൽ (strcmp(systype, RC_SYS_XENU) == 0)
			തിരികെ RC_SYS_XENU;
	}
	എങ്കിൽ (access("/kern/xen/privcmd", F_OK) == 0)
		തിരികെ RC_SYS_XEN0;
	എങ്കിൽ (access("/kern/xen", F_OK) == 0)
		തിരികെ RC_SYS_XENU;
#endif

#ifdef __linux__
	എങ്കിൽ (systype) {
		എങ്കിൽ (strcmp(systype, RC_SYS_NONE) == 0)
			തിരികെ ശൂന്യ;
		എങ്കിൽ (strcmp(systype, RC_SYS_XEN0) == 0)
			തിരികെ RC_SYS_XEN0;
		എങ്കിൽ (strcmp(systype, RC_SYS_XENU) == 0)
			തിരികെ RC_SYS_XENU;
	}
	എങ്കിൽ (access("/proc/xen", F_OK) == 0) {
		എങ്കിൽ (file_regex("/proc/xen/capabilities", "control_d"))
			തിരികെ RC_SYS_XEN0;
		തിരികെ RC_SYS_XENU;
	}
#endif

	തിരികെ ശൂന്യ;
}

സ്ഥിരം അക്ഷരം *
rc_sys(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *systype;
	സ്ഥിരം അക്ഷരം *sys;

	systype = get_systype();
	sys = detect_container(systype);
	എങ്കിൽ (!sys) {
		sys = detect_vm(systype);
	}

	തിരികെ sys;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
rc_parse_service_state(RC_SERVICE state)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; rc_service_state_names[i].name; i++) {
		എങ്കിൽ (rc_service_state_names[i].state == state)
			തിരികെ rc_service_state_names[i].name;
	}
	തിരികെ ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	വേണ്ടി (size_t i = 0; rc_service_state_names[i].name; i++)
		എങ്കിൽ (rc_service_state_names[i].state == state)
			തിരികെ rc_service_state_names[i].dir;
	തിരികെ -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
സ്ഥിതം ശൂന്യം
get_runlevel_chain(സ്ഥിരം അക്ഷരം *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	സ്ഥിരം അക്ഷരം *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	എങ്കിൽ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		തിരികെ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	എങ്കിൽ (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->മൂല്യം;

		/* Check for loop */
		എങ്കിൽ (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->മൂല്യം);
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
rc_runlevel_starting(ശൂന്യം)
{
	തിരികെ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(ശൂന്യം)
{
	തിരികെ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(ശൂന്യം)
{
	തിരികെ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

അക്ഷരം *
rc_runlevel_get(ശൂന്യം)
{
	FILE *fp;
	അക്ഷരം *runlevel = ശൂന്യ;
	size_t i;

	എങ്കിൽ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(വലുപ്പം(അക്ഷരം) * PATH_MAX);
		എങ്കിൽ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			എങ്കിൽ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} അല്ലെങ്കിൽ
			*runlevel = '\0';
		fclose(fp);
	}

	എങ്കിൽ (!runlevel || !*runlevel) {
		സ്വതന്ത്രം(runlevel);
		runlevel = ശൂന്യ;
	}

	തിരികെ runlevel;
}

bool
rc_runlevel_set(സ്ഥിരം അക്ഷരം *runlevel)
{
	FILE *fp;

	എങ്കിൽ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		തിരികെ false;

	fputs(runlevel, fp);
	fclose(fp);
	തിരികെ true;
}

bool
rc_runlevel_exists(സ്ഥിരം അക്ഷരം *runlevel)
{
	ഘടന stat buf;

	എങ്കിൽ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		തിരികെ false;
	എങ്കിൽ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		തിരികെ true;
	തിരികെ false;
}

bool
rc_runlevel_അട്ടി(സ്ഥിരം അക്ഷരം *dst, സ്ഥിരം അക്ഷരം *src)
{
	അക്ഷരം *d, *s;
	പൂർണ്ണസംഖ്യ r;

	എങ്കിൽ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		തിരികെ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	സ്വതന്ത്രം(d);
	സ്വതന്ത്രം(s);
	തിരികെ (r == 0);
}

bool
rc_runlevel_unstack(സ്ഥിരം അക്ഷരം *dst, സ്ഥിരം അക്ഷരം *src)
{
	അക്ഷരം *path;
	പൂർണ്ണസംഖ്യ r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	സ്വതന്ത്രം(path);
	തിരികെ (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(സ്ഥിരം അക്ഷരം *runlevel)
{
	RC_STRINGLIST *അട്ടി, *ancestor_list;
	അട്ടി = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, അട്ടി, ancestor_list);
	rc_stringlist_free(ancestor_list);
	തിരികെ അട്ടി;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *config_path[] = { _RC_PATH, ശൂന്യ };

സ്ഥിതം ഘടന {
	bool set;
	അക്ഷരം *svcdir, *confdir, *runleveldir;
} rc_dirs;

സ്ഥിതം ഘടന {
	അക്ഷരം *buffer;
	size_t എണ്ണം;
	പൂർണ്ണസംഖ്യ *fds;
	സ്ഥിരം അക്ഷരം **entries, **allocd;
} rc_path;

സ്ഥിതം ശൂന്യം
free_rc_dirs(ശൂന്യം)
{
#define X(ptr) ptr = (സ്വതന്ത്രം(ptr), ശൂന്യ)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = ശൂന്യ;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.എണ്ണം = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
സ്ഥിതം ശൂന്യം
append_path(FILE *fp, സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.എണ്ണം++;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
strnsep(സ്ഥിരം അക്ഷരം **str, size_t *size, സ്ഥിരം അക്ഷരം sep)
{
	സ്ഥിരം അക്ഷരം *ret = *str;

	എങ്കിൽ (!ret)
		തിരികെ ശൂന്യ;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	എങ്കിൽ (!**str)
		*str = ശൂന്യ;
	അല്ലെങ്കിൽ
		*str += 1;

	തിരികെ ret;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *
strnullsep(സ്ഥിരം അക്ഷരം **str, size_t *size)
{
	സ്ഥിരം അക്ഷരം *ret = *str;
	size_t len;

	എങ്കിൽ (!ret)
		തിരികെ ശൂന്യ;

	len = strnlen(ret, *size - 1) + 1;

	എങ്കിൽ ((*size -= len))
		*str += len;
	അല്ലെങ്കിൽ
		*str = ശൂന്യ;

	തിരികെ ret;
}

സ്ഥിതം ശൂന്യം
append_user_path(FILE *fp)
{
	സ്ഥിരം അക്ഷരം *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	എങ്കിൽ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	അല്ലെങ്കിൽ ആകുമ്പോൾ ((entry = strnsep(&xdg, &size, ':')))
		എങ്കിൽ (size) append_path(fp, "%.*s/rc", (പൂർണ്ണസംഖ്യ) size, entry);

	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

സ്ഥിതം ശൂന്യം
append_rc_path(FILE *fp, സ്ഥിരം അക്ഷരം *path, bool user)
{
	bool has_user = false;
	സ്ഥിരം അക്ഷരം *entry;
	size_t size;

	ആകുമ്പോൾ ((entry = strnsep(&path, &size, ':'))) {
		എങ്കിൽ (size)
			append_path(fp, "%.*s%s", (പൂർണ്ണസംഖ്യ)size, entry, user ? "/user" : "");
		അല്ലെങ്കിൽ എങ്കിൽ (user && !has_user)
			append_user_path(fp), has_user = true;
		അല്ലെങ്കിൽ എങ്കിൽ (!user) ആകുമ്പോൾ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

സ്ഥിതം ശൂന്യം
load_dynamic_path(bool user)
{
	സ്ഥിരം അക്ഷരം *path, *entry;
	size_t size, എണ്ണം = 0;
	FILE *fp;

	എങ്കിൽ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.എണ്ണം = ARRAY_SIZE(config_path) - 1;

	എങ്കിൽ (!(path = getenv("RC_PATH")) && !user)
		തിരികെ;

	rc_path.എണ്ണം = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	എങ്കിൽ (path)
		append_rc_path(fp, path, user);
	അല്ലെങ്കിൽ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.എണ്ണം + 1, വലുപ്പം(*rc_path.entries));

	path = rc_path.buffer;

	ആകുമ്പോൾ ((entry = strnullsep(&path, &size)) && എണ്ണം < rc_path.എണ്ണം)
		rc_path.entries[എണ്ണം++] = entry;
	rc_path.entries[rc_path.എണ്ണം] = ശൂന്യ;
}

സ്ഥിതം bool is_user = false;

bool
rc_is_user(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ _is_user = -1;

	എങ്കിൽ (_is_user == -1) {
		അക്ഷരം *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	തിരികെ _is_user;
}

ശൂന്യം
rc_set_user(ശൂന്യം)
{
	അക്ഷരം *env;

	എങ്കിൽ (is_user)
		തിരികെ;

	എങ്കിൽ (!rc_dirs.set)
		atexit(free_rc_dirs);
	അല്ലെങ്കിൽ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	എങ്കിൽ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} അല്ലെങ്കിൽ എങ്കിൽ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} അല്ലെങ്കിൽ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	എങ്കിൽ (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

സ്ഥിരം അക്ഷരം * സ്ഥിരം *
rc_scriptdirs(ശൂന്യം)
{
	എങ്കിൽ (!rc_path.entries)
		load_dynamic_path(false);
	തിരികെ rc_path.entries;
}

size_t
rc_scriptdirfds(സ്ഥിരം പൂർണ്ണസംഖ്യ **fds)
{
	സ്ഥിതം size_t len = 0;

	എങ്കിൽ (len == 0) {
		എങ്കിൽ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.എണ്ണം * വലുപ്പം(*rc_path.fds));
		വേണ്ടി (size_t i = 0; i < rc_path.എണ്ണം; i++) {
			പൂർണ്ണസംഖ്യ fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			എങ്കിൽ (fd == -1)
				തുടരുക;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	തിരികെ len;
}

സ്ഥിരം അക്ഷരം *
rc_sysconfdir(ശൂന്യം)
{
	തിരികെ RC_SYSCONFDIR;
}

സ്ഥിരം അക്ഷരം *
rc_usrconfdir(ശൂന്യം)
{
	തിരികെ rc_dirs.confdir ? rc_dirs.confdir : ശൂന്യ;
}

സ്ഥിരം അക്ഷരം *
rc_runleveldir(ശൂന്യം)
{
	തിരികെ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

സ്ഥിരം അക്ഷരം *
rc_svcdir(ശൂന്യം)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം *svcdir;

	എങ്കിൽ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	തിരികെ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

സ്ഥിതം ssize_t
safe_readlink(പൂർണ്ണസംഖ്യ targetfd, സ്ഥിരം അക്ഷരം *path, അക്ഷരം **buf, size_t bufsiz)
{
	അക്ഷരം *buffer;
	ssize_t r;

	വേണ്ടി (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		എങ്കിൽ (r < 0) {
			സ്വതന്ത്രം(buffer);
			തിരികെ -errno;
		}
		എങ്കിൽ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			തിരികെ r;
		}
		സ്വതന്ത്രം(buffer);
		എങ്കിൽ (bufsiz >= SSIZE_MAX - PATH_MAX)
			തിരികെ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
അക്ഷരം *
rc_service_resolve(സ്ഥിരം അക്ഷരം *service)
{
	അക്ഷരം *buffer;
	ഘടന stat buf;

	എങ്കിൽ (!service)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (service[0] == '/')
		തിരികെ xstrdup(service);

	/* First check started services */
	എങ്കിൽ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		എങ്കിൽ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			തിരികെ buffer;
	എങ്കിൽ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		എങ്കിൽ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			തിരികെ buffer;

	വേണ്ടി (സ്ഥിരം അക്ഷരം * സ്ഥിരം *dirs = rc_scriptdirs(); *dirs; dirs++) {
		അക്ഷരം *file = ശൂന്യ;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		എങ്കിൽ (stat(file, &buf) == 0)
			തിരികെ file;
		സ്വതന്ത്രം(file);
	}

	തിരികെ ശൂന്യ;
}

bool
rc_service_exists(സ്ഥിരം അക്ഷരം *service)
{
	അക്ഷരം *file;
	bool retval = false;
	size_t len;
	ഘടന stat buf;

	എങ്കിൽ (!service) {
		errno = EINVAL;
		തിരികെ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	എങ്കിൽ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		തിരികെ false;
	}

	എങ്കിൽ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		തിരികെ false;
	}

	എങ്കിൽ (stat(file, &buf) == 0) {
		എങ്കിൽ (buf.st_mode & S_IXUGO)
			retval = true;
		അല്ലെങ്കിൽ
			errno = ENOEXEC;
	}
	സ്വതന്ത്രം(file);
	തിരികെ retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(സ്ഥിരം അക്ഷരം *service)
{
	അക്ഷരം *svc;
	അക്ഷരം *cmd = ശൂന്യ;
	അക്ഷരം *buffer = ശൂന്യ;
	size_t len = 0;
	RC_STRINGLIST *commands = ശൂന്യ;
	അക്ഷരം *token;
	അക്ഷരം *p;
	FILE *fp;

	എങ്കിൽ (!(svc = rc_service_resolve(service)))
		തിരികെ ശൂന്യ;

	xasprintf(&cmd, OPTSTR, svc);
	സ്വതന്ത്രം(svc);

	എങ്കിൽ (!(fp = popen(cmd, "r"))) {
		സ്വതന്ത്രം(cmd);
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ആകുമ്പോൾ ((token = strsep(&p, " ")))
			എങ്കിൽ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	സ്വതന്ത്രം(buffer);
	സ്വതന്ത്രം(cmd);
	തിരികെ commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
അക്ഷരം *
rc_service_description(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *option)
{
	അക്ഷരം *svc;
	അക്ഷരം *cmd;
	അക്ഷരം *desc = ശൂന്യ;
	size_t size = 0;
	FILE *fp;

	എങ്കിൽ (!(svc = rc_service_resolve(service)))
		തിരികെ ശൂന്യ;

	എങ്കിൽ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	സ്വതന്ത്രം(svc);
	എങ്കിൽ ((fp = popen(cmd, "r"))) {
		എങ്കിൽ (xgetline(&desc, &size, fp) == -1) {
			സ്വതന്ത്രം(desc);
			desc = ശൂന്യ;
		}
		pclose(fp);
	}
	സ്വതന്ത്രം(cmd);
	തിരികെ desc;
}

bool
rc_service_in_runlevel(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *runlevel)
{
	അക്ഷരം *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	സ്വതന്ത്രം(file);
	തിരികെ r;
}

bool
rc_service_mark(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം RC_SERVICE state)
{
	പൂർണ്ണസംഖ്യ i = 0;
	സ്ഥിരം അക്ഷരം *base;
	അക്ഷരം *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	പൂർണ്ണസംഖ്യ serrno;

	എങ്കിൽ (!init)
		തിരികെ false;

	base = basename_c(service);
	എങ്കിൽ (state != RC_SERVICE_STOPPED) {
		പൂർണ്ണസംഖ്യ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		എങ്കിൽ (access(init, F_OK) != 0) {
			സ്വതന്ത്രം(init);
			തിരികെ false;
		}

		unlinkat(state_dirfd, base, 0);
		എങ്കിൽ ((symlinkat(init, state_dirfd, base)) != 0) {
			സ്വതന്ത്രം(init);
			തിരികെ false;
		}
	}

	എങ്കിൽ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		സ്വതന്ത്രം(init);
		തിരികെ true;
	}

	/* Remove any old states now */
	വേണ്ടി (i = 0; rc_service_state_names[i].name; i++) {
		പൂർണ്ണസംഖ്യ state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		എങ്കിൽ (s == state)
			തുടരുക;

		തിരഞ്ഞെടുപ്പ് (s) {
		അവസ്ഥ RC_SERVICE_STOPPED:
		അവസ്ഥ RC_SERVICE_HOTPLUGGED:
		അവസ്ഥ RC_SERVICE_SCHEDULED:
			തുടരുക;
		അവസ്ഥ RC_SERVICE_WASINACTIVE:
			എങ്കിൽ (skip_wasinactive)
				തുടരുക;
			/* fall-through */
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				തുടരുക;
			എങ്കിൽ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				എങ്കിൽ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					സ്വതന്ത്രം(init);
					തിരികെ false;
				}
				skip_wasinactive = true;
			}
			എങ്കിൽ (unlinkat(state_dirfd, base, 0) == -1) {
				സ്വതന്ത്രം(init);
				തിരികെ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	എങ്കിൽ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	എങ്കിൽ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	എങ്കിൽ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		എങ്കിൽ (dp) {
			ഘടന dirent *d;
			ആകുമ്പോൾ ((d = readdir(dp))) {
				അക്ഷരം *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				സ്വതന്ത്രം(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	സ്വതന്ത്രം(init);
	തിരികെ true;
}

RC_SERVICE
rc_service_state(സ്ഥിരം അക്ഷരം *service)
{
	സ്ഥിരം അക്ഷരം *base = basename_c(service);
	പൂർണ്ണസംഖ്യ state = RC_SERVICE_STOPPED;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; rc_service_state_names[i].name; i++) {
		എങ്കിൽ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			എങ്കിൽ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			അല്ലെങ്കിൽ
				state |= rc_service_state_names[i].state;
		}
	}

	എങ്കിൽ (state & RC_SERVICE_STARTED)
		എങ്കിൽ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	എങ്കിൽ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		എങ്കിൽ (dp) {
			ഘടന dirent *d;
			ആകുമ്പോൾ ((d = readdir(dp))) {
				എങ്കിൽ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					നിർത്തുക;
				}
			}

			closedir(dp);
		}
	}

	തിരികെ state;
}

അക്ഷരം *
rc_service_മൂല്യം_get(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *option)
{
	അക്ഷരം *buffer = ശൂന്യ;
	size_t len = 0;
	അക്ഷരം *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	സ്വതന്ത്രം(file);

	തിരികെ buffer;
}

bool
rc_service_മൂല്യം_set(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *option, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	bool ret = true;
	പൂർണ്ണസംഖ്യ optdirfd;
	FILE *fp;

	എങ്കിൽ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		തിരികെ false;

	എങ്കിൽ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		തിരികെ false;

	എങ്കിൽ (!മൂല്യം) {
		unlinkat(optdirfd, option, 0);
	} അല്ലെങ്കിൽ എങ്കിൽ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", മൂല്യം);
		fclose(fp);
	} അല്ലെങ്കിൽ {
		ret = false;
	}

	close(optdirfd);
	തിരികെ ret;
}

bool
rc_service_schedule_start(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *service_to_start)
{
	സ്ഥിരം അക്ഷരം *base_to_start = basename_c(service_to_start);
	സ്ഥിരം അക്ഷരം *base = basename_c(service);
	bool retval;
	പൂർണ്ണസംഖ്യ schedfd;
	അക്ഷരം *init;

	/* service may be a provided service, like net */
	എങ്കിൽ (!service || !rc_service_exists(service_to_start))
		തിരികെ false;

	എങ്കിൽ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		തിരികെ false;

	എങ്കിൽ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		തിരികെ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	സ്വതന്ത്രം(init);
	തിരികെ retval;
}

bool
rc_service_schedule_clear(സ്ഥിരം അക്ഷരം *service)
{
	തിരികെ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(സ്ഥിരം അക്ഷരം *runlevel)
{
	RC_STRINGLIST *list = ശൂന്യ;

	എങ്കിൽ (!runlevel) {
		സ്ഥിരം പൂർണ്ണസംഖ്യ *scriptdirfds;
		list = rc_stringlist_new();
		വേണ്ടി (size_t i = 0, എണ്ണം = rc_scriptdirfds(&scriptdirfds); i < എണ്ണം; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		തിരികെ list;
	}

	/* These special levels never contain any services */
	എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		തിരികെ rc_stringlist_new();

	തിരികെ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(സ്ഥിരം അക്ഷരം *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *അട്ടി;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(അട്ടി, stacks, entries) {
		sl = rc_services_in_runlevel(അട്ടി->മൂല്യം);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	തിരികെ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ഘടന dirent *d;
	DIR *dp;

	എങ്കിൽ (state != RC_SERVICE_SCHEDULED)
		തിരികെ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	എങ്കിൽ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		തിരികെ list;

	ആകുമ്പോൾ ((d = readdir(dp))) {
		എങ്കിൽ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			തുടരുക;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	തിരികെ list;
}

bool
rc_service_add(സ്ഥിരം അക്ഷരം *runlevel, സ്ഥിരം അക്ഷരം *service)
{
	bool retval;
	അക്ഷരം *init;
	അക്ഷരം *file;
	അക്ഷരം *path;
	അക്ഷരം *binit = ശൂന്യ;
	അക്ഷരം *i;

	എങ്കിൽ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		തിരികെ false;
	}

	എങ്കിൽ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		തിരികെ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	എങ്കിൽ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), ശൂന്യ);
		എങ്കിൽ (path == ശൂന്യ) {
			സ്വതന്ത്രം(init);
			തിരികെ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		സ്വതന്ത്രം(path);
		എങ്കിൽ (!retval) {
			സ്വതന്ത്രം(init);
			errno = EPERM;
			തിരികെ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	എങ്കിൽ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		പോകുക out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	സ്വതന്ത്രം(file);

out:
	സ്വതന്ത്രം(binit);
	സ്വതന്ത്രം(init);
	തിരികെ retval;
}

bool
rc_service_delete(സ്ഥിരം അക്ഷരം *runlevel, സ്ഥിരം അക്ഷരം *service)
{
	അക്ഷരം *file;
	പൂർണ്ണസംഖ്യ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	സ്വതന്ത്രം(file);
	തിരികെ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(സ്ഥിരം അക്ഷരം *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ഘടന dirent *d;
	DIR *dp;

	എങ്കിൽ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		തിരികെ list;

	ആകുമ്പോൾ ((d = readdir(dp))) {
		അക്ഷരം *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		എങ്കിൽ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		സ്വതന്ത്രം(file);
	}

	തിരികെ list;
}

RC_STRINGLIST *
rc_services_scheduled(സ്ഥിരം അക്ഷരം *service)
{
	തിരികെ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *env)
{
	size_t name_len = strcspn(env, "=");
	അക്ഷരം var[strlen(service) + വലുപ്പം("/") + name_len];
	പൂർണ്ണസംഖ്യ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	എങ്കിൽ (env[name_len] != '=')
		തിരികെ errno = EINVAL, false;

	snprintf(var, വലുപ്പം(var), "%s/%.*s", service, (പൂർണ്ണസംഖ്യ) name_len, env);

	എങ്കിൽ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		തിരികെ false;

	എങ്കിൽ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		തിരികെ false;

	fputs(&env[name_len + 1], fp);

	തിരികെ fclose(fp) == 0;
}

bool
rc_service_setenv(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	അക്ഷരം var[strlen(service) + വലുപ്പം("/") + strlen(name)];
	പൂർണ്ണസംഖ്യ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, വലുപ്പം(var), "%s/%s", service, name);

	എങ്കിൽ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		തിരികെ false;

	എങ്കിൽ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		തിരികെ false;

	fputs(മൂല്യം, fp);

	തിരികെ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *name, അക്ഷരം **buf)
{
	അക്ഷരം var[strlen(service) + വലുപ്പം("/") + strlen(name)], *മൂല്യം;
	പൂർണ്ണസംഖ്യ dfd = rc_dirfd(RC_DIR_ENVIRON);
	ഘടന stat stat;
	size_t എണ്ണം;
	FILE *fp;

	snprintf(var, വലുപ്പം(var), "%s/%s", service, name);

	എങ്കിൽ (fstatat(dfd, var, &stat, 0) == -1)
		തിരികെ -1;

	എങ്കിൽ (!buf)
		തിരികെ stat.st_size;

	എങ്കിൽ (!(മൂല്യം = xrealloc(*buf, stat.st_size + 1)))
		തിരികെ -1;

	എങ്കിൽ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		തിരികെ -1;

	എണ്ണം = fread(മൂല്യം, 1, stat.st_size, fp);
	മൂല്യം[എണ്ണം] = '\0';

	fclose(fp);
	*buf = മൂല്യം;
	തിരികെ എണ്ണം;
}
