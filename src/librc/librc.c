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
खाता *rc_environ_fd = रिक्त;
स्थिर पूर्णाङ्क dirfds[RC_DIR_MAX];

पूर्णाङ्क rc_dirfd(गणना rc_dir dir) {
	पूर्णाङ्क flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	पूर्णाङ्क dirfd;

	यदि (dir < 0 || dir >= RC_DIR_MAX)
		फर्काऊ -1;
	यदि (dirfds[dir] <= 0) {
		विकल्प (dir) {
		पक्ष RC_DIR_SYSCONF:
			फर्काऊ dirfds[dir] = खोलो(rc_sysconfdir(), flags);
		पक्ष RC_DIR_USRCONF:
			फर्काऊ dirfds[dir] = खोलो(rc_usrconfdir(), flags);
		पक्ष RC_DIR_RUNLEVEL:
			फर्काऊ dirfds[dir] = खोलो(rc_runleveldir(), flags);
		पक्ष RC_DIR_SVCDIR:
			यदि (सूची_गढ़ो(rc_svcdir(), 0755) == -1 && त्रुटि_सं != EEXIST)
				फर्काऊ -1;
			फर्काऊ dirfds[dir] = खोलो(rc_svcdir(), flags);
		स्वभाव:
			प्रतिज्ञा(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			यदि ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && त्रुटि_सं == ENOENT) {
				यदि (mkdirat(dirfd, dirnames[dir], 0755) == -1 && त्रुटि_सं != EEXIST)
					फर्काऊ -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			रोक;
		}
	}
	फर्काऊ dirfds[dir];
}

शून्य clear_dirfds(शून्य) {
	लागि (माप_प्रकार i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		यदि (dirfds[i] <= 0)
			जारी;

		बंद(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
स्थिर RC_STRINGLIST *
ls_dir(पूर्णाङ्क targetfd, अचर वर्ण *dir, पूर्णाङ्क options)
{
	सूची *dp;
	संरचना dirent *d;
	RC_STRINGLIST *list = रिक्त;
	संरचना stat buf;
	माप_प्रकार l;

	list = rc_stringlist_new();

	यदि (!(dp = do_opendirat(targetfd, dir)))
		फर्काऊ list;

	जबसम्म (((d = सूची_पढ़ो(dp)) != रिक्त)) {
		यदि (d->d_name[0] == '.')
			जारी;
		यदि (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			यदि (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				जारी;

			/* .sh files are not init scripts */
			l = म_माप(d->d_name);
			यदि (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				जारी;
		}
		यदि (options & LS_DIR) {
			यदि (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				जारी;
		}
		rc_stringlist_add(list, d->d_name);
	}
	बंद_सूची(dp);
	फर्काऊ list;
}

स्थिर bool
rm_dir(पूर्णाङ्क targetfd, अचर वर्ण *pathname, bool शीर्ष)
{
	bool retval = true;
	संरचना dirent *d;
	संरचना stat s;
	सूची *dp;

	यदि (!(dp = do_opendirat(targetfd, pathname)))
		फर्काऊ false;

	जबसम्म ((d = सूची_पढ़ो(dp))) {
		यदि (म_भेद(d->d_name, ".") == 0 || म_भेद(d->d_name, "..") == 0)
			जारी;
		यदि (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			रोक;
		}
		यदि (S_ISDIR(s.st_mode)) {
			यदि (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				रोक;
			}
		} अन्यथा {
			यदि (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				रोक;
			}
		}
	}
	बंद_सूची(dp);

	यदि (!retval)
		फर्काऊ false;

	यदि (शीर्ष && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		फर्काऊ false;

	फर्काऊ true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
स्थिर bool
file_regex(अचर वर्ण *file, अचर वर्ण *regex)
{
	खाता *fp;
	वर्ण *पङ्क्ति = रिक्त;
	माप_प्रकार size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	पूर्णाङ्क result;

	यदि (!(fp = ख_खोलो(file, "r")))
		फर्काऊ false;

	यदि ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		वर्ण buf[बफ_मान];
		ख_बंद(fp);
		regerror(result, &re, buf, परिमाण(buf));
		ख_लिखो(मानक_त्रुटि, "file_regex: %s", buf);
		फर्काऊ false;
	}

	जबसम्म ((len = xgetline(&पङ्क्ति, &size, fp)) != -1) {
		वर्ण *str = पङ्क्ति;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		गर {
			यदि (regexec(&re, str, 0, रिक्त, 0) == 0)
				गच्छ found;
			str += म_माप(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			जबसम्म (str < पङ्क्ति + len && *str == '\0')
				str++;
		} जबसम्म (str < पङ्क्ति + len);
	}
	retval = false;
found:
	ख_बंद(fp);
	मुक्त(पङ्क्ति);
	regfree(&re);

	फर्काऊ retval;
}
#endif


स्थिर अचर वर्ण *
get_systype(शून्य)
{
	वर्ण *systype = rc_conf_मान("rc_sys");
	यदि (systype) {
		वर्ण *s = systype;
		/* Convert to uppercase */
		जबसम्म (s && *s) {
			यदि (है_छोटा((अचिन्हित वर्ण) *s))
				*s = बड़े((अचिन्हित वर्ण) *s);
			s++;
		}
	}
	फर्काऊ systype;
}

स्थिर अचर वर्ण *
detect_container(अचर वर्ण *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	यदि (systype) {
		यदि (म_भेद(systype, RC_SYS_NONE) == 0)
		       फर्काऊ रिक्त;
		यदि (म_भेद(systype, RC_SYS_JAIL) == 0)
			फर्काऊ RC_SYS_JAIL;
		यदि (म_भेद(systype, RC_SYS_PODMAN) == 0)
			फर्काऊ RC_SYS_PODMAN;
	}

	पूर्णाङ्क jailed = 0;
	माप_प्रकार len = परिमाण(jailed);

	यदि (sysctlbyname("security.jail.jailed", &jailed, &len, रिक्त, 0) == 0)
		यदि (jailed == 1)
			फर्काऊ RC_SYS_JAIL;

	यदि (access("/var/run/.containerenv", F_OK) == 0)
		फर्काऊ RC_SYS_PODMAN;
#endif

#ifdef __linux__
	यदि (systype) {
		यदि (म_भेद(systype, RC_SYS_NONE) == 0)
			फर्काऊ रिक्त;
		यदि (म_भेद(systype, RC_SYS_UML) == 0)
			फर्काऊ RC_SYS_UML;
		यदि (म_भेद(systype, RC_SYS_VSERVER) == 0)
			फर्काऊ RC_SYS_VSERVER;
		यदि (म_भेद(systype, RC_SYS_OPENVZ) == 0)
			फर्काऊ RC_SYS_OPENVZ;
		यदि (म_भेद(systype, RC_SYS_LXC) == 0)
			फर्काऊ RC_SYS_LXC;
		यदि (म_भेद(systype, RC_SYS_RKT) == 0)
				फर्काऊ RC_SYS_RKT;
		यदि (म_भेद(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				फर्काऊ RC_SYS_SYSTEMD_NSPAWN;
		यदि (म_भेद(systype, RC_SYS_DOCKER) == 0)
				फर्काऊ RC_SYS_DOCKER;
		यदि (म_भेद(systype, RC_SYS_PODMAN) == 0)
			फर्काऊ RC_SYS_PODMAN;
		यदि (म_भेद(systype, RC_SYS_WSL) == 0)
			फर्काऊ RC_SYS_WSL;
	}
	यदि (file_regex("/proc/cpuinfo", "UML"))
		फर्काऊ RC_SYS_UML;
	अन्यथा यदि (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		फर्काऊ RC_SYS_VSERVER;
	अन्यथा यदि (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		फर्काऊ RC_SYS_OPENVZ;
	अन्यथा यदि (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		फर्काऊ RC_SYS_OPENVZ; /* old test */
	अन्यथा यदि (file_regex("/proc/1/environ", "container=lxc"))
		फर्काऊ RC_SYS_LXC;
	अन्यथा यदि (file_regex("/proc/1/environ", "container=rkt"))
		फर्काऊ RC_SYS_RKT;
	अन्यथा यदि (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		फर्काऊ RC_SYS_SYSTEMD_NSPAWN;
	अन्यथा यदि (access("/run/.containerenv", F_OK) == 0)
		फर्काऊ RC_SYS_PODMAN;
	अन्यथा यदि (access("/.dockerenv", F_OK) == 0)
		फर्काऊ RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	अन्यथा यदि (file_regex("/proc/1/environ", "container=docker"))
		फर्काऊ RC_SYS_DOCKER;
	अन्यथा यदि (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		फर्काऊ RC_SYS_WSL;
	अन्यथा यदि (दो_पर्या("WSL_DISTRO_NAME"))
		फर्काऊ RC_SYS_WSL;
	अन्यथा यदि (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		फर्काऊ RC_SYS_WSL;
#endif

	फर्काऊ रिक्त;
}

स्थिर अचर वर्ण *
detect_vm(अचर वर्ण *systype RC_UNUSED)
{
#ifdef __NetBSD__
	यदि (systype) {
		यदि (म_भेद(systype, RC_SYS_NONE) == 0)
			फर्काऊ रिक्त;
		यदि (म_भेद(systype, RC_SYS_XEN0) == 0)
			फर्काऊ RC_SYS_XEN0;
		यदि (म_भेद(systype, RC_SYS_XENU) == 0)
			फर्काऊ RC_SYS_XENU;
	}
	यदि (access("/kern/xen/privcmd", F_OK) == 0)
		फर्काऊ RC_SYS_XEN0;
	यदि (access("/kern/xen", F_OK) == 0)
		फर्काऊ RC_SYS_XENU;
#endif

#ifdef __linux__
	यदि (systype) {
		यदि (म_भेद(systype, RC_SYS_NONE) == 0)
			फर्काऊ रिक्त;
		यदि (म_भेद(systype, RC_SYS_XEN0) == 0)
			फर्काऊ RC_SYS_XEN0;
		यदि (म_भेद(systype, RC_SYS_XENU) == 0)
			फर्काऊ RC_SYS_XENU;
	}
	यदि (access("/proc/xen", F_OK) == 0) {
		यदि (file_regex("/proc/xen/capabilities", "control_d"))
			फर्काऊ RC_SYS_XEN0;
		फर्काऊ RC_SYS_XENU;
	}
#endif

	फर्काऊ रिक्त;
}

अचर वर्ण *
rc_sys(शून्य)
{
	अचर वर्ण *systype;
	अचर वर्ण *sys;

	systype = get_systype();
	sys = detect_container(systype);
	यदि (!sys) {
		sys = detect_vm(systype);
	}

	फर्काऊ sys;
}

स्थिर अचर वर्ण *
rc_parse_service_state(RC_SERVICE state)
{
	पूर्णाङ्क i;

	लागि (i = 0; rc_service_state_names[i].name; i++) {
		यदि (rc_service_state_names[i].state == state)
			फर्काऊ rc_service_state_names[i].name;
	}
	फर्काऊ रिक्त;
}

स्थिर पूर्णाङ्क
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	लागि (माप_प्रकार i = 0; rc_service_state_names[i].name; i++)
		यदि (rc_service_state_names[i].state == state)
			फर्काऊ rc_service_state_names[i].dir;
	फर्काऊ -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
स्थिर शून्य
get_runlevel_chain(अचर वर्ण *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	अचर वर्ण *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	यदि (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		फर्काऊ;

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
		nextlevel = d->मान;

		/* Check for loop */
		यदि (rc_stringlist_find(ancestor_list, nextlevel)) {
			ख_लिखो(मानक_त्रुटि, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			ख_लिखो(मानक_त्रुटि, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				ख_लिखो(मानक_त्रुटि, "\t%s\n", parent->मान);
			निकास(1);
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
	फर्काऊ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(शून्य)
{
	फर्काऊ faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(शून्य)
{
	फर्काऊ ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

वर्ण *
rc_runlevel_get(शून्य)
{
	खाता *fp;
	वर्ण *runlevel = रिक्त;
	माप_प्रकार i;

	यदि ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(परिमाण(वर्ण) * PATH_MAX);
		यदि (ख_माला_लो(runlevel, PATH_MAX, fp)) {
			i = म_माप(runlevel);
			यदि (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} अन्यथा
			*runlevel = '\0';
		ख_बंद(fp);
	}

	यदि (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = रिक्त;
	}

	फर्काऊ runlevel;
}

bool
rc_runlevel_set(अचर वर्ण *runlevel)
{
	खाता *fp;

	यदि (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		फर्काऊ false;

	ख_माला_दो(runlevel, fp);
	ख_बंद(fp);
	फर्काऊ true;
}

bool
rc_runlevel_exists(अचर वर्ण *runlevel)
{
	संरचना stat buf;

	यदि (!runlevel || म_भेद(runlevel, "") == 0 || म_भेद(runlevel, ".") == 0 || म_भेद(runlevel, "..") == 0)
		फर्काऊ false;
	यदि (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		फर्काऊ true;
	फर्काऊ false;
}

bool
rc_runlevel_थाक(अचर वर्ण *dst, अचर वर्ण *src)
{
	वर्ण *d, *s;
	पूर्णाङ्क r;

	यदि (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		फर्काऊ false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	मुक्त(d);
	मुक्त(s);
	फर्काऊ (r == 0);
}

bool
rc_runlevel_unstack(अचर वर्ण *dst, अचर वर्ण *src)
{
	वर्ण *path;
	पूर्णाङ्क r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	मुक्त(path);
	फर्काऊ (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(अचर वर्ण *runlevel)
{
	RC_STRINGLIST *थाक, *ancestor_list;
	थाक = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, थाक, ancestor_list);
	rc_stringlist_free(ancestor_list);
	फर्काऊ थाक;
}

स्थिर अचर वर्ण *config_path[] = { _RC_PATH, रिक्त };

स्थिर संरचना {
	bool set;
	वर्ण *svcdir, *confdir, *runleveldir;
} rc_dirs;

स्थिर संरचना {
	वर्ण *buffer;
	माप_प्रकार गन्ती;
	पूर्णाङ्क *fds;
	अचर वर्ण **entries, **allocd;
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
	rc_path.गन्ती = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
स्थिर शून्य
append_path(खाता *fp, अचर वर्ण *fmt, ...)
{
	बहु_सूची va;

	बहु_शुरू(va, fmt);
	भख_लिखो(fp, fmt, va);
	ख_अक्षर_दो('\0', fp);
	बहु_पूर्ण(va);

	rc_path.गन्ती++;
}

स्थिर अचर वर्ण *
strnsep(अचर वर्ण **str, माप_प्रकार *size, अचर वर्ण sep)
{
	अचर वर्ण *ret = *str;

	यदि (!ret)
		फर्काऊ रिक्त;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	यदि (!**str)
		*str = रिक्त;
	अन्यथा
		*str += 1;

	फर्काऊ ret;
}

स्थिर अचर वर्ण *
strnullsep(अचर वर्ण **str, माप_प्रकार *size)
{
	अचर वर्ण *ret = *str;
	माप_प्रकार len;

	यदि (!ret)
		फर्काऊ रिक्त;

	len = strnlen(ret, *size - 1) + 1;

	यदि ((*size -= len))
		*str += len;
	अन्यथा
		*str = रिक्त;

	फर्काऊ ret;
}

स्थिर शून्य
append_user_path(खाता *fp)
{
	अचर वर्ण *xdg, *entry;
	माप_प्रकार size;

	append_path(fp, "%s", rc_dirs.confdir);
	यदि (!(xdg = दो_पर्या("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	अन्यथा जबसम्म ((entry = strnsep(&xdg, &size, ':')))
		यदि (size) append_path(fp, "%.*s/rc", (पूर्णाङ्क) size, entry);

	लागि (माप_प्रकार i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

स्थिर शून्य
append_rc_path(खाता *fp, अचर वर्ण *path, bool user)
{
	bool has_user = false;
	अचर वर्ण *entry;
	माप_प्रकार size;

	जबसम्म ((entry = strnsep(&path, &size, ':'))) {
		यदि (size)
			append_path(fp, "%.*s%s", (पूर्णाङ्क)size, entry, user ? "/user" : "");
		अन्यथा यदि (user && !has_user)
			append_user_path(fp), has_user = true;
		अन्यथा यदि (!user) जबसम्म (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

स्थिर शून्य
load_dynamic_path(bool user)
{
	अचर वर्ण *path, *entry;
	माप_प्रकार size, गन्ती = 0;
	खाता *fp;

	यदि (!rc_dirs.set) {
		निकास_पर(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.गन्ती = ARRAY_SIZE(config_path) - 1;

	यदि (!(path = दो_पर्या("RC_PATH")) && !user)
		फर्काऊ;

	rc_path.गन्ती = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	यदि (path)
		append_rc_path(fp, path, user);
	अन्यथा
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = सुस्मृति(rc_path.गन्ती + 1, परिमाण(*rc_path.entries));

	path = rc_path.buffer;

	जबसम्म ((entry = strnullsep(&path, &size)) && गन्ती < rc_path.गन्ती)
		rc_path.entries[गन्ती++] = entry;
	rc_path.entries[rc_path.गन्ती] = रिक्त;
}

स्थिर bool is_user = false;

bool
rc_is_user(शून्य)
{
	पूर्णाङ्क _is_user = -1;

	यदि (_is_user == -1) {
		वर्ण *env = दो_पर्या("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	फर्काऊ _is_user;
}

शून्य
rc_set_user(शून्य)
{
	वर्ण *env;

	यदि (is_user)
		फर्काऊ;

	यदि (!rc_dirs.set)
		निकास_पर(free_rc_dirs);
	अन्यथा
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	यदि ((env = दो_पर्या("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} अन्यथा यदि ((env = दो_पर्या("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} अन्यथा {
		ख_लिखो(मानक_त्रुटि, "XDG_CONFIG_HOME and HOME unset.\n");
		निकास(निकास_त्रुटि);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	यदि (!(env = दो_पर्या("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		ख_लिखो(मानक_त्रुटि, "XDG_RUNTIME_DIR unset.\n");
		निकास(निकास_त्रुटि);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

अचर वर्ण * अचर *
rc_scriptdirs(शून्य)
{
	यदि (!rc_path.entries)
		load_dynamic_path(false);
	फर्काऊ rc_path.entries;
}

माप_प्रकार
rc_scriptdirfds(अचर पूर्णाङ्क **fds)
{
	स्थिर माप_प्रकार len = 0;

	यदि (len == 0) {
		यदि (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.गन्ती * परिमाण(*rc_path.fds));
		लागि (माप_प्रकार i = 0; i < rc_path.गन्ती; i++) {
			पूर्णाङ्क fd = खोलो(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			यदि (fd == -1)
				जारी;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	फर्काऊ len;
}

अचर वर्ण *
rc_sysconfdir(शून्य)
{
	फर्काऊ RC_SYSCONFDIR;
}

अचर वर्ण *
rc_usrconfdir(शून्य)
{
	फर्काऊ rc_dirs.confdir ? rc_dirs.confdir : रिक्त;
}

अचर वर्ण *
rc_runleveldir(शून्य)
{
	फर्काऊ rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

अचर वर्ण *
rc_svcdir(शून्य)
{
	स्थिर अचर वर्ण *svcdir;

	यदि (!rc_dirs.svcdir && !svcdir && !(svcdir = दो_पर्या("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	फर्काऊ rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

स्थिर ssize_t
safe_readlink(पूर्णाङ्क targetfd, अचर वर्ण *path, वर्ण **buf, माप_प्रकार bufsiz)
{
	वर्ण *buffer;
	ssize_t r;

	लागि (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		यदि (r < 0) {
			मुक्त(buffer);
			फर्काऊ -त्रुटि_सं;
		}
		यदि ((माप_प्रकार)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			फर्काऊ r;
		}
		मुक्त(buffer);
		यदि (bufsiz >= SSIZE_MAX - PATH_MAX)
			फर्काऊ -ENOMEM;
	}
}

/* Resolve a service name to its full path */
वर्ण *
rc_service_resolve(अचर वर्ण *service)
{
	वर्ण *buffer;
	संरचना stat buf;

	यदि (!service)
		फर्काऊ रिक्त;

	यदि (service[0] == '/')
		फर्काऊ xstrdup(service);

	/* First check started services */
	यदि (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		यदि (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			फर्काऊ buffer;
	यदि (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		यदि (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			फर्काऊ buffer;

	लागि (अचर वर्ण * अचर *dirs = rc_scriptdirs(); *dirs; dirs++) {
		वर्ण *file = रिक्त;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		यदि (stat(file, &buf) == 0)
			फर्काऊ file;
		मुक्त(file);
	}

	फर्काऊ रिक्त;
}

bool
rc_service_exists(अचर वर्ण *service)
{
	वर्ण *file;
	bool retval = false;
	माप_प्रकार len;
	संरचना stat buf;

	यदि (!service) {
		त्रुटि_सं = EINVAL;
		फर्काऊ false;
	}

	len = म_माप(service);

	/* .sh files are not init scripts */
	यदि (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		त्रुटि_सं = EINVAL;
		फर्काऊ false;
	}

	यदि (!(file = rc_service_resolve(service))) {
		त्रुटि_सं = ENOENT;
		फर्काऊ false;
	}

	यदि (stat(file, &buf) == 0) {
		यदि (buf.st_mode & S_IXUGO)
			retval = true;
		अन्यथा
			त्रुटि_सं = ENOEXEC;
	}
	मुक्त(file);
	फर्काऊ retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(अचर वर्ण *service)
{
	वर्ण *svc;
	वर्ण *cmd = रिक्त;
	वर्ण *buffer = रिक्त;
	माप_प्रकार len = 0;
	RC_STRINGLIST *commands = रिक्त;
	वर्ण *token;
	वर्ण *p;
	खाता *fp;

	यदि (!(svc = rc_service_resolve(service)))
		फर्काऊ रिक्त;

	xasprintf(&cmd, OPTSTR, svc);
	मुक्त(svc);

	यदि (!(fp = popen(cmd, "r"))) {
		मुक्त(cmd);
		फर्काऊ रिक्त;
	}

	यदि (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		जबसम्म ((token = strsep(&p, " ")))
			यदि (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	मुक्त(buffer);
	मुक्त(cmd);
	फर्काऊ commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
वर्ण *
rc_service_description(अचर वर्ण *service, अचर वर्ण *option)
{
	वर्ण *svc;
	वर्ण *cmd;
	वर्ण *desc = रिक्त;
	माप_प्रकार size = 0;
	खाता *fp;

	यदि (!(svc = rc_service_resolve(service)))
		फर्काऊ रिक्त;

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
	फर्काऊ desc;
}

bool
rc_service_in_runlevel(अचर वर्ण *service, अचर वर्ण *runlevel)
{
	वर्ण *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	मुक्त(file);
	फर्काऊ r;
}

bool
rc_service_mark(अचर वर्ण *service, अचर RC_SERVICE state)
{
	पूर्णाङ्क i = 0;
	अचर वर्ण *base;
	वर्ण *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	पूर्णाङ्क serrno;

	यदि (!init)
		फर्काऊ false;

	base = basename_c(service);
	यदि (state != RC_SERVICE_STOPPED) {
		पूर्णाङ्क state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		यदि (access(init, F_OK) != 0) {
			मुक्त(init);
			फर्काऊ false;
		}

		unlinkat(state_dirfd, base, 0);
		यदि ((symlinkat(init, state_dirfd, base)) != 0) {
			मुक्त(init);
			फर्काऊ false;
		}
	}

	यदि (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		मुक्त(init);
		फर्काऊ true;
	}

	/* Remove any old states now */
	लागि (i = 0; rc_service_state_names[i].name; i++) {
		पूर्णाङ्क state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		यदि (s == state)
			जारी;

		विकल्प (s) {
		पक्ष RC_SERVICE_STOPPED:
		पक्ष RC_SERVICE_HOTPLUGGED:
		पक्ष RC_SERVICE_SCHEDULED:
			जारी;
		पक्ष RC_SERVICE_WASINACTIVE:
			यदि (skip_wasinactive)
				जारी;
			/* fall-through */
		स्वभाव:
			यदि (faccessat(state_dirfd, base, F_OK, 0) != 0)
				जारी;
			यदि ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				यदि (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					मुक्त(init);
					फर्काऊ false;
				}
				skip_wasinactive = true;
			}
			यदि (unlinkat(state_dirfd, base, 0) == -1) {
				मुक्त(init);
				फर्काऊ false;
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
		सूची *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		यदि (dp) {
			संरचना dirent *d;
			जबसम्म ((d = सूची_पढ़ो(dp))) {
				वर्ण *was;

				xasprintf(&was, "%s/%s", d->d_name, base);
				unlinkat(dirfd(dp), was, 0);
				मुक्त(was);

				serrno = त्रुटि_सं;
				unlinkat(dirfd(dp), d->d_name, AT_REMOVEDIR);
				त्रुटि_सं = serrno;
			}

			बंद_सूची(dp);
		}
	}
	मुक्त(init);
	फर्काऊ true;
}

RC_SERVICE
rc_service_state(अचर वर्ण *service)
{
	अचर वर्ण *base = basename_c(service);
	पूर्णाङ्क state = RC_SERVICE_STOPPED;
	पूर्णाङ्क i;

	लागि (i = 0; rc_service_state_names[i].name; i++) {
		यदि (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			यदि (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			अन्यथा
				state |= rc_service_state_names[i].state;
		}
	}

	यदि (state & RC_SERVICE_STARTED)
		यदि (rc_service_daemons_crashed(service) && त्रुटि_सं != EACCES)
			state |= RC_SERVICE_CRASHED;

	यदि (state & RC_SERVICE_STOPPED) {
		सूची *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		यदि (dp) {
			संरचना dirent *d;
			जबसम्म ((d = सूची_पढ़ो(dp))) {
				यदि (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					रोक;
				}
			}

			बंद_सूची(dp);
		}
	}

	फर्काऊ state;
}

वर्ण *
rc_service_मान_get(अचर वर्ण *service, अचर वर्ण *option)
{
	वर्ण *buffer = रिक्त;
	माप_प्रकार len = 0;
	वर्ण *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	मुक्त(file);

	फर्काऊ buffer;
}

bool
rc_service_मान_set(अचर वर्ण *service, अचर वर्ण *option, अचर वर्ण *मान)
{
	bool ret = true;
	पूर्णाङ्क optdirfd;
	खाता *fp;

	यदि (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && त्रुटि_सं != EEXIST)
		फर्काऊ false;

	यदि ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		फर्काऊ false;

	यदि (!मान) {
		unlinkat(optdirfd, option, 0);
	} अन्यथा यदि ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		ख_लिखो(fp, "%s", मान);
		ख_बंद(fp);
	} अन्यथा {
		ret = false;
	}

	बंद(optdirfd);
	फर्काऊ ret;
}

bool
rc_service_schedule_start(अचर वर्ण *service, अचर वर्ण *service_to_start)
{
	अचर वर्ण *base_to_start = basename_c(service_to_start);
	अचर वर्ण *base = basename_c(service);
	bool retval;
	पूर्णाङ्क schedfd;
	वर्ण *init;

	/* service may be a provided service, like net */
	यदि (!service || !rc_service_exists(service_to_start))
		फर्काऊ false;

	यदि (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && त्रुटि_सं != EEXIST)
		फर्काऊ false;

	यदि ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		फर्काऊ false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	बंद(schedfd);
	मुक्त(init);
	फर्काऊ retval;
}

bool
rc_service_schedule_clear(अचर वर्ण *service)
{
	फर्काऊ !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && त्रुटि_सं == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(अचर वर्ण *runlevel)
{
	RC_STRINGLIST *list = रिक्त;

	यदि (!runlevel) {
		अचर पूर्णाङ्क *scriptdirfds;
		list = rc_stringlist_new();
		लागि (माप_प्रकार i = 0, गन्ती = rc_scriptdirfds(&scriptdirfds); i < गन्ती; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		फर्काऊ list;
	}

	/* These special levels never contain any services */
	यदि (म_भेद(runlevel, RC_LEVEL_SINGLE) == 0)
		फर्काऊ rc_stringlist_new();

	फर्काऊ ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(अचर वर्ण *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *थाक;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(थाक, stacks, entries) {
		sl = rc_services_in_runlevel(थाक->मान);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	फर्काऊ list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	संरचना dirent *d;
	सूची *dp;

	यदि (state != RC_SERVICE_SCHEDULED)
		फर्काऊ ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	यदि (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		फर्काऊ list;

	जबसम्म ((d = सूची_पढ़ो(dp))) {
		यदि (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			जारी;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	बंद_सूची(dp);

	फर्काऊ list;
}

bool
rc_service_add(अचर वर्ण *runlevel, अचर वर्ण *service)
{
	bool retval;
	वर्ण *init;
	वर्ण *file;
	वर्ण *path;
	वर्ण *binit = रिक्त;
	वर्ण *i;

	यदि (!rc_runlevel_exists(runlevel)) {
		त्रुटि_सं = ENOENT;
		फर्काऊ false;
	}

	यदि (rc_service_in_runlevel(service, runlevel)) {
		त्रुटि_सं = EEXIST;
		फर्काऊ false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	यदि (!rc_is_user() && म_भेद(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(स_नाम(init), रिक्त);
		यदि (path == रिक्त) {
			मुक्त(init);
			फर्काऊ false;
		}
		retval = (म_भेद(path, RC_INITDIR) == 0);
		मुक्त(path);
		यदि (!retval) {
			मुक्त(init);
			त्रुटि_सं = EPERM;
			फर्काऊ false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	यदि (म_भेदन(i, rc_svcdir(), म_माप(rc_svcdir())) == 0) {
		त्रुटि_सं = ENOENT;
		retval = false;
		गच्छ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	मुक्त(file);

out:
	मुक्त(binit);
	मुक्त(init);
	फर्काऊ retval;
}

bool
rc_service_delete(अचर वर्ण *runlevel, अचर वर्ण *service)
{
	वर्ण *file;
	पूर्णाङ्क r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	मुक्त(file);
	फर्काऊ (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(अचर वर्ण *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	संरचना dirent *d;
	सूची *dp;

	यदि (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		फर्काऊ list;

	जबसम्म ((d = सूची_पढ़ो(dp))) {
		वर्ण *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		यदि (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		मुक्त(file);
	}

	फर्काऊ list;
}

RC_STRINGLIST *
rc_services_scheduled(अचर वर्ण *service)
{
	फर्काऊ ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(अचर वर्ण *service, अचर वर्ण *env)
{
	माप_प्रकार name_len = म_खोज(env, "=");
	वर्ण var[म_माप(service) + परिमाण("/") + name_len];
	पूर्णाङ्क dfp = rc_dirfd(RC_DIR_ENVIRON);
	खाता *fp;

	यदि (env[name_len] != '=')
		फर्काऊ त्रुटि_सं = EINVAL, false;

	snprintf(var, परिमाण(var), "%s/%.*s", service, (पूर्णाङ्क) name_len, env);

	यदि (mkdirat(dfp, service, 0700) == -1 && त्रुटि_सं != EEXIST)
		फर्काऊ false;

	यदि (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		फर्काऊ false;

	ख_माला_दो(&env[name_len + 1], fp);

	फर्काऊ ख_बंद(fp) == 0;
}

bool
rc_service_setenv(अचर वर्ण *service, अचर वर्ण *name, अचर वर्ण *मान)
{
	वर्ण var[म_माप(service) + परिमाण("/") + म_माप(name)];
	पूर्णाङ्क dfp = rc_dirfd(RC_DIR_ENVIRON);
	खाता *fp;

	snprintf(var, परिमाण(var), "%s/%s", service, name);

	यदि (mkdirat(dfp, service, 0700) == -1 && त्रुटि_सं != EEXIST)
		फर्काऊ false;

	यदि (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		फर्काऊ false;

	ख_माला_दो(मान, fp);

	फर्काऊ ख_बंद(fp) == 0;
}

ssize_t
rc_service_getenv(अचर वर्ण *service, अचर वर्ण *name, वर्ण **buf)
{
	वर्ण var[म_माप(service) + परिमाण("/") + म_माप(name)], *मान;
	पूर्णाङ्क dfd = rc_dirfd(RC_DIR_ENVIRON);
	संरचना stat stat;
	माप_प्रकार गन्ती;
	खाता *fp;

	snprintf(var, परिमाण(var), "%s/%s", service, name);

	यदि (fstatat(dfd, var, &stat, 0) == -1)
		फर्काऊ -1;

	यदि (!buf)
		फर्काऊ stat.st_size;

	यदि (!(मान = xrealloc(*buf, stat.st_size + 1)))
		फर्काऊ -1;

	यदि (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		फर्काऊ -1;

	गन्ती = ख_पढ़ो(मान, 1, stat.st_size, fp);
	मान[गन्ती] = '\0';

	ख_बंद(fp);
	*buf = मान;
	फर्काऊ गन्ती;
}
