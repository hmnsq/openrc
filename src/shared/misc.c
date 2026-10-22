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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <spawn.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#ಆದರೆ_ಅಂತ್ಯ
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

ಬಾಹ್ಯ ಅಕ್ಷರ **environ;

bool
rc_conf_yesno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *setting)
{
	ಹಿಂತಿರುಗಿ rc_yesno(rc_conf_ಮೌಲ್ಯ (setting));
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ಖಾಲಿ
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ಖಾಲಿ
};

ಸ್ಥಿರ bool
env_allowed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ list[], ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ಗಾಗಿ (size_t i = 0; list[i]; i++)
		ಆದರೆ (fnmatch(list[i], ಮೌಲ್ಯ, FNM_NOESCAPE | FNM_PERIOD) == 0)
			ಹಿಂತಿರುಗಿ true;
	ಹಿಂತಿರುಗಿ false;
}

ಶೂನ್ಯ
env_filter(ಶೂನ್ಯ)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	ಅಕ್ಷರ *profile_ಮಾರ್ಗ;
	RC_STRING *env;
	ಅಕ್ಷರ *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_ವಿಭಜಿಸು(rc_conf_ಮೌಲ್ಯ("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		ಹಿಂತಿರುಗಿ;
	}

	xasprintf(&profile_ಮಾರ್ಗ, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_ತುಂಬು(profile_ಮಾರ್ಗ);
	ಬಿಡುಗಡೆ(profile_ಮಾರ್ಗ);

	ಆದರೆ (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_ಮಾರ್ಗ, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_ತುಂಬು(profile_ಮಾರ್ಗ);
		ಬಿಡುಗಡೆ(profile_ಮಾರ್ಗ);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	ಆಗ (environ && environ[i]) {
		env = rc_stringlist_ಕೂಡಿಸು(env_list, environ[i++]);
		e = strchr(env->ಮೌಲ್ಯ, '=');
		ಆದರೆ (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		ಆದರೆ (env_allowed(env_allowlist, env->ಮೌಲ್ಯ))
			ಮುಂದುವರಿ;
		ಆದರೆ (rc_is_user() && env_allowed(usrenv_allowlist, env->ಮೌಲ್ಯ))
			ಮುಂದುವರಿ;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			ಆದರೆ (fnmatch(var->ಮೌಲ್ಯ, env->ಮೌಲ್ಯ, FNM_NOESCAPE | FNM_PERIOD) == 0)
				ಹೋಗು ಮುಂದಿನ;

		/* OK, not allowed! */
		unsetenv(env->ಮೌಲ್ಯ);
ಮುಂದಿನ:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->ಮೌಲ್ಯ, '=');
		*e = '\0';
		ಆದರೆ (!getenv(env->ಮೌಲ್ಯ))
			setenv(env->ಮೌಲ್ಯ, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

ಅಕ್ಷರ *
default_runlevel(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *runlevel = rc_runlevel_get();

	ಆದರೆ (!runlevel)
		runlevel = xstrdup(rc_conf_ಮೌಲ್ಯ("rc_default_runlevel"));
	ಆದರೆ (!runlevel || !*runlevel) {
		ಬಿಡುಗಡೆ(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	ಹಿಂತಿರುಗಿ runlevel;
}

ಶೂನ್ಯ
env_config(ಶೂನ್ಯ)
{
	size_t pplen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(RC_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಅಕ್ಷರ *ಮಾರ್ಗ;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *e;
	size_t l;
	ರಚನೆ utsname uts;
	FILE *fp;
	ಅಕ್ಷರ *token;
	ಅಕ್ಷರ *np;
	ಅಕ್ಷರ *npp;
	ಅಕ್ಷರ *tok;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sys = rc_sys();
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svcdir = rc_svcdir();
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *init_ಮಾರ್ಗ = rc_scriptdirs();
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	ಅಕ್ಷರ *tmpdir;
	size_t ಗಾತ್ರ_2 = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	ಮಾರ್ಗ = getenv("PATH");
	ಆದರೆ (!ಮಾರ್ಗ)
		setenv("PATH", RC_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ, 1);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ (RC_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ, ಮಾರ್ಗ, pplen) != 0) {
		l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಾರ್ಗ) + pplen + 3;
		e = p = xmalloc(ಗಾತ್ರ(ಅಕ್ಷರ) * l);
		p += snprintf(p, l, "%s", RC_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		ಆಗ ((token = strsep(&ಮಾರ್ಗ, ":"))) {
			np = npp = xstrdup(RC_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ);
			ಆಗ ((tok = strsep(&npp, ":")))
				ಆದರೆ (strcmp(tok, token) == 0)
					ನಿಲ್ಲಿಸು;
			ಆದರೆ (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			ಬಿಡುಗಡೆ (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		ಬಿಡುಗಡೆ(e);
	}

	ಆದರೆ (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cache_home = getenv("XDG_CACHE_HOME");
		ಅಕ್ಷರ *cachedir = ಖಾಲಿ;

		ಆದರೆ (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		ಆದರೆ (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		ಬಿಡುಗಡೆ(cachedir);
	}

	xasprintf(&tmpdir, "%s/tmp", svcdir);
	e = default_runlevel();

	setenv("RC_VERSION", ಆವೃತ್ತಿ, 1);
	setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 1);
	setenv("RC_SVCDIR", svcdir, 1);
	setenv("RC_TMPDIR", tmpdir, 1);
	setenv("RC_BOOTLEVEL", RC_LEVEL_BOOT, 1);
	setenv("RC_RUNLEVEL", e, 1);

	fp = xopen_memstream(&ಮಾರ್ಗ, &ಗಾತ್ರ_2);
	fputs("RC_PATH=", fp);
	ಗಾಗಿ (bool ಮೊದಲ = true; *init_ಮಾರ್ಗ; init_ಮಾರ್ಗ++, ಮೊದಲ = false) {
		ಆದರೆ (!ಮೊದಲ)
			fputc(':', fp);
		fputs(*init_ಮಾರ್ಗ, fp);
	}
	xclose_memstream(fp);
	putenv(ಮಾರ್ಗ);

	ಬಿಡುಗಡೆ(e);
	ಬಿಡುಗಡೆ(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	ಆದರೆ (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		ಆದರೆ (xgetline(&ಬಫರ್, &ಗಾತ್ರ_2, fp) != -1)
			setenv("RC_DEFAULTLEVEL", ಬಫರ್, 1);
		ಇಲ್ಲದಿದ್ದರೆ
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		ಬಿಡುಗಡೆ(ಬಫರ್);
		fclose(fp);
	} ಇಲ್ಲದಿದ್ದರೆ {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	ಆದರೆ (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	ಆದರೆ (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	ಆದರೆ (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	ಆದರೆ (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	ಆದರೆ ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

ಪೂರ್ಣಾಂಕ
signal_ಸಿದ್ಧತೆ(ಪೂರ್ಣಾಂಕ sig, ಶೂನ್ಯ (*handler)(ಪೂರ್ಣಾಂಕ))
{
	ರಚನೆ sigaction sa;

	memset(&sa, 0, ಗಾತ್ರ (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	ಹಿಂತಿರುಗಿ sigaction(sig, &sa, ಖಾಲಿ);
}

ಪೂರ್ಣಾಂಕ
svc_ಬೀಗ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, bool ignore_ಬೀಗ_failure)
{
	ಪೂರ್ಣಾಂಕ fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	ಆದರೆ (fd == -1)
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		ಪೂರ್ಣಾಂಕ saved_errno = errno;
		ಆದರೆ (ignore_ಬೀಗ_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
		}
		eerror("Call to flock failed: %s", strerror(errno));
		close(fd);
		errno = saved_errno;
		ಹಿಂತಿರುಗಿ -1;
	}
	ಹಿಂತಿರುಗಿ fd;
}

ಪೂರ್ಣಾಂಕ
svc_unlock(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಪೂರ್ಣಾಂಕ fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	ಹಿಂತಿರುಗಿ -1;
}

pid_t
exec_service(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ಅಕ್ಷರ *file, sfd[32];
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = { service, "--lockfd", sfd, arg, ಖಾಲಿ };
	ಪೂರ್ಣಾಂಕ fd = svc_ಬೀಗ(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t ಖಾಲಿಗಳು;
	pid_t pid = 0;

	ಆದರೆ (fd == -1)
		ಹಿಂತಿರುಗಿ -1;
	snprintf(sfd, ಗಾತ್ರ(sfd), "%d", fd);

	file = rc_service_resolve(service);
	ಆದರೆ (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		ಹೋಗು ಹೊರಹೋಗು;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&ಖಾಲಿಗಳು);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &ಖಾಲಿಗಳು);

	ಆದರೆ ((errno = posix_spawn(&pid, file, ಖಾಲಿ, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

ಹೊರಹೋಗು:
	ಬಿಡುಗಡೆ(file);
	ಹಿಂತಿರುಗಿ pid;
}

ಪೂರ್ಣಾಂಕ
parse_ಬಹುಲಕ(mode_t *ಬಹುಲಕ, ಅಕ್ಷರ *ಪಠ್ಯ)
{
	ಅಕ್ಷರ *p;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ l;

	/* Check for a numeric mode */
	ಆದರೆ ((*ಪಠ್ಯ - '0') < 8) {
		l = strtoul(ಪಠ್ಯ, &p, 8);
		ಆದರೆ (*p || l > 07777U) {
			errno = EINVAL;
			ಹಿಂತಿರುಗಿ -1;
		}
		*ಬಹುಲಕ = (mode_t) l;
		ಹಿಂತಿರುಗಿ 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	ಹಿಂತಿರುಗಿ -1;
}

RC_DEPTREE * rc_deptree_ತುಂಬು_2(ಪೂರ್ಣಾಂಕ force, ಪೂರ್ಣಾಂಕ *regen)
{
	ಪೂರ್ಣಾಂಕ fd;
	ಪೂರ್ಣಾಂಕ retval;
	ಪೂರ್ಣಾಂಕ serrno = errno;
	ಪೂರ್ಣಾಂಕ merrno;
	time_t t;
	ಅಕ್ಷರ file[PATH_MAX];
	ರಚನೆ stat st;
	FILE *fp;

	t = 0;
	ಆದರೆ (rc_deptree_update_needed(&t, file) || force != 0) {
		ಪೂರ್ಣಾಂಕ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		ಆದರೆ (fd == -1 && merrno == EACCES)
			ಹಿಂತಿರುಗಿ rc_deptree_ತುಂಬು();
		close(fd);

		ಆದರೆ (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		ಆದರೆ (retval == 0) {
			ಆದರೆ (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				ಹಿಂತಿರುಗಿ ಖಾಲಿ;
			}

			ಆದರೆ (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				ಹೋಗು ಹೊರಹೋಗು;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ರಚನೆ timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			ಆದರೆ ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

ಹೊರಹೋಗು:
		ಆದರೆ (force == -1 && regen != ಖಾಲಿ)
			*regen = retval;
	}

	ಹಿಂತಿರುಗಿ rc_deptree_ತುಂಬು();
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ name;
	RC_SERVICE ಬಿಟ್;
} service_ಬಿಟ್ಟುಗಳು[] = {
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

RC_SERVICE lookup_service_state(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	size_t i;
	ಗಾಗಿ (i = 0; i < ARRAY_ಗಾತ್ರ(service_ಬಿಟ್ಟುಗಳು); ++i)
		ಆದರೆ (!strcmp(service, service_ಬಿಟ್ಟುಗಳು[i].name))
			ಹಿಂತಿರುಗಿ service_ಬಿಟ್ಟುಗಳು[i].ಬಿಟ್;
	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ from_ಸಮಯ_t(ಅಕ್ಷರ *ಸಮಯ_string, time_t tv)
{
	strftime(ಸಮಯ_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t ಗಮ್ಯ_ಸಮಯ_t(ಅಕ್ಷರ *timestring)
{
	ಪೂರ್ಣಾಂಕ check = 0;
	ಪೂರ್ಣಾಂಕ year = 0;
	ಪೂರ್ಣಾಂಕ month = 0;
	ಪೂರ್ಣಾಂಕ day = 0;
	ಪೂರ್ಣಾಂಕ hour = 0;
	ಪೂರ್ಣಾಂಕ min = 0;
	ಪೂರ್ಣಾಂಕ sec = 0;
	ರಚನೆ tm breakdown = {0};
	time_t ಫಲಿತಾಂಶ = -1;

	check = ಅಕ್ಷರಸರಣಿ_ಓದು(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	ಆದರೆ (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		ಫಲಿತಾಂಶ = mktime(&breakdown);
	}
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

pid_t get_pid(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pidfile)
{
	FILE *fp;
	pid_t pid;

	ಆದರೆ (!pidfile)
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ ((fp = fopen(pidfile, "r")) == ಖಾಲಿ) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		ಹಿಂತಿರುಗಿ -1;
	}

	ಆದರೆ (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		ಹಿಂತಿರುಗಿ -1;
	}

	fclose(fp);

	ಹಿಂತಿರುಗಿ pid;
}

ರಚನೆ notify notify_parse(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *notify_string)
{
	ರಚನೆ notify notify = {0};
	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		ಆದರೆ (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(notify_string, "socket", ಗಾತ್ರ("socket") - 1) == 0) {
		ಒಕ್ಕೂಟ {
			ರಚನೆ sockaddr ಶೀರ್ಷಿಕೆ;
			ರಚನೆ sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		ಪೂರ್ಣಾಂಕ written;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opts = strchr(notify_string, ':');

		ಆದರೆ (!opts || opts[1] == '\0')
			ಹಿಂತಿರುಗಿ notify;
		ಆದರೆ (strcmp(opts + 1, "ready") != 0)
			ಹಿಂತಿರುಗಿ notify;

		written = snprintf(addr.unix.sun_path, ಗಾತ್ರ(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		ಆದರೆ (written >= (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		ಆದರೆ ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		ಆದರೆ (bind(notify.fd, &addr.ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} ಇಲ್ಲದಿದ್ದರೆ {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	ಹಿಂತಿರುಗಿ notify;
}

bool notify_wait(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ರಚನೆ notify notify)
{
	ಆಯ್ಕೆ (notify.type) {
	ಸ್ಥಿತಿ NOTIFY_NONE:
		ಹಿಂತಿರುಗಿ true;
	ಸ್ಥಿತಿ NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ NOTIFY_SOCKET:
		ನಿಲ್ಲಿಸು;
	}

	ಗಾಗಿ (;;) {
		ಅಕ್ಷರ buf[BUFSIZ];
		ssize_t ಬೈಟುಗಳು = ಪಠ(notify.fd, buf, BUFSIZ);

		ಆದರೆ (ಬೈಟುಗಳು == 0)
			ಹಿಂತಿರುಗಿ false;
		ಆದರೆ (ಬೈಟುಗಳು == -1) {
			ಆದರೆ (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				ಹಿಂತಿರುಗಿ false;
			}
			ಮುಂದುವರಿ;
		}

		ಆಯ್ಕೆ (notify.type) {
		ಸ್ಥಿತಿ NOTIFY_NONE:
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NOTIFY_FD:
			ಆದರೆ (memchr(buf, '\n', ಬೈಟುಗಳು))
				ಹಿಂತಿರುಗಿ true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ NOTIFY_SOCKET:
			buf[ಬೈಟುಗಳು] = '\0';
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(buf, "READY=1")) {
				ಅಕ್ಷರ *ಮಾರ್ಗ;
				xasprintf(&ಮಾರ್ಗ, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(ಮಾರ್ಗ);
				ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
				ಹಿಂತಿರುಗಿ true;
			}
			ನಿಲ್ಲಿಸು;
		}
	}

	ಹಿಂತಿರುಗಿ true;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HAVE_CLOSE_RANGE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ close_range(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮೊದಲ RC_ಬಳಸದ,
			      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೊನೆಯ RC_ಬಳಸದ,
			      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags RC_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS_close_range
	ಹಿಂತಿರುಗಿ syscall(SYS_close_range, ಮೊದಲ, ಕೊನೆಯ, flags);
#ಇಲ್ಲದಿದ್ದರೆ
	errno = ENOSYS;
	ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ
cloexec_fds_from(ಪೂರ್ಣಾಂಕ ಮೊದಲ)
{
	ಪೂರ್ಣಾಂಕ i;
	ಆದರೆ (close_range(ಮೊದಲ, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		ಗಾಗಿ (i = getdtablesize() - 1; i >= ಮೊದಲ; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
