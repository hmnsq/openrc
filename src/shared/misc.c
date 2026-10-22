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
#यदि_परिभाषितम् HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#यदि_अन्तः
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <spawn.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/time.h>
#यदि_परिभाषितम् __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#यदि_अन्तः
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
rc_conf_yesno(ध्रुव वर्ण *setting)
{
	प्रत्यागम rc_yesno(rc_conf_मानम् (setting));
}

स्थिर ध्रुव वर्ण *ध्रुव env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	रिक्त
};

स्थिर ध्रुव वर्ण *ध्रुव usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", रिक्त
};

स्थिर bool
env_allowed(ध्रुव वर्ण *ध्रुव list[], ध्रुव वर्ण *मानम्)
{
	प्रति (size_t i = 0; list[i]; i++)
		यदि (fnmatch(list[i], मानम्, FNM_NOESCAPE | FNM_PERIOD) == 0)
			प्रत्यागम true;
	प्रत्यागम false;
}

शून्य
env_filter(शून्य)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	वर्ण *profile_मार्गः;
	RC_STRING *env;
	वर्ण *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_विभज(rc_conf_मानम्("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	यदि (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		प्रत्यागम;
	}

	xasprintf(&profile_मार्गः, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_आरोपय(profile_मार्गः);
	मुक्त(profile_मार्गः);

	यदि (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_मार्गः, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_आरोपय(profile_मार्गः);
		मुक्त(profile_मार्गः);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	यावत् (environ && environ[i]) {
		env = rc_stringlist_सङ्कलय(env_list, environ[i++]);
		e = strchr(env->मानम्, '=');
		यदि (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		यदि (env_allowed(env_allowlist, env->मानम्))
			अनुवर्त;
		यदि (rc_is_user() && env_allowed(usrenv_allowlist, env->मानम्))
			अनुवर्त;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			यदि (fnmatch(var->मानम्, env->मानम्, FNM_NOESCAPE | FNM_PERIOD) == 0)
				गच्छ अग्र;

		/* OK, not allowed! */
		unsetenv(env->मानम्);
अग्र:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->मानम्, '=');
		*e = '\0';
		यदि (!getenv(env->मानम्))
			setenv(env->मानम्, e + 1, 1);
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
		runlevel = xstrdup(rc_conf_मानम्("rc_default_runlevel"));
	यदि (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	प्रत्यागम runlevel;
}

शून्य
env_config(शून्य)
{
	size_t pplen = वाक्यदैर्घ्यम्(RC_मार्गः_उपसर्गः);
	वर्ण *मार्गः;
	वर्ण *p;
	वर्ण *e;
	size_t l;
	संरचना utsname uts;
	FILE *fp;
	वर्ण *token;
	वर्ण *np;
	वर्ण *npp;
	वर्ण *tok;
	ध्रुव वर्ण *sys = rc_sys();
	ध्रुव वर्ण *svcdir = rc_svcdir();
	ध्रुव वर्ण *ध्रुव *init_मार्गः = rc_scriptdirs();
	वर्ण *बफर = रिक्त;
	वर्ण *tmpdir;
	size_t आकार = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	मार्गः = getenv("PATH");
	यदि (!मार्गः)
		setenv("PATH", RC_मार्गः_उपसर्गः, 1);
	अन्यथा यदि (वाक्ये_तुलय (RC_मार्गः_उपसर्गः, मार्गः, pplen) != 0) {
		l = वाक्यदैर्घ्यम्(मार्गः) + pplen + 3;
		e = p = xmalloc(परिमाण(वर्ण) * l);
		p += snprintf(p, l, "%s", RC_मार्गः_उपसर्गः);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		यावत् ((token = strsep(&मार्गः, ":"))) {
			np = npp = xstrdup(RC_मार्गः_उपसर्गः);
			यावत् ((tok = strsep(&npp, ":")))
				यदि (वाक्यं_तुलय(tok, token) == 0)
					भङ्ग;
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
		ध्रुव वर्ण *cache_home = getenv("XDG_CACHE_HOME");
		वर्ण *cachedir = रिक्त;

		यदि (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		अन्यथा यदि ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		यदि (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		मुक्त(cachedir);
	}

	xasprintf(&tmpdir, "%s/tmp", svcdir);
	e = default_runlevel();

	setenv("RC_VERSION", संस्करणम्, 1);
	setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 1);
	setenv("RC_SVCDIR", svcdir, 1);
	setenv("RC_TMPDIR", tmpdir, 1);
	setenv("RC_BOOTLEVEL", RC_LEVEL_BOOT, 1);
	setenv("RC_RUNLEVEL", e, 1);

	fp = xopen_memstream(&मार्गः, &आकार);
	fputs("RC_PATH=", fp);
	प्रति (bool first = true; *init_मार्गः; init_मार्गः++, first = false) {
		यदि (!first)
			fputc(':', fp);
		fputs(*init_मार्गः, fp);
	}
	xclose_memstream(fp);
	putenv(मार्गः);

	मुक्त(e);
	मुक्त(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	यदि (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		यदि (xgetline(&बफर, &आकार, fp) != -1)
			setenv("RC_DEFAULTLEVEL", बफर, 1);
		अन्यथा
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		मुक्त(बफर);
		fclose(fp);
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

	errno = 0;
	यदि ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

पूर्णाङ्क
signal_सज्जता(पूर्णाङ्क sig, शून्य (*handler)(पूर्णाङ्क))
{
	संरचना sigaction sa;

	स्मृतिं_पूरय(&sa, 0, परिमाण (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	प्रत्यागम sigaction(sig, &sa, रिक्त);
}

पूर्णाङ्क
svc_तालकम्(ध्रुव वर्ण *applet, bool ignore_तालकम्_failure)
{
	पूर्णाङ्क fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	यदि (fd == -1)
		प्रत्यागम -1;
	यदि (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		पूर्णाङ्क saved_errno = errno;
		यदि (ignore_तालकम्_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			निर्गच्छ(EXIT_SUCCESS);
		}
		eerror("Call to flock failed: %s", strerror(errno));
		close(fd);
		errno = saved_errno;
		प्रत्यागम -1;
	}
	प्रत्यागम fd;
}

पूर्णाङ्क
svc_unlock(ध्रुव वर्ण *applet, पूर्णाङ्क fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	प्रत्यागम -1;
}

pid_t
exec_service(ध्रुव वर्ण *service, ध्रुव वर्ण *arg)
{
	वर्ण *file, sfd[32];
	ध्रुव वर्ण *argv[] = { service, "--lockfd", sfd, arg, रिक्त };
	पूर्णाङ्क fd = svc_तालकम्(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t रिक्तानि;
	pid_t pid = 0;

	यदि (fd == -1)
		प्रत्यागम -1;
	snprintf(sfd, परिमाण(sfd), "%d", fd);

	file = rc_service_resolve(service);
	यदि (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		गच्छ बहिर्गमनम्;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&रिक्तानि);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &रिक्तानि);

	यदि ((errno = posix_spawn(&pid, file, रिक्त, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

बहिर्गमनम्:
	मुक्त(file);
	प्रत्यागम pid;
}

पूर्णाङ्क
parse_बहुलकम्(mode_t *बहुलकम्, वर्ण *text)
{
	वर्ण *p;
	अचिह्नित दीर्घ l;

	/* Check for a numeric mode */
	यदि ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		यदि (*p || l > 07777U) {
			errno = EINVAL;
			प्रत्यागम -1;
		}
		*बहुलकम् = (mode_t) l;
		प्रत्यागम 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	प्रत्यागम -1;
}

RC_DEPTREE * rc_deptree_आरोपय_2(पूर्णाङ्क force, पूर्णाङ्क *regen)
{
	पूर्णाङ्क fd;
	पूर्णाङ्क retval;
	पूर्णाङ्क serrno = errno;
	पूर्णाङ्क merrno;
	time_t t;
	वर्ण file[PATH_MAX];
	संरचना stat st;
	FILE *fp;

	t = 0;
	यदि (rc_deptree_update_needed(&t, file) || force != 0) {
		पूर्णाङ्क svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		यदि (fd == -1 && merrno == EACCES)
			प्रत्यागम rc_deptree_आरोपय();
		close(fd);

		यदि (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		यदि (retval == 0) {
			यदि (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				प्रत्यागम रिक्त;
			}

			यदि (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				गच्छ बहिर्गमनम्;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (संरचना timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			यदि ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

बहिर्गमनम्:
		यदि (force == -1 && regen != रिक्त)
			*regen = retval;
	}

	प्रत्यागम rc_deptree_आरोपय();
}

स्थिर ध्रुव संरचना {
	ध्रुव वर्ण * ध्रुव name;
	RC_SERVICE bit;
} service_द्वयङ्काः[] = {
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

RC_SERVICE lookup_service_state(ध्रुव वर्ण *service)
{
	size_t i;
	प्रति (i = 0; i < ARRAY_आकार(service_द्वयङ्काः); ++i)
		यदि (!वाक्यं_तुलय(service, service_द्वयङ्काः[i].name))
			प्रत्यागम service_द्वयङ्काः[i].bit;
	प्रत्यागम 0;
}

शून्य from_time_t(वर्ण *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t गन्तव्यम्_time_t(वर्ण *timestring)
{
	पूर्णाङ्क check = 0;
	पूर्णाङ्क year = 0;
	पूर्णाङ्क month = 0;
	पूर्णाङ्क day = 0;
	पूर्णाङ्क hour = 0;
	पूर्णाङ्क min = 0;
	पूर्णाङ्क sec = 0;
	संरचना tm breakdown = {0};
	time_t परिणाम = -1;

	check = वाक्यात्_पठ(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	यदि (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		परिणाम = mktime(&breakdown);
	}
	प्रत्यागम परिणाम;
}

pid_t get_pid(ध्रुव वर्ण *applet,ध्रुव वर्ण *pidfile)
{
	FILE *fp;
	pid_t pid;

	यदि (!pidfile)
		प्रत्यागम -1;

	यदि ((fp = fopen(pidfile, "r")) == रिक्त) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		प्रत्यागम -1;
	}

	यदि (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		प्रत्यागम -1;
	}

	fclose(fp);

	प्रत्यागम pid;
}

संरचना notify notify_parse(ध्रुव वर्ण *applet, ध्रुव वर्ण *notify_string)
{
	संरचना notify notify = {0};
	यदि (वाक्यात्_पठ(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		यदि (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} अन्यथा यदि (वाक्ये_तुलय(notify_string, "socket", परिमाण("socket") - 1) == 0) {
		सङ्घ {
			संरचना sockaddr शीर्षकम्;
			संरचना sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		पूर्णाङ्क written;
		ध्रुव वर्ण *opts = strchr(notify_string, ':');

		यदि (!opts || opts[1] == '\0')
			प्रत्यागम notify;
		यदि (वाक्यं_तुलय(opts + 1, "ready") != 0)
			प्रत्यागम notify;

		written = snprintf(addr.unix.sun_path, परिमाण(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		यदि (written >= (पूर्णाङ्क)परिमाण(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		यदि ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		यदि (bind(notify.fd, &addr.शीर्षकम्, परिमाण(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} अन्यथा {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	प्रत्यागम notify;
}

bool notify_wait(ध्रुव वर्ण *applet, संरचना notify notify)
{
	विकल्प (notify.type) {
	पक्ष NOTIFY_NONE:
		प्रत्यागम true;
	पक्ष NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		भङ्ग;
	पक्ष NOTIFY_SOCKET:
		भङ्ग;
	}

	प्रति (;;) {
		वर्ण buf[BUFSIZ];
		ssize_t अष्टकानि = पठ(notify.fd, buf, BUFSIZ);

		यदि (अष्टकानि == 0)
			प्रत्यागम false;
		यदि (अष्टकानि == -1) {
			यदि (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				प्रत्यागम false;
			}
			अनुवर्त;
		}

		विकल्प (notify.type) {
		पक्ष NOTIFY_NONE:
			भङ्ग;
		पक्ष NOTIFY_FD:
			यदि (memchr(buf, '\n', अष्टकानि))
				प्रत्यागम true;
			भङ्ग;
		पक्ष NOTIFY_SOCKET:
			buf[अष्टकानि] = '\0';
			यदि (वाक्ये_अन्विष(buf, "READY=1")) {
				वर्ण *मार्गः;
				xasprintf(&मार्गः, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(मार्गः);
				मुक्त(मार्गः);
				प्रत्यागम true;
			}
			भङ्ग;
		}
	}

	प्रत्यागम true;
}

#यदि_अपरिभाषितम् HAVE_CLOSE_RANGE
स्थिर अन्तःस्थ पूर्णाङ्क close_range(अचिह्नित पूर्णाङ्क first RC_अप्रयुक्तम्,
			      अचिह्नित पूर्णाङ्क चरमः RC_अप्रयुक्तम्,
			      अचिह्नित पूर्णाङ्क flags RC_अप्रयुक्तम्)
{
#यदि_परिभाषितम् SYS_close_range
	प्रत्यागम syscall(SYS_close_range, first, चरमः, flags);
#अन्यथा
	errno = ENOSYS;
	प्रत्यागम -1;
#यदि_अन्तः
}
#यदि_अन्तः
#यदि_अपरिभाषितम् CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#यदि_अन्तः

शून्य
cloexec_fds_from(पूर्णाङ्क first)
{
	पूर्णाङ्क i;
	यदि (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		प्रति (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
