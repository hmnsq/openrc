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

#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <fnmatch.h>
#ضمن <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#ضمن <signal.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <spawn.h>
#ضمن <sys/file.h>
#ضمن <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#ضمن <sys/types.h>
#ضمن <sys/utsname.h>
#ضمن <sys/un.h>
#ضمن <sys/socket.h>
#ضمن <time.h>
#ضمن <unistd.h>
#ضمن <utime.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "version.h"
#ضمن "helpers.h"

خارجي حرف **environ;

bool
rc_conf_yesno(ثابت حرف *setting)
{
	أرجع rc_yesno(rc_conf_قيمة (setting));
}

ساكن ثابت حرف *ثابت env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	عدم
};

ساكن ثابت حرف *ثابت usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", عدم
};

ساكن bool
env_allowed(ثابت حرف *ثابت list[], ثابت حرف *قيمة)
{
	لكل (size_t i = 0; list[i]; i++)
		إذا (fnmatch(list[i], قيمة, FNM_NOESCAPE | FNM_PERIOD) == 0)
			أرجع true;
	أرجع false;
}

فراغ
env_filter(فراغ)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	حرف *profile_path;
	RC_STRING *env;
	حرف *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_قيمة("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	إذا (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		أرجع;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	حرر(profile_path);

	إذا (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		حرر(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	طالما (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->قيمة, '=');
		إذا (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		إذا (env_allowed(env_allowlist, env->قيمة))
			استمر;
		إذا (rc_is_user() && env_allowed(usrenv_allowlist, env->قيمة))
			استمر;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			إذا (fnmatch(var->قيمة, env->قيمة, FNM_NOESCAPE | FNM_PERIOD) == 0)
				اذهب next;

		/* OK, not allowed! */
		unsetenv(env->قيمة);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->قيمة, '=');
		*e = '\0';
		إذا (!getenv(env->قيمة))
			setenv(env->قيمة, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

حرف *
default_runlevel(فراغ)
{
	حرف *runlevel = rc_runlevel_get();

	إذا (!runlevel)
		runlevel = xstrdup(rc_conf_قيمة("rc_default_runlevel"));
	إذا (!runlevel || !*runlevel) {
		حرر(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	أرجع runlevel;
}

فراغ
env_config(فراغ)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	حرف *path;
	حرف *p;
	حرف *e;
	size_t l;
	بنية utsname uts;
	FILE *fp;
	حرف *token;
	حرف *np;
	حرف *npp;
	حرف *tok;
	ثابت حرف *sys = rc_sys();
	ثابت حرف *svcdir = rc_svcdir();
	ثابت حرف *ثابت *init_path = rc_scriptdirs();
	حرف *buffer = عدم;
	حرف *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	إذا (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	وإلا إذا (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(حجم(حرف) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		طالما ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			طالما ((tok = strsep(&npp, ":")))
				إذا (strcmp(tok, token) == 0)
					اكسر;
			إذا (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			حرر (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		حرر(e);
	}

	إذا (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} وإلا {
		ثابت حرف *cache_home = getenv("XDG_CACHE_HOME");
		حرف *cachedir = عدم;

		إذا (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		وإلا إذا ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		إذا (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		حرر(cachedir);
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
	لكل (bool first = true; *init_path; init_path++, first = false) {
		إذا (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	حرر(e);
	حرر(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	إذا (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		إذا (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		وإلا
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		حرر(buffer);
		fclose(fp);
	} وإلا {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	إذا (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	إذا (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	إذا (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	إذا (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	إذا ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

صحيح
signal_setup(صحيح sig, فراغ (*handler)(صحيح))
{
	بنية sigaction sa;

	memset(&sa, 0, حجم (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	أرجع sigaction(sig, &sa, عدم);
}

صحيح
svc_lock(ثابت حرف *applet, bool ignore_lock_failure)
{
	صحيح fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	إذا (fd == -1)
		أرجع -1;
	إذا (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		صحيح saved_errno = errno;
		إذا (ignore_lock_failure) {
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
		أرجع -1;
	}
	أرجع fd;
}

صحيح
svc_unlock(ثابت حرف *applet, صحيح fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	أرجع -1;
}

pid_t
exec_service(ثابت حرف *service, ثابت حرف *arg)
{
	حرف *file, sfd[32];
	ثابت حرف *argv[] = { service, "--lockfd", sfd, arg, عدم };
	صحيح fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	إذا (fd == -1)
		أرجع -1;
	snprintf(sfd, حجم(sfd), "%d", fd);

	file = rc_service_resolve(service);
	إذا (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		اذهب out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	إذا ((errno = posix_spawn(&pid, file, عدم, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	حرر(file);
	أرجع pid;
}

صحيح
parse_mode(mode_t *mode, حرف *text)
{
	حرف *p;
	unsigned long l;

	/* Check for a numeric mode */
	إذا ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		إذا (*p || l > 07777U) {
			errno = EINVAL;
			أرجع -1;
		}
		*mode = (mode_t) l;
		أرجع 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	أرجع -1;
}

RC_DEPTREE * _rc_deptree_load(صحيح force, صحيح *regen)
{
	صحيح fd;
	صحيح retval;
	صحيح serrno = errno;
	صحيح merrno;
	time_t t;
	حرف file[PATH_MAX];
	بنية stat st;
	FILE *fp;

	t = 0;
	إذا (rc_deptree_update_needed(&t, file) || force != 0) {
		صحيح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		إذا (fd == -1 && merrno == EACCES)
			أرجع rc_deptree_load();
		close(fd);

		إذا (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		إذا (retval == 0) {
			إذا (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				أرجع عدم;
			}

			إذا (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				اذهب out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (بنية timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			إذا ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		إذا (force == -1 && regen != عدم)
			*regen = retval;
	}

	أرجع rc_deptree_load();
}

ساكن ثابت بنية {
	ثابت حرف * ثابت name;
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

RC_SERVICE lookup_service_state(ثابت حرف *service)
{
	size_t i;
	لكل (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		إذا (!strcmp(service, service_bits[i].name))
			أرجع service_bits[i].bit;
	أرجع 0;
}

فراغ from_time_t(حرف *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(حرف *timestring)
{
	صحيح check = 0;
	صحيح year = 0;
	صحيح month = 0;
	صحيح day = 0;
	صحيح hour = 0;
	صحيح min = 0;
	صحيح sec = 0;
	بنية tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	إذا (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	أرجع result;
}

pid_t get_pid(ثابت حرف *applet,ثابت حرف *pidfile)
{
	FILE *fp;
	pid_t pid;

	إذا (!pidfile)
		أرجع -1;

	إذا ((fp = fopen(pidfile, "r")) == عدم) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		أرجع -1;
	}

	إذا (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		أرجع -1;
	}

	fclose(fp);

	أرجع pid;
}

بنية notify notify_parse(ثابت حرف *applet, ثابت حرف *notify_string)
{
	بنية notify notify = {0};
	إذا (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		إذا (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} وإلا إذا (strncmp(notify_string, "socket", حجم("socket") - 1) == 0) {
		اتحاد {
			بنية sockaddr header;
			بنية sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		صحيح written;
		ثابت حرف *opts = strchr(notify_string, ':');

		إذا (!opts || opts[1] == '\0')
			أرجع notify;
		إذا (strcmp(opts + 1, "ready") != 0)
			أرجع notify;

		written = snprintf(addr.unix.sun_path, حجم(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		إذا (written >= (صحيح)حجم(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		إذا ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		إذا (bind(notify.fd, &addr.header, حجم(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} وإلا {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	أرجع notify;
}

bool notify_wait(ثابت حرف *applet, بنية notify notify)
{
	بدل (notify.type) {
	حالة NOTIFY_NONE:
		أرجع true;
	حالة NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		اكسر;
	حالة NOTIFY_SOCKET:
		اكسر;
	}

	لكل (;;) {
		حرف buf[BUFSIZ];
		ssize_t bytes = اقرأ(notify.fd, buf, BUFSIZ);

		إذا (bytes == 0)
			أرجع false;
		إذا (bytes == -1) {
			إذا (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				أرجع false;
			}
			استمر;
		}

		بدل (notify.type) {
		حالة NOTIFY_NONE:
			اكسر;
		حالة NOTIFY_FD:
			إذا (memchr(buf, '\n', bytes))
				أرجع true;
			اكسر;
		حالة NOTIFY_SOCKET:
			buf[bytes] = '\0';
			إذا (strstr(buf, "READY=1")) {
				حرف *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				حرر(path);
				أرجع true;
			}
			اكسر;
		}
	}

	أرجع true;
}

#ifndef HAVE_CLOSE_RANGE
ساكن مضمن صحيح close_range(unsigned صحيح first RC_UNUSED,
			      unsigned صحيح last RC_UNUSED,
			      unsigned صحيح flags RC_UNUSED)
{
#ifdef SYS_close_range
	أرجع syscall(SYS_close_range, first, last, flags);
#وإلا
	errno = ENOSYS;
	أرجع -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

فراغ
cloexec_fds_from(صحيح first)
{
	صحيح i;
	إذا (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		لكل (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
