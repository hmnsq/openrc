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
स्थिर पूर्णांक dirfds[RC_DIR_MAX];

पूर्णांक rc_dirfd(गणना rc_dir dir) {
	पूर्णांक flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	पूर्णांक dirfd;

	जर (dir < 0 || dir >= RC_DIR_MAX)
		परत -1;
	जर (dirfds[dir] <= 0) {
		निवड (dir) {
		स्थिती RC_DIR_SYSCONF:
			परत dirfds[dir] = खोलो(rc_sysconfdir(), flags);
		स्थिती RC_DIR_USRCONF:
			परत dirfds[dir] = खोलो(rc_usrconfdir(), flags);
		स्थिती RC_DIR_RUNLEVEL:
			परत dirfds[dir] = खोलो(rc_runleveldir(), flags);
		स्थिती RC_DIR_SVCDIR:
			जर (सूची_गढ़ो(rc_svcdir(), 0755) == -1 && त्रुटि_सं != EEXIST)
				परत -1;
			परत dirfds[dir] = खोलो(rc_svcdir(), flags);
		पूर्वनिर्धारित:
			प्रतिज्ञा(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			जर ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && त्रुटि_सं == ENOENT) {
				जर (mkdirat(dirfd, dirnames[dir], 0755) == -1 && त्रुटि_सं != EEXIST)
					परत -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			थांबा;
		}
	}
	परत dirfds[dir];
}

शून्य clear_dirfds(शून्य) {
	साठी (माप_प्रकार i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		जर (dirfds[i] <= 0)
			सुरूठेवा;

		बंद(dirfds[i]);
		dirfds[i] = -1;
	}
}

#define LS_INITD	0x01
#define LS_DIR		0x02
स्थिर RC_STRINGLIST *
ls_dir(पूर्णांक targetfd, स्थिरांक अक्षर *dir, पूर्णांक options)
{
	सूची *dp;
	संरचना dirent *d;
	RC_STRINGLIST *list = रिक्त;
	संरचना stat buf;
	माप_प्रकार l;

	list = rc_stringlist_new();

	जर (!(dp = do_opendirat(targetfd, dir)))
		परत list;

	पर्यंत (((d = सूची_पढ़ो(dp)) != रिक्त)) {
		जर (d->d_name[0] == '.')
			सुरूठेवा;
		जर (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			जर (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				सुरूठेवा;

			/* .sh files are not init scripts */
			l = म_माप(d->d_name);
			जर (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				सुरूठेवा;
		}
		जर (options & LS_DIR) {
			जर (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				सुरूठेवा;
		}
		rc_stringlist_add(list, d->d_name);
	}
	बंद_सूची(dp);
	परत list;
}

स्थिर bool
rm_dir(पूर्णांक targetfd, स्थिरांक अक्षर *pathname, bool शीर्ष)
{
	bool retval = true;
	संरचना dirent *d;
	संरचना stat s;
	सूची *dp;

	जर (!(dp = do_opendirat(targetfd, pathname)))
		परत false;

	पर्यंत ((d = सूची_पढ़ो(dp))) {
		जर (म_भेद(d->d_name, ".") == 0 || म_भेद(d->d_name, "..") == 0)
			सुरूठेवा;
		जर (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			थांबा;
		}
		जर (S_ISDIR(s.st_mode)) {
			जर (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				थांबा;
			}
		} अन्यथा {
			जर (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				थांबा;
			}
		}
	}
	बंद_सूची(dp);

	जर (!retval)
		परत false;

	जर (शीर्ष && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		परत false;

	परत true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#ifdef __linux__
स्थिर bool
file_regex(स्थिरांक अक्षर *file, स्थिरांक अक्षर *regex)
{
	खाता *fp;
	अक्षर *ओळ = रिक्त;
	माप_प्रकार size = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	पूर्णांक result;

	जर (!(fp = ख_खोलो(file, "r")))
		परत false;

	जर ((result = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		अक्षर buf[बफ_मान];
		ख_बंद(fp);
		regerror(result, &re, buf, आकार(buf));
		ख_लिखो(मानक_त्रुटि, "file_regex: %s", buf);
		परत false;
	}

	पर्यंत ((len = xgetline(&ओळ, &size, fp)) != -1) {
		अक्षर *str = ओळ;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		करा {
			जर (regexec(&re, str, 0, रिक्त, 0) == 0)
				जा found;
			str += म_माप(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			पर्यंत (str < ओळ + len && *str == '\0')
				str++;
		} पर्यंत (str < ओळ + len);
	}
	retval = false;
found:
	ख_बंद(fp);
	मुक्त(ओळ);
	regfree(&re);

	परत retval;
}
#endif


स्थिर स्थिरांक अक्षर *
get_systype(शून्य)
{
	अक्षर *systype = rc_conf_मूल्य("rc_sys");
	जर (systype) {
		अक्षर *s = systype;
		/* Convert to uppercase */
		पर्यंत (s && *s) {
			जर (है_छोटा((अचिन्हित अक्षर) *s))
				*s = बड़े((अचिन्हित अक्षर) *s);
			s++;
		}
	}
	परत systype;
}

स्थिर स्थिरांक अक्षर *
detect_container(स्थिरांक अक्षर *systype RC_UNUSED)
{
#ifdef __FreeBSD__
	जर (systype) {
		जर (म_भेद(systype, RC_SYS_NONE) == 0)
		       परत रिक्त;
		जर (म_भेद(systype, RC_SYS_JAIL) == 0)
			परत RC_SYS_JAIL;
		जर (म_भेद(systype, RC_SYS_PODMAN) == 0)
			परत RC_SYS_PODMAN;
	}

	पूर्णांक jailed = 0;
	माप_प्रकार len = आकार(jailed);

	जर (sysctlbyname("security.jail.jailed", &jailed, &len, रिक्त, 0) == 0)
		जर (jailed == 1)
			परत RC_SYS_JAIL;

	जर (access("/var/run/.containerenv", F_OK) == 0)
		परत RC_SYS_PODMAN;
#endif

#ifdef __linux__
	जर (systype) {
		जर (म_भेद(systype, RC_SYS_NONE) == 0)
			परत रिक्त;
		जर (म_भेद(systype, RC_SYS_UML) == 0)
			परत RC_SYS_UML;
		जर (म_भेद(systype, RC_SYS_VSERVER) == 0)
			परत RC_SYS_VSERVER;
		जर (म_भेद(systype, RC_SYS_OPENVZ) == 0)
			परत RC_SYS_OPENVZ;
		जर (म_भेद(systype, RC_SYS_LXC) == 0)
			परत RC_SYS_LXC;
		जर (म_भेद(systype, RC_SYS_RKT) == 0)
				परत RC_SYS_RKT;
		जर (म_भेद(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				परत RC_SYS_SYSTEMD_NSPAWN;
		जर (म_भेद(systype, RC_SYS_DOCKER) == 0)
				परत RC_SYS_DOCKER;
		जर (म_भेद(systype, RC_SYS_PODMAN) == 0)
			परत RC_SYS_PODMAN;
		जर (म_भेद(systype, RC_SYS_WSL) == 0)
			परत RC_SYS_WSL;
	}
	जर (file_regex("/proc/cpuinfo", "UML"))
		परत RC_SYS_UML;
	अन्यथा जर (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		परत RC_SYS_VSERVER;
	अन्यथा जर (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		परत RC_SYS_OPENVZ;
	अन्यथा जर (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		परत RC_SYS_OPENVZ; /* old test */
	अन्यथा जर (file_regex("/proc/1/environ", "container=lxc"))
		परत RC_SYS_LXC;
	अन्यथा जर (file_regex("/proc/1/environ", "container=rkt"))
		परत RC_SYS_RKT;
	अन्यथा जर (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		परत RC_SYS_SYSTEMD_NSPAWN;
	अन्यथा जर (access("/run/.containerenv", F_OK) == 0)
		परत RC_SYS_PODMAN;
	अन्यथा जर (access("/.dockerenv", F_OK) == 0)
		परत RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	अन्यथा जर (file_regex("/proc/1/environ", "container=docker"))
		परत RC_SYS_DOCKER;
	अन्यथा जर (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		परत RC_SYS_WSL;
	अन्यथा जर (दो_पर्या("WSL_DISTRO_NAME"))
		परत RC_SYS_WSL;
	अन्यथा जर (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		परत RC_SYS_WSL;
#endif

	परत रिक्त;
}

स्थिर स्थिरांक अक्षर *
detect_vm(स्थिरांक अक्षर *systype RC_UNUSED)
{
#ifdef __NetBSD__
	जर (systype) {
		जर (म_भेद(systype, RC_SYS_NONE) == 0)
			परत रिक्त;
		जर (म_भेद(systype, RC_SYS_XEN0) == 0)
			परत RC_SYS_XEN0;
		जर (म_भेद(systype, RC_SYS_XENU) == 0)
			परत RC_SYS_XENU;
	}
	जर (access("/kern/xen/privcmd", F_OK) == 0)
		परत RC_SYS_XEN0;
	जर (access("/kern/xen", F_OK) == 0)
		परत RC_SYS_XENU;
#endif

#ifdef __linux__
	जर (systype) {
		जर (म_भेद(systype, RC_SYS_NONE) == 0)
			परत रिक्त;
		जर (म_भेद(systype, RC_SYS_XEN0) == 0)
			परत RC_SYS_XEN0;
		जर (म_भेद(systype, RC_SYS_XENU) == 0)
			परत RC_SYS_XENU;
	}
	जर (access("/proc/xen", F_OK) == 0) {
		जर (file_regex("/proc/xen/capabilities", "control_d"))
			परत RC_SYS_XEN0;
		परत RC_SYS_XENU;
	}
#endif

	परत रिक्त;
}

स्थिरांक अक्षर *
rc_sys(शून्य)
{
	स्थिरांक अक्षर *systype;
	स्थिरांक अक्षर *sys;

	systype = get_systype();
	sys = detect_container(systype);
	जर (!sys) {
		sys = detect_vm(systype);
	}

	परत sys;
}

स्थिर स्थिरांक अक्षर *
rc_parse_service_state(RC_SERVICE state)
{
	पूर्णांक i;

	साठी (i = 0; rc_service_state_names[i].name; i++) {
		जर (rc_service_state_names[i].state == state)
			परत rc_service_state_names[i].name;
	}
	परत रिक्त;
}

स्थिर पूर्णांक
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	साठी (माप_प्रकार i = 0; rc_service_state_names[i].name; i++)
		जर (rc_service_state_names[i].state == state)
			परत rc_service_state_names[i].dir;
	परत -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
स्थिर शून्य
get_runlevel_chain(स्थिरांक अक्षर *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	स्थिरांक अक्षर *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	जर (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		परत;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	जर (rc_stringlist_find(level_list, runlevel))
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
		जर (rc_stringlist_find(ancestor_list, nextlevel)) {
			ख_लिखो(मानक_त्रुटि, "Loop detected in stacked runlevels attempting to enter runlevel %s!\n",
			    nextlevel);
			ख_लिखो(मानक_त्रुटि, "Ancestors:\n");
			TAILQ_FOREACH(parent, ancestor_list, entries)
				ख_लिखो(मानक_त्रुटि, "\t%s\n", parent->मूल्य);
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
	परत faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(शून्य)
{
	परत faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(शून्य)
{
	परत ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

अक्षर *
rc_runlevel_get(शून्य)
{
	खाता *fp;
	अक्षर *runlevel = रिक्त;
	माप_प्रकार i;

	जर ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(आकार(अक्षर) * PATH_MAX);
		जर (ख_माला_लो(runlevel, PATH_MAX, fp)) {
			i = म_माप(runlevel);
			जर (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} अन्यथा
			*runlevel = '\0';
		ख_बंद(fp);
	}

	जर (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = रिक्त;
	}

	परत runlevel;
}

bool
rc_runlevel_set(स्थिरांक अक्षर *runlevel)
{
	खाता *fp;

	जर (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		परत false;

	ख_माला_दो(runlevel, fp);
	ख_बंद(fp);
	परत true;
}

bool
rc_runlevel_exists(स्थिरांक अक्षर *runlevel)
{
	संरचना stat buf;

	जर (!runlevel || म_भेद(runlevel, "") == 0 || म_भेद(runlevel, ".") == 0 || म_भेद(runlevel, "..") == 0)
		परत false;
	जर (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		परत true;
	परत false;
}

bool
rc_runlevel_चळत(स्थिरांक अक्षर *dst, स्थिरांक अक्षर *src)
{
	अक्षर *d, *s;
	पूर्णांक r;

	जर (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		परत false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	मुक्त(d);
	मुक्त(s);
	परत (r == 0);
}

bool
rc_runlevel_unstack(स्थिरांक अक्षर *dst, स्थिरांक अक्षर *src)
{
	अक्षर *path;
	पूर्णांक r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	मुक्त(path);
	परत (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(स्थिरांक अक्षर *runlevel)
{
	RC_STRINGLIST *चळत, *ancestor_list;
	चळत = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, चळत, ancestor_list);
	rc_stringlist_free(ancestor_list);
	परत चळत;
}

स्थिर स्थिरांक अक्षर *config_path[] = { _RC_PATH, रिक्त };

स्थिर संरचना {
	bool set;
	अक्षर *svcdir, *confdir, *runleveldir;
} rc_dirs;

स्थिर संरचना {
	अक्षर *buffer;
	माप_प्रकार मोजणी;
	पूर्णांक *fds;
	स्थिरांक अक्षर **entries, **allocd;
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
	rc_path.मोजणी = 0;
#undef X
}

__attribute__((__format__(__printf__, 2, 3)))
स्थिर शून्य
append_path(खाता *fp, स्थिरांक अक्षर *fmt, ...)
{
	बहु_सूची va;

	बहु_शुरू(va, fmt);
	भख_लिखो(fp, fmt, va);
	ख_अक्षर_दो('\0', fp);
	बहु_पूर्ण(va);

	rc_path.मोजणी++;
}

स्थिर स्थिरांक अक्षर *
strnsep(स्थिरांक अक्षर **str, माप_प्रकार *size, स्थिरांक अक्षर sep)
{
	स्थिरांक अक्षर *ret = *str;

	जर (!ret)
		परत रिक्त;

	*str = strchrnul(ret, sep);
	*size = *str - ret;
	जर (!**str)
		*str = रिक्त;
	अन्यथा
		*str += 1;

	परत ret;
}

स्थिर स्थिरांक अक्षर *
strnullsep(स्थिरांक अक्षर **str, माप_प्रकार *size)
{
	स्थिरांक अक्षर *ret = *str;
	माप_प्रकार len;

	जर (!ret)
		परत रिक्त;

	len = strnlen(ret, *size - 1) + 1;

	जर ((*size -= len))
		*str += len;
	अन्यथा
		*str = रिक्त;

	परत ret;
}

स्थिर शून्य
append_user_path(खाता *fp)
{
	स्थिरांक अक्षर *xdg, *entry;
	माप_प्रकार size;

	append_path(fp, "%s", rc_dirs.confdir);
	जर (!(xdg = दो_पर्या("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	अन्यथा पर्यंत ((entry = strnsep(&xdg, &size, ':')))
		जर (size) append_path(fp, "%.*s/rc", (पूर्णांक) size, entry);

	साठी (माप_प्रकार i = 0; i < ARRAY_SIZE(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

स्थिर शून्य
append_rc_path(खाता *fp, स्थिरांक अक्षर *path, bool user)
{
	bool has_user = false;
	स्थिरांक अक्षर *entry;
	माप_प्रकार size;

	पर्यंत ((entry = strnsep(&path, &size, ':'))) {
		जर (size)
			append_path(fp, "%.*s%s", (पूर्णांक)size, entry, user ? "/user" : "");
		अन्यथा जर (user && !has_user)
			append_user_path(fp), has_user = true;
		अन्यथा जर (!user) पर्यंत (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

स्थिर शून्य
load_dynamic_path(bool user)
{
	स्थिरांक अक्षर *path, *entry;
	माप_प्रकार size, मोजणी = 0;
	खाता *fp;

	जर (!rc_dirs.set) {
		निकास_पर(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.मोजणी = ARRAY_SIZE(config_path) - 1;

	जर (!(path = दो_पर्या("RC_PATH")) && !user)
		परत;

	rc_path.मोजणी = 0;
	fp = xopen_memstream(&rc_path.buffer, &size);
	जर (path)
		append_rc_path(fp, path, user);
	अन्यथा
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = सुस्मृति(rc_path.मोजणी + 1, आकार(*rc_path.entries));

	path = rc_path.buffer;

	पर्यंत ((entry = strnullsep(&path, &size)) && मोजणी < rc_path.मोजणी)
		rc_path.entries[मोजणी++] = entry;
	rc_path.entries[rc_path.मोजणी] = रिक्त;
}

स्थिर bool is_user = false;

bool
rc_is_user(शून्य)
{
	पूर्णांक _is_user = -1;

	जर (_is_user == -1) {
		अक्षर *env = दो_पर्या("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	परत _is_user;
}

शून्य
rc_set_user(शून्य)
{
	अक्षर *env;

	जर (is_user)
		परत;

	जर (!rc_dirs.set)
		निकास_पर(free_rc_dirs);
	अन्यथा
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	जर ((env = दो_पर्या("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} अन्यथा जर ((env = दो_पर्या("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} अन्यथा {
		ख_लिखो(मानक_त्रुटि, "XDG_CONFIG_HOME and HOME unset.\n");
		निकास(निकास_त्रुटि);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	जर (!(env = दो_पर्या("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		ख_लिखो(मानक_त्रुटि, "XDG_RUNTIME_DIR unset.\n");
		निकास(निकास_त्रुटि);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

स्थिरांक अक्षर * स्थिरांक *
rc_scriptdirs(शून्य)
{
	जर (!rc_path.entries)
		load_dynamic_path(false);
	परत rc_path.entries;
}

माप_प्रकार
rc_scriptdirfds(स्थिरांक पूर्णांक **fds)
{
	स्थिर माप_प्रकार len = 0;

	जर (len == 0) {
		जर (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.मोजणी * आकार(*rc_path.fds));
		साठी (माप_प्रकार i = 0; i < rc_path.मोजणी; i++) {
			पूर्णांक fd = खोलो(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			जर (fd == -1)
				सुरूठेवा;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	परत len;
}

स्थिरांक अक्षर *
rc_sysconfdir(शून्य)
{
	परत RC_SYSCONFDIR;
}

स्थिरांक अक्षर *
rc_usrconfdir(शून्य)
{
	परत rc_dirs.confdir ? rc_dirs.confdir : रिक्त;
}

स्थिरांक अक्षर *
rc_runleveldir(शून्य)
{
	परत rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

स्थिरांक अक्षर *
rc_svcdir(शून्य)
{
	स्थिर स्थिरांक अक्षर *svcdir;

	जर (!rc_dirs.svcdir && !svcdir && !(svcdir = दो_पर्या("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	परत rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

स्थिर ssize_t
safe_readlink(पूर्णांक targetfd, स्थिरांक अक्षर *path, अक्षर **buf, माप_प्रकार bufsiz)
{
	अक्षर *buffer;
	ssize_t r;

	साठी (;; bufsiz += PATH_MAX) {
		buffer = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, buffer, bufsiz);
		जर (r < 0) {
			मुक्त(buffer);
			परत -त्रुटि_सं;
		}
		जर ((माप_प्रकार)r < bufsiz) {
			buffer[r] = '\0';
			*buf = buffer;
			परत r;
		}
		मुक्त(buffer);
		जर (bufsiz >= SSIZE_MAX - PATH_MAX)
			परत -ENOMEM;
	}
}

/* Resolve a service name to its full path */
अक्षर *
rc_service_resolve(स्थिरांक अक्षर *service)
{
	अक्षर *buffer;
	संरचना stat buf;

	जर (!service)
		परत रिक्त;

	जर (service[0] == '/')
		परत xstrdup(service);

	/* First check started services */
	जर (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		जर (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &buffer, buf.st_size + 1) >= 0)
			परत buffer;
	जर (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		जर (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &buffer, buf.st_size + 1) >= 0)
			परत buffer;

	साठी (स्थिरांक अक्षर * स्थिरांक *dirs = rc_scriptdirs(); *dirs; dirs++) {
		अक्षर *file = रिक्त;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		जर (stat(file, &buf) == 0)
			परत file;
		मुक्त(file);
	}

	परत रिक्त;
}

bool
rc_service_exists(स्थिरांक अक्षर *service)
{
	अक्षर *file;
	bool retval = false;
	माप_प्रकार len;
	संरचना stat buf;

	जर (!service) {
		त्रुटि_सं = EINVAL;
		परत false;
	}

	len = म_माप(service);

	/* .sh files are not init scripts */
	जर (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		त्रुटि_सं = EINVAL;
		परत false;
	}

	जर (!(file = rc_service_resolve(service))) {
		त्रुटि_सं = ENOENT;
		परत false;
	}

	जर (stat(file, &buf) == 0) {
		जर (buf.st_mode & S_IXUGO)
			retval = true;
		अन्यथा
			त्रुटि_सं = ENOEXEC;
	}
	मुक्त(file);
	परत retval;
}

#define OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(स्थिरांक अक्षर *service)
{
	अक्षर *svc;
	अक्षर *cmd = रिक्त;
	अक्षर *buffer = रिक्त;
	माप_प्रकार len = 0;
	RC_STRINGLIST *commands = रिक्त;
	अक्षर *token;
	अक्षर *p;
	खाता *fp;

	जर (!(svc = rc_service_resolve(service)))
		परत रिक्त;

	xasprintf(&cmd, OPTSTR, svc);
	मुक्त(svc);

	जर (!(fp = popen(cmd, "r"))) {
		मुक्त(cmd);
		परत रिक्त;
	}

	जर (xgetline(&buffer, &len, fp) != -1) {
		p = buffer;
		commands = rc_stringlist_new();

		पर्यंत ((token = strsep(&p, " ")))
			जर (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	मुक्त(buffer);
	मुक्त(cmd);
	परत commands;
}

#define DESCSTR ". '%s'; echo \"${description%s%s}\""
अक्षर *
rc_service_description(स्थिरांक अक्षर *service, स्थिरांक अक्षर *option)
{
	अक्षर *svc;
	अक्षर *cmd;
	अक्षर *desc = रिक्त;
	माप_प्रकार size = 0;
	खाता *fp;

	जर (!(svc = rc_service_resolve(service)))
		परत रिक्त;

	जर (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	मुक्त(svc);
	जर ((fp = popen(cmd, "r"))) {
		जर (xgetline(&desc, &size, fp) == -1) {
			मुक्त(desc);
			desc = रिक्त;
		}
		pclose(fp);
	}
	मुक्त(cmd);
	परत desc;
}

bool
rc_service_in_runlevel(स्थिरांक अक्षर *service, स्थिरांक अक्षर *runlevel)
{
	अक्षर *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	मुक्त(file);
	परत r;
}

bool
rc_service_mark(स्थिरांक अक्षर *service, स्थिरांक RC_SERVICE state)
{
	पूर्णांक i = 0;
	स्थिरांक अक्षर *base;
	अक्षर *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	पूर्णांक serrno;

	जर (!init)
		परत false;

	base = basename_c(service);
	जर (state != RC_SERVICE_STOPPED) {
		पूर्णांक state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		जर (access(init, F_OK) != 0) {
			मुक्त(init);
			परत false;
		}

		unlinkat(state_dirfd, base, 0);
		जर ((symlinkat(init, state_dirfd, base)) != 0) {
			मुक्त(init);
			परत false;
		}
	}

	जर (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		मुक्त(init);
		परत true;
	}

	/* Remove any old states now */
	साठी (i = 0; rc_service_state_names[i].name; i++) {
		पूर्णांक state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		जर (s == state)
			सुरूठेवा;

		निवड (s) {
		स्थिती RC_SERVICE_STOPPED:
		स्थिती RC_SERVICE_HOTPLUGGED:
		स्थिती RC_SERVICE_SCHEDULED:
			सुरूठेवा;
		स्थिती RC_SERVICE_WASINACTIVE:
			जर (skip_wasinactive)
				सुरूठेवा;
			/* fall-through */
		पूर्वनिर्धारित:
			जर (faccessat(state_dirfd, base, F_OK, 0) != 0)
				सुरूठेवा;
			जर ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				जर (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					मुक्त(init);
					परत false;
				}
				skip_wasinactive = true;
			}
			जर (unlinkat(state_dirfd, base, 0) == -1) {
				मुक्त(init);
				परत false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	जर (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	जर (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	जर (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		सूची *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		जर (dp) {
			संरचना dirent *d;
			पर्यंत ((d = सूची_पढ़ो(dp))) {
				अक्षर *was;

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
	परत true;
}

RC_SERVICE
rc_service_state(स्थिरांक अक्षर *service)
{
	स्थिरांक अक्षर *base = basename_c(service);
	पूर्णांक state = RC_SERVICE_STOPPED;
	पूर्णांक i;

	साठी (i = 0; rc_service_state_names[i].name; i++) {
		जर (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			जर (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			अन्यथा
				state |= rc_service_state_names[i].state;
		}
	}

	जर (state & RC_SERVICE_STARTED)
		जर (rc_service_daemons_crashed(service) && त्रुटि_सं != EACCES)
			state |= RC_SERVICE_CRASHED;

	जर (state & RC_SERVICE_STOPPED) {
		सूची *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		जर (dp) {
			संरचना dirent *d;
			पर्यंत ((d = सूची_पढ़ो(dp))) {
				जर (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					थांबा;
				}
			}

			बंद_सूची(dp);
		}
	}

	परत state;
}

अक्षर *
rc_service_मूल्य_get(स्थिरांक अक्षर *service, स्थिरांक अक्षर *option)
{
	अक्षर *buffer = रिक्त;
	माप_प्रकार len = 0;
	अक्षर *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &buffer, &len);
	मुक्त(file);

	परत buffer;
}

bool
rc_service_मूल्य_set(स्थिरांक अक्षर *service, स्थिरांक अक्षर *option, स्थिरांक अक्षर *मूल्य)
{
	bool ret = true;
	पूर्णांक optdirfd;
	खाता *fp;

	जर (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && त्रुटि_सं != EEXIST)
		परत false;

	जर ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		परत false;

	जर (!मूल्य) {
		unlinkat(optdirfd, option, 0);
	} अन्यथा जर ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		ख_लिखो(fp, "%s", मूल्य);
		ख_बंद(fp);
	} अन्यथा {
		ret = false;
	}

	बंद(optdirfd);
	परत ret;
}

bool
rc_service_schedule_start(स्थिरांक अक्षर *service, स्थिरांक अक्षर *service_to_start)
{
	स्थिरांक अक्षर *base_to_start = basename_c(service_to_start);
	स्थिरांक अक्षर *base = basename_c(service);
	bool retval;
	पूर्णांक schedfd;
	अक्षर *init;

	/* service may be a provided service, like net */
	जर (!service || !rc_service_exists(service_to_start))
		परत false;

	जर (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && त्रुटि_सं != EEXIST)
		परत false;

	जर ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		परत false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	बंद(schedfd);
	मुक्त(init);
	परत retval;
}

bool
rc_service_schedule_clear(स्थिरांक अक्षर *service)
{
	परत !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && त्रुटि_सं == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(स्थिरांक अक्षर *runlevel)
{
	RC_STRINGLIST *list = रिक्त;

	जर (!runlevel) {
		स्थिरांक पूर्णांक *scriptdirfds;
		list = rc_stringlist_new();
		साठी (माप_प्रकार i = 0, मोजणी = rc_scriptdirfds(&scriptdirfds); i < मोजणी; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		परत list;
	}

	/* These special levels never contain any services */
	जर (म_भेद(runlevel, RC_LEVEL_SINGLE) == 0)
		परत rc_stringlist_new();

	परत ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(स्थिरांक अक्षर *runlevel)
{
	RC_STRINGLIST *list, *stacks, *sl;
	RC_STRING *चळत;

	list = rc_services_in_runlevel(runlevel);
	stacks = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(चळत, stacks, entries) {
		sl = rc_services_in_runlevel(चळत->मूल्य);
		TAILQ_CONCAT(list, sl, entries);
		rc_stringlist_free(sl);
	}
	rc_stringlist_free(stacks);
	परत list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	संरचना dirent *d;
	सूची *dp;

	जर (state != RC_SERVICE_SCHEDULED)
		परत ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	जर (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		परत list;

	पर्यंत ((d = सूची_पढ़ो(dp))) {
		जर (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			सुरूठेवा;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	बंद_सूची(dp);

	परत list;
}

bool
rc_service_add(स्थिरांक अक्षर *runlevel, स्थिरांक अक्षर *service)
{
	bool retval;
	अक्षर *init;
	अक्षर *file;
	अक्षर *path;
	अक्षर *binit = रिक्त;
	अक्षर *i;

	जर (!rc_runlevel_exists(runlevel)) {
		त्रुटि_सं = ENOENT;
		परत false;
	}

	जर (rc_service_in_runlevel(service, runlevel)) {
		त्रुटि_सं = EEXIST;
		परत false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	जर (!rc_is_user() && म_भेद(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(स_नाम(init), रिक्त);
		जर (path == रिक्त) {
			मुक्त(init);
			परत false;
		}
		retval = (म_भेद(path, RC_INITDIR) == 0);
		मुक्त(path);
		जर (!retval) {
			मुक्त(init);
			त्रुटि_सं = EPERM;
			परत false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	जर (म_भेदन(i, rc_svcdir(), म_माप(rc_svcdir())) == 0) {
		त्रुटि_सं = ENOENT;
		retval = false;
		जा out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	मुक्त(file);

out:
	मुक्त(binit);
	मुक्त(init);
	परत retval;
}

bool
rc_service_delete(स्थिरांक अक्षर *runlevel, स्थिरांक अक्षर *service)
{
	अक्षर *file;
	पूर्णांक r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	मुक्त(file);
	परत (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(स्थिरांक अक्षर *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	संरचना dirent *d;
	सूची *dp;

	जर (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		परत list;

	पर्यंत ((d = सूची_पढ़ो(dp))) {
		अक्षर *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		जर (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		मुक्त(file);
	}

	परत list;
}

RC_STRINGLIST *
rc_services_scheduled(स्थिरांक अक्षर *service)
{
	परत ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(स्थिरांक अक्षर *service, स्थिरांक अक्षर *env)
{
	माप_प्रकार name_len = म_खोज(env, "=");
	अक्षर var[म_माप(service) + आकार("/") + name_len];
	पूर्णांक dfp = rc_dirfd(RC_DIR_ENVIRON);
	खाता *fp;

	जर (env[name_len] != '=')
		परत त्रुटि_सं = EINVAL, false;

	snprintf(var, आकार(var), "%s/%.*s", service, (पूर्णांक) name_len, env);

	जर (mkdirat(dfp, service, 0700) == -1 && त्रुटि_सं != EEXIST)
		परत false;

	जर (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		परत false;

	ख_माला_दो(&env[name_len + 1], fp);

	परत ख_बंद(fp) == 0;
}

bool
rc_service_setenv(स्थिरांक अक्षर *service, स्थिरांक अक्षर *name, स्थिरांक अक्षर *मूल्य)
{
	अक्षर var[म_माप(service) + आकार("/") + म_माप(name)];
	पूर्णांक dfp = rc_dirfd(RC_DIR_ENVIRON);
	खाता *fp;

	snprintf(var, आकार(var), "%s/%s", service, name);

	जर (mkdirat(dfp, service, 0700) == -1 && त्रुटि_सं != EEXIST)
		परत false;

	जर (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		परत false;

	ख_माला_दो(मूल्य, fp);

	परत ख_बंद(fp) == 0;
}

ssize_t
rc_service_getenv(स्थिरांक अक्षर *service, स्थिरांक अक्षर *name, अक्षर **buf)
{
	अक्षर var[म_माप(service) + आकार("/") + म_माप(name)], *मूल्य;
	पूर्णांक dfd = rc_dirfd(RC_DIR_ENVIRON);
	संरचना stat stat;
	माप_प्रकार मोजणी;
	खाता *fp;

	snprintf(var, आकार(var), "%s/%s", service, name);

	जर (fstatat(dfd, var, &stat, 0) == -1)
		परत -1;

	जर (!buf)
		परत stat.st_size;

	जर (!(मूल्य = xrealloc(*buf, stat.st_size + 1)))
		परत -1;

	जर (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		परत -1;

	मोजणी = ख_पढ़ो(मूल्य, 1, stat.st_size, fp);
	मूल्य[मोजणी] = '\0';

	ख_बंद(fp);
	*buf = मूल्य;
	परत मोजणी;
}
