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

#ضمن <assert.h>
#ضمن <helpers.h>
#ضمن <ctype.h>
#ضمن <dirent.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <libgen.h>
#ضمن <limits.h>
#ضمن <regex.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/param.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

#ضمن "queue.h"
#ضمن "librc.h"
#ضمن "misc.h"
#ضمن "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = عدم;
ساكن صحيح dirfds[RC_DIR_MAX];

صحيح rc_dirfd(تعداد rc_dir dir) {
	صحيح flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	صحيح dirfd;

	אם (dir < 0 || dir >= RC_DIR_MAX)
		החזר -1;
	אם (dirfds[dir] <= 0) {
		بدل (dir) {
		حالة RC_DIR_SYSCONF:
			החזר dirfds[dir] = open(rc_sysconfdir(), flags);
		حالة RC_DIR_USRCONF:
			החזר dirfds[dir] = open(rc_usrconfdir(), flags);
		حالة RC_DIR_RUNLEVEL:
			החזר dirfds[dir] = open(rc_runleveldir(), flags);
		حالة RC_DIR_SVCDIR:
			אם (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				החזר -1;
			החזר dirfds[dir] = open(rc_svcdir(), flags);
		افتراضي:
			أكد(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			אם ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				אם (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					החזר -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			اكسر;
		}
	}
	החזר dirfds[dir];
}

فراغ clear_dirfds(فراغ) {
	עבור (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		אם (dirfds[i] <= 0)
			استمر;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
ساكن RC_STRINGLIST *
ls_dir(صحيح targetfd, ثابت حرف *dir, صحيح options)
{
	DIR *dp;
	بنية dirent *d;
	RC_STRINGLIST *list = عدم;
	بنية stat buf;
	size_t l;

	list = rc_stringlist_new();

	אם (!(dp = do_opendirat(targetfd, dir)))
		החזר list;

	כלעוד (((d = readdir(dp)) != عدم)) {
		אם (d->d_name[0] == '.')
			استمر;
		אם (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			אם (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				استمر;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			אם (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				استمر;
		}
		אם (options & LS_DIR) {
			אם (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				استمر;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	החזר list;
}

ساكن bool
rm_dir(صحيح targetfd, ثابت حرف *pathname, bool top)
{
	bool retval = true;
	بنية dirent *d;
	بنية stat s;
	DIR *dp;

	אם (!(dp = do_opendirat(targetfd, pathname)))
		החזר false;

	כלעוד ((d = readdir(dp))) {
		אם (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			استمر;
		אם (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			اكسر;
		}
		אם (S_ISDIR(s.st_mode)) {
			אם (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				اكسر;
			}
		} אחרת {
			אם (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				اكسر;
			}
		}
	}
	closedir(dp);

	אם (!retval)
		החזר false;

	אם (top && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		החזר false;

	החזר true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
ساكن bool
file_regex(ثابت حرف *file, ثابت حرف *regex)
{
	FILE *fp;
	حرف *line = عدم;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	صحيح result;

	אם (!(fp = fopen(file, "r")))
		החזר false;

	אם ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		حرف buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, حجم(buf));
		fprintf(stderr, "file_regex: %s", buf);
		החזר false;
	}

	כלעוד ((len = xgetline(&line, &size, fp)) != -1) {
		حرف *str = line;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		עשה {
			אם (regexec(&re, str, 0, عدم, 0) == 0)
				اذهب found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			כלעוד (str < line + len && *str == '\0')
				str++;
		} כלעוד (str < line + len);
	}
	retval = false;
found:
	fclose(fp);
	حرر(line);
	regfree(&re);

	החזר retval;
}
#endif


ساكن ثابت حرف *
get_systype(فراغ)
{
	حرف *systype = rc_conf_value("rc_sys");
	אם (systype) {
		حرف *s = systype;
		/* Convert to uppercase */
		כלעוד (s && *s) {
			אם (islower((unsigned حرف) *s))
				*s = toupper((unsigned حرف) *s);
			s++;
		}
	}
	החזר systype;
}

ساكن ثابت حرف *
detect_container(ثابت حرف *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	אם (systype) {
		אם (strcmp(systype, RC_SYS_NONE) == 0)
		       החזר عدم;
		אם (strcmp(systype, RC_SYS_JAIL) == 0)
			החזר RC_SYS_JAIL;
		אם (strcmp(systype, RC_SYS_PODMAN) == 0)
			החזר RC_SYS_PODMAN;
	}

	صحيح jailed = 0;
	size_t len = حجم(jailed);

	אם (sysctlbyname("security.jail.jailed", &jailed, &len, عدم, 0) == 0)
		אם (jailed == 1)
			החזר RC_SYS_JAIL;

	אם (access("/var/run/.containerenv", F_OK) == 0)
		החזר RC_SYS_PODMAN;
#endif

#ifdef __linux__
	אם (systype) {
		אם (strcmp(systype, RC_SYS_NONE) == 0)
			החזר عدم;
		אם (strcmp(systype, RC_SYS_UML) == 0)
			החזר RC_SYS_UML;
		אם (strcmp(systype, RC_SYS_VSERVER) == 0)
			החזר RC_SYS_VSERVER;
		אם (strcmp(systype, RC_SYS_OPENVZ) == 0)
			החזר RC_SYS_OPENVZ;
		אם (strcmp(systype, RC_SYS_LXC) == 0)
			החזר RC_SYS_LXC;
		אם (strcmp(systype, RC_SYS_RKT) == 0)
				החזר RC_SYS_RKT;
		אם (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				החזר RC_SYS_SYSTEMD_NSPAWN;
		אם (strcmp(systype, RC_SYS_DOCKER) == 0)
				החזר RC_SYS_DOCKER;
		אם (strcmp(systype, RC_SYS_PODMAN) == 0)
			החזר RC_SYS_PODMAN;
		אם (strcmp(systype, RC_SYS_WSL) == 0)
			החזר RC_SYS_WSL;
	}
	אם (file_regex("/proc/cpuinfo", "UML"))
		החזר RC_SYS_UML;
	אחרת אם (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		החזר RC_SYS_VSERVER;
	אחרת אם (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		החזר RC_SYS_OPENVZ;
	אחרת אם (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		החזר RC_SYS_OPENVZ; /* old test */
	אחרת אם (file_regex("/proc/1/environ", "container=lxc"))
		החזר RC_SYS_LXC;
	אחרת אם (file_regex("/proc/1/environ", "container=rkt"))
		החזר RC_SYS_RKT;
	אחרת אם (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		החזר RC_SYS_SYSTEMD_NSPAWN;
	אחרת אם (access("/run/.containerenv", F_OK) == 0)
		החזר RC_SYS_PODMAN;
	אחרת אם (access("/.dockerenv", F_OK) == 0)
		החזר RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	אחרת אם (file_regex("/proc/1/environ", "container=docker"))
		החזר RC_SYS_DOCKER;
	אחרת אם (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		החזר RC_SYS_WSL;
	אחרת אם (getenv("WSL_DISTRO_NAME"))
		החזר RC_SYS_WSL;
	אחרת אם (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		החזר RC_SYS_WSL;
#endif

	החזר عدم;
}

ساكن ثابت حرف *
detect_vm(ثابت حرف *systype RC_UNUSED)
{
#ifdef __NetBSD__
	אם (systype) {
		אם (strcmp(systype, RC_SYS_NONE) == 0)
			החזר عدم;
		אם (strcmp(systype, RC_SYS_XEN0) == 0)
			החזר RC_SYS_XEN0;
		אם (strcmp(systype, RC_SYS_XENU) == 0)
			החזר RC_SYS_XENU;
	}
	אם (access("/kern/xen/privcmd", F_OK) == 0)
		החזר RC_SYS_XEN0;
	אם (access("/kern/xen", F_OK) == 0)
		החזר RC_SYS_XENU;
#endif

#ifdef __linux__
	אם (systype) {
		אם (strcmp(systype, RC_SYS_NONE) == 0)
			החזר عدم;
		אם (strcmp(systype, RC_SYS_XEN0) == 0)
			החזר RC_SYS_XEN0;
		אם (strcmp(systype, RC_SYS_XENU) == 0)
			החזר RC_SYS_XENU;
	}
	אם (access("/proc/xen", F_OK) == 0) {
		אם (file_regex("/proc/xen/capabilities", "control_d"))
			החזר RC_SYS_XEN0;
		החזר RC_SYS_XENU;
	}
#endif

	החזר عدم;
}

ثابت حرف *
rc_sys(فراغ)
{
	ثابت حرف *systype;
	ثابت حرف *sys;

	systype = get_systype();
	sys = detect_container(systype);
	אם (!sys) {
		sys = detect_vm(systype);
	}

	החזר sys;
}

ساكن ثابت حرف *
rc_parse_service_state(RC_SERVICE state)
{
	صحيح i;

	עבור (i = 0; rc_service_state_names[i].name; i++) {
		אם (rc_service_state_names[i].state == state)
			החזר rc_service_state_names[i].name;
	}
	החזר عدم;
}

ساكن صحيح
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	עבור (size_t i = 0; rc_service_state_names[i].name; i++)
		אם (rc_service_state_names[i].state == state)
			החזר rc_service_state_names[i].dir;
	החזר -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ساكن فراغ
get_runlevel_chain(ثابت حرف *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ثابت حرف *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	אם (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		החזר;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	אם (rc_stringlist_find(level_list, runlevel))
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
		אם (rc_stringlist_find(ancestor_list, nextlevel)) {
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
rc_runlevel_starting(فراغ)
{
	החזר faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(فراغ)
{
	החזר faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(فراغ)
{
	החזר ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

حرف *
rc_runlevel_get(فراغ)
{
	FILE *fp;
	حرف *runlevel = عدم;
	size_t i;

	אם ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(حجم(حرف) * PATH_MAX);
		אם (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			אם (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} אחרת
			*runlevel = '\0';
		fclose(fp);
	}

	אם (!runlevel || !*runlevel) {
		حرر(runlevel);
		runlevel = عدم;
	}

	החזר runlevel;
}

bool
rc_runlevel_set(ثابت حرف *runlevel)
{
	FILE *fp;

	אם (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		החזר false;

	fputs(runlevel, fp);
	fclose(fp);
	החזר true;
}

bool
rc_runlevel_exists(ثابت حرف *runlevel)
{
	بنية stat buf;

	אם (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		החזר false;
	אם (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		החזר true;
	החזר false;
}

bool
rc_runlevel_stack(ثابت حرف *dst, ثابت حرف *src)
{
	حرف *d, *s;
	صحيح r;

	אם (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		החזר false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	حرر(d);
	حرر(s);
	החזר (r == 0);
}

bool
rc_runlevel_unstack(ثابت حرف *dst, ثابت حرف *src)
{
	حرف *path;
	صحيح r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	حرر(path);
	החזר (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ثابت حرف *runlevel)
{
	RC_STRINGLIST *stack, *ancestor_list;
	stack = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, stack, ancestor_list);
	rc_stringlist_free(ancestor_list);
	החזר stack;
}

ساكن ثابت حرف *config_path[] = { _RC_PATH, عدم };

ساكن بنية {
	bool set;
	حرف *svcdir, *confdir, *runleveldir;
} rc_dirs;

ساكن بنية {
	حرف *buffer;
	size_t count;
	صحيح *fds;
	ثابت حرف **entries, **allocd;
} rc_path;

ساكن فراغ
free_rc_dirs(فراغ)
{
#define X(ptr) ptr = (حرر(ptr), عدم)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = عدم;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.count = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
ساكن فراغ
append_path(FILE *fp, ثابت حرف *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.count++;
}

ساكن ثابت حرف *
strnsep(ثابت حرف **str, size_t *size, ثابت حرف sep)
{
	ثابت حرف *ret = *str;

	אם (!ret)
		החזר عدم;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	אם (!**str)
		*str = عدم;
	אחרת
		*str += 1;

	החזר ret;
}

ساكن ثابت حرف *
strnullsep(ثابت حرف **str, size_t *size)
{
	ثابت حرف *ret = *str;
	size_t len;

	אם (!ret)
		החזר عدم;

	len = strnlen(ret, *size - 1) + 1;

	אם ((*size -= len))
		*str += len;
	אחרת
		*str = عدم;

	החזר ret;
}

ساكن فراغ
append_user_path(FILE *fp)
{
	ثابت حرف *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	אם (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	אחרת כלעוד ((entry = strnsep(&xdg, &size, ':')))
		אם (size) append_path(fp, "%.*s/rc", (صحيح) size, entry);

	עבור (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

ساكن فراغ
append_rc_path(FILE *fp, ثابت حرف *path, bool user)
{
	bool has_user = false;
	ثابت حرف *entry;
	size_t size;

	כלעוד ((entry = strnsep(&path, &size, ':'))) {
		אם (size)
			append_path(fp, "%.*s%s", (صحيح)size, entry, user ? "/user" : "");
		אחרת אם (user && !has_user)
			append_user_path(fp), has_user = true;
		אחרת אם (!user) כלעוד (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

ساكن فراغ
load_dynamic_path(bool user)
{
	ثابت حرف *path, *entry;
	size_t size, count = 0;
	FILE *fp;

	אם (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.count = ARRAY_SIZE(config_path) - 1;

	אם (!(path = getenv("RC_PATH")) && !user)
		החזר;

	rc_path.count = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	אם (path)
		append_rc_path(fp, path, user);
	אחרת
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.count + 1, حجم(*rc_path.entries));

	path = rc_path.buffer;

	כלעוד ((entry = strnullsep(&path, &size)) && count < rc_path.count)
		rc_path.entries[count++] = entry;
	rc_path.entries[rc_path.count] = عدم;
}

ساكن bool is_user = false;

bool
rc_is_user(فراغ)
{
	صحيح _is_user = -1;

	אם (_is_user == -1) {
		حرف *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	החזר _is_user;
}

فراغ
rc_set_user(فراغ)
{
	حرف *env;

	אם (is_user)
		החזר;

	אם (!rc_dirs.set)
		atexit(free_rc_dirs);
	אחרת
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	אם ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} אחרת אם ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} אחרת {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	אם (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

ثابت حرف * ثابت *
rc_scriptdirs(فراغ)
{
	אם (!rc_path.entries)
		load_dynamic_path(false);
	החזר rc_path.entries;
}

size_t
rc_scriptdirfds(ثابت صحيح **fds)
{
	ساكن size_t len = 0;

	אם (len == 0) {
		אם (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.count * حجم(*rc_path.fds));
		עבור (size_t i = 0; i < rc_path.count; i++) {
			صحيح fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			אם (fd == -1)
				استمر;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	החזר len;
}

ثابت حرف *
rc_sysconfdir(فراغ)
{
	החזר RC_SYSCONFDIR;
}

ثابت حرف *
rc_usrconfdir(فراغ)
{
	החזר rc_dirs.confdir ? rc_dirs.confdir : عدم;
}

ثابت حرف *
rc_runleveldir(فراغ)
{
	החזר rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ثابت حرف *
rc_svcdir(فراغ)
{
	ساكن ثابت حرف *svcdir;

	אם (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	החזר rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ساكن ssize_t
safe_readlink(صحيح targetfd, ثابت حرف *path, حرف **buf, size_t bufsiz)
{
	حرف *buffer;
	ssize_t r;

	עבור (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		אם (r < 0) {
			حرر(buffer);
			החזר -errno;
		}
		אם ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			החזר r;
		}
		حرر(buffer);
		אם (bufsiz >= SSIZE_MAX - PATH_MAX)
			החזר -ENOMEM;
	}
}

/* Resolve a service name to its full path */
حرف *
rc_service_resolve(ثابت حرف *service)
{
	حرف *buffer;
	بنية stat buf;

	אם (!service)
		החזר عدم;

	אם (service[0] == '/')
		החזר xstrdup(service);

	/* First check started services */
	אם (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		אם (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			החזר buffer;
	אם (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		אם (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			החזר buffer;

	עבור (ثابت حرف * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		حرف *file = عدم;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		אם (stat(file, &buf) == 0)
			החזר file;
		حرر(file);
	}

	החזר عدم;
}

bool
rc_service_exists(ثابت حرف *service)
{
	حرف *file;
	bool retval = false;
	size_t len;
	بنية stat buf;

	אם (!service) {
		errno = EINVAL;
		החזר false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	אם (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		החזר false;
	}

	אם (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		החזר false;
	}

	אם (stat(file, &buf) == 0) {
		אם (buf.st_mode & S_IXUGO)
			retval = true;
		אחרת
			errno = ENOEXEC;
	}
	حرر(file);
	החזר retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ثابت حرف *service)
{
	حرف *svc;
	حرف *cmd = عدم;
	حرف *buffer = عدم;
	size_t len = 0;
	RC_STRINGLIST *commands = عدم;
	حرف *token;
	حرف *p;
	FILE *fp;

	אם (!(svc = rc_service_resolve(service)))
		החזר عدم;

	xasprintf(&cmd, OPTSTR, svc);
	حرر(svc);

	אם (!(fp = popen(cmd, "r"))) {
		حرر(cmd);
		החזר عدم;
	}

	אם (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		כלעוד ((token = strsep(&p, " ")))
			אם (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	حرر(buffer);
	حرر(cmd);
	החזר commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
حرف *
rc_service_description(ثابت حرف *service, ثابت حرف *option)
{
	حرف *svc;
	حرف *cmd;
	حرف *desc = عدم;
	size_t size = 0;
	FILE *fp;

	אם (!(svc = rc_service_resolve(service)))
		החזר عدم;

	אם (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	حرر(svc);
	אם ((fp = popen(cmd, "r"))) {
		אם (xgetline(&desc, &size, fp) == -1) {
			حرر(desc);
			desc = عدم;
		}
		pclose(fp);
	}
	حرر(cmd);
	החזר desc;
}

bool
rc_service_in_runlevel(ثابت حرف *service, ثابت حرف *runlevel)
{
	حرف *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	حرر(file);
	החזר r;
}

bool
rc_service_mark(ثابت حرف *service, ثابت RC_SERVICE state)
{
	صحيح i = 0;
	ثابت حرف *base;
	حرف *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	صحيح serrno;

	אם (!init)
		החזר false;

	base = basename_c(service);
	אם (state != RC_SERVICE_STOPPED) {
		صحيح state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		אם (access(init, F_OK) != 0) {
			حرر(init);
			החזר false;
		}

		unlinkat(state_dirfd, base, 0);
		אם ((symlinkat(init, state_dirfd, base)) != 0) {
			حرر(init);
			החזר false;
		}
	}

	אם (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		حرر(init);
		החזר true;
	}

	/* Remove any old states now */
	עבור (i = 0; rc_service_state_names[i].name; i++) {
		صحيح state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		אם (s == state)
			استمر;

		بدل (s) {
		حالة RC_SERVICE_STOPPED:
		حالة RC_SERVICE_HOTPLUGGED:
		حالة RC_SERVICE_SCHEDULED:
			استمر;
		حالة RC_SERVICE_WASINACTIVE:
			אם (skip_wasinactive)
				استمر;
			/* fall-through */
		افتراضي:
			אם (faccessat(state_dirfd, base, F_OK, 0) != 0)
				استمر;
			אם ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				אם (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					حرر(init);
					החזר false;
				}
				skip_wasinactive = true;
			}
			אם (unlinkat(state_dirfd, base, 0) == -1) {
				حرر(init);
				החזר false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	אם (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	אם (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	אם (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		אם (dp) {
			بنية dirent *d;
			כלעוד ((d = readdir(dp))) {
				حرف *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				حرر(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	حرر(init);
	החזר true;
}

RC_SERVICE
rc_service_state(ثابت حرف *service)
{
	ثابت حرف *base = basename_c(service);
	صحيح state = RC_SERVICE_STOPPED;
	صحيح i;

	עבור (i = 0; rc_service_state_names[i].name; i++) {
		אם (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			אם (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			אחרת
				state |= rc_service_state_names[i].state;
		}
	}

	אם (state & RC_SERVICE_STARTED)
		אם (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	אם (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		אם (dp) {
			بنية dirent *d;
			כלעוד ((d = readdir(dp))) {
				אם (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					اكسر;
				}
			}

			closedir(dp);
		}
	}

	החזר state;
}

حرف *
rc_service_value_get(ثابت حرف *service, ثابت حرف *option)
{
	حرف *buffer = عدم;
	size_t len = 0;
	حرف *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	حرر(file);

	החזר buffer;
}

bool
rc_service_value_set(ثابت حرف *service, ثابت حرف *option, ثابت حرف *value)
{
	bool ret = true;
	صحيح optdirfd;
	FILE *fp;

	אם (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		החזר false;

	אם ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		החזר false;

	אם (!value) {
		unlinkat(optdirfd, option, 0);
	} אחרת אם ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", value);
		fclose(fp);
	} אחרת {
		ret = false;
	}

	close(optdirfd);
	החזר ret;
}

bool
rc_service_schedule_start(ثابت حرف *service, ثابت حرف *service_to_start)
{
	ثابت حرف *base_to_start = basename_c(service_to_start);
	ثابت حرف *base = basename_c(service);
	bool retval;
	صحيح schedfd;
	حرف *init;

	/* service may be a provided service, like net */
	אם (!service || !rc_service_exists(service_to_start))
		החזר false;

	אם (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		החזר false;

	אם ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		החזר false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	حرر(init);
	החזר retval;
}

bool
rc_service_schedule_clear(ثابت حرف *service)
{
	החזר !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ثابت حرف *runlevel)
{
	RC_STRINGLIST *list = عدم;

	אם (!runlevel) {
		ثابت صحيح *scriptdirfds;
		list = rc_stringlist_new();
		עבור (size_t i = 0, count = rc_scriptdirfds(&scriptdirfds); i < count; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		החזר list;
	}

	/* These special levels never contain any services */
	אם (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		החזר rc_stringlist_new();

	החזר ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ثابت حرف *runlevel)
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
	החזר list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	بنية dirent *d;
	DIR *dp;

	אם (state != RC_SERVICE_SCHEDULED)
		החזר ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	אם (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		החזר list;

	כלעוד ((d = readdir(dp))) {
		אם (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			استمر;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	החזר list;
}

bool
rc_service_add(ثابت حرف *runlevel, ثابت حرف *service)
{
	bool retval;
	حرف *init;
	حرف *file;
	حرف *path;
	حرف *binit = عدم;
	حرف *i;

	אם (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		החזר false;
	}

	אם (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		החזר false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	אם (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), عدم);
		אם (path == عدم) {
			حرر(init);
			החזר false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		حرر(path);
		אם (!retval) {
			حرر(init);
			errno = EPERM;
			החזר false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	אם (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		اذهب out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	حرر(file);

out:
	حرر(binit);
	حرر(init);
	החזר retval;
}

bool
rc_service_delete(ثابت حرف *runlevel, ثابت حرف *service)
{
	حرف *file;
	صحيح r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	حرر(file);
	החזר (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ثابت حرف *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	بنية dirent *d;
	DIR *dp;

	אם (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		החזר list;

	כלעוד ((d = readdir(dp))) {
		حرف *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		אם (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		حرر(file);
	}

	החזר list;
}

RC_STRINGLIST *
rc_services_scheduled(ثابت حرف *service)
{
	החזר ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ثابت حرف *service, ثابت حرف *env)
{
	size_t name_len = strcspn(env, "=");
	حرف var[strlen(service) + حجم("/") + name_len];
	صحيح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	אם (env[name_len] != '=')
		החזר errno = EINVAL, false;

	snprintf(var, حجم(var), "%s/%.*s", service, (صحيح) name_len, env);

	אם (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		החזר false;

	אם (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		החזר false;

	fputs(&env[name_len + 1], fp);

	החזר fclose(fp) == 0;
}

bool
rc_service_setenv(ثابت حرف *service, ثابت حرف *name, ثابت حرف *value)
{
	حرف var[strlen(service) + حجم("/") + strlen(name)];
	صحيح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, حجم(var), "%s/%s", service, name);

	אם (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		החזר false;

	אם (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		החזר false;

	fputs(value, fp);

	החזר fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ثابت حرف *service, ثابت حرف *name, حرف **buf)
{
	حرف var[strlen(service) + حجم("/") + strlen(name)], *value;
	صحيح dfd = rc_dirfd(RC_DIR_ENVIRON);
	بنية stat stat;
	size_t count;
	FILE *fp;

	snprintf(var, حجم(var), "%s/%s", service, name);

	אם (fstatat(dfd, var, &stat, 0) == -1)
		החזר -1;

	אם (!buf)
		החזר stat.st_size;

	אם (!(value = xrealloc(*buf, stat.st_size + 1)))
		החזר -1;

	אם (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		החזר -1;

	count = fread(value, 1, stat.st_size, fp);
	value[count] = '\0';

	fclose(fp);
	*buf = value;
	החזר count;
}
