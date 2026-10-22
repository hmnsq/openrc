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

#अन्तर्भाव <assert.h>
#अन्तर्भाव <helpers.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <libgen.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <regex.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "queue.h"
#अन्तर्भाव "librc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "rc.h"
#ifdef __FreeBSD__
#  include <sys/sysctl.h>
#endif

#ifndef S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

/* File stream used for plugins to write environ vars to */
FILE *rc_environ_fd = रिक्त;
स्थिर पूर्णाङ्क dirfds[RC_DIR_MAX];

पूर्णाङ्क rc_dirfd(गणना rc_dir dir) {
	पूर्णाङ्क flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	पूर्णाङ्क dirfd;

	यदि (dir < 0 || dir >= RC_DIR_MAX)
		प्रत्यागम -1;
	यदि (dirfds[dir] <= 0) {
		विकल्प (dir) {
		पक्ष RC_DIR_SYSCONF:
			प्रत्यागम dirfds[dir] = open(rc_sysconfdir(), flags);
		पक्ष RC_DIR_USRCONF:
			प्रत्यागम dirfds[dir] = open(rc_usrconfdir(), flags);
		पक्ष RC_DIR_RUNLEVEL:
			प्रत्यागम dirfds[dir] = open(rc_runleveldir(), flags);
		पक्ष RC_DIR_SVCDIR:
			यदि (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				प्रत्यागम -1;
			प्रत्यागम dirfds[dir] = open(rc_svcdir(), flags);
		स्वभाव:
			प्रतिज्ञा(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			यदि ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				यदि (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					प्रत्यागम -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			भङ्ग;
		}
	}
	प्रत्यागम dirfds[dir];
}

शून्य clear_dirfds(शून्य) {
	प्रति (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		यदि (dirfds[i] <= 0)
			अनुवर्त;

		close(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
स्थिर RC_STRINGLIST *
ls_dir(पूर्णाङ्क targetfd, ध्रुव वर्ण *dir, पूर्णाङ्क options)
{
	DIR *dp;
	संरचना dirent *d;
	RC_STRINGLIST *list = रिक्त;
	संरचना stat buf;
	size_t l;

	list = rc_stringlist_new();

	यदि (!(dp = do_opendirat(targetfd, dir)))
		प्रत्यागम list;

	यावत् (((d = readdir(dp)) != रिक्त)) {
		यदि (d->d_name[0] == '.')
			अनुवर्त;
		यदि (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			यदि (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				अनुवर्त;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			यदि (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				अनुवर्त;
		}
		यदि (options & LS_DIR) {
			यदि (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				अनुवर्त;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	प्रत्यागम list;
}

स्थिर bool
rm_dir(पूर्णाङ्क targetfd, ध्रुव वर्ण *pathname, bool शीर्ष)
{
	bool retval = true;
	संरचना dirent *d;
	संरचना stat s;
	DIR *dp;

	यदि (!(dp = do_opendirat(targetfd, pathname)))
		प्रत्यागम false;

	यावत् ((d = readdir(dp))) {
		यदि (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			अनुवर्त;
		यदि (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			भङ्ग;
		}
		यदि (S_ISDIR(s.st_mode)) {
			यदि (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				भङ्ग;
			}
		} अन्यथा {
			यदि (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				भङ्ग;
			}
		}
	}
	closedir(dp);

	यदि (!retval)
		प्रत्यागम false;

	यदि (शीर्ष && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		प्रत्यागम false;

	प्रत्यागम true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
स्थिर bool
file_regex(ध्रुव वर्ण *file, ध्रुव वर्ण *regex)
{
	FILE *fp;
	वर्ण *पंक्ति = रिक्त;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	पूर्णाङ्क result;

	यदि (!(fp = fopen(file, "r")))
		प्रत्यागम false;

	यदि ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		वर्ण buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, परिमाण(buf));
		fprintf(stderr, "file_regex: %s", buf);
		प्रत्यागम false;
	}

	यावत् ((len = xgetline(&पंक्ति, &size, fp)) != -1) {
		वर्ण *str = पंक्ति;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		कुरु {
			यदि (regexec(&re, str, 0, रिक्त, 0) == 0)
				गच्छ found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			यावत् (str < पंक्ति + len && *str == '\0')
				str++;
		} यावत् (str < पंक्ति + len);
	}
	retval = false;
found:
	fclose(fp);
	मुक्त(पंक्ति);
	regfree(&re);

	प्रत्यागम retval;
}
#endif


स्थिर ध्रुव वर्ण *
get_systype(शून्य)
{
	वर्ण *systype = rc_conf_मूल्य("rc_sys");
	यदि (systype) {
		वर्ण *s = systype;
		/* Convert to uppercase */
		यावत् (s && *s) {
			यदि (islower((unsigned वर्ण) *s))
				*s = toupper((unsigned वर्ण) *s);
			s++;
		}
	}
	प्रत्यागम systype;
}

स्थिर ध्रुव वर्ण *
detect_container(ध्रुव वर्ण *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	यदि (systype) {
		यदि (strcmp(systype, RC_SYS_NONE) == 0)
		       प्रत्यागम रिक्त;
		यदि (strcmp(systype, RC_SYS_JAIL) == 0)
			प्रत्यागम RC_SYS_JAIL;
		यदि (strcmp(systype, RC_SYS_PODMAN) == 0)
			प्रत्यागम RC_SYS_PODMAN;
	}

	पूर्णाङ्क jailed = 0;
	size_t len = परिमाण(jailed);

	यदि (sysctlbyname("security.jail.jailed", &jailed, &len, रिक्त, 0) == 0)
		यदि (jailed == 1)
			प्रत्यागम RC_SYS_JAIL;

	यदि (access("/var/run/.containerenv", F_OK) == 0)
		प्रत्यागम RC_SYS_PODMAN;
#endif

#ifdef __linux__
	यदि (systype) {
		यदि (strcmp(systype, RC_SYS_NONE) == 0)
			प्रत्यागम रिक्त;
		यदि (strcmp(systype, RC_SYS_UML) == 0)
			प्रत्यागम RC_SYS_UML;
		यदि (strcmp(systype, RC_SYS_VSERVER) == 0)
			प्रत्यागम RC_SYS_VSERVER;
		यदि (strcmp(systype, RC_SYS_OPENVZ) == 0)
			प्रत्यागम RC_SYS_OPENVZ;
		यदि (strcmp(systype, RC_SYS_LXC) == 0)
			प्रत्यागम RC_SYS_LXC;
		यदि (strcmp(systype, RC_SYS_RKT) == 0)
				प्रत्यागम RC_SYS_RKT;
		यदि (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				प्रत्यागम RC_SYS_SYSTEMD_NSPAWN;
		यदि (strcmp(systype, RC_SYS_DOCKER) == 0)
				प्रत्यागम RC_SYS_DOCKER;
		यदि (strcmp(systype, RC_SYS_PODMAN) == 0)
			प्रत्यागम RC_SYS_PODMAN;
		यदि (strcmp(systype, RC_SYS_WSL) == 0)
			प्रत्यागम RC_SYS_WSL;
	}
	यदि (file_regex("/proc/cpuinfo", "UML"))
		प्रत्यागम RC_SYS_UML;
	अन्यथा यदि (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		प्रत्यागम RC_SYS_VSERVER;
	अन्यथा यदि (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		प्रत्यागम RC_SYS_OPENVZ;
	अन्यथा यदि (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		प्रत्यागम RC_SYS_OPENVZ; /* old test */
	अन्यथा यदि (file_regex("/proc/1/environ", "container=lxc"))
		प्रत्यागम RC_SYS_LXC;
	अन्यथा यदि (file_regex("/proc/1/environ", "container=rkt"))
		प्रत्यागम RC_SYS_RKT;
	अन्यथा यदि (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		प्रत्यागम RC_SYS_SYSTEMD_NSPAWN;
	अन्यथा यदि (access("/run/.containerenv", F_OK) == 0)
		प्रत्यागम RC_SYS_PODMAN;
	अन्यथा यदि (access("/.dockerenv", F_OK) == 0)
		प्रत्यागम RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	अन्यथा यदि (file_regex("/proc/1/environ", "container=docker"))
		प्रत्यागम RC_SYS_DOCKER;
	अन्यथा यदि (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		प्रत्यागम RC_SYS_WSL;
	अन्यथा यदि (getenv("WSL_DISTRO_NAME"))
		प्रत्यागम RC_SYS_WSL;
	अन्यथा यदि (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		प्रत्यागम RC_SYS_WSL;
#endif

	प्रत्यागम रिक्त;
}

स्थिर ध्रुव वर्ण *
detect_vm(ध्रुव वर्ण *systype RC_UNUSED)
{
#ifdef __NetBSD__
	यदि (systype) {
		यदि (strcmp(systype, RC_SYS_NONE) == 0)
			प्रत्यागम रिक्त;
		यदि (strcmp(systype, RC_SYS_XEN0) == 0)
			प्रत्यागम RC_SYS_XEN0;
		यदि (strcmp(systype, RC_SYS_XENU) == 0)
			प्रत्यागम RC_SYS_XENU;
	}
	यदि (access("/kern/xen/privcmd", F_OK) == 0)
		प्रत्यागम RC_SYS_XEN0;
	यदि (access("/kern/xen", F_OK) == 0)
		प्रत्यागम RC_SYS_XENU;
#endif

#ifdef __linux__
	यदि (systype) {
		यदि (strcmp(systype, RC_SYS_NONE) == 0)
			प्रत्यागम रिक्त;
		यदि (strcmp(systype, RC_SYS_XEN0) == 0)
			प्रत्यागम RC_SYS_XEN0;
		यदि (strcmp(systype, RC_SYS_XENU) == 0)
			प्रत्यागम RC_SYS_XENU;
	}
	यदि (access("/proc/xen", F_OK) == 0) {
		यदि (file_regex("/proc/xen/capabilities", "control_d"))
			प्रत्यागम RC_SYS_XEN0;
		प्रत्यागम RC_SYS_XENU;
	}
#endif

	प्रत्यागम रिक्त;
}

ध्रुव वर्ण *
rc_sys(शून्य)
{
	ध्रुव वर्ण *systype;
	ध्रुव वर्ण *sys;

	systype = get_systype();
	sys = detect_container(systype);
	यदि (!sys) {
		sys = detect_vm(systype);
	}

	प्रत्यागम sys;
}

स्थिर ध्रुव वर्ण *
rc_parse_service_state(RC_SERVICE state)
{
	पूर्णाङ्क i;

	प्रति (i = 0; rc_service_state_names[i].name; i++) {
		यदि (rc_service_state_names[i].state == state)
			प्रत्यागम rc_service_state_names[i].name;
	}
	प्रत्यागम रिक्त;
}

स्थिर पूर्णाङ्क
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	प्रति (size_t i = 0; rc_service_state_names[i].name; i++)
		यदि (rc_service_state_names[i].state == state)
			प्रत्यागम rc_service_state_names[i].dir;
	प्रत्यागम -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
स्थिर शून्य
get_runlevel_chain(ध्रुव वर्ण *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	ध्रुव वर्ण *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	यदि (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		प्रत्यागम;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	यदि (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->मूल्य;

		/* Check for loop */
		यदि (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->मूल्य);
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
rc_runlevel_starting(शून्य)
{
	प्रत्यागम faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(शून्य)
{
	प्रत्यागम faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(शून्य)
{
	प्रत्यागम ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

वर्ण *
rc_runlevel_get(शून्य)
{
	FILE *fp;
	वर्ण *runlevel = रिक्त;
	size_t i;

	यदि ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(परिमाण(वर्ण) * PATH_MAX);
		यदि (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			यदि (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} अन्यथा
			*runlevel = '\0';
		fclose(fp);
	}

	यदि (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = रिक्त;
	}

	प्रत्यागम runlevel;
}

bool
rc_runlevel_set(ध्रुव वर्ण *runlevel)
{
	FILE *fp;

	यदि (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		प्रत्यागम false;

	fputs(runlevel, fp);
	fclose(fp);
	प्रत्यागम true;
}

bool
rc_runlevel_exists(ध्रुव वर्ण *runlevel)
{
	संरचना stat buf;

	यदि (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		प्रत्यागम false;
	यदि (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		प्रत्यागम true;
	प्रत्यागम false;
}

bool
rc_runlevel_ढेर(ध्रुव वर्ण *dst, ध्रुव वर्ण *src)
{
	वर्ण *d, *s;
	पूर्णाङ्क r;

	यदि (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		प्रत्यागम false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	मुक्त(d);
	मुक्त(s);
	प्रत्यागम (r == 0);
}

bool
rc_runlevel_unstack(ध्रुव वर्ण *dst, ध्रुव वर्ण *src)
{
	वर्ण *path;
	पूर्णाङ्क r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	मुक्त(path);
	प्रत्यागम (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ध्रुव वर्ण *runlevel)
{
	RC_STRINGLIST *ढेर, *ancestor_list;
	ढेर = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ढेर, ancestor_list);
	rc_stringlist_free(ancestor_list);
	प्रत्यागम ढेर;
}

स्थिर ध्रुव वर्ण *config_path[] = { _RC_PATH, रिक्त };

स्थिर संरचना {
	bool set;
	वर्ण *svcdir, *confdir, *runleveldir;
} rc_dirs;

स्थिर संरचना {
	वर्ण *buffer;
	size_t गिनती;
	पूर्णाङ्क *fds;
	ध्रुव वर्ण **entries, **allocd;
} rc_path;

स्थिर शून्य
free_rc_dirs(शून्य)
{
#define X(ptr) ptr = (मुक्त(ptr), रिक्त)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = रिक्त;
	X(rc_path.buffer);
	X(rc_path.fds);
	rc_path.गिनती = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
स्थिर शून्य
append_path(FILE *fp, ध्रुव वर्ण *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(fp, fmt, va);
	fputc('\0', fp);
	va_end(va);

	rc_path.गिनती++;
}

स्थिर ध्रुव वर्ण *
strnsep(ध्रुव वर्ण **str, size_t *size, ध्रुव वर्ण sep)
{
	ध्रुव वर्ण *ret = *str;

	यदि (!ret)
		प्रत्यागम रिक्त;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	यदि (!**str)
		*str = रिक्त;
	अन्यथा
		*str += 1;

	प्रत्यागम ret;
}

स्थिर ध्रुव वर्ण *
strnullsep(ध्रुव वर्ण **str, size_t *size)
{
	ध्रुव वर्ण *ret = *str;
	size_t len;

	यदि (!ret)
		प्रत्यागम रिक्त;

	len = strnlen(ret, *size - 1) + 1;

	यदि ((*size -= len))
		*str += len;
	अन्यथा
		*str = रिक्त;

	प्रत्यागम ret;
}

स्थिर शून्य
append_user_path(FILE *fp)
{
	ध्रुव वर्ण *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	यदि (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	अन्यथा यावत् ((entry = strnsep(&xdg, &size, ':')))
		यदि (size) append_path(fp, "%.*s/rc", (पूर्णाङ्क) size, entry);

	प्रति (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

स्थिर शून्य
append_rc_path(FILE *fp, ध्रुव वर्ण *path, bool user)
{
	bool has_user = false;
	ध्रुव वर्ण *entry;
	size_t size;

	यावत् ((entry = strnsep(&path, &size, ':'))) {
		यदि (size)
			append_path(fp, "%.*s%s", (पूर्णाङ्क)size, entry, user ? "/user" : "");
		अन्यथा यदि (user && !has_user)
			append_user_path(fp), has_user = true;
		अन्यथा यदि (!user) यावत् (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

स्थिर शून्य
load_dynamic_path(bool user)
{
	ध्रुव वर्ण *path, *entry;
	size_t size, गिनती = 0;
	FILE *fp;

	यदि (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.गिनती = ARRAY_SIZE(config_path) - 1;

	यदि (!(path = getenv("RC_PATH")) && !user)
		प्रत्यागम;

	rc_path.गिनती = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	यदि (path)
		append_rc_path(fp, path, user);
	अन्यथा
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.गिनती + 1, परिमाण(*rc_path.entries));

	path = rc_path.buffer;

	यावत् ((entry = strnullsep(&path, &size)) && गिनती < rc_path.गिनती)
		rc_path.entries[गिनती++] = entry;
	rc_path.entries[rc_path.गिनती] = रिक्त;
}

स्थिर bool is_user = false;

bool
rc_is_user(शून्य)
{
	पूर्णाङ्क _is_user = -1;

	यदि (_is_user == -1) {
		वर्ण *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	प्रत्यागम _is_user;
}

शून्य
rc_set_user(शून्य)
{
	वर्ण *env;

	यदि (is_user)
		प्रत्यागम;

	यदि (!rc_dirs.set)
		atexit(free_rc_dirs);
	अन्यथा
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	यदि ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} अन्यथा यदि ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} अन्यथा {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	यदि (!(env = getenv("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		fprintf(stderr, "XDG_RUNTIME_DIR unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

ध्रुव वर्ण * ध्रुव *
rc_scriptdirs(शून्य)
{
	यदि (!rc_path.entries)
		load_dynamic_path(false);
	प्रत्यागम rc_path.entries;
}

size_t
rc_scriptdirfds(ध्रुव पूर्णाङ्क **fds)
{
	स्थिर size_t len = 0;

	यदि (len == 0) {
		यदि (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.गिनती * परिमाण(*rc_path.fds));
		प्रति (size_t i = 0; i < rc_path.गिनती; i++) {
			पूर्णाङ्क fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			यदि (fd == -1)
				अनुवर्त;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	प्रत्यागम len;
}

ध्रुव वर्ण *
rc_sysconfdir(शून्य)
{
	प्रत्यागम RC_SYSCONFDIR;
}

ध्रुव वर्ण *
rc_usrconfdir(शून्य)
{
	प्रत्यागम rc_dirs.confdir ? rc_dirs.confdir : रिक्त;
}

ध्रुव वर्ण *
rc_runleveldir(शून्य)
{
	प्रत्यागम rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ध्रुव वर्ण *
rc_svcdir(शून्य)
{
	स्थिर ध्रुव वर्ण *svcdir;

	यदि (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	प्रत्यागम rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

स्थिर ssize_t
safe_readlink(पूर्णाङ्क targetfd, ध्रुव वर्ण *path, वर्ण **buf, size_t bufsiz)
{
	वर्ण *buffer;
	ssize_t r;

	प्रति (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		यदि (r < 0) {
			मुक्त(buffer);
			प्रत्यागम -errno;
		}
		यदि ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			प्रत्यागम r;
		}
		मुक्त(buffer);
		यदि (bufsiz >= SSIZE_MAX - PATH_MAX)
			प्रत्यागम -ENOMEM;
	}
}

/* Resolve a service name to its full path */
वर्ण *
rc_service_resolve(ध्रुव वर्ण *service)
{
	वर्ण *buffer;
	संरचना stat buf;

	यदि (!service)
		प्रत्यागम रिक्त;

	यदि (service[0] == '/')
		प्रत्यागम xstrdup(service);

	/* First check started services */
	यदि (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		यदि (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			प्रत्यागम buffer;
	यदि (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		यदि (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			प्रत्यागम buffer;

	प्रति (ध्रुव वर्ण * ध्रुव *dirs = rc_scriptdirs(); *dirs; dirs++) {
		वर्ण *file = रिक्त;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		यदि (stat(file, &buf) == 0)
			प्रत्यागम file;
		मुक्त(file);
	}

	प्रत्यागम रिक्त;
}

bool
rc_service_exists(ध्रुव वर्ण *service)
{
	वर्ण *file;
	bool retval = false;
	size_t len;
	संरचना stat buf;

	यदि (!service) {
		errno = EINVAL;
		प्रत्यागम false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	यदि (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		प्रत्यागम false;
	}

	यदि (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		प्रत्यागम false;
	}

	यदि (stat(file, &buf) == 0) {
		यदि (buf.st_mode & S_IXUGO)
			retval = true;
		अन्यथा
			errno = ENOEXEC;
	}
	मुक्त(file);
	प्रत्यागम retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(ध्रुव वर्ण *service)
{
	वर्ण *svc;
	वर्ण *cmd = रिक्त;
	वर्ण *buffer = रिक्त;
	size_t len = 0;
	RC_STRINGLIST *commands = रिक्त;
	वर्ण *token;
	वर्ण *p;
	FILE *fp;

	यदि (!(svc = rc_service_resolve(service)))
		प्रत्यागम रिक्त;

	xasprintf(&cmd, OPTSTR, svc);
	मुक्त(svc);

	यदि (!(fp = popen(cmd, "r"))) {
		मुक्त(cmd);
		प्रत्यागम रिक्त;
	}

	यदि (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		यावत् ((token = strsep(&p, " ")))
			यदि (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	मुक्त(buffer);
	मुक्त(cmd);
	प्रत्यागम commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
वर्ण *
rc_service_description(ध्रुव वर्ण *service, ध्रुव वर्ण *option)
{
	वर्ण *svc;
	वर्ण *cmd;
	वर्ण *desc = रिक्त;
	size_t size = 0;
	FILE *fp;

	यदि (!(svc = rc_service_resolve(service)))
		प्रत्यागम रिक्त;

	यदि (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	मुक्त(svc);
	यदि ((fp = popen(cmd, "r"))) {
		यदि (xgetline(&desc, &size, fp) == -1) {
			मुक्त(desc);
			desc = रिक्त;
		}
		pclose(fp);
	}
	मुक्त(cmd);
	प्रत्यागम desc;
}

bool
rc_service_in_runlevel(ध्रुव वर्ण *service, ध्रुव वर्ण *runlevel)
{
	वर्ण *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	मुक्त(file);
	प्रत्यागम r;
}

bool
rc_service_mark(ध्रुव वर्ण *service, ध्रुव RC_SERVICE state)
{
	पूर्णाङ्क i = 0;
	ध्रुव वर्ण *base;
	वर्ण *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	पूर्णाङ्क serrno;

	यदि (!init)
		प्रत्यागम false;

	base = basename_c(service);
	यदि (state != RC_SERVICE_STOPPED) {
		पूर्णाङ्क state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		यदि (access(init, F_OK) != 0) {
			मुक्त(init);
			प्रत्यागम false;
		}

		unlinkat(state_dirfd, base, 0);
		यदि ((symlinkat(init, state_dirfd, base)) != 0) {
			मुक्त(init);
			प्रत्यागम false;
		}
	}

	यदि (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		मुक्त(init);
		प्रत्यागम true;
	}

	/* Remove any old states now */
	प्रति (i = 0; rc_service_state_names[i].name; i++) {
		पूर्णाङ्क state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		यदि (s == state)
			अनुवर्त;

		विकल्प (s) {
		पक्ष RC_SERVICE_STOPPED:
		पक्ष RC_SERVICE_HOTPLUGGED:
		पक्ष RC_SERVICE_SCHEDULED:
			अनुवर्त;
		पक्ष RC_SERVICE_WASINACTIVE:
			यदि (skip_wasinactive)
				अनुवर्त;
			/* fall-through */
		स्वभाव:
			यदि (faccessat(state_dirfd, base, F_OK, 0) != 0)
				अनुवर्त;
			यदि ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				यदि (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					मुक्त(init);
					प्रत्यागम false;
				}
				skip_wasinactive = true;
			}
			यदि (unlinkat(state_dirfd, base, 0) == -1) {
				मुक्त(init);
				प्रत्यागम false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	यदि (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	यदि (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	यदि (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		यदि (dp) {
			संरचना dirent *d;
			यावत् ((d = readdir(dp))) {
				वर्ण *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				मुक्त(was);

				serrno = errno;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				errno = serrno;
			}

			closedir(dp);
		}
	}
	मुक्त(init);
	प्रत्यागम true;
}

RC_SERVICE
rc_service_state(ध्रुव वर्ण *service)
{
	ध्रुव वर्ण *base = basename_c(service);
	पूर्णाङ्क state = RC_SERVICE_STOPPED;
	पूर्णाङ्क i;

	प्रति (i = 0; rc_service_state_names[i].name; i++) {
		यदि (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			यदि (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			अन्यथा
				state |= rc_service_state_names[i].state;
		}
	}

	यदि (state & RC_SERVICE_STARTED)
		यदि (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	यदि (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		यदि (dp) {
			संरचना dirent *d;
			यावत् ((d = readdir(dp))) {
				यदि (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					भङ्ग;
				}
			}

			closedir(dp);
		}
	}

	प्रत्यागम state;
}

वर्ण *
rc_service_मूल्य_get(ध्रुव वर्ण *service, ध्रुव वर्ण *option)
{
	वर्ण *buffer = रिक्त;
	size_t len = 0;
	वर्ण *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	मुक्त(file);

	प्रत्यागम buffer;
}

bool
rc_service_मूल्य_set(ध्रुव वर्ण *service, ध्रुव वर्ण *option, ध्रुव वर्ण *मूल्य)
{
	bool ret = true;
	पूर्णाङ्क optdirfd;
	FILE *fp;

	यदि (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		प्रत्यागम false;

	यदि ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		प्रत्यागम false;

	यदि (!मूल्य) {
		unlinkat(optdirfd, option, 0);
	} अन्यथा यदि ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", मूल्य);
		fclose(fp);
	} अन्यथा {
		ret = false;
	}

	close(optdirfd);
	प्रत्यागम ret;
}

bool
rc_service_schedule_start(ध्रुव वर्ण *service, ध्रुव वर्ण *service_to_start)
{
	ध्रुव वर्ण *base_to_start = basename_c(service_to_start);
	ध्रुव वर्ण *base = basename_c(service);
	bool retval;
	पूर्णाङ्क schedfd;
	वर्ण *init;

	/* service may be a provided service, like net */
	यदि (!service || !rc_service_exists(service_to_start))
		प्रत्यागम false;

	यदि (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		प्रत्यागम false;

	यदि ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		प्रत्यागम false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	मुक्त(init);
	प्रत्यागम retval;
}

bool
rc_service_schedule_clear(ध्रुव वर्ण *service)
{
	प्रत्यागम !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ध्रुव वर्ण *runlevel)
{
	RC_STRINGLIST *list = रिक्त;

	यदि (!runlevel) {
		ध्रुव पूर्णाङ्क *scriptdirfds;
		list = rc_stringlist_new();
		प्रति (size_t i = 0, गिनती = rc_scriptdirfds(&scriptdirfds); i < गिनती; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		प्रत्यागम list;
	}

	/* These special levels never contain any services */
	यदि (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		प्रत्यागम rc_stringlist_new();

	प्रत्यागम ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ध्रुव वर्ण *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *ढेर;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(ढेर, stacks, entries) {
		sl = rc_services_in_runlevel(ढेर->मूल्य);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	प्रत्यागम list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	संरचना dirent *d;
	DIR *dp;

	यदि (state != RC_SERVICE_SCHEDULED)
		प्रत्यागम ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	यदि (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		प्रत्यागम list;

	यावत् ((d = readdir(dp))) {
		यदि (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			अनुवर्त;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	प्रत्यागम list;
}

bool
rc_service_add(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *service)
{
	bool retval;
	वर्ण *init;
	वर्ण *file;
	वर्ण *path;
	वर्ण *binit = रिक्त;
	वर्ण *i;

	यदि (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		प्रत्यागम false;
	}

	यदि (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		प्रत्यागम false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	यदि (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), रिक्त);
		यदि (path == रिक्त) {
			मुक्त(init);
			प्रत्यागम false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		मुक्त(path);
		यदि (!retval) {
			मुक्त(init);
			errno = EPERM;
			प्रत्यागम false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	यदि (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
		errno = ENOENT;
		retval = false;
		गच्छ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	मुक्त(file);

out:
	मुक्त(binit);
	मुक्त(init);
	प्रत्यागम retval;
}

bool
rc_service_delete(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *service)
{
	वर्ण *file;
	पूर्णाङ्क r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	मुक्त(file);
	प्रत्यागम (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ध्रुव वर्ण *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	संरचना dirent *d;
	DIR *dp;

	यदि (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		प्रत्यागम list;

	यावत् ((d = readdir(dp))) {
		वर्ण *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		यदि (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		मुक्त(file);
	}

	प्रत्यागम list;
}

RC_STRINGLIST *
rc_services_scheduled(ध्रुव वर्ण *service)
{
	प्रत्यागम ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ध्रुव वर्ण *service, ध्रुव वर्ण *env)
{
	size_t name_len = strcspn(env, "=");
	वर्ण var[strlen(service) + परिमाण("/") + name_len];
	पूर्णाङ्क dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	यदि (env[name_len] != '=')
		प्रत्यागम errno = EINVAL, false;

	snprintf(var, परिमाण(var), "%s/%.*s", service, (पूर्णाङ्क) name_len, env);

	यदि (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		प्रत्यागम false;

	यदि (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		प्रत्यागम false;

	fputs(&env[name_len + 1], fp);

	प्रत्यागम fclose(fp) == 0;
}

bool
rc_service_setenv(ध्रुव वर्ण *service, ध्रुव वर्ण *name, ध्रुव वर्ण *मूल्य)
{
	वर्ण var[strlen(service) + परिमाण("/") + strlen(name)];
	पूर्णाङ्क dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, परिमाण(var), "%s/%s", service, name);

	यदि (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		प्रत्यागम false;

	यदि (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		प्रत्यागम false;

	fputs(मूल्य, fp);

	प्रत्यागम fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ध्रुव वर्ण *service, ध्रुव वर्ण *name, वर्ण **buf)
{
	वर्ण var[strlen(service) + परिमाण("/") + strlen(name)], *मूल्य;
	पूर्णाङ्क dfd = rc_dirfd(RC_DIR_ENVIRON);
	संरचना stat stat;
	size_t गिनती;
	FILE *fp;

	snprintf(var, परिमाण(var), "%s/%s", service, name);

	यदि (fstatat(dfd, var, &stat, 0) == -1)
		प्रत्यागम -1;

	यदि (!buf)
		प्रत्यागम stat.st_size;

	यदि (!(मूल्य = xrealloc(*buf, stat.st_size + 1)))
		प्रत्यागम -1;

	यदि (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		प्रत्यागम -1;

	गिनती = fread(मूल्य, 1, stat.st_size, fp);
	मूल्य[गिनती] = '\0';

	fclose(fp);
	*buf = मूल्य;
	प्रत्यागम गिनती;
}
