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

#ຖ້າ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ບໍ່ດັ່ງນັ້ນ
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

extern var_na **environ;

const var_na *applet = NULL;
const var_na *extraopts = "stop | start | restart | status | describe | zap";
const var_na getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "debug",      0, NULL, 'd'},
	{ "dry-run",    0, NULL, 'Z'},
	{ "ifstarted",  0, NULL, 's'},
	{ "ifstopped",  0, NULL, 'S'},
	{ "nodeps",     0, NULL, 'D'},
	{ "lockfd",     1, NULL, 'l'},
	longopts_COMMON
};
const var_na *const longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
const var_na *usagestring = NULL;

static var_na *service, *runlevel, *ibsave, *prefix;
static RC_DEPTREE *deptree;
static RC_STRINGLIST *applet_list, *services, *tmplist;
static RC_STRINGLIST *restart_services;
static RC_STRINGLIST *need_services;
static RC_STRINGLIST *use_services;
static RC_STRINGLIST *want_services;
static ຈຳນວນເຕັມ exclusive_fd = -1, master_tty = -1;
static bool in_background, deps, dry_run;
static ca_ncala bool sighup, skip_mark, timedout;
static pid_t service_pid;
static ຈຳນວນເຕັມ signal_pipe[2] = { -1, -1 };

static RC_STRINGLIST *deptypes_b;	/* broken deps */
static RC_STRINGLIST *deptypes_n;	/* needed deps */
static RC_STRINGLIST *deptypes_nw;	/* need+want deps */
static RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
static RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
static RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
static RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

static void
handle_signal(ຈຳນວນເຕັມ sig)
{
	ຈຳນວນເຕັມ serrno = errno, status;
	pid_t pid;
	const var_na *signame = NULL;
	sa_nracanaa winsize ws;

	switch (sig) {
	pak_sa SIGHUP:
		sighup = true;
		bha_nga;

	pak_sa SIGUSR1:
		skip_mark = true;
		bha_nga;

	pak_sa SIGCHLD:
		ໃນຂະນະທີ່ ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			ຖ້າ (signal_pipe[1] > -1 && pid == service_pid) {
				ຖ້າ (write(signal_pipe[1], &status, parimaa_na(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		bha_nga;

	pak_sa SIGWINCH:
		ຖ້າ (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		bha_nga;

	pak_sa SIGINT:
		ຖ້າ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	pak_sa SIGTERM:
		ຖ້າ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	pak_sa SIGQUIT:
		ຖ້າ (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		ຖ້າ (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	pak_sa SIGALRM:
		timedout = true;
		bha_nga;

	default:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

static void
unhotplug(void)
{
	ຖ້າ (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

static void
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	ຖ້າ (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		ສົ່ງຄືນ;

	TAILQ_FOREACH(svc, list, entries) {
		ຖ້າ (!(rc_service_state(svc->value) & RC_SERVICE_STOPPED))
			continue;
		ຖ້າ (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->value);
			ewarn("WARNING: %s will start when %s has started", svc->value, applet);
		} ບໍ່ດັ່ງນັ້ນ {
			service_start(svc->value);
		}
	}
}

static void
restore_state(void)
{
	RC_SERVICE state;

	ຖ້າ (rc_in_plugin || exclusive_fd == -1)
		ສົ່ງຄືນ;
	state = rc_service_state(applet);
	ຖ້າ (state & RC_SERVICE_STOPPING) {
		ຖ້າ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ບໍ່ດັ່ງນັ້ນ
			rc_service_mark(applet, RC_SERVICE_STARTED);

		ຖ້າ (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (state & RC_SERVICE_STARTING) {
		ຖ້າ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ບໍ່ດັ່ງນັ້ນ
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		ຖ້າ (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

static void
cleanup(void)
{
	restore_state();

	ຖ້າ (!rc_in_plugin) {
		ຖ້າ (restart_services)
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
svc_getenv(const var_na *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	var_na *var = NULL;

	TAILQ_FOREACH(reexport, reexports, entries) {
		ຖ້າ (rc_service_getenv(svc, reexport->value, &var) == -1)
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
static ຈຳນວນເຕັມ
write_prefix(const var_na *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	const var_na *ec = ecolor(ECOLOR_HILITE);
	const var_na *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	ຈຳນວນເຕັມ fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	ຖ້າ (lock_fd != -1) {
		ໃນຂະນະທີ່ (flock(lock_fd, LOCK_EX) != 0) {
			ຖ້າ (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				bha_nga;
			}
		}
	}
	ບໍ່ດັ່ງນັ້ນ
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	ສຳລັບ (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		ຖ້າ (buffer[i] == '\033' && !*prefixed) {
			ສຳລັບ (j = i + 1; j < bytes; j++) {
				ຖ້າ (buffer[j] == 'A')
					*prefixed = true;
				ຖ້າ (isalpha((unsigned ຈຳນວນເຕັມ)buffer[j]))
					bha_nga;
			}
		}

		ຖ້າ (!*prefixed) {
			ret += write(fd, ec, strlen(ec));
			ret += write(fd, prefix, strlen(prefix));
			ret += write(fd, ec_normal, strlen(ec_normal));
			ret += write(fd, "|", 1);
			*prefixed = true;
		}

		ຖ້າ (buffer[i] == '\n')
			*prefixed = false;
		ret += write(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	ສົ່ງຄືນ ret;
}

static ຈຳນວນເຕັມ
svc_exec(const var_na *command)
{
	ຈຳນວນເຕັມ ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	sa_nracanaa termios tt;
	sa_nracanaa winsize ws;
	ຈຳນວນເຕັມ flags = 0;
	sa_nracanaa pollfd fd[2];
	bool prefixed = false;
	ຈຳນວນເຕັມ slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	var_na *openrc_sh = NULL;
	const var_na *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		NULL
	};

	/* Setup our signal pipe */
	ຖ້າ (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		ສົ່ງຄືນ 1;
	}

	ຖ້າ ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		ສົ່ງຄືນ 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	ຖ້າ (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		ຖ້າ (openpty(&master_tty, &slave_tty, NULL, &tt, &ws) == 0) {
			ຖ້າ (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ຖ້າ (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ຖ້າ ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				ສົ່ງຄືນ 1;
			}
		}
	}

	ຖ້າ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	ຖ້າ ((errno = posix_spawn(&service_pid, argv[0], &tty, NULL, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		ສົ່ງຄືນ 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	free(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	ສຳລັບ (;;) {
		ຖ້າ (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			ຖ້າ (errno == EINTR)
				continue;
			eerror("%s: poll: %s", applet, strerror(errno));
			bha_nga;
		}

		ຖ້າ (fd[1].revents & (POLLIN | POLLHUP)) {
			var_na buffer[BUFSIZ];
			write_prefix(buffer, read(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		ຖ້າ (fd[0].revents & (POLLIN | POLLHUP)) {
			ຖ້າ (errno = 0, read(signal_pipe[0], &ret, parimaa_na(ret)) < (ssize_t)parimaa_na(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			ບໍ່ດັ່ງນັ້ນ ຖ້າ ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			bha_nga;
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

	ຖ້າ (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	ສົ່ງຄືນ ret;
}

static bool
svc_wait(const var_na *svc)
{
	ຈຳນວນເຕັມ fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	const var_na *base = basename_c(svc);
	ຈຳນວນເຕັມ timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	ຖ້າ (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	ຖ້າ ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		ສົ່ງຄືນ errno == ENOENT;

	timedout = false;
	ສຳລັບ (;;) {
		ຖ້າ (!forever)
			alarm(WARN_TIMEOUT);

		ຖ້າ (flock(fd, LOCK_SH) == 0) {
			retval = true;
			bha_nga;
		}

		ຖ້າ (errno != EINTR)
			bha_nga;

		ຖ້າ (!forever && timedout) {
			timedout = false;
			ຖ້າ ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			ບໍ່ດັ່ງນັ້ນ
				bha_nga;
		}
	}

	alarm(0);
	close(fd);
	ສົ່ງຄືນ retval;
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
	ຈຳນວນເຕັມ ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	ຖ້າ (in_background) {
		ຖ້າ (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			goto exit;

		ຖ້າ (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		ຖ້າ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	ຖ້າ (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		goto exit;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	ຖ້າ (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	ຖ້າ (exclusive_fd == -1) {
		ຖ້າ (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			goto exit;
		}

		ຖ້າ (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		ບໍ່ດັ່ງນັ້ນ
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	ສົ່ງຄືນ;

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
	ຈຳນວນເຕັມ depoptions = RC_DEP_TRACE;
	size_t len, svc_count;
	var_na *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	ຖ້າ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ຖ້າ (!deptree && ((deptree = _rc_deptree_load(0, NULL)) == NULL)) {
		eerror("failed to load deptree");
		goto exit;
	}

	ຖ້າ (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	ຖ້າ (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			ຖ້າ (first)
				first = false;
			ບໍ່ດັ່ງນັ້ນ
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

	ຖ້າ (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->value);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			ຖ້າ (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				continue;
			ຖ້າ (state & RC_SERVICE_STOPPED) {
				ຖ້າ (dry_run) {
					printf(" %s", svc->value);
					continue;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->value);
				ຖ້າ (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	ຖ້າ (dry_run)
		ສົ່ງຄືນ;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->value);
		ຖ້າ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->value);
			continue;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		ຖ້າ (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			ຖ້າ (!rc_stringlist_find(need_services, svc->value) &&
					!rc_stringlist_find(want_services, svc->value) &&
					!rc_stringlist_find(use_services, svc->value))
				continue;
		}

		ຖ້າ (!svc_wait(svc->value))
			eerror("%s: timed out waiting for %s", applet, svc->value);
		state = rc_service_state(svc->value);
		ຖ້າ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->value);
			continue;
		}
		ຖ້າ (rc_stringlist_find(need_services, svc->value)) {
			ຖ້າ (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->value);
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->value);
				goto exit;
			}
		}
	}

	ຖ້າ (TAILQ_FIRST(tmplist)) {
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
			ຖ້າ (svc_count > 0)
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

	ສົ່ງຄືນ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


static void set_reexports(void)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	var_na *buf;
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

	ຖ້າ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	ຖ້າ (ibsave)
		unsetenv("IN_BACKGROUND");

	ຖ້າ (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	ຖ້າ (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (!started)
		eerrorx("ERROR: %s failed to start", applet);

	ຖ້າ (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    ຖ້າ (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
		    service_start(svc->value);
	rc_stringlist_free(services);
	services = NULL;

	/* Do the same for any services we provide */
	ຖ້າ (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->value);
			TAILQ_FOREACH(svc2, services, entries)
				ຖ້າ (rc_service_state(svc2->value) & RC_SERVICE_STOPPED)
					service_start(svc2->value);
			rc_stringlist_free(services);
			services = NULL;
		}
		rc_stringlist_free(tmplist);
		tmplist = NULL;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

static ຈຳນວນເຕັມ
svc_start(void)
{
	ຖ້າ (dry_run)
		einfon("start:");
	ບໍ່ດັ່ງນັ້ນ
		svc_start_check();

	ຖ້າ (deps)
		svc_start_deps();

	ຖ້າ (dry_run)
		printf(" %s\n", applet);
	ບໍ່ດັ່ງນັ້ນ
		svc_start_real();

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	ຖ້າ (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		goto exit;

	ຖ້າ (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		goto exit;

	ຖ້າ (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		ສົ່ງຄືນ 1;
	}

	ຖ້າ (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	ຖ້າ (exclusive_fd == -1) {
		ຖ້າ (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		ບໍ່ດັ່ງນັ້ນ
			eerror("ERROR: %s stopped by something else", applet);
		goto exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	ຖ້າ (!rc_runlevel_stopping()) {
		ຖ້າ (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	ສົ່ງຄືນ 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

static void
svc_stop_deps(RC_SERVICE state)
{
	ຈຳນວນເຕັມ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	ຖ້າ (state & RC_SERVICE_WASINACTIVE)
		ສົ່ງຄືນ;

	errno = 0;
	ຖ້າ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ຖ້າ (!deptree && ((deptree = _rc_deptree_load(0, NULL)) == NULL)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	ຖ້າ (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->value);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		ຖ້າ (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			continue;
		ຖ້າ (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			continue;
		ຖ້າ (dry_run) {
			printf(" %s", svc->value);
			continue;
		}
		svc_wait(svc->value);
		ຖ້າ (rc_service_state(svc->value) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->value);
			ຖ້າ (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->value);
		}
	}
	rc_stringlist_free(services);
	services = NULL;
	ຖ້າ (dry_run)
		ສົ່ງຄືນ;

	TAILQ_FOREACH(svc, tmplist, entries) {
		ຖ້າ (rc_service_state(svc->value) & RC_SERVICE_STOPPED) {
			continue;
		}
		svc_wait(svc->value);
		ຖ້າ (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
			continue;
		ຖ້າ (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			ຖ້າ (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
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
		ຖ້າ (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
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
	ຖ້າ (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	ຖ້າ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	ຖ້າ (ibsave)
		unsetenv("IN_BACKGROUND");

	ຖ້າ (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	ຖ້າ (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

static ຈຳນວນເຕັມ
svc_stop(void)
{
	RC_SERVICE state = 0;

	ຖ້າ (dry_run)
		einfon("stop:");
	ບໍ່ດັ່ງນັ້ນ
		ຖ້າ (svc_stop_check(&state) == 1)
			ສົ່ງຄືນ 1; /* Service has been stopped already */

	ຖ້າ (deps)
		svc_stop_deps(state);

	ຖ້າ (dry_run)
		printf(" %s\n", applet);
	ບໍ່ດັ່ງນັ້ນ
		svc_stop_real();

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ
svc_restart(void)
{
	ຖ້າ (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		ຖ້າ (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = NULL;

	ສົ່ງຄືນ 0;
}

static bool
service_plugable(void)
{
	var_na *list, *p, *token;
	bool allow = true, truefalse;
	var_na *match = rc_conf_value("rc_hotplug");

	ຖ້າ (!match)
		match = rc_conf_value("rc_plug_services");
	ຖ້າ (!match)
		ສົ່ງຄືນ false;

	list = xstrdup(match);
	p = list;
	ໃນຂະນະທີ່ ((token = strsep(&p, " "))) {
		ຖ້າ (token[0] == '!') {
			truefalse = false;
			token++;
		} ບໍ່ດັ່ງນັ້ນ {
			truefalse = true;
		}

		ຖ້າ (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			bha_nga;
		}
	}
	free(list);
	ສົ່ງຄືນ allow;
}

static ຈຳນວນເຕັມ
simple_command(void)
{
	var_na *save = prefix;
	ຈຳນວນເຕັມ retval;

	eprefix(NULL);
	prefix = NULL;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	ສົ່ງຄືນ retval;
}

static ຈຳນວນເຕັມ
resolve_deptype(void)
{
	ຈຳນວນເຕັມ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	ຖ້າ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ຖ້າ (!deptree && ((deptree = _rc_deptree_load(0, NULL)) == NULL))
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

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ
cmd_condrestart(void)
{
	ຖ້າ (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ
cmd_stop(void)
{
	RC_STRING *svc;

	ຖ້າ (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	ຖ້າ (deps && in_background)
		get_started_services();

	ຖ້າ (svc_stop() == 1)
		ສົ່ງຄືນ 0; /* Service has been stopped already */

	ຖ້າ (deps) {
		ຖ້າ (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		ຖ້າ (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				ຖ້າ (rc_service_state(svc->value) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->value);
		}
	}

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ
cmd_zap(void)
{
	einfo("Manually resetting %s to stopped state", applet);
	ຖ້າ (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	ສົ່ງຄືນ 0;
}

sa_nracanaa {
	const var_na *name;
	ຈຳນວນເຕັມ (*handler)(void);
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

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv)
{
	ຈຳນວນເຕັມ retval, opt;
	const var_na *workingdir = "/";
	var_na *pidstr = NULL;
	size_t l = 0, ll;
	sa_nracanaa stat stbuf;

	/* Show help if insufficient args */
	ຖ້າ (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	ຖ້າ (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	ຖ້າ (!(service = (realpath(argv[1], NULL))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	ຖ້າ (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	ໃນຂະນະທີ່ ((opt = getopt_long(argc, argv, getoptstring, longopts, NULL)) != -1)
		switch (opt) {
		pak_sa 'd':
			setenv("RC_DEBUG", "YES", 1);
			bha_nga;
		pak_sa 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			bha_nga;
		pak_sa 's':
			ຖ້າ (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			bha_nga;
		pak_sa 'S':
			ຖ້າ (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			bha_nga;
		pak_sa 'D':
			deps = false;
			bha_nga;
		pak_sa 'Z':
			dry_run = true;
			bha_nga;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ຖ້າ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ຖ້າ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	ຖ້າ ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == NULL) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (ຈຳນວນເຕັມ) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	free(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	ຖ້າ (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(NULL);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->value);
			ຖ້າ (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = NULL;
		ll = strlen(applet);
		ຖ້າ (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(parimaa_na(var_na) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	ຖ້າ (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	ຖ້າ (deps && getenv("RC_PID") == NULL && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	ຖ້າ (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	ຖ້າ (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	ຖ້າ (rc_yesno(getenv("IN_HOTPLUG"))) {
		ຖ້າ (!service_plugable())
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

	ຖ້າ (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	ໃນຂະນະທີ່ (optind < argc) {
		ຈຳນວນເຕັມ cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		ຖ້າ (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		ສຳລັບ (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			ຖ້າ (strcmp(optarg, commands[i].name) != 0)
				continue;

			ຖ້າ ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			ຖ້າ (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = NULL;
			}

			goto next_cmd;
		}

		ຖ້າ ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = NULL;
	}

	ສົ່ງຄືນ retval;
}
