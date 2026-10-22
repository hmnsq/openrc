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
#جيڪڏهن_وصف_ٿيل __FreeBSD__
#  include <sys/sysctl.h>
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#جيڪڏهن_پورو

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = عدم;
ساكن صحيح dirfds[RC_DIR_MAX];

صحيح rc_dirfd(تعداد rc_dir dir) {
	صحيح flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	صحيح dirfd;

	إذا (dir < 0 || dir >= RC_DIR_MAX)
		أرجع -1;
	إذا (dirfds[dir] <= 0) {
		بدل (dir) {
		حالة RC_DIR_SYSCONF:
			أرجع dirfds[dir] = open(rc_sysconfdir(), flags);
		حالة RC_DIR_USRCONF:
			أرجع dirfds[dir] = open(rc_usrconfdir(), flags);
		حالة RC_DIR_RUNLEVEL:
			أرجع dirfds[dir] = open(rc_runleveldir(), flags);
		حالة RC_DIR_SVCDIR:
			إذا (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				أرجع -1;
			أرجع dirfds[dir] = open(rc_svcdir(), flags);
		افتراضي:
			أكد(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			إذا ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				إذا (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					أرجع -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			اكسر;
		}
	}
	أرجع dirfds[dir];
}

فراغ clear_dirfds(فراغ) {
	لكل (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		إذا (dirfds[i] <= 0)
			استمر;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#وصف_ڪريو LS_INITD	0x01
#وصف_ڪريو LS_DIR		0x02
ساكن RC_STRINGLIST *
ls_dir(صحيح targetfd, ثابت حرف *dir, صحيح options)
{
	DIR *dp;
	بنية dirent *d;
	RC_STRINGLIST *list = عدم;
	بنية stat buf;
	size_t l;

	list = rc_stringlist_new();

	إذا (!(dp = do_opendirat(targetfd, dir)))
		أرجع list;

	طالما (((d = readdir(dp)) != عدم)) {
		إذا (d->d_name[0] == '.')
			استمر;
		إذا (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			إذا (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				استمر;

			/* .sh files are not init scripts */
			l = لڙي_ڊيگهه(d->d_name);
			إذا (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				استمر;
		}
		إذا (options & LS_DIR) {
			إذا (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				استمر;
		}
		rc_stringlist_جوڙيو(list, d->d_name);
	}
	closedir(dp);
	أرجع list;
}

ساكن bool
rm_dir(صحيح targetfd, ثابت حرف *pathname, bool چوٽي)
{
	bool retval = true;
	بنية dirent *d;
	بنية stat s;
	DIR *dp;

	إذا (!(dp = do_opendirat(targetfd, pathname)))
		أرجع false;

	طالما ((d = readdir(dp))) {
		إذا (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			استمر;
		إذا (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			اكسر;
		}
		إذا (S_ISDIR(s.st_mode)) {
			إذا (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				اكسر;
			}
		} وإلا {
			إذا (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				اكسر;
			}
		}
	}
	closedir(dp);

	إذا (!retval)
		أرجع false;

	إذا (چوٽي && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		أرجع false;

	أرجع true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#جيڪڏهن_وصف_ٿيل __linux__
ساكن bool
file_regex(ثابت حرف *file, ثابت حرف *regex)
{
	FILE *fp;
	حرف *سٽ = عدم;
	size_t سائيز = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	صحيح نتيجو;

	إذا (!(fp = fopen(file, "r")))
		أرجع false;

	إذا ((نتيجو = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		حرف buf[BUFSIZ];
		fclose(fp);
		regerror(نتيجو, &re, buf, حجم(buf));
		fprintf(stderr, "file_regex: %s", buf);
		أرجع false;
	}

	طالما ((len = xgetline(&سٽ, &سائيز, fp)) != -1) {
		حرف *str = سٽ;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		افعل {
			إذا (regexec(&re, str, 0, عدم, 0) == 0)
				اذهب مليو;
			str += لڙي_ڊيگهه(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			طالما (str < سٽ + len && *str == '\0')
				str++;
		} طالما (str < سٽ + len);
	}
	retval = false;
مليو:
	fclose(fp);
	حرر(سٽ);
	regfree(&re);

	أرجع retval;
}
#جيڪڏهن_پورو


ساكن ثابت حرف *
get_systype(فراغ)
{
	حرف *systype = rc_conf_قدر("rc_sys");
	إذا (systype) {
		حرف *s = systype;
		/* Convert to uppercase */
		طالما (s && *s) {
			إذا (islower((بنانشان حرف) *s))
				*s = وڏو_اکر((بنانشان حرف) *s);
			s++;
		}
	}
	أرجع systype;
}

ساكن ثابت حرف *
detect_container(ثابت حرف *systype RC_اڻ_استعمال_ٿيل)
{
#جيڪڏهن_وصف_ٿيل __FreeBSD__
	إذا (systype) {
		إذا (strcmp(systype, RC_SYS_NONE) == 0)
		       أرجع عدم;
		إذا (strcmp(systype, RC_SYS_JAIL) == 0)
			أرجع RC_SYS_JAIL;
		إذا (strcmp(systype, RC_SYS_PODMAN) == 0)
			أرجع RC_SYS_PODMAN;
	}

	صحيح jailed = 0;
	size_t len = حجم(jailed);

	إذا (sysctlbyname("security.jail.jailed", &jailed, &len, عدم, 0) == 0)
		إذا (jailed == 1)
			أرجع RC_SYS_JAIL;

	إذا (access("/var/run/.containerenv", F_OK) == 0)
		أرجع RC_SYS_PODMAN;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __linux__
	إذا (systype) {
		إذا (strcmp(systype, RC_SYS_NONE) == 0)
			أرجع عدم;
		إذا (strcmp(systype, RC_SYS_UML) == 0)
			أرجع RC_SYS_UML;
		إذا (strcmp(systype, RC_SYS_VSERVER) == 0)
			أرجع RC_SYS_VSERVER;
		إذا (strcmp(systype, RC_SYS_OPENVZ) == 0)
			أرجع RC_SYS_OPENVZ;
		إذا (strcmp(systype, RC_SYS_LXC) == 0)
			أرجع RC_SYS_LXC;
		إذا (strcmp(systype, RC_SYS_RKT) == 0)
				أرجع RC_SYS_RKT;
		إذا (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				أرجع RC_SYS_SYSTEMD_NSPAWN;
		إذا (strcmp(systype, RC_SYS_DOCKER) == 0)
				أرجع RC_SYS_DOCKER;
		إذا (strcmp(systype, RC_SYS_PODMAN) == 0)
			أرجع RC_SYS_PODMAN;
		إذا (strcmp(systype, RC_SYS_WSL) == 0)
			أرجع RC_SYS_WSL;
	}
	إذا (file_regex("/proc/cpuinfo", "UML"))
		أرجع RC_SYS_UML;
	وإلا إذا (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		أرجع RC_SYS_VSERVER;
	وإلا إذا (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		أرجع RC_SYS_OPENVZ;
	وإلا إذا (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		أرجع RC_SYS_OPENVZ; /* old test */
	وإلا إذا (file_regex("/proc/1/environ", "container=lxc"))
		أرجع RC_SYS_LXC;
	وإلا إذا (file_regex("/proc/1/environ", "container=rkt"))
		أرجع RC_SYS_RKT;
	وإلا إذا (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		أرجع RC_SYS_SYSTEMD_NSPAWN;
	وإلا إذا (access("/run/.containerenv", F_OK) == 0)
		أرجع RC_SYS_PODMAN;
	وإلا إذا (access("/.dockerenv", F_OK) == 0)
		أرجع RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	وإلا إذا (file_regex("/proc/1/environ", "container=docker"))
		أرجع RC_SYS_DOCKER;
	وإلا إذا (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		أرجع RC_SYS_WSL;
	وإلا إذا (getenv("WSL_DISTRO_NAME"))
		أرجع RC_SYS_WSL;
	وإلا إذا (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		أرجع RC_SYS_WSL;
#جيڪڏهن_پورو

	أرجع عدم;
}

ساكن ثابت حرف *
detect_vm(ثابت حرف *systype RC_اڻ_استعمال_ٿيل)
{
#جيڪڏهن_وصف_ٿيل __NetBSD__
	إذا (systype) {
		إذا (strcmp(systype, RC_SYS_NONE) == 0)
			أرجع عدم;
		إذا (strcmp(systype, RC_SYS_XEN0) == 0)
			أرجع RC_SYS_XEN0;
		إذا (strcmp(systype, RC_SYS_XENU) == 0)
			أرجع RC_SYS_XENU;
	}
	إذا (access("/kern/xen/privcmd", F_OK) == 0)
		أرجع RC_SYS_XEN0;
	إذا (access("/kern/xen", F_OK) == 0)
		أرجع RC_SYS_XENU;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __linux__
	إذا (systype) {
		إذا (strcmp(systype, RC_SYS_NONE) == 0)
			أرجع عدم;
		إذا (strcmp(systype, RC_SYS_XEN0) == 0)
			أرجع RC_SYS_XEN0;
		إذا (strcmp(systype, RC_SYS_XENU) == 0)
			أرجع RC_SYS_XENU;
	}
	إذا (access("/proc/xen", F_OK) == 0) {
		إذا (file_regex("/proc/xen/capabilities", "control_d"))
			أرجع RC_SYS_XEN0;
		أرجع RC_SYS_XENU;
	}
#جيڪڏهن_پورو

	أرجع عدم;
}

ثابت حرف *
rc_sys(فراغ)
{
	ثابت حرف *systype;
	ثابت حرف *sys;

	systype = get_systype();
	sys = detect_container(systype);
	إذا (!sys) {
		sys = detect_vm(systype);
	}

	أرجع sys;
}

ساكن ثابت حرف *
rc_parse_service_state(RC_SERVICE state)
{
	صحيح i;

	لكل (i = 0; rc_service_state_نالا[i].name; i++) {
		إذا (rc_service_state_نالا[i].state == state)
			أرجع rc_service_state_نالا[i].name;
	}
	أرجع عدم;
}

ساكن صحيح
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	لكل (size_t i = 0; rc_service_state_نالا[i].name; i++)
		إذا (rc_service_state_نالا[i].state == state)
			أرجع rc_service_state_نالا[i].dir;
	أرجع -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
ساكن فراغ
get_runlevel_chain(ثابت حرف *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *والد;
	ثابت حرف *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	إذا (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		أرجع;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	إذا (rc_stringlist_ڳوليو(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_جوڙيو(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->قدر;

		/* Check for loop */
		إذا (rc_stringlist_ڳوليو(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(والد, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", والد->قدر);
			ٻاهر_نڪرو(1);
		}

		/* Add new ancestor */
		rc_stringlist_جوڙيو(ancestor_list, nextlevel);

		get_runlevel_chain(nextlevel, level_list, ancestor_list);

		rc_stringlist_delete(ancestor_list, nextlevel);
	}
	rc_stringlist_free(dirs);
}

bool
rc_runlevel_starting(فراغ)
{
	أرجع faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(فراغ)
{
	أرجع faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(فراغ)
{
	أرجع ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

حرف *
rc_runlevel_get(فراغ)
{
	FILE *fp;
	حرف *runlevel = عدم;
	size_t i;

	إذا ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(حجم(حرف) * PATH_MAX);
		إذا (fgets(runlevel, PATH_MAX, fp)) {
			i = لڙي_ڊيگهه(runlevel);
			إذا (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} وإلا
			*runlevel = '\0';
		fclose(fp);
	}

	إذا (!runlevel || !*runlevel) {
		حرر(runlevel);
		runlevel = عدم;
	}

	أرجع runlevel;
}

bool
rc_runlevel_سيٽ(ثابت حرف *runlevel)
{
	FILE *fp;

	إذا (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		أرجع false;

	fputs(runlevel, fp);
	fclose(fp);
	أرجع true;
}

bool
rc_runlevel_exists(ثابت حرف *runlevel)
{
	بنية stat buf;

	إذا (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		أرجع false;
	إذا (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		أرجع true;
	أرجع false;
}

bool
rc_runlevel_ڍير(ثابت حرف *dst, ثابت حرف *src)
{
	حرف *d, *s;
	صحيح r;

	إذا (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		أرجع false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	حرر(d);
	حرر(s);
	أرجع (r == 0);
}

bool
rc_runlevel_unstack(ثابت حرف *dst, ثابت حرف *src)
{
	حرف *رستو;
	صحيح r;

	xasprintf(&رستو, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(رستو);
	حرر(رستو);
	أرجع (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ثابت حرف *runlevel)
{
	RC_STRINGLIST *ڍير, *ancestor_list;
	ڍير = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_جوڙيو(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ڍير, ancestor_list);
	rc_stringlist_free(ancestor_list);
	أرجع ڍير;
}

ساكن ثابت حرف *config_رستو[] = { RC_رستو, عدم };

ساكن بنية {
	bool سيٽ;
	حرف *svcdir, *confdir, *runleveldir;
} rc_dirs;

ساكن بنية {
	حرف *بفر;
	size_t ڳڻپ;
	صحيح *fds;
	ثابت حرف **entries, **allocd;
} rc_رستو;

ساكن فراغ
free_rc_dirs(فراغ)
{
#وصف_ڪريو X(ptr) ptr = (حرر(ptr), عدم)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_رستو.allocd);
	/* entries alias freed allocation */
	rc_رستو.entries = عدم;
	X(rc_رستو.بفر);
	X(rc_رستو.fds);
	rc_رستو.ڳڻپ = 0;
#وصف_رد X
}

__attribute__((__format__(__printf__, 2, 3)))
ساكن فراغ
append_رستو(FILE *fp, ثابت حرف *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_رستو.ڳڻپ++;
}

ساكن ثابت حرف *
strnsep(ثابت حرف **str, size_t *سائيز, ثابت حرف sep)
{
	ثابت حرف *ret = *str;

	إذا (!ret)
		أرجع عدم;

	*str = strchrnul(ret, sep);
	*سائيز = *str - ret;
	إذا (!**str)
		*str = عدم;
	وإلا
		*str += 1;

	أرجع ret;
}

ساكن ثابت حرف *
strnullsep(ثابت حرف **str, size_t *سائيز)
{
	ثابت حرف *ret = *str;
	size_t len;

	إذا (!ret)
		أرجع عدم;

	len = strnlen(ret, *سائيز - 1) + 1;

	إذا ((*سائيز -= len))
		*str += len;
	وإلا
		*str = عدم;

	أرجع ret;
}

ساكن فراغ
append_user_رستو(FILE *fp)
{
	ثابت حرف *xdg, *داخلا;
	size_t سائيز;

	append_رستو(fp, "%s", rc_dirs.confdir);
	إذا (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_رستو(fp, "/etc/xdg/rc");
	وإلا طالما ((داخلا = strnsep(&xdg, &سائيز, ':')))
		إذا (سائيز) append_رستو(fp, "%.*s/rc", (صحيح) سائيز, داخلا);

	لكل (size_t i = 0; i < ARRAY_سائيز(config_رستو) - 2; i++)
		append_رستو(fp, "%s/user", config_رستو[i]);
	append_رستو(fp, "%s", rc_dirs.svcdir);
}

ساكن فراغ
append_rc_رستو(FILE *fp, ثابت حرف *رستو, bool user)
{
	bool has_user = false;
	ثابت حرف *داخلا;
	size_t سائيز;

	طالما ((داخلا = strnsep(&رستو, &سائيز, ':'))) {
		إذا (سائيز)
			append_رستو(fp, "%.*s%s", (صحيح)سائيز, داخلا, user ? "/user" : "");
		وإلا إذا (user && !has_user)
			append_user_رستو(fp), has_user = true;
		وإلا إذا (!user) طالما (*rc_رستو.entries)
			append_رستو(fp, "%s", *rc_رستو.entries++);
	}
}

ساكن فراغ
ڀريو_dynamic_رستو(bool user)
{
	ثابت حرف *رستو, *داخلا;
	size_t سائيز, ڳڻپ = 0;
	FILE *fp;

	إذا (!rc_dirs.سيٽ) {
		atexit(free_rc_dirs);
		rc_dirs.سيٽ = true;
	}

	rc_رستو.entries = config_رستو;
	rc_رستو.ڳڻپ = ARRAY_سائيز(config_رستو) - 1;

	إذا (!(رستو = getenv("RC_PATH")) && !user)
		أرجع;

	rc_رستو.ڳڻپ = 0;
	fp = xopen_memstream(&rc_رستو.بفر, &سائيز);
	إذا (رستو)
		append_rc_رستو(fp, رستو, user);
	وإلا
		append_user_رستو(fp);
	xclose_memstream(fp);

	rc_رستو.allocd = rc_رستو.entries = صفر_يادگيري_ڏيو(rc_رستو.ڳڻپ + 1, حجم(*rc_رستو.entries));

	رستو = rc_رستو.بفر;

	طالما ((داخلا = strnullsep(&رستو, &سائيز)) && ڳڻپ < rc_رستو.ڳڻپ)
		rc_رستو.entries[ڳڻپ++] = داخلا;
	rc_رستو.entries[rc_رستو.ڳڻپ] = عدم;
}

ساكن bool is_user = false;

bool
rc_is_user(فراغ)
{
	صحيح _is_user = -1;

	إذا (_is_user == -1) {
		حرف *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	أرجع _is_user;
}

فراغ
rc_سيٽ_user(فراغ)
{
	حرف *env;

	إذا (is_user)
		أرجع;

	إذا (!rc_dirs.سيٽ)
		atexit(free_rc_dirs);
	وإلا
		free_rc_dirs();

	is_user = true;
	rc_dirs.سيٽ = true;
	setenv("RC_USER_SERVICES", "yes", true);

	إذا ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} وإلا إذا ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} وإلا {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		ٻاهر_نڪرو(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	إذا (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		ٻاهر_نڪرو(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	ڀريو_dynamic_رستو(true);

	clear_dirfds();
}

ثابت حرف * ثابت *
rc_scriptdirs(فراغ)
{
	إذا (!rc_رستو.entries)
		ڀريو_dynamic_رستو(false);
	أرجع rc_رستو.entries;
}

size_t
rc_scriptdirfds(ثابت صحيح **fds)
{
	ساكن size_t len = 0;

	إذا (len == 0) {
		إذا (!rc_رستو.entries)
			ڀريو_dynamic_رستو(false);

		rc_رستو.fds = xmalloc(rc_رستو.ڳڻپ * حجم(*rc_رستو.fds));
		لكل (size_t i = 0; i < rc_رستو.ڳڻپ; i++) {
			صحيح fd = open(rc_رستو.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			إذا (fd == -1)
				استمر;

			rc_رستو.fds[len++] = fd;
		}
	}

	*fds = rc_رستو.fds;
	أرجع len;
}

ثابت حرف *
rc_sysconfdir(فراغ)
{
	أرجع RC_SYSCONFDIR;
}

ثابت حرف *
rc_usrconfdir(فراغ)
{
	أرجع rc_dirs.confdir ? rc_dirs.confdir : عدم;
}

ثابت حرف *
rc_runleveldir(فراغ)
{
	أرجع rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ثابت حرف *
rc_svcdir(فراغ)
{
	ساكن ثابت حرف *svcdir;

	إذا (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	أرجع rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

ساكن ssize_t
محفوظ_readlink(صحيح targetfd, ثابت حرف *رستو, حرف **buf, size_t bufsiz)
{
	حرف *بفر;
	ssize_t r;

	لكل (;; bufsiz += PATH_MAX) {
		بفر = xmalloc(bufsiz);
		r = readlinkat(targetfd, رستو, بفر, bufsiz);
		إذا (r < 0) {
			حرر(بفر);
			أرجع -errno;
		}
		إذا ((size_t)r < bufsiz) {
			بفر[r] = '\0';
			*buf = بفر;
			أرجع r;
		}
		حرر(بفر);
		إذا (bufsiz >= SSIZE_MAX - PATH_MAX)
			أرجع -ENOMEM;
	}
}

/* Resolve a service name to its full path */
حرف *
rc_service_resolve(ثابت حرف *service)
{
	حرف *بفر;
	بنية stat buf;

	إذا (!service)
		أرجع عدم;

	إذا (service[0] == '/')
		أرجع xstrdup(service);

	/* First check started services */
	إذا (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		إذا (محفوظ_readlink(rc_dirfd(RC_DIR_STARTED), service, &بفر, buf.st_size + 1) >= 0)
			أرجع بفر;
	إذا (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		إذا (محفوظ_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &بفر, buf.st_size + 1) >= 0)
			أرجع بفر;

	لكل (ثابت حرف * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		حرف *file = عدم;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		إذا (stat(file, &buf) == 0)
			أرجع file;
		حرر(file);
	}

	أرجع عدم;
}

bool
rc_service_exists(ثابت حرف *service)
{
	حرف *file;
	bool retval = false;
	size_t len;
	بنية stat buf;

	إذا (!service) {
		errno = EINVAL;
		أرجع false;
	}

	len = لڙي_ڊيگهه(service);

	/* .sh files are not init scripts */
	إذا (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		أرجع false;
	}

	إذا (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		أرجع false;
	}

	إذا (stat(file, &buf) == 0) {
		إذا (buf.st_mode & S_IXUGO)
			retval = true;
		وإلا
			errno = ENOEXEC;
	}
	حرر(file);
	أرجع retval;
}

#وصف_ڪريو OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ثابت حرف *service)
{
	حرف *svc;
	حرف *cmd = عدم;
	حرف *بفر = عدم;
	size_t len = 0;
	RC_STRINGLIST *commands = عدم;
	حرف *token;
	حرف *p;
	FILE *fp;

	إذا (!(svc = rc_service_resolve(service)))
		أرجع عدم;

	xasprintf(&cmd, OPTSTR, svc);
	حرر(svc);

	إذا (!(fp = popen(cmd, "r"))) {
		حرر(cmd);
		أرجع عدم;
	}

	إذا (xgetline(&بفر, &len, fp) != -1) {
		p = بفر;
		commands = rc_stringlist_new();

		طالما ((token = strsep(&p, " ")))
			إذا (token[0] != '\0')
				rc_stringlist_جوڙيو(commands, token);
	}

	pclose(fp);
	حرر(بفر);
	حرر(cmd);
	أرجع commands;
}

#وصف_ڪريو DESCSTR ". '%s'; echo \"${description%s%s}\""
حرف *
rc_service_description(ثابت حرف *service, ثابت حرف *option)
{
	حرف *svc;
	حرف *cmd;
	حرف *desc = عدم;
	size_t سائيز = 0;
	FILE *fp;

	إذا (!(svc = rc_service_resolve(service)))
		أرجع عدم;

	إذا (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	حرر(svc);
	إذا ((fp = popen(cmd, "r"))) {
		إذا (xgetline(&desc, &سائيز, fp) == -1) {
			حرر(desc);
			desc = عدم;
		}
		pclose(fp);
	}
	حرر(cmd);
	أرجع desc;
}

bool
rc_service_اندريون_runlevel(ثابت حرف *service, ثابت حرف *runlevel)
{
	حرف *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	حرر(file);
	أرجع r;
}

bool
rc_service_mark(ثابت حرف *service, ثابت RC_SERVICE state)
{
	صحيح i = 0;
	ثابت حرف *بنياد;
	حرف *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	صحيح serrno;

	إذا (!init)
		أرجع false;

	بنياد = basename_c(service);
	إذا (state != RC_SERVICE_STOPPED) {
		صحيح state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		إذا (access(init, F_OK) != 0) {
			حرر(init);
			أرجع false;
		}

		unlinkat(state_dirfd, بنياد, 0);
		إذا ((symlinkat(init, state_dirfd, بنياد)) != 0) {
			حرر(init);
			أرجع false;
		}
	}

	إذا (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		حرر(init);
		أرجع true;
	}

	/* Remove any old states now */
	لكل (i = 0; rc_service_state_نالا[i].name; i++) {
		صحيح state_dirfd = rc_dirfd(rc_service_state_نالا[i].dir);
		RC_SERVICE s = rc_service_state_نالا[i].state;

		إذا (s == state)
			استمر;

		بدل (s) {
		حالة RC_SERVICE_STOPPED:
		حالة RC_SERVICE_HOTPLUGGED:
		حالة RC_SERVICE_SCHEDULED:
			استمر;
		حالة RC_SERVICE_WASINACTIVE:
			إذا (skip_wasinactive)
				استمر;
			/* fall-through */
		افتراضي:
			إذا (faccessat(state_dirfd, بنياد, F_OK, 0) != 0)
				استمر;
			إذا ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				إذا (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), بنياد) == -1) {
					حرر(init);
					أرجع false;
				}
				skip_wasinactive = true;
			}
			إذا (unlinkat(state_dirfd, بنياد, 0) == -1) {
				حرر(init);
				أرجع false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	إذا (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), بنياد, 0);

	/* Remove any options and daemons the service may have stored */
	إذا (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), بنياد, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), بنياد, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), بنياد, 0);
		rc_service_وقت_جدول_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	إذا (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		إذا (dp) {
			بنية dirent *d;
			طالما ((d = readdir(dp))) {
				حرف *was;

				xasprintf(&was, "%s/%s", d->d_name, بنياد);
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
	أرجع true;
}

RC_SERVICE
rc_service_state(ثابت حرف *service)
{
	ثابت حرف *بنياد = basename_c(service);
	صحيح state = RC_SERVICE_STOPPED;
	صحيح i;

	لكل (i = 0; rc_service_state_نالا[i].name; i++) {
		إذا (faccessat(rc_dirfd(rc_service_state_نالا[i].dir), بنياد, F_OK, 0) == 0) {
			إذا (rc_service_state_نالا[i].state <= 0x10)
				state = rc_service_state_نالا[i].state;
			وإلا
				state |= rc_service_state_نالا[i].state;
		}
	}

	إذا (state & RC_SERVICE_STARTED)
		إذا (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	إذا (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		إذا (dp) {
			بنية dirent *d;
			طالما ((d = readdir(dp))) {
				إذا (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					اكسر;
				}
			}

			closedir(dp);
		}
	}

	أرجع state;
}

حرف *
rc_service_قدر_get(ثابت حرف *service, ثابت حرف *option)
{
	حرف *بفر = عدم;
	size_t len = 0;
	حرف *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &بفر, &len);
	حرر(file);

	أرجع بفر;
}

bool
rc_service_قدر_سيٽ(ثابت حرف *service, ثابت حرف *option, ثابت حرف *قدر)
{
	bool ret = true;
	صحيح optdirfd;
	FILE *fp;

	إذا (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		أرجع false;

	إذا ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		أرجع false;

	إذا (!قدر) {
		unlinkat(optdirfd, option, 0);
	} وإلا إذا ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", قدر);
		fclose(fp);
	} وإلا {
		ret = false;
	}

	close(optdirfd);
	أرجع ret;
}

bool
rc_service_وقت_جدول_شروعات(ثابت حرف *service, ثابت حرف *service_منزل_شروعات)
{
	ثابت حرف *بنياد_منزل_شروعات = basename_c(service_منزل_شروعات);
	ثابت حرف *بنياد = basename_c(service);
	bool retval;
	صحيح schedfd;
	حرف *init;

	/* service may be a provided service, like net */
	إذا (!service || !rc_service_exists(service_منزل_شروعات))
		أرجع false;

	إذا (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), بنياد, 0755) != 0 && errno != EEXIST)
		أرجع false;

	إذا ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), بنياد, O_RDONLY)) == -1)
		أرجع false;

	init = rc_service_resolve(service_منزل_شروعات);

	retval = (faccessat(schedfd, بنياد_منزل_شروعات, F_OK, 0) == 0) || (symlinkat(init, schedfd, بنياد_منزل_شروعات) == 0);

	close(schedfd);
	حرر(init);
	أرجع retval;
}

bool
rc_service_وقت_جدول_clear(ثابت حرف *service)
{
	أرجع !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_اندريون_runlevel(ثابت حرف *runlevel)
{
	RC_STRINGLIST *list = عدم;

	إذا (!runlevel) {
		ثابت صحيح *scriptdirfds;
		list = rc_stringlist_new();
		لكل (size_t i = 0, ڳڻپ = rc_scriptdirfds(&scriptdirfds); i < ڳڻپ; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		أرجع list;
	}

	/* These special levels never contain any services */
	إذا (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		أرجع rc_stringlist_new();

	أرجع ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_اندريون_runlevel_stacked(ثابت حرف *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ڍير;

	list = rc_services_اندريون_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ڍير, stacks, entries) {
		sl = rc_services_اندريون_runlevel(ڍير->قدر);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	أرجع list;
}

RC_STRINGLIST *
rc_services_اندريون_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	بنية dirent *d;
	DIR *dp;

	إذا (state != RC_SERVICE_SCHEDULED)
		أرجع ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	إذا (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		أرجع list;

	طالما ((d = readdir(dp))) {
		إذا (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			استمر;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	أرجع list;
}

bool
rc_service_جوڙيو(ثابت حرف *runlevel, ثابت حرف *service)
{
	bool retval;
	حرف *init;
	حرف *file;
	حرف *رستو;
	حرف *binit = عدم;
	حرف *i;

	إذا (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		أرجع false;
	}

	إذا (rc_service_اندريون_runlevel(service, runlevel)) {
		errno = EEXIST;
		أرجع false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	إذا (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		رستو = realpath(dirname(init), عدم);
		إذا (رستو == عدم) {
			حرر(init);
			أرجع false;
		}
		retval = (strcmp(رستو, RC_INITDIR) == 0);
		حرر(رستو);
		إذا (!retval) {
			حرر(init);
			errno = EPERM;
			أرجع false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	إذا (لڙي_ڀيٽ(i, rc_svcdir(), لڙي_ڊيگهه(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		اذهب ٻاهريون;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	حرر(file);

ٻاهريون:
	حرر(binit);
	حرر(init);
	أرجع retval;
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
	أرجع (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ثابت حرف *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	بنية dirent *d;
	DIR *dp;

	إذا (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		أرجع list;

	طالما ((d = readdir(dp))) {
		حرف *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		إذا (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_جوڙيو(list, d->d_name);
		حرر(file);
	}

	أرجع list;
}

RC_STRINGLIST *
rc_services_scheduled(ثابت حرف *service)
{
	أرجع ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ثابت حرف *service, ثابت حرف *env)
{
	size_t name_len = strcspn(env, "=");
	حرف var[لڙي_ڊيگهه(service) + حجم("/") + name_len];
	صحيح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	إذا (env[name_len] != '=')
		أرجع errno = EINVAL, false;

	snprintf(var, حجم(var), "%s/%.*s", service, (صحيح) name_len, env);

	إذا (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		أرجع false;

	إذا (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		أرجع false;

	fputs(&env[name_len + 1], fp);

	أرجع fclose(fp) == 0;
}

bool
rc_service_setenv(ثابت حرف *service, ثابت حرف *name, ثابت حرف *قدر)
{
	حرف var[لڙي_ڊيگهه(service) + حجم("/") + لڙي_ڊيگهه(name)];
	صحيح dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, حجم(var), "%s/%s", service, name);

	إذا (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		أرجع false;

	إذا (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		أرجع false;

	fputs(قدر, fp);

	أرجع fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ثابت حرف *service, ثابت حرف *name, حرف **buf)
{
	حرف var[لڙي_ڊيگهه(service) + حجم("/") + لڙي_ڊيگهه(name)], *قدر;
	صحيح dfd = rc_dirfd(RC_DIR_ENVIRON);
	بنية stat stat;
	size_t ڳڻپ;
	FILE *fp;

	snprintf(var, حجم(var), "%s/%s", service, name);

	إذا (fstatat(dfd, var, &stat, 0) == -1)
		أرجع -1;

	إذا (!buf)
		أرجع stat.st_size;

	إذا (!(قدر = xrealloc(*buf, stat.st_size + 1)))
		أرجع -1;

	إذا (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		أرجع -1;

	ڳڻپ = fread(قدر, 1, stat.st_size, fp);
	قدر[ڳڻپ] = '\0';

	fclose(fp);
	*buf = قدر;
	أرجع ڳڻپ;
}
