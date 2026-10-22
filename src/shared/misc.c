/*
 * misc.c
 * misc functions
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

#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <fnmatch.h>
#समावेश <limits.h>
#अगर_घोषित HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#पूर्ण_अगर
#समावेश <signal.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <spawn.h>
#समावेश <sys/file.h>
#समावेश <sys/time.h>
#अगर_घोषित __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#पूर्ण_अगर
#समावेश <sys/types.h>
#समावेश <sys/utsname.h>
#समावेश <sys/un.h>
#समावेश <sys/socket.h>
#समावेश <time.h>
#समावेश <unistd.h>
#समावेश <utime.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "version.h"
#समावेश "helpers.h"

बाह्य अक्षर **environ;

bool
rc_conf_yesno(स्थिर अक्षर *setting)
{
	वापस rc_yesno(rc_conf_मूल्य (setting));
}

अटल स्थिर अक्षर *स्थिर env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	शून्य
};

अटल स्थिर अक्षर *स्थिर usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", शून्य
};

अटल bool
env_allowed(स्थिर अक्षर *स्थिर list[], स्थिर अक्षर *मूल्य)
{
	क्रम (माप_प्रकार i = 0; list[i]; i++)
		अगर (fnmatch(list[i], मूल्य, FNM_NOESCAPE | FNM_PERIOD) == 0)
			वापस true;
	वापस false;
}

व्योम
env_filter(व्योम)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	अक्षर *profile_path;
	RC_STRING *env;
	अक्षर *e;
	माप_प्रकार i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_मूल्य("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	अगर (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		वापस;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	मुक्त(profile_path);

	अगर (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		मुक्त(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	जबतक (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = म_अक्षर(env->मूल्य, '=');
		अगर (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		अगर (env_allowed(env_allowlist, env->मूल्य))
			जारी;
		अगर (rc_is_user() && env_allowed(usrenv_allowlist, env->मूल्य))
			जारी;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			अगर (fnmatch(var->मूल्य, env->मूल्य, FNM_NOESCAPE | FNM_PERIOD) == 0)
				जाओ अग्र;

		/* OK, not allowed! */
		unsetenv(env->मूल्य);
अग्र:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = म_अक्षर(env->मूल्य, '=');
		*e = '\0';
		अगर (!दो_पर्या(env->मूल्य))
			setenv(env->मूल्य, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

अक्षर *
default_runlevel(व्योम)
{
	अक्षर *runlevel = rc_runlevel_get();

	अगर (!runlevel)
		runlevel = xstrdup(rc_conf_मूल्य("rc_default_runlevel"));
	अगर (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	वापस runlevel;
}

व्योम
env_config(व्योम)
{
	माप_प्रकार pplen = म_माप(RC_PATH_PREFIX);
	अक्षर *path;
	अक्षर *p;
	अक्षर *e;
	माप_प्रकार l;
	काष्ठा utsname uts;
	खाता *fp;
	अक्षर *token;
	अक्षर *np;
	अक्षर *npp;
	अक्षर *tok;
	स्थिर अक्षर *sys = rc_sys();
	स्थिर अक्षर *svcdir = rc_svcdir();
	स्थिर अक्षर *स्थिर *init_path = rc_scriptdirs();
	अक्षर *बफर = शून्य;
	अक्षर *tmpdir;
	माप_प्रकार आकार = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = दो_पर्या("PATH");
	अगर (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	अन्यथा अगर (म_भेदन (RC_PATH_PREFIX, path, pplen) != 0) {
		l = म_माप(path) + pplen + 3;
		e = p = xmalloc(माप(अक्षर) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		जबतक ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			जबतक ((tok = strsep(&npp, ":")))
				अगर (म_भेद(tok, token) == 0)
					अवरोध;
			अगर (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			मुक्त (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		मुक्त(e);
	}

	अगर (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} अन्यथा {
		स्थिर अक्षर *cache_home = दो_पर्या("XDG_CACHE_HOME");
		अक्षर *cachedir = शून्य;

		अगर (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		अन्यथा अगर ((cache_home = दो_पर्या("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		अगर (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		मुक्त(cachedir);
	}

	xasprintf(&tmpdir, "%s/tmp", svcdir);
	e = default_runlevel();

	setenv("RC_VERSION", VERSION, 1);
	setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 1);
	setenv("RC_SVCDIR", svcdir, 1);
	setenv("RC_TMPDIR", tmpdir, 1);
	setenv("RC_BOOTLEVEL", RC_LEVEL_BOOT, 1);
	setenv("RC_RUNLEVEL", e, 1);

	fp = xopen_memstream(&path, &आकार);
	ख_माला_दो("RC_PATH=", fp);
	क्रम (bool first = true; *init_path; init_path++, first = false) {
		अगर (!first)
			ख_अक्षर_दो(':', fp);
		ख_माला_दो(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	मुक्त(e);
	मुक्त(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	अगर (!rc_is_user() && (fp = ख_खोलो(RC_KRUNLEVEL, "r"))) {
		अगर (xgetline(&बफर, &आकार, fp) != -1)
			setenv("RC_DEFAULTLEVEL", बफर, 1);
		अन्यथा
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		मुक्त(बफर);
		ख_बंद(fp);
	} अन्यथा {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	अगर (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	अगर (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	अगर (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	अगर (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	त्रुटि_सं = 0;
	अगर ((!rc_conf_yesno("rc_color") && त्रुटि_सं == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

पूर्णांक
signal_setup(पूर्णांक sig, व्योम (*handler)(पूर्णांक))
{
	काष्ठा sigaction sa;

	स_रखो(&sa, 0, माप (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	वापस sigaction(sig, &sa, शून्य);
}

पूर्णांक
svc_lock(स्थिर अक्षर *applet, bool ignore_lock_failure)
{
	पूर्णांक fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	अगर (fd == -1)
		वापस -1;
	अगर (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		पूर्णांक saved_errno = त्रुटि_सं;
		अगर (ignore_lock_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			निकास(निकास_सफल);
		}
		eerror("Call to flock failed: %s", म_त्रुटि(त्रुटि_सं));
		बंद(fd);
		त्रुटि_सं = saved_errno;
		वापस -1;
	}
	वापस fd;
}

पूर्णांक
svc_unlock(स्थिर अक्षर *applet, पूर्णांक fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	बंद(fd);
	वापस -1;
}

pid_t
exec_service(स्थिर अक्षर *service, स्थिर अक्षर *arg)
{
	अक्षर *file, sfd[32];
	स्थिर अक्षर *argv[] = { service, "--lockfd", sfd, arg, शून्य };
	पूर्णांक fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	अगर (fd == -1)
		वापस -1;
	snprintf(sfd, माप(sfd), "%d", fd);

	file = rc_service_resolve(service);
	अगर (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		जाओ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	अगर ((त्रुटि_सं = posix_spawn(&pid, file, शून्य, &chldmask, UNCONST(argv), environ))) {
		ख_लिखो(मानक_त्रुटि, "posix_spawn: %s\n", म_त्रुटि(त्रुटि_सं));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	बंद(fd);

out:
	मुक्त(file);
	वापस pid;
}

पूर्णांक
parse_mode(mode_t *mode, अक्षर *text)
{
	अक्षर *p;
	अचिन्हित दीर्घ l;

	/* Check for a numeric mode */
	अगर ((*text - '0') < 8) {
		l = म_से_अदीर्घ(text, &p, 8);
		अगर (*p || l > 07777U) {
			त्रुटि_सं = EINVAL;
			वापस -1;
		}
		*mode = (mode_t) l;
		वापस 0;
	}

	/* We currently don't check g+w type stuff */
	त्रुटि_सं = EINVAL;
	वापस -1;
}

RC_DEPTREE * _rc_deptree_load(पूर्णांक force, पूर्णांक *regen)
{
	पूर्णांक fd;
	पूर्णांक retval;
	पूर्णांक serrno = त्रुटि_सं;
	पूर्णांक merrno;
	समय_प्रकार t;
	अक्षर file[PATH_MAX];
	काष्ठा stat st;
	खाता *fp;

	t = 0;
	अगर (rc_deptree_update_needed(&t, file) || force != 0) {
		पूर्णांक svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = त्रुटि_सं;
		त्रुटि_सं = serrno;
		अगर (fd == -1 && merrno == EACCES)
			वापस rc_deptree_load();
		बंद(fd);

		अगर (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		अगर (retval == 0) {
			अगर (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), म_त्रुटि(त्रुटि_सं));
				वापस शून्य;
			}

			अगर (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				जाओ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), स_समय(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (काष्ठा timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			अगर ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				ख_लिखो(fp, "%s\n", file);
				ख_बंद(fp);
			}
		}

out:
		अगर (force == -1 && regen != शून्य)
			*regen = retval;
	}

	वापस rc_deptree_load();
}

अटल स्थिर काष्ठा {
	स्थिर अक्षर * स्थिर name;
	RC_SERVICE bit;
} service_bits[] = {
	{ "service_started",     RC_SERVICE_STARTED,     },
	{ "service_stopped",     RC_SERVICE_STOPPED,     },
	{ "service_inactive",    RC_SERVICE_INACTIVE,    },
	{ "service_starting",    RC_SERVICE_STARTING,    },
	{ "service_stopping",    RC_SERVICE_STOPPING,    },
	{ "service_hotplugged",  RC_SERVICE_HOTPLUGGED,  },
	{ "service_wasinactive", RC_SERVICE_WASINACTIVE, },
	{ "service_failed",      RC_SERVICE_FAILED,      },
	{ "service_crashed",     RC_SERVICE_CRASHED,     },
};

RC_SERVICE lookup_service_state(स्थिर अक्षर *service)
{
	माप_प्रकार i;
	क्रम (i = 0; i < ARRAY_आकार(service_bits); ++i)
		अगर (!म_भेद(service, service_bits[i].name))
			वापस service_bits[i].bit;
	वापस 0;
}

व्योम from_time_t(अक्षर *time_string, समय_प्रकार tv)
{
	स_माला(time_string, 20, "%Y-%m-%d %H:%M:%S", स_स्थानीय(&tv));
}

समय_प्रकार to_time_t(अक्षर *timestring)
{
	पूर्णांक check = 0;
	पूर्णांक year = 0;
	पूर्णांक month = 0;
	पूर्णांक day = 0;
	पूर्णांक hour = 0;
	पूर्णांक min = 0;
	पूर्णांक sec = 0;
	काष्ठा पंचांग breakdown = {0};
	समय_प्रकार परिणाम = -1;

	check = माला_पूछो(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	अगर (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		परिणाम = स_गढ़ो(&breakdown);
	}
	वापस परिणाम;
}

pid_t get_pid(स्थिर अक्षर *applet,स्थिर अक्षर *pidfile)
{
	खाता *fp;
	pid_t pid;

	अगर (!pidfile)
		वापस -1;

	अगर ((fp = ख_खोलो(pidfile, "r")) == शून्य) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
		वापस -1;
	}

	अगर (ख_पूछो(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		ख_बंद(fp);
		वापस -1;
	}

	ख_बंद(fp);

	वापस pid;
}

काष्ठा notify notify_parse(स्थिर अक्षर *applet, स्थिर अक्षर *notify_string)
{
	काष्ठा notify notify = {0};
	अगर (माला_पूछो(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		अगर (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, म_त्रुटि(त्रुटि_सं));
	} अन्यथा अगर (म_भेदन(notify_string, "socket", माप("socket") - 1) == 0) {
		जोड़ {
			काष्ठा sockaddr header;
			काष्ठा sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		पूर्णांक written;
		स्थिर अक्षर *opts = म_अक्षर(notify_string, ':');

		अगर (!opts || opts[1] == '\0')
			वापस notify;
		अगर (म_भेद(opts + 1, "ready") != 0)
			वापस notify;

		written = snprintf(addr.unix.sun_path, माप(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		अगर (written >= (पूर्णांक)माप(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		अगर ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, म_त्रुटि(त्रुटि_सं));
		अगर (bind(notify.fd, &addr.header, माप(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, म_त्रुटि(त्रुटि_सं));
	} अन्यथा {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	वापस notify;
}

bool notify_wait(स्थिर अक्षर *applet, काष्ठा notify notify)
{
	चयन (notify.type) {
	हाल NOTIFY_NONE:
		वापस true;
	हाल NOTIFY_FD:
		बंद(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		अवरोध;
	हाल NOTIFY_SOCKET:
		अवरोध;
	}

	क्रम (;;) {
		अक्षर buf[बफ_मान];
		ssize_t bytes = पढ़ो(notify.fd, buf, बफ_मान);

		अगर (bytes == 0)
			वापस false;
		अगर (bytes == -1) {
			अगर (त्रुटि_सं != EINTR) {
				eerror("%s: read failed '%s'\n", applet, म_त्रुटि(त्रुटि_सं));
				वापस false;
			}
			जारी;
		}

		चयन (notify.type) {
		हाल NOTIFY_NONE:
			अवरोध;
		हाल NOTIFY_FD:
			अगर (स_प्रथम(buf, '\n', bytes))
				वापस true;
			अवरोध;
		हाल NOTIFY_SOCKET:
			buf[bytes] = '\0';
			अगर (म_माला(buf, "READY=1")) {
				अक्षर *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				मुक्त(path);
				वापस true;
			}
			अवरोध;
		}
	}

	वापस true;
}

#अगर_अघोषित HAVE_CLOSE_RANGE
अटल अंतरभूत पूर्णांक close_range(अचिन्हित पूर्णांक first RC_UNUSED,
			      अचिन्हित पूर्णांक last RC_UNUSED,
			      अचिन्हित पूर्णांक flags RC_UNUSED)
{
#अगर_घोषित SYS_close_range
	वापस syscall(SYS_close_range, first, last, flags);
#अन्यथा
	त्रुटि_सं = ENOSYS;
	वापस -1;
#पूर्ण_अगर
}
#पूर्ण_अगर
#अगर_अघोषित CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#पूर्ण_अगर

व्योम
cloexec_fds_from(पूर्णांक first)
{
	पूर्णांक i;
	अगर (close_range(first, अच_पूर्णांक_उच्च, CLOSE_RANGE_CLOEXEC) < 0) {
		क्रम (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
