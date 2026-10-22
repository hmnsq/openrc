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

#شامل <ctype.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <fnmatch.h>
#شامل <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <spawn.h>
#شامل <sys/file.h>
#شامل <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#شامل <sys/types.h>
#شامل <sys/utsname.h>
#شامل <sys/un.h>
#شامل <sys/socket.h>
#شامل <time.h>
#شامل <unistd.h>
#شامل <utime.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "version.h"
#شامل "helpers.h"

بیرونی نویسه **environ;

bool
rc_conf_yesno(ثابت نویسه *setting)
{
	بازگشت rc_yesno(rc_conf_مقدار (setting));
}

ایستا ثابت نویسه *ثابت env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	پوچ
};

ایستا ثابت نویسه *ثابت usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", پوچ
};

ایستا bool
env_allowed(ثابت نویسه *ثابت list[], ثابت نویسه *مقدار)
{
	برای (size_t i = 0; list[i]; i++)
		اگر (fnmatch(list[i], مقدار, FNM_NOESCAPE | FNM_PERIOD) == 0)
			بازگشت true;
	بازگشت false;
}

تهی
env_filter(تهی)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	نویسه *profile_path;
	RC_STRING *env;
	نویسه *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_مقدار("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	اگر (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		بازگشت;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	آزاد(profile_path);

	اگر (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		آزاد(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	وقتی (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->مقدار, '=');
		اگر (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		اگر (env_allowed(env_allowlist, env->مقدار))
			ادامه;
		اگر (rc_is_user() && env_allowed(usrenv_allowlist, env->مقدار))
			ادامه;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			اگر (fnmatch(var->مقدار, env->مقدار, FNM_NOESCAPE | FNM_PERIOD) == 0)
				برو next;

		/* OK, not allowed! */
		unsetenv(env->مقدار);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->مقدار, '=');
		*e = '\0';
		اگر (!getenv(env->مقدار))
			setenv(env->مقدار, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

نویسه *
default_runlevel(تهی)
{
	نویسه *runlevel = rc_runlevel_get();

	اگر (!runlevel)
		runlevel = xstrdup(rc_conf_مقدار("rc_default_runlevel"));
	اگر (!runlevel || !*runlevel) {
		آزاد(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	بازگشت runlevel;
}

تهی
env_config(تهی)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	نویسه *path;
	نویسه *p;
	نویسه *e;
	size_t l;
	ساختار utsname uts;
	FILE *fp;
	نویسه *token;
	نویسه *np;
	نویسه *npp;
	نویسه *tok;
	ثابت نویسه *sys = rc_sys();
	ثابت نویسه *svcdir = rc_svcdir();
	ثابت نویسه *ثابت *init_path = rc_scriptdirs();
	نویسه *buffer = پوچ;
	نویسه *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	اگر (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	وگرنه اگر (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(اندازه(نویسه) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		وقتی ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			وقتی ((tok = strsep(&npp, ":")))
				اگر (strcmp(tok, token) == 0)
					بشکن;
			اگر (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			آزاد (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		آزاد(e);
	}

	اگر (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} وگرنه {
		ثابت نویسه *cache_home = getenv("XDG_CACHE_HOME");
		نویسه *cachedir = پوچ;

		اگر (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		وگرنه اگر ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		اگر (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		آزاد(cachedir);
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
	برای (bool first = true; *init_path; init_path++, first = false) {
		اگر (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	آزاد(e);
	آزاد(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	اگر (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		اگر (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		وگرنه
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		آزاد(buffer);
		fclose(fp);
	} وگرنه {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	اگر (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	اگر (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	اگر (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	اگر (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	اگر ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

صحیح
signal_setup(صحیح sig, تهی (*handler)(صحیح))
{
	ساختار sigaction sa;

	memset(&sa, 0, اندازه (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	بازگشت sigaction(sig, &sa, پوچ);
}

صحیح
svc_lock(ثابت نویسه *applet, bool ignore_lock_failure)
{
	صحیح fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	اگر (fd == -1)
		بازگشت -1;
	اگر (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		صحیح saved_errno = errno;
		اگر (ignore_lock_failure) {
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
		بازگشت -1;
	}
	بازگشت fd;
}

صحیح
svc_unlock(ثابت نویسه *applet, صحیح fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	بازگشت -1;
}

pid_t
exec_service(ثابت نویسه *service, ثابت نویسه *arg)
{
	نویسه *file, sfd[32];
	ثابت نویسه *argv[] = { service, "--lockfd", sfd, arg, پوچ };
	صحیح fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	اگر (fd == -1)
		بازگشت -1;
	snprintf(sfd, اندازه(sfd), "%d", fd);

	file = rc_service_resolve(service);
	اگر (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		برو out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	اگر ((errno = posix_spawn(&pid, file, پوچ, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	آزاد(file);
	بازگشت pid;
}

صحیح
parse_mode(mode_t *mode, نویسه *text)
{
	نویسه *p;
	unsigned long l;

	/* Check for a numeric mode */
	اگر ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		اگر (*p || l > 07777U) {
			errno = EINVAL;
			بازگشت -1;
		}
		*mode = (mode_t) l;
		بازگشت 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	بازگشت -1;
}

RC_DEPTREE * _rc_deptree_load(صحیح force, صحیح *regen)
{
	صحیح fd;
	صحیح retval;
	صحیح serrno = errno;
	صحیح merrno;
	time_t t;
	نویسه file[PATH_MAX];
	ساختار stat st;
	FILE *fp;

	t = 0;
	اگر (rc_deptree_update_needed(&t, file) || force != 0) {
		صحیح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		اگر (fd == -1 && merrno == EACCES)
			بازگشت rc_deptree_load();
		close(fd);

		اگر (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		اگر (retval == 0) {
			اگر (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				بازگشت پوچ;
			}

			اگر (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				برو out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ساختار timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			اگر ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		اگر (force == -1 && regen != پوچ)
			*regen = retval;
	}

	بازگشت rc_deptree_load();
}

ایستا ثابت ساختار {
	ثابت نویسه * ثابت name;
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

RC_SERVICE lookup_service_state(ثابت نویسه *service)
{
	size_t i;
	برای (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		اگر (!strcmp(service, service_bits[i].name))
			بازگشت service_bits[i].bit;
	بازگشت 0;
}

تهی from_time_t(نویسه *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(نویسه *timestring)
{
	صحیح check = 0;
	صحیح year = 0;
	صحیح month = 0;
	صحیح day = 0;
	صحیح hour = 0;
	صحیح min = 0;
	صحیح sec = 0;
	ساختار tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	اگر (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	بازگشت result;
}

pid_t get_pid(ثابت نویسه *applet,ثابت نویسه *pidfile)
{
	FILE *fp;
	pid_t pid;

	اگر (!pidfile)
		بازگشت -1;

	اگر ((fp = fopen(pidfile, "r")) == پوچ) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		بازگشت -1;
	}

	اگر (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		بازگشت -1;
	}

	fclose(fp);

	بازگشت pid;
}

ساختار notify notify_parse(ثابت نویسه *applet, ثابت نویسه *notify_string)
{
	ساختار notify notify = {0};
	اگر (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		اگر (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} وگرنه اگر (strncmp(notify_string, "socket", اندازه("socket") - 1) == 0) {
		اجتماع {
			ساختار sockaddr header;
			ساختار sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		صحیح written;
		ثابت نویسه *opts = strchr(notify_string, ':');

		اگر (!opts || opts[1] == '\0')
			بازگشت notify;
		اگر (strcmp(opts + 1, "ready") != 0)
			بازگشت notify;

		written = snprintf(addr.unix.sun_path, اندازه(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		اگر (written >= (صحیح)اندازه(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		اگر ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		اگر (bind(notify.fd, &addr.header, اندازه(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} وگرنه {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	بازگشت notify;
}

bool notify_wait(ثابت نویسه *applet, ساختار notify notify)
{
	انتخاب (notify.type) {
	حالت NOTIFY_NONE:
		بازگشت true;
	حالت NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		بشکن;
	حالت NOTIFY_SOCKET:
		بشکن;
	}

	برای (;;) {
		نویسه buf[BUFSIZ];
		ssize_t bytes = اقرأ(notify.fd, buf, BUFSIZ);

		اگر (bytes == 0)
			بازگشت false;
		اگر (bytes == -1) {
			اگر (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				بازگشت false;
			}
			ادامه;
		}

		انتخاب (notify.type) {
		حالت NOTIFY_NONE:
			بشکن;
		حالت NOTIFY_FD:
			اگر (memchr(buf, '\n', bytes))
				بازگشت true;
			بشکن;
		حالت NOTIFY_SOCKET:
			buf[bytes] = '\0';
			اگر (strstr(buf, "READY=1")) {
				نویسه *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				آزاد(path);
				بازگشت true;
			}
			بشکن;
		}
	}

	بازگشت true;
}

#ifndef HAVE_CLOSE_RANGE
ایستا درونخطی صحیح close_range(unsigned صحیح first RC_UNUSED,
			      unsigned صحیح last RC_UNUSED,
			      unsigned صحیح flags RC_UNUSED)
{
#ifdef SYS_close_range
	بازگشت syscall(SYS_close_range, first, last, flags);
#وگرنه
	errno = ENOSYS;
	بازگشت -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

تهی
cloexec_fds_from(صحیح first)
{
	صحیح i;
	اگر (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		برای (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
