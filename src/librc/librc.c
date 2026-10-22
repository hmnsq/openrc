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
#اگر_تعریف_شدہ __FreeBSD__
#  include <sys/sysctl.h>
#اگر_ختم

#اگر_غیر_تعریف_شدہ S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#اگر_ختم

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = تہی;
ساکن صحیح dirfds[RC_DIR_MAX];

صحیح rc_dirfd(شمار rc_dir dir) {
	صحیح flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	صحیح dirfd;

	اگر (dir < 0 || dir >= RC_DIR_MAX)
		واپس -1;
	اگر (dirfds[dir] <= 0) {
		انتخاب (dir) {
		حالت RC_DIR_SYSCONF:
			واپس dirfds[dir] = open(rc_sysconfdir(), flags);
		حالت RC_DIR_USRCONF:
			واپس dirfds[dir] = open(rc_usrconfdir(), flags);
		حالت RC_DIR_RUNLEVEL:
			واپس dirfds[dir] = open(rc_runleveldir(), flags);
		حالت RC_DIR_SVCDIR:
			اگر (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				واپس -1;
			واپس dirfds[dir] = open(rc_svcdir(), flags);
		طےشدہ:
			أكد(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			اگر ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				اگر (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					واپس -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			توڑو;
		}
	}
	واپس dirfds[dir];
}

خالی clear_dirfds(خالی) {
	برائے (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		اگر (dirfds[i] <= 0)
			جاری;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#تعریف_کرو LS_INITD	0x01
#تعریف_کرو LS_DIR		0x02
ساکن RC_STRINGLIST *
ls_dir(صحیح targetfd, ثابت حرف *dir, صحیح options)
{
	DIR *dp;
	ساخت dirent *d;
	RC_STRINGLIST *list = تہی;
	ساخت stat buf;
	size_t l;

	list = rc_stringlist_new();

	اگر (!(dp = do_opendirat(targetfd, dir)))
		واپس list;

	جب (((d = readdir(dp)) != تہی)) {
		اگر (d->d_name[0] == '.')
			جاری;
		اگر (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			اگر (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				جاری;

			/* .sh files are not init scripts */
			l = لڑی_لمبائی(d->d_name);
			اگر (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				جاری;
		}
		اگر (options & LS_DIR) {
			اگر (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				جاری;
		}
		rc_stringlist_جوڑو(list, d->d_name);
	}
	closedir(dp);
	واپس list;
}

ساکن bool
rm_dir(صحیح targetfd, ثابت حرف *pathname, bool چوٹی)
{
	bool retval = true;
	ساخت dirent *d;
	ساخت stat s;
	DIR *dp;

	اگر (!(dp = do_opendirat(targetfd, pathname)))
		واپس false;

	جب ((d = readdir(dp))) {
		اگر (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			جاری;
		اگر (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			توڑو;
		}
		اگر (S_ISDIR(s.st_mode)) {
			اگر (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				توڑو;
			}
		} ورنہ {
			اگر (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				توڑو;
			}
		}
	}
	closedir(dp);

	اگر (!retval)
		واپس false;

	اگر (چوٹی && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		واپس false;

	واپس true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#اگر_تعریف_شدہ __linux__
ساکن bool
file_regex(ثابت حرف *file, ثابت حرف *regex)
{
	FILE *fp;
	حرف *سطر = تہی;
	size_t سائز = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	صحیح نتیجہ;

	اگر (!(fp = fopen(file, "r")))
		واپس false;

	اگر ((نتیجہ = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		حرف buf[BUFSIZ];
		fclose(fp);
		regerror(نتیجہ, &re, buf, حجم(buf));
		fprintf(stderr, "file_regex: %s", buf);
		واپس false;
	}

	جب ((len = xgetline(&سطر, &سائز, fp)) != -1) {
		حرف *str = سطر;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		کرو {
			اگر (regexec(&re, str, 0, تہی, 0) == 0)
				جاؤ ملا;
			str += لڑی_لمبائی(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			جب (str < سطر + len && *str == '\0')
				str++;
		} جب (str < سطر + len);
	}
	retval = false;
ملا:
	fclose(fp);
	آزاد(سطر);
	regfree(&re);

	واپس retval;
}
#اگر_ختم


ساکن ثابت حرف *
get_systype(خالی)
{
	حرف *systype = rc_conf_قدر("rc_sys");
	اگر (systype) {
		حرف *s = systype;
		/* Convert to uppercase */
		جب (s && *s) {
			اگر (islower((غیرعلامتی حرف) *s))
				*s = بڑا_حرف((غیرعلامتی حرف) *s);
			s++;
		}
	}
	واپس systype;
}

ساکن ثابت حرف *
detect_container(ثابت حرف *systype RC_غیر_مستعمل)
{
#اگر_تعریف_شدہ __FreeBSD__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
		       واپس تہی;
		اگر (strcmp(systype, RC_SYS_JAIL) == 0)
			واپس RC_SYS_JAIL;
		اگر (strcmp(systype, RC_SYS_PODMAN) == 0)
			واپس RC_SYS_PODMAN;
	}

	صحیح jailed = 0;
	size_t len = حجم(jailed);

	اگر (sysctlbyname("security.jail.jailed", &jailed, &len, تہی, 0) == 0)
		اگر (jailed == 1)
			واپس RC_SYS_JAIL;

	اگر (access("/var/run/.containerenv", F_OK) == 0)
		واپس RC_SYS_PODMAN;
#اگر_ختم

#اگر_تعریف_شدہ __linux__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
			واپس تہی;
		اگر (strcmp(systype, RC_SYS_UML) == 0)
			واپس RC_SYS_UML;
		اگر (strcmp(systype, RC_SYS_VSERVER) == 0)
			واپس RC_SYS_VSERVER;
		اگر (strcmp(systype, RC_SYS_OPENVZ) == 0)
			واپس RC_SYS_OPENVZ;
		اگر (strcmp(systype, RC_SYS_LXC) == 0)
			واپس RC_SYS_LXC;
		اگر (strcmp(systype, RC_SYS_RKT) == 0)
				واپس RC_SYS_RKT;
		اگر (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				واپس RC_SYS_SYSTEMD_NSPAWN;
		اگر (strcmp(systype, RC_SYS_DOCKER) == 0)
				واپس RC_SYS_DOCKER;
		اگر (strcmp(systype, RC_SYS_PODMAN) == 0)
			واپس RC_SYS_PODMAN;
		اگر (strcmp(systype, RC_SYS_WSL) == 0)
			واپس RC_SYS_WSL;
	}
	اگر (file_regex("/proc/cpuinfo", "UML"))
		واپس RC_SYS_UML;
	ورنہ اگر (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		واپس RC_SYS_VSERVER;
	ورنہ اگر (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		واپس RC_SYS_OPENVZ;
	ورنہ اگر (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		واپس RC_SYS_OPENVZ; /* old test */
	ورنہ اگر (file_regex("/proc/1/environ", "container=lxc"))
		واپس RC_SYS_LXC;
	ورنہ اگر (file_regex("/proc/1/environ", "container=rkt"))
		واپس RC_SYS_RKT;
	ورنہ اگر (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		واپس RC_SYS_SYSTEMD_NSPAWN;
	ورنہ اگر (access("/run/.containerenv", F_OK) == 0)
		واپس RC_SYS_PODMAN;
	ورنہ اگر (access("/.dockerenv", F_OK) == 0)
		واپس RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	ورنہ اگر (file_regex("/proc/1/environ", "container=docker"))
		واپس RC_SYS_DOCKER;
	ورنہ اگر (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		واپس RC_SYS_WSL;
	ورنہ اگر (getenv("WSL_DISTRO_NAME"))
		واپس RC_SYS_WSL;
	ورنہ اگر (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		واپس RC_SYS_WSL;
#اگر_ختم

	واپس تہی;
}

ساکن ثابت حرف *
detect_vm(ثابت حرف *systype RC_غیر_مستعمل)
{
#اگر_تعریف_شدہ __NetBSD__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
			واپس تہی;
		اگر (strcmp(systype, RC_SYS_XEN0) == 0)
			واپس RC_SYS_XEN0;
		اگر (strcmp(systype, RC_SYS_XENU) == 0)
			واپس RC_SYS_XENU;
	}
	اگر (access("/kern/xen/privcmd", F_OK) == 0)
		واپس RC_SYS_XEN0;
	اگر (access("/kern/xen", F_OK) == 0)
		واپس RC_SYS_XENU;
#اگر_ختم

#اگر_تعریف_شدہ __linux__
	اگر (systype) {
		اگر (strcmp(systype, RC_SYS_NONE) == 0)
			واپس تہی;
		اگر (strcmp(systype, RC_SYS_XEN0) == 0)
			واپس RC_SYS_XEN0;
		اگر (strcmp(systype, RC_SYS_XENU) == 0)
			واپس RC_SYS_XENU;
	}
	اگر (access("/proc/xen", F_OK) == 0) {
		اگر (file_regex("/proc/xen/capabilities", "control_d"))
			واپس RC_SYS_XEN0;
		واپس RC_SYS_XENU;
	}
#اگر_ختم

	واپس تہی;
}

ثابت حرف *
rc_sys(خالی)
{
	ثابت حرف *systype;
	ثابت حرف *sys;

	systype = get_systype();
	sys = detect_container(systype);
	اگر (!sys) {
		sys = detect_vm(systype);
	}

	واپس sys;
}

ساکن ثابت حرف *
rc_parse_service_state(RC_SERVICE state)
{
	صحیح i;

	برائے (i = 0; rc_service_state_نام[i].name; i++) {
		اگر (rc_service_state_نام[i].state == state)
			واپس rc_service_state_نام[i].name;
	}
	واپس تہی;
}

ساکن صحیح
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	برائے (size_t i = 0; rc_service_state_نام[i].name; i++)
		اگر (rc_service_state_نام[i].state == state)
			واپس rc_service_state_نام[i].dir;
	واپس -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ساکن خالی
get_runlevel_chain(ثابت حرف *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *والد;
	ثابت حرف *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	اگر (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		واپس;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	اگر (rc_stringlist_ڈھونڈو(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_جوڑو(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->قدر;

		/* Check for loop */
		اگر (rc_stringlist_ڈھونڈو(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(والد, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", والد->قدر);
			باہر_نکلو(1);
		}

		/* Add new ancestor */
		rc_stringlist_جوڑو(ancestor_list, nextlevel);

		get_runlevel_chain(nextlevel, level_list, ancestor_list);

		rc_stringlist_delete(ancestor_list, nextlevel);
	}
	rc_stringlist_free(dirs);
}

bool
rc_runlevel_starting(خالی)
{
	واپس faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(خالی)
{
	واپس faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(خالی)
{
	واپس ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

حرف *
rc_runlevel_get(خالی)
{
	FILE *fp;
	حرف *runlevel = تہی;
	size_t i;

	اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(حجم(حرف) * PATH_MAX);
		اگر (fgets(runlevel, PATH_MAX, fp)) {
			i = لڑی_لمبائی(runlevel);
			اگر (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} ورنہ
			*runlevel = '\0';
		fclose(fp);
	}

	اگر (!runlevel || !*runlevel) {
		آزاد(runlevel);
		runlevel = تہی;
	}

	واپس runlevel;
}

bool
rc_runlevel_سیٹ(ثابت حرف *runlevel)
{
	FILE *fp;

	اگر (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		واپس false;

	fputs(runlevel, fp);
	fclose(fp);
	واپس true;
}

bool
rc_runlevel_exists(ثابت حرف *runlevel)
{
	ساخت stat buf;

	اگر (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		واپس false;
	اگر (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		واپس true;
	واپس false;
}

bool
rc_runlevel_ڈھیر(ثابت حرف *dst, ثابت حرف *src)
{
	حرف *d, *s;
	صحیح r;

	اگر (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		واپس false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	آزاد(d);
	آزاد(s);
	واپس (r == 0);
}

bool
rc_runlevel_unstack(ثابت حرف *dst, ثابت حرف *src)
{
	حرف *راستہ;
	صحیح r;

	xasprintf(&راستہ, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(راستہ);
	آزاد(راستہ);
	واپس (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ثابت حرف *runlevel)
{
	RC_STRINGLIST *ڈھیر, *ancestor_list;
	ڈھیر = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_جوڑو(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ڈھیر, ancestor_list);
	rc_stringlist_free(ancestor_list);
	واپس ڈھیر;
}

ساکن ثابت حرف *config_راستہ[] = { RC_راستہ, تہی };

ساکن ساخت {
	bool سیٹ;
	حرف *svcdir, *confdir, *runleveldir;
} rc_dirs;

ساکن ساخت {
	حرف *بفر;
	size_t گنتی;
	صحیح *fds;
	ثابت حرف **entries, **allocd;
} rc_راستہ;

ساکن خالی
free_rc_dirs(خالی)
{
#تعریف_کرو X(ptr) ptr = (آزاد(ptr), تہی)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_راستہ.allocd);
	/* entries alias freed allocation */
	rc_راستہ.entries = تہی;
	X(rc_راستہ.بفر);
	X(rc_راستہ.fds);
	rc_راستہ.گنتی = 0;
#تعریف_منسوخ X
}

__attribute__((__format__(__printf__, 2, 3)))
ساکن خالی
append_راستہ(FILE *fp, ثابت حرف *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_راستہ.گنتی++;
}

ساکن ثابت حرف *
strnsep(ثابت حرف **str, size_t *سائز, ثابت حرف sep)
{
	ثابت حرف *ret = *str;

	اگر (!ret)
		واپس تہی;

	*str = strchrnul(ret, sep);
	*سائز = *str - ret;
	اگر (!**str)
		*str = تہی;
	ورنہ
		*str += 1;

	واپس ret;
}

ساکن ثابت حرف *
strnullsep(ثابت حرف **str, size_t *سائز)
{
	ثابت حرف *ret = *str;
	size_t len;

	اگر (!ret)
		واپس تہی;

	len = strnlen(ret, *سائز - 1) + 1;

	اگر ((*سائز -= len))
		*str += len;
	ورنہ
		*str = تہی;

	واپس ret;
}

ساکن خالی
append_user_راستہ(FILE *fp)
{
	ثابت حرف *xdg, *اندراج_2;
	size_t سائز;

	append_راستہ(fp, "%s", rc_dirs.confdir);
	اگر (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_راستہ(fp, "/etc/xdg/rc");
	ورنہ جب ((اندراج_2 = strnsep(&xdg, &سائز, ':')))
		اگر (سائز) append_راستہ(fp, "%.*s/rc", (صحیح) سائز, اندراج_2);

	برائے (size_t i = 0; i < ARRAY_سائز(config_راستہ) - 2; i++)
		append_راستہ(fp, "%s/user", config_راستہ[i]);
	append_راستہ(fp, "%s", rc_dirs.svcdir);
}

ساکن خالی
append_rc_راستہ(FILE *fp, ثابت حرف *راستہ, bool user)
{
	bool has_user = false;
	ثابت حرف *اندراج_2;
	size_t سائز;

	جب ((اندراج_2 = strnsep(&راستہ, &سائز, ':'))) {
		اگر (سائز)
			append_راستہ(fp, "%.*s%s", (صحیح)سائز, اندراج_2, user ? "/user" : "");
		ورنہ اگر (user && !has_user)
			append_user_راستہ(fp), has_user = true;
		ورنہ اگر (!user) جب (*rc_راستہ.entries)
			append_راستہ(fp, "%s", *rc_راستہ.entries++);
	}
}

ساکن خالی
بھرو_dynamic_راستہ(bool user)
{
	ثابت حرف *راستہ, *اندراج_2;
	size_t سائز, گنتی = 0;
	FILE *fp;

	اگر (!rc_dirs.سیٹ) {
		atexit(free_rc_dirs);
		rc_dirs.سیٹ = true;
	}

	rc_راستہ.entries = config_راستہ;
	rc_راستہ.گنتی = ARRAY_سائز(config_راستہ) - 1;

	اگر (!(راستہ = getenv("RC_PATH")) && !user)
		واپس;

	rc_راستہ.گنتی = 0;
	fp = xopen_memstream(&rc_راستہ.بفر, &سائز);
	اگر (راستہ)
		append_rc_راستہ(fp, راستہ, user);
	ورنہ
		append_user_راستہ(fp);
	xclose_memstream(fp);

	rc_راستہ.allocd = rc_راستہ.entries = صفر_حافظہ_دو(rc_راستہ.گنتی + 1, حجم(*rc_راستہ.entries));

	راستہ = rc_راستہ.بفر;

	جب ((اندراج_2 = strnullsep(&راستہ, &سائز)) && گنتی < rc_راستہ.گنتی)
		rc_راستہ.entries[گنتی++] = اندراج_2;
	rc_راستہ.entries[rc_راستہ.گنتی] = تہی;
}

ساکن bool is_user = false;

bool
rc_is_user(خالی)
{
	صحیح _is_user = -1;

	اگر (_is_user == -1) {
		حرف *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	واپس _is_user;
}

خالی
rc_سیٹ_user(خالی)
{
	حرف *env;

	اگر (is_user)
		واپس;

	اگر (!rc_dirs.سیٹ)
		atexit(free_rc_dirs);
	ورنہ
		free_rc_dirs();

	is_user = true;
	rc_dirs.سیٹ = true;
	setenv("RC_USER_SERVICES", "yes", true);

	اگر ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} ورنہ اگر ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} ورنہ {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		باہر_نکلو(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	اگر (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		باہر_نکلو(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	بھرو_dynamic_راستہ(true);

	clear_dirfds();
}

ثابت حرف * ثابت *
rc_scriptdirs(خالی)
{
	اگر (!rc_راستہ.entries)
		بھرو_dynamic_راستہ(false);
	واپس rc_راستہ.entries;
}

size_t
rc_scriptdirfds(ثابت صحیح **fds)
{
	ساکن size_t len = 0;

	اگر (len == 0) {
		اگر (!rc_راستہ.entries)
			بھرو_dynamic_راستہ(false);

		rc_راستہ.fds = xmalloc(rc_راستہ.گنتی * حجم(*rc_راستہ.fds));
		برائے (size_t i = 0; i < rc_راستہ.گنتی; i++) {
			صحیح fd = open(rc_راستہ.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			اگر (fd == -1)
				جاری;

			rc_راستہ.fds[len++] = fd;
		}
	}

	*fds = rc_راستہ.fds;
	واپس len;
}

ثابت حرف *
rc_sysconfdir(خالی)
{
	واپس RC_SYSCONFDIR;
}

ثابت حرف *
rc_usrconfdir(خالی)
{
	واپس rc_dirs.confdir ? rc_dirs.confdir : تہی;
}

ثابت حرف *
rc_runleveldir(خالی)
{
	واپس rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ثابت حرف *
rc_svcdir(خالی)
{
	ساکن ثابت حرف *svcdir;

	اگر (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	واپس rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ساکن ssize_t
محفوظ_readlink(صحیح targetfd, ثابت حرف *راستہ, حرف **buf, size_t bufsiz)
{
	حرف *بفر;
	ssize_t r;

	برائے (;; bufsiz += PATH_MAX) {
		بفر = xmalloc(bufsiz);
		r = readlinkat(targetfd, راستہ, بفر, bufsiz);
		اگر (r < 0) {
			آزاد(بفر);
			واپس -errno;
		}
		اگر ((size_t)r < bufsiz) {
			بفر[r] = '\0';
			*buf = بفر;
			واپس r;
		}
		آزاد(بفر);
		اگر (bufsiz >= SSIZE_MAX - PATH_MAX)
			واپس -ENOMEM;
	}
}

/* Resolve a service name to its full path */
حرف *
rc_service_resolve(ثابت حرف *service)
{
	حرف *بفر;
	ساخت stat buf;

	اگر (!service)
		واپس تہی;

	اگر (service[0] == '/')
		واپس xstrdup(service);

	/* First check started services */
	اگر (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		اگر (محفوظ_readlink(rc_dirfd(RC_DIR_STARTED), service, &بفر, buf.st_size + 1) >= 0)
			واپس بفر;
	اگر (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		اگر (محفوظ_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &بفر, buf.st_size + 1) >= 0)
			واپس بفر;

	برائے (ثابت حرف * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		حرف *file = تہی;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		اگر (stat(file, &buf) == 0)
			واپس file;
		آزاد(file);
	}

	واپس تہی;
}

bool
rc_service_exists(ثابت حرف *service)
{
	حرف *file;
	bool retval = false;
	size_t len;
	ساخت stat buf;

	اگر (!service) {
		errno = EINVAL;
		واپس false;
	}

	len = لڑی_لمبائی(service);

	/* .sh files are not init scripts */
	اگر (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		واپس false;
	}

	اگر (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		واپس false;
	}

	اگر (stat(file, &buf) == 0) {
		اگر (buf.st_mode & S_IXUGO)
			retval = true;
		ورنہ
			errno = ENOEXEC;
	}
	آزاد(file);
	واپس retval;
}

#تعریف_کرو OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ثابت حرف *service)
{
	حرف *svc;
	حرف *cmd = تہی;
	حرف *بفر = تہی;
	size_t len = 0;
	RC_STRINGLIST *commands = تہی;
	حرف *token;
	حرف *p;
	FILE *fp;

	اگر (!(svc = rc_service_resolve(service)))
		واپس تہی;

	xasprintf(&cmd, OPTSTR, svc);
	آزاد(svc);

	اگر (!(fp = popen(cmd, "r"))) {
		آزاد(cmd);
		واپس تہی;
	}

	اگر (xgetline(&بفر, &len, fp) != -1) {
		p = بفر;
		commands = rc_stringlist_new();

		جب ((token = strsep(&p, " ")))
			اگر (token[0] != '\0')
				rc_stringlist_جوڑو(commands, token);
	}

	pclose(fp);
	آزاد(بفر);
	آزاد(cmd);
	واپس commands;
}

#تعریف_کرو DESCSTR ". '%s'; echo \"${description%s%s}\""
حرف *
rc_service_description(ثابت حرف *service, ثابت حرف *option)
{
	حرف *svc;
	حرف *cmd;
	حرف *desc = تہی;
	size_t سائز = 0;
	FILE *fp;

	اگر (!(svc = rc_service_resolve(service)))
		واپس تہی;

	اگر (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	آزاد(svc);
	اگر ((fp = popen(cmd, "r"))) {
		اگر (xgetline(&desc, &سائز, fp) == -1) {
			آزاد(desc);
			desc = تہی;
		}
		pclose(fp);
	}
	آزاد(cmd);
	واپس desc;
}

bool
rc_service_اندر_runlevel(ثابت حرف *service, ثابت حرف *runlevel)
{
	حرف *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	آزاد(file);
	واپس r;
}

bool
rc_service_mark(ثابت حرف *service, ثابت RC_SERVICE state)
{
	صحیح i = 0;
	ثابت حرف *بنیاد;
	حرف *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	صحیح serrno;

	اگر (!init)
		واپس false;

	بنیاد = basename_c(service);
	اگر (state != RC_SERVICE_STOPPED) {
		صحیح state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		اگر (access(init, F_OK) != 0) {
			آزاد(init);
			واپس false;
		}

		unlinkat(state_dirfd, بنیاد, 0);
		اگر ((symlinkat(init, state_dirfd, بنیاد)) != 0) {
			آزاد(init);
			واپس false;
		}
	}

	اگر (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		آزاد(init);
		واپس true;
	}

	/* Remove any old states now */
	برائے (i = 0; rc_service_state_نام[i].name; i++) {
		صحیح state_dirfd = rc_dirfd(rc_service_state_نام[i].dir);
		RC_SERVICE s = rc_service_state_نام[i].state;

		اگر (s == state)
			جاری;

		انتخاب (s) {
		حالت RC_SERVICE_STOPPED:
		حالت RC_SERVICE_HOTPLUGGED:
		حالت RC_SERVICE_SCHEDULED:
			جاری;
		حالت RC_SERVICE_WASINACTIVE:
			اگر (skip_wasinactive)
				جاری;
			/* fall-through */
		طےشدہ:
			اگر (faccessat(state_dirfd, بنیاد, F_OK, 0) != 0)
				جاری;
			اگر ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				اگر (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), بنیاد) == -1) {
					آزاد(init);
					واپس false;
				}
				skip_wasinactive = true;
			}
			اگر (unlinkat(state_dirfd, بنیاد, 0) == -1) {
				آزاد(init);
				واپس false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	اگر (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), بنیاد, 0);

	/* Remove any options and daemons the service may have stored */
	اگر (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), بنیاد, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), بنیاد, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), بنیاد, 0);
		rc_service_جدول_وقت_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	اگر (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		اگر (dp) {
			ساخت dirent *d;
			جب ((d = readdir(dp))) {
				حرف *was;

				xasprintf(&was, "%s/%s", d->d_name, بنیاد);
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
	واپس true;
}

RC_SERVICE
rc_service_state(ثابت حرف *service)
{
	ثابت حرف *بنیاد = basename_c(service);
	صحیح state = RC_SERVICE_STOPPED;
	صحیح i;

	برائے (i = 0; rc_service_state_نام[i].name; i++) {
		اگر (faccessat(rc_dirfd(rc_service_state_نام[i].dir), بنیاد, F_OK, 0) == 0) {
			اگر (rc_service_state_نام[i].state <= 0x10)
				state = rc_service_state_نام[i].state;
			ورنہ
				state |= rc_service_state_نام[i].state;
		}
	}

	اگر (state & RC_SERVICE_STARTED)
		اگر (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	اگر (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		اگر (dp) {
			ساخت dirent *d;
			جب ((d = readdir(dp))) {
				اگر (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					توڑو;
				}
			}

			closedir(dp);
		}
	}

	واپس state;
}

حرف *
rc_service_قدر_get(ثابت حرف *service, ثابت حرف *option)
{
	حرف *بفر = تہی;
	size_t len = 0;
	حرف *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &بفر, &len);
	آزاد(file);

	واپس بفر;
}

bool
rc_service_قدر_سیٹ(ثابت حرف *service, ثابت حرف *option, ثابت حرف *قدر)
{
	bool ret = true;
	صحیح optdirfd;
	FILE *fp;

	اگر (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		واپس false;

	اگر ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		واپس false;

	اگر (!قدر) {
		unlinkat(optdirfd, option, 0);
	} ورنہ اگر ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", قدر);
		fclose(fp);
	} ورنہ {
		ret = false;
	}

	close(optdirfd);
	واپس ret;
}

bool
rc_service_جدول_وقت_آغاز(ثابت حرف *service, ثابت حرف *service_مقصد_آغاز)
{
	ثابت حرف *بنیاد_مقصد_آغاز = basename_c(service_مقصد_آغاز);
	ثابت حرف *بنیاد = basename_c(service);
	bool retval;
	صحیح schedfd;
	حرف *init;

	/* service may be a provided service, like net */
	اگر (!service || !rc_service_exists(service_مقصد_آغاز))
		واپس false;

	اگر (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), بنیاد, 0755) != 0 && errno != EEXIST)
		واپس false;

	اگر ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), بنیاد, O_RDONLY)) == -1)
		واپس false;

	init = rc_service_resolve(service_مقصد_آغاز);

	retval = (faccessat(schedfd, بنیاد_مقصد_آغاز, F_OK, 0) == 0) || (symlinkat(init, schedfd, بنیاد_مقصد_آغاز) == 0);

	close(schedfd);
	آزاد(init);
	واپس retval;
}

bool
rc_service_جدول_وقت_clear(ثابت حرف *service)
{
	واپس !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_اندر_runlevel(ثابت حرف *runlevel)
{
	RC_STRINGLIST *list = تہی;

	اگر (!runlevel) {
		ثابت صحیح *scriptdirfds;
		list = rc_stringlist_new();
		برائے (size_t i = 0, گنتی = rc_scriptdirfds(&scriptdirfds); i < گنتی; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		واپس list;
	}

	/* These special levels never contain any services */
	اگر (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		واپس rc_stringlist_new();

	واپس ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_اندر_runlevel_stacked(ثابت حرف *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ڈھیر;

	list = rc_services_اندر_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ڈھیر, stacks, entries) {
		sl = rc_services_اندر_runlevel(ڈھیر->قدر);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	واپس list;
}

RC_STRINGLIST *
rc_services_اندر_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	ساخت dirent *d;
	DIR *dp;

	اگر (state != RC_SERVICE_SCHEDULED)
		واپس ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	اگر (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		واپس list;

	جب ((d = readdir(dp))) {
		اگر (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			جاری;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	واپس list;
}

bool
rc_service_جوڑو(ثابت حرف *runlevel, ثابت حرف *service)
{
	bool retval;
	حرف *init;
	حرف *file;
	حرف *راستہ;
	حرف *binit = تہی;
	حرف *i;

	اگر (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		واپس false;
	}

	اگر (rc_service_اندر_runlevel(service, runlevel)) {
		errno = EEXIST;
		واپس false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	اگر (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		راستہ = realpath(dirname(init), تہی);
		اگر (راستہ == تہی) {
			آزاد(init);
			واپس false;
		}
		retval = (strcmp(راستہ, RC_INITDIR) == 0);
		آزاد(راستہ);
		اگر (!retval) {
			آزاد(init);
			errno = EPERM;
			واپس false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	اگر (لڑی_موازنہ(i, rc_svcdir(), لڑی_لمبائی(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		جاؤ برآمد;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	آزاد(file);

برآمد:
	آزاد(binit);
	آزاد(init);
	واپس retval;
}

bool
rc_service_delete(ثابت حرف *runlevel, ثابت حرف *service)
{
	حرف *file;
	صحیح r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	آزاد(file);
	واپس (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ثابت حرف *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	ساخت dirent *d;
	DIR *dp;

	اگر (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		واپس list;

	جب ((d = readdir(dp))) {
		حرف *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		اگر (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_جوڑو(list, d->d_name);
		آزاد(file);
	}

	واپس list;
}

RC_STRINGLIST *
rc_services_scheduled(ثابت حرف *service)
{
	واپس ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ثابت حرف *service, ثابت حرف *env)
{
	size_t name_len = strcspn(env, "=");
	حرف var[لڑی_لمبائی(service) + حجم("/") + name_len];
	صحیح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	اگر (env[name_len] != '=')
		واپس errno = EINVAL, false;

	snprintf(var, حجم(var), "%s/%.*s", service, (صحیح) name_len, env);

	اگر (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		واپس false;

	اگر (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		واپس false;

	fputs(&env[name_len + 1], fp);

	واپس fclose(fp) == 0;
}

bool
rc_service_setenv(ثابت حرف *service, ثابت حرف *name, ثابت حرف *قدر)
{
	حرف var[لڑی_لمبائی(service) + حجم("/") + لڑی_لمبائی(name)];
	صحیح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, حجم(var), "%s/%s", service, name);

	اگر (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		واپس false;

	اگر (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		واپس false;

	fputs(قدر, fp);

	واپس fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ثابت حرف *service, ثابت حرف *name, حرف **buf)
{
	حرف var[لڑی_لمبائی(service) + حجم("/") + لڑی_لمبائی(name)], *قدر;
	صحیح dfd = rc_dirfd(RC_DIR_ENVIRON);
	ساخت stat stat;
	size_t گنتی;
	FILE *fp;

	snprintf(var, حجم(var), "%s/%s", service, name);

	اگر (fstatat(dfd, var, &stat, 0) == -1)
		واپس -1;

	اگر (!buf)
		واپس stat.st_size;

	اگر (!(قدر = xrealloc(*buf, stat.st_size + 1)))
		واپس -1;

	اگر (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		واپس -1;

	گنتی = fread(قدر, 1, stat.st_size, fp);
	قدر[گنتی] = '\0';

	fclose(fp);
	*buf = قدر;
	واپس گنتی;
}
