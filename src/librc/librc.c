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
static puur_naa_nka dirfds[RC_DIR_MAX];

puur_naa_nka rc_dirfd(ga_nanaa rc_dir dir) {
	puur_naa_nka flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	puur_naa_nka dirfd;

	ပေႃး (dir < 0 || dir >= RC_DIR_MAX)
		ပွၵ်ႈ -1;
	ပေႃး (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ပွၵ်ႈ dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ပွၵ်ႈ dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ပွၵ်ႈ dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			ပေႃး (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ပွၵ်ႈ -1;
			ပွၵ်ႈ dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			ပေႃး ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				ပေႃး (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ပွၵ်ႈ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ပွၵ်ႈ dirfds[dir];
}

void clear_dirfds(void) {
	for (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		ပေႃး (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(puur_naa_nka targetfd, const var_na *dir, puur_naa_nka options)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *list = NULL;
	sa_nracanaa stat buf;
	size_t l;

	list = rc_stringlist_new();

	ပေႃး (!(dp = do_opendirat(targetfd, dir)))
		ပွၵ်ႈ list;

	ၶၢဝ်း (((d = readdir(dp)) != NULL)) {
		ပေႃး (d->d_name[0] == '.')
			continue;
		ပေႃး (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			ပေႃး (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			ပေႃး (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		ပေႃး (options & LS_DIR) {
			ပေႃး (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ပွၵ်ႈ list;
}

static bool
rm_dir(puur_naa_nka targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	ပေႃး (!(dp = do_opendirat(targetfd, pathname)))
		ပွၵ်ႈ false;

	ၶၢဝ်း ((d = readdir(dp))) {
		ပေႃး (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		ပေႃး (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		ပေႃး (S_ISDIR(s.st_mode)) {
			ပေႃး (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} ပေႃးဢမ်ႇ {
			ပေႃး (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	ပေႃး (!retval)
		ပွၵ်ႈ false;

	ပေႃး (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ပွၵ်ႈ false;

	ပွၵ်ႈ true;
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
	puur_naa_nka result;

	ပေႃး (!(fp = fopen(file, "r")))
		ပွၵ်ႈ false;

	ပေႃး ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ပွၵ်ႈ false;
	}

	ၶၢဝ်း ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			ပေႃး (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			ၶၢဝ်း (str < line + len && *str == '\0')
				str++;
		} ၶၢဝ်း (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ပွၵ်ႈ retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	ပေႃး (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		ၶၢဝ်း (s && *s) {
			ပေႃး (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ပွၵ်ႈ systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	ပေႃး (systype) {
		ပေႃး (strcmp(systype, RC_SYS_NONE) == 0)
		       ပွၵ်ႈ NULL;
		ပေႃး (strcmp(systype, RC_SYS_JAIL) == 0)
			ပွၵ်ႈ RC_SYS_JAIL;
		ပေႃး (strcmp(systype, RC_SYS_PODMAN) == 0)
			ပွၵ်ႈ RC_SYS_PODMAN;
	}

	puur_naa_nka jailed = 0;
	size_t len = parimaa_na(jailed);

	ပေႃး (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		ပေႃး (jailed == 1)
			ပွၵ်ႈ RC_SYS_JAIL;

	ပေႃး (access("/var/run/.containerenv", F_OK) == 0)
		ပွၵ်ႈ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	ပေႃး (systype) {
		ပေႃး (strcmp(systype, RC_SYS_NONE) == 0)
			ပွၵ်ႈ NULL;
		ပေႃး (strcmp(systype, RC_SYS_UML) == 0)
			ပွၵ်ႈ RC_SYS_UML;
		ပေႃး (strcmp(systype, RC_SYS_VSERVER) == 0)
			ပွၵ်ႈ RC_SYS_VSERVER;
		ပေႃး (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ပွၵ်ႈ RC_SYS_OPENVZ;
		ပေႃး (strcmp(systype, RC_SYS_LXC) == 0)
			ပွၵ်ႈ RC_SYS_LXC;
		ပေႃး (strcmp(systype, RC_SYS_RKT) == 0)
				ပွၵ်ႈ RC_SYS_RKT;
		ပေႃး (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ပွၵ်ႈ RC_SYS_SYSTEMD_NSPAWN;
		ပေႃး (strcmp(systype, RC_SYS_DOCKER) == 0)
				ပွၵ်ႈ RC_SYS_DOCKER;
		ပေႃး (strcmp(systype, RC_SYS_PODMAN) == 0)
			ပွၵ်ႈ RC_SYS_PODMAN;
		ပေႃး (strcmp(systype, RC_SYS_WSL) == 0)
			ပွၵ်ႈ RC_SYS_WSL;
	}
	ပေႃး (file_regex("/proc/cpuinfo", "UML"))
		ပွၵ်ႈ RC_SYS_UML;
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ပွၵ်ႈ RC_SYS_VSERVER;
	ပေႃးဢမ်ႇ ပေႃး (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ပွၵ်ႈ RC_SYS_OPENVZ;
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ပွၵ်ႈ RC_SYS_OPENVZ; /* old test */
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/1/environ", "container=lxc"))
		ပွၵ်ႈ RC_SYS_LXC;
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/1/environ", "container=rkt"))
		ပွၵ်ႈ RC_SYS_RKT;
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ပွၵ်ႈ RC_SYS_SYSTEMD_NSPAWN;
	ပေႃးဢမ်ႇ ပေႃး (access("/run/.containerenv", F_OK) == 0)
		ပွၵ်ႈ RC_SYS_PODMAN;
	ပေႃးဢမ်ႇ ပေႃး (access("/.dockerenv", F_OK) == 0)
		ပွၵ်ႈ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/1/environ", "container=docker"))
		ပွၵ်ႈ RC_SYS_DOCKER;
	ပေႃးဢမ်ႇ ပေႃး (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ပွၵ်ႈ RC_SYS_WSL;
	ပေႃးဢမ်ႇ ပေႃး (getenv("WSL_DISTRO_NAME"))
		ပွၵ်ႈ RC_SYS_WSL;
	ပေႃးဢမ်ႇ ပေႃး (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ပွၵ်ႈ RC_SYS_WSL;
#endif

	ပွၵ်ႈ NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	ပေႃး (systype) {
		ပေႃး (strcmp(systype, RC_SYS_NONE) == 0)
			ပွၵ်ႈ NULL;
		ပေႃး (strcmp(systype, RC_SYS_XEN0) == 0)
			ပွၵ်ႈ RC_SYS_XEN0;
		ပေႃး (strcmp(systype, RC_SYS_XENU) == 0)
			ပွၵ်ႈ RC_SYS_XENU;
	}
	ပေႃး (access("/kern/xen/privcmd", F_OK) == 0)
		ပွၵ်ႈ RC_SYS_XEN0;
	ပေႃး (access("/kern/xen", F_OK) == 0)
		ပွၵ်ႈ RC_SYS_XENU;
#endif

#ifdef __linux__
	ပေႃး (systype) {
		ပေႃး (strcmp(systype, RC_SYS_NONE) == 0)
			ပွၵ်ႈ NULL;
		ပေႃး (strcmp(systype, RC_SYS_XEN0) == 0)
			ပွၵ်ႈ RC_SYS_XEN0;
		ပေႃး (strcmp(systype, RC_SYS_XENU) == 0)
			ပွၵ်ႈ RC_SYS_XENU;
	}
	ပေႃး (access("/proc/xen", F_OK) == 0) {
		ပေႃး (file_regex("/proc/xen/capabilities", "control_d"))
			ပွၵ်ႈ RC_SYS_XEN0;
		ပွၵ်ႈ RC_SYS_XENU;
	}
#endif

	ပွၵ်ႈ NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	ပေႃး (!sys) {
		sys = detect_vm(systype);
	}

	ပွၵ်ႈ sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	puur_naa_nka i;

	for (i = 0; rc_service_state_names[i].name; i++) {
		ပေႃး (rc_service_state_names[i].state == state)
			ပွၵ်ႈ rc_service_state_names[i].name;
	}
	ပွၵ်ႈ NULL;
}

static puur_naa_nka
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	for (size_t i = 0; rc_service_state_names[i].name; i++)
		ပေႃး (rc_service_state_names[i].state == state)
			ပွၵ်ႈ rc_service_state_names[i].dir;
	ပွၵ်ႈ -1;
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
	ပေႃး (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ပွၵ်ႈ;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	ပေႃး (rc_stringlist_find(level_list, runlevel))
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
		ပေႃး (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ပွၵ်ႈ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ပွၵ်ႈ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ပွၵ်ႈ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	ပေႃး ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		ပေႃး (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			ပေႃး (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ပေႃးဢမ်ႇ
			*runlevel = '\0';
		fclose(fp);
	}

	ပေႃး (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ပွၵ်ႈ runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	ပေႃး (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ပွၵ်ႈ false;

	fputs(runlevel, fp);
	fclose(fp);
	ပွၵ်ႈ true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	ပေႃး (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ပွၵ်ႈ false;
	ပေႃး (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ပွၵ်ႈ true;
	ပွၵ်ႈ false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	puur_naa_nka r;

	ပေႃး (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ပွၵ်ႈ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ပွၵ်ႈ (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	puur_naa_nka r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ပွၵ်ႈ (r == 0);
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
	ပွၵ်ႈ stack;
}

static const var_na *config_path[] = { _RC_PATH, NULL };

static sa_nracanaa {
	bool set;
	var_na *svcdir, *confdir, *runleveldir;
} rc_dirs;

static sa_nracanaa {
	var_na *buffer;
	size_t count;
	puur_naa_nka *fds;
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

	ပေႃး (!ret)
		ပွၵ်ႈ NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	ပေႃး (!**str)
		*str = NULL;
	ပေႃးဢမ်ႇ
		*str += 1;

	ပွၵ်ႈ ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	ပေႃး (!ret)
		ပွၵ်ႈ NULL;

	len = strnlen(ret, *size - 1) + 1;

	ပေႃး ((*size -= len))
		*str += len;
	ပေႃးဢမ်ႇ
		*str = NULL;

	ပွၵ်ႈ ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	ပေႃး (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	ပေႃးဢမ်ႇ ၶၢဝ်း ((entry = strnsep(&xdg, &size, ':')))
		ပေႃး (size) append_path(fp, "%.*s/rc", (puur_naa_nka) size, entry);

	for (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	ၶၢဝ်း ((entry = strnsep(&path, &size, ':'))) {
		ပေႃး (size)
			append_path(fp, "%.*s%s", (puur_naa_nka)size, entry, user ? "/user" : "");
		ပေႃးဢမ်ႇ ပေႃး (user && !has_user)
			append_user_path(fp), has_user = true;
		ပေႃးဢမ်ႇ ပေႃး (!user) ၶၢဝ်း (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	ပေႃး (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	ပေႃး (!(path = getenv("RC_PATH")) && !user)
		ပွၵ်ႈ;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	ပေႃး (path)
		append_rc_path(fp, path, user);
	ပေႃးဢမ်ႇ
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	ၶၢဝ်း ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	puur_naa_nka _is_user = -1;

	ပေႃး (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ပွၵ်ႈ _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	ပေႃး (is_user)
		ပွၵ်ႈ;

	ပေႃး (!rc_dirs.set)
		atexit(free_rc_dirs);
	ပေႃးဢမ်ႇ
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	ပေႃး ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ပေႃးဢမ်ႇ ပေႃး ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ပေႃးဢမ်ႇ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	ပေႃး (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	ပေႃး (!rc_path.entries)
		load_dynamic_path(false);
	ပွၵ်ႈ rc_path.entries;
}

size_t
rc_scriptdirfds(const puur_naa_nka **fds)
{
	static size_t len = 0;

	ပေႃး (len == 0) {
		ပေႃး (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		for (size_t i = 0; i < rc_path.count; i++) {
			puur_naa_nka fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			ပေႃး (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ပွၵ်ႈ len;
}

const var_na *
rc_sysconfdir(void)
{
	ပွၵ်ႈ RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ပွၵ်ႈ rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ပွၵ်ႈ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	ပေႃး (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ပွၵ်ႈ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(puur_naa_nka targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	for (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		ပေႃး (r < 0) {
			free(buffer);
			ပွၵ်ႈ -errno;
		}
		ပေႃး ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ပွၵ်ႈ r;
		}
		free(buffer);
		ပေႃး (bufsiz >= SSIZE_MAX - PATH_MAX)
			ပွၵ်ႈ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	ပေႃး (!service)
		ပွၵ်ႈ NULL;

	ပေႃး (service[0] == '/')
		ပွၵ်ႈ xstrdup(service);

	/* First check started services */
	ပေႃး (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ပေႃး (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ပွၵ်ႈ buffer;
	ပေႃး (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		ပေႃး (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ပွၵ်ႈ buffer;

	for (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		ပေႃး (stat(file, &buf) == 0)
			ပွၵ်ႈ file;
		free(file);
	}

	ပွၵ်ႈ NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	ပေႃး (!service) {
		errno = EINVAL;
		ပွၵ်ႈ false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	ပေႃး (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ပွၵ်ႈ false;
	}

	ပေႃး (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ပွၵ်ႈ false;
	}

	ပေႃး (stat(file, &buf) == 0) {
		ပေႃး (buf.st_mode & S_IXUGO)
			retval = true;
		ပေႃးဢမ်ႇ
			errno = ENOEXEC;
	}
	free(file);
	ပွၵ်ႈ retval;
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

	ပေႃး (!(svc = rc_service_resolve(service)))
		ပွၵ်ႈ NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	ပေႃး (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ပွၵ်ႈ NULL;
	}

	ပေႃး (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		ၶၢဝ်း ((token = strsep(&p, " ")))
			ပေႃး (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ပွၵ်ႈ commands;
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

	ပေႃး (!(svc = rc_service_resolve(service)))
		ပွၵ်ႈ NULL;

	ပေႃး (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	ပေႃး ((fp = popen(cmd, "r"))) {
		ပေႃး (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ပွၵ်ႈ desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ပွၵ်ႈ r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	puur_naa_nka i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	puur_naa_nka serrno;

	ပေႃး (!init)
		ပွၵ်ႈ false;

	base = basename_c(service);
	ပေႃး (state != RC_SERVICE_STOPPED) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		ပေႃး (access(init, F_OK) != 0) {
			free(init);
			ပွၵ်ႈ false;
		}

		unlinkat(state_dirfd, base, 0);
		ပေႃး ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ပွၵ်ႈ false;
		}
	}

	ပေႃး (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ပွၵ်ႈ true;
	}

	/* Remove any old states now */
	for (i = 0; rc_service_state_names[i].name; i++) {
		puur_naa_nka state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		ပေႃး (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			ပေႃး (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			ပေႃး (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			ပေႃး ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				ပေႃး (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ပွၵ်ႈ false;
				}
				skip_wasinactive = true;
			}
			ပေႃး (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ပွၵ်ႈ false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	ပေႃး (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	ပေႃး (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	ပေႃး (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ပေႃး (dp) {
			sa_nracanaa dirent *d;
			ၶၢဝ်း ((d = readdir(dp))) {
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
	ပွၵ်ႈ true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	puur_naa_nka state = RC_SERVICE_STOPPED;
	puur_naa_nka i;

	for (i = 0; rc_service_state_names[i].name; i++) {
		ပေႃး (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			ပေႃး (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			ပေႃးဢမ်ႇ
				state |= rc_service_state_names[i].state;
		}
	}

	ပေႃး (state & RC_SERVICE_STARTED)
		ပေႃး (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	ပေႃး (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		ပေႃး (dp) {
			sa_nracanaa dirent *d;
			ၶၢဝ်း ((d = readdir(dp))) {
				ပေႃး (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ပွၵ်ႈ state;
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

	ပွၵ်ႈ buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	puur_naa_nka optdirfd;
	FILE *fp;

	ပေႃး (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ပွၵ်ႈ false;

	ပေႃး ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ပွၵ်ႈ false;

	ပေႃး (!value) {
		unlinkat(optdirfd, option, 0);
	} ပေႃးဢမ်ႇ ပေႃး ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} ပေႃးဢမ်ႇ {
		ret = false;
	}

	close(optdirfd);
	ပွၵ်ႈ ret;
}

bool
rc_service_schedule_start(const var_na *service, const var_na *service_to_start)
{
	const var_na *base_to_start = basename_c(service_to_start);
	const var_na *base = basename_c(service);
	bool retval;
	puur_naa_nka schedfd;
	var_na *init;

	/* service may be a provided service, like net */
	ပေႃး (!service || !rc_service_exists(service_to_start))
		ပွၵ်ႈ false;

	ပေႃး (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ပွၵ်ႈ false;

	ပေႃး ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ပွၵ်ႈ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ပွၵ်ႈ retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ပွၵ်ႈ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	ပေႃး (!runlevel) {
		const puur_naa_nka *scriptdirfds;
		list = rc_stringlist_new();
		for (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ပွၵ်ႈ list;
	}

	/* These special levels never contain any services */
	ပေႃး (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ပွၵ်ႈ rc_stringlist_new();

	ပွၵ်ႈ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ပွၵ်ႈ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	ပေႃး (state != RC_SERVICE_SCHEDULED)
		ပွၵ်ႈ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	ပေႃး (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ပွၵ်ႈ list;

	ၶၢဝ်း ((d = readdir(dp))) {
		ပေႃး (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ပွၵ်ႈ list;
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

	ပေႃး (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ပွၵ်ႈ false;
	}

	ပေႃး (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ပွၵ်ႈ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	ပေႃး (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		ပေႃး (path == NULL) {
			free(init);
			ပွၵ်ႈ false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		ပေႃး (!retval) {
			free(init);
			errno = EPERM;
			ပွၵ်ႈ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	ပေႃး (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ပွၵ်ႈ retval;
}

bool
rc_service_delete(const var_na *runlevel, const var_na *service)
{
	var_na *file;
	puur_naa_nka r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	ပွၵ်ႈ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	ပေႃး (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ပွၵ်ႈ list;

	ၶၢဝ်း ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		ပေႃး (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ပွၵ်ႈ list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ပွၵ်ႈ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	ပေႃး (env[name_len] != '=')
		ပွၵ်ႈ errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (puur_naa_nka) name_len, env);

	ပေႃး (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ပွၵ်ႈ false;

	ပေႃး (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ပွၵ်ႈ false;

	fputs(&env[name_len + 1], fp);

	ပွၵ်ႈ fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	puur_naa_nka dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ပေႃး (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ပွၵ်ႈ false;

	ပေႃး (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ပွၵ်ႈ false;

	fputs(value, fp);

	ပွၵ်ႈ fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const var_na *service, const var_na *name, var_na **buf)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)], *value;
	puur_naa_nka dfd = rc_dirfd(RC_DIR_ENVIRON);
	sa_nracanaa stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	ပေႃး (fstatat(dfd, var, &stat, 0) == -1)
		ပွၵ်ႈ -1;

	ပေႃး (!buf)
		ပွၵ်ႈ stat.st_size;

	ပေႃး (!(value = xrealloc(*buf, stat.st_size + 1)))
		ပွၵ်ႈ -1;

	ပေႃး (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ပွၵ်ႈ -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ပွၵ်ႈ count;
}
