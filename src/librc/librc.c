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

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <helpers.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <libgen.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <regex.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/param.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "librc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = রিক্ত;
স্থির পূর্ণাঙ্ক dirfds[RC_DIR_MAX];

পূর্ণাঙ্ক rc_dirfd(গণনা rc_dir dir) {
	পূর্ণাঙ্ক flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	পূর্ণাঙ্ক dirfd;

	যদি (dir < 0 || dir >= RC_DIR_MAX)
		প্রত্যাগম -1;
	যদি (dirfds[dir] <= 0) {
		বিকল্প (dir) {
		পক্ষ RC_DIR_SYSCONF:
			প্রত্যাগম dirfds[dir] = open(rc_sysconfdir(), flags);
		পক্ষ RC_DIR_USRCONF:
			প্রত্যাগম dirfds[dir] = open(rc_usrconfdir(), flags);
		পক্ষ RC_DIR_RUNLEVEL:
			প্রত্যাগম dirfds[dir] = open(rc_runleveldir(), flags);
		পক্ষ RC_DIR_SVCDIR:
			যদি (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				প্রত্যাগম -1;
			প্রত্যাগম dirfds[dir] = open(rc_svcdir(), flags);
		স্বভাব:
			প্রতিজ্ঞা(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			যদি ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				যদি (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					প্রত্যাগম -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			ভঙ্গ;
		}
	}
	প্রত্যাগম dirfds[dir];
}

শূন্য clear_dirfds(শূন্য) {
	প্রতি (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		যদি (dirfds[i] <= 0)
			অনুবর্ত;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
স্থির RC_STRINGLIST *
ls_dir(পূর্ণাঙ্ক targetfd, ধ্রুব বর্ণ *dir, পূর্ণাঙ্ক options)
{
	DIR *dp;
	সংরচনা dirent *d;
	RC_STRINGLIST *list = রিক্ত;
	সংরচনা stat buf;
	size_t l;

	list = rc_stringlist_new();

	যদি (!(dp = do_opendirat(targetfd, dir)))
		প্রত্যাগম list;

	যাবত্ (((d = readdir(dp)) != রিক্ত)) {
		যদি (d->d_name[0] == '.')
			অনুবর্ত;
		যদি (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			যদি (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				অনুবর্ত;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			যদি (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				অনুবর্ত;
		}
		যদি (options & LS_DIR) {
			যদি (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				অনুবর্ত;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	প্রত্যাগম list;
}

স্থির bool
rm_dir(পূর্ণাঙ্ক targetfd, ধ্রুব বর্ণ *pathname, bool শীর্ষ)
{
	bool retval = true;
	সংরচনা dirent *d;
	সংরচনা stat s;
	DIR *dp;

	যদি (!(dp = do_opendirat(targetfd, pathname)))
		প্রত্যাগম false;

	যাবত্ ((d = readdir(dp))) {
		যদি (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			অনুবর্ত;
		যদি (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			ভঙ্গ;
		}
		যদি (S_ISDIR(s.st_mode)) {
			যদি (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				ভঙ্গ;
			}
		} অন্যথা {
			যদি (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				ভঙ্গ;
			}
		}
	}
	closedir(dp);

	যদি (!retval)
		প্রত্যাগম false;

	যদি (শীর্ষ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		প্রত্যাগম false;

	প্রত্যাগম true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
স্থির bool
file_regex(ধ্রুব বর্ণ *file, ধ্রুব বর্ণ *regex)
{
	FILE *fp;
	বর্ণ *সারি = রিক্ত;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	পূর্ণাঙ্ক result;

	যদি (!(fp = fopen(file, "r")))
		প্রত্যাগম false;

	যদি ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		বর্ণ buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, পরিমাণ(buf));
		fprintf(stderr, "file_regex: %s", buf);
		প্রত্যাগম false;
	}

	যাবত্ ((len = xgetline(&সারি, &size, fp)) != -1) {
		বর্ণ *str = সারি;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		কুরু {
			যদি (regexec(&re, str, 0, রিক্ত, 0) == 0)
				গচ্ছ found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			যাবত্ (str < সারি + len && *str == '\0')
				str++;
		} যাবত্ (str < সারি + len);
	}
	retval = false;
found:
	fclose(fp);
	মুক্ত(সারি);
	regfree(&re);

	প্রত্যাগম retval;
}
#endif


স্থির ধ্রুব বর্ণ *
get_systype(শূন্য)
{
	বর্ণ *systype = rc_conf_মান("rc_sys");
	যদি (systype) {
		বর্ণ *s = systype;
		/* Convert to uppercase */
		যাবত্ (s && *s) {
			যদি (islower((unsigned বর্ণ) *s))
				*s = toupper((unsigned বর্ণ) *s);
			s++;
		}
	}
	প্রত্যাগম systype;
}

স্থির ধ্রুব বর্ণ *
detect_container(ধ্রুব বর্ণ *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
		       প্রত্যাগম রিক্ত;
		যদি (strcmp(systype, RC_SYS_JAIL) == 0)
			প্রত্যাগম RC_SYS_JAIL;
		যদি (strcmp(systype, RC_SYS_PODMAN) == 0)
			প্রত্যাগম RC_SYS_PODMAN;
	}

	পূর্ণাঙ্ক jailed = 0;
	size_t len = পরিমাণ(jailed);

	যদি (sysctlbyname("security.jail.jailed", &jailed, &len, রিক্ত, 0) == 0)
		যদি (jailed == 1)
			প্রত্যাগম RC_SYS_JAIL;

	যদি (access("/var/run/.containerenv", F_OK) == 0)
		প্রত্যাগম RC_SYS_PODMAN;
#endif

#ifdef __linux__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
			প্রত্যাগম রিক্ত;
		যদি (strcmp(systype, RC_SYS_UML) == 0)
			প্রত্যাগম RC_SYS_UML;
		যদি (strcmp(systype, RC_SYS_VSERVER) == 0)
			প্রত্যাগম RC_SYS_VSERVER;
		যদি (strcmp(systype, RC_SYS_OPENVZ) == 0)
			প্রত্যাগম RC_SYS_OPENVZ;
		যদি (strcmp(systype, RC_SYS_LXC) == 0)
			প্রত্যাগম RC_SYS_LXC;
		যদি (strcmp(systype, RC_SYS_RKT) == 0)
				প্রত্যাগম RC_SYS_RKT;
		যদি (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				প্রত্যাগম RC_SYS_SYSTEMD_NSPAWN;
		যদি (strcmp(systype, RC_SYS_DOCKER) == 0)
				প্রত্যাগম RC_SYS_DOCKER;
		যদি (strcmp(systype, RC_SYS_PODMAN) == 0)
			প্রত্যাগম RC_SYS_PODMAN;
		যদি (strcmp(systype, RC_SYS_WSL) == 0)
			প্রত্যাগম RC_SYS_WSL;
	}
	যদি (file_regex("/proc/cpuinfo", "UML"))
		প্রত্যাগম RC_SYS_UML;
	অন্যথা যদি (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		প্রত্যাগম RC_SYS_VSERVER;
	অন্যথা যদি (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		প্রত্যাগম RC_SYS_OPENVZ;
	অন্যথা যদি (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		প্রত্যাগম RC_SYS_OPENVZ; /* old test */
	অন্যথা যদি (file_regex("/proc/1/environ", "container=lxc"))
		প্রত্যাগম RC_SYS_LXC;
	অন্যথা যদি (file_regex("/proc/1/environ", "container=rkt"))
		প্রত্যাগম RC_SYS_RKT;
	অন্যথা যদি (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		প্রত্যাগম RC_SYS_SYSTEMD_NSPAWN;
	অন্যথা যদি (access("/run/.containerenv", F_OK) == 0)
		প্রত্যাগম RC_SYS_PODMAN;
	অন্যথা যদি (access("/.dockerenv", F_OK) == 0)
		প্রত্যাগম RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	অন্যথা যদি (file_regex("/proc/1/environ", "container=docker"))
		প্রত্যাগম RC_SYS_DOCKER;
	অন্যথা যদি (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		প্রত্যাগম RC_SYS_WSL;
	অন্যথা যদি (getenv("WSL_DISTRO_NAME"))
		প্রত্যাগম RC_SYS_WSL;
	অন্যথা যদি (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		প্রত্যাগম RC_SYS_WSL;
#endif

	প্রত্যাগম রিক্ত;
}

স্থির ধ্রুব বর্ণ *
detect_vm(ধ্রুব বর্ণ *systype RC_UNUSED)
{
#ifdef __NetBSD__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
			প্রত্যাগম রিক্ত;
		যদি (strcmp(systype, RC_SYS_XEN0) == 0)
			প্রত্যাগম RC_SYS_XEN0;
		যদি (strcmp(systype, RC_SYS_XENU) == 0)
			প্রত্যাগম RC_SYS_XENU;
	}
	যদি (access("/kern/xen/privcmd", F_OK) == 0)
		প্রত্যাগম RC_SYS_XEN0;
	যদি (access("/kern/xen", F_OK) == 0)
		প্রত্যাগম RC_SYS_XENU;
#endif

#ifdef __linux__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
			প্রত্যাগম রিক্ত;
		যদি (strcmp(systype, RC_SYS_XEN0) == 0)
			প্রত্যাগম RC_SYS_XEN0;
		যদি (strcmp(systype, RC_SYS_XENU) == 0)
			প্রত্যাগম RC_SYS_XENU;
	}
	যদি (access("/proc/xen", F_OK) == 0) {
		যদি (file_regex("/proc/xen/capabilities", "control_d"))
			প্রত্যাগম RC_SYS_XEN0;
		প্রত্যাগম RC_SYS_XENU;
	}
#endif

	প্রত্যাগম রিক্ত;
}

ধ্রুব বর্ণ *
rc_sys(শূন্য)
{
	ধ্রুব বর্ণ *systype;
	ধ্রুব বর্ণ *sys;

	systype = get_systype();
	sys = detect_container(systype);
	যদি (!sys) {
		sys = detect_vm(systype);
	}

	প্রত্যাগম sys;
}

স্থির ধ্রুব বর্ণ *
rc_parse_service_state(RC_SERVICE state)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; rc_service_state_names[i].name; i++) {
		যদি (rc_service_state_names[i].state == state)
			প্রত্যাগম rc_service_state_names[i].name;
	}
	প্রত্যাগম রিক্ত;
}

স্থির পূর্ণাঙ্ক
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	প্রতি (size_t i = 0; rc_service_state_names[i].name; i++)
		যদি (rc_service_state_names[i].state == state)
			প্রত্যাগম rc_service_state_names[i].dir;
	প্রত্যাগম -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
স্থির শূন্য
get_runlevel_chain(ধ্রুব বর্ণ *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ধ্রুব বর্ণ *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	যদি (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		প্রত্যাগম;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	যদি (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->মান;

		/* Check for loop */
		যদি (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->মান);
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
rc_runlevel_starting(শূন্য)
{
	প্রত্যাগম faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(শূন্য)
{
	প্রত্যাগম faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(শূন্য)
{
	প্রত্যাগম ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

বর্ণ *
rc_runlevel_get(শূন্য)
{
	FILE *fp;
	বর্ণ *runlevel = রিক্ত;
	size_t i;

	যদি ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(পরিমাণ(বর্ণ) * PATH_MAX);
		যদি (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			যদি (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} অন্যথা
			*runlevel = '\0';
		fclose(fp);
	}

	যদি (!runlevel || !*runlevel) {
		মুক্ত(runlevel);
		runlevel = রিক্ত;
	}

	প্রত্যাগম runlevel;
}

bool
rc_runlevel_set(ধ্রুব বর্ণ *runlevel)
{
	FILE *fp;

	যদি (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		প্রত্যাগম false;

	fputs(runlevel, fp);
	fclose(fp);
	প্রত্যাগম true;
}

bool
rc_runlevel_exists(ধ্রুব বর্ণ *runlevel)
{
	সংরচনা stat buf;

	যদি (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		প্রত্যাগম false;
	যদি (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		প্রত্যাগম true;
	প্রত্যাগম false;
}

bool
rc_runlevel_স্তূপ(ধ্রুব বর্ণ *dst, ধ্রুব বর্ণ *src)
{
	বর্ণ *d, *s;
	পূর্ণাঙ্ক r;

	যদি (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		প্রত্যাগম false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	মুক্ত(d);
	মুক্ত(s);
	প্রত্যাগম (r == 0);
}

bool
rc_runlevel_unstack(ধ্রুব বর্ণ *dst, ধ্রুব বর্ণ *src)
{
	বর্ণ *path;
	পূর্ণাঙ্ক r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	মুক্ত(path);
	প্রত্যাগম (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ধ্রুব বর্ণ *runlevel)
{
	RC_STRINGLIST *স্তূপ, *ancestor_list;
	স্তূপ = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, স্তূপ, ancestor_list);
	rc_stringlist_free(ancestor_list);
	প্রত্যাগম স্তূপ;
}

স্থির ধ্রুব বর্ণ *config_path[] = { _RC_PATH, রিক্ত };

স্থির সংরচনা {
	bool set;
	বর্ণ *svcdir, *confdir, *runleveldir;
} rc_dirs;

স্থির সংরচনা {
	বর্ণ *buffer;
	size_t গুনতি;
	পূর্ণাঙ্ক *fds;
	ধ্রুব বর্ণ **entries, **allocd;
} rc_path;

স্থির শূন্য
free_rc_dirs(শূন্য)
{
#define X(ptr) ptr = (মুক্ত(ptr), রিক্ত)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = রিক্ত;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.গুনতি = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
স্থির শূন্য
append_path(FILE *fp, ধ্রুব বর্ণ *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.গুনতি++;
}

স্থির ধ্রুব বর্ণ *
strnsep(ধ্রুব বর্ণ **str, size_t *size, ধ্রুব বর্ণ sep)
{
	ধ্রুব বর্ণ *ret = *str;

	যদি (!ret)
		প্রত্যাগম রিক্ত;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	যদি (!**str)
		*str = রিক্ত;
	অন্যথা
		*str += 1;

	প্রত্যাগম ret;
}

স্থির ধ্রুব বর্ণ *
strnullsep(ধ্রুব বর্ণ **str, size_t *size)
{
	ধ্রুব বর্ণ *ret = *str;
	size_t len;

	যদি (!ret)
		প্রত্যাগম রিক্ত;

	len = strnlen(ret, *size - 1) + 1;

	যদি ((*size -= len))
		*str += len;
	অন্যথা
		*str = রিক্ত;

	প্রত্যাগম ret;
}

স্থির শূন্য
append_user_path(FILE *fp)
{
	ধ্রুব বর্ণ *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	যদি (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	অন্যথা যাবত্ ((entry = strnsep(&xdg, &size, ':')))
		যদি (size) append_path(fp, "%.*s/rc", (পূর্ণাঙ্ক) size, entry);

	প্রতি (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

স্থির শূন্য
append_rc_path(FILE *fp, ধ্রুব বর্ণ *path, bool user)
{
	bool has_user = false;
	ধ্রুব বর্ণ *entry;
	size_t size;

	যাবত্ ((entry = strnsep(&path, &size, ':'))) {
		যদি (size)
			append_path(fp, "%.*s%s", (পূর্ণাঙ্ক)size, entry, user ? "/user" : "");
		অন্যথা যদি (user && !has_user)
			append_user_path(fp), has_user = true;
		অন্যথা যদি (!user) যাবত্ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

স্থির শূন্য
load_dynamic_path(bool user)
{
	ধ্রুব বর্ণ *path, *entry;
	size_t size, গুনতি = 0;
	FILE *fp;

	যদি (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.গুনতি = ARRAY_SIZE(config_path) - 1;

	যদি (!(path = getenv("RC_PATH")) && !user)
		প্রত্যাগম;

	rc_path.গুনতি = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	যদি (path)
		append_rc_path(fp, path, user);
	অন্যথা
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.গুনতি + 1, পরিমাণ(*rc_path.entries));

	path = rc_path.buffer;

	যাবত্ ((entry = strnullsep(&path, &size)) && গুনতি < rc_path.গুনতি)
		rc_path.entries[গুনতি++] = entry;
	rc_path.entries[rc_path.গুনতি] = রিক্ত;
}

স্থির bool is_user = false;

bool
rc_is_user(শূন্য)
{
	পূর্ণাঙ্ক _is_user = -1;

	যদি (_is_user == -1) {
		বর্ণ *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	প্রত্যাগম _is_user;
}

শূন্য
rc_set_user(শূন্য)
{
	বর্ণ *env;

	যদি (is_user)
		প্রত্যাগম;

	যদি (!rc_dirs.set)
		atexit(free_rc_dirs);
	অন্যথা
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	যদি ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} অন্যথা যদি ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} অন্যথা {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	যদি (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

ধ্রুব বর্ণ * ধ্রুব *
rc_scriptdirs(শূন্য)
{
	যদি (!rc_path.entries)
		load_dynamic_path(false);
	প্রত্যাগম rc_path.entries;
}

size_t
rc_scriptdirfds(ধ্রুব পূর্ণাঙ্ক **fds)
{
	স্থির size_t len = 0;

	যদি (len == 0) {
		যদি (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.গুনতি * পরিমাণ(*rc_path.fds));
		প্রতি (size_t i = 0; i < rc_path.গুনতি; i++) {
			পূর্ণাঙ্ক fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			যদি (fd == -1)
				অনুবর্ত;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	প্রত্যাগম len;
}

ধ্রুব বর্ণ *
rc_sysconfdir(শূন্য)
{
	প্রত্যাগম RC_SYSCONFDIR;
}

ধ্রুব বর্ণ *
rc_usrconfdir(শূন্য)
{
	প্রত্যাগম rc_dirs.confdir ? rc_dirs.confdir : রিক্ত;
}

ধ্রুব বর্ণ *
rc_runleveldir(শূন্য)
{
	প্রত্যাগম rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ধ্রুব বর্ণ *
rc_svcdir(শূন্য)
{
	স্থির ধ্রুব বর্ণ *svcdir;

	যদি (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	প্রত্যাগম rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

স্থির ssize_t
safe_readlink(পূর্ণাঙ্ক targetfd, ধ্রুব বর্ণ *path, বর্ণ **buf, size_t bufsiz)
{
	বর্ণ *buffer;
	ssize_t r;

	প্রতি (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		যদি (r < 0) {
			মুক্ত(buffer);
			প্রত্যাগম -errno;
		}
		যদি ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			প্রত্যাগম r;
		}
		মুক্ত(buffer);
		যদি (bufsiz >= SSIZE_MAX - PATH_MAX)
			প্রত্যাগম -ENOMEM;
	}
}

/* Resolve a service name to its full path */
বর্ণ *
rc_service_resolve(ধ্রুব বর্ণ *service)
{
	বর্ণ *buffer;
	সংরচনা stat buf;

	যদি (!service)
		প্রত্যাগম রিক্ত;

	যদি (service[0] == '/')
		প্রত্যাগম xstrdup(service);

	/* First check started services */
	যদি (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		যদি (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			প্রত্যাগম buffer;
	যদি (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		যদি (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			প্রত্যাগম buffer;

	প্রতি (ধ্রুব বর্ণ * ধ্রুব *dirs = rc_scriptdirs(); *dirs; dirs++) {
		বর্ণ *file = রিক্ত;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		যদি (stat(file, &buf) == 0)
			প্রত্যাগম file;
		মুক্ত(file);
	}

	প্রত্যাগম রিক্ত;
}

bool
rc_service_exists(ধ্রুব বর্ণ *service)
{
	বর্ণ *file;
	bool retval = false;
	size_t len;
	সংরচনা stat buf;

	যদি (!service) {
		errno = EINVAL;
		প্রত্যাগম false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	যদি (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		প্রত্যাগম false;
	}

	যদি (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		প্রত্যাগম false;
	}

	যদি (stat(file, &buf) == 0) {
		যদি (buf.st_mode & S_IXUGO)
			retval = true;
		অন্যথা
			errno = ENOEXEC;
	}
	মুক্ত(file);
	প্রত্যাগম retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ধ্রুব বর্ণ *service)
{
	বর্ণ *svc;
	বর্ণ *cmd = রিক্ত;
	বর্ণ *buffer = রিক্ত;
	size_t len = 0;
	RC_STRINGLIST *commands = রিক্ত;
	বর্ণ *token;
	বর্ণ *p;
	FILE *fp;

	যদি (!(svc = rc_service_resolve(service)))
		প্রত্যাগম রিক্ত;

	xasprintf(&cmd, OPTSTR, svc);
	মুক্ত(svc);

	যদি (!(fp = popen(cmd, "r"))) {
		মুক্ত(cmd);
		প্রত্যাগম রিক্ত;
	}

	যদি (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		যাবত্ ((token = strsep(&p, " ")))
			যদি (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	মুক্ত(buffer);
	মুক্ত(cmd);
	প্রত্যাগম commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
বর্ণ *
rc_service_description(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *option)
{
	বর্ণ *svc;
	বর্ণ *cmd;
	বর্ণ *desc = রিক্ত;
	size_t size = 0;
	FILE *fp;

	যদি (!(svc = rc_service_resolve(service)))
		প্রত্যাগম রিক্ত;

	যদি (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	মুক্ত(svc);
	যদি ((fp = popen(cmd, "r"))) {
		যদি (xgetline(&desc, &size, fp) == -1) {
			মুক্ত(desc);
			desc = রিক্ত;
		}
		pclose(fp);
	}
	মুক্ত(cmd);
	প্রত্যাগম desc;
}

bool
rc_service_in_runlevel(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *runlevel)
{
	বর্ণ *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	মুক্ত(file);
	প্রত্যাগম r;
}

bool
rc_service_mark(ধ্রুব বর্ণ *service, ধ্রুব RC_SERVICE state)
{
	পূর্ণাঙ্ক i = 0;
	ধ্রুব বর্ণ *base;
	বর্ণ *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	পূর্ণাঙ্ক serrno;

	যদি (!init)
		প্রত্যাগম false;

	base = basename_c(service);
	যদি (state != RC_SERVICE_STOPPED) {
		পূর্ণাঙ্ক state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		যদি (access(init, F_OK) != 0) {
			মুক্ত(init);
			প্রত্যাগম false;
		}

		unlinkat(state_dirfd, base, 0);
		যদি ((symlinkat(init, state_dirfd, base)) != 0) {
			মুক্ত(init);
			প্রত্যাগম false;
		}
	}

	যদি (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		মুক্ত(init);
		প্রত্যাগম true;
	}

	/* Remove any old states now */
	প্রতি (i = 0; rc_service_state_names[i].name; i++) {
		পূর্ণাঙ্ক state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		যদি (s == state)
			অনুবর্ত;

		বিকল্প (s) {
		পক্ষ RC_SERVICE_STOPPED:
		পক্ষ RC_SERVICE_HOTPLUGGED:
		পক্ষ RC_SERVICE_SCHEDULED:
			অনুবর্ত;
		পক্ষ RC_SERVICE_WASINACTIVE:
			যদি (skip_wasinactive)
				অনুবর্ত;
			/* fall-through */
		স্বভাব:
			যদি (faccessat(state_dirfd, base, F_OK, 0) != 0)
				অনুবর্ত;
			যদি ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				যদি (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					মুক্ত(init);
					প্রত্যাগম false;
				}
				skip_wasinactive = true;
			}
			যদি (unlinkat(state_dirfd, base, 0) == -1) {
				মুক্ত(init);
				প্রত্যাগম false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	যদি (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	যদি (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	যদি (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		যদি (dp) {
			সংরচনা dirent *d;
			যাবত্ ((d = readdir(dp))) {
				বর্ণ *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				মুক্ত(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	মুক্ত(init);
	প্রত্যাগম true;
}

RC_SERVICE
rc_service_state(ধ্রুব বর্ণ *service)
{
	ধ্রুব বর্ণ *base = basename_c(service);
	পূর্ণাঙ্ক state = RC_SERVICE_STOPPED;
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; rc_service_state_names[i].name; i++) {
		যদি (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			যদি (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			অন্যথা
				state |= rc_service_state_names[i].state;
		}
	}

	যদি (state & RC_SERVICE_STARTED)
		যদি (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	যদি (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		যদি (dp) {
			সংরচনা dirent *d;
			যাবত্ ((d = readdir(dp))) {
				যদি (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					ভঙ্গ;
				}
			}

			closedir(dp);
		}
	}

	প্রত্যাগম state;
}

বর্ণ *
rc_service_মান_get(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *option)
{
	বর্ণ *buffer = রিক্ত;
	size_t len = 0;
	বর্ণ *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	মুক্ত(file);

	প্রত্যাগম buffer;
}

bool
rc_service_মান_set(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *option, ধ্রুব বর্ণ *মান)
{
	bool ret = true;
	পূর্ণাঙ্ক optdirfd;
	FILE *fp;

	যদি (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		প্রত্যাগম false;

	যদি ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		প্রত্যাগম false;

	যদি (!মান) {
		unlinkat(optdirfd, option, 0);
	} অন্যথা যদি ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", মান);
		fclose(fp);
	} অন্যথা {
		ret = false;
	}

	close(optdirfd);
	প্রত্যাগম ret;
}

bool
rc_service_schedule_start(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *service_to_start)
{
	ধ্রুব বর্ণ *base_to_start = basename_c(service_to_start);
	ধ্রুব বর্ণ *base = basename_c(service);
	bool retval;
	পূর্ণাঙ্ক schedfd;
	বর্ণ *init;

	/* service may be a provided service, like net */
	যদি (!service || !rc_service_exists(service_to_start))
		প্রত্যাগম false;

	যদি (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		প্রত্যাগম false;

	যদি ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		প্রত্যাগম false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	মুক্ত(init);
	প্রত্যাগম retval;
}

bool
rc_service_schedule_clear(ধ্রুব বর্ণ *service)
{
	প্রত্যাগম !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ধ্রুব বর্ণ *runlevel)
{
	RC_STRINGLIST *list = রিক্ত;

	যদি (!runlevel) {
		ধ্রুব পূর্ণাঙ্ক *scriptdirfds;
		list = rc_stringlist_new();
		প্রতি (size_t i = 0, গুনতি = rc_scriptdirfds(&scriptdirfds); i < গুনতি; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		প্রত্যাগম list;
	}

	/* These special levels never contain any services */
	যদি (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		প্রত্যাগম rc_stringlist_new();

	প্রত্যাগম ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ধ্রুব বর্ণ *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *স্তূপ;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(স্তূপ, stacks, entries) {
		sl = rc_services_in_runlevel(স্তূপ->মান);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	প্রত্যাগম list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	সংরচনা dirent *d;
	DIR *dp;

	যদি (state != RC_SERVICE_SCHEDULED)
		প্রত্যাগম ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	যদি (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		প্রত্যাগম list;

	যাবত্ ((d = readdir(dp))) {
		যদি (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			অনুবর্ত;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	প্রত্যাগম list;
}

bool
rc_service_add(ধ্রুব বর্ণ *runlevel, ধ্রুব বর্ণ *service)
{
	bool retval;
	বর্ণ *init;
	বর্ণ *file;
	বর্ণ *path;
	বর্ণ *binit = রিক্ত;
	বর্ণ *i;

	যদি (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		প্রত্যাগম false;
	}

	যদি (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		প্রত্যাগম false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	যদি (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), রিক্ত);
		যদি (path == রিক্ত) {
			মুক্ত(init);
			প্রত্যাগম false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		মুক্ত(path);
		যদি (!retval) {
			মুক্ত(init);
			errno = EPERM;
			প্রত্যাগম false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	যদি (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		গচ্ছ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	মুক্ত(file);

out:
	মুক্ত(binit);
	মুক্ত(init);
	প্রত্যাগম retval;
}

bool
rc_service_delete(ধ্রুব বর্ণ *runlevel, ধ্রুব বর্ণ *service)
{
	বর্ণ *file;
	পূর্ণাঙ্ক r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	মুক্ত(file);
	প্রত্যাগম (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ধ্রুব বর্ণ *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	সংরচনা dirent *d;
	DIR *dp;

	যদি (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		প্রত্যাগম list;

	যাবত্ ((d = readdir(dp))) {
		বর্ণ *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		যদি (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		মুক্ত(file);
	}

	প্রত্যাগম list;
}

RC_STRINGLIST *
rc_services_scheduled(ধ্রুব বর্ণ *service)
{
	প্রত্যাগম ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *env)
{
	size_t name_len = strcspn(env, "=");
	বর্ণ var[strlen(service) + পরিমাণ("/") + name_len];
	পূর্ণাঙ্ক dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	যদি (env[name_len] != '=')
		প্রত্যাগম errno = EINVAL, false;

	snprintf(var, পরিমাণ(var), "%s/%.*s", service, (পূর্ণাঙ্ক) name_len, env);

	যদি (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		প্রত্যাগম false;

	যদি (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		প্রত্যাগম false;

	fputs(&env[name_len + 1], fp);

	প্রত্যাগম fclose(fp) == 0;
}

bool
rc_service_setenv(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *name, ধ্রুব বর্ণ *মান)
{
	বর্ণ var[strlen(service) + পরিমাণ("/") + strlen(name)];
	পূর্ণাঙ্ক dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, পরিমাণ(var), "%s/%s", service, name);

	যদি (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		প্রত্যাগম false;

	যদি (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		প্রত্যাগম false;

	fputs(মান, fp);

	প্রত্যাগম fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *name, বর্ণ **buf)
{
	বর্ণ var[strlen(service) + পরিমাণ("/") + strlen(name)], *মান;
	পূর্ণাঙ্ক dfd = rc_dirfd(RC_DIR_ENVIRON);
	সংরচনা stat stat;
	size_t গুনতি;
	FILE *fp;

	snprintf(var, পরিমাণ(var), "%s/%s", service, name);

	যদি (fstatat(dfd, var, &stat, 0) == -1)
		প্রত্যাগম -1;

	যদি (!buf)
		প্রত্যাগম stat.st_size;

	যদি (!(মান = xrealloc(*buf, stat.st_size + 1)))
		প্রত্যাগম -1;

	যদি (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		প্রত্যাগম -1;

	গুনতি = fread(মান, 1, stat.st_size, fp);
	মান[গুনতি] = '\0';

	fclose(fp);
	*buf = মান;
	প্রত্যাগম গুনতি;
}
