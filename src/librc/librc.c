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

#அந்தர்பாவ <assert.h>
#அந்தர்பாவ <helpers.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <libgen.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <regex.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/param.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "librc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = வெற்று;
நிலையான முழு dirfds[RC_DIR_MAX];

முழு rc_dirfd(எண்ணிக்கை rc_dir dir) {
	முழு flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	முழு dirfd;

	எனில் (dir < 0 || dir >= RC_DIR_MAX)
		திரும்பு -1;
	எனில் (dirfds[dir] <= 0) {
		தேர்வு (dir) {
		நிலை RC_DIR_SYSCONF:
			திரும்பு dirfds[dir] = open(rc_sysconfdir(), flags);
		நிலை RC_DIR_USRCONF:
			திரும்பு dirfds[dir] = open(rc_usrconfdir(), flags);
		நிலை RC_DIR_RUNLEVEL:
			திரும்பு dirfds[dir] = open(rc_runleveldir(), flags);
		நிலை RC_DIR_SVCDIR:
			எனில் (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				திரும்பு -1;
			திரும்பு dirfds[dir] = open(rc_svcdir(), flags);
		இயல்பு:
			ப்ரதிஜ்ஞா(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			எனில் ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				எனில் (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					திரும்பு -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			நிறுத்து;
		}
	}
	திரும்பு dirfds[dir];
}

வெறுமை clear_dirfds(வெறுமை) {
	க்கு (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		எனில் (dirfds[i] <= 0)
			தொடர்;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
நிலையான RC_STRINGLIST *
ls_dir(முழு targetfd, மாறிலி எழுத்து *dir, முழு options)
{
	DIR *dp;
	கட்டமைப்பு dirent *d;
	RC_STRINGLIST *list = வெற்று;
	கட்டமைப்பு stat buf;
	size_t l;

	list = rc_stringlist_new();

	எனில் (!(dp = do_opendirat(targetfd, dir)))
		திரும்பு list;

	போது (((d = readdir(dp)) != வெற்று)) {
		எனில் (d->d_name[0] == '.')
			தொடர்;
		எனில் (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			எனில் (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				தொடர்;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			எனில் (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				தொடர்;
		}
		எனில் (options & LS_DIR) {
			எனில் (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				தொடர்;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	திரும்பு list;
}

நிலையான bool
rm_dir(முழு targetfd, மாறிலி எழுத்து *pathname, bool உச்சி)
{
	bool retval = true;
	கட்டமைப்பு dirent *d;
	கட்டமைப்பு stat s;
	DIR *dp;

	எனில் (!(dp = do_opendirat(targetfd, pathname)))
		திரும்பு false;

	போது ((d = readdir(dp))) {
		எனில் (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			தொடர்;
		எனில் (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			நிறுத்து;
		}
		எனில் (S_ISDIR(s.st_mode)) {
			எனில் (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				நிறுத்து;
			}
		} இல்லையெனில் {
			எனில் (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				நிறுத்து;
			}
		}
	}
	closedir(dp);

	எனில் (!retval)
		திரும்பு false;

	எனில் (உச்சி && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		திரும்பு false;

	திரும்பு true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
நிலையான bool
file_regex(மாறிலி எழுத்து *file, மாறிலி எழுத்து *regex)
{
	FILE *fp;
	எழுத்து *வரி = வெற்று;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	முழு result;

	எனில் (!(fp = fopen(file, "r")))
		திரும்பு false;

	எனில் ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		எழுத்து buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, அளவு(buf));
		fprintf(stderr, "file_regex: %s", buf);
		திரும்பு false;
	}

	போது ((len = xgetline(&வரி, &size, fp)) != -1) {
		எழுத்து *str = வரி;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		செய் {
			எனில் (regexec(&re, str, 0, வெற்று, 0) == 0)
				செல் found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			போது (str < வரி + len && *str == '\0')
				str++;
		} போது (str < வரி + len);
	}
	retval = false;
found:
	fclose(fp);
	விடுவி(வரி);
	regfree(&re);

	திரும்பு retval;
}
#endif


நிலையான மாறிலி எழுத்து *
get_systype(வெறுமை)
{
	எழுத்து *systype = rc_conf_மதிப்பு("rc_sys");
	எனில் (systype) {
		எழுத்து *s = systype;
		/* Convert to uppercase */
		போது (s && *s) {
			எனில் (islower((unsigned எழுத்து) *s))
				*s = toupper((unsigned எழுத்து) *s);
			s++;
		}
	}
	திரும்பு systype;
}

நிலையான மாறிலி எழுத்து *
detect_container(மாறிலி எழுத்து *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	எனில் (systype) {
		எனில் (strcmp(systype, RC_SYS_NONE) == 0)
		       திரும்பு வெற்று;
		எனில் (strcmp(systype, RC_SYS_JAIL) == 0)
			திரும்பு RC_SYS_JAIL;
		எனில் (strcmp(systype, RC_SYS_PODMAN) == 0)
			திரும்பு RC_SYS_PODMAN;
	}

	முழு jailed = 0;
	size_t len = அளவு(jailed);

	எனில் (sysctlbyname("security.jail.jailed", &jailed, &len, வெற்று, 0) == 0)
		எனில் (jailed == 1)
			திரும்பு RC_SYS_JAIL;

	எனில் (access("/var/run/.containerenv", F_OK) == 0)
		திரும்பு RC_SYS_PODMAN;
#endif

#ifdef __linux__
	எனில் (systype) {
		எனில் (strcmp(systype, RC_SYS_NONE) == 0)
			திரும்பு வெற்று;
		எனில் (strcmp(systype, RC_SYS_UML) == 0)
			திரும்பு RC_SYS_UML;
		எனில் (strcmp(systype, RC_SYS_VSERVER) == 0)
			திரும்பு RC_SYS_VSERVER;
		எனில் (strcmp(systype, RC_SYS_OPENVZ) == 0)
			திரும்பு RC_SYS_OPENVZ;
		எனில் (strcmp(systype, RC_SYS_LXC) == 0)
			திரும்பு RC_SYS_LXC;
		எனில் (strcmp(systype, RC_SYS_RKT) == 0)
				திரும்பு RC_SYS_RKT;
		எனில் (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				திரும்பு RC_SYS_SYSTEMD_NSPAWN;
		எனில் (strcmp(systype, RC_SYS_DOCKER) == 0)
				திரும்பு RC_SYS_DOCKER;
		எனில் (strcmp(systype, RC_SYS_PODMAN) == 0)
			திரும்பு RC_SYS_PODMAN;
		எனில் (strcmp(systype, RC_SYS_WSL) == 0)
			திரும்பு RC_SYS_WSL;
	}
	எனில் (file_regex("/proc/cpuinfo", "UML"))
		திரும்பு RC_SYS_UML;
	இல்லையெனில் எனில் (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		திரும்பு RC_SYS_VSERVER;
	இல்லையெனில் எனில் (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		திரும்பு RC_SYS_OPENVZ;
	இல்லையெனில் எனில் (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		திரும்பு RC_SYS_OPENVZ; /* old test */
	இல்லையெனில் எனில் (file_regex("/proc/1/environ", "container=lxc"))
		திரும்பு RC_SYS_LXC;
	இல்லையெனில் எனில் (file_regex("/proc/1/environ", "container=rkt"))
		திரும்பு RC_SYS_RKT;
	இல்லையெனில் எனில் (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		திரும்பு RC_SYS_SYSTEMD_NSPAWN;
	இல்லையெனில் எனில் (access("/run/.containerenv", F_OK) == 0)
		திரும்பு RC_SYS_PODMAN;
	இல்லையெனில் எனில் (access("/.dockerenv", F_OK) == 0)
		திரும்பு RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	இல்லையெனில் எனில் (file_regex("/proc/1/environ", "container=docker"))
		திரும்பு RC_SYS_DOCKER;
	இல்லையெனில் எனில் (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		திரும்பு RC_SYS_WSL;
	இல்லையெனில் எனில் (getenv("WSL_DISTRO_NAME"))
		திரும்பு RC_SYS_WSL;
	இல்லையெனில் எனில் (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		திரும்பு RC_SYS_WSL;
#endif

	திரும்பு வெற்று;
}

நிலையான மாறிலி எழுத்து *
detect_vm(மாறிலி எழுத்து *systype RC_UNUSED)
{
#ifdef __NetBSD__
	எனில் (systype) {
		எனில் (strcmp(systype, RC_SYS_NONE) == 0)
			திரும்பு வெற்று;
		எனில் (strcmp(systype, RC_SYS_XEN0) == 0)
			திரும்பு RC_SYS_XEN0;
		எனில் (strcmp(systype, RC_SYS_XENU) == 0)
			திரும்பு RC_SYS_XENU;
	}
	எனில் (access("/kern/xen/privcmd", F_OK) == 0)
		திரும்பு RC_SYS_XEN0;
	எனில் (access("/kern/xen", F_OK) == 0)
		திரும்பு RC_SYS_XENU;
#endif

#ifdef __linux__
	எனில் (systype) {
		எனில் (strcmp(systype, RC_SYS_NONE) == 0)
			திரும்பு வெற்று;
		எனில் (strcmp(systype, RC_SYS_XEN0) == 0)
			திரும்பு RC_SYS_XEN0;
		எனில் (strcmp(systype, RC_SYS_XENU) == 0)
			திரும்பு RC_SYS_XENU;
	}
	எனில் (access("/proc/xen", F_OK) == 0) {
		எனில் (file_regex("/proc/xen/capabilities", "control_d"))
			திரும்பு RC_SYS_XEN0;
		திரும்பு RC_SYS_XENU;
	}
#endif

	திரும்பு வெற்று;
}

மாறிலி எழுத்து *
rc_sys(வெறுமை)
{
	மாறிலி எழுத்து *systype;
	மாறிலி எழுத்து *sys;

	systype = get_systype();
	sys = detect_container(systype);
	எனில் (!sys) {
		sys = detect_vm(systype);
	}

	திரும்பு sys;
}

நிலையான மாறிலி எழுத்து *
rc_parse_service_state(RC_SERVICE state)
{
	முழு i;

	க்கு (i = 0; rc_service_state_names[i].name; i++) {
		எனில் (rc_service_state_names[i].state == state)
			திரும்பு rc_service_state_names[i].name;
	}
	திரும்பு வெற்று;
}

நிலையான முழு
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	க்கு (size_t i = 0; rc_service_state_names[i].name; i++)
		எனில் (rc_service_state_names[i].state == state)
			திரும்பு rc_service_state_names[i].dir;
	திரும்பு -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
நிலையான வெறுமை
get_runlevel_chain(மாறிலி எழுத்து *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	மாறிலி எழுத்து *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	எனில் (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		திரும்பு;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	எனில் (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->மதிப்பு;

		/* Check for loop */
		எனில் (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->மதிப்பு);
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
rc_runlevel_starting(வெறுமை)
{
	திரும்பு faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(வெறுமை)
{
	திரும்பு faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(வெறுமை)
{
	திரும்பு ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

எழுத்து *
rc_runlevel_get(வெறுமை)
{
	FILE *fp;
	எழுத்து *runlevel = வெற்று;
	size_t i;

	எனில் ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(அளவு(எழுத்து) * PATH_MAX);
		எனில் (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			எனில் (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} இல்லையெனில்
			*runlevel = '\0';
		fclose(fp);
	}

	எனில் (!runlevel || !*runlevel) {
		விடுவி(runlevel);
		runlevel = வெற்று;
	}

	திரும்பு runlevel;
}

bool
rc_runlevel_set(மாறிலி எழுத்து *runlevel)
{
	FILE *fp;

	எனில் (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		திரும்பு false;

	fputs(runlevel, fp);
	fclose(fp);
	திரும்பு true;
}

bool
rc_runlevel_exists(மாறிலி எழுத்து *runlevel)
{
	கட்டமைப்பு stat buf;

	எனில் (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		திரும்பு false;
	எனில் (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		திரும்பு true;
	திரும்பு false;
}

bool
rc_runlevel_அடுக்கு(மாறிலி எழுத்து *dst, மாறிலி எழுத்து *src)
{
	எழுத்து *d, *s;
	முழு r;

	எனில் (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		திரும்பு false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	விடுவி(d);
	விடுவி(s);
	திரும்பு (r == 0);
}

bool
rc_runlevel_unstack(மாறிலி எழுத்து *dst, மாறிலி எழுத்து *src)
{
	எழுத்து *path;
	முழு r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	விடுவி(path);
	திரும்பு (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(மாறிலி எழுத்து *runlevel)
{
	RC_STRINGLIST *அடுக்கு, *ancestor_list;
	அடுக்கு = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, அடுக்கு, ancestor_list);
	rc_stringlist_free(ancestor_list);
	திரும்பு அடுக்கு;
}

நிலையான மாறிலி எழுத்து *config_path[] = { _RC_PATH, வெற்று };

நிலையான கட்டமைப்பு {
	bool set;
	எழுத்து *svcdir, *confdir, *runleveldir;
} rc_dirs;

நிலையான கட்டமைப்பு {
	எழுத்து *buffer;
	size_t மொத்தம்;
	முழு *fds;
	மாறிலி எழுத்து **entries, **allocd;
} rc_path;

நிலையான வெறுமை
free_rc_dirs(வெறுமை)
{
#define X(ptr) ptr = (விடுவி(ptr), வெற்று)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = வெற்று;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.மொத்தம் = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
நிலையான வெறுமை
append_path(FILE *fp, மாறிலி எழுத்து *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.மொத்தம்++;
}

நிலையான மாறிலி எழுத்து *
strnsep(மாறிலி எழுத்து **str, size_t *size, மாறிலி எழுத்து sep)
{
	மாறிலி எழுத்து *ret = *str;

	எனில் (!ret)
		திரும்பு வெற்று;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	எனில் (!**str)
		*str = வெற்று;
	இல்லையெனில்
		*str += 1;

	திரும்பு ret;
}

நிலையான மாறிலி எழுத்து *
strnullsep(மாறிலி எழுத்து **str, size_t *size)
{
	மாறிலி எழுத்து *ret = *str;
	size_t len;

	எனில் (!ret)
		திரும்பு வெற்று;

	len = strnlen(ret, *size - 1) + 1;

	எனில் ((*size -= len))
		*str += len;
	இல்லையெனில்
		*str = வெற்று;

	திரும்பு ret;
}

நிலையான வெறுமை
append_user_path(FILE *fp)
{
	மாறிலி எழுத்து *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	எனில் (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	இல்லையெனில் போது ((entry = strnsep(&xdg, &size, ':')))
		எனில் (size) append_path(fp, "%.*s/rc", (முழு) size, entry);

	க்கு (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

நிலையான வெறுமை
append_rc_path(FILE *fp, மாறிலி எழுத்து *path, bool user)
{
	bool has_user = false;
	மாறிலி எழுத்து *entry;
	size_t size;

	போது ((entry = strnsep(&path, &size, ':'))) {
		எனில் (size)
			append_path(fp, "%.*s%s", (முழு)size, entry, user ? "/user" : "");
		இல்லையெனில் எனில் (user && !has_user)
			append_user_path(fp), has_user = true;
		இல்லையெனில் எனில் (!user) போது (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

நிலையான வெறுமை
load_dynamic_path(bool user)
{
	மாறிலி எழுத்து *path, *entry;
	size_t size, மொத்தம் = 0;
	FILE *fp;

	எனில் (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.மொத்தம் = ARRAY_SIZE(config_path) - 1;

	எனில் (!(path = getenv("RC_PATH")) && !user)
		திரும்பு;

	rc_path.மொத்தம் = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	எனில் (path)
		append_rc_path(fp, path, user);
	இல்லையெனில்
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.மொத்தம் + 1, அளவு(*rc_path.entries));

	path = rc_path.buffer;

	போது ((entry = strnullsep(&path, &size)) && மொத்தம் < rc_path.மொத்தம்)
		rc_path.entries[மொத்தம்++] = entry;
	rc_path.entries[rc_path.மொத்தம்] = வெற்று;
}

நிலையான bool is_user = false;

bool
rc_is_user(வெறுமை)
{
	முழு _is_user = -1;

	எனில் (_is_user == -1) {
		எழுத்து *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	திரும்பு _is_user;
}

வெறுமை
rc_set_user(வெறுமை)
{
	எழுத்து *env;

	எனில் (is_user)
		திரும்பு;

	எனில் (!rc_dirs.set)
		atexit(free_rc_dirs);
	இல்லையெனில்
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	எனில் ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} இல்லையெனில் எனில் ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} இல்லையெனில் {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	எனில் (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

மாறிலி எழுத்து * மாறிலி *
rc_scriptdirs(வெறுமை)
{
	எனில் (!rc_path.entries)
		load_dynamic_path(false);
	திரும்பு rc_path.entries;
}

size_t
rc_scriptdirfds(மாறிலி முழு **fds)
{
	நிலையான size_t len = 0;

	எனில் (len == 0) {
		எனில் (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.மொத்தம் * அளவு(*rc_path.fds));
		க்கு (size_t i = 0; i < rc_path.மொத்தம்; i++) {
			முழு fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			எனில் (fd == -1)
				தொடர்;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	திரும்பு len;
}

மாறிலி எழுத்து *
rc_sysconfdir(வெறுமை)
{
	திரும்பு RC_SYSCONFDIR;
}

மாறிலி எழுத்து *
rc_usrconfdir(வெறுமை)
{
	திரும்பு rc_dirs.confdir ? rc_dirs.confdir : வெற்று;
}

மாறிலி எழுத்து *
rc_runleveldir(வெறுமை)
{
	திரும்பு rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

மாறிலி எழுத்து *
rc_svcdir(வெறுமை)
{
	நிலையான மாறிலி எழுத்து *svcdir;

	எனில் (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	திரும்பு rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

நிலையான ssize_t
safe_readlink(முழு targetfd, மாறிலி எழுத்து *path, எழுத்து **buf, size_t bufsiz)
{
	எழுத்து *buffer;
	ssize_t r;

	க்கு (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		எனில் (r < 0) {
			விடுவி(buffer);
			திரும்பு -errno;
		}
		எனில் ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			திரும்பு r;
		}
		விடுவி(buffer);
		எனில் (bufsiz >= SSIZE_MAX - PATH_MAX)
			திரும்பு -ENOMEM;
	}
}

/* Resolve a service name to its full path */
எழுத்து *
rc_service_resolve(மாறிலி எழுத்து *service)
{
	எழுத்து *buffer;
	கட்டமைப்பு stat buf;

	எனில் (!service)
		திரும்பு வெற்று;

	எனில் (service[0] == '/')
		திரும்பு xstrdup(service);

	/* First check started services */
	எனில் (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		எனில் (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			திரும்பு buffer;
	எனில் (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		எனில் (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			திரும்பு buffer;

	க்கு (மாறிலி எழுத்து * மாறிலி *dirs = rc_scriptdirs(); *dirs; dirs++) {
		எழுத்து *file = வெற்று;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		எனில் (stat(file, &buf) == 0)
			திரும்பு file;
		விடுவி(file);
	}

	திரும்பு வெற்று;
}

bool
rc_service_exists(மாறிலி எழுத்து *service)
{
	எழுத்து *file;
	bool retval = false;
	size_t len;
	கட்டமைப்பு stat buf;

	எனில் (!service) {
		errno = EINVAL;
		திரும்பு false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	எனில் (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		திரும்பு false;
	}

	எனில் (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		திரும்பு false;
	}

	எனில் (stat(file, &buf) == 0) {
		எனில் (buf.st_mode & S_IXUGO)
			retval = true;
		இல்லையெனில்
			errno = ENOEXEC;
	}
	விடுவி(file);
	திரும்பு retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(மாறிலி எழுத்து *service)
{
	எழுத்து *svc;
	எழுத்து *cmd = வெற்று;
	எழுத்து *buffer = வெற்று;
	size_t len = 0;
	RC_STRINGLIST *commands = வெற்று;
	எழுத்து *token;
	எழுத்து *p;
	FILE *fp;

	எனில் (!(svc = rc_service_resolve(service)))
		திரும்பு வெற்று;

	xasprintf(&cmd, OPTSTR, svc);
	விடுவி(svc);

	எனில் (!(fp = popen(cmd, "r"))) {
		விடுவி(cmd);
		திரும்பு வெற்று;
	}

	எனில் (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		போது ((token = strsep(&p, " ")))
			எனில் (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	விடுவி(buffer);
	விடுவி(cmd);
	திரும்பு commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
எழுத்து *
rc_service_description(மாறிலி எழுத்து *service, மாறிலி எழுத்து *option)
{
	எழுத்து *svc;
	எழுத்து *cmd;
	எழுத்து *desc = வெற்று;
	size_t size = 0;
	FILE *fp;

	எனில் (!(svc = rc_service_resolve(service)))
		திரும்பு வெற்று;

	எனில் (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	விடுவி(svc);
	எனில் ((fp = popen(cmd, "r"))) {
		எனில் (xgetline(&desc, &size, fp) == -1) {
			விடுவி(desc);
			desc = வெற்று;
		}
		pclose(fp);
	}
	விடுவி(cmd);
	திரும்பு desc;
}

bool
rc_service_in_runlevel(மாறிலி எழுத்து *service, மாறிலி எழுத்து *runlevel)
{
	எழுத்து *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	விடுவி(file);
	திரும்பு r;
}

bool
rc_service_mark(மாறிலி எழுத்து *service, மாறிலி RC_SERVICE state)
{
	முழு i = 0;
	மாறிலி எழுத்து *base;
	எழுத்து *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	முழு serrno;

	எனில் (!init)
		திரும்பு false;

	base = basename_c(service);
	எனில் (state != RC_SERVICE_STOPPED) {
		முழு state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		எனில் (access(init, F_OK) != 0) {
			விடுவி(init);
			திரும்பு false;
		}

		unlinkat(state_dirfd, base, 0);
		எனில் ((symlinkat(init, state_dirfd, base)) != 0) {
			விடுவி(init);
			திரும்பு false;
		}
	}

	எனில் (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		விடுவி(init);
		திரும்பு true;
	}

	/* Remove any old states now */
	க்கு (i = 0; rc_service_state_names[i].name; i++) {
		முழு state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		எனில் (s == state)
			தொடர்;

		தேர்வு (s) {
		நிலை RC_SERVICE_STOPPED:
		நிலை RC_SERVICE_HOTPLUGGED:
		நிலை RC_SERVICE_SCHEDULED:
			தொடர்;
		நிலை RC_SERVICE_WASINACTIVE:
			எனில் (skip_wasinactive)
				தொடர்;
			/* fall-through */
		இயல்பு:
			எனில் (faccessat(state_dirfd, base, F_OK, 0) != 0)
				தொடர்;
			எனில் ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				எனில் (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					விடுவி(init);
					திரும்பு false;
				}
				skip_wasinactive = true;
			}
			எனில் (unlinkat(state_dirfd, base, 0) == -1) {
				விடுவி(init);
				திரும்பு false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	எனில் (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	எனில் (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	எனில் (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		எனில் (dp) {
			கட்டமைப்பு dirent *d;
			போது ((d = readdir(dp))) {
				எழுத்து *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				விடுவி(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	விடுவி(init);
	திரும்பு true;
}

RC_SERVICE
rc_service_state(மாறிலி எழுத்து *service)
{
	மாறிலி எழுத்து *base = basename_c(service);
	முழு state = RC_SERVICE_STOPPED;
	முழு i;

	க்கு (i = 0; rc_service_state_names[i].name; i++) {
		எனில் (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			எனில் (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			இல்லையெனில்
				state |= rc_service_state_names[i].state;
		}
	}

	எனில் (state & RC_SERVICE_STARTED)
		எனில் (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	எனில் (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		எனில் (dp) {
			கட்டமைப்பு dirent *d;
			போது ((d = readdir(dp))) {
				எனில் (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					நிறுத்து;
				}
			}

			closedir(dp);
		}
	}

	திரும்பு state;
}

எழுத்து *
rc_service_மதிப்பு_get(மாறிலி எழுத்து *service, மாறிலி எழுத்து *option)
{
	எழுத்து *buffer = வெற்று;
	size_t len = 0;
	எழுத்து *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	விடுவி(file);

	திரும்பு buffer;
}

bool
rc_service_மதிப்பு_set(மாறிலி எழுத்து *service, மாறிலி எழுத்து *option, மாறிலி எழுத்து *மதிப்பு)
{
	bool ret = true;
	முழு optdirfd;
	FILE *fp;

	எனில் (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		திரும்பு false;

	எனில் ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		திரும்பு false;

	எனில் (!மதிப்பு) {
		unlinkat(optdirfd, option, 0);
	} இல்லையெனில் எனில் ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", மதிப்பு);
		fclose(fp);
	} இல்லையெனில் {
		ret = false;
	}

	close(optdirfd);
	திரும்பு ret;
}

bool
rc_service_schedule_start(மாறிலி எழுத்து *service, மாறிலி எழுத்து *service_to_start)
{
	மாறிலி எழுத்து *base_to_start = basename_c(service_to_start);
	மாறிலி எழுத்து *base = basename_c(service);
	bool retval;
	முழு schedfd;
	எழுத்து *init;

	/* service may be a provided service, like net */
	எனில் (!service || !rc_service_exists(service_to_start))
		திரும்பு false;

	எனில் (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		திரும்பு false;

	எனில் ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		திரும்பு false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	விடுவி(init);
	திரும்பு retval;
}

bool
rc_service_schedule_clear(மாறிலி எழுத்து *service)
{
	திரும்பு !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(மாறிலி எழுத்து *runlevel)
{
	RC_STRINGLIST *list = வெற்று;

	எனில் (!runlevel) {
		மாறிலி முழு *scriptdirfds;
		list = rc_stringlist_new();
		க்கு (size_t i = 0, மொத்தம் = rc_scriptdirfds(&scriptdirfds); i < மொத்தம்; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		திரும்பு list;
	}

	/* These special levels never contain any services */
	எனில் (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		திரும்பு rc_stringlist_new();

	திரும்பு ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(மாறிலி எழுத்து *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *அடுக்கு;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(அடுக்கு, stacks, entries) {
		sl = rc_services_in_runlevel(அடுக்கு->மதிப்பு);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	திரும்பு list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	கட்டமைப்பு dirent *d;
	DIR *dp;

	எனில் (state != RC_SERVICE_SCHEDULED)
		திரும்பு ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	எனில் (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		திரும்பு list;

	போது ((d = readdir(dp))) {
		எனில் (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			தொடர்;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	திரும்பு list;
}

bool
rc_service_add(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *service)
{
	bool retval;
	எழுத்து *init;
	எழுத்து *file;
	எழுத்து *path;
	எழுத்து *binit = வெற்று;
	எழுத்து *i;

	எனில் (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		திரும்பு false;
	}

	எனில் (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		திரும்பு false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	எனில் (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), வெற்று);
		எனில் (path == வெற்று) {
			விடுவி(init);
			திரும்பு false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		விடுவி(path);
		எனில் (!retval) {
			விடுவி(init);
			errno = EPERM;
			திரும்பு false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	எனில் (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		செல் out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	விடுவி(file);

out:
	விடுவி(binit);
	விடுவி(init);
	திரும்பு retval;
}

bool
rc_service_delete(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *service)
{
	எழுத்து *file;
	முழு r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	விடுவி(file);
	திரும்பு (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(மாறிலி எழுத்து *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	கட்டமைப்பு dirent *d;
	DIR *dp;

	எனில் (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		திரும்பு list;

	போது ((d = readdir(dp))) {
		எழுத்து *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		எனில் (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		விடுவி(file);
	}

	திரும்பு list;
}

RC_STRINGLIST *
rc_services_scheduled(மாறிலி எழுத்து *service)
{
	திரும்பு ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(மாறிலி எழுத்து *service, மாறிலி எழுத்து *env)
{
	size_t name_len = strcspn(env, "=");
	எழுத்து var[strlen(service) + அளவு("/") + name_len];
	முழு dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	எனில் (env[name_len] != '=')
		திரும்பு errno = EINVAL, false;

	snprintf(var, அளவு(var), "%s/%.*s", service, (முழு) name_len, env);

	எனில் (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		திரும்பு false;

	எனில் (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		திரும்பு false;

	fputs(&env[name_len + 1], fp);

	திரும்பு fclose(fp) == 0;
}

bool
rc_service_setenv(மாறிலி எழுத்து *service, மாறிலி எழுத்து *name, மாறிலி எழுத்து *மதிப்பு)
{
	எழுத்து var[strlen(service) + அளவு("/") + strlen(name)];
	முழு dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, அளவு(var), "%s/%s", service, name);

	எனில் (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		திரும்பு false;

	எனில் (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		திரும்பு false;

	fputs(மதிப்பு, fp);

	திரும்பு fclose(fp) == 0;
}

ssize_t
rc_service_getenv(மாறிலி எழுத்து *service, மாறிலி எழுத்து *name, எழுத்து **buf)
{
	எழுத்து var[strlen(service) + அளவு("/") + strlen(name)], *மதிப்பு;
	முழு dfd = rc_dirfd(RC_DIR_ENVIRON);
	கட்டமைப்பு stat stat;
	size_t மொத்தம்;
	FILE *fp;

	snprintf(var, அளவு(var), "%s/%s", service, name);

	எனில் (fstatat(dfd, var, &stat, 0) == -1)
		திரும்பு -1;

	எனில் (!buf)
		திரும்பு stat.st_size;

	எனில் (!(மதிப்பு = xrealloc(*buf, stat.st_size + 1)))
		திரும்பு -1;

	எனில் (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		திரும்பு -1;

	மொத்தம் = fread(மதிப்பு, 1, stat.st_size, fp);
	மதிப்பு[மொத்தம்] = '\0';

	fclose(fp);
	*buf = மதிப்பு;
	திரும்பு மொத்தம்;
}
