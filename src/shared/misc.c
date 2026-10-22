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

#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <fnmatch.h>
#અન્તર્ભાવ <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <spawn.h>
#અન્તર્ભાવ <sys/file.h>
#અન્તર્ભાવ <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/utsname.h>
#અન્તર્ભાવ <sys/un.h>
#અન્તર્ભાવ <sys/socket.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <utime.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "version.h"
#અન્તર્ભાવ "helpers.h"

બાહ્ય અક્ષર **environ;

bool
rc_conf_yesno(અચલ અક્ષર *setting)
{
	પાછા rc_yesno(rc_conf_કિંમત (setting));
}

સ્થિર અચલ અક્ષર *અચલ env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	ખાલી
};

સ્થિર અચલ અક્ષર *અચલ usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", ખાલી
};

સ્થિર bool
env_allowed(અચલ અક્ષર *અચલ list[], અચલ અક્ષર *કિંમત)
{
	માટે (size_t i = 0; list[i]; i++)
		જો (fnmatch(list[i], કિંમત, FNM_NOESCAPE | FNM_PERIOD) == 0)
			પાછા true;
	પાછા false;
}

શૂન્ય
env_filter(શૂન્ય)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	અક્ષર *profile_path;
	RC_STRING *env;
	અક્ષર *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_કિંમત("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	જો (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		પાછા;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	મુક્ત(profile_path);

	જો (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		મુક્ત(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	જ્યારે (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->કિંમત, '=');
		જો (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		જો (env_allowed(env_allowlist, env->કિંમત))
			ચાલુ;
		જો (rc_is_user() && env_allowed(usrenv_allowlist, env->કિંમત))
			ચાલુ;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			જો (fnmatch(var->કિંમત, env->કિંમત, FNM_NOESCAPE | FNM_PERIOD) == 0)
				જાઓ next;

		/* OK, not allowed! */
		unsetenv(env->કિંમત);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->કિંમત, '=');
		*e = '\0';
		જો (!getenv(env->કિંમત))
			setenv(env->કિંમત, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

અક્ષર *
default_runlevel(શૂન્ય)
{
	અક્ષર *runlevel = rc_runlevel_get();

	જો (!runlevel)
		runlevel = xstrdup(rc_conf_કિંમત("rc_default_runlevel"));
	જો (!runlevel || !*runlevel) {
		મુક્ત(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	પાછા runlevel;
}

શૂન્ય
env_config(શૂન્ય)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	અક્ષર *path;
	અક્ષર *p;
	અક્ષર *e;
	size_t l;
	સંરચના utsname uts;
	FILE *fp;
	અક્ષર *token;
	અક્ષર *np;
	અક્ષર *npp;
	અક્ષર *tok;
	અચલ અક્ષર *sys = rc_sys();
	અચલ અક્ષર *svcdir = rc_svcdir();
	અચલ અક્ષર *અચલ *init_path = rc_scriptdirs();
	અક્ષર *buffer = ખાલી;
	અક્ષર *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	જો (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	નહીંતર જો (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(કદ(અક્ષર) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		જ્યારે ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			જ્યારે ((tok = strsep(&npp, ":")))
				જો (strcmp(tok, token) == 0)
					તોડો;
			જો (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			મુક્ત (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		મુક્ત(e);
	}

	જો (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} નહીંતર {
		અચલ અક્ષર *cache_home = getenv("XDG_CACHE_HOME");
		અક્ષર *cachedir = ખાલી;

		જો (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		નહીંતર જો ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		જો (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		મુક્ત(cachedir);
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
	માટે (bool first = true; *init_path; init_path++, first = false) {
		જો (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	મુક્ત(e);
	મુક્ત(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	જો (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		જો (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		નહીંતર
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		મુક્ત(buffer);
		fclose(fp);
	} નહીંતર {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	જો (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	જો (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	જો (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	જો (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	જો ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

પૂર્ણાંક
signal_setup(પૂર્ણાંક sig, શૂન્ય (*handler)(પૂર્ણાંક))
{
	સંરચના sigaction sa;

	memset(&sa, 0, કદ (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	પાછા sigaction(sig, &sa, ખાલી);
}

પૂર્ણાંક
svc_lock(અચલ અક્ષર *applet, bool ignore_lock_failure)
{
	પૂર્ણાંક fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	જો (fd == -1)
		પાછા -1;
	જો (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		પૂર્ણાંક saved_errno = errno;
		જો (ignore_lock_failure) {
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
		પાછા -1;
	}
	પાછા fd;
}

પૂર્ણાંક
svc_unlock(અચલ અક્ષર *applet, પૂર્ણાંક fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	પાછા -1;
}

pid_t
exec_service(અચલ અક્ષર *service, અચલ અક્ષર *arg)
{
	અક્ષર *file, sfd[32];
	અચલ અક્ષર *argv[] = { service, "--lockfd", sfd, arg, ખાલી };
	પૂર્ણાંક fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	જો (fd == -1)
		પાછા -1;
	snprintf(sfd, કદ(sfd), "%d", fd);

	file = rc_service_resolve(service);
	જો (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		જાઓ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	જો ((errno = posix_spawn(&pid, file, ખાલી, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	મુક્ત(file);
	પાછા pid;
}

પૂર્ણાંક
parse_mode(mode_t *mode, અક્ષર *text)
{
	અક્ષર *p;
	unsigned long l;

	/* Check for a numeric mode */
	જો ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		જો (*p || l > 07777U) {
			errno = EINVAL;
			પાછા -1;
		}
		*mode = (mode_t) l;
		પાછા 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	પાછા -1;
}

RC_DEPTREE * _rc_deptree_load(પૂર્ણાંક force, પૂર્ણાંક *regen)
{
	પૂર્ણાંક fd;
	પૂર્ણાંક retval;
	પૂર્ણાંક serrno = errno;
	પૂર્ણાંક merrno;
	time_t t;
	અક્ષર file[PATH_MAX];
	સંરચના stat st;
	FILE *fp;

	t = 0;
	જો (rc_deptree_update_needed(&t, file) || force != 0) {
		પૂર્ણાંક svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		જો (fd == -1 && merrno == EACCES)
			પાછા rc_deptree_load();
		close(fd);

		જો (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		જો (retval == 0) {
			જો (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				પાછા ખાલી;
			}

			જો (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				જાઓ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (સંરચના timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			જો ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		જો (force == -1 && regen != ખાલી)
			*regen = retval;
	}

	પાછા rc_deptree_load();
}

સ્થિર અચલ સંરચના {
	અચલ અક્ષર * અચલ name;
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

RC_SERVICE lookup_service_state(અચલ અક્ષર *service)
{
	size_t i;
	માટે (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		જો (!strcmp(service, service_bits[i].name))
			પાછા service_bits[i].bit;
	પાછા 0;
}

શૂન્ય from_time_t(અક્ષર *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(અક્ષર *timestring)
{
	પૂર્ણાંક check = 0;
	પૂર્ણાંક year = 0;
	પૂર્ણાંક month = 0;
	પૂર્ણાંક day = 0;
	પૂર્ણાંક hour = 0;
	પૂર્ણાંક min = 0;
	પૂર્ણાંક sec = 0;
	સંરચના tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	જો (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	પાછા result;
}

pid_t get_pid(અચલ અક્ષર *applet,અચલ અક્ષર *pidfile)
{
	FILE *fp;
	pid_t pid;

	જો (!pidfile)
		પાછા -1;

	જો ((fp = fopen(pidfile, "r")) == ખાલી) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		પાછા -1;
	}

	જો (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		પાછા -1;
	}

	fclose(fp);

	પાછા pid;
}

સંરચના notify notify_parse(અચલ અક્ષર *applet, અચલ અક્ષર *notify_string)
{
	સંરચના notify notify = {0};
	જો (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		જો (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} નહીંતર જો (strncmp(notify_string, "socket", કદ("socket") - 1) == 0) {
		સંઘ {
			સંરચના sockaddr header;
			સંરચના sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		પૂર્ણાંક written;
		અચલ અક્ષર *opts = strchr(notify_string, ':');

		જો (!opts || opts[1] == '\0')
			પાછા notify;
		જો (strcmp(opts + 1, "ready") != 0)
			પાછા notify;

		written = snprintf(addr.unix.sun_path, કદ(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		જો (written >= (પૂર્ણાંક)કદ(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		જો ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		જો (bind(notify.fd, &addr.header, કદ(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} નહીંતર {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	પાછા notify;
}

bool notify_wait(અચલ અક્ષર *applet, સંરચના notify notify)
{
	પસંદગી (notify.type) {
	સ્થિતિ NOTIFY_NONE:
		પાછા true;
	સ્થિતિ NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		તોડો;
	સ્થિતિ NOTIFY_SOCKET:
		તોડો;
	}

	માટે (;;) {
		અક્ષર buf[BUFSIZ];
		ssize_t bytes = પઠ(notify.fd, buf, BUFSIZ);

		જો (bytes == 0)
			પાછા false;
		જો (bytes == -1) {
			જો (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				પાછા false;
			}
			ચાલુ;
		}

		પસંદગી (notify.type) {
		સ્થિતિ NOTIFY_NONE:
			તોડો;
		સ્થિતિ NOTIFY_FD:
			જો (memchr(buf, '\n', bytes))
				પાછા true;
			તોડો;
		સ્થિતિ NOTIFY_SOCKET:
			buf[bytes] = '\0';
			જો (strstr(buf, "READY=1")) {
				અક્ષર *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				મુક્ત(path);
				પાછા true;
			}
			તોડો;
		}
	}

	પાછા true;
}

#ifndef HAVE_CLOSE_RANGE
સ્થિર અંતઃરેખ પૂર્ણાંક close_range(unsigned પૂર્ણાંક first RC_UNUSED,
			      unsigned પૂર્ણાંક last RC_UNUSED,
			      unsigned પૂર્ણાંક flags RC_UNUSED)
{
#ifdef SYS_close_range
	પાછા syscall(SYS_close_range, first, last, flags);
#નહીંતર
	errno = ENOSYS;
	પાછા -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

શૂન્ય
cloexec_fds_from(પૂર્ણાંક first)
{
	પૂર્ણાંક i;
	જો (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		માટે (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
