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

#समावेश <assert.h>
#समावेश <helpers.h>
#समावेश <ctype.h>
#समावेश <dirent.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <libgen.h>
#समावेश <limits.h>
#समावेश <regex.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/param.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

#समावेश "queue.h"
#समावेश "librc.h"
#समावेश "misc.h"
#समावेश "rc.h"
#अगर_घोषित __FreeBSD__
#  include <sys/sysctl.h>
#पूर्ण_अगर

#अगर_अघोषित S_IXUGO
#  define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#पूर्ण_अगर

/* File stream used for plugins to write environ vars to */
खाता *rc_environ_fd = शून्य;
अटल पूर्णांक dirfds[RC_DIR_MAX];

पूर्णांक rc_dirfd(क्रमागत rc_dir dir) {
	पूर्णांक flags = O_RDONLY | O_CLOEXEC | O_DIRECTORY;
	पूर्णांक dirfd;

	अगर (dir < 0 || dir >= RC_DIR_MAX)
		वापस -1;
	अगर (dirfds[dir] <= 0) {
		चयन (dir) {
		हाल RC_DIR_SYSCONF:
			वापस dirfds[dir] = खोलो(rc_sysconfdir(), flags);
		हाल RC_DIR_USRCONF:
			वापस dirfds[dir] = खोलो(rc_usrconfdir(), flags);
		हाल RC_DIR_RUNLEVEL:
			वापस dirfds[dir] = खोलो(rc_runleveldir(), flags);
		हाल RC_DIR_SVCDIR:
			अगर (सूची_गढ़ो(rc_svcdir(), 0755) == -1 && त्रुटि_सं != EEXIST)
				वापस -1;
			वापस dirfds[dir] = खोलो(rc_svcdir(), flags);
		शेष:
			निश्चित(dirnames[dir]);
			dirfd = rc_dirfd(RC_DIR_SVCDIR);
			अगर ((dirfds[dir] = openat(dirfd, dirnames[dir], flags)) == -1 && त्रुटि_सं == ENOENT) {
				अगर (mkdirat(dirfd, dirnames[dir], 0755) == -1 && त्रुटि_सं != EEXIST)
					वापस -1;
				dirfds[dir] = openat(dirfd, dirnames[dir], flags);
			}
			अवरोध;
		}
	}
	वापस dirfds[dir];
}

व्योम clear_dirfds(व्योम) {
	क्रम (माप_प्रकार i = 0; i < RC_DIR_MAX; i++) {
		/* dirfd is zero-initialized, though zero is a valid
		 * FD, dirfd should never contain FDs which are <= 2. */
		अगर (dirfds[i] <= 0)
			जारी;

		बंद(dirfds[i]);
		dirfds[i] = -1;
	}
}

#घोषणा LS_INITD	0x01
#घोषणा LS_DIR		0x02
अटल RC_STRINGLIST *
ls_dir(पूर्णांक targetfd, स्थिर अक्षर *dir, पूर्णांक options)
{
	सूची *dp;
	काष्ठा dirent *d;
	RC_STRINGLIST *list = शून्य;
	काष्ठा stat buf;
	माप_प्रकार l;

	list = rc_stringlist_new();

	अगर (!(dp = do_opendirat(targetfd, dir)))
		वापस list;

	जबतक (((d = सूची_पढ़ो(dp)) != शून्य)) {
		अगर (d->d_name[0] == '.')
			जारी;
		अगर (options & LS_INITD) {
			/* Check that our file really exists.
			 * This is important as a service maybe in a
			 * runlevel, but could have been removed. */
			अगर (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0)
				जारी;

			/* .sh files are not init scripts */
			l = म_माप(d->d_name);
			अगर (l > 2 && d->d_name[l - 3] == '.' &&
					d->d_name[l - 2] == 's' &&
					d->d_name[l - 1] == 'h')
				जारी;
		}
		अगर (options & LS_DIR) {
			अगर (fstatat(dirfd(dp), d->d_name, &buf, 0) != 0 || !S_ISDIR(buf.st_mode))
				जारी;
		}
		rc_stringlist_add(list, d->d_name);
	}
	बंद_सूची(dp);
	वापस list;
}

अटल bool
rm_dir(पूर्णांक targetfd, स्थिर अक्षर *pathname, bool शीर्ष)
{
	bool retval = true;
	काष्ठा dirent *d;
	काष्ठा stat s;
	सूची *dp;

	अगर (!(dp = do_opendirat(targetfd, pathname)))
		वापस false;

	जबतक ((d = सूची_पढ़ो(dp))) {
		अगर (म_भेद(d->d_name, ".") == 0 || म_भेद(d->d_name, "..") == 0)
			जारी;
		अगर (fstatat(dirfd(dp), d->d_name, &s, 0) != 0) {
			retval = false;
			अवरोध;
		}
		अगर (S_ISDIR(s.st_mode)) {
			अगर (!rm_dir(dirfd(dp), d->d_name, true)) {
				retval = false;
				अवरोध;
			}
		} अन्यथा {
			अगर (unlinkat(dirfd(dp), d->d_name, 0) != 0) {
				retval = false;
				अवरोध;
			}
		}
	}
	बंद_सूची(dp);

	अगर (!retval)
		वापस false;

	अगर (शीर्ष && unlinkat(targetfd, pathname, AT_REMOVEDIR) != 0)
		वापस false;

	वापस true;
}

/* Other systems may need this at some point, but for now it's Linux only */
#अगर_घोषित __linux__
अटल bool
file_regex(स्थिर अक्षर *file, स्थिर अक्षर *regex)
{
	खाता *fp;
	अक्षर *पंक्ति = शून्य;
	माप_प्रकार आकार = 0;
	ssize_t len = 0;
	regex_t re;
	bool retval = true;
	पूर्णांक परिणाम;

	अगर (!(fp = ख_खोलो(file, "r")))
		वापस false;

	अगर ((परिणाम = regcomp(&re, regex, REG_EXTENDED | REG_NOSUB)) != 0) {
		अक्षर buf[बफ_मान];
		ख_बंद(fp);
		regerror(परिणाम, &re, buf, माप(buf));
		ख_लिखो(मानक_त्रुटि, "file_regex: %s", buf);
		वापस false;
	}

	जबतक ((len = xgetline(&पंक्ति, &आकार, fp)) != -1) {
		अक्षर *str = पंक्ति;
		/* some /proc files have \0 separated content so we have to
		   loop through the 'line' */
		करो {
			अगर (regexec(&re, str, 0, शून्य, 0) == 0)
				जाओ found;
			str += म_माप(str) + 1;
			/* len is the size of allocated buffer and we don't
			   want call regexec BUFSIZE times. find next str */
			जबतक (str < पंक्ति + len && *str == '\0')
				str++;
		} जबतक (str < पंक्ति + len);
	}
	retval = false;
found:
	ख_बंद(fp);
	मुक्त(पंक्ति);
	regfree(&re);

	वापस retval;
}
#पूर्ण_अगर


अटल स्थिर अक्षर *
get_systype(व्योम)
{
	अक्षर *systype = rc_conf_मूल्य("rc_sys");
	अगर (systype) {
		अक्षर *s = systype;
		/* Convert to uppercase */
		जबतक (s && *s) {
			अगर (है_छोटा((अचिन्हित अक्षर) *s))
				*s = बड़े((अचिन्हित अक्षर) *s);
			s++;
		}
	}
	वापस systype;
}

अटल स्थिर अक्षर *
detect_container(स्थिर अक्षर *systype RC_UNUSED)
{
#अगर_घोषित __FreeBSD__
	अगर (systype) {
		अगर (म_भेद(systype, RC_SYS_NONE) == 0)
		       वापस शून्य;
		अगर (म_भेद(systype, RC_SYS_JAIL) == 0)
			वापस RC_SYS_JAIL;
		अगर (म_भेद(systype, RC_SYS_PODMAN) == 0)
			वापस RC_SYS_PODMAN;
	}

	पूर्णांक jailed = 0;
	माप_प्रकार len = माप(jailed);

	अगर (sysctlbyname("security.jail.jailed", &jailed, &len, शून्य, 0) == 0)
		अगर (jailed == 1)
			वापस RC_SYS_JAIL;

	अगर (access("/var/run/.containerenv", F_OK) == 0)
		वापस RC_SYS_PODMAN;
#पूर्ण_अगर

#अगर_घोषित __linux__
	अगर (systype) {
		अगर (म_भेद(systype, RC_SYS_NONE) == 0)
			वापस शून्य;
		अगर (म_भेद(systype, RC_SYS_UML) == 0)
			वापस RC_SYS_UML;
		अगर (म_भेद(systype, RC_SYS_VSERVER) == 0)
			वापस RC_SYS_VSERVER;
		अगर (म_भेद(systype, RC_SYS_OPENVZ) == 0)
			वापस RC_SYS_OPENVZ;
		अगर (म_भेद(systype, RC_SYS_LXC) == 0)
			वापस RC_SYS_LXC;
		अगर (म_भेद(systype, RC_SYS_RKT) == 0)
				वापस RC_SYS_RKT;
		अगर (म_भेद(systype, RC_SYS_SYSTEMD_NSPAWN) == 0)
				वापस RC_SYS_SYSTEMD_NSPAWN;
		अगर (म_भेद(systype, RC_SYS_DOCKER) == 0)
				वापस RC_SYS_DOCKER;
		अगर (म_भेद(systype, RC_SYS_PODMAN) == 0)
			वापस RC_SYS_PODMAN;
		अगर (म_भेद(systype, RC_SYS_WSL) == 0)
			वापस RC_SYS_WSL;
	}
	अगर (file_regex("/proc/cpuinfo", "UML"))
		वापस RC_SYS_UML;
	अन्यथा अगर (file_regex("/proc/self/status",
		"(s_context|VxID):[[:space:]]*[1-9]"))
		वापस RC_SYS_VSERVER;
	अन्यथा अगर (access("/proc/vz/veinfo", F_OK) == 0 && access("/proc/vz/version", F_OK) != 0)
		वापस RC_SYS_OPENVZ;
	अन्यथा अगर (file_regex("/proc/self/status",
		"envID:[[:space:]]*[1-9]"))
		वापस RC_SYS_OPENVZ; /* old test */
	अन्यथा अगर (file_regex("/proc/1/environ", "container=lxc"))
		वापस RC_SYS_LXC;
	अन्यथा अगर (file_regex("/proc/1/environ", "container=rkt"))
		वापस RC_SYS_RKT;
	अन्यथा अगर (file_regex("/proc/1/environ", "container=systemd-nspawn"))
		वापस RC_SYS_SYSTEMD_NSPAWN;
	अन्यथा अगर (access("/run/.containerenv", F_OK) == 0)
		वापस RC_SYS_PODMAN;
	अन्यथा अगर (access("/.dockerenv", F_OK) == 0)
		वापस RC_SYS_DOCKER;
	/* old test, I'm not sure when this was valid. */
	अन्यथा अगर (file_regex("/proc/1/environ", "container=docker"))
		वापस RC_SYS_DOCKER;
	अन्यथा अगर (file_regex("/proc/sys/kernel/osrelease", "microsoft"))
		वापस RC_SYS_WSL;
	अन्यथा अगर (दो_पर्या("WSL_DISTRO_NAME"))
		वापस RC_SYS_WSL;
	अन्यथा अगर (access("/proc/sys/fs/binfmt_misc/WSLInterop", F_OK) == 0)
		वापस RC_SYS_WSL;
#पूर्ण_अगर

	वापस शून्य;
}

अटल स्थिर अक्षर *
detect_vm(स्थिर अक्षर *systype RC_UNUSED)
{
#अगर_घोषित __NetBSD__
	अगर (systype) {
		अगर (म_भेद(systype, RC_SYS_NONE) == 0)
			वापस शून्य;
		अगर (म_भेद(systype, RC_SYS_XEN0) == 0)
			वापस RC_SYS_XEN0;
		अगर (म_भेद(systype, RC_SYS_XENU) == 0)
			वापस RC_SYS_XENU;
	}
	अगर (access("/kern/xen/privcmd", F_OK) == 0)
		वापस RC_SYS_XEN0;
	अगर (access("/kern/xen", F_OK) == 0)
		वापस RC_SYS_XENU;
#पूर्ण_अगर

#अगर_घोषित __linux__
	अगर (systype) {
		अगर (म_भेद(systype, RC_SYS_NONE) == 0)
			वापस शून्य;
		अगर (म_भेद(systype, RC_SYS_XEN0) == 0)
			वापस RC_SYS_XEN0;
		अगर (म_भेद(systype, RC_SYS_XENU) == 0)
			वापस RC_SYS_XENU;
	}
	अगर (access("/proc/xen", F_OK) == 0) {
		अगर (file_regex("/proc/xen/capabilities", "control_d"))
			वापस RC_SYS_XEN0;
		वापस RC_SYS_XENU;
	}
#पूर्ण_अगर

	वापस शून्य;
}

स्थिर अक्षर *
rc_sys(व्योम)
{
	स्थिर अक्षर *systype;
	स्थिर अक्षर *sys;

	systype = get_systype();
	sys = detect_container(systype);
	अगर (!sys) {
		sys = detect_vm(systype);
	}

	वापस sys;
}

अटल स्थिर अक्षर *
rc_parse_service_state(RC_SERVICE state)
{
	पूर्णांक i;

	क्रम (i = 0; rc_service_state_names[i].name; i++) {
		अगर (rc_service_state_names[i].state == state)
			वापस rc_service_state_names[i].name;
	}
	वापस शून्य;
}

अटल पूर्णांक
rc_parse_service_state_dirfd(RC_SERVICE state)
{
	क्रम (माप_प्रकार i = 0; rc_service_state_names[i].name; i++)
		अगर (rc_service_state_names[i].state == state)
			वापस rc_service_state_names[i].dir;
	वापस -1;
}

/* Returns a list of all the chained runlevels used by the
 * specified runlevel in dependency order, including the
 * specified runlevel. */
अटल व्योम
get_runlevel_chain(स्थिर अक्षर *runlevel, RC_STRINGLIST *level_list, RC_STRINGLIST *ancestor_list)
{
	RC_STRINGLIST *dirs;
	RC_STRING *d, *parent;
	स्थिर अक्षर *nextlevel;

	/*
	 * If we haven't been passed a runlevel or a level list, or
	 * if the passed runlevel doesn't exist then we're done already!
	 */
	अगर (!runlevel || !level_list || !rc_runlevel_exists(runlevel))
		वापस;

	/*
	 * We want to add this runlevel to the list but if
	 * it is already in the list it needs to go at the
	 * end again.
	 */
	अगर (rc_stringlist_find(level_list, runlevel))
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
		अगर (rc_stringlist_find(ancestor_list, nextlevel)) {
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
rc_runlevel_starting(व्योम)
{
	वापस faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", F_OK, 0) == 0;
}

bool
rc_runlevel_stopping(व्योम)
{
	वापस faccessat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", F_OK, 0) == 0;
}

RC_STRINGLIST *rc_runlevel_list(व्योम)
{
	वापस ls_dir(AT_FDCWD, rc_runleveldir(), LS_DIR);
}

अक्षर *
rc_runlevel_get(व्योम)
{
	खाता *fp;
	अक्षर *runlevel = शून्य;
	माप_प्रकार i;

	अगर ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_RDONLY))) {
		runlevel = xmalloc(माप(अक्षर) * PATH_MAX);
		अगर (ख_माला_लो(runlevel, PATH_MAX, fp)) {
			i = म_माप(runlevel);
			अगर (i && runlevel[i - 1] == '\n')
				runlevel[i - 1] = 0;
		} अन्यथा
			*runlevel = '\0';
		ख_बंद(fp);
	}

	अगर (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = शून्य;
	}

	वापस runlevel;
}

bool
rc_runlevel_set(स्थिर अक्षर *runlevel)
{
	खाता *fp;

	अगर (!(fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "softlevel", O_WRONLY | O_CREAT | O_TRUNC)))
		वापस false;

	ख_माला_दो(runlevel, fp);
	ख_बंद(fp);
	वापस true;
}

bool
rc_runlevel_exists(स्थिर अक्षर *runlevel)
{
	काष्ठा stat buf;

	अगर (!runlevel || म_भेद(runlevel, "") == 0 || म_भेद(runlevel, ".") == 0 || म_भेद(runlevel, "..") == 0)
		वापस false;
	अगर (fstatat(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, &buf, 0) == 0 && S_ISDIR(buf.st_mode))
		वापस true;
	वापस false;
}

bool
rc_runlevel_ढेर(स्थिर अक्षर *dst, स्थिर अक्षर *src)
{
	अक्षर *d, *s;
	पूर्णांक r;

	अगर (!rc_runlevel_exists(dst) || !rc_runlevel_exists(src))
		वापस false;
	xasprintf(&s, "../%s", src);
	xasprintf(&d, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = symlink(s, d);
	मुक्त(d);
	मुक्त(s);
	वापस (r == 0);
}

bool
rc_runlevel_unstack(स्थिर अक्षर *dst, स्थिर अक्षर *src)
{
	अक्षर *path;
	पूर्णांक r;

	xasprintf(&path, "%s/%s/%s", rc_runleveldir(), dst, src);
	r = unlink(path);
	मुक्त(path);
	वापस (r == 0);
}

RC_STRINGLIST *
rc_runlevel_stacks(स्थिर अक्षर *runlevel)
{
	RC_STRINGLIST *ढेर, *ancestor_list;
	ढेर = rc_stringlist_new();
	ancestor_list = rc_stringlist_new();
	rc_stringlist_add(ancestor_list, runlevel);
	get_runlevel_chain(runlevel, ढेर, ancestor_list);
	rc_stringlist_free(ancestor_list);
	वापस ढेर;
}

अटल स्थिर अक्षर *config_path[] = { _RC_PATH, शून्य };

अटल काष्ठा {
	bool set;
	अक्षर *svcdir, *confdir, *runleveldir;
} rc_dirs;

अटल काष्ठा {
	अक्षर *बफर;
	माप_प्रकार गिनती;
	पूर्णांक *fds;
	स्थिर अक्षर **entries, **allocd;
} rc_path;

अटल व्योम
free_rc_dirs(व्योम)
{
#घोषणा X(ptr) ptr = (मुक्त(ptr), शून्य)
	X(rc_dirs.svcdir);
	X(rc_dirs.confdir);
	X(rc_dirs.runleveldir);
	X(rc_path.allocd);
	/* entries alias freed allocation */
	rc_path.entries = शून्य;
	X(rc_path.बफर);
	X(rc_path.fds);
	rc_path.गिनती = 0;
#अघोषित X
}

__attribute__((__format__(__printf__, 2, 3)))
अटल व्योम
append_path(खाता *fp, स्थिर अक्षर *fmt, ...)
{
	बहु_सूची va;

	बहु_शुरू(va, fmt);
	भख_लिखो(fp, fmt, va);
	ख_अक्षर_दो('\0', fp);
	बहु_पूर्ण(va);

	rc_path.गिनती++;
}

अटल स्थिर अक्षर *
strnsep(स्थिर अक्षर **str, माप_प्रकार *आकार, स्थिर अक्षर sep)
{
	स्थिर अक्षर *ret = *str;

	अगर (!ret)
		वापस शून्य;

	*str = strchrnul(ret, sep);
	*आकार = *str - ret;
	अगर (!**str)
		*str = शून्य;
	अन्यथा
		*str += 1;

	वापस ret;
}

अटल स्थिर अक्षर *
strnullsep(स्थिर अक्षर **str, माप_प्रकार *आकार)
{
	स्थिर अक्षर *ret = *str;
	माप_प्रकार len;

	अगर (!ret)
		वापस शून्य;

	len = strnlen(ret, *आकार - 1) + 1;

	अगर ((*आकार -= len))
		*str += len;
	अन्यथा
		*str = शून्य;

	वापस ret;
}

अटल व्योम
append_user_path(खाता *fp)
{
	स्थिर अक्षर *xdg, *entry;
	माप_प्रकार आकार;

	append_path(fp, "%s", rc_dirs.confdir);
	अगर (!(xdg = दो_पर्या("XDG_CONFIG_DIRS")))
		append_path(fp, "/etc/xdg/rc");
	अन्यथा जबतक ((entry = strnsep(&xdg, &आकार, ':')))
		अगर (आकार) append_path(fp, "%.*s/rc", (पूर्णांक) आकार, entry);

	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(config_path) - 2; i++)
		append_path(fp, "%s/user", config_path[i]);
	append_path(fp, "%s", rc_dirs.svcdir);
}

अटल व्योम
append_rc_path(खाता *fp, स्थिर अक्षर *path, bool user)
{
	bool has_user = false;
	स्थिर अक्षर *entry;
	माप_प्रकार आकार;

	जबतक ((entry = strnsep(&path, &आकार, ':'))) {
		अगर (आकार)
			append_path(fp, "%.*s%s", (पूर्णांक)आकार, entry, user ? "/user" : "");
		अन्यथा अगर (user && !has_user)
			append_user_path(fp), has_user = true;
		अन्यथा अगर (!user) जबतक (*rc_path.entries)
			append_path(fp, "%s", *rc_path.entries++);
	}
}

अटल व्योम
load_dynamic_path(bool user)
{
	स्थिर अक्षर *path, *entry;
	माप_प्रकार आकार, गिनती = 0;
	खाता *fp;

	अगर (!rc_dirs.set) {
		निकास_पर(free_rc_dirs);
		rc_dirs.set = true;
	}

	rc_path.entries = config_path;
	rc_path.गिनती = ARRAY_आकार(config_path) - 1;

	अगर (!(path = दो_पर्या("RC_PATH")) && !user)
		वापस;

	rc_path.गिनती = 0;
	fp = xopen_memstream(&rc_path.बफर, &आकार);
	अगर (path)
		append_rc_path(fp, path, user);
	अन्यथा
		append_user_path(fp);
	xclose_memstream(fp);

	rc_path.allocd = rc_path.entries = सुस्मृति(rc_path.गिनती + 1, माप(*rc_path.entries));

	path = rc_path.बफर;

	जबतक ((entry = strnullsep(&path, &आकार)) && गिनती < rc_path.गिनती)
		rc_path.entries[गिनती++] = entry;
	rc_path.entries[rc_path.गिनती] = शून्य;
}

अटल bool is_user = false;

bool
rc_is_user(व्योम)
{
	पूर्णांक _is_user = -1;

	अगर (_is_user == -1) {
		अक्षर *env = दो_पर्या("RC_USER_SERVICES");

		_is_user = env ? rc_yesno(env) : is_user;
	}

	वापस _is_user;
}

व्योम
rc_set_user(व्योम)
{
	अक्षर *env;

	अगर (is_user)
		वापस;

	अगर (!rc_dirs.set)
		निकास_पर(free_rc_dirs);
	अन्यथा
		free_rc_dirs();

	is_user = true;
	rc_dirs.set = true;
	setenv("RC_USER_SERVICES", "yes", true);

	अगर ((env = दो_पर्या("XDG_CONFIG_HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/rc", env);
	} अन्यथा अगर ((env = दो_पर्या("HOME"))) {
		xasprintf(&rc_dirs.confdir, "%s/.config/rc", env);
	} अन्यथा {
		ख_लिखो(मानक_त्रुटि, "XDG_CONFIG_HOME and HOME unset.\n");
		निकास(निकास_त्रुटि);
	}

	xasprintf(&rc_dirs.runleveldir, "%s/runlevels", rc_dirs.confdir);

	अगर (!(env = दो_पर्या("XDG_RUNTIME_DIR"))) {
		/* FIXME: fallback to something else? */
		ख_लिखो(मानक_त्रुटि, "XDG_RUNTIME_DIR unset.\n");
		निकास(निकास_त्रुटि);
	}

	xasprintf(&rc_dirs.svcdir, "%s/openrc", env);

	load_dynamic_path(true);

	clear_dirfds();
}

स्थिर अक्षर * स्थिर *
rc_scriptdirs(व्योम)
{
	अगर (!rc_path.entries)
		load_dynamic_path(false);
	वापस rc_path.entries;
}

माप_प्रकार
rc_scriptdirfds(स्थिर पूर्णांक **fds)
{
	अटल माप_प्रकार len = 0;

	अगर (len == 0) {
		अगर (!rc_path.entries)
			load_dynamic_path(false);

		rc_path.fds = xmalloc(rc_path.गिनती * माप(*rc_path.fds));
		क्रम (माप_प्रकार i = 0; i < rc_path.गिनती; i++) {
			पूर्णांक fd = खोलो(rc_path.entries[i], O_RDONLY | O_DIRECTORY | O_CLOEXEC);

			अगर (fd == -1)
				जारी;

			rc_path.fds[len++] = fd;
		}
	}

	*fds = rc_path.fds;
	वापस len;
}

स्थिर अक्षर *
rc_sysconfdir(व्योम)
{
	वापस RC_SYSCONFDIR;
}

स्थिर अक्षर *
rc_usrconfdir(व्योम)
{
	वापस rc_dirs.confdir ? rc_dirs.confdir : शून्य;
}

स्थिर अक्षर *
rc_runleveldir(व्योम)
{
	वापस rc_dirs.runleveldir ? rc_dirs.runleveldir : RC_RUNLEVELDIR;
}

स्थिर अक्षर *
rc_svcdir(व्योम)
{
	अटल स्थिर अक्षर *svcdir;

	अगर (!rc_dirs.svcdir && !svcdir && !(svcdir = दो_पर्या("RC_SVCDIR")))
		svcdir = RC_SVCDIR;
	वापस rc_dirs.svcdir ? rc_dirs.svcdir : svcdir;
}

अटल ssize_t
safe_readlink(पूर्णांक targetfd, स्थिर अक्षर *path, अक्षर **buf, माप_प्रकार bufsiz)
{
	अक्षर *बफर;
	ssize_t r;

	क्रम (;; bufsiz += PATH_MAX) {
		बफर = xmalloc(bufsiz);
		r = readlinkat(targetfd, path, बफर, bufsiz);
		अगर (r < 0) {
			मुक्त(बफर);
			वापस -त्रुटि_सं;
		}
		अगर ((माप_प्रकार)r < bufsiz) {
			बफर[r] = '\0';
			*buf = बफर;
			वापस r;
		}
		मुक्त(बफर);
		अगर (bufsiz >= SSIZE_MAX - PATH_MAX)
			वापस -ENOMEM;
	}
}

/* Resolve a service name to its full path */
अक्षर *
rc_service_resolve(स्थिर अक्षर *service)
{
	अक्षर *बफर;
	काष्ठा stat buf;

	अगर (!service)
		वापस शून्य;

	अगर (service[0] == '/')
		वापस xstrdup(service);

	/* First check started services */
	अगर (fstatat(rc_dirfd(RC_DIR_STARTED), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		अगर (safe_readlink(rc_dirfd(RC_DIR_STARTED), service, &बफर, buf.st_size + 1) >= 0)
			वापस बफर;
	अगर (fstatat(rc_dirfd(RC_DIR_INACTIVE), service, &buf, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(buf.st_mode))
		अगर (safe_readlink(rc_dirfd(RC_DIR_INACTIVE), service, &बफर, buf.st_size + 1) >= 0)
			वापस बफर;

	क्रम (स्थिर अक्षर * स्थिर *dirs = rc_scriptdirs(); *dirs; dirs++) {
		अक्षर *file = शून्य;
		xasprintf(&file, "%s/init.d/%s", *dirs, service);
		अगर (stat(file, &buf) == 0)
			वापस file;
		मुक्त(file);
	}

	वापस शून्य;
}

bool
rc_service_exists(स्थिर अक्षर *service)
{
	अक्षर *file;
	bool retval = false;
	माप_प्रकार len;
	काष्ठा stat buf;

	अगर (!service) {
		त्रुटि_सं = EINVAL;
		वापस false;
	}

	len = म_माप(service);

	/* .sh files are not init scripts */
	अगर (len > 2 && service[len - 3] == '.' &&
			service[len - 2] == 's' &&
			service[len - 1] == 'h') {
		त्रुटि_सं = EINVAL;
		वापस false;
	}

	अगर (!(file = rc_service_resolve(service))) {
		त्रुटि_सं = ENOENT;
		वापस false;
	}

	अगर (stat(file, &buf) == 0) {
		अगर (buf.st_mode & S_IXUGO)
			retval = true;
		अन्यथा
			त्रुटि_सं = ENOEXEC;
	}
	मुक्त(file);
	वापस retval;
}

#घोषणा OPTSTR \
". '%s'; echo $extra_commands $extra_started_commands $extra_stopped_commands"

RC_STRINGLIST *
rc_service_extra_commands(स्थिर अक्षर *service)
{
	अक्षर *svc;
	अक्षर *cmd = शून्य;
	अक्षर *बफर = शून्य;
	माप_प्रकार len = 0;
	RC_STRINGLIST *commands = शून्य;
	अक्षर *token;
	अक्षर *p;
	खाता *fp;

	अगर (!(svc = rc_service_resolve(service)))
		वापस शून्य;

	xasprintf(&cmd, OPTSTR, svc);
	मुक्त(svc);

	अगर (!(fp = popen(cmd, "r"))) {
		मुक्त(cmd);
		वापस शून्य;
	}

	अगर (xgetline(&बफर, &len, fp) != -1) {
		p = बफर;
		commands = rc_stringlist_new();

		जबतक ((token = strsep(&p, " ")))
			अगर (token[0] != '\0')
				rc_stringlist_add(commands, token);
	}

	pclose(fp);
	मुक्त(बफर);
	मुक्त(cmd);
	वापस commands;
}

#घोषणा DESCSTR ". '%s'; echo \"${description%s%s}\""
अक्षर *
rc_service_description(स्थिर अक्षर *service, स्थिर अक्षर *option)
{
	अक्षर *svc;
	अक्षर *cmd;
	अक्षर *desc = शून्य;
	माप_प्रकार आकार = 0;
	खाता *fp;

	अगर (!(svc = rc_service_resolve(service)))
		वापस शून्य;

	अगर (!option)
		option = "";

	xasprintf(&cmd, DESCSTR, svc, *option ? "_" : "", option);
	मुक्त(svc);
	अगर ((fp = popen(cmd, "r"))) {
		अगर (xgetline(&desc, &आकार, fp) == -1) {
			मुक्त(desc);
			desc = शून्य;
		}
		pclose(fp);
	}
	मुक्त(cmd);
	वापस desc;
}

bool
rc_service_in_runlevel(स्थिर अक्षर *service, स्थिर अक्षर *runlevel)
{
	अक्षर *file;
	bool r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	r = access(file, F_OK) == 0;
	मुक्त(file);
	वापस r;
}

bool
rc_service_mark(स्थिर अक्षर *service, स्थिर RC_SERVICE state)
{
	पूर्णांक i = 0;
	स्थिर अक्षर *base;
	अक्षर *init = rc_service_resolve(service);
	bool skip_wasinactive = false;
	पूर्णांक serrno;

	अगर (!init)
		वापस false;

	base = basename_c(service);
	अगर (state != RC_SERVICE_STOPPED) {
		पूर्णांक state_dirfd = rc_dirfd(rc_parse_service_state_dirfd(state));
		अगर (access(init, F_OK) != 0) {
			मुक्त(init);
			वापस false;
		}

		unlinkat(state_dirfd, base, 0);
		अगर ((symlinkat(init, state_dirfd, base)) != 0) {
			मुक्त(init);
			वापस false;
		}
	}

	अगर (state == RC_SERVICE_HOTPLUGGED || state == RC_SERVICE_FAILED) {
		मुक्त(init);
		वापस true;
	}

	/* Remove any old states now */
	क्रम (i = 0; rc_service_state_names[i].name; i++) {
		पूर्णांक state_dirfd = rc_dirfd(rc_service_state_names[i].dir);
		RC_SERVICE s = rc_service_state_names[i].state;

		अगर (s == state)
			जारी;

		चयन (s) {
		हाल RC_SERVICE_STOPPED:
		हाल RC_SERVICE_HOTPLUGGED:
		हाल RC_SERVICE_SCHEDULED:
			जारी;
		हाल RC_SERVICE_WASINACTIVE:
			अगर (skip_wasinactive)
				जारी;
			/* fall-through */
		शेष:
			अगर (faccessat(state_dirfd, base, F_OK, 0) != 0)
				जारी;
			अगर ((state == RC_SERVICE_STARTING || state == RC_SERVICE_STOPPING) && s == RC_SERVICE_INACTIVE) {
				अगर (symlinkat(init, rc_dirfd(RC_DIR_WASINACTIVE), base) == -1) {
					मुक्त(init);
					वापस false;
				}
				skip_wasinactive = true;
			}
			अगर (unlinkat(state_dirfd, base, 0) == -1) {
				मुक्त(init);
				वापस false;
			}
		}
	}

	/* Remove the exclusive state if we're inactive */
	अगर (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED || state == RC_SERVICE_INACTIVE)
		unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), base, 0);

	/* Remove any options and daemons the service may have stored */
	अगर (state == RC_SERVICE_STOPPED) {
		rm_dir(rc_dirfd(RC_DIR_OPTIONS), base, true);
		rm_dir(rc_dirfd(RC_DIR_DAEMONS), base, true);
		unlinkat(rc_dirfd(RC_DIR_ENVIRON), base, 0);
		rc_service_schedule_clear(service);
	}

	/* These are final states, so remove us from scheduled */
	अगर (state == RC_SERVICE_STARTED || state == RC_SERVICE_STOPPED) {
		सूची *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		अगर (dp) {
			काष्ठा dirent *d;
			जबतक ((d = सूची_पढ़ो(dp))) {
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
	वापस true;
}

RC_SERVICE
rc_service_state(स्थिर अक्षर *service)
{
	स्थिर अक्षर *base = basename_c(service);
	पूर्णांक state = RC_SERVICE_STOPPED;
	पूर्णांक i;

	क्रम (i = 0; rc_service_state_names[i].name; i++) {
		अगर (faccessat(rc_dirfd(rc_service_state_names[i].dir), base, F_OK, 0) == 0) {
			अगर (rc_service_state_names[i].state <= 0x10)
				state = rc_service_state_names[i].state;
			अन्यथा
				state |= rc_service_state_names[i].state;
		}
	}

	अगर (state & RC_SERVICE_STARTED)
		अगर (rc_service_daemons_crashed(service) && त्रुटि_सं != EACCES)
			state |= RC_SERVICE_CRASHED;

	अगर (state & RC_SERVICE_STOPPED) {
		सूची *dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED));
		अगर (dp) {
			काष्ठा dirent *d;
			जबतक ((d = सूची_पढ़ो(dp))) {
				अगर (faccessat(dirfd(dp), service, F_OK, 0) == 0) {
					state |= RC_SERVICE_SCHEDULED;
					अवरोध;
				}
			}

			बंद_सूची(dp);
		}
	}

	वापस state;
}

अक्षर *
rc_service_मूल्य_get(स्थिर अक्षर *service, स्थिर अक्षर *option)
{
	अक्षर *बफर = शून्य;
	माप_प्रकार len = 0;
	अक्षर *file;

	xasprintf(&file, "%s/options/%s/%s", rc_svcdir(), service, option);
	rc_getfile(file, &बफर, &len);
	मुक्त(file);

	वापस बफर;
}

bool
rc_service_मूल्य_set(स्थिर अक्षर *service, स्थिर अक्षर *option, स्थिर अक्षर *मूल्य)
{
	bool ret = true;
	पूर्णांक optdirfd;
	खाता *fp;

	अगर (mkdirat(rc_dirfd(RC_DIR_OPTIONS), service, 0755) != 0 && त्रुटि_सं != EEXIST)
		वापस false;

	अगर ((optdirfd = openat(rc_dirfd(RC_DIR_OPTIONS), service, O_RDONLY | O_DIRECTORY)) == -1)
		वापस false;

	अगर (!मूल्य) {
		unlinkat(optdirfd, option, 0);
	} अन्यथा अगर ((fp = do_fopenat(optdirfd, option, O_WRONLY | O_CREAT | O_TRUNC))) {
		ख_लिखो(fp, "%s", मूल्य);
		ख_बंद(fp);
	} अन्यथा {
		ret = false;
	}

	बंद(optdirfd);
	वापस ret;
}

bool
rc_service_schedule_start(स्थिर अक्षर *service, स्थिर अक्षर *service_to_start)
{
	स्थिर अक्षर *base_to_start = basename_c(service_to_start);
	स्थिर अक्षर *base = basename_c(service);
	bool retval;
	पूर्णांक schedfd;
	अक्षर *init;

	/* service may be a provided service, like net */
	अगर (!service || !rc_service_exists(service_to_start))
		वापस false;

	अगर (mkdirat(rc_dirfd(RC_DIR_SCHEDULED), base, 0755) != 0 && त्रुटि_सं != EEXIST)
		वापस false;

	अगर ((schedfd = openat(rc_dirfd(RC_DIR_SCHEDULED), base, O_RDONLY)) == -1)
		वापस false;

	init = rc_service_resolve(service_to_start);

	retval = (faccessat(schedfd, base_to_start, F_OK, 0) == 0) || (symlinkat(init, schedfd, base_to_start) == 0);

	बंद(schedfd);
	मुक्त(init);
	वापस retval;
}

bool
rc_service_schedule_clear(स्थिर अक्षर *service)
{
	वापस !rm_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), true) && त्रुटि_सं == ENOENT;
}

RC_STRINGLIST *
rc_services_in_runlevel(स्थिर अक्षर *runlevel)
{
	RC_STRINGLIST *list = शून्य;

	अगर (!runlevel) {
		स्थिर पूर्णांक *scriptdirfds;
		list = rc_stringlist_new();
		क्रम (माप_प्रकार i = 0, गिनती = rc_scriptdirfds(&scriptdirfds); i < गिनती; i++) {
			RC_STRINGLIST *svcs;

			svcs = ls_dir(scriptdirfds[i], "init.d", LS_INITD);
			TAILQ_CONCAT(list, svcs, entries);
			rc_stringlist_free(svcs);
		}
		वापस list;
	}

	/* These special levels never contain any services */
	अगर (म_भेद(runlevel, RC_LEVEL_SINGLE) == 0)
		वापस rc_stringlist_new();

	वापस ls_dir(rc_dirfd(RC_DIR_RUNLEVEL), runlevel, LS_INITD);
}

RC_STRINGLIST *
rc_services_in_runlevel_stacked(स्थिर अक्षर *runlevel)
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
	वापस list;
}

RC_STRINGLIST *
rc_services_in_state(RC_SERVICE state)
{
	RC_STRINGLIST *services;
	RC_STRINGLIST *list;
	काष्ठा dirent *d;
	सूची *dp;

	अगर (state != RC_SERVICE_SCHEDULED)
		वापस ls_dir(rc_dirfd(RC_DIR_SVCDIR), rc_parse_service_state(state), LS_INITD);

	list = rc_stringlist_new();
	अगर (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "scheduled")))
		वापस list;

	जबतक ((d = सूची_पढ़ो(dp))) {
		अगर (!(services = ls_dir(dirfd(dp), d->d_name, LS_INITD)))
			जारी;
		TAILQ_CONCAT(list, services, entries);
		rc_stringlist_free(services);
	}
	बंद_सूची(dp);

	वापस list;
}

bool
rc_service_add(स्थिर अक्षर *runlevel, स्थिर अक्षर *service)
{
	bool retval;
	अक्षर *init;
	अक्षर *file;
	अक्षर *path;
	अक्षर *binit = शून्य;
	अक्षर *i;

	अगर (!rc_runlevel_exists(runlevel)) {
		त्रुटि_सं = ENOENT;
		वापस false;
	}

	अगर (rc_service_in_runlevel(service, runlevel)) {
		त्रुटि_सं = EEXIST;
		वापस false;
	}

	i = init = rc_service_resolve(service);

	/* We need to ensure that only things in /etc/init.d are added
	 * to the boot runlevel -- not applicable for user services. */
	अगर (!rc_is_user() && म_भेद(runlevel, RC_LEVEL_BOOT) == 0) {
		path = realpath(स_नाम(init), शून्य);
		अगर (path == शून्य) {
			मुक्त(init);
			वापस false;
		}
		retval = (म_भेद(path, RC_INITDIR) == 0);
		मुक्त(path);
		अगर (!retval) {
			मुक्त(init);
			त्रुटि_सं = EPERM;
			वापस false;
		}
		xasprintf(&binit, "%s/%s", RC_INITDIR, service);
		i = binit;
	}

	/* We shouldn't add any temporary script to a runlevel */
	अगर (म_भेदन(i, rc_svcdir(), म_माप(rc_svcdir())) == 0) {
		त्रुटि_सं = ENOENT;
		retval = false;
		जाओ out;
	}

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel, basename_c(service));
	retval = (symlink(i, file) == 0);
	मुक्त(file);

out:
	मुक्त(binit);
	मुक्त(init);
	वापस retval;
}

bool
rc_service_delete(स्थिर अक्षर *runlevel, स्थिर अक्षर *service)
{
	अक्षर *file;
	पूर्णांक r;

	xasprintf(&file, "%s/%s/%s", rc_runleveldir(), runlevel,
		basename_c(service));
	r = unlink(file);
	मुक्त(file);
	वापस (r == 0);
}

RC_STRINGLIST *
rc_services_scheduled_by(स्थिर अक्षर *service)
{
	RC_STRINGLIST *list = rc_stringlist_new();
	काष्ठा dirent *d;
	सूची *dp;

	अगर (!(dp = do_dopendir(rc_dirfd(RC_DIR_SCHEDULED))))
		वापस list;

	जबतक ((d = सूची_पढ़ो(dp))) {
		अक्षर *file;
		xasprintf(&file, "%s/%s", d->d_name, service);
		अगर (faccessat(dirfd(dp), file, F_OK, 0) == 0)
			rc_stringlist_add(list, d->d_name);
		मुक्त(file);
	}

	वापस list;
}

RC_STRINGLIST *
rc_services_scheduled(स्थिर अक्षर *service)
{
	वापस ls_dir(rc_dirfd(RC_DIR_SCHEDULED), basename_c(service), LS_INITD);
}

bool
rc_service_putenv(स्थिर अक्षर *service, स्थिर अक्षर *env)
{
	माप_प्रकार name_len = म_खोज(env, "=");
	अक्षर var[म_माप(service) + माप("/") + name_len];
	पूर्णांक dfp = rc_dirfd(RC_DIR_ENVIRON);
	खाता *fp;

	अगर (env[name_len] != '=')
		वापस त्रुटि_सं = EINVAL, false;

	snprintf(var, माप(var), "%s/%.*s", service, (पूर्णांक) name_len, env);

	अगर (mkdirat(dfp, service, 0700) == -1 && त्रुटि_सं != EEXIST)
		वापस false;

	अगर (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		वापस false;

	ख_माला_दो(&env[name_len + 1], fp);

	वापस ख_बंद(fp) == 0;
}

bool
rc_service_setenv(स्थिर अक्षर *service, स्थिर अक्षर *name, स्थिर अक्षर *मूल्य)
{
	अक्षर var[म_माप(service) + माप("/") + म_माप(name)];
	पूर्णांक dfp = rc_dirfd(RC_DIR_ENVIRON);
	खाता *fp;

	snprintf(var, माप(var), "%s/%s", service, name);

	अगर (mkdirat(dfp, service, 0700) == -1 && त्रुटि_सं != EEXIST)
		वापस false;

	अगर (!(fp = do_fopenat(dfp, var, O_WRONLY | O_CREAT | O_TRUNC)))
		वापस false;

	ख_माला_दो(मूल्य, fp);

	वापस ख_बंद(fp) == 0;
}

ssize_t
rc_service_getenv(स्थिर अक्षर *service, स्थिर अक्षर *name, अक्षर **buf)
{
	अक्षर var[म_माप(service) + माप("/") + म_माप(name)], *मूल्य;
	पूर्णांक dfd = rc_dirfd(RC_DIR_ENVIRON);
	काष्ठा stat stat;
	माप_प्रकार गिनती;
	खाता *fp;

	snprintf(var, माप(var), "%s/%s", service, name);

	अगर (fstatat(dfd, var, &stat, 0) == -1)
		वापस -1;

	अगर (!buf)
		वापस stat.st_size;

	अगर (!(मूल्य = xrealloc(*buf, stat.st_size + 1)))
		वापस -1;

	अगर (!(fp = do_fopenat(dfd, var, O_RDONLY)))
		वापस -1;

	गिनती = ख_पढ़ो(मूल्य, 1, stat.st_size, fp);
	मूल्य[गिनती] = '\0';

	ख_बंद(fp);
	*buf = मूल्य;
	वापस गिनती;
}
