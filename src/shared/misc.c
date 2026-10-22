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

#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <fnmatch.h>
#অন্তর্ভাব <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <spawn.h>
#অন্তর্ভাব <sys/file.h>
#অন্তর্ভাব <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/utsname.h>
#অন্তর্ভাব <sys/un.h>
#অন্তর্ভাব <sys/socket.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <utime.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "version.h"
#অন্তর্ভাব "helpers.h"

বাহ্য বর্ণ **environ;

bool
rc_conf_yesno(ধ্রুব বর্ণ *setting)
{
	প্রত্যাগম rc_yesno(rc_conf_মান (setting));
}

স্থির ধ্রুব বর্ণ *ধ্রুব env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	রিক্ত
};

স্থির ধ্রুব বর্ণ *ধ্রুব usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", রিক্ত
};

স্থির bool
env_allowed(ধ্রুব বর্ণ *ধ্রুব list[], ধ্রুব বর্ণ *মান)
{
	প্রতি (size_t i = 0; list[i]; i++)
		যদি (fnmatch(list[i], মান, FNM_NOESCAPE | FNM_PERIOD) == 0)
			প্রত্যাগম true;
	প্রত্যাগম false;
}

শূন্য
env_filter(শূন্য)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	বর্ণ *profile_path;
	RC_STRING *env;
	বর্ণ *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_মান("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	যদি (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		প্রত্যাগম;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	মুক্ত(profile_path);

	যদি (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		মুক্ত(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	যাবত্ (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->মান, '=');
		যদি (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		যদি (env_allowed(env_allowlist, env->মান))
			অনুবর্ত;
		যদি (rc_is_user() && env_allowed(usrenv_allowlist, env->মান))
			অনুবর্ত;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			যদি (fnmatch(var->মান, env->মান, FNM_NOESCAPE | FNM_PERIOD) == 0)
				গচ্ছ next;

		/* OK, not allowed! */
		unsetenv(env->মান);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->মান, '=');
		*e = '\0';
		যদি (!getenv(env->মান))
			setenv(env->মান, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

বর্ণ *
default_runlevel(শূন্য)
{
	বর্ণ *runlevel = rc_runlevel_get();

	যদি (!runlevel)
		runlevel = xstrdup(rc_conf_মান("rc_default_runlevel"));
	যদি (!runlevel || !*runlevel) {
		মুক্ত(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	প্রত্যাগম runlevel;
}

শূন্য
env_config(শূন্য)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	বর্ণ *path;
	বর্ণ *p;
	বর্ণ *e;
	size_t l;
	সংরচনা utsname uts;
	FILE *fp;
	বর্ণ *token;
	বর্ণ *np;
	বর্ণ *npp;
	বর্ণ *tok;
	ধ্রুব বর্ণ *sys = rc_sys();
	ধ্রুব বর্ণ *svcdir = rc_svcdir();
	ধ্রুব বর্ণ *ধ্রুব *init_path = rc_scriptdirs();
	বর্ণ *buffer = রিক্ত;
	বর্ণ *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	যদি (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	অন্যথা যদি (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(পরিমাণ(বর্ণ) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		যাবত্ ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			যাবত্ ((tok = strsep(&npp, ":")))
				যদি (strcmp(tok, token) == 0)
					ভঙ্গ;
			যদি (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			মুক্ত (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		মুক্ত(e);
	}

	যদি (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} অন্যথা {
		ধ্রুব বর্ণ *cache_home = getenv("XDG_CACHE_HOME");
		বর্ণ *cachedir = রিক্ত;

		যদি (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		অন্যথা যদি ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		যদি (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		মুক্ত(cachedir);
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
	প্রতি (bool first = true; *init_path; init_path++, first = false) {
		যদি (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	মুক্ত(e);
	মুক্ত(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	যদি (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		যদি (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		অন্যথা
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		মুক্ত(buffer);
		fclose(fp);
	} অন্যথা {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	যদি (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	যদি (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	যদি (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	যদি (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	যদি ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

পূর্ণাঙ্ক
signal_setup(পূর্ণাঙ্ক sig, শূন্য (*handler)(পূর্ণাঙ্ক))
{
	সংরচনা sigaction sa;

	memset(&sa, 0, পরিমাণ (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	প্রত্যাগম sigaction(sig, &sa, রিক্ত);
}

পূর্ণাঙ্ক
svc_lock(ধ্রুব বর্ণ *applet, bool ignore_lock_failure)
{
	পূর্ণাঙ্ক fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	যদি (fd == -1)
		প্রত্যাগম -1;
	যদি (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		পূর্ণাঙ্ক saved_errno = errno;
		যদি (ignore_lock_failure) {
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
		প্রত্যাগম -1;
	}
	প্রত্যাগম fd;
}

পূর্ণাঙ্ক
svc_unlock(ধ্রুব বর্ণ *applet, পূর্ণাঙ্ক fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	প্রত্যাগম -1;
}

pid_t
exec_service(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *arg)
{
	বর্ণ *file, sfd[32];
	ধ্রুব বর্ণ *argv[] = { service, "--lockfd", sfd, arg, রিক্ত };
	পূর্ণাঙ্ক fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	যদি (fd == -1)
		প্রত্যাগম -1;
	snprintf(sfd, পরিমাণ(sfd), "%d", fd);

	file = rc_service_resolve(service);
	যদি (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		গচ্ছ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	যদি ((errno = posix_spawn(&pid, file, রিক্ত, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	মুক্ত(file);
	প্রত্যাগম pid;
}

পূর্ণাঙ্ক
parse_mode(mode_t *mode, বর্ণ *text)
{
	বর্ণ *p;
	unsigned long l;

	/* Check for a numeric mode */
	যদি ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		যদি (*p || l > 07777U) {
			errno = EINVAL;
			প্রত্যাগম -1;
		}
		*mode = (mode_t) l;
		প্রত্যাগম 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	প্রত্যাগম -1;
}

RC_DEPTREE * _rc_deptree_load(পূর্ণাঙ্ক force, পূর্ণাঙ্ক *regen)
{
	পূর্ণাঙ্ক fd;
	পূর্ণাঙ্ক retval;
	পূর্ণাঙ্ক serrno = errno;
	পূর্ণাঙ্ক merrno;
	time_t t;
	বর্ণ file[PATH_MAX];
	সংরচনা stat st;
	FILE *fp;

	t = 0;
	যদি (rc_deptree_update_needed(&t, file) || force != 0) {
		পূর্ণাঙ্ক svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		যদি (fd == -1 && merrno == EACCES)
			প্রত্যাগম rc_deptree_load();
		close(fd);

		যদি (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		যদি (retval == 0) {
			যদি (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				প্রত্যাগম রিক্ত;
			}

			যদি (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				গচ্ছ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (সংরচনা timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			যদি ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		যদি (force == -1 && regen != রিক্ত)
			*regen = retval;
	}

	প্রত্যাগম rc_deptree_load();
}

স্থির ধ্রুব সংরচনা {
	ধ্রুব বর্ণ * ধ্রুব name;
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

RC_SERVICE lookup_service_state(ধ্রুব বর্ণ *service)
{
	size_t i;
	প্রতি (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		যদি (!strcmp(service, service_bits[i].name))
			প্রত্যাগম service_bits[i].bit;
	প্রত্যাগম 0;
}

শূন্য from_time_t(বর্ণ *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(বর্ণ *timestring)
{
	পূর্ণাঙ্ক check = 0;
	পূর্ণাঙ্ক year = 0;
	পূর্ণাঙ্ক month = 0;
	পূর্ণাঙ্ক day = 0;
	পূর্ণাঙ্ক hour = 0;
	পূর্ণাঙ্ক min = 0;
	পূর্ণাঙ্ক sec = 0;
	সংরচনা tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	যদি (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	প্রত্যাগম result;
}

pid_t get_pid(ধ্রুব বর্ণ *applet,ধ্রুব বর্ণ *pidfile)
{
	FILE *fp;
	pid_t pid;

	যদি (!pidfile)
		প্রত্যাগম -1;

	যদি ((fp = fopen(pidfile, "r")) == রিক্ত) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		প্রত্যাগম -1;
	}

	যদি (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		প্রত্যাগম -1;
	}

	fclose(fp);

	প্রত্যাগম pid;
}

সংরচনা notify notify_parse(ধ্রুব বর্ণ *applet, ধ্রুব বর্ণ *notify_string)
{
	সংরচনা notify notify = {0};
	যদি (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		যদি (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} অন্যথা যদি (strncmp(notify_string, "socket", পরিমাণ("socket") - 1) == 0) {
		সঙ্ঘ {
			সংরচনা sockaddr header;
			সংরচনা sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		পূর্ণাঙ্ক written;
		ধ্রুব বর্ণ *opts = strchr(notify_string, ':');

		যদি (!opts || opts[1] == '\0')
			প্রত্যাগম notify;
		যদি (strcmp(opts + 1, "ready") != 0)
			প্রত্যাগম notify;

		written = snprintf(addr.unix.sun_path, পরিমাণ(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		যদি (written >= (পূর্ণাঙ্ক)পরিমাণ(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		যদি ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		যদি (bind(notify.fd, &addr.header, পরিমাণ(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} অন্যথা {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	প্রত্যাগম notify;
}

bool notify_wait(ধ্রুব বর্ণ *applet, সংরচনা notify notify)
{
	বিকল্প (notify.type) {
	পক্ষ NOTIFY_NONE:
		প্রত্যাগম true;
	পক্ষ NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		ভঙ্গ;
	পক্ষ NOTIFY_SOCKET:
		ভঙ্গ;
	}

	প্রতি (;;) {
		বর্ণ buf[BUFSIZ];
		ssize_t bytes = পঠ(notify.fd, buf, BUFSIZ);

		যদি (bytes == 0)
			প্রত্যাগম false;
		যদি (bytes == -1) {
			যদি (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				প্রত্যাগম false;
			}
			অনুবর্ত;
		}

		বিকল্প (notify.type) {
		পক্ষ NOTIFY_NONE:
			ভঙ্গ;
		পক্ষ NOTIFY_FD:
			যদি (memchr(buf, '\n', bytes))
				প্রত্যাগম true;
			ভঙ্গ;
		পক্ষ NOTIFY_SOCKET:
			buf[bytes] = '\0';
			যদি (strstr(buf, "READY=1")) {
				বর্ণ *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				মুক্ত(path);
				প্রত্যাগম true;
			}
			ভঙ্গ;
		}
	}

	প্রত্যাগম true;
}

#ifndef HAVE_CLOSE_RANGE
স্থির অন্তঃস্থ পূর্ণাঙ্ক close_range(unsigned পূর্ণাঙ্ক first RC_UNUSED,
			      unsigned পূর্ণাঙ্ক last RC_UNUSED,
			      unsigned পূর্ণাঙ্ক flags RC_UNUSED)
{
#ifdef SYS_close_range
	প্রত্যাগম syscall(SYS_close_range, first, last, flags);
#অন্যথা
	errno = ENOSYS;
	প্রত্যাগম -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

শূন্য
cloexec_fds_from(পূর্ণাঙ্ক first)
{
	পূর্ণাঙ্ক i;
	যদি (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		প্রতি (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
