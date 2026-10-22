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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <fnmatch.h>
#अन्तर्भाव <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <spawn.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/utsname.h>
#अन्तर्भाव <sys/un.h>
#अन्तर्भाव <sys/socket.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <utime.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "version.h"
#अन्तर्भाव "helpers.h"

बाह्य अक्षर **environ;

bool
rc_conf_yesno(स्थिरांक अक्षर *setting)
{
	परत rc_yesno(rc_conf_मूल्य (setting));
}

स्थिर स्थिरांक अक्षर *स्थिरांक env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	रिक्त
};

स्थिर स्थिरांक अक्षर *स्थिरांक usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", रिक्त
};

स्थिर bool
env_allowed(स्थिरांक अक्षर *स्थिरांक list[], स्थिरांक अक्षर *मूल्य)
{
	साठी (माप_प्रकार i = 0; list[i]; i++)
		जर (fnmatch(list[i], मूल्य, FNM_NOESCAPE | FNM_PERIOD) == 0)
			परत true;
	परत false;
}

शून्य
env_filter(शून्य)
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
	जर (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		परत;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	मुक्त(profile_path);

	जर (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		मुक्त(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	पर्यंत (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = म_अक्षर(env->मूल्य, '=');
		जर (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		जर (env_allowed(env_allowlist, env->मूल्य))
			सुरूठेवा;
		जर (rc_is_user() && env_allowed(usrenv_allowlist, env->मूल्य))
			सुरूठेवा;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			जर (fnmatch(var->मूल्य, env->मूल्य, FNM_NOESCAPE | FNM_PERIOD) == 0)
				जा next;

		/* OK, not allowed! */
		unsetenv(env->मूल्य);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = म_अक्षर(env->मूल्य, '=');
		*e = '\0';
		जर (!दो_पर्या(env->मूल्य))
			setenv(env->मूल्य, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

अक्षर *
default_runlevel(शून्य)
{
	अक्षर *runlevel = rc_runlevel_get();

	जर (!runlevel)
		runlevel = xstrdup(rc_conf_मूल्य("rc_default_runlevel"));
	जर (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	परत runlevel;
}

शून्य
env_config(शून्य)
{
	माप_प्रकार pplen = म_माप(RC_PATH_PREFIX);
	अक्षर *path;
	अक्षर *p;
	अक्षर *e;
	माप_प्रकार l;
	संरचना utsname uts;
	खाता *fp;
	अक्षर *token;
	अक्षर *np;
	अक्षर *npp;
	अक्षर *tok;
	स्थिरांक अक्षर *sys = rc_sys();
	स्थिरांक अक्षर *svcdir = rc_svcdir();
	स्थिरांक अक्षर *स्थिरांक *init_path = rc_scriptdirs();
	अक्षर *buffer = रिक्त;
	अक्षर *tmpdir;
	माप_प्रकार size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = दो_पर्या("PATH");
	जर (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	अन्यथा जर (म_भेदन (RC_PATH_PREFIX, path, pplen) != 0) {
		l = म_माप(path) + pplen + 3;
		e = p = xmalloc(आकार(अक्षर) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		पर्यंत ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			पर्यंत ((tok = strsep(&npp, ":")))
				जर (म_भेद(tok, token) == 0)
					थांबा;
			जर (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			मुक्त (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		मुक्त(e);
	}

	जर (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} अन्यथा {
		स्थिरांक अक्षर *cache_home = दो_पर्या("XDG_CACHE_HOME");
		अक्षर *cachedir = रिक्त;

		जर (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		अन्यथा जर ((cache_home = दो_पर्या("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		जर (cachedir)
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

	fp = xopen_memstream(&path, &size);
	ख_माला_दो("RC_PATH=", fp);
	साठी (bool first = true; *init_path; init_path++, first = false) {
		जर (!first)
			ख_अक्षर_दो(':', fp);
		ख_माला_दो(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	मुक्त(e);
	मुक्त(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	जर (!rc_is_user() && (fp = ख_खोलो(RC_KRUNLEVEL, "r"))) {
		जर (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		अन्यथा
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		मुक्त(buffer);
		ख_बंद(fp);
	} अन्यथा {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	जर (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	जर (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	जर (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	जर (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	त्रुटि_सं = 0;
	जर ((!rc_conf_yesno("rc_color") && त्रुटि_सं == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

पूर्णांक
signal_setup(पूर्णांक sig, शून्य (*handler)(पूर्णांक))
{
	संरचना sigaction sa;

	स_रखो(&sa, 0, आकार (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	परत sigaction(sig, &sa, रिक्त);
}

पूर्णांक
svc_lock(स्थिरांक अक्षर *applet, bool ignore_lock_failure)
{
	पूर्णांक fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	जर (fd == -1)
		परत -1;
	जर (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		पूर्णांक saved_errno = त्रुटि_सं;
		जर (ignore_lock_failure) {
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
		परत -1;
	}
	परत fd;
}

पूर्णांक
svc_unlock(स्थिरांक अक्षर *applet, पूर्णांक fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	बंद(fd);
	परत -1;
}

pid_t
exec_service(स्थिरांक अक्षर *service, स्थिरांक अक्षर *arg)
{
	अक्षर *file, sfd[32];
	स्थिरांक अक्षर *argv[] = { service, "--lockfd", sfd, arg, रिक्त };
	पूर्णांक fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	जर (fd == -1)
		परत -1;
	snprintf(sfd, आकार(sfd), "%d", fd);

	file = rc_service_resolve(service);
	जर (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		जा out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	जर ((त्रुटि_सं = posix_spawn(&pid, file, रिक्त, &chldmask, UNCONST(argv), environ))) {
		ख_लिखो(मानक_त्रुटि, "posix_spawn: %s\n", म_त्रुटि(त्रुटि_सं));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	बंद(fd);

out:
	मुक्त(file);
	परत pid;
}

पूर्णांक
parse_mode(mode_t *mode, अक्षर *text)
{
	अक्षर *p;
	अचिन्हित दीर्घ l;

	/* Check for a numeric mode */
	जर ((*text - '0') < 8) {
		l = म_से_अदीर्घ(text, &p, 8);
		जर (*p || l > 07777U) {
			त्रुटि_सं = EINVAL;
			परत -1;
		}
		*mode = (mode_t) l;
		परत 0;
	}

	/* We currently don't check g+w type stuff */
	त्रुटि_सं = EINVAL;
	परत -1;
}

RC_DEPTREE * _rc_deptree_load(पूर्णांक force, पूर्णांक *regen)
{
	पूर्णांक fd;
	पूर्णांक retval;
	पूर्णांक serrno = त्रुटि_सं;
	पूर्णांक merrno;
	समय_प्रकार t;
	अक्षर file[PATH_MAX];
	संरचना stat st;
	खाता *fp;

	t = 0;
	जर (rc_deptree_update_needed(&t, file) || force != 0) {
		पूर्णांक svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = त्रुटि_सं;
		त्रुटि_सं = serrno;
		जर (fd == -1 && merrno == EACCES)
			परत rc_deptree_load();
		बंद(fd);

		जर (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		जर (retval == 0) {
			जर (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), म_त्रुटि(त्रुटि_सं));
				परत रिक्त;
			}

			जर (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				जा out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), स_समय(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (संरचना timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			जर ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				ख_लिखो(fp, "%s\n", file);
				ख_बंद(fp);
			}
		}

out:
		जर (force == -1 && regen != रिक्त)
			*regen = retval;
	}

	परत rc_deptree_load();
}

स्थिर स्थिरांक संरचना {
	स्थिरांक अक्षर * स्थिरांक name;
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

RC_SERVICE lookup_service_state(स्थिरांक अक्षर *service)
{
	माप_प्रकार i;
	साठी (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		जर (!म_भेद(service, service_bits[i].name))
			परत service_bits[i].bit;
	परत 0;
}

शून्य from_time_t(अक्षर *time_string, समय_प्रकार tv)
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
	संरचना पंचांग breakdown = {0};
	समय_प्रकार result = -1;

	check = माला_पूछो(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	जर (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = स_गढ़ो(&breakdown);
	}
	परत result;
}

pid_t get_pid(स्थिरांक अक्षर *applet,स्थिरांक अक्षर *pidfile)
{
	खाता *fp;
	pid_t pid;

	जर (!pidfile)
		परत -1;

	जर ((fp = ख_खोलो(pidfile, "r")) == रिक्त) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
		परत -1;
	}

	जर (ख_पूछो(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		ख_बंद(fp);
		परत -1;
	}

	ख_बंद(fp);

	परत pid;
}

संरचना notify notify_parse(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *notify_string)
{
	संरचना notify notify = {0};
	जर (माला_पूछो(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		जर (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, म_त्रुटि(त्रुटि_सं));
	} अन्यथा जर (म_भेदन(notify_string, "socket", आकार("socket") - 1) == 0) {
		संघ {
			संरचना sockaddr header;
			संरचना sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		पूर्णांक written;
		स्थिरांक अक्षर *opts = म_अक्षर(notify_string, ':');

		जर (!opts || opts[1] == '\0')
			परत notify;
		जर (म_भेद(opts + 1, "ready") != 0)
			परत notify;

		written = snprintf(addr.unix.sun_path, आकार(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		जर (written >= (पूर्णांक)आकार(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		जर ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, म_त्रुटि(त्रुटि_सं));
		जर (bind(notify.fd, &addr.header, आकार(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, म_त्रुटि(त्रुटि_सं));
	} अन्यथा {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	परत notify;
}

bool notify_wait(स्थिरांक अक्षर *applet, संरचना notify notify)
{
	निवड (notify.type) {
	स्थिती NOTIFY_NONE:
		परत true;
	स्थिती NOTIFY_FD:
		बंद(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		थांबा;
	स्थिती NOTIFY_SOCKET:
		थांबा;
	}

	साठी (;;) {
		अक्षर buf[बफ_मान];
		ssize_t bytes = पठ(notify.fd, buf, बफ_मान);

		जर (bytes == 0)
			परत false;
		जर (bytes == -1) {
			जर (त्रुटि_सं != EINTR) {
				eerror("%s: read failed '%s'\n", applet, म_त्रुटि(त्रुटि_सं));
				परत false;
			}
			सुरूठेवा;
		}

		निवड (notify.type) {
		स्थिती NOTIFY_NONE:
			थांबा;
		स्थिती NOTIFY_FD:
			जर (स_प्रथम(buf, '\n', bytes))
				परत true;
			थांबा;
		स्थिती NOTIFY_SOCKET:
			buf[bytes] = '\0';
			जर (म_माला(buf, "READY=1")) {
				अक्षर *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				मुक्त(path);
				परत true;
			}
			थांबा;
		}
	}

	परत true;
}

#ifndef HAVE_CLOSE_RANGE
स्थिर ओळीत पूर्णांक close_range(अचिन्हित पूर्णांक first RC_UNUSED,
			      अचिन्हित पूर्णांक last RC_UNUSED,
			      अचिन्हित पूर्णांक flags RC_UNUSED)
{
#ifdef SYS_close_range
	परत syscall(SYS_close_range, first, last, flags);
#अन्यथा
	त्रुटि_सं = ENOSYS;
	परत -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

शून्य
cloexec_fds_from(पूर्णांक first)
{
	पूर्णांक i;
	जर (close_range(first, अच_पूर्णांक_उच्च, CLOSE_RANGE_CLOEXEC) < 0) {
		साठी (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
