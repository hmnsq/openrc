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

#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <fnmatch.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <spawn.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>
#ಅನ್ತರ್ಭಾವ <sys/un.h>
#ಅನ್ತರ್ಭಾವ <sys/socket.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <utime.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "version.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಬಾಹ್ಯ ವರ್ಣ **environ;

bool
rc_conf_yesno(ಧ್ರುವ ವರ್ಣ *setting)
{
	ಪ್ರತ್ಯಾಗಮ rc_yesno(rc_conf_ಮೌಲ್ಯ (setting));
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ರಿಕ್ತ
};

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ರಿಕ್ತ
};

ಸ್ಥಿರ bool
env_allowed(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ list[], ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಪ್ರತಿ (size_t i = 0; list[i]; i++)
		ಯದಿ (fnmatch(list[i], ಮೌಲ್ಯ, FNM_NOESCAPE | FNM_PERIOD) == 0)
			ಪ್ರತ್ಯಾಗಮ true;
	ಪ್ರತ್ಯಾಗಮ false;
}

ಶೂನ್ಯ
env_filter(ಶೂನ್ಯ)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	ವರ್ಣ *profile_path;
	RC_STRING *env;
	ವರ್ಣ *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_ಮೌಲ್ಯ("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	ಯದಿ (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		ಪ್ರತ್ಯಾಗಮ;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	ಮುಕ್ತ(profile_path);

	ಯದಿ (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		ಮುಕ್ತ(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	ಯಾವತ್ (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->ಮೌಲ್ಯ, '=');
		ಯದಿ (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		ಯದಿ (env_allowed(env_allowlist, env->ಮೌಲ್ಯ))
			ಅನುವರ್ತ;
		ಯದಿ (rc_is_user() && env_allowed(usrenv_allowlist, env->ಮೌಲ್ಯ))
			ಅನುವರ್ತ;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			ಯದಿ (fnmatch(var->ಮೌಲ್ಯ, env->ಮೌಲ್ಯ, FNM_NOESCAPE | FNM_PERIOD) == 0)
				ಗಚ್ಛ next;

		/* OK, not allowed! */
		unsetenv(env->ಮೌಲ್ಯ);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->ಮೌಲ್ಯ, '=');
		*e = '\0';
		ಯದಿ (!getenv(env->ಮೌಲ್ಯ))
			setenv(env->ಮೌಲ್ಯ, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

ವರ್ಣ *
default_runlevel(ಶೂನ್ಯ)
{
	ವರ್ಣ *runlevel = rc_runlevel_get();

	ಯದಿ (!runlevel)
		runlevel = xstrdup(rc_conf_ಮೌಲ್ಯ("rc_default_runlevel"));
	ಯದಿ (!runlevel || !*runlevel) {
		ಮುಕ್ತ(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	ಪ್ರತ್ಯಾಗಮ runlevel;
}

ಶೂನ್ಯ
env_config(ಶೂನ್ಯ)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	ವರ್ಣ *path;
	ವರ್ಣ *p;
	ವರ್ಣ *e;
	size_t l;
	ಸಂರಚನಾ utsname uts;
	FILE *fp;
	ವರ್ಣ *token;
	ವರ್ಣ *np;
	ವರ್ಣ *npp;
	ವರ್ಣ *tok;
	ಧ್ರುವ ವರ್ಣ *sys = rc_sys();
	ಧ್ರುವ ವರ್ಣ *svcdir = rc_svcdir();
	ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *init_path = rc_scriptdirs();
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	ವರ್ಣ *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	ಯದಿ (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	ಅನ್ಯಥಾ ಯದಿ (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(ಪರಿಮಾಣ(ವರ್ಣ) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		ಯಾವತ್ ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			ಯಾವತ್ ((tok = strsep(&npp, ":")))
				ಯದಿ (strcmp(tok, token) == 0)
					ಭಙ್ಗ;
			ಯದಿ (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			ಮುಕ್ತ (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		ಮುಕ್ತ(e);
	}

	ಯದಿ (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} ಅನ್ಯಥಾ {
		ಧ್ರುವ ವರ್ಣ *cache_home = getenv("XDG_CACHE_HOME");
		ವರ್ಣ *cachedir = ರಿಕ್ತ;

		ಯದಿ (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		ಅನ್ಯಥಾ ಯದಿ ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		ಯದಿ (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		ಮುಕ್ತ(cachedir);
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
	fputs("RC_PATH=", fp);
	ಪ್ರತಿ (bool first = true; *init_path; init_path++, first = false) {
		ಯದಿ (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	ಮುಕ್ತ(e);
	ಮುಕ್ತ(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	ಯದಿ (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		ಯದಿ (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		ಅನ್ಯಥಾ
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		ಮುಕ್ತ(buffer);
		fclose(fp);
	} ಅನ್ಯಥಾ {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	ಯದಿ (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	ಯದಿ (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	ಯದಿ (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	ಯದಿ (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	ಯದಿ ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

ಪೂರ್ಣಾಙ್ಕ
signal_setup(ಪೂರ್ಣಾಙ್ಕ sig, ಶೂನ್ಯ (*handler)(ಪೂರ್ಣಾಙ್ಕ))
{
	ಸಂರಚನಾ sigaction sa;

	memset(&sa, 0, ಪರಿಮಾಣ (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	ಪ್ರತ್ಯಾಗಮ sigaction(sig, &sa, ರಿಕ್ತ);
}

ಪೂರ್ಣಾಙ್ಕ
svc_lock(ಧ್ರುವ ವರ್ಣ *applet, bool ignore_lock_failure)
{
	ಪೂರ್ಣಾಙ್ಕ fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	ಯದಿ (fd == -1)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		ಪೂರ್ಣಾಙ್ಕ saved_errno = errno;
		ಯದಿ (ignore_lock_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			exit(EXIT_SUCCESS);
		}
		eerror("Call to flock failed: %s", strerror(errno));
		close(fd);
		errno = saved_errno;
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಪ್ರತ್ಯಾಗಮ fd;
}

ಪೂರ್ಣಾಙ್ಕ
svc_unlock(ಧ್ರುವ ವರ್ಣ *applet, ಪೂರ್ಣಾಙ್ಕ fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	ಪ್ರತ್ಯಾಗಮ -1;
}

pid_t
exec_service(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *arg)
{
	ವರ್ಣ *file, sfd[32];
	ಧ್ರುವ ವರ್ಣ *argv[] = { service, "--lockfd", sfd, arg, ರಿಕ್ತ };
	ಪೂರ್ಣಾಙ್ಕ fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	ಯದಿ (fd == -1)
		ಪ್ರತ್ಯಾಗಮ -1;
	snprintf(sfd, ಪರಿಮಾಣ(sfd), "%d", fd);

	file = rc_service_resolve(service);
	ಯದಿ (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		ಗಚ್ಛ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	ಯದಿ ((errno = posix_spawn(&pid, file, ರಿಕ್ತ, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	ಮುಕ್ತ(file);
	ಪ್ರತ್ಯಾಗಮ pid;
}

ಪೂರ್ಣಾಙ್ಕ
parse_mode(mode_t *mode, ವರ್ಣ *text)
{
	ವರ್ಣ *p;
	unsigned long l;

	/* Check for a numeric mode */
	ಯದಿ ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		ಯದಿ (*p || l > 07777U) {
			errno = EINVAL;
			ಪ್ರತ್ಯಾಗಮ -1;
		}
		*mode = (mode_t) l;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	ಪ್ರತ್ಯಾಗಮ -1;
}

RC_DEPTREE * _rc_deptree_load(ಪೂರ್ಣಾಙ್ಕ force, ಪೂರ್ಣಾಙ್ಕ *regen)
{
	ಪೂರ್ಣಾಙ್ಕ fd;
	ಪೂರ್ಣಾಙ್ಕ retval;
	ಪೂರ್ಣಾಙ್ಕ serrno = errno;
	ಪೂರ್ಣಾಙ್ಕ merrno;
	time_t t;
	ವರ್ಣ file[PATH_MAX];
	ಸಂರಚನಾ stat st;
	FILE *fp;

	t = 0;
	ಯದಿ (rc_deptree_update_needed(&t, file) || force != 0) {
		ಪೂರ್ಣಾಙ್ಕ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		ಯದಿ (fd == -1 && merrno == EACCES)
			ಪ್ರತ್ಯಾಗಮ rc_deptree_load();
		close(fd);

		ಯದಿ (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		ಯದಿ (retval == 0) {
			ಯದಿ (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
			}

			ಯದಿ (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				ಗಚ್ಛ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ಸಂರಚನಾ timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			ಯದಿ ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		ಯದಿ (force == -1 && regen != ರಿಕ್ತ)
			*regen = retval;
	}

	ಪ್ರತ್ಯಾಗಮ rc_deptree_load();
}

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
	ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ name;
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

RC_SERVICE lookup_service_state(ಧ್ರುವ ವರ್ಣ *service)
{
	size_t i;
	ಪ್ರತಿ (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		ಯದಿ (!strcmp(service, service_bits[i].name))
			ಪ್ರತ್ಯಾಗಮ service_bits[i].bit;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ from_time_t(ವರ್ಣ *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(ವರ್ಣ *timestring)
{
	ಪೂರ್ಣಾಙ್ಕ check = 0;
	ಪೂರ್ಣಾಙ್ಕ year = 0;
	ಪೂರ್ಣಾಙ್ಕ month = 0;
	ಪೂರ್ಣಾಙ್ಕ day = 0;
	ಪೂರ್ಣಾಙ್ಕ hour = 0;
	ಪೂರ್ಣಾಙ್ಕ min = 0;
	ಪೂರ್ಣಾಙ್ಕ sec = 0;
	ಸಂರಚನಾ tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	ಯದಿ (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	ಪ್ರತ್ಯಾಗಮ result;
}

pid_t get_pid(ಧ್ರುವ ವರ್ಣ *applet,ಧ್ರುವ ವರ್ಣ *pidfile)
{
	FILE *fp;
	pid_t pid;

	ಯದಿ (!pidfile)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ ((fp = fopen(pidfile, "r")) == ರಿಕ್ತ) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	ಯದಿ (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ pid;
}

ಸಂರಚನಾ notify notify_parse(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *notify_string)
{
	ಸಂರಚನಾ notify notify = {0};
	ಯದಿ (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		ಯದಿ (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} ಅನ್ಯಥಾ ಯದಿ (strncmp(notify_string, "socket", ಪರಿಮಾಣ("socket") - 1) == 0) {
		ಸಙ್ಘ {
			ಸಂರಚನಾ sockaddr header;
			ಸಂರಚನಾ sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		ಪೂರ್ಣಾಙ್ಕ written;
		ಧ್ರುವ ವರ್ಣ *opts = strchr(notify_string, ':');

		ಯದಿ (!opts || opts[1] == '\0')
			ಪ್ರತ್ಯಾಗಮ notify;
		ಯದಿ (strcmp(opts + 1, "ready") != 0)
			ಪ್ರತ್ಯಾಗಮ notify;

		written = snprintf(addr.unix.sun_path, ಪರಿಮಾಣ(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		ಯದಿ (written >= (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		ಯದಿ ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		ಯದಿ (bind(notify.fd, &addr.header, ಪರಿಮಾಣ(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} ಅನ್ಯಥಾ {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	ಪ್ರತ್ಯಾಗಮ notify;
}

bool notify_wait(ಧ್ರುವ ವರ್ಣ *applet, ಸಂರಚನಾ notify notify)
{
	ವಿಕಲ್ಪ (notify.type) {
	ಪಕ್ಷ NOTIFY_NONE:
		ಪ್ರತ್ಯಾಗಮ true;
	ಪಕ್ಷ NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		ಭಙ್ಗ;
	ಪಕ್ಷ NOTIFY_SOCKET:
		ಭಙ್ಗ;
	}

	ಪ್ರತಿ (;;) {
		ವರ್ಣ buf[BUFSIZ];
		ssize_t bytes = ಪಠ(notify.fd, buf, BUFSIZ);

		ಯದಿ (bytes == 0)
			ಪ್ರತ್ಯಾಗಮ false;
		ಯದಿ (bytes == -1) {
			ಯದಿ (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				ಪ್ರತ್ಯಾಗಮ false;
			}
			ಅನುವರ್ತ;
		}

		ವಿಕಲ್ಪ (notify.type) {
		ಪಕ್ಷ NOTIFY_NONE:
			ಭಙ್ಗ;
		ಪಕ್ಷ NOTIFY_FD:
			ಯದಿ (memchr(buf, '\n', bytes))
				ಪ್ರತ್ಯಾಗಮ true;
			ಭಙ್ಗ;
		ಪಕ್ಷ NOTIFY_SOCKET:
			buf[bytes] = '\0';
			ಯದಿ (strstr(buf, "READY=1")) {
				ವರ್ಣ *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				ಮುಕ್ತ(path);
				ಪ್ರತ್ಯಾಗಮ true;
			}
			ಭಙ್ಗ;
		}
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

#ifndef HAVE_CLOSE_RANGE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ close_range(unsigned ಪೂರ್ಣಾಙ್ಕ first RC_UNUSED,
			      unsigned ಪೂರ್ಣಾಙ್ಕ last RC_UNUSED,
			      unsigned ಪೂರ್ಣಾಙ್ಕ flags RC_UNUSED)
{
#ifdef SYS_close_range
	ಪ್ರತ್ಯಾಗಮ syscall(SYS_close_range, first, last, flags);
#ಅನ್ಯಥಾ
	errno = ENOSYS;
	ಪ್ರತ್ಯಾಗಮ -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

ಶೂನ್ಯ
cloexec_fds_from(ಪೂರ್ಣಾಙ್ಕ first)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಯದಿ (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		ಪ್ರತಿ (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
