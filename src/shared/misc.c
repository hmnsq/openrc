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

#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <fnmatch.h>
#അന്തര്ഭാവ <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <spawn.h>
#അന്തര്ഭാവ <sys/file.h>
#അന്തര്ഭാവ <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/utsname.h>
#അന്തര്ഭാവ <sys/un.h>
#അന്തര്ഭാവ <sys/socket.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <utime.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "version.h"
#അന്തര്ഭാവ "helpers.h"

ബാഹ്യ അക്ഷരം **environ;

bool
rc_conf_yesno(സ്ഥിരം അക്ഷരം *setting)
{
	തിരികെ rc_yesno(rc_conf_മൂല്യം (setting));
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ശൂന്യ
};

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ശൂന്യ
};

സ്ഥിതം bool
env_allowed(സ്ഥിരം അക്ഷരം *സ്ഥിരം list[], സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	വേണ്ടി (size_t i = 0; list[i]; i++)
		എങ്കിൽ (fnmatch(list[i], മൂല്യം, FNM_NOESCAPE | FNM_PERIOD) == 0)
			തിരികെ true;
	തിരികെ false;
}

ശൂന്യം
env_filter(ശൂന്യം)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	അക്ഷരം *profile_path;
	RC_STRING *env;
	അക്ഷരം *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_മൂല്യം("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	എങ്കിൽ (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		തിരികെ;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	സ്വതന്ത്രം(profile_path);

	എങ്കിൽ (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		സ്വതന്ത്രം(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	ആകുമ്പോൾ (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->മൂല്യം, '=');
		എങ്കിൽ (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		എങ്കിൽ (env_allowed(env_allowlist, env->മൂല്യം))
			തുടരുക;
		എങ്കിൽ (rc_is_user() && env_allowed(usrenv_allowlist, env->മൂല്യം))
			തുടരുക;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			എങ്കിൽ (fnmatch(var->മൂല്യം, env->മൂല്യം, FNM_NOESCAPE | FNM_PERIOD) == 0)
				പോകുക next;

		/* OK, not allowed! */
		unsetenv(env->മൂല്യം);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->മൂല്യം, '=');
		*e = '\0';
		എങ്കിൽ (!getenv(env->മൂല്യം))
			setenv(env->മൂല്യം, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

അക്ഷരം *
default_runlevel(ശൂന്യം)
{
	അക്ഷരം *runlevel = rc_runlevel_get();

	എങ്കിൽ (!runlevel)
		runlevel = xstrdup(rc_conf_മൂല്യം("rc_default_runlevel"));
	എങ്കിൽ (!runlevel || !*runlevel) {
		സ്വതന്ത്രം(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	തിരികെ runlevel;
}

ശൂന്യം
env_config(ശൂന്യം)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	അക്ഷരം *path;
	അക്ഷരം *p;
	അക്ഷരം *e;
	size_t l;
	ഘടന utsname uts;
	FILE *fp;
	അക്ഷരം *token;
	അക്ഷരം *np;
	അക്ഷരം *npp;
	അക്ഷരം *tok;
	സ്ഥിരം അക്ഷരം *sys = rc_sys();
	സ്ഥിരം അക്ഷരം *svcdir = rc_svcdir();
	സ്ഥിരം അക്ഷരം *സ്ഥിരം *init_path = rc_scriptdirs();
	അക്ഷരം *buffer = ശൂന്യ;
	അക്ഷരം *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	എങ്കിൽ (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	അല്ലെങ്കിൽ എങ്കിൽ (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(വലുപ്പം(അക്ഷരം) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		ആകുമ്പോൾ ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			ആകുമ്പോൾ ((tok = strsep(&npp, ":")))
				എങ്കിൽ (strcmp(tok, token) == 0)
					നിർത്തുക;
			എങ്കിൽ (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			സ്വതന്ത്രം (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		സ്വതന്ത്രം(e);
	}

	എങ്കിൽ (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} അല്ലെങ്കിൽ {
		സ്ഥിരം അക്ഷരം *cache_home = getenv("XDG_CACHE_HOME");
		അക്ഷരം *cachedir = ശൂന്യ;

		എങ്കിൽ (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		അല്ലെങ്കിൽ എങ്കിൽ ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		എങ്കിൽ (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		സ്വതന്ത്രം(cachedir);
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
	വേണ്ടി (bool first = true; *init_path; init_path++, first = false) {
		എങ്കിൽ (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	സ്വതന്ത്രം(e);
	സ്വതന്ത്രം(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	എങ്കിൽ (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		എങ്കിൽ (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		അല്ലെങ്കിൽ
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		സ്വതന്ത്രം(buffer);
		fclose(fp);
	} അല്ലെങ്കിൽ {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	എങ്കിൽ (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	എങ്കിൽ (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	എങ്കിൽ (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	എങ്കിൽ (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	എങ്കിൽ ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

പൂർണ്ണസംഖ്യ
signal_setup(പൂർണ്ണസംഖ്യ sig, ശൂന്യം (*handler)(പൂർണ്ണസംഖ്യ))
{
	ഘടന sigaction sa;

	memset(&sa, 0, വലുപ്പം (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	തിരികെ sigaction(sig, &sa, ശൂന്യ);
}

പൂർണ്ണസംഖ്യ
svc_lock(സ്ഥിരം അക്ഷരം *applet, bool ignore_lock_failure)
{
	പൂർണ്ണസംഖ്യ fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	എങ്കിൽ (fd == -1)
		തിരികെ -1;
	എങ്കിൽ (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		പൂർണ്ണസംഖ്യ saved_errno = errno;
		എങ്കിൽ (ignore_lock_failure) {
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
		തിരികെ -1;
	}
	തിരികെ fd;
}

പൂർണ്ണസംഖ്യ
svc_unlock(സ്ഥിരം അക്ഷരം *applet, പൂർണ്ണസംഖ്യ fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	തിരികെ -1;
}

pid_t
exec_service(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *arg)
{
	അക്ഷരം *file, sfd[32];
	സ്ഥിരം അക്ഷരം *argv[] = { service, "--lockfd", sfd, arg, ശൂന്യ };
	പൂർണ്ണസംഖ്യ fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	എങ്കിൽ (fd == -1)
		തിരികെ -1;
	snprintf(sfd, വലുപ്പം(sfd), "%d", fd);

	file = rc_service_resolve(service);
	എങ്കിൽ (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		പോകുക out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	എങ്കിൽ ((errno = posix_spawn(&pid, file, ശൂന്യ, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	സ്വതന്ത്രം(file);
	തിരികെ pid;
}

പൂർണ്ണസംഖ്യ
parse_mode(mode_t *mode, അക്ഷരം *text)
{
	അക്ഷരം *p;
	unsigned long l;

	/* Check for a numeric mode */
	എങ്കിൽ ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		എങ്കിൽ (*p || l > 07777U) {
			errno = EINVAL;
			തിരികെ -1;
		}
		*mode = (mode_t) l;
		തിരികെ 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	തിരികെ -1;
}

RC_DEPTREE * _rc_deptree_load(പൂർണ്ണസംഖ്യ force, പൂർണ്ണസംഖ്യ *regen)
{
	പൂർണ്ണസംഖ്യ fd;
	പൂർണ്ണസംഖ്യ retval;
	പൂർണ്ണസംഖ്യ serrno = errno;
	പൂർണ്ണസംഖ്യ merrno;
	time_t t;
	അക്ഷരം file[PATH_MAX];
	ഘടന stat st;
	FILE *fp;

	t = 0;
	എങ്കിൽ (rc_deptree_update_needed(&t, file) || force != 0) {
		പൂർണ്ണസംഖ്യ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		എങ്കിൽ (fd == -1 && merrno == EACCES)
			തിരികെ rc_deptree_load();
		close(fd);

		എങ്കിൽ (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		എങ്കിൽ (retval == 0) {
			എങ്കിൽ (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				തിരികെ ശൂന്യ;
			}

			എങ്കിൽ (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				പോകുക out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (ഘടന timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			എങ്കിൽ ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		എങ്കിൽ (force == -1 && regen != ശൂന്യ)
			*regen = retval;
	}

	തിരികെ rc_deptree_load();
}

സ്ഥിതം സ്ഥിരം ഘടന {
	സ്ഥിരം അക്ഷരം * സ്ഥിരം name;
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

RC_SERVICE lookup_service_state(സ്ഥിരം അക്ഷരം *service)
{
	size_t i;
	വേണ്ടി (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		എങ്കിൽ (!strcmp(service, service_bits[i].name))
			തിരികെ service_bits[i].bit;
	തിരികെ 0;
}

ശൂന്യം from_time_t(അക്ഷരം *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(അക്ഷരം *timestring)
{
	പൂർണ്ണസംഖ്യ check = 0;
	പൂർണ്ണസംഖ്യ year = 0;
	പൂർണ്ണസംഖ്യ month = 0;
	പൂർണ്ണസംഖ്യ day = 0;
	പൂർണ്ണസംഖ്യ hour = 0;
	പൂർണ്ണസംഖ്യ min = 0;
	പൂർണ്ണസംഖ്യ sec = 0;
	ഘടന tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	എങ്കിൽ (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	തിരികെ result;
}

pid_t get_pid(സ്ഥിരം അക്ഷരം *applet,സ്ഥിരം അക്ഷരം *pidfile)
{
	FILE *fp;
	pid_t pid;

	എങ്കിൽ (!pidfile)
		തിരികെ -1;

	എങ്കിൽ ((fp = fopen(pidfile, "r")) == ശൂന്യ) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		തിരികെ -1;
	}

	എങ്കിൽ (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		തിരികെ -1;
	}

	fclose(fp);

	തിരികെ pid;
}

ഘടന notify notify_parse(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *notify_string)
{
	ഘടന notify notify = {0};
	എങ്കിൽ (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		എങ്കിൽ (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} അല്ലെങ്കിൽ എങ്കിൽ (strncmp(notify_string, "socket", വലുപ്പം("socket") - 1) == 0) {
		സംഘം {
			ഘടന sockaddr header;
			ഘടന sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		പൂർണ്ണസംഖ്യ written;
		സ്ഥിരം അക്ഷരം *opts = strchr(notify_string, ':');

		എങ്കിൽ (!opts || opts[1] == '\0')
			തിരികെ notify;
		എങ്കിൽ (strcmp(opts + 1, "ready") != 0)
			തിരികെ notify;

		written = snprintf(addr.unix.sun_path, വലുപ്പം(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		എങ്കിൽ (written >= (പൂർണ്ണസംഖ്യ)വലുപ്പം(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		എങ്കിൽ ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		എങ്കിൽ (bind(notify.fd, &addr.header, വലുപ്പം(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} അല്ലെങ്കിൽ {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	തിരികെ notify;
}

bool notify_wait(സ്ഥിരം അക്ഷരം *applet, ഘടന notify notify)
{
	തിരഞ്ഞെടുപ്പ് (notify.type) {
	അവസ്ഥ NOTIFY_NONE:
		തിരികെ true;
	അവസ്ഥ NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		നിർത്തുക;
	അവസ്ഥ NOTIFY_SOCKET:
		നിർത്തുക;
	}

	വേണ്ടി (;;) {
		അക്ഷരം buf[BUFSIZ];
		ssize_t bytes = പഠ(notify.fd, buf, BUFSIZ);

		എങ്കിൽ (bytes == 0)
			തിരികെ false;
		എങ്കിൽ (bytes == -1) {
			എങ്കിൽ (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				തിരികെ false;
			}
			തുടരുക;
		}

		തിരഞ്ഞെടുപ്പ് (notify.type) {
		അവസ്ഥ NOTIFY_NONE:
			നിർത്തുക;
		അവസ്ഥ NOTIFY_FD:
			എങ്കിൽ (memchr(buf, '\n', bytes))
				തിരികെ true;
			നിർത്തുക;
		അവസ്ഥ NOTIFY_SOCKET:
			buf[bytes] = '\0';
			എങ്കിൽ (strstr(buf, "READY=1")) {
				അക്ഷരം *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				സ്വതന്ത്രം(path);
				തിരികെ true;
			}
			നിർത്തുക;
		}
	}

	തിരികെ true;
}

#ifndef HAVE_CLOSE_RANGE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ close_range(unsigned പൂർണ്ണസംഖ്യ first RC_UNUSED,
			      unsigned പൂർണ്ണസംഖ്യ last RC_UNUSED,
			      unsigned പൂർണ്ണസംഖ്യ flags RC_UNUSED)
{
#ifdef SYS_close_range
	തിരികെ syscall(SYS_close_range, first, last, flags);
#അല്ലെങ്കിൽ
	errno = ENOSYS;
	തിരികെ -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

ശൂന്യം
cloexec_fds_from(പൂർണ്ണസംഖ്യ first)
{
	പൂർണ്ണസംഖ്യ i;
	എങ്കിൽ (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		വേണ്ടി (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
