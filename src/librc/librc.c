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
static පූර්ණ_සංඛ්‍යා dirfds[RC_DIR_MAX];

පූර්ණ_සංඛ්‍යා rc_dirfd(ga_nanaa rc_dir dir) {
	පූර්ණ_සංඛ්‍යා flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	පූර්ණ_සංඛ්‍යා dirfd;

	නම් (dir < 0 || dir >= RC_DIR_MAX)
		ආපසු -1;
	නම් (dirfds[dir] <= 0) {
		switch (dir) {
		pak_sa RC_DIR_SYSCONF:
			ආපසු dirfds[dir] = open(rc_sysconfdir(), flags);
		pak_sa RC_DIR_USRCONF:
			ආපසු dirfds[dir] = open(rc_usrconfdir(), flags);
		pak_sa RC_DIR_RUNLEVEL:
			ආපසු dirfds[dir] = open(rc_runleveldir(), flags);
		pak_sa RC_DIR_SVCDIR:
			නම් (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				ආපසු -1;
			ආපසු dirfds[dir] = open(rc_svcdir(), flags);
		default:
			assert(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			නම් ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				නම් (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					ආපසු -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			bha_nga;
		}
	}
	ආපසු dirfds[dir];
}

void clear_dirfds(void) {
	සඳහා (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		නම් (dirfds[i] <= 0)
			continue;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
static RC_STRINGLIST *
ls_dir(පූර්ණ_සංඛ්‍යා targetfd, const var_na *dir, පූර්ණ_සංඛ්‍යා options)
{
	DIR *dp;
	sa_nracanaa dirent *d;
	RC_STRINGLIST *list = NULL;
	sa_nracanaa stat buf;
	size_t l;

	list = rc_stringlist_new();

	නම් (!(dp = do_opendirat(targetfd, dir)))
		ආපසු list;

	අතරතුර (((d = readdir(dp)) != NULL)) {
		නම් (d->d_name[0] == '.')
			continue;
		නම් (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			නම් (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			නම් (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				continue;
		}
		නම් (options & LS_DIR) {
			නම් (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				continue;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	ආපසු list;
}

static bool
rm_dir(පූර්ණ_සංඛ්‍යා targetfd, const var_na *pathname, bool top)
{
	bool retval = true;
	sa_nracanaa dirent *d;
	sa_nracanaa stat s;
	DIR *dp;

	නම් (!(dp = do_opendirat(targetfd, pathname)))
		ආපසු false;

	අතරතුර ((d = readdir(dp))) {
		නම් (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;
		නම් (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			bha_nga;
		}
		නම් (S_ISDIR(s.st_mode)) {
			නම් (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				bha_nga;
			}
		} නැතහොත් {
			නම් (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				bha_nga;
			}
		}
	}
	closedir(dp);

	නම් (!retval)
		ආපසු false;

	නම් (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		ආපසු false;

	ආපසු true;
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
	පූර්ණ_සංඛ්‍යා result;

	නම් (!(fp = fopen(file, "r")))
		ආපසු false;

	නම් ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		var_na buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, parimaa_na(buf));
		fprintf(stderr, "file_regex: %s", buf);
		ආපසු false;
	}

	අතරතුර ((len = xgetline(&line, &size, fp)) != -1) {
		var_na *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		do {
			නම් (regexec(&re, str, 0, NULL, 0) == 0)
				goto found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			අතරතුර (str < line + len && *str == '\0')
				str++;
		} අතරතුර (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	free(line);
	regfree(&re);

	ආපසු retval;
}
#endif


static const var_na *
get_systype(void)
{
	var_na *systype = rc_conf_value("rc_sys");
	නම් (systype) {
		var_na *s = systype;
		/* Convert to uppercase */
		අතරතුර (s && *s) {
			නම් (islower((unsigned var_na) *s))
				*s = toupper((unsigned var_na) *s);
			s++;
		}
	}
	ආපසු systype;
}

static const var_na *
detect_container(const var_na *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	නම් (systype) {
		නම් (strcmp(systype, RC_SYS_NONE) == 0)
		       ආපසු NULL;
		නම් (strcmp(systype, RC_SYS_JAIL) == 0)
			ආපසු RC_SYS_JAIL;
		නම් (strcmp(systype, RC_SYS_PODMAN) == 0)
			ආපසු RC_SYS_PODMAN;
	}

	පූර්ණ_සංඛ්‍යා jailed = 0;
	size_t len = parimaa_na(jailed);

	නම් (sysctlbyname("security.jail.jailed", &jailed, &len, NULL, 0) == 0)
		නම් (jailed == 1)
			ආපසු RC_SYS_JAIL;

	නම් (access("/var/run/.containerenv", F_OK) == 0)
		ආපසු RC_SYS_PODMAN;
#endif

#ifdef __linux__
	නම් (systype) {
		නම් (strcmp(systype, RC_SYS_NONE) == 0)
			ආපසු NULL;
		නම් (strcmp(systype, RC_SYS_UML) == 0)
			ආපසු RC_SYS_UML;
		නම් (strcmp(systype, RC_SYS_VSERVER) == 0)
			ආපසු RC_SYS_VSERVER;
		නම් (strcmp(systype, RC_SYS_OPENVZ) == 0)
			ආපසු RC_SYS_OPENVZ;
		නම් (strcmp(systype, RC_SYS_LXC) == 0)
			ආපසු RC_SYS_LXC;
		නම් (strcmp(systype, RC_SYS_RKT) == 0)
				ආපසු RC_SYS_RKT;
		නම් (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				ආපසු RC_SYS_SYSTEMD_NSPAWN;
		නම් (strcmp(systype, RC_SYS_DOCKER) == 0)
				ආපසු RC_SYS_DOCKER;
		නම් (strcmp(systype, RC_SYS_PODMAN) == 0)
			ආපසු RC_SYS_PODMAN;
		නම් (strcmp(systype, RC_SYS_WSL) == 0)
			ආපසු RC_SYS_WSL;
	}
	නම් (file_regex("/proc/cpuinfo", "UML"))
		ආපසු RC_SYS_UML;
	නැතහොත් නම් (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		ආපසු RC_SYS_VSERVER;
	නැතහොත් නම් (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		ආපසු RC_SYS_OPENVZ;
	නැතහොත් නම් (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		ආපසු RC_SYS_OPENVZ; /* old test */
	නැතහොත් නම් (file_regex("/proc/1/environ", "container=lxc"))
		ආපසු RC_SYS_LXC;
	නැතහොත් නම් (file_regex("/proc/1/environ", "container=rkt"))
		ආපසු RC_SYS_RKT;
	නැතහොත් නම් (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		ආපසු RC_SYS_SYSTEMD_NSPAWN;
	නැතහොත් නම් (access("/run/.containerenv", F_OK) == 0)
		ආපසු RC_SYS_PODMAN;
	නැතහොත් නම් (access("/.dockerenv", F_OK) == 0)
		ආපසු RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	නැතහොත් නම් (file_regex("/proc/1/environ", "container=docker"))
		ආපසු RC_SYS_DOCKER;
	නැතහොත් නම් (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		ආපසු RC_SYS_WSL;
	නැතහොත් නම් (getenv("WSL_DISTRO_NAME"))
		ආපසු RC_SYS_WSL;
	නැතහොත් නම් (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		ආපසු RC_SYS_WSL;
#endif

	ආපසු NULL;
}

static const var_na *
detect_vm(const var_na *systype RC_UNUSED)
{
#ifdef __NetBSD__
	නම් (systype) {
		නම් (strcmp(systype, RC_SYS_NONE) == 0)
			ආපසු NULL;
		නම් (strcmp(systype, RC_SYS_XEN0) == 0)
			ආපසු RC_SYS_XEN0;
		නම් (strcmp(systype, RC_SYS_XENU) == 0)
			ආපසු RC_SYS_XENU;
	}
	නම් (access("/kern/xen/privcmd", F_OK) == 0)
		ආපසු RC_SYS_XEN0;
	නම් (access("/kern/xen", F_OK) == 0)
		ආපසු RC_SYS_XENU;
#endif

#ifdef __linux__
	නම් (systype) {
		නම් (strcmp(systype, RC_SYS_NONE) == 0)
			ආපසු NULL;
		නම් (strcmp(systype, RC_SYS_XEN0) == 0)
			ආපසු RC_SYS_XEN0;
		නම් (strcmp(systype, RC_SYS_XENU) == 0)
			ආපසු RC_SYS_XENU;
	}
	නම් (access("/proc/xen", F_OK) == 0) {
		නම් (file_regex("/proc/xen/capabilities", "control_d"))
			ආපසු RC_SYS_XEN0;
		ආපසු RC_SYS_XENU;
	}
#endif

	ආපසු NULL;
}

const var_na *
rc_sys(void)
{
	const var_na *systype;
	const var_na *sys;

	systype = get_systype();
	sys = detect_container(systype);
	නම් (!sys) {
		sys = detect_vm(systype);
	}

	ආපසු sys;
}

static const var_na *
rc_parse_service_state(RC_SERVICE state)
{
	පූර්ණ_සංඛ්‍යා i;

	සඳහා (i = 0; rc_service_state_names[i].name; i++) {
		නම් (rc_service_state_names[i].state == state)
			ආපසු rc_service_state_names[i].name;
	}
	ආපසු NULL;
}

static පූර්ණ_සංඛ්‍යා
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	සඳහා (size_t i = 0; rc_service_state_names[i].name; i++)
		නම් (rc_service_state_names[i].state == state)
			ආපසු rc_service_state_names[i].dir;
	ආපසු -1;
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
	නම් (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		ආපසු;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	නම් (rc_stringlist_find(level_list, runlevel))
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
		නම් (rc_stringlist_find(ancestor_list, nextlevel)) {
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
	ආපසු faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(void)
{
	ආපසු faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(void)
{
	ආපසු ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

var_na *
rc_runlevel_get(void)
{
	FILE *fp;
	var_na *runlevel = NULL;
	size_t i;

	නම් ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(parimaa_na(var_na) * PATH_MAX);
		නම් (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			නම් (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} නැතහොත්
			*runlevel = '\0';
		fclose(fp);
	}

	නම් (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = NULL;
	}

	ආපසු runlevel;
}

bool
rc_runlevel_set(const var_na *runlevel)
{
	FILE *fp;

	නම් (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		ආපසු false;

	fputs(runlevel, fp);
	fclose(fp);
	ආපසු true;
}

bool
rc_runlevel_exists(const var_na *runlevel)
{
	sa_nracanaa stat buf;

	නම් (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		ආපසු false;
	නම් (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		ආපසු true;
	ආපසු false;
}

bool
rc_runlevel_stack(const var_na *dst, const var_na *src)
{
	var_na *d, *s;
	පූර්ණ_සංඛ්‍යා r;

	නම් (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		ආපසු false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	free(d);
	free(s);
	ආපසු (r == 0);
}

bool
rc_runlevel_unstack(const var_na *dst, const var_na *src)
{
	var_na *path;
	පූර්ණ_සංඛ්‍යා r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	free(path);
	ආපසු (r == 0);
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
	ආපසු stack;
}

static const var_na *config_path[] = { _RC_PATH, NULL };

static sa_nracanaa {
	bool set;
	var_na *svcdir, *confdir, *runleveldir;
} rc_dirs;

static sa_nracanaa {
	var_na *buffer;
	size_t count;
	පූර්ණ_සංඛ්‍යා *fds;
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

	නම් (!ret)
		ආපසු NULL;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	නම් (!**str)
		*str = NULL;
	නැතහොත්
		*str += 1;

	ආපසු ret;
}

static const var_na *
strnullsep(const var_na **str, size_t *size)
{
	const var_na *ret = *str;
	size_t len;

	නම් (!ret)
		ආපසු NULL;

	len = strnlen(ret, *size - 1) + 1;

	නම් ((*size -= len))
		*str += len;
	නැතහොත්
		*str = NULL;

	ආපසු ret;
}

static void
append_user_path(FILE *fp)
{
	const var_na *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	නම් (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	නැතහොත් අතරතුර ((entry = strnsep(&xdg, &size, ':')))
		නම් (size) append_path(fp, "%.*s/rc", (පූර්ණ_සංඛ්‍යා) size, entry);

	සඳහා (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

static void
append_rc_path(FILE *fp, const var_na *path, bool user)
{
	bool has_user = false;
	const var_na *entry;
	size_t size;

	අතරතුර ((entry = strnsep(&path, &size, ':'))) {
		නම් (size)
			append_path(fp, "%.*s%s", (පූර්ණ_සංඛ්‍යා)size, entry, user ? "/user" : "");
		නැතහොත් නම් (user && !has_user)
			append_user_path(fp), has_user = true;
		නැතහොත් නම් (!user) අතරතුර (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

static void
load_dynamic_path(bool user)
{
	const var_na *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	නම් (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	නම් (!(path = getenv("RC_PATH")) && !user)
		ආපසු;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	නම් (path)
		append_rc_path(fp, path, user);
	නැතහොත්
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, parimaa_na(*rc_path.entries));

	path = rc_path.buffer;

	අතරතුර ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = NULL;
}

static bool is_user = false;

bool
rc_is_user(void)
{
	පූර්ණ_සංඛ්‍යා _is_user = -1;

	නම් (_is_user == -1) {
		var_na *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	ආපසු _is_user;
}

void
rc_set_user(void)
{
	var_na *env;

	නම් (is_user)
		ආපසු;

	නම් (!rc_dirs.set)
		atexit(free_rc_dirs);
	නැතහොත්
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	නම් ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} නැතහොත් නම් ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} නැතහොත් {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	නම් (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	නම් (!rc_path.entries)
		load_dynamic_path(false);
	ආපසු rc_path.entries;
}

size_t
rc_scriptdirfds(const පූර්ණ_සංඛ්‍යා **fds)
{
	static size_t len = 0;

	නම් (len == 0) {
		නම් (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * parimaa_na(*rc_path.fds));
		සඳහා (size_t i = 0; i < rc_path.count; i++) {
			පූර්ණ_සංඛ්‍යා fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			නම් (fd == -1)
				continue;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	ආපසු len;
}

const var_na *
rc_sysconfdir(void)
{
	ආපසු RC_SYSCONFDIR;
}

const var_na *
rc_usrconfdir(void)
{
	ආපසු rc_dirs.confdir ? rc_dirs.confdir : NULL;
}

const var_na *
rc_runleveldir(void)
{
	ආපසු rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

const var_na *
rc_svcdir(void)
{
	static const var_na *svcdir;

	නම් (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	ආපසු rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

static ssize_t
safe_readlink(පූර්ණ_සංඛ්‍යා targetfd, const var_na *path, var_na **buf, size_t bufsiz)
{
	var_na *buffer;
	ssize_t r;

	සඳහා (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		නම් (r < 0) {
			free(buffer);
			ආපසු -errno;
		}
		නම් ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			ආපසු r;
		}
		free(buffer);
		නම් (bufsiz >= SSIZE_MAX - PATH_MAX)
			ආපසු -ENOMEM;
	}
}

/* Resolve a service name to its full path */
var_na *
rc_service_resolve(const var_na *service)
{
	var_na *buffer;
	sa_nracanaa stat buf;

	නම් (!service)
		ආපසු NULL;

	නම් (service[0] == '/')
		ආපසු xstrdup(service);

	/* First check started services */
	නම් (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		නම් (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			ආපසු buffer;
	නම් (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		නම් (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			ආපසු buffer;

	සඳහා (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		var_na *file = NULL;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		නම් (stat(file, &buf) == 0)
			ආපසු file;
		free(file);
	}

	ආපසු NULL;
}

bool
rc_service_exists(const var_na *service)
{
	var_na *file;
	bool retval = false;
	size_t len;
	sa_nracanaa stat buf;

	නම් (!service) {
		errno = EINVAL;
		ආපසු false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	නම් (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		ආපසු false;
	}

	නම් (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		ආපසු false;
	}

	නම් (stat(file, &buf) == 0) {
		නම් (buf.st_mode & S_IXUGO)
			retval = true;
		නැතහොත්
			errno = ENOEXEC;
	}
	free(file);
	ආපසු retval;
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

	නම් (!(svc = rc_service_resolve(service)))
		ආපසු NULL;

	xasprintf(&cmd, OPTSTR, svc);
	free(svc);

	නම් (!(fp = popen(cmd, "r"))) {
		free(cmd);
		ආපසු NULL;
	}

	නම් (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		අතරතුර ((token = strsep(&p, " ")))
			නම් (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	free(buffer);
	free(cmd);
	ආපසු commands;
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

	නම් (!(svc = rc_service_resolve(service)))
		ආපසු NULL;

	නම් (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	free(svc);
	නම් ((fp = popen(cmd, "r"))) {
		නම් (xgetline(&desc, &size, fp) == -1) {
			free(desc);
			desc = NULL;
		}
		pclose(fp);
	}
	free(cmd);
	ආපසු desc;
}

bool
rc_service_in_runlevel(const var_na *service, const var_na *runlevel)
{
	var_na *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	free(file);
	ආපසු r;
}

bool
rc_service_mark(const var_na *service, const RC_SERVICE state)
{
	පූර්ණ_සංඛ්‍යා i = 0;
	const var_na *base;
	var_na *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	පූර්ණ_සංඛ්‍යා serrno;

	නම් (!init)
		ආපසු false;

	base = basename_c(service);
	නම් (state != RC_SERVICE_STOPPED) {
		පූර්ණ_සංඛ්‍යා state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		නම් (access(init, F_OK) != 0) {
			free(init);
			ආපසු false;
		}

		unlinkat(state_dirfd, base, 0);
		නම් ((symlinkat(init, state_dirfd, base)) != 0) {
			free(init);
			ආපසු false;
		}
	}

	නම් (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		free(init);
		ආපසු true;
	}

	/* Remove any old states now */
	සඳහා (i = 0; rc_service_state_names[i].name; i++) {
		පූර්ණ_සංඛ්‍යා state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		නම් (s == state)
			continue;

		switch (s) {
		pak_sa RC_SERVICE_STOPPED:
		pak_sa RC_SERVICE_HOTPLUGGED:
		pak_sa RC_SERVICE_SCHEDULED:
			continue;
		pak_sa RC_SERVICE_WASINACTIVE:
			නම් (skip_wasinactive)
				continue;
			/* fall-through */
		default:
			නම් (faccessat(state_dirfd, base, F_OK, 0) != 0)
				continue;
			නම් ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				නම් (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					free(init);
					ආපසු false;
				}
				skip_wasinactive = true;
			}
			නම් (unlinkat(state_dirfd, base, 0) == -1) {
				free(init);
				ආපසු false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	නම් (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	නම් (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	නම් (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		නම් (dp) {
			sa_nracanaa dirent *d;
			අතරතුර ((d = readdir(dp))) {
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
	ආපසු true;
}

RC_SERVICE
rc_service_state(const var_na *service)
{
	const var_na *base = basename_c(service);
	පූර්ණ_සංඛ්‍යා state = RC_SERVICE_STOPPED;
	පූර්ණ_සංඛ්‍යා i;

	සඳහා (i = 0; rc_service_state_names[i].name; i++) {
		නම් (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			නම් (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			නැතහොත්
				state |= rc_service_state_names[i].state;
		}
	}

	නම් (state & RC_SERVICE_STARTED)
		නම් (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	නම් (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		නම් (dp) {
			sa_nracanaa dirent *d;
			අතරතුර ((d = readdir(dp))) {
				නම් (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					bha_nga;
				}
			}

			closedir(dp);
		}
	}

	ආපසු state;
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

	ආපසු buffer;
}

bool
rc_service_value_set(const var_na *service, const var_na *option, const var_na *value)
{
	bool ret = true;
	පූර්ණ_සංඛ්‍යා optdirfd;
	FILE *fp;

	නම් (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		ආපසු false;

	නම් ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		ආපසු false;

	නම් (!value) {
		unlinkat(optdirfd, option, 0);
	} නැතහොත් නම් ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} නැතහොත් {
		ret = false;
	}

	close(optdirfd);
	ආපසු ret;
}

bool
rc_service_schedule_start(const var_na *service, const var_na *service_to_start)
{
	const var_na *base_to_start = basename_c(service_to_start);
	const var_na *base = basename_c(service);
	bool retval;
	පූර්ණ_සංඛ්‍යා schedfd;
	var_na *init;

	/* service may be a provided service, like net */
	නම් (!service || !rc_service_exists(service_to_start))
		ආපසු false;

	නම් (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		ආපසු false;

	නම් ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		ආපසු false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	free(init);
	ආපසු retval;
}

bool
rc_service_schedule_clear(const var_na *service)
{
	ආපසු !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(const var_na *runlevel)
{
	RC_STRINGLIST *list = NULL;

	නම් (!runlevel) {
		const පූර්ණ_සංඛ්‍යා *scriptdirfds;
		list = rc_stringlist_new();
		සඳහා (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		ආපසු list;
	}

	/* These special levels never contain any services */
	නම් (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		ආපසු rc_stringlist_new();

	ආපසු ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
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
	ආපසු list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	sa_nracanaa dirent *d;
	DIR *dp;

	නම් (state != RC_SERVICE_SCHEDULED)
		ආපසු ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	නම් (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		ආපසු list;

	අතරතුර ((d = readdir(dp))) {
		නම් (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			continue;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	ආපසු list;
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

	නම් (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		ආපසු false;
	}

	නම් (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		ආපසු false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	නම් (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), NULL);
		නම් (path == NULL) {
			free(init);
			ආපසු false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		free(path);
		නම් (!retval) {
			free(init);
			errno = EPERM;
			ආපසු false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	නම් (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	ආපසු retval;
}

bool
rc_service_delete(const var_na *runlevel, const var_na *service)
{
	var_na *file;
	පූර්ණ_සංඛ්‍යා r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	free(file);
	ආපසු (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(const var_na *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	sa_nracanaa dirent *d;
	DIR *dp;

	නම් (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		ආපසු list;

	අතරතුර ((d = readdir(dp))) {
		var_na *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		නම් (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		free(file);
	}

	ආපසු list;
}

RC_STRINGLIST *
rc_services_scheduled(const var_na *service)
{
	ආපසු ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(const var_na *service, const var_na *env)
{
	size_t name_len = strcspn(env, "=");
	var_na var[strlen(service) + parimaa_na("/") + name_len];
	පූර්ණ_සංඛ්‍යා dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	නම් (env[name_len] != '=')
		ආපසු errno = EINVAL, false;

	snprintf(var, parimaa_na(var), "%s/%.*s", service, (පූර්ණ_සංඛ්‍යා) name_len, env);

	නම් (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ආපසු false;

	නම් (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ආපසු false;

	fputs(&env[name_len + 1], fp);

	ආපසු fclose(fp) == 0;
}

bool
rc_service_setenv(const var_na *service, const var_na *name, const var_na *value)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)];
	පූර්ණ_සංඛ්‍යා dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	නම් (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		ආපසු false;

	නම් (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		ආපසු false;

	fputs(value, fp);

	ආපසු fclose(fp) == 0;
}

ssize_t
rc_service_getenv(const var_na *service, const var_na *name, var_na **buf)
{
	var_na var[strlen(service) + parimaa_na("/") + strlen(name)], *value;
	පූර්ණ_සංඛ්‍යා dfd = rc_dirfd(RC_DIR_ENVIRON);
	sa_nracanaa stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, parimaa_na(var), "%s/%s", service, name);

	නම් (fstatat(dfd, var, &stat, 0) == -1)
		ආපසු -1;

	නම් (!buf)
		ආපසු stat.st_size;

	නම් (!(value = xrealloc(*buf, stat.st_size + 1)))
		ආපසු -1;

	නම් (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		ආපසු -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	ආපසු count;
}
