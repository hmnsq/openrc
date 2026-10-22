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

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fnmatch.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spawn.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/file.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/un.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/socket.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <utime.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "version.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਬਾਹਰੀ ਅੱਖਰ **environ;

bool
rc_conf_yesno(ਅਚਲ ਅੱਖਰ *setting)
{
	ਵਾਪਸ rc_yesno(rc_conf_ਮੁੱਲ (setting));
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ਨਿਰਾ
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ਨਿਰਾ
};

ਸਥਿਰ bool
env_allowed(ਅਚਲ ਅੱਖਰ *ਅਚਲ list[], ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਲਈ (size_t i = 0; list[i]; i++)
		ਜੇ (fnmatch(list[i], ਮੁੱਲ, FNM_NOESCAPE | FNM_PERIOD) == 0)
			ਵਾਪਸ true;
	ਵਾਪਸ false;
}

ਖਾਲੀ
env_filter(ਖਾਲੀ)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	ਅੱਖਰ *profile_ਰਸਤਾ;
	RC_STRING *env;
	ਅੱਖਰ *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_ਵੰਡੋ(rc_conf_ਮੁੱਲ("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	ਜੇ (rc_stringlist_ਲੱਭੋ(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		ਵਾਪਸ;
	}

	xasprintf(&profile_ਰਸਤਾ, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_ਲੱਦੋ(profile_ਰਸਤਾ);
	ਮੁਕਤ(profile_ਰਸਤਾ);

	ਜੇ (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_ਰਸਤਾ, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_ਲੱਦੋ(profile_ਰਸਤਾ);
		ਮੁਕਤ(profile_ਰਸਤਾ);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	ਜਦੋਂ (environ && environ[i]) {
		env = rc_stringlist_ਜੋੜੋ(env_list, environ[i++]);
		e = strchr(env->ਮੁੱਲ, '=');
		ਜੇ (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		ਜੇ (env_allowed(env_allowlist, env->ਮੁੱਲ))
			ਜਾਰੀ;
		ਜੇ (rc_is_user() && env_allowed(usrenv_allowlist, env->ਮੁੱਲ))
			ਜਾਰੀ;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			ਜੇ (fnmatch(var->ਮੁੱਲ, env->ਮੁੱਲ, FNM_NOESCAPE | FNM_PERIOD) == 0)
				ਜਾਓ ਅਗਲਾ;

		/* OK, not allowed! */
		unsetenv(env->ਮੁੱਲ);
ਅਗਲਾ:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->ਮੁੱਲ, '=');
		*e = '\0';
		ਜੇ (!getenv(env->ਮੁੱਲ))
			setenv(env->ਮੁੱਲ, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

ਅੱਖਰ *
default_runlevel(ਖਾਲੀ)
{
	ਅੱਖਰ *runlevel = rc_runlevel_get();

	ਜੇ (!runlevel)
		runlevel = xstrdup(rc_conf_ਮੁੱਲ("rc_default_runlevel"));
	ਜੇ (!runlevel || !*runlevel) {
		ਮੁਕਤ(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	ਵਾਪਸ runlevel;
}

ਖਾਲੀ
env_config(ਖਾਲੀ)
{
	size_t pplen = ਲੜੀ_ਲੰਬਾਈ(RC_ਰਸਤਾ_ਅਗੇਤਰ);
	ਅੱਖਰ *ਰਸਤਾ;
	ਅੱਖਰ *p;
	ਅੱਖਰ *e;
	size_t l;
	ਬਣਤਰ utsname uts;
	FILE *fp;
	ਅੱਖਰ *token;
	ਅੱਖਰ *np;
	ਅੱਖਰ *npp;
	ਅੱਖਰ *tok;
	ਅਚਲ ਅੱਖਰ *sys = rc_sys();
	ਅਚਲ ਅੱਖਰ *svcdir = rc_svcdir();
	ਅਚਲ ਅੱਖਰ *ਅਚਲ *init_ਰਸਤਾ = rc_scriptdirs();
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	ਅੱਖਰ *tmpdir;
	size_t ਮਾਪ = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	ਰਸਤਾ = getenv("PATH");
	ਜੇ (!ਰਸਤਾ)
		setenv("PATH", RC_ਰਸਤਾ_ਅਗੇਤਰ, 1);
	ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ (RC_ਰਸਤਾ_ਅਗੇਤਰ, ਰਸਤਾ, pplen) != 0) {
		l = ਲੜੀ_ਲੰਬਾਈ(ਰਸਤਾ) + pplen + 3;
		e = p = xmalloc(ਆਕਾਰ(ਅੱਖਰ) * l);
		p += snprintf(p, l, "%s", RC_ਰਸਤਾ_ਅਗੇਤਰ);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		ਜਦੋਂ ((token = strsep(&ਰਸਤਾ, ":"))) {
			np = npp = xstrdup(RC_ਰਸਤਾ_ਅਗੇਤਰ);
			ਜਦੋਂ ((tok = strsep(&npp, ":")))
				ਜੇ (strcmp(tok, token) == 0)
					ਰੋਕੋ;
			ਜੇ (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			ਮੁਕਤ (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		ਮੁਕਤ(e);
	}

	ਜੇ (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} ਨਹੀਂਤਾਂ {
		ਅਚਲ ਅੱਖਰ *cache_home = getenv("XDG_CACHE_HOME");
		ਅੱਖਰ *cachedir = ਨਿਰਾ;

		ਜੇ (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		ਨਹੀਂਤਾਂ ਜੇ ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		ਜੇ (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		ਮੁਕਤ(cachedir);
	}

	xasprintf(&tmpdir, "%s/tmp", svcdir);
	e = default_runlevel();

	setenv("RC_VERSION", ਸੰਸਕਰਣ, 1);
	setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 1);
	setenv("RC_SVCDIR", svcdir, 1);
	setenv("RC_TMPDIR", tmpdir, 1);
	setenv("RC_BOOTLEVEL", RC_LEVEL_BOOT, 1);
	setenv("RC_RUNLEVEL", e, 1);

	fp = xopen_memstream(&ਰਸਤਾ, &ਮਾਪ);
	fputs("RC_PATH=", fp);
	ਲਈ (bool ਪਹਿਲਾ = true; *init_ਰਸਤਾ; init_ਰਸਤਾ++, ਪਹਿਲਾ = false) {
		ਜੇ (!ਪਹਿਲਾ)
			fputc(':', fp);
		fputs(*init_ਰਸਤਾ, fp);
	}
	xclose_memstream(fp);
	putenv(ਰਸਤਾ);

	ਮੁਕਤ(e);
	ਮੁਕਤ(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	ਜੇ (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		ਜੇ (xgetline(&ਬਫਰ, &ਮਾਪ, fp) != -1)
			setenv("RC_DEFAULTLEVEL", ਬਫਰ, 1);
		ਨਹੀਂਤਾਂ
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		ਮੁਕਤ(ਬਫਰ);
		fclose(fp);
	} ਨਹੀਂਤਾਂ {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	ਜੇ (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	ਜੇ (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	ਜੇ (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	ਜੇ (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	ਜੇ ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

ਪੂਰਨ
signal_ਤਿਆਰੀ(ਪੂਰਨ sig, ਖਾਲੀ (*handler)(ਪੂਰਨ))
{
	ਬਣਤਰ sigaction sa;

	memset(&sa, 0, ਆਕਾਰ (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	ਵਾਪਸ sigaction(sig, &sa, ਨਿਰਾ);
}

ਪੂਰਨ
svc_ਤਾਲਾ(ਅਚਲ ਅੱਖਰ *applet, bool ignore_ਤਾਲਾ_failure)
{
	ਪੂਰਨ fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	ਜੇ (fd == -1)
		ਵਾਪਸ -1;
	ਜੇ (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		ਪੂਰਨ saved_errno = errno;
		ਜੇ (ignore_ਤਾਲਾ_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
		}
		eerror("Call to flock failed: %s", strerror(errno));
		close(fd);
		errno = saved_errno;
		ਵਾਪਸ -1;
	}
	ਵਾਪਸ fd;
}

ਪੂਰਨ
svc_unlock(ਅਚਲ ਅੱਖਰ *applet, ਪੂਰਨ fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	ਵਾਪਸ -1;
}

pid_t
exec_service(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *arg)
{
	ਅੱਖਰ *file, sfd[32];
	ਅਚਲ ਅੱਖਰ *argv[] = { service, "--lockfd", sfd, arg, ਨਿਰਾ };
	ਪੂਰਨ fd = svc_ਤਾਲਾ(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t ਖਾਲੀ_ਥਾਵਾਂ;
	pid_t pid = 0;

	ਜੇ (fd == -1)
		ਵਾਪਸ -1;
	snprintf(sfd, ਆਕਾਰ(sfd), "%d", fd);

	file = rc_service_resolve(service);
	ਜੇ (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&ਖਾਲੀ_ਥਾਵਾਂ);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &ਖਾਲੀ_ਥਾਵਾਂ);

	ਜੇ ((errno = posix_spawn(&pid, file, ਨਿਰਾ, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

ਬਾਹਰਲਾ:
	ਮੁਕਤ(file);
	ਵਾਪਸ pid;
}

ਪੂਰਨ
parse_ਬਹੁਲਕ(mode_t *ਬਹੁਲਕ, ਅੱਖਰ *ਪਾਠ)
{
	ਅੱਖਰ *p;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ l;

	/* Check for a numeric mode */
	ਜੇ ((*ਪਾਠ - '0') < 8) {
		l = strtoul(ਪਾਠ, &p, 8);
		ਜੇ (*p || l > 07777U) {
			errno = EINVAL;
			ਵਾਪਸ -1;
		}
		*ਬਹੁਲਕ = (mode_t) l;
		ਵਾਪਸ 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	ਵਾਪਸ -1;
}

RC_DEPTREE * rc_deptree_ਲੱਦੋ_2(ਪੂਰਨ force, ਪੂਰਨ *regen)
{
	ਪੂਰਨ fd;
	ਪੂਰਨ retval;
	ਪੂਰਨ serrno = errno;
	ਪੂਰਨ merrno;
	time_t t;
	ਅੱਖਰ file[PATH_MAX];
	ਬਣਤਰ stat st;
	FILE *fp;

	t = 0;
	ਜੇ (rc_deptree_update_needed(&t, file) || force != 0) {
		ਪੂਰਨ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		ਜੇ (fd == -1 && merrno == EACCES)
			ਵਾਪਸ rc_deptree_ਲੱਦੋ();
		close(fd);

		ਜੇ (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		ਜੇ (retval == 0) {
			ਜੇ (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				ਵਾਪਸ ਨਿਰਾ;
			}

			ਜੇ (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				ਜਾਓ ਬਾਹਰਲਾ;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ਬਣਤਰ timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			ਜੇ ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

ਬਾਹਰਲਾ:
		ਜੇ (force == -1 && regen != ਨਿਰਾ)
			*regen = retval;
	}

	ਵਾਪਸ rc_deptree_ਲੱਦੋ();
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
	ਅਚਲ ਅੱਖਰ * ਅਚਲ name;
	RC_SERVICE ਦੋਆਧਾਰੀ_ਅੰਕ;
} service_ਬਿੱਟ[] = {
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

RC_SERVICE lookup_service_state(ਅਚਲ ਅੱਖਰ *service)
{
	size_t i;
	ਲਈ (i = 0; i < ARRAY_ਮਾਪ(service_ਬਿੱਟ); ++i)
		ਜੇ (!strcmp(service, service_ਬਿੱਟ[i].name))
			ਵਾਪਸ service_ਬਿੱਟ[i].ਦੋਆਧਾਰੀ_ਅੰਕ;
	ਵਾਪਸ 0;
}

ਖਾਲੀ from_ਸਮਾਂ_t(ਅੱਖਰ *ਸਮਾਂ_string, time_t tv)
{
	strftime(ਸਮਾਂ_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t ਮੰਜ਼ਿਲ_ਸਮਾਂ_t(ਅੱਖਰ *timestring)
{
	ਪੂਰਨ check = 0;
	ਪੂਰਨ year = 0;
	ਪੂਰਨ month = 0;
	ਪੂਰਨ day = 0;
	ਪੂਰਨ hour = 0;
	ਪੂਰਨ min = 0;
	ਪੂਰਨ sec = 0;
	ਬਣਤਰ tm breakdown = {0};
	time_t ਨਤੀਜਾ = -1;

	check = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	ਜੇ (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		ਨਤੀਜਾ = mktime(&breakdown);
	}
	ਵਾਪਸ ਨਤੀਜਾ;
}

pid_t get_pid(ਅਚਲ ਅੱਖਰ *applet,ਅਚਲ ਅੱਖਰ *pidfile)
{
	FILE *fp;
	pid_t pid;

	ਜੇ (!pidfile)
		ਵਾਪਸ -1;

	ਜੇ ((fp = fopen(pidfile, "r")) == ਨਿਰਾ) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		ਵਾਪਸ -1;
	}

	ਜੇ (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		ਵਾਪਸ -1;
	}

	fclose(fp);

	ਵਾਪਸ pid;
}

ਬਣਤਰ notify notify_parse(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *notify_string)
{
	ਬਣਤਰ notify notify = {0};
	ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		ਜੇ (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੁਲਨਾ(notify_string, "socket", ਆਕਾਰ("socket") - 1) == 0) {
		ਸੰਘ {
			ਬਣਤਰ sockaddr ਸਿਰਲੇਖ;
			ਬਣਤਰ sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		ਪੂਰਨ written;
		ਅਚਲ ਅੱਖਰ *opts = strchr(notify_string, ':');

		ਜੇ (!opts || opts[1] == '\0')
			ਵਾਪਸ notify;
		ਜੇ (strcmp(opts + 1, "ready") != 0)
			ਵਾਪਸ notify;

		written = snprintf(addr.unix.sun_path, ਆਕਾਰ(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		ਜੇ (written >= (ਪੂਰਨ)ਆਕਾਰ(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		ਜੇ ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		ਜੇ (bind(notify.fd, &addr.ਸਿਰਲੇਖ, ਆਕਾਰ(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} ਨਹੀਂਤਾਂ {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	ਵਾਪਸ notify;
}

bool notify_wait(ਅਚਲ ਅੱਖਰ *applet, ਬਣਤਰ notify notify)
{
	ਚੋਣ (notify.type) {
	ਹਾਲਤ NOTIFY_NONE:
		ਵਾਪਸ true;
	ਹਾਲਤ NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		ਰੋਕੋ;
	ਹਾਲਤ NOTIFY_SOCKET:
		ਰੋਕੋ;
	}

	ਲਈ (;;) {
		ਅੱਖਰ buf[BUFSIZ];
		ssize_t ਬਾਈਟਾਂ = ਪਠ(notify.fd, buf, BUFSIZ);

		ਜੇ (ਬਾਈਟਾਂ == 0)
			ਵਾਪਸ false;
		ਜੇ (ਬਾਈਟਾਂ == -1) {
			ਜੇ (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				ਵਾਪਸ false;
			}
			ਜਾਰੀ;
		}

		ਚੋਣ (notify.type) {
		ਹਾਲਤ NOTIFY_NONE:
			ਰੋਕੋ;
		ਹਾਲਤ NOTIFY_FD:
			ਜੇ (memchr(buf, '\n', ਬਾਈਟਾਂ))
				ਵਾਪਸ true;
			ਰੋਕੋ;
		ਹਾਲਤ NOTIFY_SOCKET:
			buf[ਬਾਈਟਾਂ] = '\0';
			ਜੇ (ਲੜੀ_ਖੋਜ(buf, "READY=1")) {
				ਅੱਖਰ *ਰਸਤਾ;
				xasprintf(&ਰਸਤਾ, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(ਰਸਤਾ);
				ਮੁਕਤ(ਰਸਤਾ);
				ਵਾਪਸ true;
			}
			ਰੋਕੋ;
		}
	}

	ਵਾਪਸ true;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_CLOSE_RANGE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ close_range(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਪਹਿਲਾ RC_ਅਣਵਰਤਿਆ,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਆਖਰੀ RC_ਅਣਵਰਤਿਆ,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags RC_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS_close_range
	ਵਾਪਸ syscall(SYS_close_range, ਪਹਿਲਾ, ਆਖਰੀ, flags);
#ਨਹੀਂਤਾਂ
	errno = ENOSYS;
	ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#ਜੇ_ਅੰਤ

ਖਾਲੀ
cloexec_fds_from(ਪੂਰਨ ਪਹਿਲਾ)
{
	ਪੂਰਨ i;
	ਜੇ (close_range(ਪਹਿਲਾ, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		ਲਈ (i = getdtablesize() - 1; i >= ਪਹਿਲਾ; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
