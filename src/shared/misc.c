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

#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <fnmatch.h>
#அந்தர்பாவ <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <spawn.h>
#அந்தர்பாவ <sys/file.h>
#அந்தர்பாவ <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/utsname.h>
#அந்தர்பாவ <sys/un.h>
#அந்தர்பாவ <sys/socket.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <utime.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "version.h"
#அந்தர்பாவ "helpers.h"

வெளி எழுத்து **environ;

bool
rc_conf_yesno(மாறிலி எழுத்து *setting)
{
	திரும்பு rc_yesno(rc_conf_மதிப்பு (setting));
}

நிலையான மாறிலி எழுத்து *மாறிலி env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	வெற்று
};

நிலையான மாறிலி எழுத்து *மாறிலி usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", வெற்று
};

நிலையான bool
env_allowed(மாறிலி எழுத்து *மாறிலி list[], மாறிலி எழுத்து *மதிப்பு)
{
	க்கு (size_t i = 0; list[i]; i++)
		எனில் (fnmatch(list[i], மதிப்பு, FNM_NOESCAPE | FNM_PERIOD) == 0)
			திரும்பு true;
	திரும்பு false;
}

வெறுமை
env_filter(வெறுமை)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	எழுத்து *profile_path;
	RC_STRING *env;
	எழுத்து *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_மதிப்பு("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	எனில் (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		திரும்பு;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	விடுவி(profile_path);

	எனில் (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		விடுவி(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	போது (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->மதிப்பு, '=');
		எனில் (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		எனில் (env_allowed(env_allowlist, env->மதிப்பு))
			தொடர்;
		எனில் (rc_is_user() && env_allowed(usrenv_allowlist, env->மதிப்பு))
			தொடர்;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			எனில் (fnmatch(var->மதிப்பு, env->மதிப்பு, FNM_NOESCAPE | FNM_PERIOD) == 0)
				செல் next;

		/* OK, not allowed! */
		unsetenv(env->மதிப்பு);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->மதிப்பு, '=');
		*e = '\0';
		எனில் (!getenv(env->மதிப்பு))
			setenv(env->மதிப்பு, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

எழுத்து *
default_runlevel(வெறுமை)
{
	எழுத்து *runlevel = rc_runlevel_get();

	எனில் (!runlevel)
		runlevel = xstrdup(rc_conf_மதிப்பு("rc_default_runlevel"));
	எனில் (!runlevel || !*runlevel) {
		விடுவி(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	திரும்பு runlevel;
}

வெறுமை
env_config(வெறுமை)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	எழுத்து *path;
	எழுத்து *p;
	எழுத்து *e;
	size_t l;
	கட்டமைப்பு utsname uts;
	FILE *fp;
	எழுத்து *token;
	எழுத்து *np;
	எழுத்து *npp;
	எழுத்து *tok;
	மாறிலி எழுத்து *sys = rc_sys();
	மாறிலி எழுத்து *svcdir = rc_svcdir();
	மாறிலி எழுத்து *மாறிலி *init_path = rc_scriptdirs();
	எழுத்து *buffer = வெற்று;
	எழுத்து *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	எனில் (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	இல்லையெனில் எனில் (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(அளவு(எழுத்து) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		போது ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			போது ((tok = strsep(&npp, ":")))
				எனில் (strcmp(tok, token) == 0)
					நிறுத்து;
			எனில் (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			விடுவி (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		விடுவி(e);
	}

	எனில் (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} இல்லையெனில் {
		மாறிலி எழுத்து *cache_home = getenv("XDG_CACHE_HOME");
		எழுத்து *cachedir = வெற்று;

		எனில் (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		இல்லையெனில் எனில் ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		எனில் (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		விடுவி(cachedir);
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
	க்கு (bool first = true; *init_path; init_path++, first = false) {
		எனில் (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	விடுவி(e);
	விடுவி(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	எனில் (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		எனில் (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		இல்லையெனில்
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		விடுவி(buffer);
		fclose(fp);
	} இல்லையெனில் {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	எனில் (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	எனில் (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	எனில் (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	எனில் (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	எனில் ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

முழு
signal_setup(முழு sig, வெறுமை (*handler)(முழு))
{
	கட்டமைப்பு sigaction sa;

	memset(&sa, 0, அளவு (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	திரும்பு sigaction(sig, &sa, வெற்று);
}

முழு
svc_lock(மாறிலி எழுத்து *applet, bool ignore_lock_failure)
{
	முழு fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	எனில் (fd == -1)
		திரும்பு -1;
	எனில் (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		முழு saved_errno = errno;
		எனில் (ignore_lock_failure) {
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
		திரும்பு -1;
	}
	திரும்பு fd;
}

முழு
svc_unlock(மாறிலி எழுத்து *applet, முழு fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	திரும்பு -1;
}

pid_t
exec_service(மாறிலி எழுத்து *service, மாறிலி எழுத்து *arg)
{
	எழுத்து *file, sfd[32];
	மாறிலி எழுத்து *argv[] = { service, "--lockfd", sfd, arg, வெற்று };
	முழு fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	எனில் (fd == -1)
		திரும்பு -1;
	snprintf(sfd, அளவு(sfd), "%d", fd);

	file = rc_service_resolve(service);
	எனில் (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		செல் out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	எனில் ((errno = posix_spawn(&pid, file, வெற்று, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	விடுவி(file);
	திரும்பு pid;
}

முழு
parse_mode(mode_t *mode, எழுத்து *text)
{
	எழுத்து *p;
	unsigned long l;

	/* Check for a numeric mode */
	எனில் ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		எனில் (*p || l > 07777U) {
			errno = EINVAL;
			திரும்பு -1;
		}
		*mode = (mode_t) l;
		திரும்பு 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	திரும்பு -1;
}

RC_DEPTREE * _rc_deptree_load(முழு force, முழு *regen)
{
	முழு fd;
	முழு retval;
	முழு serrno = errno;
	முழு merrno;
	time_t t;
	எழுத்து file[PATH_MAX];
	கட்டமைப்பு stat st;
	FILE *fp;

	t = 0;
	எனில் (rc_deptree_update_needed(&t, file) || force != 0) {
		முழு svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		எனில் (fd == -1 && merrno == EACCES)
			திரும்பு rc_deptree_load();
		close(fd);

		எனில் (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		எனில் (retval == 0) {
			எனில் (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				திரும்பு வெற்று;
			}

			எனில் (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				செல் out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (கட்டமைப்பு timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			எனில் ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		எனில் (force == -1 && regen != வெற்று)
			*regen = retval;
	}

	திரும்பு rc_deptree_load();
}

நிலையான மாறிலி கட்டமைப்பு {
	மாறிலி எழுத்து * மாறிலி name;
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

RC_SERVICE lookup_service_state(மாறிலி எழுத்து *service)
{
	size_t i;
	க்கு (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		எனில் (!strcmp(service, service_bits[i].name))
			திரும்பு service_bits[i].bit;
	திரும்பு 0;
}

வெறுமை from_time_t(எழுத்து *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(எழுத்து *timestring)
{
	முழு check = 0;
	முழு year = 0;
	முழு month = 0;
	முழு day = 0;
	முழு hour = 0;
	முழு min = 0;
	முழு sec = 0;
	கட்டமைப்பு tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	எனில் (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	திரும்பு result;
}

pid_t get_pid(மாறிலி எழுத்து *applet,மாறிலி எழுத்து *pidfile)
{
	FILE *fp;
	pid_t pid;

	எனில் (!pidfile)
		திரும்பு -1;

	எனில் ((fp = fopen(pidfile, "r")) == வெற்று) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		திரும்பு -1;
	}

	எனில் (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		திரும்பு -1;
	}

	fclose(fp);

	திரும்பு pid;
}

கட்டமைப்பு notify notify_parse(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *notify_string)
{
	கட்டமைப்பு notify notify = {0};
	எனில் (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		எனில் (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} இல்லையெனில் எனில் (strncmp(notify_string, "socket", அளவு("socket") - 1) == 0) {
		ஒன்றிப்பு {
			கட்டமைப்பு sockaddr header;
			கட்டமைப்பு sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		முழு written;
		மாறிலி எழுத்து *opts = strchr(notify_string, ':');

		எனில் (!opts || opts[1] == '\0')
			திரும்பு notify;
		எனில் (strcmp(opts + 1, "ready") != 0)
			திரும்பு notify;

		written = snprintf(addr.unix.sun_path, அளவு(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		எனில் (written >= (முழு)அளவு(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		எனில் ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		எனில் (bind(notify.fd, &addr.header, அளவு(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} இல்லையெனில் {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	திரும்பு notify;
}

bool notify_wait(மாறிலி எழுத்து *applet, கட்டமைப்பு notify notify)
{
	தேர்வு (notify.type) {
	நிலை NOTIFY_NONE:
		திரும்பு true;
	நிலை NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		நிறுத்து;
	நிலை NOTIFY_SOCKET:
		நிறுத்து;
	}

	க்கு (;;) {
		எழுத்து buf[BUFSIZ];
		ssize_t bytes = பட(notify.fd, buf, BUFSIZ);

		எனில் (bytes == 0)
			திரும்பு false;
		எனில் (bytes == -1) {
			எனில் (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				திரும்பு false;
			}
			தொடர்;
		}

		தேர்வு (notify.type) {
		நிலை NOTIFY_NONE:
			நிறுத்து;
		நிலை NOTIFY_FD:
			எனில் (memchr(buf, '\n', bytes))
				திரும்பு true;
			நிறுத்து;
		நிலை NOTIFY_SOCKET:
			buf[bytes] = '\0';
			எனில் (strstr(buf, "READY=1")) {
				எழுத்து *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				விடுவி(path);
				திரும்பு true;
			}
			நிறுத்து;
		}
	}

	திரும்பு true;
}

#ifndef HAVE_CLOSE_RANGE
நிலையான உள்வரி முழு close_range(unsigned முழு first RC_UNUSED,
			      unsigned முழு last RC_UNUSED,
			      unsigned முழு flags RC_UNUSED)
{
#ifdef SYS_close_range
	திரும்பு syscall(SYS_close_range, first, last, flags);
#இல்லையெனில்
	errno = ENOSYS;
	திரும்பு -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

வெறுமை
cloexec_fds_from(முழு first)
{
	முழு i;
	எனில் (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		க்கு (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
