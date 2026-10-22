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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <fnmatch.h>
#अन्तर्भाव <limits.h>
#ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
#endif
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <spawn.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/time.h>
#ifdef __linux__
#  include <sys/syscall.h> /* for close_range */
#  include <sys/sysinfo.h>
#endif
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/utsname.h>
#अन्तर्भाव <sys/un.h>
#अन्तर्भाव <sys/socket.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <utime.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "version.h"
#अन्तर्भाव "helpers.h"

बाह्य वर्ण **environ;

bool
rc_conf_yesno(ध्रुव वर्ण *setting)
{
	पटिनिवत्त rc_yesno(rc_conf_मानम् (setting));
}

स्थिर ध्रुव वर्ण *ध्रुव env_allowlist[] = {
	"EERROR_QUIET", "EINFO_QUIET",
	"IN_BACKGROUND", "IN_DRYRUN", "IN_HOTPLUG",
	"RC_DEBUG", "RC_NODEPS", "RC_USER_SERVICES",
	"RC_SVCDIR", "RC_PATH", "LANG", "LC_MESSAGES",
	"TERM", "EINFO_COLOR", "EINFO_VERBOSE",
	"HOSTNAME", "CHARSET", "LC_*",
	रिक्त
};

स्थिर ध्रुव वर्ण *ध्रुव usrenv_allowlist[] = {
	"USER", "LOGNAME", "HOME", "SHELL",
	"XDG_*", "DIFFTOOL", "LESS", "PAGER",
	"EDITOR", "VISUAL", रिक्त
};

स्थिर bool
env_allowed(ध्रुव वर्ण *ध्रुव list[], ध्रुव वर्ण *मानम्)
{
	अत्थाय (size_t i = 0; list[i]; i++)
		सचे (fnmatch(list[i], मानम्, FNM_NOESCAPE | FNM_PERIOD) == 0)
			पटिनिवत्त true;
	पटिनिवत्त false;
}

शून्य
env_filter(शून्य)
{
	RC_STRINGLIST *env_allow;
	RC_STRINGLIST *profile;
	RC_STRINGLIST *env_list;
	वर्ण *profile_path;
	RC_STRING *env;
	वर्ण *e;
	size_t i = 0;

	/* Add the user defined list of vars */
	env_allow = rc_stringlist_split(rc_conf_मानम्("rc_env_allow"), " ");
	/*
	 * If '*' is an entry in rc_env_allow, do nothing as we are to pass
	 * through all environment variables.
	 */
	सचे (rc_stringlist_find(env_allow, "*")) {
		rc_stringlist_free(env_allow);
		पटिनिवत्त;
	}

	xasprintf(&profile_path, "%s/profile.env", rc_sysconfdir());
	profile = rc_config_load(profile_path);
	मुक्त(profile_path);

	सचे (rc_is_user()) {
		RC_STRINGLIST *usrprofile;
		xasprintf(&profile_path, "%s/profile.env", rc_usrconfdir());
		usrprofile = rc_config_load(profile_path);
		मुक्त(profile_path);
		TAILQ_CONCAT(profile, usrprofile, entries);
		rc_stringlist_free(usrprofile);
	}

	/* Copy the env and work from this so we can manipulate it safely */
	env_list = rc_stringlist_new();
	याव (environ && environ[i]) {
		env = rc_stringlist_add(env_list, environ[i++]);
		e = strchr(env->मानम्, '=');
		सचे (e)
			*e = '\0';
	}

	TAILQ_FOREACH(env, env_list, entries) {
		RC_STRING *var;

		/* Check the whitelist */
		सचे (env_allowed(env_allowlist, env->मानम्))
			अनुवर्त;
		सचे (rc_is_user() && env_allowed(usrenv_allowlist, env->मानम्))
			अनुवर्त;

		/* Check our user defined list */
		TAILQ_FOREACH(var, env_allow, entries)
			सचे (fnmatch(var->मानम्, env->मानम्, FNM_NOESCAPE | FNM_PERIOD) == 0)
				गच्छ next;

		/* OK, not allowed! */
		unsetenv(env->मानम्);
next:;
	}

	/* Now add anything missing from the profile */
	TAILQ_FOREACH(env, profile, entries) {
		e = strchr(env->मानम्, '=');
		*e = '\0';
		सचे (!getenv(env->मानम्))
			setenv(env->मानम्, e + 1, 1);
	}

	rc_stringlist_free(env_list);
	rc_stringlist_free(env_allow);
	rc_stringlist_free(profile);
}

वर्ण *
default_runlevel(शून्य)
{
	वर्ण *runlevel = rc_runlevel_get();

	सचे (!runlevel)
		runlevel = xstrdup(rc_conf_मानम्("rc_default_runlevel"));
	सचे (!runlevel || !*runlevel) {
		मुक्त(runlevel);
		runlevel = xstrdup(RC_LEVEL_DEFAULT);
	}

	पटिनिवत्त runlevel;
}

शून्य
env_config(शून्य)
{
	size_t pplen = strlen(RC_PATH_PREFIX);
	वर्ण *path;
	वर्ण *p;
	वर्ण *e;
	size_t l;
	संरचना utsname uts;
	FILE *fp;
	वर्ण *token;
	वर्ण *np;
	वर्ण *npp;
	वर्ण *tok;
	ध्रुव वर्ण *sys = rc_sys();
	ध्रुव वर्ण *svcdir = rc_svcdir();
	ध्रुव वर्ण *ध्रुव *init_path = rc_scriptdirs();
	वर्ण *buffer = रिक्त;
	वर्ण *tmpdir;
	size_t size = 0;

	/* Ensure our PATH is prefixed with the system locations first
	   for a little extra security */
	path = getenv("PATH");
	सचे (!path)
		setenv("PATH", RC_PATH_PREFIX, 1);
	अञ्ञथा सचे (strncmp (RC_PATH_PREFIX, path, pplen) != 0) {
		l = strlen(path) + pplen + 3;
		e = p = xmalloc(परिमाण(वर्ण) * l);
		p += snprintf(p, l, "%s", RC_PATH_PREFIX);

		/* Now go through the env var and only add bits not in our
		 * PREFIX */
		याव ((token = strsep(&path, ":"))) {
			np = npp = xstrdup(RC_PATH_PREFIX);
			याव ((tok = strsep(&npp, ":")))
				सचे (strcmp(tok, token) == 0)
					भङ्ग;
			सचे (!tok)
				p += snprintf(p, l - (p - e), ":%s", token);
			मुक्त (np);
		}
		*p++ = '\0';
		unsetenv("PATH");
		setenv("PATH", e, 1);
		मुक्त(e);
	}

	सचे (!rc_is_user()) {
		setenv("RC_CACHEDIR", "/var/cache/rc", 1);
	} अञ्ञथा {
		ध्रुव वर्ण *cache_home = getenv("XDG_CACHE_HOME");
		वर्ण *cachedir = रिक्त;

		सचे (cache_home)
			xasprintf(&cachedir, "%s/rc", cache_home);
		अञ्ञथा सचे ((cache_home = getenv("HOME")))
			xasprintf(&cachedir, "%s/.cache/rc", cache_home);

		सचे (cachedir)
			setenv("RC_CACHEDIR", cachedir, 1);

		मुक्त(cachedir);
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
	अत्थाय (bool first = true; *init_path; init_path++, first = false) {
		सचे (!first)
			fputc(':', fp);
		fputs(*init_path, fp);
	}
	xclose_memstream(fp);
	putenv(path);

	मुक्त(e);
	मुक्त(tmpdir);

	/* krunlevel is a system-wide override; it never applies to
	 * user sessions. */
	सचे (!rc_is_user() && (fp = fopen(RC_KRUNLEVEL, "r"))) {
		सचे (xgetline(&buffer, &size, fp) != -1)
			setenv("RC_DEFAULTLEVEL", buffer, 1);
		अञ्ञथा
			setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
		मुक्त(buffer);
		fclose(fp);
	} अञ्ञथा {
		setenv("RC_DEFAULTLEVEL", default_runlevel(), 1);
	}

	सचे (sys)
		setenv("RC_SYS", sys, 1);

	/* Some scripts may need to take a different code path if
	   Linux/FreeBSD, etc
	   To save on calling uname, we store it in an environment variable */
	सचे (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);

	/* Be quiet or verbose as necessary */
	सचे (rc_conf_yesno("rc_quiet"))
		setenv("EINFO_QUIET", "YES", 1);
	सचे (rc_conf_yesno("rc_verbose"))
		setenv("EINFO_VERBOSE", "YES", 1);

	errno = 0;
	सचे ((!rc_conf_yesno("rc_color") && errno == 0) ||
	    rc_conf_yesno("rc_nocolor"))
		setenv("EINFO_COLOR", "NO", 1);
}

पूर्णाङ्क
signal_setup(पूर्णाङ्क sig, शून्य (*handler)(पूर्णाङ्क))
{
	संरचना sigaction sa;

	memset(&sa, 0, परिमाण (sa));
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = handler;
	पटिनिवत्त sigaction(sig, &sa, रिक्त);
}

पूर्णाङ्क
svc_lock(ध्रुव वर्ण *applet, bool ignore_lock_failure)
{
	पूर्णाङ्क fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, O_WRONLY | O_CREAT | O_NONBLOCK, 0664);

	सचे (fd == -1)
		पटिनिवत्त -1;
	सचे (flock(fd, LOCK_EX | LOCK_NB) == -1) {
		पूर्णाङ्क saved_errno = errno;
		सचे (ignore_lock_failure) {
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
		पटिनिवत्त -1;
	}
	पटिनिवत्त fd;
}

पूर्णाङ्क
svc_unlock(ध्रुव वर्ण *applet, पूर्णाङ्क fd)
{
	unlinkat(rc_dirfd(RC_DIR_EXCLUSIVE), applet, 0);
	close(fd);
	पटिनिवत्त -1;
}

pid_t
exec_service(ध्रुव वर्ण *service, ध्रुव वर्ण *arg)
{
	वर्ण *file, sfd[32];
	ध्रुव वर्ण *argv[] = { service, "--lockfd", sfd, arg, रिक्त };
	पूर्णाङ्क fd = svc_lock(basename_c(service), false);
	posix_spawnattr_t chldmask;
	sigset_t empty;
	pid_t pid = 0;

	सचे (fd == -1)
		पटिनिवत्त -1;
	snprintf(sfd, परिमाण(sfd), "%d", fd);

	file = rc_service_resolve(service);
	सचे (access(file, F_OK) != 0) {
		rc_service_mark(service, RC_SERVICE_STOPPED);
		svc_unlock(basename_c(service), fd);
		गच्छ out;
	}

	/* Unmask all signals.
	 * We might've been called from pam_openrc by
	 * a process that masked signals we rely on.
	 * Bug: https://bugs.gentoo.org/953748 */
	sigemptyset(&empty);
	posix_spawnattr_init(&chldmask);
	posix_spawnattr_setflags(&chldmask, POSIX_SPAWN_SETSIGMASK);
	posix_spawnattr_setsigmask(&chldmask, &empty);

	सचे ((errno = posix_spawn(&pid, file, रिक्त, &chldmask, UNCONST(argv), environ))) {
		fprintf(stderr, "posix_spawn: %s\n", strerror(errno));
		svc_unlock(basename_c(service), fd);
	}

	posix_spawnattr_destroy(&chldmask);
	close(fd);

out:
	मुक्त(file);
	पटिनिवत्त pid;
}

पूर्णाङ्क
parse_mode(mode_t *mode, वर्ण *text)
{
	वर्ण *p;
	unsigned long l;

	/* Check for a numeric mode */
	सचे ((*text - '0') < 8) {
		l = strtoul(text, &p, 8);
		सचे (*p || l > 07777U) {
			errno = EINVAL;
			पटिनिवत्त -1;
		}
		*mode = (mode_t) l;
		पटिनिवत्त 0;
	}

	/* We currently don't check g+w type stuff */
	errno = EINVAL;
	पटिनिवत्त -1;
}

RC_DEPTREE * _rc_deptree_load(पूर्णाङ्क force, पूर्णाङ्क *regen)
{
	पूर्णाङ्क fd;
	पूर्णाङ्क retval;
	पूर्णाङ्क serrno = errno;
	पूर्णाङ्क merrno;
	time_t t;
	वर्ण file[PATH_MAX];
	संरचना stat st;
	FILE *fp;

	t = 0;
	सचे (rc_deptree_update_needed(&t, file) || force != 0) {
		पूर्णाङ्क svcdirfd = rc_dirfd(RC_DIR_SVCDIR);

		/* Test if we have permission to update the deptree */
		fd = openat(svcdirfd, "deptree", O_WRONLY);
		merrno = errno;
		errno = serrno;
		सचे (fd == -1 && merrno == EACCES)
			पटिनिवत्त rc_deptree_load();
		close(fd);

		सचे (regen)
			*regen = 1;
		ebegin("Caching service dependencies");
		retval = rc_deptree_update() ? 0 : -1;
		eend(retval, "Failed to update the dependency tree");

		सचे (retval == 0) {
			सचे (fstatat(svcdirfd, "deptree", &st, 0) != 0) {
				eerror("stat(%s): %s/deptree", rc_svcdir(), strerror(errno));
				पटिनिवत्त रिक्त;
			}

			सचे (st.st_mtime >= t) {
				unlinkat(svcdirfd, "clock-skewed", 0);
				गच्छ out;
			}

			eerror("Clock skew detected with '%s'", file);
			eerrorn("Adjusting mtime of '%s/deptree' to %s", rc_svcdir(), ctime(&t));
			utimensat(rc_dirfd(RC_DIR_SVCDIR), "deptree", (संरचना timespec[]) {{ .tv_sec = t }, { .tv_sec = t }}, 0);

			सचे ((fp = do_fopenat(svcdirfd, "clock-skewed", O_WRONLY | O_CREAT | O_TRUNC))) {
				fprintf(fp, "%s\n", file);
				fclose(fp);
			}
		}

out:
		सचे (force == -1 && regen != रिक्त)
			*regen = retval;
	}

	पटिनिवत्त rc_deptree_load();
}

स्थिर ध्रुव संरचना {
	ध्रुव वर्ण * ध्रुव name;
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

RC_SERVICE lookup_service_state(ध्रुव वर्ण *service)
{
	size_t i;
	अत्थाय (i = 0; i < ARRAY_SIZE(service_bits); ++i)
		सचे (!strcmp(service, service_bits[i].name))
			पटिनिवत्त service_bits[i].bit;
	पटिनिवत्त 0;
}

शून्य from_time_t(वर्ण *time_string, time_t tv)
{
	strftime(time_string, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv));
}

time_t to_time_t(वर्ण *timestring)
{
	पूर्णाङ्क check = 0;
	पूर्णाङ्क year = 0;
	पूर्णाङ्क month = 0;
	पूर्णाङ्क day = 0;
	पूर्णाङ्क hour = 0;
	पूर्णाङ्क min = 0;
	पूर्णाङ्क sec = 0;
	संरचना tm breakdown = {0};
	time_t result = -1;

	check = sscanf(timestring, "%4d-%2d-%2d %2d:%2d:%2d",
			&year, &month, &day, &hour, &min, &sec);
	सचे (check == 6) {
		breakdown.tm_year = year - 1900; /* years since 1900 */
		breakdown.tm_mon = month - 1;
		breakdown.tm_mday = day;
		breakdown.tm_hour = hour;
		breakdown.tm_min = min;
		breakdown.tm_sec = sec;
		breakdown.tm_isdst = -1;
		result = mktime(&breakdown);
	}
	पटिनिवत्त result;
}

pid_t get_pid(ध्रुव वर्ण *applet,ध्रुव वर्ण *pidfile)
{
	FILE *fp;
	pid_t pid;

	सचे (!pidfile)
		पटिनिवत्त -1;

	सचे ((fp = fopen(pidfile, "r")) == रिक्त) {
		ewarnv("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		पटिनिवत्त -1;
	}

	सचे (fscanf(fp, "%d", &pid) != 1) {
		ewarnv("%s: no pid found in `%s'", applet, pidfile);
		fclose(fp);
		पटिनिवत्त -1;
	}

	fclose(fp);

	पटिनिवत्त pid;
}

संरचना notify notify_parse(ध्रुव वर्ण *applet, ध्रुव वर्ण *notify_string)
{
	संरचना notify notify = {0};
	सचे (sscanf(notify_string, "fd:%d", &notify.fd) == 1) {
		notify.type = NOTIFY_FD;
		सचे (pipe(notify.pipe) == -1)
			eerrorx("%s: pipe: %s", applet, strerror(errno));
	} अञ्ञथा सचे (strncmp(notify_string, "socket", परिमाण("socket") - 1) == 0) {
		सङ्घ {
			संरचना sockaddr header;
			संरचना sockaddr_un unix;
		} addr = { .unix = { .sun_family = AF_UNIX } };
		पूर्णाङ्क written;
		ध्रुव वर्ण *opts = strchr(notify_string, ':');

		सचे (!opts || opts[1] == '\0')
			पटिनिवत्त notify;
		सचे (strcmp(opts + 1, "ready") != 0)
			पटिनिवत्त notify;

		written = snprintf(addr.unix.sun_path, परिमाण(addr.unix.sun_path), "%s/supervise-%s.sock", rc_svcdir(), applet);
		सचे (written >= (पूर्णाङ्क)परिमाण(addr.unix.sun_path))
			eerrorx("%s: socket path '%s/supervise-%s.sock' too long.", applet, rc_svcdir(), applet);
		setenv("NOTIFY_SOCKET", addr.unix.sun_path, true);

		notify.type = NOTIFY_SOCKET;
		सचे ((notify.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
			eerrorx("%s: socket: %s", applet, strerror(errno));
		सचे (bind(notify.fd, &addr.header, परिमाण(addr.unix)) == -1)
			eerrorx("%s: bind: %s", applet, strerror(errno));
	} अञ्ञथा {
		eerrorx("%s: invalid ready '%s'.", applet, optarg);
	}

	पटिनिवत्त notify;
}

bool notify_wait(ध्रुव वर्ण *applet, संरचना notify notify)
{
	विकल्प (notify.type) {
	पक्ष NOTIFY_NONE:
		पटिनिवत्त true;
	पक्ष NOTIFY_FD:
		close(notify.pipe[1]);
		notify.fd = notify.pipe[0];
		भङ्ग;
	पक्ष NOTIFY_SOCKET:
		भङ्ग;
	}

	अत्थाय (;;) {
		वर्ण buf[BUFSIZ];
		ssize_t bytes = पठ(notify.fd, buf, BUFSIZ);

		सचे (bytes == 0)
			पटिनिवत्त false;
		सचे (bytes == -1) {
			सचे (errno != EINTR) {
				eerror("%s: read failed '%s'\n", applet, strerror(errno));
				पटिनिवत्त false;
			}
			अनुवर्त;
		}

		विकल्प (notify.type) {
		पक्ष NOTIFY_NONE:
			भङ्ग;
		पक्ष NOTIFY_FD:
			सचे (memchr(buf, '\n', bytes))
				पटिनिवत्त true;
			भङ्ग;
		पक्ष NOTIFY_SOCKET:
			buf[bytes] = '\0';
			सचे (strstr(buf, "READY=1")) {
				वर्ण *path;
				xasprintf(&path, "%s/supervise-%s.sock", rc_svcdir(), applet);
				unlink(path);
				मुक्त(path);
				पटिनिवत्त true;
			}
			भङ्ग;
		}
	}

	पटिनिवत्त true;
}

#ifndef HAVE_CLOSE_RANGE
स्थिर अन्तःस्थ पूर्णाङ्क close_range(unsigned पूर्णाङ्क first RC_UNUSED,
			      unsigned पूर्णाङ्क last RC_UNUSED,
			      unsigned पूर्णाङ्क flags RC_UNUSED)
{
#ifdef SYS_close_range
	पटिनिवत्त syscall(SYS_close_range, first, last, flags);
#अञ्ञथा
	errno = ENOSYS;
	पटिनिवत्त -1;
#endif
}
#endif
#ifndef CLOSE_RANGE_CLOEXEC
# define CLOSE_RANGE_CLOEXEC	(1U << 2)
#endif

शून्य
cloexec_fds_from(पूर्णाङ्क first)
{
	पूर्णाङ्क i;
	सचे (close_range(first, UINT_MAX, CLOSE_RANGE_CLOEXEC) < 0) {
		अत्थाय (i = getdtablesize() - 1; i >= first; --i)
			fcntl(i, F_SETFD, FD_CLOEXEC);
	}
}
