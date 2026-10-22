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

#شامل <assert.h>
#شامل <helpers.h>
#شامل <ctype.h>
#شامل <dirent.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <libgen.h>
#شامل <limits.h>
#شامل <regex.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/param.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

#شامل "queue.h"
#شامل "librc.h"
#شامل "misc.h"
#شامل "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = پوچ;
ایستا صحیح dirfds[RC_DIR_MAX];

صحیح rc_dirfd(شمارش rc_dir dir) {
	صحیح flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	صحیح dirfd;

	اگر (dir < 0 || dir >= RC_DIR_MAX)
		بازگشت -1;
	اگر (dirfds[dir] <= 0) {
		انتخاب (dir) {
		حالت RC_DIR_SYSCONF:
			بازگشت dirfds[dir] = open(rc_sysconfdir(), flags);
		حالت RC_DIR_USRCONF:
			بازگشت dirfds[dir] = open(rc_usrconfdir(), flags);
		حالت RC_DIR_RUNLEVEL:
			بازگشت dirfds[dir] = open(rc_runleveldir(), flags);
		حالت RC_DIR_SVCDIR:
			اگر (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				بازگشت -1;
			بازگشت dirfds[dir] = open(rc_svcdir(), flags);
		پیشفرض:
			أكد(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			اگر ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				اگر (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					بازگشت -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			بشکن;
		}
	}
	بازگشت dirfds[dir];
}

تهی clear_dirfds(تهی) {
	برای (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		اگر (dirfds[i] <= 0)
			ادامه;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
ایستا RC_STRINGLIST *
ls_dir(صحیح targetfd, ثابت نویسه *dir, صحیح options)
{
	DIR *dp;
	ساختار dirent *d;
	RC_STRINGLIST *list = پوچ;
	ساختار stat buf;
	size_t l;

	list = rc_stringlist_new();

	اگر (!(dp = do_opendirat(targetfd, dir)))
		بازگشت list;

	وقتی (((d = readdir(dp)) != پوچ)) {
		اگر (d->d_name[0] == '.')
			ادامه;
		اگر (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			اگر (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				ادامه;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			اگر (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				ادامه;
		}
		اگر (options & LS_DIR) {
			اگر (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				ادامه;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	بازگشت list;
}

ایستا bool
rm_dir(صحیح targetfd, ثابت نویسه *pathname, bool بالاترین)
{
	bool retval = true;
	ساختار dirent *d;
	ساختار stat s;
	DIR *dp;

	اگر (!(dp = do_opendirat(targetfd, pathname)))
		بازگشت false;

	وقتی ((d = readdir(dp))) {
		اگر (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ادامه;
		اگر (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			بشکن;
		}
		اگر (S_ISDIR(s.st_mode)) {
			اگر (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				بشکن;
			}
		} وگرنه {
			اگر (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				بشکن;
			}
		}
	}
	closedir(dp);

	اگر (!retval)
		بازگشت false;

	اگر (بالاترین && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		بازگشت false;

	بازگشت true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
ایستا bool
file_regex(ثابت نویسه *file, ثابت نویسه *regex)
{
	FILE *fp;
	نویسه *سطر = پوچ;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	صحیح result;

	اگر (!(fp = fopen(file, "r")))
		بازگشت false;

	اگر ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		نویسه buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, اندازه(buf));
		fprintf(stderr, "file_regex: %s", buf);
		بازگشت false;
	}

	وقتی ((len = xgetline(&سطر, &size, fp)) != -1) {
		نویسه *str = سطر;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		انجامبده {
			اگر (regexec(&re, str, 0, پوچ, 0) == 0)
				برو found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			وقتی (str < سطر + len && *str == '\0')
				str++;
		} وقتی (str < سطر + len);
	}
	retval = false;
found:
	fclose(fp);
	آزاد(سطر);
	regfree(&re);

	بازگشت retval;
}
#endif


ایستا ثابت نویسه *
get_systype(تهی)
{
	نویسه *systype = rc_conf_مقدار("rc_sys");
	اگر (systype) {
		نویسه *s = systype;
		/* Convert to uppercase */
		وقتی (s && *s) {
			اگر (islower((unsigned نویسه) *s))
				*s = toupper((unsigned نویسه) *s);
			s++;
		}
	}
	بازگشت systype;
}

ایستا ثابت نویسه *
detect_container(ثابت نویسه *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
		       بازگشت پوچ;
		اگر (strcmp(systype, RC_SYS_JAIL) == 0)
			بازگشت RC_SYS_JAIL;
		اگر (strcmp(systype, RC_SYS_PODMAN) == 0)
			بازگشت RC_SYS_PODMAN;
	}

	صحیح jailed = 0;
	size_t len = اندازه(jailed);

	اگر (sysctlbyname("security.jail.jailed", &jailed, &len, پوچ, 0) == 0)
		اگر (jailed == 1)
			بازگشت RC_SYS_JAIL;

	اگر (access("/var/run/.containerenv", F_OK) == 0)
		بازگشت RC_SYS_PODMAN;
#endif

#ifdef __linux__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
			بازگشت پوچ;
		اگر (strcmp(systype, RC_SYS_UML) == 0)
			بازگشت RC_SYS_UML;
		اگر (strcmp(systype, RC_SYS_VSERVER) == 0)
			بازگشت RC_SYS_VSERVER;
		اگر (strcmp(systype, RC_SYS_OPENVZ) == 0)
			بازگشت RC_SYS_OPENVZ;
		اگر (strcmp(systype, RC_SYS_LXC) == 0)
			بازگشت RC_SYS_LXC;
		اگر (strcmp(systype, RC_SYS_RKT) == 0)
				بازگشت RC_SYS_RKT;
		اگر (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				بازگشت RC_SYS_SYSTEMD_NSPAWN;
		اگر (strcmp(systype, RC_SYS_DOCKER) == 0)
				بازگشت RC_SYS_DOCKER;
		اگر (strcmp(systype, RC_SYS_PODMAN) == 0)
			بازگشت RC_SYS_PODMAN;
		اگر (strcmp(systype, RC_SYS_WSL) == 0)
			بازگشت RC_SYS_WSL;
	}
	اگر (file_regex("/proc/cpuinfo", "UML"))
		بازگشت RC_SYS_UML;
	وگرنه اگر (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		بازگشت RC_SYS_VSERVER;
	وگرنه اگر (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		بازگشت RC_SYS_OPENVZ;
	وگرنه اگر (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		بازگشت RC_SYS_OPENVZ; /* old test */
	وگرنه اگر (file_regex("/proc/1/environ", "container=lxc"))
		بازگشت RC_SYS_LXC;
	وگرنه اگر (file_regex("/proc/1/environ", "container=rkt"))
		بازگشت RC_SYS_RKT;
	وگرنه اگر (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		بازگشت RC_SYS_SYSTEMD_NSPAWN;
	وگرنه اگر (access("/run/.containerenv", F_OK) == 0)
		بازگشت RC_SYS_PODMAN;
	وگرنه اگر (access("/.dockerenv", F_OK) == 0)
		بازگشت RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	وگرنه اگر (file_regex("/proc/1/environ", "container=docker"))
		بازگشت RC_SYS_DOCKER;
	وگرنه اگر (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		بازگشت RC_SYS_WSL;
	وگرنه اگر (getenv("WSL_DISTRO_NAME"))
		بازگشت RC_SYS_WSL;
	وگرنه اگر (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		بازگشت RC_SYS_WSL;
#endif

	بازگشت پوچ;
}

ایستا ثابت نویسه *
detect_vm(ثابت نویسه *systype RC_UNUSED)
{
#ifdef __NetBSD__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
			بازگشت پوچ;
		اگر (strcmp(systype, RC_SYS_XEN0) == 0)
			بازگشت RC_SYS_XEN0;
		اگر (strcmp(systype, RC_SYS_XENU) == 0)
			بازگشت RC_SYS_XENU;
	}
	اگر (access("/kern/xen/privcmd", F_OK) == 0)
		بازگشت RC_SYS_XEN0;
	اگر (access("/kern/xen", F_OK) == 0)
		بازگشت RC_SYS_XENU;
#endif

#ifdef __linux__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
			بازگشت پوچ;
		اگر (strcmp(systype, RC_SYS_XEN0) == 0)
			بازگشت RC_SYS_XEN0;
		اگر (strcmp(systype, RC_SYS_XENU) == 0)
			بازگشت RC_SYS_XENU;
	}
	اگر (access("/proc/xen", F_OK) == 0) {
		اگر (file_regex("/proc/xen/capabilities", "control_d"))
			بازگشت RC_SYS_XEN0;
		بازگشت RC_SYS_XENU;
	}
#endif

	بازگشت پوچ;
}

ثابت نویسه *
rc_sys(تهی)
{
	ثابت نویسه *systype;
	ثابت نویسه *sys;

	systype = get_systype();
	sys = detect_container(systype);
	اگر (!sys) {
		sys = detect_vm(systype);
	}

	بازگشت sys;
}

ایستا ثابت نویسه *
rc_parse_service_state(RC_SERVICE state)
{
	صحیح i;

	برای (i = 0; rc_service_state_names[i].name; i++) {
		اگر (rc_service_state_names[i].state == state)
			بازگشت rc_service_state_names[i].name;
	}
	بازگشت پوچ;
}

ایستا صحیح
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	برای (size_t i = 0; rc_service_state_names[i].name; i++)
		اگر (rc_service_state_names[i].state == state)
			بازگشت rc_service_state_names[i].dir;
	بازگشت -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ایستا تهی
get_runlevel_chain(ثابت نویسه *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ثابت نویسه *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	اگر (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		بازگشت;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	اگر (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->مقدار;

		/* Check for loop */
		اگر (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->مقدار);
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
rc_runlevel_starting(تهی)
{
	بازگشت faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(تهی)
{
	بازگشت faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(تهی)
{
	بازگشت ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

نویسه *
rc_runlevel_get(تهی)
{
	FILE *fp;
	نویسه *runlevel = پوچ;
	size_t i;

	اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(اندازه(نویسه) * PATH_MAX);
		اگر (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			اگر (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} وگرنه
			*runlevel = '\0';
		fclose(fp);
	}

	اگر (!runlevel || !*runlevel) {
		آزاد(runlevel);
		runlevel = پوچ;
	}

	بازگشت runlevel;
}

bool
rc_runlevel_set(ثابت نویسه *runlevel)
{
	FILE *fp;

	اگر (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		بازگشت false;

	fputs(runlevel, fp);
	fclose(fp);
	بازگشت true;
}

bool
rc_runlevel_exists(ثابت نویسه *runlevel)
{
	ساختار stat buf;

	اگر (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		بازگشت false;
	اگر (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		بازگشت true;
	بازگشت false;
}

bool
rc_runlevel_پشته(ثابت نویسه *dst, ثابت نویسه *src)
{
	نویسه *d, *s;
	صحیح r;

	اگر (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		بازگشت false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	آزاد(d);
	آزاد(s);
	بازگشت (r == 0);
}

bool
rc_runlevel_unstack(ثابت نویسه *dst, ثابت نویسه *src)
{
	نویسه *path;
	صحیح r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	آزاد(path);
	بازگشت (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ثابت نویسه *runlevel)
{
	RC_STRINGLIST *پشته, *ancestor_list;
	پشته = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, پشته, ancestor_list);
	rc_stringlist_free(ancestor_list);
	بازگشت پشته;
}

ایستا ثابت نویسه *config_path[] = { _RC_PATH, پوچ };

ایستا ساختار {
	bool set;
	نویسه *svcdir, *confdir, *runleveldir;
} rc_dirs;

ایستا ساختار {
	نویسه *buffer;
	size_t شمار;
	صحیح *fds;
	ثابت نویسه **entries, **allocd;
} rc_path;

ایستا تهی
free_rc_dirs(تهی)
{
#define X(ptr) ptr = (آزاد(ptr), پوچ)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = پوچ;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.شمار = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
ایستا تهی
append_path(FILE *fp, ثابت نویسه *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.شمار++;
}

ایستا ثابت نویسه *
strnsep(ثابت نویسه **str, size_t *size, ثابت نویسه sep)
{
	ثابت نویسه *ret = *str;

	اگر (!ret)
		بازگشت پوچ;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	اگر (!**str)
		*str = پوچ;
	وگرنه
		*str += 1;

	بازگشت ret;
}

ایستا ثابت نویسه *
strnullsep(ثابت نویسه **str, size_t *size)
{
	ثابت نویسه *ret = *str;
	size_t len;

	اگر (!ret)
		بازگشت پوچ;

	len = strnlen(ret, *size - 1) + 1;

	اگر ((*size -= len))
		*str += len;
	وگرنه
		*str = پوچ;

	بازگشت ret;
}

ایستا تهی
append_user_path(FILE *fp)
{
	ثابت نویسه *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	اگر (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	وگرنه وقتی ((entry = strnsep(&xdg, &size, ':')))
		اگر (size) append_path(fp, "%.*s/rc", (صحیح) size, entry);

	برای (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

ایستا تهی
append_rc_path(FILE *fp, ثابت نویسه *path, bool user)
{
	bool has_user = false;
	ثابت نویسه *entry;
	size_t size;

	وقتی ((entry = strnsep(&path, &size, ':'))) {
		اگر (size)
			append_path(fp, "%.*s%s", (صحیح)size, entry, user ? "/user" : "");
		وگرنه اگر (user && !has_user)
			append_user_path(fp), has_user = true;
		وگرنه اگر (!user) وقتی (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

ایستا تهی
load_dynamic_path(bool user)
{
	ثابت نویسه *path, *entry;
	size_t size, شمار = 0;
	FILE *fp;

	اگر (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.شمار = ARRAY_SIZE(config_path) - 1;

	اگر (!(path = getenv("RC_PATH")) && !user)
		بازگشت;

	rc_path.شمار = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	اگر (path)
		append_rc_path(fp, path, user);
	وگرنه
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.شمار + 1, اندازه(*rc_path.entries));

	path = rc_path.buffer;

	وقتی ((entry = strnullsep(&path, &size)) && شمار < rc_path.شمار)
		rc_path.entries[شمار++] = entry;
	rc_path.entries[rc_path.شمار] = پوچ;
}

ایستا bool is_user = false;

bool
rc_is_user(تهی)
{
	صحیح _is_user = -1;

	اگر (_is_user == -1) {
		نویسه *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	بازگشت _is_user;
}

تهی
rc_set_user(تهی)
{
	نویسه *env;

	اگر (is_user)
		بازگشت;

	اگر (!rc_dirs.set)
		atexit(free_rc_dirs);
	وگرنه
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	اگر ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} وگرنه اگر ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} وگرنه {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	اگر (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

ثابت نویسه * ثابت *
rc_scriptdirs(تهی)
{
	اگر (!rc_path.entries)
		load_dynamic_path(false);
	بازگشت rc_path.entries;
}

size_t
rc_scriptdirfds(ثابت صحیح **fds)
{
	ایستا size_t len = 0;

	اگر (len == 0) {
		اگر (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.شمار * اندازه(*rc_path.fds));
		برای (size_t i = 0; i < rc_path.شمار; i++) {
			صحیح fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			اگر (fd == -1)
				ادامه;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	بازگشت len;
}

ثابت نویسه *
rc_sysconfdir(تهی)
{
	بازگشت RC_SYSCONFDIR;
}

ثابت نویسه *
rc_usrconfdir(تهی)
{
	بازگشت rc_dirs.confdir ? rc_dirs.confdir : پوچ;
}

ثابت نویسه *
rc_runleveldir(تهی)
{
	بازگشت rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ثابت نویسه *
rc_svcdir(تهی)
{
	ایستا ثابت نویسه *svcdir;

	اگر (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	بازگشت rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ایستا ssize_t
safe_readlink(صحیح targetfd, ثابت نویسه *path, نویسه **buf, size_t bufsiz)
{
	نویسه *buffer;
	ssize_t r;

	برای (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		اگر (r < 0) {
			آزاد(buffer);
			بازگشت -errno;
		}
		اگر ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			بازگشت r;
		}
		آزاد(buffer);
		اگر (bufsiz >= SSIZE_MAX - PATH_MAX)
			بازگشت -ENOMEM;
	}
}

/* Resolve a service name to its full path */
نویسه *
rc_service_resolve(ثابت نویسه *service)
{
	نویسه *buffer;
	ساختار stat buf;

	اگر (!service)
		بازگشت پوچ;

	اگر (service[0] == '/')
		بازگشت xstrdup(service);

	/* First check started services */
	اگر (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		اگر (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			بازگشت buffer;
	اگر (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		اگر (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			بازگشت buffer;

	برای (ثابت نویسه * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		نویسه *file = پوچ;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		اگر (stat(file, &buf) == 0)
			بازگشت file;
		آزاد(file);
	}

	بازگشت پوچ;
}

bool
rc_service_exists(ثابت نویسه *service)
{
	نویسه *file;
	bool retval = false;
	size_t len;
	ساختار stat buf;

	اگر (!service) {
		errno = EINVAL;
		بازگشت false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	اگر (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		بازگشت false;
	}

	اگر (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		بازگشت false;
	}

	اگر (stat(file, &buf) == 0) {
		اگر (buf.st_mode & S_IXUGO)
			retval = true;
		وگرنه
			errno = ENOEXEC;
	}
	آزاد(file);
	بازگشت retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ثابت نویسه *service)
{
	نویسه *svc;
	نویسه *cmd = پوچ;
	نویسه *buffer = پوچ;
	size_t len = 0;
	RC_STRINGLIST *commands = پوچ;
	نویسه *token;
	نویسه *p;
	FILE *fp;

	اگر (!(svc = rc_service_resolve(service)))
		بازگشت پوچ;

	xasprintf(&cmd, OPTSTR, svc);
	آزاد(svc);

	اگر (!(fp = popen(cmd, "r"))) {
		آزاد(cmd);
		بازگشت پوچ;
	}

	اگر (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		وقتی ((token = strsep(&p, " ")))
			اگر (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	آزاد(buffer);
	آزاد(cmd);
	بازگشت commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
نویسه *
rc_service_description(ثابت نویسه *service, ثابت نویسه *option)
{
	نویسه *svc;
	نویسه *cmd;
	نویسه *desc = پوچ;
	size_t size = 0;
	FILE *fp;

	اگر (!(svc = rc_service_resolve(service)))
		بازگشت پوچ;

	اگر (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	آزاد(svc);
	اگر ((fp = popen(cmd, "r"))) {
		اگر (xgetline(&desc, &size, fp) == -1) {
			آزاد(desc);
			desc = پوچ;
		}
		pclose(fp);
	}
	آزاد(cmd);
	بازگشت desc;
}

bool
rc_service_in_runlevel(ثابت نویسه *service, ثابت نویسه *runlevel)
{
	نویسه *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	آزاد(file);
	بازگشت r;
}

bool
rc_service_mark(ثابت نویسه *service, ثابت RC_SERVICE state)
{
	صحیح i = 0;
	ثابت نویسه *base;
	نویسه *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	صحیح serrno;

	اگر (!init)
		بازگشت false;

	base = basename_c(service);
	اگر (state != RC_SERVICE_STOPPED) {
		صحیح state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		اگر (access(init, F_OK) != 0) {
			آزاد(init);
			بازگشت false;
		}

		unlinkat(state_dirfd, base, 0);
		اگر ((symlinkat(init, state_dirfd, base)) != 0) {
			آزاد(init);
			بازگشت false;
		}
	}

	اگر (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		آزاد(init);
		بازگشت true;
	}

	/* Remove any old states now */
	برای (i = 0; rc_service_state_names[i].name; i++) {
		صحیح state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		اگر (s == state)
			ادامه;

		انتخاب (s) {
		حالت RC_SERVICE_STOPPED:
		حالت RC_SERVICE_HOTPLUGGED:
		حالت RC_SERVICE_SCHEDULED:
			ادامه;
		حالت RC_SERVICE_WASINACTIVE:
			اگر (skip_wasinactive)
				ادامه;
			/* fall-through */
		پیشفرض:
			اگر (faccessat(state_dirfd, base, F_OK, 0) != 0)
				ادامه;
			اگر ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				اگر (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					آزاد(init);
					بازگشت false;
				}
				skip_wasinactive = true;
			}
			اگر (unlinkat(state_dirfd, base, 0) == -1) {
				آزاد(init);
				بازگشت false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	اگر (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	اگر (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	اگر (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		اگر (dp) {
			ساختار dirent *d;
			وقتی ((d = readdir(dp))) {
				نویسه *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				آزاد(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	آزاد(init);
	بازگشت true;
}

RC_SERVICE
rc_service_state(ثابت نویسه *service)
{
	ثابت نویسه *base = basename_c(service);
	صحیح state = RC_SERVICE_STOPPED;
	صحیح i;

	برای (i = 0; rc_service_state_names[i].name; i++) {
		اگر (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			اگر (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			وگرنه
				state |= rc_service_state_names[i].state;
		}
	}

	اگر (state & RC_SERVICE_STARTED)
		اگر (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	اگر (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		اگر (dp) {
			ساختار dirent *d;
			وقتی ((d = readdir(dp))) {
				اگر (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					بشکن;
				}
			}

			closedir(dp);
		}
	}

	بازگشت state;
}

نویسه *
rc_service_مقدار_get(ثابت نویسه *service, ثابت نویسه *option)
{
	نویسه *buffer = پوچ;
	size_t len = 0;
	نویسه *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	آزاد(file);

	بازگشت buffer;
}

bool
rc_service_مقدار_set(ثابت نویسه *service, ثابت نویسه *option, ثابت نویسه *مقدار)
{
	bool ret = true;
	صحیح optdirfd;
	FILE *fp;

	اگر (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		بازگشت false;

	اگر ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		بازگشت false;

	اگر (!مقدار) {
		unlinkat(optdirfd, option, 0);
	} وگرنه اگر ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", مقدار);
		fclose(fp);
	} وگرنه {
		ret = false;
	}

	close(optdirfd);
	بازگشت ret;
}

bool
rc_service_schedule_start(ثابت نویسه *service, ثابت نویسه *service_to_start)
{
	ثابت نویسه *base_to_start = basename_c(service_to_start);
	ثابت نویسه *base = basename_c(service);
	bool retval;
	صحیح schedfd;
	نویسه *init;

	/* service may be a provided service, like net */
	اگر (!service || !rc_service_exists(service_to_start))
		بازگشت false;

	اگر (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		بازگشت false;

	اگر ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		بازگشت false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	آزاد(init);
	بازگشت retval;
}

bool
rc_service_schedule_clear(ثابت نویسه *service)
{
	بازگشت !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ثابت نویسه *runlevel)
{
	RC_STRINGLIST *list = پوچ;

	اگر (!runlevel) {
		ثابت صحیح *scriptdirfds;
		list = rc_stringlist_new();
		برای (size_t i = 0, شمار = rc_scriptdirfds(&scriptdirfds); i < شمار; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		بازگشت list;
	}

	/* These special levels never contain any services */
	اگر (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		بازگشت rc_stringlist_new();

	بازگشت ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ثابت نویسه *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *پشته;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(پشته, stacks, entries) {
		sl = rc_services_in_runlevel(پشته->مقدار);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	بازگشت list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ساختار dirent *d;
	DIR *dp;

	اگر (state != RC_SERVICE_SCHEDULED)
		بازگشت ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	اگر (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		بازگشت list;

	وقتی ((d = readdir(dp))) {
		اگر (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			ادامه;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	بازگشت list;
}

bool
rc_service_add(ثابت نویسه *runlevel, ثابت نویسه *service)
{
	bool retval;
	نویسه *init;
	نویسه *file;
	نویسه *path;
	نویسه *binit = پوچ;
	نویسه *i;

	اگر (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		بازگشت false;
	}

	اگر (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		بازگشت false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	اگر (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), پوچ);
		اگر (path == پوچ) {
			آزاد(init);
			بازگشت false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		آزاد(path);
		اگر (!retval) {
			آزاد(init);
			errno = EPERM;
			بازگشت false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	اگر (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		برو out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	آزاد(file);

out:
	آزاد(binit);
	آزاد(init);
	بازگشت retval;
}

bool
rc_service_delete(ثابت نویسه *runlevel, ثابت نویسه *service)
{
	نویسه *file;
	صحیح r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	آزاد(file);
	بازگشت (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ثابت نویسه *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ساختار dirent *d;
	DIR *dp;

	اگر (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		بازگشت list;

	وقتی ((d = readdir(dp))) {
		نویسه *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		اگر (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		آزاد(file);
	}

	بازگشت list;
}

RC_STRINGLIST *
rc_services_scheduled(ثابت نویسه *service)
{
	بازگشت ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ثابت نویسه *service, ثابت نویسه *env)
{
	size_t name_len = strcspn(env, "=");
	نویسه var[strlen(service) + اندازه("/") + name_len];
	صحیح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	اگر (env[name_len] != '=')
		بازگشت errno = EINVAL, false;

	snprintf(var, اندازه(var), "%s/%.*s", service, (صحیح) name_len, env);

	اگر (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		بازگشت false;

	اگر (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		بازگشت false;

	fputs(&env[name_len + 1], fp);

	بازگشت fclose(fp) == 0;
}

bool
rc_service_setenv(ثابت نویسه *service, ثابت نویسه *name, ثابت نویسه *مقدار)
{
	نویسه var[strlen(service) + اندازه("/") + strlen(name)];
	صحیح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, اندازه(var), "%s/%s", service, name);

	اگر (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		بازگشت false;

	اگر (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		بازگشت false;

	fputs(مقدار, fp);

	بازگشت fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ثابت نویسه *service, ثابت نویسه *name, نویسه **buf)
{
	نویسه var[strlen(service) + اندازه("/") + strlen(name)], *مقدار;
	صحیح dfd = rc_dirfd(RC_DIR_ENVIRON);
	ساختار stat stat;
	size_t شمار;
	FILE *fp;

	snprintf(var, اندازه(var), "%s/%s", service, name);

	اگر (fstatat(dfd, var, &stat, 0) == -1)
		بازگشت -1;

	اگر (!buf)
		بازگشت stat.st_size;

	اگر (!(مقدار = xrealloc(*buf, stat.st_size + 1)))
		بازگشت -1;

	اگر (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		بازگشت -1;

	شمار = fread(مقدار, 1, stat.st_size, fp);
	مقدار[شمار] = '\0';

	fclose(fp);
	*buf = مقدار;
	بازگشت شمار;
}
