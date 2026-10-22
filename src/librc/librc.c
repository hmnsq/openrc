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
FILE *rc_environ_fd = ফাঁকা;
স্থির পূর্ণসংখ্যা dirfds[RC_DIR_MAX];

পূর্ণসংখ্যা rc_dirfd(গণনা rc_dir dir) {
	পূর্ণসংখ্যা flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	পূর্ণসংখ্যা dirfd;

	যদি (dir < 0 || dir >= RC_DIR_MAX)
		ফেরত -1;
	যদি (dirfds[dir] <= 0) {
		নির্বাচন (dir) {
		ক্ষেত্র RC_DIR_SYSCONF:
			ফেরত dirfds[dir] = open(rc_sysconfdir(), flags);
		ক্ষেত্র RC_DIR_USRCONF:
			ফেরত dirfds[dir] = open(rc_usrconfdir(), flags);
		ক্ষেত্র RC_DIR_RUNLEVEL:
			ফেরত dirfds[dir] = open(rc_runleveldir(), flags);
		ক্ষেত্র RC_DIR_SVCDIR:
			যদি (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ফেরত -1;
			ফেরত dirfds[dir] = open(rc_svcdir(), flags);
		পূর্বনির্ধারিত:
			প্রতিজ্ঞা(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			যদি ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				যদি (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ফেরত -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			বিরতি;
		}
	}
	ফেরত dirfds[dir];
}

শূন্য clear_dirfds(শূন্য) {
	জন্য (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		যদি (dirfds[i] <= 0)
			চালু;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
স্থির RC_STRINGLIST *
ls_dir(পূর্ণসংখ্যা targetfd, ধ্রুবক অক্ষর *dir, পূর্ণসংখ্যা options)
{
	DIR *dp;
	কাঠামো dirent *d;
	RC_STRINGLIST *list = ফাঁকা;
	কাঠামো stat buf;
	size_t l;

	list = rc_stringlist_new();

	যদি (!(dp = do_opendirat(targetfd, dir)))
		ফেরত list;

	যখন (((d = readdir(dp)) != ফাঁকা)) {
		যদি (d->d_name[0] == '.')
			চালু;
		যদি (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			যদি (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				চালু;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			যদি (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				চালু;
		}
		যদি (options & LS_DIR) {
			যদি (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				চালু;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ফেরত list;
}

স্থির bool
rm_dir(পূর্ণসংখ্যা targetfd, ধ্রুবক অক্ষর *pathname, bool শীর্ষ)
{
	bool retval = true;
	কাঠামো dirent *d;
	কাঠামো stat s;
	DIR *dp;

	যদি (!(dp = do_opendirat(targetfd, pathname)))
		ফেরত false;

	যখন ((d = readdir(dp))) {
		যদি (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			চালু;
		যদি (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			বিরতি;
		}
		যদি (S_ISDIR(s.st_mode)) {
			যদি (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				বিরতি;
			}
		} অন্যথা {
			যদি (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				বিরতি;
			}
		}
	}
	closedir(dp);

	যদি (!retval)
		ফেরত false;

	যদি (শীর্ষ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ফেরত false;

	ফেরত true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
স্থির bool
file_regex(ধ্রুবক অক্ষর *file, ধ্রুবক অক্ষর *regex)
{
	FILE *fp;
	অক্ষর *সারি = ফাঁকা;
	size_t আকার_2 = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	পূর্ণসংখ্যা ফলাফল;

	যদি (!(fp = fopen(file, "r")))
		ফেরত false;

	যদি ((ফলাফল = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		অক্ষর buf[BUFSIZ];
		fclose(fp);
		regerror(ফলাফল, &re, buf, আকার(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ফেরত false;
	}

	যখন ((len = xgetline(&সারি, &আকার_2, fp)) != -1) {
		অক্ষর *str = সারি;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		করো {
			যদি (regexec(&re, str, 0, ফাঁকা, 0) == 0)
				যাও found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			যখন (str < সারি + len && *str == '\0')
				str++;
		} যখন (str < সারি + len);
	}
	retval = false;
found:
	fclose(fp);
	মুক্ত(সারি);
	regfree(&re);

	ফেরত retval;
}
#endif


স্থির ধ্রুবক অক্ষর *
get_systype(শূন্য)
{
	অক্ষর *systype = rc_conf_মান("rc_sys");
	যদি (systype) {
		অক্ষর *s = systype;
		/* Convert to uppercase */
		যখন (s && *s) {
			যদি (islower((unsigned অক্ষর) *s))
				*s = toupper((unsigned অক্ষর) *s);
			s++;
		}
	}
	ফেরত systype;
}

স্থির ধ্রুবক অক্ষর *
detect_container(ধ্রুবক অক্ষর *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
		       ফেরত ফাঁকা;
		যদি (strcmp(systype, RC_SYS_JAIL) == 0)
			ফেরত RC_SYS_JAIL;
		যদি (strcmp(systype, RC_SYS_PODMAN) == 0)
			ফেরত RC_SYS_PODMAN;
	}

	পূর্ণসংখ্যা jailed = 0;
	size_t len = আকার(jailed);

	যদি (sysctlbyname("security.jail.jailed", &jailed, &len, ফাঁকা, 0) == 0)
		যদি (jailed == 1)
			ফেরত RC_SYS_JAIL;

	যদি (access("/var/run/.containerenv", F_OK) == 0)
		ফেরত RC_SYS_PODMAN;
#endif

#ifdef __linux__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
			ফেরত ফাঁকা;
		যদি (strcmp(systype, RC_SYS_UML) == 0)
			ফেরত RC_SYS_UML;
		যদি (strcmp(systype, RC_SYS_VSERVER) == 0)
			ফেরত RC_SYS_VSERVER;
		যদি (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ফেরত RC_SYS_OPENVZ;
		যদি (strcmp(systype, RC_SYS_LXC) == 0)
			ফেরত RC_SYS_LXC;
		যদি (strcmp(systype, RC_SYS_RKT) == 0)
				ফেরত RC_SYS_RKT;
		যদি (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ফেরত RC_SYS_SYSTEMD_NSPAWN;
		যদি (strcmp(systype, RC_SYS_DOCKER) == 0)
				ফেরত RC_SYS_DOCKER;
		যদি (strcmp(systype, RC_SYS_PODMAN) == 0)
			ফেরত RC_SYS_PODMAN;
		যদি (strcmp(systype, RC_SYS_WSL) == 0)
			ফেরত RC_SYS_WSL;
	}
	যদি (file_regex("/proc/cpuinfo", "UML"))
		ফেরত RC_SYS_UML;
	অন্যথা যদি (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ফেরত RC_SYS_VSERVER;
	অন্যথা যদি (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ফেরত RC_SYS_OPENVZ;
	অন্যথা যদি (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ফেরত RC_SYS_OPENVZ; /* old test */
	অন্যথা যদি (file_regex("/proc/1/environ", "container=lxc"))
		ফেরত RC_SYS_LXC;
	অন্যথা যদি (file_regex("/proc/1/environ", "container=rkt"))
		ফেরত RC_SYS_RKT;
	অন্যথা যদি (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ফেরত RC_SYS_SYSTEMD_NSPAWN;
	অন্যথা যদি (access("/run/.containerenv", F_OK) == 0)
		ফেরত RC_SYS_PODMAN;
	অন্যথা যদি (access("/.dockerenv", F_OK) == 0)
		ফেরত RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	অন্যথা যদি (file_regex("/proc/1/environ", "container=docker"))
		ফেরত RC_SYS_DOCKER;
	অন্যথা যদি (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ফেরত RC_SYS_WSL;
	অন্যথা যদি (getenv("WSL_DISTRO_NAME"))
		ফেরত RC_SYS_WSL;
	অন্যথা যদি (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ফেরত RC_SYS_WSL;
#endif

	ফেরত ফাঁকা;
}

স্থির ধ্রুবক অক্ষর *
detect_vm(ধ্রুবক অক্ষর *systype RC_UNUSED)
{
#ifdef __NetBSD__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
			ফেরত ফাঁকা;
		যদি (strcmp(systype, RC_SYS_XEN0) == 0)
			ফেরত RC_SYS_XEN0;
		যদি (strcmp(systype, RC_SYS_XENU) == 0)
			ফেরত RC_SYS_XENU;
	}
	যদি (access("/kern/xen/privcmd", F_OK) == 0)
		ফেরত RC_SYS_XEN0;
	যদি (access("/kern/xen", F_OK) == 0)
		ফেরত RC_SYS_XENU;
#endif

#ifdef __linux__
	যদি (systype) {
		যদি (strcmp(systype, RC_SYS_NONE) == 0)
			ফেরত ফাঁকা;
		যদি (strcmp(systype, RC_SYS_XEN0) == 0)
			ফেরত RC_SYS_XEN0;
		যদি (strcmp(systype, RC_SYS_XENU) == 0)
			ফেরত RC_SYS_XENU;
	}
	যদি (access("/proc/xen", F_OK) == 0) {
		যদি (file_regex("/proc/xen/capabilities", "control_d"))
			ফেরত RC_SYS_XEN0;
		ফেরত RC_SYS_XENU;
	}
#endif

	ফেরত ফাঁকা;
}

ধ্রুবক অক্ষর *
rc_sys(শূন্য)
{
	ধ্রুবক অক্ষর *systype;
	ধ্রুবক অক্ষর *sys;

	systype = get_systype();
	sys = detect_container(systype);
	যদি (!sys) {
		sys = detect_vm(systype);
	}

	ফেরত sys;
}

স্থির ধ্রুবক অক্ষর *
rc_parse_service_state(RC_SERVICE state)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; rc_service_state_names[i].name; i++) {
		যদি (rc_service_state_names[i].state == state)
			ফেরত rc_service_state_names[i].name;
	}
	ফেরত ফাঁকা;
}

স্থির পূর্ণসংখ্যা
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	জন্য (size_t i = 0; rc_service_state_names[i].name; i++)
		যদি (rc_service_state_names[i].state == state)
			ফেরত rc_service_state_names[i].dir;
	ফেরত -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
স্থির শূন্য
get_runlevel_chain(ধ্রুবক অক্ষর *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ধ্রুবক অক্ষর *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	যদি (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ফেরত;

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
	ফেরত faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(শূন্য)
{
	ফেরত faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(শূন্য)
{
	ফেরত ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

অক্ষর *
rc_runlevel_get(শূন্য)
{
	FILE *fp;
	অক্ষর *runlevel = ফাঁকা;
	size_t i;

	যদি ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(আকার(অক্ষর) * PATH_MAX);
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
		runlevel = ফাঁকা;
	}

	ফেরত runlevel;
}

bool
rc_runlevel_set(ধ্রুবক অক্ষর *runlevel)
{
	FILE *fp;

	যদি (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ফেরত false;

	fputs(runlevel, fp);
	fclose(fp);
	ফেরত true;
}

bool
rc_runlevel_exists(ধ্রুবক অক্ষর *runlevel)
{
	কাঠামো stat buf;

	যদি (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ফেরত false;
	যদি (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ফেরত true;
	ফেরত false;
}

bool
rc_runlevel_স্তূপ(ধ্রুবক অক্ষর *dst, ধ্রুবক অক্ষর *src)
{
	অক্ষর *d, *s;
	পূর্ণসংখ্যা r;

	যদি (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ফেরত false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	মুক্ত(d);
	মুক্ত(s);
	ফেরত (r == 0);
}

bool
rc_runlevel_unstack(ধ্রুবক অক্ষর *dst, ধ্রুবক অক্ষর *src)
{
	অক্ষর *path;
	পূর্ণসংখ্যা r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	মুক্ত(path);
	ফেরত (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ধ্রুবক অক্ষর *runlevel)
{
	RC_STRINGLIST *স্তূপ, *ancestor_list;
	স্তূপ = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, স্তূপ, ancestor_list);
	rc_stringlist_free(ancestor_list);
	ফেরত স্তূপ;
}

স্থির ধ্রুবক অক্ষর *config_path[] = { _RC_PATH, ফাঁকা };

স্থির কাঠামো {
	bool set;
	অক্ষর *svcdir, *confdir, *runleveldir;
} rc_dirs;

স্থির কাঠামো {
	অক্ষর *বাফার;
	size_t গুনতি;
	পূর্ণসংখ্যা *fds;
	ধ্রুবক অক্ষর **entries, **allocd;
} rc_path;

স্থির শূন্য
free_rc_dirs(শূন্য)
{
#define X(ptr) ptr = (মুক্ত(ptr), ফাঁকা)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = ফাঁকা;
	X(rc_path.বাফার);
	X(rc_path.fds);
	rc_path.গুনতি = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
স্থির শূন্য
append_path(FILE *fp, ধ্রুবক অক্ষর *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.গুনতি++;
}

স্থির ধ্রুবক অক্ষর *
strnsep(ধ্রুবক অক্ষর **str, size_t *আকার_2, ধ্রুবক অক্ষর sep)
{
	ধ্রুবক অক্ষর *ret = *str;

	যদি (!ret)
		ফেরত ফাঁকা;

	*str = strchrnul(ret, sep);
	*আকার_2 = *str - ret;
	যদি (!**str)
		*str = ফাঁকা;
	অন্যথা
		*str += 1;

	ফেরত ret;
}

স্থির ধ্রুবক অক্ষর *
strnullsep(ধ্রুবক অক্ষর **str, size_t *আকার_2)
{
	ধ্রুবক অক্ষর *ret = *str;
	size_t len;

	যদি (!ret)
		ফেরত ফাঁকা;

	len = strnlen(ret, *আকার_2 - 1) + 1;

	যদি ((*আকার_2 -= len))
		*str += len;
	অন্যথা
		*str = ফাঁকা;

	ফেরত ret;
}

স্থির শূন্য
append_user_path(FILE *fp)
{
	ধ্রুবক অক্ষর *xdg, *entry;
	size_t আকার_2;

	append_path(fp, "%s", rc_dirs.confdir);
	যদি (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	অন্যথা যখন ((entry = strnsep(&xdg, &আকার_2, ':')))
		যদি (আকার_2) append_path(fp, "%.*s/rc", (পূর্ণসংখ্যা) আকার_2, entry);

	জন্য (size_t i = 0; i < ARRAY_আকার(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

স্থির শূন্য
append_rc_path(FILE *fp, ধ্রুবক অক্ষর *path, bool user)
{
	bool has_user = false;
	ধ্রুবক অক্ষর *entry;
	size_t আকার_2;

	যখন ((entry = strnsep(&path, &আকার_2, ':'))) {
		যদি (আকার_2)
			append_path(fp, "%.*s%s", (পূর্ণসংখ্যা)আকার_2, entry, user ? "/user" : "");
		অন্যথা যদি (user && !has_user)
			append_user_path(fp), has_user = true;
		অন্যথা যদি (!user) যখন (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

স্থির শূন্য
load_dynamic_path(bool user)
{
	ধ্রুবক অক্ষর *path, *entry;
	size_t আকার_2, গুনতি = 0;
	FILE *fp;

	যদি (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.গুনতি = ARRAY_আকার(config_path) - 1;

	যদি (!(path = getenv("RC_PATH")) && !user)
		ফেরত;

	rc_path.গুনতি = 0;
	fp = xopen_memstream(&rc_path.বাফার, &আকার_2);
	যদি (path)
		append_rc_path(fp, path, user);
	অন্যথা
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.গুনতি + 1, আকার(*rc_path.entries));

	path = rc_path.বাফার;

	যখন ((entry = strnullsep(&path, &আকার_2)) && গুনতি < rc_path.গুনতি)
		rc_path.entries[গুনতি++] = entry;
	rc_path.entries[rc_path.গুনতি] = ফাঁকা;
}

স্থির bool is_user = false;

bool
rc_is_user(শূন্য)
{
	পূর্ণসংখ্যা _is_user = -1;

	যদি (_is_user == -1) {
		অক্ষর *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ফেরত _is_user;
}

শূন্য
rc_set_user(শূন্য)
{
	অক্ষর *env;

	যদি (is_user)
		ফেরত;

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

ধ্রুবক অক্ষর * ধ্রুবক *
rc_scriptdirs(শূন্য)
{
	যদি (!rc_path.entries)
		load_dynamic_path(false);
	ফেরত rc_path.entries;
}

size_t
rc_scriptdirfds(ধ্রুবক পূর্ণসংখ্যা **fds)
{
	স্থির size_t len = 0;

	যদি (len == 0) {
		যদি (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.গুনতি * আকার(*rc_path.fds));
		জন্য (size_t i = 0; i < rc_path.গুনতি; i++) {
			পূর্ণসংখ্যা fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			যদি (fd == -1)
				চালু;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ফেরত len;
}

ধ্রুবক অক্ষর *
rc_sysconfdir(শূন্য)
{
	ফেরত RC_SYSCONFDIR;
}

ধ্রুবক অক্ষর *
rc_usrconfdir(শূন্য)
{
	ফেরত rc_dirs.confdir ? rc_dirs.confdir : ফাঁকা;
}

ধ্রুবক অক্ষর *
rc_runleveldir(শূন্য)
{
	ফেরত rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ধ্রুবক অক্ষর *
rc_svcdir(শূন্য)
{
	স্থির ধ্রুবক অক্ষর *svcdir;

	যদি (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ফেরত rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

স্থির ssize_t
safe_readlink(পূর্ণসংখ্যা targetfd, ধ্রুবক অক্ষর *path, অক্ষর **buf, size_t bufsiz)
{
	অক্ষর *বাফার;
	ssize_t r;

	জন্য (;; bufsiz += PATH_MAX) {
		বাফার = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, বাফার, bufsiz);
		যদি (r < 0) {
			মুক্ত(বাফার);
			ফেরত -errno;
		}
		যদি ((size_t)r < bufsiz) {
			বাফার[r] = '\0';
			*buf = বাফার;
			ফেরত r;
		}
		মুক্ত(বাফার);
		যদি (bufsiz >= SSIZE_MAX - PATH_MAX)
			ফেরত -ENOMEM;
	}
}

/* Resolve a service name to its full path */
অক্ষর *
rc_service_resolve(ধ্রুবক অক্ষর *service)
{
	অক্ষর *বাফার;
	কাঠামো stat buf;

	যদি (!service)
		ফেরত ফাঁকা;

	যদি (service[0] == '/')
		ফেরত xstrdup(service);

	/* First check started services */
	যদি (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		যদি (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &বাফার, buf.st_size + 1) >= 0)
			ফেরত বাফার;
	যদি (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		যদি (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &বাফার, buf.st_size + 1) >= 0)
			ফেরত বাফার;

	জন্য (ধ্রুবক অক্ষর * ধ্রুবক *dirs = rc_scriptdirs(); *dirs; dirs++) {
		অক্ষর *file = ফাঁকা;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		যদি (stat(file, &buf) == 0)
			ফেরত file;
		মুক্ত(file);
	}

	ফেরত ফাঁকা;
}

bool
rc_service_exists(ধ্রুবক অক্ষর *service)
{
	অক্ষর *file;
	bool retval = false;
	size_t len;
	কাঠামো stat buf;

	যদি (!service) {
		errno = EINVAL;
		ফেরত false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	যদি (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ফেরত false;
	}

	যদি (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ফেরত false;
	}

	যদি (stat(file, &buf) == 0) {
		যদি (buf.st_mode & S_IXUGO)
			retval = true;
		অন্যথা
			errno = ENOEXEC;
	}
	মুক্ত(file);
	ফেরত retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ধ্রুবক অক্ষর *service)
{
	অক্ষর *svc;
	অক্ষর *cmd = ফাঁকা;
	অক্ষর *বাফার = ফাঁকা;
	size_t len = 0;
	RC_STRINGLIST *commands = ফাঁকা;
	অক্ষর *token;
	অক্ষর *p;
	FILE *fp;

	যদি (!(svc = rc_service_resolve(service)))
		ফেরত ফাঁকা;

	xasprintf(&cmd, OPTSTR, svc);
	মুক্ত(svc);

	যদি (!(fp = popen(cmd, "r"))) {
		মুক্ত(cmd);
		ফেরত ফাঁকা;
	}

	যদি (xgetline(&বাফার, &len, fp) != -1) {
		p = বাফার;
		commands = rc_stringlist_new();

		যখন ((token = strsep(&p, " ")))
			যদি (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	মুক্ত(বাফার);
	মুক্ত(cmd);
	ফেরত commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
অক্ষর *
rc_service_description(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *option)
{
	অক্ষর *svc;
	অক্ষর *cmd;
	অক্ষর *desc = ফাঁকা;
	size_t আকার_2 = 0;
	FILE *fp;

	যদি (!(svc = rc_service_resolve(service)))
		ফেরত ফাঁকা;

	যদি (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	মুক্ত(svc);
	যদি ((fp = popen(cmd, "r"))) {
		যদি (xgetline(&desc, &আকার_2, fp) == -1) {
			মুক্ত(desc);
			desc = ফাঁকা;
		}
		pclose(fp);
	}
	মুক্ত(cmd);
	ফেরত desc;
}

bool
rc_service_in_runlevel(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *runlevel)
{
	অক্ষর *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	মুক্ত(file);
	ফেরত r;
}

bool
rc_service_mark(ধ্রুবক অক্ষর *service, ধ্রুবক RC_SERVICE state)
{
	পূর্ণসংখ্যা i = 0;
	ধ্রুবক অক্ষর *base;
	অক্ষর *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	পূর্ণসংখ্যা serrno;

	যদি (!init)
		ফেরত false;

	base = basename_c(service);
	যদি (state != RC_SERVICE_STOPPED) {
		পূর্ণসংখ্যা state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		যদি (access(init, F_OK) != 0) {
			মুক্ত(init);
			ফেরত false;
		}

		unlinkat(state_dirfd, base, 0);
		যদি ((symlinkat(init, state_dirfd, base)) != 0) {
			মুক্ত(init);
			ফেরত false;
		}
	}

	যদি (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		মুক্ত(init);
		ফেরত true;
	}

	/* Remove any old states now */
	জন্য (i = 0; rc_service_state_names[i].name; i++) {
		পূর্ণসংখ্যা state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		যদি (s == state)
			চালু;

		নির্বাচন (s) {
		ক্ষেত্র RC_SERVICE_STOPPED:
		ক্ষেত্র RC_SERVICE_HOTPLUGGED:
		ক্ষেত্র RC_SERVICE_SCHEDULED:
			চালু;
		ক্ষেত্র RC_SERVICE_WASINACTIVE:
			যদি (skip_wasinactive)
				চালু;
			/* fall-through */
		পূর্বনির্ধারিত:
			যদি (faccessat(state_dirfd, base, F_OK, 0) != 0)
				চালু;
			যদি ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				যদি (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					মুক্ত(init);
					ফেরত false;
				}
				skip_wasinactive = true;
			}
			যদি (unlinkat(state_dirfd, base, 0) == -1) {
				মুক্ত(init);
				ফেরত false;
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
			কাঠামো dirent *d;
			যখন ((d = readdir(dp))) {
				অক্ষর *was;

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
	ফেরত true;
}

RC_SERVICE
rc_service_state(ধ্রুবক অক্ষর *service)
{
	ধ্রুবক অক্ষর *base = basename_c(service);
	পূর্ণসংখ্যা state = RC_SERVICE_STOPPED;
	পূর্ণসংখ্যা i;

	জন্য (i = 0; rc_service_state_names[i].name; i++) {
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
			কাঠামো dirent *d;
			যখন ((d = readdir(dp))) {
				যদি (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					বিরতি;
				}
			}

			closedir(dp);
		}
	}

	ফেরত state;
}

অক্ষর *
rc_service_মান_get(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *option)
{
	অক্ষর *বাফার = ফাঁকা;
	size_t len = 0;
	অক্ষর *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &বাফার, &len);
	মুক্ত(file);

	ফেরত বাফার;
}

bool
rc_service_মান_set(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *option, ধ্রুবক অক্ষর *মান)
{
	bool ret = true;
	পূর্ণসংখ্যা optdirfd;
	FILE *fp;

	যদি (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ফেরত false;

	যদি ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ফেরত false;

	যদি (!মান) {
		unlinkat(optdirfd, option, 0);
	} অন্যথা যদি ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", মান);
		fclose(fp);
	} অন্যথা {
		ret = false;
	}

	close(optdirfd);
	ফেরত ret;
}

bool
rc_service_schedule_start(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *service_to_start)
{
	ধ্রুবক অক্ষর *base_to_start = basename_c(service_to_start);
	ধ্রুবক অক্ষর *base = basename_c(service);
	bool retval;
	পূর্ণসংখ্যা schedfd;
	অক্ষর *init;

	/* service may be a provided service, like net */
	যদি (!service || !rc_service_exists(service_to_start))
		ফেরত false;

	যদি (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ফেরত false;

	যদি ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ফেরত false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	মুক্ত(init);
	ফেরত retval;
}

bool
rc_service_schedule_clear(ধ্রুবক অক্ষর *service)
{
	ফেরত !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ধ্রুবক অক্ষর *runlevel)
{
	RC_STRINGLIST *list = ফাঁকা;

	যদি (!runlevel) {
		ধ্রুবক পূর্ণসংখ্যা *scriptdirfds;
		list = rc_stringlist_new();
		জন্য (size_t i = 0, গুনতি = rc_scriptdirfds(&scriptdirfds); i < গুনতি; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ফেরত list;
	}

	/* These special levels never contain any services */
	যদি (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ফেরত rc_stringlist_new();

	ফেরত ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ধ্রুবক অক্ষর *runlevel)
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
	ফেরত list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	কাঠামো dirent *d;
	DIR *dp;

	যদি (state != RC_SERVICE_SCHEDULED)
		ফেরত ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	যদি (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ফেরত list;

	যখন ((d = readdir(dp))) {
		যদি (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			চালু;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ফেরত list;
}

bool
rc_service_add(ধ্রুবক অক্ষর *runlevel, ধ্রুবক অক্ষর *service)
{
	bool retval;
	অক্ষর *init;
	অক্ষর *file;
	অক্ষর *path;
	অক্ষর *binit = ফাঁকা;
	অক্ষর *i;

	যদি (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ফেরত false;
	}

	যদি (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ফেরত false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	যদি (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), ফাঁকা);
		যদি (path == ফাঁকা) {
			মুক্ত(init);
			ফেরত false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		মুক্ত(path);
		যদি (!retval) {
			মুক্ত(init);
			errno = EPERM;
			ফেরত false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	যদি (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		যাও out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	মুক্ত(file);

out:
	মুক্ত(binit);
	মুক্ত(init);
	ফেরত retval;
}

bool
rc_service_delete(ধ্রুবক অক্ষর *runlevel, ধ্রুবক অক্ষর *service)
{
	অক্ষর *file;
	পূর্ণসংখ্যা r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	মুক্ত(file);
	ফেরত (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ধ্রুবক অক্ষর *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	কাঠামো dirent *d;
	DIR *dp;

	যদি (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ফেরত list;

	যখন ((d = readdir(dp))) {
		অক্ষর *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		যদি (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		মুক্ত(file);
	}

	ফেরত list;
}

RC_STRINGLIST *
rc_services_scheduled(ধ্রুবক অক্ষর *service)
{
	ফেরত ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *env)
{
	size_t name_len = strcspn(env, "=");
	অক্ষর var[strlen(service) + আকার("/") + name_len];
	পূর্ণসংখ্যা dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	যদি (env[name_len] != '=')
		ফেরত errno = EINVAL, false;

	snprintf(var, আকার(var), "%s/%.*s", service, (পূর্ণসংখ্যা) name_len, env);

	যদি (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ফেরত false;

	যদি (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ফেরত false;

	fputs(&env[name_len + 1], fp);

	ফেরত fclose(fp) == 0;
}

bool
rc_service_setenv(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *মান)
{
	অক্ষর var[strlen(service) + আকার("/") + strlen(name)];
	পূর্ণসংখ্যা dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, আকার(var), "%s/%s", service, name);

	যদি (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ফেরত false;

	যদি (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ফেরত false;

	fputs(মান, fp);

	ফেরত fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ধ্রুবক অক্ষর *service, ধ্রুবক অক্ষর *name, অক্ষর **buf)
{
	অক্ষর var[strlen(service) + আকার("/") + strlen(name)], *মান;
	পূর্ণসংখ্যা dfd = rc_dirfd(RC_DIR_ENVIRON);
	কাঠামো stat stat;
	size_t গুনতি;
	FILE *fp;

	snprintf(var, আকার(var), "%s/%s", service, name);

	যদি (fstatat(dfd, var, &stat, 0) == -1)
		ফেরত -1;

	যদি (!buf)
		ফেরত stat.st_size;

	যদি (!(মান = xrealloc(*buf, stat.st_size + 1)))
		ফেরত -1;

	যদি (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ফেরত -1;

	গুনতি = fread(মান, 1, stat.st_size, fp);
	মান[গুনতি] = '\0';

	fclose(fp);
	*buf = মান;
	ফেরত গুনতি;
}
