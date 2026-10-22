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

#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <fnmatch.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <spawn.h>
#ଅନ୍ତର୍ଭାଵ <sys/file.h>
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/utsname.h>
#ଅନ୍ତର୍ଭାଵ <sys/un.h>
#ଅନ୍ତର୍ଭାଵ <sys/socket.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <utime.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "version.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ବାହ୍ୟ ଅକ୍ଷର **environ;

bool
rc_conf_yesno(ଅଚଳ ଅକ୍ଷର *setting)
{
	ଫେରନ୍ତୁ rc_yesno(rc_conf_ମୂଲ୍ୟ (setting));
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ଖାଲି
};

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ଖାଲି
};

ସ୍ଥିର bool
env_allowed(ଅଚଳ ଅକ୍ଷର *ଅଚଳ list[], ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ପାଇଁ (size_t i = 0; list[i]; i++)
		ଯଦି (fnmatch(list[i], ମୂଲ୍ୟ, FNM_NOESCAPE | FNM_PERIOD) == 0)
			ଫେରନ୍ତୁ true;
	ଫେରନ୍ତୁ false;
}

ଶୂନ୍ୟ
env_filter(ଶୂନ୍ୟ)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	ଅକ୍ଷର *profile_path;
	RC_STRING *env;
	ଅକ୍ଷର *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_ମୂଲ୍ୟ("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	ଯଦି (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		ଫେରନ୍ତୁ;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	ମୁକ୍ତ(profile_path);

	ଯଦି (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		ମୁକ୍ତ(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	ଯେତେବେଳେ (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->ମୂଲ୍ୟ, '=');
		ଯଦି (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		ଯଦି (env_allowed(env_allowlist, env->ମୂଲ୍ୟ))
			ଜାରି;
		ଯଦି (rc_is_user() && env_allowed(usrenv_allowlist, env->ମୂଲ୍ୟ))
			ଜାରି;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			ଯଦି (fnmatch(var->ମୂଲ୍ୟ, env->ମୂଲ୍ୟ, FNM_NOESCAPE | FNM_PERIOD) == 0)
				ଯାଅ next;

		/* OK, not allowed! */
		unsetenv(env->ମୂଲ୍ୟ);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->ମୂଲ୍ୟ, '=');
		*e = '\0';
		ଯଦି (!getenv(env->ମୂଲ୍ୟ))
			setenv(env->ମୂଲ୍ୟ, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

ଅକ୍ଷର *
default_runlevel(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *runlevel = rc_runlevel_get();

	ଯଦି (!runlevel)
		runlevel = xstrdup(rc_conf_ମୂଲ୍ୟ("rc_default_runlevel"));
	ଯଦି (!runlevel || !*runlevel) {
		ମୁକ୍ତ(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	ଫେରନ୍ତୁ runlevel;
}

ଶୂନ୍ୟ
env_config(ଶୂନ୍ୟ)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	ଅକ୍ଷର *path;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *e;
	size_t l;
	ସଂରଚନା utsname uts;
	FILE *fp;
	ଅକ୍ଷର *token;
	ଅକ୍ଷର *np;
	ଅକ୍ଷର *npp;
	ଅକ୍ଷର *tok;
	ଅଚଳ ଅକ୍ଷର *sys = rc_sys();
	ଅଚଳ ଅକ୍ଷର *svcdir = rc_svcdir();
	ଅଚଳ ଅକ୍ଷର *ଅଚଳ *init_path = rc_scriptdirs();
	ଅକ୍ଷର *buffer = ଖାଲି;
	ଅକ୍ଷର *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	ଯଦି (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	ଅନ୍ୟଥା ଯଦି (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(ଆକାର(ଅକ୍ଷର) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		ଯେତେବେଳେ ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			ଯେତେବେଳେ ((tok = strsep(&npp, ":")))
				ଯଦି (strcmp(tok, token) == 0)
					ଭାଙ୍ଗ;
			ଯଦି (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			ମୁକ୍ତ (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		ମୁକ୍ତ(e);
	}

	ଯଦି (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} ଅନ୍ୟଥା {
		ଅଚଳ ଅକ୍ଷର *cache_home = getenv("XDG_CACHE_HOME");
		ଅକ୍ଷର *cachedir = ଖାଲି;

		ଯଦି (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		ଅନ୍ୟଥା ଯଦି ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		ଯଦି (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		ମୁକ୍ତ(cachedir);
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
	ପାଇଁ (bool first = true; *init_path; init_path++, first = false) {
		ଯଦି (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	ମୁକ୍ତ(e);
	ମୁକ୍ତ(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	ଯଦି (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		ଯଦି (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		ଅନ୍ୟଥା
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		ମୁକ୍ତ(buffer);
		fclose(fp);
	} ଅନ୍ୟଥା {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	ଯଦି (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	ଯଦି (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	ଯଦି (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	ଯଦି (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	ଯଦି ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
signal_setup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, ଶୂନ୍ୟ (*handler)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
{
	ସଂରଚନା sigaction sa;

	memset(&sa, 0, ଆକାର (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	ଫେରନ୍ତୁ sigaction(sig, &sa, ଖାଲି);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_lock(ଅଚଳ ଅକ୍ଷର *applet, bool ignore_lock_failure)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	ଯଦି (fd == -1)
		ଫେରନ୍ତୁ -1;
	ଯଦି (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା saved_errno = errno;
		ଯଦି (ignore_lock_failure) {
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
		ଫେରନ୍ତୁ -1;
	}
	ଫେରନ୍ତୁ fd;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_unlock(ଅଚଳ ଅକ୍ଷର *applet, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	ଫେରନ୍ତୁ -1;
}

pid_t
exec_service(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *arg)
{
	ଅକ୍ଷର *file, sfd[32];
	ଅଚଳ ଅକ୍ଷର *argv[] = { service, "--lockfd", sfd, arg, ଖାଲି };
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	ଯଦି (fd == -1)
		ଫେରନ୍ତୁ -1;
	snprintf(sfd, ଆକାର(sfd), "%d", fd);

	file = rc_service_resolve(service);
	ଯଦି (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		ଯାଅ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	ଯଦି ((errno = posix_spawn(&pid, file, ଖାଲି, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	ମୁକ୍ତ(file);
	ଫେରନ୍ତୁ pid;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
parse_mode(mode_t *mode, ଅକ୍ଷର *text)
{
	ଅକ୍ଷର *p;
	unsigned long l;

	/* Check for a numeric mode */
	ଯଦି ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		ଯଦି (*p || l > 07777U) {
			errno = EINVAL;
			ଫେରନ୍ତୁ -1;
		}
		*mode = (mode_t) l;
		ଫେରନ୍ତୁ 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	ଫେରନ୍ତୁ -1;
}

RC_DEPTREE * _rc_deptree_load(ପୂର୍ଣ୍ଣସଂଖ୍ୟା force, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *regen)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା merrno;
	time_t t;
	ଅକ୍ଷର file[PATH_MAX];
	ସଂରଚନା stat st;
	FILE *fp;

	t = 0;
	ଯଦି (rc_deptree_update_needed(&t, file) || force != 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		ଯଦି (fd == -1 && merrno == EACCES)
			ଫେରନ୍ତୁ rc_deptree_load();
		close(fd);

		ଯଦି (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		ଯଦି (retval == 0) {
			ଯଦି (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				ଫେରନ୍ତୁ ଖାଲି;
			}

			ଯଦି (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				ଯାଅ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ସଂରଚନା timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			ଯଦି ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		ଯଦି (force == -1 && regen != ଖାଲି)
			*regen = retval;
	}

	ଫେରନ୍ତୁ rc_deptree_load();
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା {
	ଅଚଳ ଅକ୍ଷର * ଅଚଳ name;
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

RC_SERVICE lookup_service_state(ଅଚଳ ଅକ୍ଷର *service)
{
	size_t i;
	ପାଇଁ (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		ଯଦି (!strcmp(service, service_bits[i].name))
			ଫେରନ୍ତୁ service_bits[i].bit;
	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ from_time_t(ଅକ୍ଷର *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(ଅକ୍ଷର *timestring)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା check = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା year = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା month = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା day = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hour = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sec = 0;
	ସଂରଚନା tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	ଯଦି (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	ଫେରନ୍ତୁ result;
}

pid_t get_pid(ଅଚଳ ଅକ୍ଷର *applet,ଅଚଳ ଅକ୍ଷର *pidfile)
{
	FILE *fp;
	pid_t pid;

	ଯଦି (!pidfile)
		ଫେରନ୍ତୁ -1;

	ଯଦି ((fp = fopen(pidfile, "r")) == ଖାଲି) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		ଫେରନ୍ତୁ -1;
	}

	ଯଦି (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		ଫେରନ୍ତୁ -1;
	}

	fclose(fp);

	ଫେରନ୍ତୁ pid;
}

ସଂରଚନା notify notify_parse(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *notify_string)
{
	ସଂରଚନା notify notify = {0};
	ଯଦି (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		ଯଦି (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} ଅନ୍ୟଥା ଯଦି (strncmp(notify_string, "socket", ଆକାର("socket") - 1) == 0) {
		ସଂଘ {
			ସଂରଚନା sockaddr header;
			ସଂରଚନା sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା written;
		ଅଚଳ ଅକ୍ଷର *opts = strchr(notify_string, ':');

		ଯଦି (!opts || opts[1] == '\0')
			ଫେରନ୍ତୁ notify;
		ଯଦି (strcmp(opts + 1, "ready") != 0)
			ଫେରନ୍ତୁ notify;

		written = snprintf(addr.unix.sun_path, ଆକାର(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		ଯଦି (written >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		ଯଦି ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		ଯଦି (bind(notify.fd, &addr.header, ଆକାର(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} ଅନ୍ୟଥା {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	ଫେରନ୍ତୁ notify;
}

bool notify_wait(ଅଚଳ ଅକ୍ଷର *applet, ସଂରଚନା notify notify)
{
	ବାଛ (notify.type) {
	ସ୍ଥିତି NOTIFY_NONE:
		ଫେରନ୍ତୁ true;
	ସ୍ଥିତି NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		ଭାଙ୍ଗ;
	ସ୍ଥିତି NOTIFY_SOCKET:
		ଭାଙ୍ଗ;
	}

	ପାଇଁ (;;) {
		ଅକ୍ଷର buf[BUFSIZ];
		ssize_t bytes = ପଠ(notify.fd, buf, BUFSIZ);

		ଯଦି (bytes == 0)
			ଫେରନ୍ତୁ false;
		ଯଦି (bytes == -1) {
			ଯଦି (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				ଫେରନ୍ତୁ false;
			}
			ଜାରି;
		}

		ବାଛ (notify.type) {
		ସ୍ଥିତି NOTIFY_NONE:
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NOTIFY_FD:
			ଯଦି (memchr(buf, '\n', bytes))
				ଫେରନ୍ତୁ true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି NOTIFY_SOCKET:
			buf[bytes] = '\0';
			ଯଦି (strstr(buf, "READY=1")) {
				ଅକ୍ଷର *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				ମୁକ୍ତ(path);
				ଫେରନ୍ତୁ true;
			}
			ଭାଙ୍ଗ;
		}
	}

	ଫେରନ୍ତୁ true;
}

#ifndef HAVE_CLOSE_RANGE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା close_range(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା first RC_UNUSED,
			      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା last RC_UNUSED,
			      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags RC_UNUSED)
{
#ifdef SYS_close_range
	ଫେରନ୍ତୁ syscall(SYS_close_range, first, last, flags);
#ଅନ୍ୟଥା
	errno = ENOSYS;
	ଫେରନ୍ତୁ -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

ଶୂନ୍ୟ
cloexec_fds_from(ପୂର୍ଣ୍ଣସଂଖ୍ୟା first)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଯଦି (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		ପାଇଁ (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
