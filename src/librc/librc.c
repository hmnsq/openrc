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

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <helpers.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <libgen.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <regex.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/param.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "librc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = ખાલી;
સ્થિર પૂર્ણાંક dirfds[RC_DIR_MAX];

પૂર્ણાંક rc_dirfd(ગણના rc_dir dir) {
	પૂર્ણાંક flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	પૂર્ણાંક dirfd;

	જો (dir < 0 || dir >= RC_DIR_MAX)
		પાછા -1;
	જો (dirfds[dir] <= 0) {
		પસંદગી (dir) {
		સ્થિતિ RC_DIR_SYSCONF:
			પાછા dirfds[dir] = open(rc_sysconfdir(), flags);
		સ્થિતિ RC_DIR_USRCONF:
			પાછા dirfds[dir] = open(rc_usrconfdir(), flags);
		સ્થિતિ RC_DIR_RUNLEVEL:
			પાછા dirfds[dir] = open(rc_runleveldir(), flags);
		સ્થિતિ RC_DIR_SVCDIR:
			જો (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				પાછા -1;
			પાછા dirfds[dir] = open(rc_svcdir(), flags);
		મૂળભૂત:
			પ્રતિજ્ઞા(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			જો ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				જો (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					પાછા -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			તોડો;
		}
	}
	પાછા dirfds[dir];
}

શૂન્ય clear_dirfds(શૂન્ય) {
	માટે (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		જો (dirfds[i] <= 0)
			ચાલુ;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
સ્થિર RC_STRINGLIST *
ls_dir(પૂર્ણાંક targetfd, અચલ અક્ષર *dir, પૂર્ણાંક options)
{
	DIR *dp;
	સંરચના dirent *d;
	RC_STRINGLIST *list = ખાલી;
	સંરચના stat buf;
	size_t l;

	list = rc_stringlist_new();

	જો (!(dp = do_opendirat(targetfd, dir)))
		પાછા list;

	જ્યારે (((d = readdir(dp)) != ખાલી)) {
		જો (d->d_name[0] == '.')
			ચાલુ;
		જો (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			જો (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				ચાલુ;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			જો (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				ચાલુ;
		}
		જો (options & LS_DIR) {
			જો (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				ચાલુ;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	પાછા list;
}

સ્થિર bool
rm_dir(પૂર્ણાંક targetfd, અચલ અક્ષર *pathname, bool ટોચ)
{
	bool retval = true;
	સંરચના dirent *d;
	સંરચના stat s;
	DIR *dp;

	જો (!(dp = do_opendirat(targetfd, pathname)))
		પાછા false;

	જ્યારે ((d = readdir(dp))) {
		જો (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ચાલુ;
		જો (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			તોડો;
		}
		જો (S_ISDIR(s.st_mode)) {
			જો (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				તોડો;
			}
		} નહીંતર {
			જો (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				તોડો;
			}
		}
	}
	closedir(dp);

	જો (!retval)
		પાછા false;

	જો (ટોચ && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		પાછા false;

	પાછા true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
સ્થિર bool
file_regex(અચલ અક્ષર *file, અચલ અક્ષર *regex)
{
	FILE *fp;
	અક્ષર *લીટી = ખાલી;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	પૂર્ણાંક result;

	જો (!(fp = fopen(file, "r")))
		પાછા false;

	જો ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		અક્ષર buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, કદ(buf));
		fprintf(stderr, "file_regex: %s", buf);
		પાછા false;
	}

	જ્યારે ((len = xgetline(&લીટી, &size, fp)) != -1) {
		અક્ષર *str = લીટી;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		કરો {
			જો (regexec(&re, str, 0, ખાલી, 0) == 0)
				જાઓ found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			જ્યારે (str < લીટી + len && *str == '\0')
				str++;
		} જ્યારે (str < લીટી + len);
	}
	retval = false;
found:
	fclose(fp);
	મુક્ત(લીટી);
	regfree(&re);

	પાછા retval;
}
#endif


સ્થિર અચલ અક્ષર *
get_systype(શૂન્ય)
{
	અક્ષર *systype = rc_conf_કિંમત("rc_sys");
	જો (systype) {
		અક્ષર *s = systype;
		/* Convert to uppercase */
		જ્યારે (s && *s) {
			જો (islower((unsigned અક્ષર) *s))
				*s = toupper((unsigned અક્ષર) *s);
			s++;
		}
	}
	પાછા systype;
}

સ્થિર અચલ અક્ષર *
detect_container(અચલ અક્ષર *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	જો (systype) {
		જો (strcmp(systype, RC_SYS_NONE) == 0)
		       પાછા ખાલી;
		જો (strcmp(systype, RC_SYS_JAIL) == 0)
			પાછા RC_SYS_JAIL;
		જો (strcmp(systype, RC_SYS_PODMAN) == 0)
			પાછા RC_SYS_PODMAN;
	}

	પૂર્ણાંક jailed = 0;
	size_t len = કદ(jailed);

	જો (sysctlbyname("security.jail.jailed", &jailed, &len, ખાલી, 0) == 0)
		જો (jailed == 1)
			પાછા RC_SYS_JAIL;

	જો (access("/var/run/.containerenv", F_OK) == 0)
		પાછા RC_SYS_PODMAN;
#endif

#ifdef __linux__
	જો (systype) {
		જો (strcmp(systype, RC_SYS_NONE) == 0)
			પાછા ખાલી;
		જો (strcmp(systype, RC_SYS_UML) == 0)
			પાછા RC_SYS_UML;
		જો (strcmp(systype, RC_SYS_VSERVER) == 0)
			પાછા RC_SYS_VSERVER;
		જો (strcmp(systype, RC_SYS_OPENVZ) == 0)
			પાછા RC_SYS_OPENVZ;
		જો (strcmp(systype, RC_SYS_LXC) == 0)
			પાછા RC_SYS_LXC;
		જો (strcmp(systype, RC_SYS_RKT) == 0)
				પાછા RC_SYS_RKT;
		જો (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				પાછા RC_SYS_SYSTEMD_NSPAWN;
		જો (strcmp(systype, RC_SYS_DOCKER) == 0)
				પાછા RC_SYS_DOCKER;
		જો (strcmp(systype, RC_SYS_PODMAN) == 0)
			પાછા RC_SYS_PODMAN;
		જો (strcmp(systype, RC_SYS_WSL) == 0)
			પાછા RC_SYS_WSL;
	}
	જો (file_regex("/proc/cpuinfo", "UML"))
		પાછા RC_SYS_UML;
	નહીંતર જો (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		પાછા RC_SYS_VSERVER;
	નહીંતર જો (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		પાછા RC_SYS_OPENVZ;
	નહીંતર જો (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		પાછા RC_SYS_OPENVZ; /* old test */
	નહીંતર જો (file_regex("/proc/1/environ", "container=lxc"))
		પાછા RC_SYS_LXC;
	નહીંતર જો (file_regex("/proc/1/environ", "container=rkt"))
		પાછા RC_SYS_RKT;
	નહીંતર જો (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		પાછા RC_SYS_SYSTEMD_NSPAWN;
	નહીંતર જો (access("/run/.containerenv", F_OK) == 0)
		પાછા RC_SYS_PODMAN;
	નહીંતર જો (access("/.dockerenv", F_OK) == 0)
		પાછા RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	નહીંતર જો (file_regex("/proc/1/environ", "container=docker"))
		પાછા RC_SYS_DOCKER;
	નહીંતર જો (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		પાછા RC_SYS_WSL;
	નહીંતર જો (getenv("WSL_DISTRO_NAME"))
		પાછા RC_SYS_WSL;
	નહીંતર જો (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		પાછા RC_SYS_WSL;
#endif

	પાછા ખાલી;
}

સ્થિર અચલ અક્ષર *
detect_vm(અચલ અક્ષર *systype RC_UNUSED)
{
#ifdef __NetBSD__
	જો (systype) {
		જો (strcmp(systype, RC_SYS_NONE) == 0)
			પાછા ખાલી;
		જો (strcmp(systype, RC_SYS_XEN0) == 0)
			પાછા RC_SYS_XEN0;
		જો (strcmp(systype, RC_SYS_XENU) == 0)
			પાછા RC_SYS_XENU;
	}
	જો (access("/kern/xen/privcmd", F_OK) == 0)
		પાછા RC_SYS_XEN0;
	જો (access("/kern/xen", F_OK) == 0)
		પાછા RC_SYS_XENU;
#endif

#ifdef __linux__
	જો (systype) {
		જો (strcmp(systype, RC_SYS_NONE) == 0)
			પાછા ખાલી;
		જો (strcmp(systype, RC_SYS_XEN0) == 0)
			પાછા RC_SYS_XEN0;
		જો (strcmp(systype, RC_SYS_XENU) == 0)
			પાછા RC_SYS_XENU;
	}
	જો (access("/proc/xen", F_OK) == 0) {
		જો (file_regex("/proc/xen/capabilities", "control_d"))
			પાછા RC_SYS_XEN0;
		પાછા RC_SYS_XENU;
	}
#endif

	પાછા ખાલી;
}

અચલ અક્ષર *
rc_sys(શૂન્ય)
{
	અચલ અક્ષર *systype;
	અચલ અક્ષર *sys;

	systype = get_systype();
	sys = detect_container(systype);
	જો (!sys) {
		sys = detect_vm(systype);
	}

	પાછા sys;
}

સ્થિર અચલ અક્ષર *
rc_parse_service_state(RC_SERVICE state)
{
	પૂર્ણાંક i;

	માટે (i = 0; rc_service_state_names[i].name; i++) {
		જો (rc_service_state_names[i].state == state)
			પાછા rc_service_state_names[i].name;
	}
	પાછા ખાલી;
}

સ્થિર પૂર્ણાંક
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	માટે (size_t i = 0; rc_service_state_names[i].name; i++)
		જો (rc_service_state_names[i].state == state)
			પાછા rc_service_state_names[i].dir;
	પાછા -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
સ્થિર શૂન્ય
get_runlevel_chain(અચલ અક્ષર *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	અચલ અક્ષર *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	જો (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		પાછા;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	જો (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->કિંમત;

		/* Check for loop */
		જો (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->કિંમત);
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
rc_runlevel_starting(શૂન્ય)
{
	પાછા faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(શૂન્ય)
{
	પાછા faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(શૂન્ય)
{
	પાછા ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

અક્ષર *
rc_runlevel_get(શૂન્ય)
{
	FILE *fp;
	અક્ષર *runlevel = ખાલી;
	size_t i;

	જો ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(કદ(અક્ષર) * PATH_MAX);
		જો (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			જો (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} નહીંતર
			*runlevel = '\0';
		fclose(fp);
	}

	જો (!runlevel || !*runlevel) {
		મુક્ત(runlevel);
		runlevel = ખાલી;
	}

	પાછા runlevel;
}

bool
rc_runlevel_set(અચલ અક્ષર *runlevel)
{
	FILE *fp;

	જો (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		પાછા false;

	fputs(runlevel, fp);
	fclose(fp);
	પાછા true;
}

bool
rc_runlevel_exists(અચલ અક્ષર *runlevel)
{
	સંરચના stat buf;

	જો (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		પાછા false;
	જો (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		પાછા true;
	પાછા false;
}

bool
rc_runlevel_ઢગલો(અચલ અક્ષર *dst, અચલ અક્ષર *src)
{
	અક્ષર *d, *s;
	પૂર્ણાંક r;

	જો (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		પાછા false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	મુક્ત(d);
	મુક્ત(s);
	પાછા (r == 0);
}

bool
rc_runlevel_unstack(અચલ અક્ષર *dst, અચલ અક્ષર *src)
{
	અક્ષર *path;
	પૂર્ણાંક r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	મુક્ત(path);
	પાછા (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(અચલ અક્ષર *runlevel)
{
	RC_STRINGLIST *ઢગલો, *ancestor_list;
	ઢગલો = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ઢગલો, ancestor_list);
	rc_stringlist_free(ancestor_list);
	પાછા ઢગલો;
}

સ્થિર અચલ અક્ષર *config_path[] = { _RC_PATH, ખાલી };

સ્થિર સંરચના {
	bool set;
	અક્ષર *svcdir, *confdir, *runleveldir;
} rc_dirs;

સ્થિર સંરચના {
	અક્ષર *buffer;
	size_t ગણતરી;
	પૂર્ણાંક *fds;
	અચલ અક્ષર **entries, **allocd;
} rc_path;

સ્થિર શૂન્ય
free_rc_dirs(શૂન્ય)
{
#define X(ptr) ptr = (મુક્ત(ptr), ખાલી)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = ખાલી;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.ગણતરી = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
સ્થિર શૂન્ય
append_path(FILE *fp, અચલ અક્ષર *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.ગણતરી++;
}

સ્થિર અચલ અક્ષર *
strnsep(અચલ અક્ષર **str, size_t *size, અચલ અક્ષર sep)
{
	અચલ અક્ષર *ret = *str;

	જો (!ret)
		પાછા ખાલી;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	જો (!**str)
		*str = ખાલી;
	નહીંતર
		*str += 1;

	પાછા ret;
}

સ્થિર અચલ અક્ષર *
strnullsep(અચલ અક્ષર **str, size_t *size)
{
	અચલ અક્ષર *ret = *str;
	size_t len;

	જો (!ret)
		પાછા ખાલી;

	len = strnlen(ret, *size - 1) + 1;

	જો ((*size -= len))
		*str += len;
	નહીંતર
		*str = ખાલી;

	પાછા ret;
}

સ્થિર શૂન્ય
append_user_path(FILE *fp)
{
	અચલ અક્ષર *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	જો (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	નહીંતર જ્યારે ((entry = strnsep(&xdg, &size, ':')))
		જો (size) append_path(fp, "%.*s/rc", (પૂર્ણાંક) size, entry);

	માટે (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

સ્થિર શૂન્ય
append_rc_path(FILE *fp, અચલ અક્ષર *path, bool user)
{
	bool has_user = false;
	અચલ અક્ષર *entry;
	size_t size;

	જ્યારે ((entry = strnsep(&path, &size, ':'))) {
		જો (size)
			append_path(fp, "%.*s%s", (પૂર્ણાંક)size, entry, user ? "/user" : "");
		નહીંતર જો (user && !has_user)
			append_user_path(fp), has_user = true;
		નહીંતર જો (!user) જ્યારે (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

સ્થિર શૂન્ય
load_dynamic_path(bool user)
{
	અચલ અક્ષર *path, *entry;
	size_t size, ગણતરી = 0;
	FILE *fp;

	જો (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.ગણતરી = ARRAY_SIZE(config_path) - 1;

	જો (!(path = getenv("RC_PATH")) && !user)
		પાછા;

	rc_path.ગણતરી = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	જો (path)
		append_rc_path(fp, path, user);
	નહીંતર
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.ગણતરી + 1, કદ(*rc_path.entries));

	path = rc_path.buffer;

	જ્યારે ((entry = strnullsep(&path, &size)) && ગણતરી < rc_path.ગણતરી)
		rc_path.entries[ગણતરી++] = entry;
	rc_path.entries[rc_path.ગણતરી] = ખાલી;
}

સ્થિર bool is_user = false;

bool
rc_is_user(શૂન્ય)
{
	પૂર્ણાંક _is_user = -1;

	જો (_is_user == -1) {
		અક્ષર *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	પાછા _is_user;
}

શૂન્ય
rc_set_user(શૂન્ય)
{
	અક્ષર *env;

	જો (is_user)
		પાછા;

	જો (!rc_dirs.set)
		atexit(free_rc_dirs);
	નહીંતર
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	જો ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} નહીંતર જો ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} નહીંતર {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	જો (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

અચલ અક્ષર * અચલ *
rc_scriptdirs(શૂન્ય)
{
	જો (!rc_path.entries)
		load_dynamic_path(false);
	પાછા rc_path.entries;
}

size_t
rc_scriptdirfds(અચલ પૂર્ણાંક **fds)
{
	સ્થિર size_t len = 0;

	જો (len == 0) {
		જો (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.ગણતરી * કદ(*rc_path.fds));
		માટે (size_t i = 0; i < rc_path.ગણતરી; i++) {
			પૂર્ણાંક fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			જો (fd == -1)
				ચાલુ;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	પાછા len;
}

અચલ અક્ષર *
rc_sysconfdir(શૂન્ય)
{
	પાછા RC_SYSCONFDIR;
}

અચલ અક્ષર *
rc_usrconfdir(શૂન્ય)
{
	પાછા rc_dirs.confdir ? rc_dirs.confdir : ખાલી;
}

અચલ અક્ષર *
rc_runleveldir(શૂન્ય)
{
	પાછા rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

અચલ અક્ષર *
rc_svcdir(શૂન્ય)
{
	સ્થિર અચલ અક્ષર *svcdir;

	જો (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	પાછા rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

સ્થિર ssize_t
safe_readlink(પૂર્ણાંક targetfd, અચલ અક્ષર *path, અક્ષર **buf, size_t bufsiz)
{
	અક્ષર *buffer;
	ssize_t r;

	માટે (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		જો (r < 0) {
			મુક્ત(buffer);
			પાછા -errno;
		}
		જો ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			પાછા r;
		}
		મુક્ત(buffer);
		જો (bufsiz >= SSIZE_MAX - PATH_MAX)
			પાછા -ENOMEM;
	}
}

/* Resolve a service name to its full path */
અક્ષર *
rc_service_resolve(અચલ અક્ષર *service)
{
	અક્ષર *buffer;
	સંરચના stat buf;

	જો (!service)
		પાછા ખાલી;

	જો (service[0] == '/')
		પાછા xstrdup(service);

	/* First check started services */
	જો (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		જો (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			પાછા buffer;
	જો (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		જો (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			પાછા buffer;

	માટે (અચલ અક્ષર * અચલ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		અક્ષર *file = ખાલી;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		જો (stat(file, &buf) == 0)
			પાછા file;
		મુક્ત(file);
	}

	પાછા ખાલી;
}

bool
rc_service_exists(અચલ અક્ષર *service)
{
	અક્ષર *file;
	bool retval = false;
	size_t len;
	સંરચના stat buf;

	જો (!service) {
		errno = EINVAL;
		પાછા false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	જો (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		પાછા false;
	}

	જો (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		પાછા false;
	}

	જો (stat(file, &buf) == 0) {
		જો (buf.st_mode & S_IXUGO)
			retval = true;
		નહીંતર
			errno = ENOEXEC;
	}
	મુક્ત(file);
	પાછા retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(અચલ અક્ષર *service)
{
	અક્ષર *svc;
	અક્ષર *cmd = ખાલી;
	અક્ષર *buffer = ખાલી;
	size_t len = 0;
	RC_STRINGLIST *commands = ખાલી;
	અક્ષર *token;
	અક્ષર *p;
	FILE *fp;

	જો (!(svc = rc_service_resolve(service)))
		પાછા ખાલી;

	xasprintf(&cmd, OPTSTR, svc);
	મુક્ત(svc);

	જો (!(fp = popen(cmd, "r"))) {
		મુક્ત(cmd);
		પાછા ખાલી;
	}

	જો (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		જ્યારે ((token = strsep(&p, " ")))
			જો (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	મુક્ત(buffer);
	મુક્ત(cmd);
	પાછા commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
અક્ષર *
rc_service_description(અચલ અક્ષર *service, અચલ અક્ષર *option)
{
	અક્ષર *svc;
	અક્ષર *cmd;
	અક્ષર *desc = ખાલી;
	size_t size = 0;
	FILE *fp;

	જો (!(svc = rc_service_resolve(service)))
		પાછા ખાલી;

	જો (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	મુક્ત(svc);
	જો ((fp = popen(cmd, "r"))) {
		જો (xgetline(&desc, &size, fp) == -1) {
			મુક્ત(desc);
			desc = ખાલી;
		}
		pclose(fp);
	}
	મુક્ત(cmd);
	પાછા desc;
}

bool
rc_service_in_runlevel(અચલ અક્ષર *service, અચલ અક્ષર *runlevel)
{
	અક્ષર *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	મુક્ત(file);
	પાછા r;
}

bool
rc_service_mark(અચલ અક્ષર *service, અચલ RC_SERVICE state)
{
	પૂર્ણાંક i = 0;
	અચલ અક્ષર *base;
	અક્ષર *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	પૂર્ણાંક serrno;

	જો (!init)
		પાછા false;

	base = basename_c(service);
	જો (state != RC_SERVICE_STOPPED) {
		પૂર્ણાંક state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		જો (access(init, F_OK) != 0) {
			મુક્ત(init);
			પાછા false;
		}

		unlinkat(state_dirfd, base, 0);
		જો ((symlinkat(init, state_dirfd, base)) != 0) {
			મુક્ત(init);
			પાછા false;
		}
	}

	જો (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		મુક્ત(init);
		પાછા true;
	}

	/* Remove any old states now */
	માટે (i = 0; rc_service_state_names[i].name; i++) {
		પૂર્ણાંક state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		જો (s == state)
			ચાલુ;

		પસંદગી (s) {
		સ્થિતિ RC_SERVICE_STOPPED:
		સ્થિતિ RC_SERVICE_HOTPLUGGED:
		સ્થિતિ RC_SERVICE_SCHEDULED:
			ચાલુ;
		સ્થિતિ RC_SERVICE_WASINACTIVE:
			જો (skip_wasinactive)
				ચાલુ;
			/* fall-through */
		મૂળભૂત:
			જો (faccessat(state_dirfd, base, F_OK, 0) != 0)
				ચાલુ;
			જો ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				જો (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					મુક્ત(init);
					પાછા false;
				}
				skip_wasinactive = true;
			}
			જો (unlinkat(state_dirfd, base, 0) == -1) {
				મુક્ત(init);
				પાછા false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	જો (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	જો (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	જો (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		જો (dp) {
			સંરચના dirent *d;
			જ્યારે ((d = readdir(dp))) {
				અક્ષર *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				મુક્ત(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	મુક્ત(init);
	પાછા true;
}

RC_SERVICE
rc_service_state(અચલ અક્ષર *service)
{
	અચલ અક્ષર *base = basename_c(service);
	પૂર્ણાંક state = RC_SERVICE_STOPPED;
	પૂર્ણાંક i;

	માટે (i = 0; rc_service_state_names[i].name; i++) {
		જો (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			જો (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			નહીંતર
				state |= rc_service_state_names[i].state;
		}
	}

	જો (state & RC_SERVICE_STARTED)
		જો (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	જો (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		જો (dp) {
			સંરચના dirent *d;
			જ્યારે ((d = readdir(dp))) {
				જો (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					તોડો;
				}
			}

			closedir(dp);
		}
	}

	પાછા state;
}

અક્ષર *
rc_service_કિંમત_get(અચલ અક્ષર *service, અચલ અક્ષર *option)
{
	અક્ષર *buffer = ખાલી;
	size_t len = 0;
	અક્ષર *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	મુક્ત(file);

	પાછા buffer;
}

bool
rc_service_કિંમત_set(અચલ અક્ષર *service, અચલ અક્ષર *option, અચલ અક્ષર *કિંમત)
{
	bool ret = true;
	પૂર્ણાંક optdirfd;
	FILE *fp;

	જો (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		પાછા false;

	જો ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		પાછા false;

	જો (!કિંમત) {
		unlinkat(optdirfd, option, 0);
	} નહીંતર જો ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", કિંમત);
		fclose(fp);
	} નહીંતર {
		ret = false;
	}

	close(optdirfd);
	પાછા ret;
}

bool
rc_service_schedule_start(અચલ અક્ષર *service, અચલ અક્ષર *service_to_start)
{
	અચલ અક્ષર *base_to_start = basename_c(service_to_start);
	અચલ અક્ષર *base = basename_c(service);
	bool retval;
	પૂર્ણાંક schedfd;
	અક્ષર *init;

	/* service may be a provided service, like net */
	જો (!service || !rc_service_exists(service_to_start))
		પાછા false;

	જો (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		પાછા false;

	જો ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		પાછા false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	મુક્ત(init);
	પાછા retval;
}

bool
rc_service_schedule_clear(અચલ અક્ષર *service)
{
	પાછા !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(અચલ અક્ષર *runlevel)
{
	RC_STRINGLIST *list = ખાલી;

	જો (!runlevel) {
		અચલ પૂર્ણાંક *scriptdirfds;
		list = rc_stringlist_new();
		માટે (size_t i = 0, ગણતરી = rc_scriptdirfds(&scriptdirfds); i < ગણતરી; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		પાછા list;
	}

	/* These special levels never contain any services */
	જો (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		પાછા rc_stringlist_new();

	પાછા ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(અચલ અક્ષર *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ઢગલો;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ઢગલો, stacks, entries) {
		sl = rc_services_in_runlevel(ઢગલો->કિંમત);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	પાછા list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	સંરચના dirent *d;
	DIR *dp;

	જો (state != RC_SERVICE_SCHEDULED)
		પાછા ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	જો (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		પાછા list;

	જ્યારે ((d = readdir(dp))) {
		જો (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			ચાલુ;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	પાછા list;
}

bool
rc_service_add(અચલ અક્ષર *runlevel, અચલ અક્ષર *service)
{
	bool retval;
	અક્ષર *init;
	અક્ષર *file;
	અક્ષર *path;
	અક્ષર *binit = ખાલી;
	અક્ષર *i;

	જો (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		પાછા false;
	}

	જો (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		પાછા false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	જો (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), ખાલી);
		જો (path == ખાલી) {
			મુક્ત(init);
			પાછા false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		મુક્ત(path);
		જો (!retval) {
			મુક્ત(init);
			errno = EPERM;
			પાછા false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	જો (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		જાઓ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	મુક્ત(file);

out:
	મુક્ત(binit);
	મુક્ત(init);
	પાછા retval;
}

bool
rc_service_delete(અચલ અક્ષર *runlevel, અચલ અક્ષર *service)
{
	અક્ષર *file;
	પૂર્ણાંક r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	મુક્ત(file);
	પાછા (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(અચલ અક્ષર *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	સંરચના dirent *d;
	DIR *dp;

	જો (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		પાછા list;

	જ્યારે ((d = readdir(dp))) {
		અક્ષર *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		જો (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		મુક્ત(file);
	}

	પાછા list;
}

RC_STRINGLIST *
rc_services_scheduled(અચલ અક્ષર *service)
{
	પાછા ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(અચલ અક્ષર *service, અચલ અક્ષર *env)
{
	size_t name_len = strcspn(env, "=");
	અક્ષર var[strlen(service) + કદ("/") + name_len];
	પૂર્ણાંક dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	જો (env[name_len] != '=')
		પાછા errno = EINVAL, false;

	snprintf(var, કદ(var), "%s/%.*s", service, (પૂર્ણાંક) name_len, env);

	જો (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		પાછા false;

	જો (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		પાછા false;

	fputs(&env[name_len + 1], fp);

	પાછા fclose(fp) == 0;
}

bool
rc_service_setenv(અચલ અક્ષર *service, અચલ અક્ષર *name, અચલ અક્ષર *કિંમત)
{
	અક્ષર var[strlen(service) + કદ("/") + strlen(name)];
	પૂર્ણાંક dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, કદ(var), "%s/%s", service, name);

	જો (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		પાછા false;

	જો (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		પાછા false;

	fputs(કિંમત, fp);

	પાછા fclose(fp) == 0;
}

ssize_t
rc_service_getenv(અચલ અક્ષર *service, અચલ અક્ષર *name, અક્ષર **buf)
{
	અક્ષર var[strlen(service) + કદ("/") + strlen(name)], *કિંમત;
	પૂર્ણાંક dfd = rc_dirfd(RC_DIR_ENVIRON);
	સંરચના stat stat;
	size_t ગણતરી;
	FILE *fp;

	snprintf(var, કદ(var), "%s/%s", service, name);

	જો (fstatat(dfd, var, &stat, 0) == -1)
		પાછા -1;

	જો (!buf)
		પાછા stat.st_size;

	જો (!(કિંમત = xrealloc(*buf, stat.st_size + 1)))
		પાછા -1;

	જો (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		પાછા -1;

	ગણતરી = fread(કિંમત, 1, stat.st_size, fp);
	કિંમત[ગણતરી] = '\0';

	fclose(fp);
	*buf = કિંમત;
	પાછા ગણતરી;
}
