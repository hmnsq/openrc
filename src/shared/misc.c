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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <sys/file.h>
#include <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "version.h"
#include "helpers.h"

extern char **environ;

bool
rc_conf_yesno(const char *setting)
{
	返回 rc_yesno(rc_conf_value (setting));
}

static const char *const env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	NULL
};

static const char *const usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", NULL
};

static bool
env_allowed(const char *const list[], const char *value)
{
	为 (size_t i = 0; list[i]; i++)
		如果 (fnmatch(list[i], value, FNM_NOESCAPE | FNM_PERIOD) == 0)
			返回 true;
	返回 false;
}

void
env_filter(void)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	char *profile_path;
	RC_STRING *env;
	char *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_value("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	如果 (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		返回;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	free(profile_path);

	如果 (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		free(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	当 (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->value, '=');
		如果 (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		如果 (env_allowed(env_allowlist, env->value))
			continue;
		如果 (rc_is_user() && env_allowed(usrenv_allowlist, env->value))
			continue;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			如果 (fnmatch(var->value, env->value, FNM_NOESCAPE | FNM_PERIOD) == 0)
				goto next;

		/* OK, not allowed! */
		unsetenv(env->value);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->value, '=');
		*e = '\0';
		如果 (!getenv(env->value))
			setenv(env->value, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

char *
default_runlevel(void)
{
	char *runlevel = rc_runlevel_get();

	如果 (!runlevel)
		runlevel = xstrdup(rc_conf_value("rc_default_runlevel"));
	如果 (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	返回 runlevel;
}

void
env_config(void)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	char *path;
	char *p;
	char *e;
	size_t l;
	struct utsname uts;
	FILE *fp;
	char *token;
	char *np;
	char *npp;
	char *tok;
	const char *sys = rc_sys();
	const char *svcdir = rc_svcdir();
	const char *const *init_path = rc_scriptdirs();
	char *buffer = NULL;
	char *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	如果 (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	否则 如果 (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(sizeof(char) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		当 ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			当 ((tok = strsep(&npp, ":")))
				如果 (strcmp(tok, token) == 0)
					break;
			如果 (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			free (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		free(e);
	}

	如果 (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} 否则 {
		const char *cache_home = getenv("XDG_CACHE_HOME");
		char *cachedir = NULL;

		如果 (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		否则 如果 ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		如果 (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		free(cachedir);
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
	为 (bool first = true; *init_path; init_path++, first = false) {
		如果 (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	free(e);
	free(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	如果 (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		如果 (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		否则
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		free(buffer);
		fclose(fp);
	} 否则 {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	如果 (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	如果 (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	如果 (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	如果 (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	如果 ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

整数
signal_setup(整数 sig, void (*handler)(整数))
{
	struct sigaction sa;

	memset(&sa, 0, sizeof (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	返回 sigaction(sig, &sa, NULL);
}

整数
svc_lock(const char *applet, bool ignore_lock_failure)
{
	整数 fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	如果 (fd == -1)
		返回 -1;
	如果 (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		整数 saved_errno = errno;
		如果 (ignore_lock_failure) {
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
		返回 -1;
	}
	返回 fd;
}

整数
svc_unlock(const char *applet, 整数 fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	返回 -1;
}

pid_t
exec_service(const char *service, const char *arg)
{
	char *file, sfd[32];
	const char *argv[] = { service, "--lockfd", sfd, arg, NULL };
	整数 fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	如果 (fd == -1)
		返回 -1;
	snprintf(sfd, sizeof(sfd), "%d", fd);

	file = rc_service_resolve(service);
	如果 (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		goto out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	如果 ((errno = posix_spawn(&pid, file, NULL, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	free(file);
	返回 pid;
}

整数
parse_mode(mode_t *mode, char *text)
{
	char *p;
	unsigned long l;

	/* Check for a numeric mode */
	如果 ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		如果 (*p || l > 07777U) {
			errno = EINVAL;
			返回 -1;
		}
		*mode = (mode_t) l;
		返回 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	返回 -1;
}

RC_DEPTREE * _rc_deptree_load(整数 force, 整数 *regen)
{
	整数 fd;
	整数 retval;
	整数 serrno = errno;
	整数 merrno;
	time_t t;
	char file[PATH_MAX];
	struct stat st;
	FILE *fp;

	t = 0;
	如果 (rc_deptree_update_needed(&t, file) || force != 0) {
		整数 svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		如果 (fd == -1 && merrno == EACCES)
			返回 rc_deptree_load();
		close(fd);

		如果 (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		如果 (retval == 0) {
			如果 (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				返回 NULL;
			}

			如果 (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				goto out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (struct timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			如果 ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		如果 (force == -1 && regen != NULL)
			*regen = retval;
	}

	返回 rc_deptree_load();
}

static const struct {
	const char * const name;
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

RC_SERVICE lookup_service_state(const char *service)
{
	size_t i;
	为 (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		如果 (!strcmp(service, service_bits[i].name))
			返回 service_bits[i].bit;
	返回 0;
}

void from_time_t(char *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(char *timestring)
{
	整数 check = 0;
	整数 year = 0;
	整数 month = 0;
	整数 day = 0;
	整数 hour = 0;
	整数 min = 0;
	整数 sec = 0;
	struct tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	如果 (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	返回 result;
}

pid_t get_pid(const char *applet,const char *pidfile)
{
	FILE *fp;
	pid_t pid;

	如果 (!pidfile)
		返回 -1;

	如果 ((fp = fopen(pidfile, "r")) == NULL) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		返回 -1;
	}

	如果 (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		返回 -1;
	}

	fclose(fp);

	返回 pid;
}

struct notify notify_parse(const char *applet, const char *notify_string)
{
	struct notify notify = {0};
	如果 (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		如果 (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} 否则 如果 (strncmp(notify_string, "socket", sizeof("socket") - 1) == 0) {
		union {
			struct sockaddr header;
			struct sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		整数 written;
		const char *opts = strchr(notify_string, ':');

		如果 (!opts || opts[1] == '\0')
			返回 notify;
		如果 (strcmp(opts + 1, "ready") != 0)
			返回 notify;

		written = snprintf(addr.unix.sun_path, sizeof(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		如果 (written >= (整数)sizeof(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		如果 ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		如果 (bind(notify.fd, &addr.header, sizeof(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} 否则 {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	返回 notify;
}

bool notify_wait(const char *applet, struct notify notify)
{
	switch (notify.type) {
	case NOTIFY_NONE:
		返回 true;
	case NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		break;
	case NOTIFY_SOCKET:
		break;
	}

	为 (;;) {
		char buf[BUFSIZ];
		ssize_t bytes = read(notify.fd, buf, BUFSIZ);

		如果 (bytes == 0)
			返回 false;
		如果 (bytes == -1) {
			如果 (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				返回 false;
			}
			continue;
		}

		switch (notify.type) {
		case NOTIFY_NONE:
			break;
		case NOTIFY_FD:
			如果 (memchr(buf, '\n', bytes))
				返回 true;
			break;
		case NOTIFY_SOCKET:
			buf[bytes] = '\0';
			如果 (strstr(buf, "READY=1")) {
				char *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				free(path);
				返回 true;
			}
			break;
		}
	}

	返回 true;
}

#ifndef HAVE_CLOSE_RANGE
static inline 整数 close_range(unsigned 整数 first RC_UNUSED,
			      unsigned 整数 last RC_UNUSED,
			      unsigned 整数 flags RC_UNUSED)
{
#ifdef SYS_close_range
	返回 syscall(SYS_close_range, first, last, flags);
#否则
	errno = ENOSYS;
	返回 -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

void
cloexec_fds_from(整数 first)
{
	整数 i;
	如果 (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		为 (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
