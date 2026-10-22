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

extern var_na **environ;

bool
rc_conf_yesno(const var_na *setting)
{
	ပြန်ပေး rc_yesno(rc_conf_value (setting));
}

static const var_na *const env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	NULL
};

static const var_na *const usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", NULL
};

static bool
env_allowed(const var_na *const list[], const var_na *value)
{
	အတွက် (size_t i = 0; list[i]; i++)
		အကယ်၍ (fnmatch(list[i], value, FNM_NOESCAPE | FNM_PERIOD) == 0)
			ပြန်ပေး true;
	ပြန်ပေး false;
}

void
env_filter(void)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	var_na *profile_path;
	RC_STRING *env;
	var_na *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_value("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	အကယ်၍ (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		ပြန်ပေး;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	free(profile_path);

	အကယ်၍ (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		free(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	နေစဉ် (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->value, '=');
		အကယ်၍ (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		အကယ်၍ (env_allowed(env_allowlist, env->value))
			continue;
		အကယ်၍ (rc_is_user() && env_allowed(usrenv_allowlist, env->value))
			continue;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			အကယ်၍ (fnmatch(var->value, env->value, FNM_NOESCAPE | FNM_PERIOD) == 0)
				goto next;

		/* OK, not allowed! */
		unsetenv(env->value);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->value, '=');
		*e = '\0';
		အကယ်၍ (!getenv(env->value))
			setenv(env->value, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

var_na *
default_runlevel(void)
{
	var_na *runlevel = rc_runlevel_get();

	အကယ်၍ (!runlevel)
		runlevel = xstrdup(rc_conf_value("rc_default_runlevel"));
	အကယ်၍ (!runlevel || !*runlevel) {
		free(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	ပြန်ပေး runlevel;
}

void
env_config(void)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	var_na *path;
	var_na *p;
	var_na *e;
	size_t l;
	sa_nracanaa utsname uts;
	FILE *fp;
	var_na *token;
	var_na *np;
	var_na *npp;
	var_na *tok;
	const var_na *sys = rc_sys();
	const var_na *svcdir = rc_svcdir();
	const var_na *const *init_path = rc_scriptdirs();
	var_na *buffer = NULL;
	var_na *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	အကယ်၍ (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	မဟုတ်ပါက အကယ်၍ (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(parimaa_na(var_na) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		နေစဉ် ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			နေစဉ် ((tok = strsep(&npp, ":")))
				အကယ်၍ (strcmp(tok, token) == 0)
					bha_nga;
			အကယ်၍ (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			free (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		free(e);
	}

	အကယ်၍ (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} မဟုတ်ပါက {
		const var_na *cache_home = getenv("XDG_CACHE_HOME");
		var_na *cachedir = NULL;

		အကယ်၍ (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		မဟုတ်ပါက အကယ်၍ ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		အကယ်၍ (cachedir)
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
	အတွက် (bool first = true; *init_path; init_path++, first = false) {
		အကယ်၍ (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	free(e);
	free(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	အကယ်၍ (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		အကယ်၍ (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		မဟုတ်ပါက
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		free(buffer);
		fclose(fp);
	} မဟုတ်ပါက {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	အကယ်၍ (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	အကယ်၍ (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	အကယ်၍ (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	အကယ်၍ (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	အကယ်၍ ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

ကိန်းပြည့်
signal_setup(ကိန်းပြည့် sig, void (*handler)(ကိန်းပြည့်))
{
	sa_nracanaa sigaction sa;

	memset(&sa, 0, parimaa_na (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	ပြန်ပေး sigaction(sig, &sa, NULL);
}

ကိန်းပြည့်
svc_lock(const var_na *applet, bool ignore_lock_failure)
{
	ကိန်းပြည့် fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	အကယ်၍ (fd == -1)
		ပြန်ပေး -1;
	အကယ်၍ (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		ကိန်းပြည့် saved_errno = errno;
		အကယ်၍ (ignore_lock_failure) {
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
		ပြန်ပေး -1;
	}
	ပြန်ပေး fd;
}

ကိန်းပြည့်
svc_unlock(const var_na *applet, ကိန်းပြည့် fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	ပြန်ပေး -1;
}

pid_t
exec_service(const var_na *service, const var_na *arg)
{
	var_na *file, sfd[32];
	const var_na *argv[] = { service, "--lockfd", sfd, arg, NULL };
	ကိန်းပြည့် fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	အကယ်၍ (fd == -1)
		ပြန်ပေး -1;
	snprintf(sfd, parimaa_na(sfd), "%d", fd);

	file = rc_service_resolve(service);
	အကယ်၍ (access(file, F_OK) != 0) {
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

	အကယ်၍ ((errno = posix_spawn(&pid, file, NULL, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	free(file);
	ပြန်ပေး pid;
}

ကိန်းပြည့်
parse_mode(mode_t *mode, var_na *text)
{
	var_na *p;
	unsigned long l;

	/* Check for a numeric mode */
	အကယ်၍ ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		အကယ်၍ (*p || l > 07777U) {
			errno = EINVAL;
			ပြန်ပေး -1;
		}
		*mode = (mode_t) l;
		ပြန်ပေး 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	ပြန်ပေး -1;
}

RC_DEPTREE * _rc_deptree_load(ကိန်းပြည့် force, ကိန်းပြည့် *regen)
{
	ကိန်းပြည့် fd;
	ကိန်းပြည့် retval;
	ကိန်းပြည့် serrno = errno;
	ကိန်းပြည့် merrno;
	time_t t;
	var_na file[PATH_MAX];
	sa_nracanaa stat st;
	FILE *fp;

	t = 0;
	အကယ်၍ (rc_deptree_update_needed(&t, file) || force != 0) {
		ကိန်းပြည့် svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		အကယ်၍ (fd == -1 && merrno == EACCES)
			ပြန်ပေး rc_deptree_load();
		close(fd);

		အကယ်၍ (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		အကယ်၍ (retval == 0) {
			အကယ်၍ (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				ပြန်ပေး NULL;
			}

			အကယ်၍ (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				goto out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (sa_nracanaa timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			အကယ်၍ ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		အကယ်၍ (force == -1 && regen != NULL)
			*regen = retval;
	}

	ပြန်ပေး rc_deptree_load();
}

static const sa_nracanaa {
	const var_na * const name;
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

RC_SERVICE lookup_service_state(const var_na *service)
{
	size_t i;
	အတွက် (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		အကယ်၍ (!strcmp(service, service_bits[i].name))
			ပြန်ပေး service_bits[i].bit;
	ပြန်ပေး 0;
}

void from_time_t(var_na *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(var_na *timestring)
{
	ကိန်းပြည့် check = 0;
	ကိန်းပြည့် year = 0;
	ကိန်းပြည့် month = 0;
	ကိန်းပြည့် day = 0;
	ကိန်းပြည့် hour = 0;
	ကိန်းပြည့် min = 0;
	ကိန်းပြည့် sec = 0;
	sa_nracanaa tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	အကယ်၍ (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	ပြန်ပေး result;
}

pid_t get_pid(const var_na *applet,const var_na *pidfile)
{
	FILE *fp;
	pid_t pid;

	အကယ်၍ (!pidfile)
		ပြန်ပေး -1;

	အကယ်၍ ((fp = fopen(pidfile, "r")) == NULL) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		ပြန်ပေး -1;
	}

	အကယ်၍ (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		ပြန်ပေး -1;
	}

	fclose(fp);

	ပြန်ပေး pid;
}

sa_nracanaa notify notify_parse(const var_na *applet, const var_na *notify_string)
{
	sa_nracanaa notify notify = {0};
	အကယ်၍ (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		အကယ်၍ (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} မဟုတ်ပါက အကယ်၍ (strncmp(notify_string, "socket", parimaa_na("socket") - 1) == 0) {
		sa_ngha {
			sa_nracanaa sockaddr header;
			sa_nracanaa sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		ကိန်းပြည့် written;
		const var_na *opts = strchr(notify_string, ':');

		အကယ်၍ (!opts || opts[1] == '\0')
			ပြန်ပေး notify;
		အကယ်၍ (strcmp(opts + 1, "ready") != 0)
			ပြန်ပေး notify;

		written = snprintf(addr.unix.sun_path, parimaa_na(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		အကယ်၍ (written >= (ကိန်းပြည့်)parimaa_na(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		အကယ်၍ ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		အကယ်၍ (bind(notify.fd, &addr.header, parimaa_na(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} မဟုတ်ပါက {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	ပြန်ပေး notify;
}

bool notify_wait(const var_na *applet, sa_nracanaa notify notify)
{
	switch (notify.type) {
	pak_sa NOTIFY_NONE:
		ပြန်ပေး true;
	pak_sa NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		bha_nga;
	pak_sa NOTIFY_SOCKET:
		bha_nga;
	}

	အတွက် (;;) {
		var_na buf[BUFSIZ];
		ssize_t bytes = read(notify.fd, buf, BUFSIZ);

		အကယ်၍ (bytes == 0)
			ပြန်ပေး false;
		အကယ်၍ (bytes == -1) {
			အကယ်၍ (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				ပြန်ပေး false;
			}
			continue;
		}

		switch (notify.type) {
		pak_sa NOTIFY_NONE:
			bha_nga;
		pak_sa NOTIFY_FD:
			အကယ်၍ (memchr(buf, '\n', bytes))
				ပြန်ပေး true;
			bha_nga;
		pak_sa NOTIFY_SOCKET:
			buf[bytes] = '\0';
			အကယ်၍ (strstr(buf, "READY=1")) {
				var_na *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				free(path);
				ပြန်ပေး true;
			}
			bha_nga;
		}
	}

	ပြန်ပေး true;
}

#ifndef HAVE_CLOSE_RANGE
static anta_hstha ကိန်းပြည့် close_range(unsigned ကိန်းပြည့် first RC_UNUSED,
			      unsigned ကိန်းပြည့် last RC_UNUSED,
			      unsigned ကိန်းပြည့် flags RC_UNUSED)
{
#ifdef SYS_close_range
	ပြန်ပေး syscall(SYS_close_range, first, last, flags);
#မဟုတ်ပါက
	errno = ENOSYS;
	ပြန်ပေး -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

void
cloexec_fds_from(ကိန်းပြည့် first)
{
	ကိန်းပြည့် i;
	အကယ်၍ (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		အတွက် (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
