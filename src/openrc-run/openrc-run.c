/*
 * openrc-run.c
 * Handle launching of init scripts.
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
#include <getopt.h>
#include <libgen.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>

#nếu defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#không_thì
#  include <libutil.h>
#endif

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "rc_exec.h"
#include "misc.h"
#include "timeutils.h"
#include "plugin.h"
#include "selinux.h"
#include "_usage.h"
#include "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

extern char **environ;

const char *applet = NULL;
const char *extraopts = "stop | start | restart | status | describe | zap";
const char getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
const struct option longopts[] = {
	{ "debug",      0, NULL, 'd'},
	{ "dry-run",    0, NULL, 'Z'},
	{ "ifstarted",  0, NULL, 's'},
	{ "ifstopped",  0, NULL, 'S'},
	{ "nodeps",     0, NULL, 'D'},
	{ "lockfd",     1, NULL, 'l'},
	longopts_COMMON
};
const char *const longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
const char *usagestring = NULL;

static char *service, *runlevel, *ibsave, *prefix;
static RC_DEPTREE *deptree;
static RC_STRINGLIST *applet_list, *services, *tmplist;
static RC_STRINGLIST *restart_services;
static RC_STRINGLIST *need_services;
static RC_STRINGLIST *use_services;
static RC_STRINGLIST *want_services;
static số_nguyên exclusive_fd = -1, master_tty = -1;
static bool in_background, deps, dry_run;
static volatile bool sighup, skip_mark, timedout;
static pid_t service_pid;
static số_nguyên signal_pipe[2] = { -1, -1 };

static RC_STRINGLIST *deptypes_b;	/* broken deps */
static RC_STRINGLIST *deptypes_n;	/* needed deps */
static RC_STRINGLIST *deptypes_nw;	/* need+want deps */
static RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
static RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
static RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
static RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

static void
handle_signal(số_nguyên sig)
{
	số_nguyên serrno = errno, status;
	pid_t pid;
	const char *signame = NULL;
	struct winsize ws;

	switch (sig) {
	case SIGHUP:
		sighup = true;
		break;

	case SIGUSR1:
		skip_mark = true;
		break;

	case SIGCHLD:
		trong_khi ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			nếu (signal_pipe[1] > -1 && pid == service_pid) {
				nếu (write(signal_pipe[1], &status, sizeof(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		break;

	case SIGWINCH:
		nếu (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		break;

	case SIGINT:
		nếu (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	case SIGTERM:
		nếu (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	case SIGQUIT:
		nếu (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		nếu (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	case SIGALRM:
		timedout = true;
		break;

	default:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

static void
unhotplug(void)
{
	nếu (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

static void
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	nếu (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		trả_về;

	TAILQ_FOREACH(svc, list, entries) {
		nếu (!(rc_service_state(svc->value) & RC_SERVICE_STOPPED))
			continue;
		nếu (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->value);
			ewarn("WARNING: %s will start when %s has started", svc->value, applet);
		} không_thì {
			service_start(svc->value);
		}
	}
}

static void
restore_state(void)
{
	RC_SERVICE state;

	nếu (rc_in_plugin || exclusive_fd == -1)
		trả_về;
	state = rc_service_state(applet);
	nếu (state & RC_SERVICE_STOPPING) {
		nếu (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		không_thì
			rc_service_mark(applet, RC_SERVICE_STARTED);

		nếu (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} không_thì nếu (state & RC_SERVICE_STARTING) {
		nếu (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		không_thì
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		nếu (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

static void
cleanup(void)
{
	restore_state();

	nếu (!rc_in_plugin) {
		nếu (restart_services)
			start_services(restart_services);
	}

	rc_plugin_unload();

	rc_stringlist_free(deptypes_b);
	rc_stringlist_free(deptypes_n);
	rc_stringlist_free(deptypes_nw);
	rc_stringlist_free(deptypes_nwu);
	rc_stringlist_free(deptypes_nwua);
	rc_stringlist_free(deptypes_m);
	rc_stringlist_free(deptypes_mwua);
	rc_deptree_free(deptree);
	rc_stringlist_free(restart_services);
	rc_stringlist_free(need_services);
	rc_stringlist_free(use_services);
	rc_stringlist_free(want_services);
	rc_stringlist_free(services);
	rc_stringlist_free(applet_list);
	rc_stringlist_free(tmplist);
	free(ibsave);
	free(prefix);
	free(runlevel);
	free(service);
}

static void
svc_getenv(const char *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	char *var = NULL;

	TAILQ_FOREACH(reexport, reexports, entries) {
		nếu (rc_service_getenv(svc, reexport->value, &var) == -1)
			continue;
		setenv(reexport->value, var, true);
	}

	free(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
static số_nguyên
write_prefix(const char *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	const char *ec = ecolor(ECOLOR_HILITE);
	const char *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	số_nguyên fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	nếu (lock_fd != -1) {
		trong_khi (flock(lock_fd, LOCK_EX) != 0) {
			nếu (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				break;
			}
		}
	}
	không_thì
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	cho (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		nếu (buffer[i] == '\033' && !*prefixed) {
			cho (j = i + 1; j < bytes; j++) {
				nếu (buffer[j] == 'A')
					*prefixed = true;
				nếu (isalpha((unsigned số_nguyên)buffer[j]))
					break;
			}
		}

		nếu (!*prefixed) {
			ret += write(fd, ec, strlen(ec));
			ret += write(fd, prefix, strlen(prefix));
			ret += write(fd, ec_normal, strlen(ec_normal));
			ret += write(fd, "|", 1);
			*prefixed = true;
		}

		nếu (buffer[i] == '\n')
			*prefixed = false;
		ret += write(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	trả_về ret;
}

static số_nguyên
svc_exec(const char *command)
{
	số_nguyên ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	struct termios tt;
	struct winsize ws;
	số_nguyên flags = 0;
	struct pollfd fd[2];
	bool prefixed = false;
	số_nguyên slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	char *openrc_sh = NULL;
	const char *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		NULL
	};

	/* Setup our signal pipe */
	nếu (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		trả_về 1;
	}

	nếu ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		trả_về 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	nếu (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		nếu (openpty(&master_tty, &slave_tty, NULL, &tt, &ws) == 0) {
			nếu (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			nếu (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			nếu ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				trả_về 1;
			}
		}
	}

	nếu (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	nếu ((errno = posix_spawn(&service_pid, argv[0], &tty, NULL, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		trả_về 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	free(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	cho (;;) {
		nếu (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			nếu (errno == EINTR)
				continue;
			eerror("%s: poll: %s", applet, strerror(errno));
			break;
		}

		nếu (fd[1].revents & (POLLIN | POLLHUP)) {
			char buffer[BUFSIZ];
			write_prefix(buffer, read(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		nếu (fd[0].revents & (POLLIN | POLLHUP)) {
			nếu (errno = 0, read(signal_pipe[0], &ret, sizeof(ret)) < (ssize_t)sizeof(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			không_thì nếu ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			break;
		}
	}

	sigemptyset (&sigchldmask);
	sigaddset (&sigchldmask, SIGCHLD);
	sigprocmask (SIG_BLOCK, &sigchldmask, &oldmask);

	close(signal_pipe[0]);
	/* We should set signal_pipe[1] to -1 before closing it to avoid invalid write if
	 * sighandler runs between close() and cleaning the variable */
	tmp = signal_pipe[1];
	signal_pipe[0] = signal_pipe[1] = -1;
	close(tmp);

	sigprocmask (SIG_SETMASK, &oldmask, NULL);

	nếu (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	trả_về ret;
}

static bool
svc_wait(const char *svc)
{
	số_nguyên fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	const char *base = basename_c(svc);
	số_nguyên timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	nếu (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	nếu ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		trả_về errno == ENOENT;

	timedout = false;
	cho (;;) {
		nếu (!forever)
			alarm(WARN_TIMEOUT);

		nếu (flock(fd, LOCK_SH) == 0) {
			retval = true;
			break;
		}

		nếu (errno != EINTR)
			break;

		nếu (!forever && timedout) {
			timedout = false;
			nếu ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			không_thì
				break;
		}
	}

	alarm(0);
	close(fd);
	trả_về retval;
}

static void
get_started_services(void)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	free(tmp);
}

static void
setup_deptypes(void)
{
	deptypes_b = rc_stringlist_new();
	rc_stringlist_add(deptypes_b, "broken");

	deptypes_n = rc_stringlist_new();
	rc_stringlist_add(deptypes_n, "ineed");

	deptypes_nw = rc_stringlist_new();
	rc_stringlist_add(deptypes_nw, "ineed");
	rc_stringlist_add(deptypes_nw, "iwant");

	deptypes_nwu = rc_stringlist_new();
	rc_stringlist_add(deptypes_nwu, "ineed");
	rc_stringlist_add(deptypes_nwu, "iwant");
	rc_stringlist_add(deptypes_nwu, "iuse");

	deptypes_nwua = rc_stringlist_new();
	rc_stringlist_add(deptypes_nwua, "ineed");
	rc_stringlist_add(deptypes_nwua, "iwant");
	rc_stringlist_add(deptypes_nwua, "iuse");
	rc_stringlist_add(deptypes_nwua, "iafter");

	deptypes_m = rc_stringlist_new();
	rc_stringlist_add(deptypes_m, "needsme");

	deptypes_mwua = rc_stringlist_new();
	rc_stringlist_add(deptypes_mwua, "needsme");
	rc_stringlist_add(deptypes_mwua, "wantsme");
	rc_stringlist_add(deptypes_mwua, "usesme");
	rc_stringlist_add(deptypes_mwua, "beforeme");
}

static void
svc_start_check(void)
{
	số_nguyên ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	nếu (in_background) {
		nếu (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			goto exit;

		nếu (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		nếu (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	nếu (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		goto exit;
	} không_thì nếu (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	nếu (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	nếu (exclusive_fd == -1) {
		nếu (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			goto exit;
		}

		nếu (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		không_thì
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	trả_về;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

static void
svc_start_deps(void)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	số_nguyên depoptions = RC_DEP_TRACE;
	size_t len, svc_count;
	char *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	nếu (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	nếu (!deptree && ((deptree = _rc_deptree_load(0, NULL)) == NULL)) {
		eerror("failed to load deptree");
		goto exit;
	}

	nếu (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	nếu (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			nếu (first)
				first = false;
			không_thì
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->value);
		}
		fprintf(stderr, "\n");
		goto exit;
	}
	rc_stringlist_free(services);
	services = NULL;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	nếu (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->value);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			nếu (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				continue;
			nếu (state & RC_SERVICE_STOPPED) {
				nếu (dry_run) {
					printf(" %s", svc->value);
					continue;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->value);
				nếu (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	nếu (dry_run)
		trả_về;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->value);
		nếu (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->value);
			continue;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		nếu (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			nếu (!rc_stringlist_find(need_services, svc->value) &&
					!rc_stringlist_find(want_services, svc->value) &&
					!rc_stringlist_find(use_services, svc->value))
				continue;
		}

		nếu (!svc_wait(svc->value))
			eerror("%s: timed out waiting for %s", applet, svc->value);
		state = rc_service_state(svc->value);
		nếu (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->value);
			continue;
		}
		nếu (rc_stringlist_find(need_services, svc->value)) {
			nếu (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->value);
			} không_thì nếu (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->value);
				goto exit;
			}
		}
	}

	nếu (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = NULL;
		mem = xopen_memstream(&tmp, &len);
		svc_count = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->value, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->value);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->value, applet);
			rc_stringlist_free(use_services);
			use_services = NULL;
			nếu (svc_count > 0)
				fputs(", ", mem);
			fputs(svc->value, mem);
			++svc_count;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = NULL;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_count > 1 ? "have" : "has");
		free(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = NULL;
	rc_stringlist_free(services);
	services = NULL;

	trả_về;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


static void set_reexports(void)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	char *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->value);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

static void svc_start_real(void)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	nếu (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	nếu (ibsave)
		unsetenv("IN_BACKGROUND");

	nếu (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	nếu (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	không_thì nếu (!started)
		eerrorx("ERROR: %s failed to start", applet);

	nếu (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    nếu (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
		    service_start(svc->value);
	rc_stringlist_free(services);
	services = NULL;

	/* Do the same for any services we provide */
	nếu (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->value);
			TAILQ_FOREACH(svc2, services, entries)
				nếu (rc_service_state(svc2->value) & RC_SERVICE_STOPPED)
					service_start(svc2->value);
			rc_stringlist_free(services);
			services = NULL;
		}
		rc_stringlist_free(tmplist);
		tmplist = NULL;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

static số_nguyên
svc_start(void)
{
	nếu (dry_run)
		einfon("start:");
	không_thì
		svc_start_check();

	nếu (deps)
		svc_start_deps();

	nếu (dry_run)
		printf(" %s\n", applet);
	không_thì
		svc_start_real();

	trả_về 0;
}

static số_nguyên
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	nếu (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		goto exit;

	nếu (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		goto exit;

	nếu (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		trả_về 1;
	}

	nếu (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	nếu (exclusive_fd == -1) {
		nếu (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		không_thì nếu (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		không_thì
			eerror("ERROR: %s stopped by something else", applet);
		goto exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	nếu (!rc_runlevel_stopping()) {
		nếu (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		không_thì nếu (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	trả_về 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

static void
svc_stop_deps(RC_SERVICE state)
{
	số_nguyên depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	nếu (state & RC_SERVICE_WASINACTIVE)
		trả_về;

	errno = 0;
	nếu (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	nếu (!deptree && ((deptree = _rc_deptree_load(0, NULL)) == NULL)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	nếu (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->value);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		nếu (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			continue;
		nếu (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			continue;
		nếu (dry_run) {
			printf(" %s", svc->value);
			continue;
		}
		svc_wait(svc->value);
		nếu (rc_service_state(svc->value) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->value);
			nếu (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->value);
		}
	}
	rc_stringlist_free(services);
	services = NULL;
	nếu (dry_run)
		trả_về;

	TAILQ_FOREACH(svc, tmplist, entries) {
		nếu (rc_service_state(svc->value) & RC_SERVICE_STOPPED) {
			continue;
		}
		svc_wait(svc->value);
		nếu (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
			continue;
		nếu (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			nếu (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				continue;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->value);
	}
	rc_stringlist_free(tmplist);
	tmplist = NULL;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		nếu (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
			continue;
		svc_wait(svc->value);
	}
	rc_stringlist_free(services);
	services = NULL;
}

static void
load_dep_env(void)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->value);

	rc_stringlist_free(depsvcs);
}

static void
svc_stop_real(void)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	nếu (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	nếu (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	nếu (ibsave)
		unsetenv("IN_BACKGROUND");

	nếu (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	nếu (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

static số_nguyên
svc_stop(void)
{
	RC_SERVICE state = 0;

	nếu (dry_run)
		einfon("stop:");
	không_thì
		nếu (svc_stop_check(&state) == 1)
			trả_về 1; /* Service has been stopped already */

	nếu (deps)
		svc_stop_deps(state);

	nếu (dry_run)
		printf(" %s\n", applet);
	không_thì
		svc_stop_real();

	trả_về 0;
}

static số_nguyên
svc_restart(void)
{
	nếu (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		nếu (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = NULL;

	trả_về 0;
}

static bool
service_plugable(void)
{
	char *list, *p, *token;
	bool allow = true, truefalse;
	char *match = rc_conf_value("rc_hotplug");

	nếu (!match)
		match = rc_conf_value("rc_plug_services");
	nếu (!match)
		trả_về false;

	list = xstrdup(match);
	p = list;
	trong_khi ((token = strsep(&p, " "))) {
		nếu (token[0] == '!') {
			truefalse = false;
			token++;
		} không_thì {
			truefalse = true;
		}

		nếu (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			break;
		}
	}
	free(list);
	trả_về allow;
}

static số_nguyên
simple_command(void)
{
	char *save = prefix;
	số_nguyên retval;

	eprefix(NULL);
	prefix = NULL;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	trả_về retval;
}

static số_nguyên
resolve_deptype(void)
{
	số_nguyên depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	nếu (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	nếu (!deptree && ((deptree = _rc_deptree_load(0, NULL)) == NULL))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = NULL;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->value);
	printf ("\n");

	rc_stringlist_free(services);
	services = NULL;

	trả_về 0;
}

static số_nguyên
cmd_condrestart(void)
{
	nếu (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	trả_về 0;
}

static số_nguyên
cmd_stop(void)
{
	RC_STRING *svc;

	nếu (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	nếu (deps && in_background)
		get_started_services();

	nếu (svc_stop() == 1)
		trả_về 0; /* Service has been stopped already */

	nếu (deps) {
		nếu (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		nếu (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				nếu (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->value);
		}
	}

	trả_về 0;
}

static số_nguyên
cmd_zap(void)
{
	einfo("Manually resetting %s to stopped state", applet);
	nếu (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	trả_về 0;
}

struct {
	const char *name;
	số_nguyên (*handler)(void);
	bool reset_restart_list;
} commands[] = {
	{ "describe",           simple_command,  false },
	{ "help",               simple_command,  false },
	{ "depend",             simple_command,  false },
	{ "status",             simple_command,  false },
	{ "ineed",              resolve_deptype, false },
	{ "iuse",               resolve_deptype, false },
	{ "iwant",              resolve_deptype, false },
	{ "needsme",            resolve_deptype, false },
	{ "usesme",             resolve_deptype, false },
	{ "wantsme",            resolve_deptype, false },
	{ "iafter",             resolve_deptype, false },
	{ "ibefore",            resolve_deptype, false },
	{ "iprovide",           resolve_deptype, false },
	{ "start",              svc_start,       true  },
	{ "stop",               cmd_stop,        true  },
	{ "restart",            svc_restart,     true  },
	{ "condrestart",        cmd_condrestart, true  },
	{ "conditionalrestart", cmd_condrestart, true  },
	{ "zap",                cmd_zap,         true  },
};

số_nguyên main(số_nguyên argc, char **argv)
{
	số_nguyên retval, opt;
	const char *workingdir = "/";
	char *pidstr = NULL;
	size_t l = 0, ll;
	struct stat stbuf;

	/* Show help if insufficient args */
	nếu (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	nếu (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	nếu (!(service = (realpath(argv[1], NULL))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	nếu (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	trong_khi ((opt = getopt_long(argc, argv, getoptstring, longopts, NULL)) != -1)
		switch (opt) {
		case 'd':
			setenv("RC_DEBUG", "YES", 1);
			break;
		case 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			break;
		case 's':
			nếu (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			break;
		case 'S':
			nếu (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			break;
		case 'D':
			deps = false;
			break;
		case 'Z':
			dry_run = true;
			break;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	nếu (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	nếu (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	nếu ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == NULL) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (số_nguyên) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	free(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	nếu (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(NULL);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->value);
			nếu (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = NULL;
		ll = strlen(applet);
		nếu (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(sizeof(char) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	nếu (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	nếu (deps && getenv("RC_PID") == NULL && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	nếu (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	nếu (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	nếu (rc_yesno(getenv("IN_HOTPLUG"))) {
		nếu (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		in_background = true;
	}

	/* Setup a signal handler */
	signal_setup(SIGHUP, handle_signal);
	signal_setup(SIGALRM, handle_signal);
	signal_setup(SIGUSR1, handle_signal);
	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);
	signal_setup(SIGCHLD, handle_signal);

	/* Load our plugins */
	rc_plugin_load();

	applet_list = rc_stringlist_new();
	rc_stringlist_add(applet_list, applet);

	nếu (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	trong_khi (optind < argc) {
		số_nguyên cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		nếu (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		cho (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			nếu (strcmp(optarg, commands[i].name) != 0)
				continue;

			nếu ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			nếu (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = NULL;
			}

			goto next_cmd;
		}

		nếu ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = NULL;
	}

	trả_về retval;
}
