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
#اگر_تعریف_شدہ HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#اگر_ختم
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <spawn.h>
#شامل <sys/file.h>
#شامل <sys/time.h>
#اگر_تعریف_شدہ __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#اگر_ختم
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

خارجی حرف **environ;

bool
rc_conf_yesno(ثابت حرف *setting)
{
	واپس rc_yesno(rc_conf_قدر (setting));
}

ساکن ثابت حرف *ثابت env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	تہی
};

ساکن ثابت حرف *ثابت usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", تہی
};

ساکن bool
env_allowed(ثابت حرف *ثابت list[], ثابت حرف *قدر)
{
	برائے (size_t i = 0; list[i]; i++)
		اگر (fnmatch(list[i], قدر, FNM_NOESCAPE | FNM_PERIOD) == 0)
			واپس true;
	واپس false;
}

خالی
env_filter(خالی)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	حرف *profile_راستہ;
	RC_STRING *env;
	حرف *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_بانٹو(rc_conf_قدر("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	اگر (rc_stringlist_ڈھونڈو(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		واپس;
	}

	xasprintf(&profile_راستہ, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_بھرو(profile_راستہ);
	آزاد(profile_راستہ);

	اگر (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_راستہ, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_بھرو(profile_راستہ);
		آزاد(profile_راستہ);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	جب (environ && environ[i]) {
		env = rc_stringlist_جوڑو(env_list, environ[i++]);
		e = strchr(env->قدر, '=');
		اگر (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		اگر (env_allowed(env_allowlist, env->قدر))
			جاری;
		اگر (rc_is_user() && env_allowed(usrenv_allowlist, env->قدر))
			جاری;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			اگر (fnmatch(var->قدر, env->قدر, FNM_NOESCAPE | FNM_PERIOD) == 0)
				جاؤ اگلا;

		/* OK, not allowed! */
		unsetenv(env->قدر);
اگلا:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->قدر, '=');
		*e = '\0';
		اگر (!getenv(env->قدر))
			setenv(env->قدر, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

حرف *
default_runlevel(خالی)
{
	حرف *runlevel = rc_runlevel_get();

	اگر (!runlevel)
		runlevel = xstrdup(rc_conf_قدر("rc_default_runlevel"));
	اگر (!runlevel || !*runlevel) {
		آزاد(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	واپس runlevel;
}

خالی
env_config(خالی)
{
	size_t pplen = لڑی_لمبائی(RC_راستہ_سابقہ);
	حرف *راستہ;
	حرف *p;
	حرف *e;
	size_t l;
	ساخت utsname uts;
	FILE *fp;
	حرف *token;
	حرف *np;
	حرف *npp;
	حرف *tok;
	ثابت حرف *sys = rc_sys();
	ثابت حرف *svcdir = rc_svcdir();
	ثابت حرف *ثابت *init_راستہ = rc_scriptdirs();
	حرف *بفر = تہی;
	حرف *tmpdir;
	size_t سائز = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	راستہ = getenv("PATH");
	اگر (!راستہ)
		setenv("PATH", RC_راستہ_سابقہ, 1);
	ورنہ اگر (لڑی_موازنہ (RC_راستہ_سابقہ, راستہ, pplen) != 0) {
		l = لڑی_لمبائی(راستہ) + pplen + 3;
		e = p = xmalloc(حجم(حرف) * l);
		p += snprintf(p, l, "%s", RC_راستہ_سابقہ);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		جب ((token = strsep(&راستہ, ":"))) {
			np = npp = xstrdup(RC_راستہ_سابقہ);
			جب ((tok = strsep(&npp, ":")))
				اگر (strcmp(tok, token) == 0)
					توڑو;
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
	} ورنہ {
		ثابت حرف *cache_home = getenv("XDG_CACHE_HOME");
		حرف *cachedir = تہی;

		اگر (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		ورنہ اگر ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		اگر (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		آزاد(cachedir);
	}

	xasprintf(&tmpdir, "%s/tmp", svcdir);
	e = default_runlevel();

	setenv("RC_VERSION", نسخہ, 1);
	setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 1);
	setenv("RC_SVCDIR", svcdir, 1);
	setenv("RC_TMPDIR", tmpdir, 1);
	setenv("RC_BOOTLEVEL", RC_LEVEL_BOOT, 1);
	setenv("RC_RUNLEVEL", e, 1);

	fp = xopen_memstream(&راستہ, &سائز);
	fputs("RC_PATH=", fp);
	برائے (bool پہلا = true; *init_راستہ; init_راستہ++, پہلا = false) {
		اگر (!پہلا)
			fputc(':', fp);
		fputs(*init_راستہ, fp);
	}
	xclose_memstream(fp);
	putenv(راستہ);

	آزاد(e);
	آزاد(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	اگر (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		اگر (xgetline(&بفر, &سائز, fp) != -1)
			setenv("RC_DEFAULTLEVEL", بفر, 1);
		ورنہ
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		آزاد(بفر);
		fclose(fp);
	} ورنہ {
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
signal_تیاری(صحیح sig, خالی (*handler)(صحیح))
{
	ساخت sigaction sa;

	memset(&sa, 0, حجم (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	واپس sigaction(sig, &sa, تہی);
}

صحیح
svc_تالا(ثابت حرف *applet, bool ignore_تالا_failure)
{
	صحیح fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	اگر (fd == -1)
		واپس -1;
	اگر (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		صحیح saved_errno = errno;
		اگر (ignore_تالا_failure) {
			/* Two services with a need b, and b's start()
			 * calling restart --no-deps on a would cause
			 * harmless errors: just ignore them.
			 * See https://github.com/OpenRC/openrc/issues/224
			 */
			باہر_نکلو(EXIT_SUCCESS);
		}
		eerror("Call to flock failed: %s", strerror(errno));
		close(fd);
		errno = saved_errno;
		واپس -1;
	}
	واپس fd;
}

صحیح
svc_unlock(ثابت حرف *applet, صحیح fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	واپس -1;
}

pid_t
exec_service(ثابت حرف *service, ثابت حرف *arg)
{
	حرف *file, sfd[32];
	ثابت حرف *argv[] = { service, "--lockfd", sfd, arg, تہی };
	صحیح fd = svc_تالا(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t خالی_خانے;
	pid_t pid = 0;

	اگر (fd == -1)
		واپس -1;
	snprintf(sfd, حجم(sfd), "%d", fd);

	file = rc_service_resolve(service);
	اگر (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		جاؤ برآمد;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&خالی_خانے);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &خالی_خانے);

	اگر ((errno = posix_spawn(&pid, file, تہی, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

برآمد:
	آزاد(file);
	واپس pid;
}

صحیح
parse_منوال(mode_t *منوال, حرف *متن)
{
	حرف *p;
	غیرعلامتی طویل l;

	/* Check for a numeric mode */
	اگر ((*متن - '0') < 8) {
		l = strtoul(متن, &p, 8);
		اگر (*p || l > 07777U) {
			errno = EINVAL;
			واپس -1;
		}
		*منوال = (mode_t) l;
		واپس 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	واپس -1;
}

RC_DEPTREE * rc_deptree_بھرو_2(صحیح force, صحیح *regen)
{
	صحیح fd;
	صحیح retval;
	صحیح serrno = errno;
	صحیح merrno;
	time_t t;
	حرف file[PATH_MAX];
	ساخت stat st;
	FILE *fp;

	t = 0;
	اگر (rc_deptree_update_needed(&t, file) || force != 0) {
		صحیح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		اگر (fd == -1 && merrno == EACCES)
			واپس rc_deptree_بھرو();
		close(fd);

		اگر (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		اگر (retval == 0) {
			اگر (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				واپس تہی;
			}

			اگر (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				جاؤ برآمد;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ساخت timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			اگر ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

برآمد:
		اگر (force == -1 && regen != تہی)
			*regen = retval;
	}

	واپس rc_deptree_بھرو();
}

ساکن ثابت ساخت {
	ثابت حرف * ثابت name;
	RC_SERVICE ثنائی_ہندسہ;
} service_بٹ[] = {
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
	برائے (i = 0; i < ARRAY_سائز(service_بٹ); ++i)
		اگر (!strcmp(service, service_بٹ[i].name))
			واپس service_بٹ[i].ثنائی_ہندسہ;
	واپس 0;
}

خالی from_وقت_t(حرف *وقت_string, time_t tv)
{
	strftime(وقت_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t مقصد_وقت_t(حرف *timestring)
{
	صحیح check = 0;
	صحیح year = 0;
	صحیح month = 0;
	صحیح day = 0;
	صحیح hour = 0;
	صحیح min = 0;
	صحیح sec = 0;
	ساخت tm breakdown = {0};
	time_t نتیجہ = -1;

	check = لڑی_سے_پڑھو(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	اگر (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		نتیجہ = mktime(&breakdown);
	}
	واپس نتیجہ;
}

pid_t get_pid(ثابت حرف *applet,ثابت حرف *pidfile)
{
	FILE *fp;
	pid_t pid;

	اگر (!pidfile)
		واپس -1;

	اگر ((fp = fopen(pidfile, "r")) == تہی) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		واپس -1;
	}

	اگر (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		واپس -1;
	}

	fclose(fp);

	واپس pid;
}

ساخت notify notify_parse(ثابت حرف *applet, ثابت حرف *notify_string)
{
	ساخت notify notify = {0};
	اگر (لڑی_سے_پڑھو(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		اگر (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} ورنہ اگر (لڑی_موازنہ(notify_string, "socket", حجم("socket") - 1) == 0) {
		اتحاد {
			ساخت sockaddr سرخی;
			ساخت sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		صحیح written;
		ثابت حرف *opts = strchr(notify_string, ':');

		اگر (!opts || opts[1] == '\0')
			واپس notify;
		اگر (strcmp(opts + 1, "ready") != 0)
			واپس notify;

		written = snprintf(addr.unix.sun_path, حجم(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		اگر (written >= (صحیح)حجم(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		اگر ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		اگر (bind(notify.fd, &addr.سرخی, حجم(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} ورنہ {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	واپس notify;
}

bool notify_wait(ثابت حرف *applet, ساخت notify notify)
{
	انتخاب (notify.type) {
	حالت NOTIFY_NONE:
		واپس true;
	حالت NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		توڑو;
	حالت NOTIFY_SOCKET:
		توڑو;
	}

	برائے (;;) {
		حرف buf[BUFSIZ];
		ssize_t بائٹیں = اقرأ(notify.fd, buf, BUFSIZ);

		اگر (بائٹیں == 0)
			واپس false;
		اگر (بائٹیں == -1) {
			اگر (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				واپس false;
			}
			جاری;
		}

		انتخاب (notify.type) {
		حالت NOTIFY_NONE:
			توڑو;
		حالت NOTIFY_FD:
			اگر (memchr(buf, '\n', بائٹیں))
				واپس true;
			توڑو;
		حالت NOTIFY_SOCKET:
			buf[بائٹیں] = '\0';
			اگر (لڑی_تلاش(buf, "READY=1")) {
				حرف *راستہ;
				xasprintf(&راستہ, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(راستہ);
				آزاد(راستہ);
				واپس true;
			}
			توڑو;
		}
	}

	واپس true;
}

#اگر_غیر_تعریف_شدہ HAVE_CLOSE_RANGE
ساکن اندرونی صحیح close_range(غیرعلامتی صحیح پہلا RC_غیر_مستعمل,
			      غیرعلامتی صحیح آخری RC_غیر_مستعمل,
			      غیرعلامتی صحیح flags RC_غیر_مستعمل)
{
#اگر_تعریف_شدہ SYS_close_range
	واپس syscall(SYS_close_range, پہلا, آخری, flags);
#ورنہ
	errno = ENOSYS;
	واپس -1;
#اگر_ختم
}
#اگر_ختم
#اگر_غیر_تعریف_شدہ CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#اگر_ختم

خالی
cloexec_fds_from(صحیح پہلا)
{
	صحیح i;
	اگر (close_range(پہلا, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		برائے (i = getdtablesize() - 1; i >= پہلا; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
