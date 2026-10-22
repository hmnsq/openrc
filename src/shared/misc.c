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

बाह्य वर्ण **environ;

bool
rc_conf_yesno(अचर वर्ण *setting)
{
	फर्काऊ rc_yesno(rc_conf_मान (setting));
}

स्थिर अचर वर्ण *अचर env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	रिक्त
};

स्थिर अचर वर्ण *अचर usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", रिक्त
};

स्थिर bool
env_allowed(अचर वर्ण *अचर list[], अचर वर्ण *मान)
{
	लागि (माप_प्रकार i = 0; list[i]; i++)
		यदि (fnmatch(list[i], मान, FNM_NOESCAPE | FNM_PERIOD) == 0)
			फर्काऊ true;
	फर्काऊ false;
}

शून्य
env_filter(शून्य)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	वर्ण *profile_path;
	RC_STRING *env;
	वर्ण *e;
	माप_प्रकार i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_मान("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	यदि (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		फर्काऊ;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	मुक्त(profile_path);

	यदि (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		मुक्त(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	जबसम्म (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = म_अक्षर(env->मान, '=');
		यदि (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		यदि (env_allowed(env_allowlist, env->मान))
			जारी;
		यदि (rc_is_user() && env_allowed(usrenv_allowlist, env->मान))
			जारी;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			यदि (fnmatch(var->मान, env->मान, FNM_NOESCAPE | FNM_PERIOD) == 0)
				गच्छ next;

		/* OK, not allowed! */
		unsetenv(env->मान);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = म_अक्षर(env->मान, '=');
		*e = '\0';
		यदि (!दो_पर्या(env->मान))
			setenv(env->मान, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

वर्ण *
default_runlevel(शून्य)
{
	वर्ण *runlevel = rc_runlevel_get();

	यदि (!runlevel)
		runlevel = xstrdup(rc_conf_मान("rc_default_runlevel"));
	यदि (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	फर्काऊ runlevel;
}

शून्य
env_config(शून्य)
{
	माप_प्रकार pplen = म_माप(RC_PATH_PREFIX);
	वर्ण *path;
	वर्ण *p;
	वर्ण *e;
	माप_प्रकार l;
	संरचना utsname uts;
	खाता *fp;
	वर्ण *token;
	वर्ण *np;
	वर्ण *npp;
	वर्ण *tok;
	अचर वर्ण *sys = rc_sys();
	अचर वर्ण *svcdir = rc_svcdir();
	अचर वर्ण *अचर *init_path = rc_scriptdirs();
	वर्ण *buffer = रिक्त;
	वर्ण *tmpdir;
	माप_प्रकार size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = दो_पर्या("PATH");
	यदि (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	अन्यथा यदि (म_भेदन (RC_PATH_PREFIX, path, pplen) != 0) {
		l = म_माप(path) + pplen + 3;
		e = p = xmalloc(परिमाण(वर्ण) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		जबसम्म ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			जबसम्म ((tok = strsep(&npp, ":")))
				यदि (म_भेद(tok, token) == 0)
					रोक;
			यदि (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			मुक्त (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		मुक्त(e);
	}

	यदि (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} अन्यथा {
		अचर वर्ण *cache_home = दो_पर्या("XDG_CACHE_HOME");
		वर्ण *cachedir = रिक्त;

		यदि (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		अन्यथा यदि ((cache_home = दो_पर्या("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		यदि (cachedir)
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
	लागि (bool first = true; *init_path; init_path++, first = false) {
		यदि (!first)
			ख_अक्षर_दो(':', fp);
		ख_माला_दो(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	मुक्त(e);
	मुक्त(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	यदि (!rc_is_user() && (fp = ख_खोलो(RC_KRUNLEVEL, "r"))) {
		यदि (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		अन्यथा
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		मुक्त(buffer);
		ख_बंद(fp);
	} अन्यथा {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	यदि (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	यदि (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	यदि (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	यदि (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	त्रुटि_सं = 0;
	यदि ((!rc_conf_yesno("rc_color") && त्रुटि_सं == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

पूर्णाङ्क
signal_setup(पूर्णाङ्क sig, शून्य (*handler)(पूर्णाङ्क))
{
	संरचना sigaction sa;

	स_रखो(&sa, 0, परिमाण (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	फर्काऊ sigaction(sig, &sa, रिक्त);
}

पूर्णाङ्क
svc_lock(अचर वर्ण *applet, bool ignore_lock_failure)
{
	पूर्णाङ्क fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	यदि (fd == -1)
		फर्काऊ -1;
	यदि (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		पूर्णाङ्क saved_errno = त्रुटि_सं;
		यदि (ignore_lock_failure) {
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
		फर्काऊ -1;
	}
	फर्काऊ fd;
}

पूर्णाङ्क
svc_unlock(अचर वर्ण *applet, पूर्णाङ्क fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	बंद(fd);
	फर्काऊ -1;
}

pid_t
exec_service(अचर वर्ण *service, अचर वर्ण *arg)
{
	वर्ण *file, sfd[32];
	अचर वर्ण *argv[] = { service, "--lockfd", sfd, arg, रिक्त };
	पूर्णाङ्क fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	यदि (fd == -1)
		फर्काऊ -1;
	snprintf(sfd, परिमाण(sfd), "%d", fd);

	file = rc_service_resolve(service);
	यदि (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		गच्छ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	यदि ((त्रुटि_सं = posix_spawn(&pid, file, रिक्त, &chldmask, UNCONST(argv), environ))) {
		ख_लिखो(मानक_त्रुटि, "posix_spawn: %s\n", म_त्रुटि(त्रुटि_सं));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	बंद(fd);

out:
	मुक्त(file);
	फर्काऊ pid;
}

पूर्णाङ्क
parse_mode(mode_t *mode, वर्ण *text)
{
	वर्ण *p;
	अचिन्हित दीर्घ l;

	/* Check for a numeric mode */
	यदि ((*text - '0') < 8) {
		l = म_से_अदीर्घ(text, &p, 8);
		यदि (*p || l > 07777U) {
			त्रुटि_सं = EINVAL;
			फर्काऊ -1;
		}
		*mode = (mode_t) l;
		फर्काऊ 0;
	}

	/* We currently don't check g+w type stuff */
	त्रुटि_सं = EINVAL;
	फर्काऊ -1;
}

RC_DEPTREE * _rc_deptree_load(पूर्णाङ्क force, पूर्णाङ्क *regen)
{
	पूर्णाङ्क fd;
	पूर्णाङ्क retval;
	पूर्णाङ्क serrno = त्रुटि_सं;
	पूर्णाङ्क merrno;
	समय_प्रकार t;
	वर्ण file[PATH_MAX];
	संरचना stat st;
	खाता *fp;

	t = 0;
	यदि (rc_deptree_update_needed(&t, file) || force != 0) {
		पूर्णाङ्क svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = त्रुटि_सं;
		त्रुटि_सं = serrno;
		यदि (fd == -1 && merrno == EACCES)
			फर्काऊ rc_deptree_load();
		बंद(fd);

		यदि (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		यदि (retval == 0) {
			यदि (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), म_त्रुटि(त्रुटि_सं));
				फर्काऊ रिक्त;
			}

			यदि (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				गच्छ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), स_समय(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (संरचना timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			यदि ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				ख_लिखो(fp, "%s\n", file);
				ख_बंद(fp);
			}
		}

out:
		यदि (force == -1 && regen != रिक्त)
			*regen = retval;
	}

	फर्काऊ rc_deptree_load();
}

स्थिर अचर संरचना {
	अचर वर्ण * अचर name;
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

RC_SERVICE lookup_service_state(अचर वर्ण *service)
{
	माप_प्रकार i;
	लागि (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		यदि (!म_भेद(service, service_bits[i].name))
			फर्काऊ service_bits[i].bit;
	फर्काऊ 0;
}

शून्य from_time_t(वर्ण *time_string, समय_प्रकार tv)
{
	स_माला(time_string, 20, "%Y-%m-%d %H:%M:%S", स_स्थानीय(&tv));
}

समय_प्रकार to_time_t(वर्ण *timestring)
{
	पूर्णाङ्क check = 0;
	पूर्णाङ्क year = 0;
	पूर्णाङ्क month = 0;
	पूर्णाङ्क day = 0;
	पूर्णाङ्क hour = 0;
	पूर्णाङ्क min = 0;
	पूर्णाङ्क sec = 0;
	संरचना पंचांग breakdown = {0};
	समय_प्रकार result = -1;

	check = माला_पूछो(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	यदि (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = स_गढ़ो(&breakdown);
	}
	फर्काऊ result;
}

pid_t get_pid(अचर वर्ण *applet,अचर वर्ण *pidfile)
{
	खाता *fp;
	pid_t pid;

	यदि (!pidfile)
		फर्काऊ -1;

	यदि ((fp = ख_खोलो(pidfile, "r")) == रिक्त) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, म_त्रुटि(त्रुटि_सं));
		फर्काऊ -1;
	}

	यदि (ख_पूछो(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		ख_बंद(fp);
		फर्काऊ -1;
	}

	ख_बंद(fp);

	फर्काऊ pid;
}

संरचना notify notify_parse(अचर वर्ण *applet, अचर वर्ण *notify_string)
{
	संरचना notify notify = {0};
	यदि (माला_पूछो(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		यदि (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, म_त्रुटि(त्रुटि_सं));
	} अन्यथा यदि (म_भेदन(notify_string, "socket", परिमाण("socket") - 1) == 0) {
		सङ्घ {
			संरचना sockaddr header;
			संरचना sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		पूर्णाङ्क written;
		अचर वर्ण *opts = म_अक्षर(notify_string, ':');

		यदि (!opts || opts[1] == '\0')
			फर्काऊ notify;
		यदि (म_भेद(opts + 1, "ready") != 0)
			फर्काऊ notify;

		written = snprintf(addr.unix.sun_path, परिमाण(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		यदि (written >= (पूर्णाङ्क)परिमाण(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		यदि ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, म_त्रुटि(त्रुटि_सं));
		यदि (bind(notify.fd, &addr.header, परिमाण(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, म_त्रुटि(त्रुटि_सं));
	} अन्यथा {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	फर्काऊ notify;
}

bool notify_wait(अचर वर्ण *applet, संरचना notify notify)
{
	विकल्प (notify.type) {
	पक्ष NOTIFY_NONE:
		फर्काऊ true;
	पक्ष NOTIFY_FD:
		बंद(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		रोक;
	पक्ष NOTIFY_SOCKET:
		रोक;
	}

	लागि (;;) {
		वर्ण buf[बफ_मान];
		ssize_t bytes = पठ(notify.fd, buf, बफ_मान);

		यदि (bytes == 0)
			फर्काऊ false;
		यदि (bytes == -1) {
			यदि (त्रुटि_सं != EINTR) {
				eerror("%s: read failed '%s'\n", applet, म_त्रुटि(त्रुटि_सं));
				फर्काऊ false;
			}
			जारी;
		}

		विकल्प (notify.type) {
		पक्ष NOTIFY_NONE:
			रोक;
		पक्ष NOTIFY_FD:
			यदि (स_प्रथम(buf, '\n', bytes))
				फर्काऊ true;
			रोक;
		पक्ष NOTIFY_SOCKET:
			buf[bytes] = '\0';
			यदि (म_माला(buf, "READY=1")) {
				वर्ण *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				मुक्त(path);
				फर्काऊ true;
			}
			रोक;
		}
	}

	फर्काऊ true;
}

#ifndef HAVE_CLOSE_RANGE
स्थिर अन्तःस्थ पूर्णाङ्क close_range(अचिन्हित पूर्णाङ्क first RC_UNUSED,
			      अचिन्हित पूर्णाङ्क last RC_UNUSED,
			      अचिन्हित पूर्णाङ्क flags RC_UNUSED)
{
#ifdef SYS_close_range
	फर्काऊ syscall(SYS_close_range, first, last, flags);
#अन्यथा
	त्रुटि_सं = ENOSYS;
	फर्काऊ -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

शून्य
cloexec_fds_from(पूर्णाङ्क first)
{
	पूर्णाङ्क i;
	यदि (close_range(first, अच_पूर्णांक_उच्च, CLOSE_RANGE_CLOEXEC) < 0) {
		लागि (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
