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

#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <fnmatch.h>
#అన్తర్భావ <limits.h>
#నిర్వచితమైతే HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#అయితే_ముగింపు
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <spawn.h>
#అన్తర్భావ <sys/file.h>
#అన్తర్భావ <sys/time.h>
#నిర్వచితమైతే __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#అయితే_ముగింపు
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/utsname.h>
#అన్తర్భావ <sys/un.h>
#అన్తర్భావ <sys/socket.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <utime.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "version.h"
#అన్తర్భావ "helpers.h"

బాహ్య అక్షరం **environ;

bool
rc_conf_yesno(స్థిరరాశి అక్షరం *setting)
{
	తిరిగి rc_yesno(rc_conf_విలువ (setting));
}

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ఖాళీ
};

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ఖాళీ
};

స్థిర bool
env_allowed(స్థిరరాశి అక్షరం *స్థిరరాశి list[], స్థిరరాశి అక్షరం *విలువ)
{
	కొరకు (size_t i = 0; list[i]; i++)
		ఒకవేళ (fnmatch(list[i], విలువ, FNM_NOESCAPE | FNM_PERIOD) == 0)
			తిరిగి true;
	తిరిగి false;
}

శూన్య
env_filter(శూన్య)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	అక్షరం *profile_మార్గం;
	RC_STRING *env;
	అక్షరం *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_విభజించు(rc_conf_విలువ("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	ఒకవేళ (rc_stringlist_కనుగొను(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		తిరిగి;
	}

	xasprintf(&profile_మార్గం, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_ఎక్కించు(profile_మార్గం);
	విడుదల(profile_మార్గం);

	ఒకవేళ (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_మార్గం, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_ఎక్కించు(profile_మార్గం);
		విడుదల(profile_మార్గం);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	అయితే (environ && environ[i]) {
		env = rc_stringlist_కూడు(env_list, environ[i++]);
		e = strchr(env->విలువ, '=');
		ఒకవేళ (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		ఒకవేళ (env_allowed(env_allowlist, env->విలువ))
			కొనసాగు;
		ఒకవేళ (rc_is_user() && env_allowed(usrenv_allowlist, env->విలువ))
			కొనసాగు;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			ఒకవేళ (fnmatch(var->విలువ, env->విలువ, FNM_NOESCAPE | FNM_PERIOD) == 0)
				వెళ్ళు తదుపరి;

		/* OK, not allowed! */
		unsetenv(env->విలువ);
తదుపరి:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->విలువ, '=');
		*e = '\0';
		ఒకవేళ (!getenv(env->విలువ))
			setenv(env->విలువ, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

అక్షరం *
default_runlevel(శూన్య)
{
	అక్షరం *runlevel = rc_runlevel_get();

	ఒకవేళ (!runlevel)
		runlevel = xstrdup(rc_conf_విలువ("rc_default_runlevel"));
	ఒకవేళ (!runlevel || !*runlevel) {
		విడుదల(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	తిరిగి runlevel;
}

శూన్య
env_config(శూన్య)
{
	size_t pplen = అక్షరశ్రేణి_పొడవు(RC_మార్గం_ఉపసర్గ);
	అక్షరం *మార్గం;
	అక్షరం *p;
	అక్షరం *e;
	size_t l;
	నిర్మాణం utsname uts;
	FILE *fp;
	అక్షరం *token;
	అక్షరం *np;
	అక్షరం *npp;
	అక్షరం *tok;
	స్థిరరాశి అక్షరం *sys = rc_sys();
	స్థిరరాశి అక్షరం *svcdir = rc_svcdir();
	స్థిరరాశి అక్షరం *స్థిరరాశి *init_మార్గం = rc_scriptdirs();
	అక్షరం *బఫరు = ఖాళీ;
	అక్షరం *tmpdir;
	size_t కొలత = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	మార్గం = getenv("PATH");
	ఒకవేళ (!మార్గం)
		setenv("PATH", RC_మార్గం_ఉపసర్గ, 1);
	లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక (RC_మార్గం_ఉపసర్గ, మార్గం, pplen) != 0) {
		l = అక్షరశ్రేణి_పొడవు(మార్గం) + pplen + 3;
		e = p = xmalloc(పరిమాణం(అక్షరం) * l);
		p += snprintf(p, l, "%s", RC_మార్గం_ఉపసర్గ);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		అయితే ((token = strsep(&మార్గం, ":"))) {
			np = npp = xstrdup(RC_మార్గం_ఉపసర్గ);
			అయితే ((tok = strsep(&npp, ":")))
				ఒకవేళ (strcmp(tok, token) == 0)
					ఆపు;
			ఒకవేళ (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			విడుదల (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		విడుదల(e);
	}

	ఒకవేళ (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} లేకపోతే {
		స్థిరరాశి అక్షరం *cache_home = getenv("XDG_CACHE_HOME");
		అక్షరం *cachedir = ఖాళీ;

		ఒకవేళ (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		లేకపోతే ఒకవేళ ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		ఒకవేళ (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		విడుదల(cachedir);
	}

	xasprintf(&tmpdir, "%s/tmp", svcdir);
	e = default_runlevel();

	setenv("RC_VERSION", సంస్కరణ, 1);
	setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 1);
	setenv("RC_SVCDIR", svcdir, 1);
	setenv("RC_TMPDIR", tmpdir, 1);
	setenv("RC_BOOTLEVEL", RC_LEVEL_BOOT, 1);
	setenv("RC_RUNLEVEL", e, 1);

	fp = xopen_memstream(&మార్గం, &కొలత);
	fputs("RC_PATH=", fp);
	కొరకు (bool మొదటి = true; *init_మార్గం; init_మార్గం++, మొదటి = false) {
		ఒకవేళ (!మొదటి)
			fputc(':', fp);
		fputs(*init_మార్గం, fp);
	}
	xclose_memstream(fp);
	putenv(మార్గం);

	విడుదల(e);
	విడుదల(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	ఒకవేళ (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		ఒకవేళ (xgetline(&బఫరు, &కొలత, fp) != -1)
			setenv("RC_DEFAULTLEVEL", బఫరు, 1);
		లేకపోతే
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		విడుదల(బఫరు);
		fclose(fp);
	} లేకపోతే {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	ఒకవేళ (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	ఒకవేళ (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	ఒకవేళ (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	ఒకవేళ (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	ఒకవేళ ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

పూర్ణాంకం
signal_అమరిక(పూర్ణాంకం sig, శూన్య (*handler)(పూర్ణాంకం))
{
	నిర్మాణం sigaction sa;

	memset(&sa, 0, పరిమాణం (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	తిరిగి sigaction(sig, &sa, ఖాళీ);
}

పూర్ణాంకం
svc_తాళం(స్థిరరాశి అక్షరం *applet, bool ignore_తాళం_failure)
{
	పూర్ణాంకం fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	ఒకవేళ (fd == -1)
		తిరిగి -1;
	ఒకవేళ (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		పూర్ణాంకం saved_errno = errno;
		ఒకవేళ (ignore_తాళం_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			నిష్క్రమించు(EXIT_SUCCESS);
		}
		eerror("Call to flock failed: %s", strerror(errno));
		close(fd);
		errno = saved_errno;
		తిరిగి -1;
	}
	తిరిగి fd;
}

పూర్ణాంకం
svc_unlock(స్థిరరాశి అక్షరం *applet, పూర్ణాంకం fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	తిరిగి -1;
}

pid_t
exec_service(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *arg)
{
	అక్షరం *file, sfd[32];
	స్థిరరాశి అక్షరం *argv[] = { service, "--lockfd", sfd, arg, ఖాళీ };
	పూర్ణాంకం fd = svc_తాళం(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t ఖాళీలు;
	pid_t pid = 0;

	ఒకవేళ (fd == -1)
		తిరిగి -1;
	snprintf(sfd, పరిమాణం(sfd), "%d", fd);

	file = rc_service_resolve(service);
	ఒకవేళ (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		వెళ్ళు బహిర్గతం;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&ఖాళీలు);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &ఖాళీలు);

	ఒకవేళ ((errno = posix_spawn(&pid, file, ఖాళీ, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

బహిర్గతం:
	విడుదల(file);
	తిరిగి pid;
}

పూర్ణాంకం
parse_బహుళకం(mode_t *బహుళకం, అక్షరం *పాఠ్యం)
{
	అక్షరం *p;
	గుర్తులేని దీర్ఘ l;

	/* Check for a numeric mode */
	ఒకవేళ ((*పాఠ్యం - '0') < 8) {
		l = strtoul(పాఠ్యం, &p, 8);
		ఒకవేళ (*p || l > 07777U) {
			errno = EINVAL;
			తిరిగి -1;
		}
		*బహుళకం = (mode_t) l;
		తిరిగి 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	తిరిగి -1;
}

RC_DEPTREE * rc_deptree_ఎక్కించు_2(పూర్ణాంకం force, పూర్ణాంకం *regen)
{
	పూర్ణాంకం fd;
	పూర్ణాంకం retval;
	పూర్ణాంకం serrno = errno;
	పూర్ణాంకం merrno;
	time_t t;
	అక్షరం file[PATH_MAX];
	నిర్మాణం stat st;
	FILE *fp;

	t = 0;
	ఒకవేళ (rc_deptree_update_needed(&t, file) || force != 0) {
		పూర్ణాంకం svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		ఒకవేళ (fd == -1 && merrno == EACCES)
			తిరిగి rc_deptree_ఎక్కించు();
		close(fd);

		ఒకవేళ (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		ఒకవేళ (retval == 0) {
			ఒకవేళ (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				తిరిగి ఖాళీ;
			}

			ఒకవేళ (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				వెళ్ళు బహిర్గతం;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (నిర్మాణం timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			ఒకవేళ ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

బహిర్గతం:
		ఒకవేళ (force == -1 && regen != ఖాళీ)
			*regen = retval;
	}

	తిరిగి rc_deptree_ఎక్కించు();
}

స్థిర స్థిరరాశి నిర్మాణం {
	స్థిరరాశి అక్షరం * స్థిరరాశి name;
	RC_SERVICE బిట్టు;
} service_బిట్లు[] = {
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

RC_SERVICE lookup_service_state(స్థిరరాశి అక్షరం *service)
{
	size_t i;
	కొరకు (i = 0; i < ARRAY_కొలత(service_బిట్లు); ++i)
		ఒకవేళ (!strcmp(service, service_బిట్లు[i].name))
			తిరిగి service_బిట్లు[i].బిట్టు;
	తిరిగి 0;
}

శూన్య from_సమయం_t(అక్షరం *సమయం_string, time_t tv)
{
	strftime(సమయం_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t గమ్యం_సమయం_t(అక్షరం *timestring)
{
	పూర్ణాంకం check = 0;
	పూర్ణాంకం year = 0;
	పూర్ణాంకం month = 0;
	పూర్ణాంకం day = 0;
	పూర్ణాంకం hour = 0;
	పూర్ణాంకం min = 0;
	పూర్ణాంకం sec = 0;
	నిర్మాణం tm breakdown = {0};
	time_t ఫలితం = -1;

	check = అక్షరశ్రేణి_చదువు(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	ఒకవేళ (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		ఫలితం = mktime(&breakdown);
	}
	తిరిగి ఫలితం;
}

pid_t get_pid(స్థిరరాశి అక్షరం *applet,స్థిరరాశి అక్షరం *pidfile)
{
	FILE *fp;
	pid_t pid;

	ఒకవేళ (!pidfile)
		తిరిగి -1;

	ఒకవేళ ((fp = fopen(pidfile, "r")) == ఖాళీ) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		తిరిగి -1;
	}

	ఒకవేళ (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		తిరిగి -1;
	}

	fclose(fp);

	తిరిగి pid;
}

నిర్మాణం notify notify_parse(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *notify_string)
{
	నిర్మాణం notify notify = {0};
	ఒకవేళ (అక్షరశ్రేణి_చదువు(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		ఒకవేళ (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} లేకపోతే ఒకవేళ (అక్షరశ్రేణి_పోలిక(notify_string, "socket", పరిమాణం("socket") - 1) == 0) {
		సంఘం {
			నిర్మాణం sockaddr శీర్షిక;
			నిర్మాణం sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		పూర్ణాంకం written;
		స్థిరరాశి అక్షరం *opts = strchr(notify_string, ':');

		ఒకవేళ (!opts || opts[1] == '\0')
			తిరిగి notify;
		ఒకవేళ (strcmp(opts + 1, "ready") != 0)
			తిరిగి notify;

		written = snprintf(addr.unix.sun_path, పరిమాణం(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		ఒకవేళ (written >= (పూర్ణాంకం)పరిమాణం(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		ఒకవేళ ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		ఒకవేళ (bind(notify.fd, &addr.శీర్షిక, పరిమాణం(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} లేకపోతే {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	తిరిగి notify;
}

bool notify_wait(స్థిరరాశి అక్షరం *applet, నిర్మాణం notify notify)
{
	ఎంపిక (notify.type) {
	స్థితి NOTIFY_NONE:
		తిరిగి true;
	స్థితి NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		ఆపు;
	స్థితి NOTIFY_SOCKET:
		ఆపు;
	}

	కొరకు (;;) {
		అక్షరం buf[BUFSIZ];
		ssize_t బైట్లు = పఠ(notify.fd, buf, BUFSIZ);

		ఒకవేళ (బైట్లు == 0)
			తిరిగి false;
		ఒకవేళ (బైట్లు == -1) {
			ఒకవేళ (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				తిరిగి false;
			}
			కొనసాగు;
		}

		ఎంపిక (notify.type) {
		స్థితి NOTIFY_NONE:
			ఆపు;
		స్థితి NOTIFY_FD:
			ఒకవేళ (memchr(buf, '\n', బైట్లు))
				తిరిగి true;
			ఆపు;
		స్థితి NOTIFY_SOCKET:
			buf[బైట్లు] = '\0';
			ఒకవేళ (అక్షరశ్రేణి_శోధన(buf, "READY=1")) {
				అక్షరం *మార్గం;
				xasprintf(&మార్గం, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(మార్గం);
				విడుదల(మార్గం);
				తిరిగి true;
			}
			ఆపు;
		}
	}

	తిరిగి true;
}

#నిర్వచితం_కాకపోతే HAVE_CLOSE_RANGE
స్థిర అంతఃవరుస పూర్ణాంకం close_range(గుర్తులేని పూర్ణాంకం మొదటి RC_వాడని,
			      గుర్తులేని పూర్ణాంకం చివరి RC_వాడని,
			      గుర్తులేని పూర్ణాంకం flags RC_వాడని)
{
#నిర్వచితమైతే SYS_close_range
	తిరిగి syscall(SYS_close_range, మొదటి, చివరి, flags);
#లేకపోతే
	errno = ENOSYS;
	తిరిగి -1;
#అయితే_ముగింపు
}
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#అయితే_ముగింపు

శూన్య
cloexec_fds_from(పూర్ణాంకం మొదటి)
{
	పూర్ణాంకం i;
	ఒకవేళ (close_range(మొదటి, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		కొరకు (i = getdtablesize() - 1; i >= మొదటి; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
