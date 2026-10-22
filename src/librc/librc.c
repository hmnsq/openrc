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
static ຈຳນວນເຕັມ dirfds[RC_DIR_MAX];

ຈຳນວນເຕັມ rc_dirfd(ga_nanaa rc_dir dir) {
	ຈຳນວນເຕັມ flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	ຈຳນວນເຕັມ dirfd;

	ຖ້າ (dir < 0 || dir >= RC_DIR_MAX)
		ສົ່ງຄືນ -1;
	ຖ້າ (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ສົ່ງຄືນ dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ສົ່ງຄືນ dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ສົ່ງຄືນ dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			ຖ້າ (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ສົ່ງຄືນ -1;
			ສົ່ງຄືນ dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ຖ້າ ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ຖ້າ (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ສົ່ງຄືນ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ສົ່ງຄືນ dirfds[dir];
}

void clear_dirfds(void) {
	ສຳລັບ (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ຖ້າ (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(ຈຳນວນເຕັມ targetfd, const var_na *dir, ຈຳນວນເຕັມ options)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *list = NULL;
	sa_nracanaa stat buf;
	size_t l;

	list = rc_stringlist_new();

	ຖ້າ (!(dp = do_opendirat(targetfd, dir)))
		ສົ່ງຄືນ list;

	ໃນຂະນະທີ່ (((d = readdir(dp)) != NULL)) {
		ຖ້າ (d->d_name[0] == '.')
			continue;
		ຖ້າ (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ຖ້າ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ຖ້າ (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ຖ້າ (options & LS_DIR) {
			ຖ້າ (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ສົ່ງຄືນ list;
}

static bool
rm_dir(ຈຳນວນເຕັມ targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	ຖ້າ (!(dp = do_opendirat(targetfd, pathname)))
		ສົ່ງຄືນ false;

	ໃນຂະນະທີ່ ((d = readdir(dp))) {
		ຖ້າ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ຖ້າ (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		ຖ້າ (S_ISDIR(s.st_mode)) {
			ຖ້າ (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} ບໍ່ດັ່ງນັ້ນ {
			ຖ້າ (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	ຖ້າ (!retval)
		ສົ່ງຄືນ false;

	ຖ້າ (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ສົ່ງຄືນ false;

	ສົ່ງຄືນ true;
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
	ຈຳນວນເຕັມ result;

	ຖ້າ (!(fp = fopen(file, "r")))
		ສົ່ງຄືນ false;

	ຖ້າ ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ສົ່ງຄືນ false;
	}

	ໃນຂະນະທີ່ ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ຖ້າ (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ໃນຂະນະທີ່ (str < line + len && *str == '\0')
				str++;
		} ໃນຂະນະທີ່ (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ສົ່ງຄືນ retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	ຖ້າ (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		ໃນຂະນະທີ່ (s && *s) {
			ຖ້າ (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ສົ່ງຄືນ systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ຖ້າ (systype) {
		ຖ້າ (strcmp(systype, RC_SYS_NONE) == 0)
		       ສົ່ງຄືນ NULL;
		ຖ້າ (strcmp(systype, RC_SYS_JAIL) == 0)
			ສົ່ງຄືນ RC_SYS_JAIL;
		ຖ້າ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ສົ່ງຄືນ RC_SYS_PODMAN;
	}

	ຈຳນວນເຕັມ jailed = 0;
	size_t len = parimaa_na(jailed);

	ຖ້າ (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ຖ້າ (jailed == 1)
			ສົ່ງຄືນ RC_SYS_JAIL;

	ຖ້າ (access("/var/run/.containerenv", F_OK) == 0)
		ສົ່ງຄືນ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ຖ້າ (systype) {
		ຖ້າ (strcmp(systype, RC_SYS_NONE) == 0)
			ສົ່ງຄືນ NULL;
		ຖ້າ (strcmp(systype, RC_SYS_UML) == 0)
			ສົ່ງຄືນ RC_SYS_UML;
		ຖ້າ (strcmp(systype, RC_SYS_VSERVER) == 0)
			ສົ່ງຄືນ RC_SYS_VSERVER;
		ຖ້າ (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ສົ່ງຄືນ RC_SYS_OPENVZ;
		ຖ້າ (strcmp(systype, RC_SYS_LXC) == 0)
			ສົ່ງຄືນ RC_SYS_LXC;
		ຖ້າ (strcmp(systype, RC_SYS_RKT) == 0)
				ສົ່ງຄືນ RC_SYS_RKT;
		ຖ້າ (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ສົ່ງຄືນ RC_SYS_SYSTEMD_NSPAWN;
		ຖ້າ (strcmp(systype, RC_SYS_DOCKER) == 0)
				ສົ່ງຄືນ RC_SYS_DOCKER;
		ຖ້າ (strcmp(systype, RC_SYS_PODMAN) == 0)
			ສົ່ງຄືນ RC_SYS_PODMAN;
		ຖ້າ (strcmp(systype, RC_SYS_WSL) == 0)
			ສົ່ງຄືນ RC_SYS_WSL;
	}
	ຖ້າ (file_regex("/proc/cpuinfo", "UML"))
		ສົ່ງຄືນ RC_SYS_UML;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ສົ່ງຄືນ RC_SYS_VSERVER;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ສົ່ງຄືນ RC_SYS_OPENVZ;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ສົ່ງຄືນ RC_SYS_OPENVZ; /* old test */
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/1/environ", "container=lxc"))
		ສົ່ງຄືນ RC_SYS_LXC;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/1/environ", "container=rkt"))
		ສົ່ງຄືນ RC_SYS_RKT;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ສົ່ງຄືນ RC_SYS_SYSTEMD_NSPAWN;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (access("/run/.containerenv", F_OK) == 0)
		ສົ່ງຄືນ RC_SYS_PODMAN;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (access("/.dockerenv", F_OK) == 0)
		ສົ່ງຄືນ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/1/environ", "container=docker"))
		ສົ່ງຄືນ RC_SYS_DOCKER;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ສົ່ງຄືນ RC_SYS_WSL;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (getenv("WSL_DISTRO_NAME"))
		ສົ່ງຄືນ RC_SYS_WSL;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ສົ່ງຄືນ RC_SYS_WSL;
#endif

	ສົ່ງຄືນ NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ຖ້າ (systype) {
		ຖ້າ (strcmp(systype, RC_SYS_NONE) == 0)
			ສົ່ງຄືນ NULL;
		ຖ້າ (strcmp(systype, RC_SYS_XEN0) == 0)
			ສົ່ງຄືນ RC_SYS_XEN0;
		ຖ້າ (strcmp(systype, RC_SYS_XENU) == 0)
			ສົ່ງຄືນ RC_SYS_XENU;
	}
	ຖ້າ (access("/kern/xen/privcmd", F_OK) == 0)
		ສົ່ງຄືນ RC_SYS_XEN0;
	ຖ້າ (access("/kern/xen", F_OK) == 0)
		ສົ່ງຄືນ RC_SYS_XENU;
#endif

#ifdef __linux__
	ຖ້າ (systype) {
		ຖ້າ (strcmp(systype, RC_SYS_NONE) == 0)
			ສົ່ງຄືນ NULL;
		ຖ້າ (strcmp(systype, RC_SYS_XEN0) == 0)
			ສົ່ງຄືນ RC_SYS_XEN0;
		ຖ້າ (strcmp(systype, RC_SYS_XENU) == 0)
			ສົ່ງຄືນ RC_SYS_XENU;
	}
	ຖ້າ (access("/proc/xen", F_OK) == 0) {
		ຖ້າ (file_regex("/proc/xen/capabilities", "control_d"))
			ສົ່ງຄືນ RC_SYS_XEN0;
		ສົ່ງຄືນ RC_SYS_XENU;
	}
#endif

	ສົ່ງຄືນ NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ຖ້າ (!sys) {
		sys = detect_vm(systype);
	}

	ສົ່ງຄືນ sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	ຈຳນວນເຕັມ i;

	ສຳລັບ (i = 0; rc_service_state_names[i].name; i++) {
		ຖ້າ (rc_service_state_names[i].state == state)
			ສົ່ງຄືນ rc_service_state_names[i].name;
	}
	ສົ່ງຄືນ NULL;
}

static ຈຳນວນເຕັມ
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	ສຳລັບ (size_t i = 0; rc_service_state_names[i].name; i++)
		ຖ້າ (rc_service_state_names[i].state == state)
			ສົ່ງຄືນ rc_service_state_names[i].dir;
	ສົ່ງຄືນ -1;
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
	ຖ້າ (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ສົ່ງຄືນ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ຖ້າ (rc_stringlist_find(level_list, runlevel))
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
		ຖ້າ (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ສົ່ງຄືນ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ສົ່ງຄືນ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ສົ່ງຄືນ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	ຖ້າ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		ຖ້າ (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ຖ້າ (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ບໍ່ດັ່ງນັ້ນ
			*runlevel = '\0';
		fclose(fp);
	}

	ຖ້າ (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ສົ່ງຄືນ runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	ຖ້າ (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ສົ່ງຄືນ false;

	fputs(runlevel, fp);
	fclose(fp);
	ສົ່ງຄືນ true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	ຖ້າ (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ສົ່ງຄືນ false;
	ຖ້າ (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ສົ່ງຄືນ true;
	ສົ່ງຄືນ false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	ຈຳນວນເຕັມ r;

	ຖ້າ (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ສົ່ງຄືນ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ສົ່ງຄືນ (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	ຈຳນວນເຕັມ r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ສົ່ງຄືນ (r == 0);
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
	ສົ່ງຄືນ stack;
}

static const var_na *config_path[] = { _RC_PATH, NULL };

static sa_nracanaa {
	bool set;
	var_na *svcdir, *confdir, *runleveldir;
} rc_dirs;

static sa_nracanaa {
	var_na *buffer;
	size_t count;
	ຈຳນວນເຕັມ *fds;
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

	ຖ້າ (!ret)
		ສົ່ງຄືນ NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ຖ້າ (!**str)
		*str = NULL;
	ບໍ່ດັ່ງນັ້ນ
		*str += 1;

	ສົ່ງຄືນ ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	ຖ້າ (!ret)
		ສົ່ງຄືນ NULL;

	len = strnlen(ret, *size - 1) + 1;

	ຖ້າ ((*size -= len))
		*str += len;
	ບໍ່ດັ່ງນັ້ນ
		*str = NULL;

	ສົ່ງຄືນ ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ຖ້າ (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ບໍ່ດັ່ງນັ້ນ ໃນຂະນະທີ່ ((entry = strnsep(&xdg, &size, ':')))
		ຖ້າ (size) append_path(fp, "%.*s/rc", (ຈຳນວນເຕັມ) size, entry);

	ສຳລັບ (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	ໃນຂະນະທີ່ ((entry = strnsep(&path, &size, ':'))) {
		ຖ້າ (size)
			append_path(fp, "%.*s%s", (ຈຳນວນເຕັມ)size, entry, user ? "/user" : "");
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (user && !has_user)
			append_user_path(fp), has_user = true;
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (!user) ໃນຂະນະທີ່ (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ຖ້າ (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ຖ້າ (!(path = getenv("RC_PATH")) && !user)
		ສົ່ງຄືນ;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ຖ້າ (path)
		append_rc_path(fp, path, user);
	ບໍ່ດັ່ງນັ້ນ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	ໃນຂະນະທີ່ ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	ຈຳນວນເຕັມ _is_user = -1;

	ຖ້າ (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ສົ່ງຄືນ _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	ຖ້າ (is_user)
		ສົ່ງຄືນ;

	ຖ້າ (!rc_dirs.set)
		atexit(free_rc_dirs);
	ບໍ່ດັ່ງນັ້ນ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ຖ້າ ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ບໍ່ດັ່ງນັ້ນ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ຖ້າ (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ຖ້າ (!rc_path.entries)
		load_dynamic_path(false);
	ສົ່ງຄືນ rc_path.entries;
}

size_t
rc_scriptdirfds(const ຈຳນວນເຕັມ **fds)
{
	static size_t len = 0;

	ຖ້າ (len == 0) {
		ຖ້າ (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		ສຳລັບ (size_t i = 0; i < rc_path.count; i++) {
			ຈຳນວນເຕັມ fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ຖ້າ (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ສົ່ງຄືນ len;
}

const var_na *
rc_sysconfdir(void)
{
	ສົ່ງຄືນ RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ສົ່ງຄືນ rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ສົ່ງຄືນ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	ຖ້າ (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ສົ່ງຄືນ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(ຈຳນວນເຕັມ targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	ສຳລັບ (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ຖ້າ (r < 0) {
			free(buffer);
			ສົ່ງຄືນ -errno;
		}
		ຖ້າ ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ສົ່ງຄືນ r;
		}
		free(buffer);
		ຖ້າ (bufsiz >= SSIZE_MAX - PATH_MAX)
			ສົ່ງຄືນ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	ຖ້າ (!service)
		ສົ່ງຄືນ NULL;

	ຖ້າ (service[0] == '/')
		ສົ່ງຄືນ xstrdup(service);

	/* First check started services */
	ຖ້າ (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ຖ້າ (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ສົ່ງຄືນ buffer;
	ຖ້າ (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ຖ້າ (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ສົ່ງຄືນ buffer;

	ສຳລັບ (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ຖ້າ (stat(file, &buf) == 0)
			ສົ່ງຄືນ file;
		free(file);
	}

	ສົ່ງຄືນ NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	ຖ້າ (!service) {
		errno = EINVAL;
		ສົ່ງຄືນ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ຖ້າ (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ສົ່ງຄືນ false;
	}

	ຖ້າ (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ສົ່ງຄືນ false;
	}

	ຖ້າ (stat(file, &buf) == 0) {
		ຖ້າ (buf.st_mode & S_IXUGO)
			retval = true;
		ບໍ່ດັ່ງນັ້ນ
			errno = ENOEXEC;
	}
	free(file);
	ສົ່ງຄືນ retval;
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

	ຖ້າ (!(svc = rc_service_resolve(service)))
		ສົ່ງຄືນ NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ຖ້າ (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ສົ່ງຄືນ NULL;
	}

	ຖ້າ (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ໃນຂະນະທີ່ ((token = strsep(&p, " ")))
			ຖ້າ (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ສົ່ງຄືນ commands;
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

	ຖ້າ (!(svc = rc_service_resolve(service)))
		ສົ່ງຄືນ NULL;

	ຖ້າ (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ຖ້າ ((fp = popen(cmd, "r"))) {
		ຖ້າ (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ສົ່ງຄືນ desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ສົ່ງຄືນ r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	ຈຳນວນເຕັມ i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	ຈຳນວນເຕັມ serrno;

	ຖ້າ (!init)
		ສົ່ງຄືນ false;

	base = basename_c(service);
	ຖ້າ (state != RC_SERVICE_STOPPED) {
		ຈຳນວນເຕັມ state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ຖ້າ (access(init, F_OK) != 0) {
			free(init);
			ສົ່ງຄືນ false;
		}

		unlinkat(state_dirfd, base, 0);
		ຖ້າ ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ສົ່ງຄືນ false;
		}
	}

	ຖ້າ (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ສົ່ງຄືນ true;
	}

	/* Remove any old states now */
	ສຳລັບ (i = 0; rc_service_state_names[i].name; i++) {
		ຈຳນວນເຕັມ state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ຖ້າ (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			ຖ້າ (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ຖ້າ (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ຖ້າ ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ຖ້າ (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ສົ່ງຄືນ false;
				}
				skip_wasinactive = true;
			}
			ຖ້າ (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ສົ່ງຄືນ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ຖ້າ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ຖ້າ (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ຖ້າ (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ຖ້າ (dp) {
			sa_nracanaa dirent *d;
			ໃນຂະນະທີ່ ((d = readdir(dp))) {
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
	ສົ່ງຄືນ true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	ຈຳນວນເຕັມ state = RC_SERVICE_STOPPED;
	ຈຳນວນເຕັມ i;

	ສຳລັບ (i = 0; rc_service_state_names[i].name; i++) {
		ຖ້າ (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ຖ້າ (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ບໍ່ດັ່ງນັ້ນ
				state |= rc_service_state_names[i].state;
		}
	}

	ຖ້າ (state & RC_SERVICE_STARTED)
		ຖ້າ (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ຖ້າ (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ຖ້າ (dp) {
			sa_nracanaa dirent *d;
			ໃນຂະນະທີ່ ((d = readdir(dp))) {
				ຖ້າ (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ສົ່ງຄືນ state;
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

	ສົ່ງຄືນ buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	ຈຳນວນເຕັມ optdirfd;
	FILE *fp;

	ຖ້າ (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ສົ່ງຄືນ false;

	ຖ້າ ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ສົ່ງຄືນ false;

	ຖ້າ (!value) {
		unlinkat(optdirfd, option, 0);
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} ບໍ່ດັ່ງນັ້ນ {
		ret = false;
	}

	close(optdirfd);
	ສົ່ງຄືນ ret;
}

bool
rc_service_schedule_start(const var_na *service, const var_na *service_to_start)
{
	const var_na *base_to_start = basename_c(service_to_start);
	const var_na *base = basename_c(service);
	bool retval;
	ຈຳນວນເຕັມ schedfd;
	var_na *init;

	/* service may be a provided service, like net */
	ຖ້າ (!service || !rc_service_exists(service_to_start))
		ສົ່ງຄືນ false;

	ຖ້າ (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ສົ່ງຄືນ false;

	ຖ້າ ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ສົ່ງຄືນ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ສົ່ງຄືນ retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ສົ່ງຄືນ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ຖ້າ (!runlevel) {
		const ຈຳນວນເຕັມ *scriptdirfds;
		list = rc_stringlist_new();
		ສຳລັບ (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ສົ່ງຄືນ list;
	}

	/* These special levels never contain any services */
	ຖ້າ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ສົ່ງຄືນ rc_stringlist_new();

	ສົ່ງຄືນ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ສົ່ງຄືນ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	ຖ້າ (state != RC_SERVICE_SCHEDULED)
		ສົ່ງຄືນ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ຖ້າ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ສົ່ງຄືນ list;

	ໃນຂະນະທີ່ ((d = readdir(dp))) {
		ຖ້າ (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ສົ່ງຄືນ list;
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

	ຖ້າ (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ສົ່ງຄືນ false;
	}

	ຖ້າ (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ສົ່ງຄືນ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ຖ້າ (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ຖ້າ (path == NULL) {
			free(init);
			ສົ່ງຄືນ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ຖ້າ (!retval) {
			free(init);
			errno = EPERM;
			ສົ່ງຄືນ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ຖ້າ (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ສົ່ງຄືນ retval;
}

bool
rc_service_delete(const var_na *runlevel, const var_na *service)
{
	var_na *file;
	ຈຳນວນເຕັມ r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	ສົ່ງຄືນ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	ຖ້າ (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ສົ່ງຄືນ list;

	ໃນຂະນະທີ່ ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ຖ້າ (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ສົ່ງຄືນ list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ສົ່ງຄືນ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	ຈຳນວນເຕັມ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ຖ້າ (env[name_len] != '=')
		ສົ່ງຄືນ errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (ຈຳນວນເຕັມ) name_len, env);

	ຖ້າ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ສົ່ງຄືນ false;

	ຖ້າ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ສົ່ງຄືນ false;

	fputs(&env[name_len + 1], fp);

	ສົ່ງຄືນ fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	ຈຳນວນເຕັມ dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ຖ້າ (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ສົ່ງຄືນ false;

	ຖ້າ (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ສົ່ງຄືນ false;

	fputs(value, fp);

	ສົ່ງຄືນ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const var_na *service, const var_na *name, var_na **buf)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)], *value;
	ຈຳນວນເຕັມ dfd = rc_dirfd(RC_DIR_ENVIRON);
	sa_nracanaa stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ຖ້າ (fstatat(dfd, var, &stat, 0) == -1)
		ສົ່ງຄືນ -1;

	ຖ້າ (!buf)
		ສົ່ງຄືນ stat.st_size;

	ຖ້າ (!(value = xrealloc(*buf, stat.st_size + 1)))
		ສົ່ງຄືນ -1;

	ຖ້າ (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ສົ່ງຄືນ -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ສົ່ງຄືນ count;
}
