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

	सचे (dir < 0 || dir >= RC_DIR_MAX)
		पटिनिवत्त -1;
	सचे (dirfds[dir] <= 0) {
		विकल्प (dir) {
		पक्ष RC_DIR_SYSCONF:
			पटिनिवत्त dirfds[dir] = open(rc_sysconfdir(), flags);
		पक्ष RC_DIR_USRCONF:
			पटिनिवत्त dirfds[dir] = open(rc_usrconfdir(), flags);
		पक्ष RC_DIR_RUNLEVEL:
			पटिनिवत्त dirfds[dir] = open(rc_runleveldir(), flags);
		पक्ष RC_DIR_SVCDIR:
			सचे (mkdir(rc_svcdir(), 0755) == -1 && errno != EEXIST)
				पटिनिवत्त -1;
			पटिनिवत्त dirfds[dir] = open(rc_svcdir(), flags);
		स्वभाव:
			प्रतिज्ञा(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			सचे ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && errno == ENOENT) {
				सचे (mkdirat(dirfd, dirnames[dir], 0755) == -1 && errno != EEXIST)
					पटिनिवत्त -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			भङ्ग;
		}
	}
	पटिनिवत्त dirfds[dir];
}

शून्य clear_dirfds(शून्य) {
	अत्थाय (size_t i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		सचे (dirfds[i] <= 0)
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

	सचे (!(dp = do_opendirat(targetfd, dir)))
		पटिनिवत्त list;

	याव (((d = readdir(dp)) != रिक्त)) {
		सचे (d->d_name[0] == '.')
			अनुवर्त;
		सचे (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			सचे (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				अनुवर्त;

			/* .sh files are not init scripts */
			l = strlen(d->d_name);
			सचे (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				अनुवर्त;
		}
		सचे (options & LS_DIR) {
			सचे (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				अनुवर्त;
		}
		rc_stringlist_add(list, d->d_name);
	}
	closedir(dp);
	पटिनिवत्त list;
}

स्थिर bool
rm_dir(पूर्णाङ्क targetfd, ध्रुव वर्ण *pathname, bool शिखरम्)
{
	bool retval = true;
	संरचना dirent *d;
	संरचना stat s;
	DIR *dp;

	सचे (!(dp = do_opendirat(targetfd, pathname)))
		पटिनिवत्त false;

	याव ((d = readdir(dp))) {
		सचे (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			अनुवर्त;
		सचे (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			भङ्ग;
		}
		सचे (S_ISDIR(s.st_mode)) {
			सचे (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				भङ्ग;
			}
		} अञ्ञथा {
			सचे (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				भङ्ग;
			}
		}
	}
	closedir(dp);

	सचे (!retval)
		पटिनिवत्त false;

	सचे (शिखरम् && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		पटिनिवत्त false;

	पटिनिवत्त true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
स्थिर bool
file_regex(ध्रुव वर्ण *file, ध्रुव वर्ण *regex)
{
	FILE *fp;
	वर्ण *पङ्क्तिः = रिक्त;
	size_t size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	पूर्णाङ्क result;

	सचे (!(fp = fopen(file, "r")))
		पटिनिवत्त false;

	सचे ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		वर्ण buf[BUFSIZ];
		fclose(fp);
		regerror(result, &re, buf, परिमाण(buf));
		fprintf(stderr, "file_regex: %s", buf);
		पटिनिवत्त false;
	}

	याव ((len = xgetline(&पङ्क्तिः, &size, fp)) != -1) {
		वर्ण *str = पङ्क्तिः;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		करोहि {
			सचे (regexec(&re, str, 0, रिक्त, 0) == 0)
				गच्छ found;
			str += strlen(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			याव (str < पङ्क्तिः + len && *str == '\0')
				str++;
		} याव (str < पङ्क्तिः + len);
	}
	retval = false;
found:
	fclose(fp);
	मुक्त(पङ्क्तिः);
	regfree(&re);

	पटिनिवत्त retval;
}
#endif


स्थिर ध्रुव वर्ण *
get_systype(शून्य)
{
	वर्ण *systype = rc_conf_मानम्("rc_sys");
	सचे (systype) {
		वर्ण *s = systype;
		/* Convert to uppercase */
		याव (s && *s) {
			सचे (islower((unsigned वर्ण) *s))
				*s = toupper((unsigned वर्ण) *s);
			s++;
		}
	}
	पटिनिवत्त systype;
}

स्थिर ध्रुव वर्ण *
detect_container(ध्रुव वर्ण *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	सचे (systype) {
		सचे (strcmp(systype, RC_SYS_NONE) == 0)
		       पटिनिवत्त रिक्त;
		सचे (strcmp(systype, RC_SYS_JAIL) == 0)
			पटिनिवत्त RC_SYS_JAIL;
		सचे (strcmp(systype, RC_SYS_PODMAN) == 0)
			पटिनिवत्त RC_SYS_PODMAN;
	}

	पूर्णाङ्क jailed = 0;
	size_t len = परिमाण(jailed);

	सचे (sysctlbyname("security.jail.jailed", &jailed, &len, रिक्त, 0) == 0)
		सचे (jailed == 1)
			पटिनिवत्त RC_SYS_JAIL;

	सचे (access("/var/run/.containerenv", F_OK) == 0)
		पटिनिवत्त RC_SYS_PODMAN;
#endif

#ifdef __linux__
	सचे (systype) {
		सचे (strcmp(systype, RC_SYS_NONE) == 0)
			पटिनिवत्त रिक्त;
		सचे (strcmp(systype, RC_SYS_UML) == 0)
			पटिनिवत्त RC_SYS_UML;
		सचे (strcmp(systype, RC_SYS_VSERVER) == 0)
			पटिनिवत्त RC_SYS_VSERVER;
		सचे (strcmp(systype, RC_SYS_OPENVZ) == 0)
			पटिनिवत्त RC_SYS_OPENVZ;
		सचे (strcmp(systype, RC_SYS_LXC) == 0)
			पटिनिवत्त RC_SYS_LXC;
		सचे (strcmp(systype, RC_SYS_RKT) == 0)
				पटिनिवत्त RC_SYS_RKT;
		सचे (strcmp(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				पटिनिवत्त RC_SYS_SYSTEMD_NSPAWN;
		सचे (strcmp(systype, RC_SYS_DOCKER) == 0)
				पटिनिवत्त RC_SYS_DOCKER;
		सचे (strcmp(systype, RC_SYS_PODMAN) == 0)
			पटिनिवत्त RC_SYS_PODMAN;
		सचे (strcmp(systype, RC_SYS_WSL) == 0)
			पटिनिवत्त RC_SYS_WSL;
	}
	सचे (file_regex("/proc/cpuinfo", "UML"))
		पटिनिवत्त RC_SYS_UML;
	अञ्ञथा सचे (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		पटिनिवत्त RC_SYS_VSERVER;
	अञ्ञथा सचे (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		पटिनिवत्त RC_SYS_OPENVZ;
	अञ्ञथा सचे (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		पटिनिवत्त RC_SYS_OPENVZ; /* old test */
	अञ्ञथा सचे (file_regex("/proc/1/environ", "container=lxc"))
		पटिनिवत्त RC_SYS_LXC;
	अञ्ञथा सचे (file_regex("/proc/1/environ", "container=rkt"))
		पटिनिवत्त RC_SYS_RKT;
	अञ्ञथा सचे (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		पटिनिवत्त RC_SYS_SYSTEMD_NSPAWN;
	अञ्ञथा सचे (access("/run/.containerenv", F_OK) == 0)
		पटिनिवत्त RC_SYS_PODMAN;
	अञ्ञथा सचे (access("/.dockerenv", F_OK) == 0)
		पटिनिवत्त RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	अञ्ञथा सचे (file_regex("/proc/1/environ", "container=docker"))
		पटिनिवत्त RC_SYS_DOCKER;
	अञ्ञथा सचे (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		पटिनिवत्त RC_SYS_WSL;
	अञ्ञथा सचे (getenv("WSL_DISTRO_NAME"))
		पटिनिवत्त RC_SYS_WSL;
	अञ्ञथा सचे (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		पटिनिवत्त RC_SYS_WSL;
#endif

	पटिनिवत्त रिक्त;
}

स्थिर ध्रुव वर्ण *
detect_vm(ध्रुव वर्ण *systype RC_UNUSED)
{
#ifdef __NetBSD__
	सचे (systype) {
		सचे (strcmp(systype, RC_SYS_NONE) == 0)
			पटिनिवत्त रिक्त;
		सचे (strcmp(systype, RC_SYS_XEN0) == 0)
			पटिनिवत्त RC_SYS_XEN0;
		सचे (strcmp(systype, RC_SYS_XENU) == 0)
			पटिनिवत्त RC_SYS_XENU;
	}
	सचे (access("/kern/xen/privcmd", F_OK) == 0)
		पटिनिवत्त RC_SYS_XEN0;
	सचे (access("/kern/xen", F_OK) == 0)
		पटिनिवत्त RC_SYS_XENU;
#endif

#ifdef __linux__
	सचे (systype) {
		सचे (strcmp(systype, RC_SYS_NONE) == 0)
			पटिनिवत्त रिक्त;
		सचे (strcmp(systype, RC_SYS_XEN0) == 0)
			पटिनिवत्त RC_SYS_XEN0;
		सचे (strcmp(systype, RC_SYS_XENU) == 0)
			पटिनिवत्त RC_SYS_XENU;
	}
	सचे (access("/proc/xen", F_OK) == 0) {
		सचे (file_regex("/proc/xen/capabilities", "control_d"))
			पटिनिवत्त RC_SYS_XEN0;
		पटिनिवत्त RC_SYS_XENU;
	}
#endif

	पटिनिवत्त रिक्त;
}

ध्रुव वर्ण *
rc_sys(शून्य)
{
	ध्रुव वर्ण *systype;
	ध्रुव वर्ण *sys;

	systype = get_systype();
	sys = detect_container(systype);
	सचे (!sys) {
		sys = detect_vm(systype);
	}

	पटिनिवत्त sys;
}

स्थिर ध्रुव वर्ण *
rc_parse_service_state(RC_SERVICE state)
{
	पूर्णाङ्क i;

	अत्थाय (i = 0; rc_service_state_names[i].name; i++) {
		सचे (rc_service_state_names[i].state == state)
			पटिनिवत्त rc_service_state_names[i].name;
	}
	पटिनिवत्त रिक्त;
}

स्थिर पूर्णाङ्क
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	अत्थाय (size_t i = 0; rc_service_state_names[i].name; i++)
		सचे (rc_service_state_names[i].state == state)
			पटिनिवत्त rc_service_state_names[i].dir;
	पटिनिवत्त -1;
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
	सचे (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		पटिनिवत्त;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	सचे (rc_stringlist_find(level_list, runlevel))
		rc_stringlist_delete(level_list, runlevel);
	rc_stringlist_add(level_list, runlevel);

	/*
	 * We can now do exactly the above procedure for our chained
	 * runlevels.
	 */
	dirs = ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_DIR);
	TAILQ_FOREACH(d, dirs, entries) {
		nextlevel = d->मानम्;

		/* Check for loop */
		सचे (rc_stringlist_find(ancestor_list, nextlevel)) {
			fprintf(stderr, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			fprintf(stderr, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				fprintf(stderr, "\t%s\n", parent->मानम्);
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
	पटिनिवत्त faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(शून्य)
{
	पटिनिवत्त faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(शून्य)
{
	पटिनिवत्त ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

वर्ण *
rc_runlevel_get(शून्य)
{
	FILE *fp;
	वर्ण *runlevel = रिक्त;
	size_t i;

	सचे ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(परिमाण(वर्ण) * PATH_MAX);
		सचे (fgets(runlevel, PATH_MAX, fp)) {
			i = strlen(runlevel);
			सचे (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} अञ्ञथा
			*runlevel = '\0';
		fclose(fp);
	}

	सचे (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = रिक्त;
	}

	पटिनिवत्त runlevel;
}

bool
rc_runlevel_set(ध्रुव वर्ण *runlevel)
{
	FILE *fp;

	सचे (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		पटिनिवत्त false;

	fputs(runlevel, fp);
	fclose(fp);
	पटिनिवत्त true;
}

bool
rc_runlevel_exists(ध्रुव वर्ण *runlevel)
{
	संरचना stat buf;

	सचे (!runlevel || strcmp(runlevel, "") == 0 || strcmp(runlevel, ".") == 0 || strcmp(runlevel, "..") == 0)
		पटिनिवत्त false;
	सचे (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		पटिनिवत्त true;
	पटिनिवत्त false;
}

bool
rc_runlevel_राशिः(ध्रुव वर्ण *dst, ध्रुव वर्ण *src)
{
	वर्ण *d, *s;
	पूर्णाङ्क r;

	सचे (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		पटिनिवत्त false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	मुक्त(d);
	मुक्त(s);
	पटिनिवत्त (r == 0);
}

bool
rc_runlevel_unstack(ध्रुव वर्ण *dst, ध्रुव वर्ण *src)
{
	वर्ण *path;
	पूर्णाङ्क r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	मुक्त(path);
	पटिनिवत्त (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(ध्रुव वर्ण *runlevel)
{
	RC_STRINGLIST *राशिः, *ancestor_list;
	राशिः = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, राशिः, ancestor_list);
	rc_stringlist_free(ancestor_list);
	पटिनिवत्त राशिः;
}

स्थिर ध्रुव वर्ण *config_path[] = { _RC_PATH, रिक्त };

स्थिर संरचना {
	bool set;
	वर्ण *svcdir, *confdir, *runleveldir;
} rc_dirs;

स्थिर संरचना {
	वर्ण *buffer;
	size_t गणनम्;
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
	rc_path.गणनम् = 0;
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

	rc_path.गणनम्++;
}

स्थिर ध्रुव वर्ण *
strnsep(ध्रुव वर्ण **str, size_t *size, ध्रुव वर्ण sep)
{
	ध्रुव वर्ण *ret = *str;

	सचे (!ret)
		पटिनिवत्त रिक्त;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	सचे (!**str)
		*str = रिक्त;
	अञ्ञथा
		*str += 1;

	पटिनिवत्त ret;
}

स्थिर ध्रुव वर्ण *
strnullsep(ध्रुव वर्ण **str, size_t *size)
{
	ध्रुव वर्ण *ret = *str;
	size_t len;

	सचे (!ret)
		पटिनिवत्त रिक्त;

	len = strnlen(ret, *size - 1) + 1;

	सचे ((*size -= len))
		*str += len;
	अञ्ञथा
		*str = रिक्त;

	पटिनिवत्त ret;
}

स्थिर शून्य
append_user_path(FILE *fp)
{
	ध्रुव वर्ण *xdg, *entry;
	size_t size;

	append_path(fp, "%s", rc_dirs.confdir);
	सचे (!(xdg = getenv("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	अञ्ञथा याव ((entry = strnsep(&xdg, &size, ':')))
		सचे (size) append_path(fp, "%.*s/rc", (पूर्णाङ्क) size, entry);

	अत्थाय (size_t i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

स्थिर शून्य
append_rc_path(FILE *fp, ध्रुव वर्ण *path, bool user)
{
	bool has_user = false;
	ध्रुव वर्ण *entry;
	size_t size;

	याव ((entry = strnsep(&path, &size, ':'))) {
		सचे (size)
			append_path(fp, "%.*s%s", (पूर्णाङ्क)size, entry, user ? "/user" : "");
		अञ्ञथा सचे (user && !has_user)
			append_user_path(fp), has_user = true;
		अञ्ञथा सचे (!user) याव (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

स्थिर शून्य
load_dynamic_path(bool user)
{
	ध्रुव वर्ण *path, *entry;
	size_t size, गणनम् = 0;
	FILE *fp;

	सचे (!rc_dirs.set) {
		atexit(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.गणनम् = ARRAY_SIZE(config_path) - 1;

	सचे (!(path = getenv("RC_PATH")) && !user)
		पटिनिवत्त;

	rc_path.गणनम् = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	सचे (path)
		append_rc_path(fp, path, user);
	अञ्ञथा
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = calloc(rc_path.गणनम् + 1, परिमाण(*rc_path.entries));

	path = rc_path.buffer;

	याव ((entry = strnullsep(&path, &size)) && गणनम् < rc_path.गणनम्)
		rc_path.entries[गणनम्++] = entry;
	rc_path.entries[rc_path.गणनम्] = रिक्त;
}

स्थिर bool is_user = false;

bool
rc_is_user(शून्य)
{
	पूर्णाङ्क _is_user = -1;

	सचे (_is_user == -1) {
		वर्ण *env = getenv("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	पटिनिवत्त _is_user;
}

शून्य
rc_set_user(शून्य)
{
	वर्ण *env;

	सचे (is_user)
		पटिनिवत्त;

	सचे (!rc_dirs.set)
		atexit(free_rc_dirs);
	अञ्ञथा
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	सचे ((env = getenv("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} अञ्ञथा सचे ((env = getenv("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} अञ्ञथा {
		fprintf(stderr, "XDG_CONFIG_HOME and HOME unset.\n");
		exit(EXIT_FAILURE);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	सचे (!(env = getenv("XDG_RUNTIME_DIR"))) {
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
	सचे (!rc_path.entries)
		load_dynamic_path(false);
	पटिनिवत्त rc_path.entries;
}

size_t
rc_scriptdirfds(ध्रुव पूर्णाङ्क **fds)
{
	स्थिर size_t len = 0;

	सचे (len == 0) {
		सचे (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.गणनम् * परिमाण(*rc_path.fds));
		अत्थाय (size_t i = 0; i < rc_path.गणनम्; i++) {
			पूर्णाङ्क fd = open(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			सचे (fd == -1)
				अनुवर्त;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	पटिनिवत्त len;
}

ध्रुव वर्ण *
rc_sysconfdir(शून्य)
{
	पटिनिवत्त RC_SYSCONFDIR;
}

ध्रुव वर्ण *
rc_usrconfdir(शून्य)
{
	पटिनिवत्त rc_dirs.confdir ? rc_dirs.confdir : रिक्त;
}

ध्रुव वर्ण *
rc_runleveldir(शून्य)
{
	पटिनिवत्त rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

ध्रुव वर्ण *
rc_svcdir(शून्य)
{
	स्थिर ध्रुव वर्ण *svcdir;

	सचे (!rc_dirs.svcdir && !svcdir && !(svcdir = getenv("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	पटिनिवत्त rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

स्थिर ssize_t
safe_readlink(पूर्णाङ्क targetfd, ध्रुव वर्ण *path, वर्ण **buf, size_t bufsiz)
{
	वर्ण *buffer;
	ssize_t r;

	अत्थाय (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		सचे (r < 0) {
			मुक्त(buffer);
			पटिनिवत्त -errno;
		}
		सचे ((size_t)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			पटिनिवत्त r;
		}
		मुक्त(buffer);
		सचे (bufsiz >= SSIZE_MAX - PATH_MAX)
			पटिनिवत्त -ENOMEM;
	}
}

/* Resolve a service name to its full path */
वर्ण *
rc_service_resolve(ध्रुव वर्ण *service)
{
	वर्ण *buffer;
	संरचना stat buf;

	सचे (!service)
		पटिनिवत्त रिक्त;

	सचे (service[0] == '/')
		पटिनिवत्त xstrdup(service);

	/* First check started services */
	सचे (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		सचे (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			पटिनिवत्त buffer;
	सचे (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		सचे (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			पटिनिवत्त buffer;

	अत्थाय (ध्रुव वर्ण * ध्रुव *dirs = rc_scriptdirs(); *dirs; dirs++) {
		वर्ण *file = रिक्त;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		सचे (stat(file, &buf) == 0)
			पटिनिवत्त file;
		मुक्त(file);
	}

	पटिनिवत्त रिक्त;
}

bool
rc_service_exists(ध्रुव वर्ण *service)
{
	वर्ण *file;
	bool retval = false;
	size_t len;
	संरचना stat buf;

	सचे (!service) {
		errno = EINVAL;
		पटिनिवत्त false;
	}

	len = strlen(service);

	/* .sh files are not init scripts */
	सचे (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		errno = EINVAL;
		पटिनिवत्त false;
	}

	सचे (!(file = rc_service_resolve(service))) {
		errno = ENOENT;
		पटिनिवत्त false;
	}

	सचे (stat(file, &buf) == 0) {
		सचे (buf.st_mode & S_IXUGO)
			retval = true;
		अञ्ञथा
			errno = ENOEXEC;
	}
	मुक्त(file);
	पटिनिवत्त retval;
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

	सचे (!(svc = rc_service_resolve(service)))
		पटिनिवत्त रिक्त;

	xasprintf(&cmd, OPTSTR, svc);
	मुक्त(svc);

	सचे (!(fp = popen(cmd, "r"))) {
		मुक्त(cmd);
		पटिनिवत्त रिक्त;
	}

	सचे (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		याव ((token = strsep(&p, " ")))
			सचे (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	मुक्त(buffer);
	मुक्त(cmd);
	पटिनिवत्त commands;
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

	सचे (!(svc = rc_service_resolve(service)))
		पटिनिवत्त रिक्त;

	सचे (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	मुक्त(svc);
	सचे ((fp = popen(cmd, "r"))) {
		सचे (xgetline(&desc, &size, fp) == -1) {
			मुक्त(desc);
			desc = रिक्त;
		}
		pclose(fp);
	}
	मुक्त(cmd);
	पटिनिवत्त desc;
}

bool
rc_service_in_runlevel(ध्रुव वर्ण *service, ध्रुव वर्ण *runlevel)
{
	वर्ण *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	मुक्त(file);
	पटिनिवत्त r;
}

bool
rc_service_mark(ध्रुव वर्ण *service, ध्रुव RC_SERVICE state)
{
	पूर्णाङ्क i = 0;
	ध्रुव वर्ण *base;
	वर्ण *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	पूर्णाङ्क serrno;

	सचे (!init)
		पटिनिवत्त false;

	base = basename_c(service);
	सचे (state != RC_SERVICE_STOPPED) {
		पूर्णाङ्क state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		सचे (access(init, F_OK) != 0) {
			मुक्त(init);
			पटिनिवत्त false;
		}

		unlinkat(state_dirfd, base, 0);
		सचे ((symlinkat(init, state_dirfd, base)) != 0) {
			मुक्त(init);
			पटिनिवत्त false;
		}
	}

	सचे (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		मुक्त(init);
		पटिनिवत्त true;
	}

	/* Remove any old states now */
	अत्थाय (i = 0; rc_service_state_names[i].name; i++) {
		पूर्णाङ्क state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		सचे (s == state)
			अनुवर्त;

		विकल्प (s) {
		पक्ष RC_SERVICE_STOPPED:
		पक्ष RC_SERVICE_HOTPLUGGED:
		पक्ष RC_SERVICE_SCHEDULED:
			अनुवर्त;
		पक्ष RC_SERVICE_WASINACTIVE:
			सचे (skip_wasinactive)
				अनुवर्त;
			/* fall-through */
		स्वभाव:
			सचे (faccessat(state_dirfd, base, F_OK, 0) != 0)
				अनुवर्त;
			सचे ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				सचे (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					मुक्त(init);
					पटिनिवत्त false;
				}
				skip_wasinactive = true;
			}
			सचे (unlinkat(state_dirfd, base, 0) == -1) {
				मुक्त(init);
				पटिनिवत्त false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	सचे (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	सचे (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	सचे (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		सचे (dp) {
			संरचना dirent *d;
			याव ((d = readdir(dp))) {
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
	पटिनिवत्त true;
}

RC_SERVICE
rc_service_state(ध्रुव वर्ण *service)
{
	ध्रुव वर्ण *base = basename_c(service);
	पूर्णाङ्क state = RC_SERVICE_STOPPED;
	पूर्णाङ्क i;

	अत्थाय (i = 0; rc_service_state_names[i].name; i++) {
		सचे (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			सचे (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			अञ्ञथा
				state |= rc_service_state_names[i].state;
		}
	}

	सचे (state & RC_SERVICE_STARTED)
		सचे (rc_service_daemons_crashed(service) && errno != EACCES)
			state |= RC_SERVICE_CRASHED;

	सचे (state & RC_SERVICE_STOPPED) {
		DIR *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		सचे (dp) {
			संरचना dirent *d;
			याव ((d = readdir(dp))) {
				सचे (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					भङ्ग;
				}
			}

			closedir(dp);
		}
	}

	पटिनिवत्त state;
}

वर्ण *
rc_service_मानम्_get(ध्रुव वर्ण *service, ध्रुव वर्ण *option)
{
	वर्ण *buffer = रिक्त;
	size_t len = 0;
	वर्ण *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	मुक्त(file);

	पटिनिवत्त buffer;
}

bool
rc_service_मानम्_set(ध्रुव वर्ण *service, ध्रुव वर्ण *option, ध्रुव वर्ण *मानम्)
{
	bool ret = true;
	पूर्णाङ्क optdirfd;
	FILE *fp;

	सचे (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && errno != EEXIST)
		पटिनिवत्त false;

	सचे ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		पटिनिवत्त false;

	सचे (!मानम्) {
		unlinkat(optdirfd, option, 0);
	} अञ्ञथा सचे ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		fprintf(fp, "%s", मानम्);
		fclose(fp);
	} अञ्ञथा {
		ret = false;
	}

	close(optdirfd);
	पटिनिवत्त ret;
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
	सचे (!service || !rc_service_exists(service_to_start))
		पटिनिवत्त false;

	सचे (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && errno != EEXIST)
		पटिनिवत्त false;

	सचे ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		पटिनिवत्त false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	close(schedfd);
	मुक्त(init);
	पटिनिवत्त retval;
}

bool
rc_service_schedule_clear(ध्रुव वर्ण *service)
{
	पटिनिवत्त !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && errno == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(ध्रुव वर्ण *runlevel)
{
	RC_STRINGLIST *list = रिक्त;

	सचे (!runlevel) {
		ध्रुव पूर्णाङ्क *scriptdirfds;
		list = rc_stringlist_new();
		अत्थाय (size_t i = 0, गणनम् = rc_scriptdirfds(&scriptdirfds); i < गणनम्; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		पटिनिवत्त list;
	}

	/* These special levels never contain any services */
	सचे (strcmp(runlevel, RC_LEVEL_SINGLE) == 0)
		पटिनिवत्त rc_stringlist_new();

	पटिनिवत्त ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(ध्रुव वर्ण *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *राशिः;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(राशिः, stacks, entries) {
		sl = rc_services_in_runlevel(राशिः->मानम्);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	पटिनिवत्त list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	संरचना dirent *d;
	DIR *dp;

	सचे (state != RC_SERVICE_SCHEDULED)
		पटिनिवत्त ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	सचे (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		पटिनिवत्त list;

	याव ((d = readdir(dp))) {
		सचे (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			अनुवर्त;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	closedir(dp);

	पटिनिवत्त list;
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

	सचे (!rc_runlevel_exists(runlevel)) {
		errno = ENOENT;
		पटिनिवत्त false;
	}

	सचे (rc_service_in_runlevel(service, runlevel)) {
		errno = EEXIST;
		पटिनिवत्त false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	सचे (!rc_is_user() && strcmp(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(dirname(init), रिक्त);
		सचे (path == रिक्त) {
			मुक्त(init);
			पटिनिवत्त false;
		}
		retval = (strcmp(path, RC_INITDIR) == 0);
		मुक्त(path);
		सचे (!retval) {
			मुक्त(init);
			errno = EPERM;
			पटिनिवत्त false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	सचे (strncmp(i, rc_svcdir(), strlen(rc_svcdir())) == 0) {
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
	पटिनिवत्त retval;
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
	पटिनिवत्त (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(ध्रुव वर्ण *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	संरचना dirent *d;
	DIR *dp;

	सचे (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		पटिनिवत्त list;

	याव ((d = readdir(dp))) {
		वर्ण *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		सचे (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		मुक्त(file);
	}

	पटिनिवत्त list;
}

RC_STRINGLIST *
rc_services_scheduled(ध्रुव वर्ण *service)
{
	पटिनिवत्त ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(ध्रुव वर्ण *service, ध्रुव वर्ण *env)
{
	size_t name_len = strcspn(env, "=");
	वर्ण var[strlen(service) + परिमाण("/") + name_len];
	पूर्णाङ्क dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	सचे (env[name_len] != '=')
		पटिनिवत्त errno = EINVAL, false;

	snprintf(var, परिमाण(var), "%s/%.*s", service, (पूर्णाङ्क) name_len, env);

	सचे (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		पटिनिवत्त false;

	सचे (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		पटिनिवत्त false;

	fputs(&env[name_len + 1], fp);

	पटिनिवत्त fclose(fp) == 0;
}

bool
rc_service_setenv(ध्रुव वर्ण *service, ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्)
{
	वर्ण var[strlen(service) + परिमाण("/") + strlen(name)];
	पूर्णाङ्क dfp = rc_dirfd(RC_DIR_ENVIRON);
	FILE *fp;

	snprintf(var, परिमाण(var), "%s/%s", service, name);

	सचे (mkdirat(dfp, service, 0700) == -1 && errno != EEXIST)
		पटिनिवत्त false;

	सचे (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		पटिनिवत्त false;

	fputs(मानम्, fp);

	पटिनिवत्त fclose(fp) == 0;
}

ssize_t
rc_service_getenv(ध्रुव वर्ण *service, ध्रुव वर्ण *name, वर्ण **buf)
{
	वर्ण var[strlen(service) + परिमाण("/") + strlen(name)], *मानम्;
	पूर्णाङ्क dfd = rc_dirfd(RC_DIR_ENVIRON);
	संरचना stat stat;
	size_t गणनम्;
	FILE *fp;

	snprintf(var, परिमाण(var), "%s/%s", service, name);

	सचे (fstatat(dfd, var, &stat, 0) == -1)
		पटिनिवत्त -1;

	सचे (!buf)
		पटिनिवत्त stat.st_size;

	सचे (!(मानम् = xrealloc(*buf, stat.st_size + 1)))
		पटिनिवत्त -1;

	सचे (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		पटिनिवत्त -1;

	गणनम् = fread(मानम्, 1, stat.st_size, fp);
	मानम्[गणनम्] = '\0';

	fclose(fp);
	*buf = मानम्;
	पटिनिवत्त गणनम्;
}
