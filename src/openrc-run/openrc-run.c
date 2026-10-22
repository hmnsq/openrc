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

#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <fnmatch.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <libgen.h>
#ଅନ୍ତର୍ଭାଵ <poll.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <spawn.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <sys/file.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <termios.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>

#ଯଦି defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ନଚେତଯଦି defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ଅନ୍ୟଥା
#  include <libutil.h>
#endif

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"
#ଅନ୍ତର୍ଭାଵ "plugin.h"
#ଅନ୍ତର୍ଭାଵ "selinux.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

ବାହ୍ୟ ଅକ୍ଷର **environ;

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = "stop | start | restart | status | describe | zap";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "debug",      0, ଖାଲି, 'd'},
	{ "dry-run",    0, ଖାଲି, 'Z'},
	{ "ifstarted",  0, ଖାଲି, 's'},
	{ "ifstopped",  0, ଖାଲି, 'S'},
	{ "nodeps",     0, ଖାଲି, 'D'},
	{ "lockfd",     1, ଖାଲି, 'l'},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର *ଅଚଳ longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ସ୍ଥିର ଅକ୍ଷର *service, *runlevel, *ibsave, *prefix;
ସ୍ଥିର RC_DEPTREE *deptree;
ସ୍ଥିର RC_STRINGLIST *applet_list, *services, *tmplist;
ସ୍ଥିର RC_STRINGLIST *restart_services;
ସ୍ଥିର RC_STRINGLIST *need_services;
ସ୍ଥିର RC_STRINGLIST *use_services;
ସ୍ଥିର RC_STRINGLIST *want_services;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା exclusive_fd = -1, master_tty = -1;
ସ୍ଥିର bool in_background, deps, dry_run;
ସ୍ଥିର ଅସ୍ଥିର bool sighup, skip_mark, timedout;
ସ୍ଥିର pid_t service_pid;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା signal_pipe[2] = { -1, -1 };

ସ୍ଥିର RC_STRINGLIST *deptypes_b;	/* broken deps */
ସ୍ଥିର RC_STRINGLIST *deptypes_n;	/* needed deps */
ସ୍ଥିର RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ସ୍ଥିର RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ସ୍ଥିର RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ସ୍ଥିର RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ସ୍ଥିର RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ସ୍ଥିର ଶୂନ୍ୟ
handle_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno, status;
	pid_t pid;
	ଅଚଳ ଅକ୍ଷର *signame = ଖାଲି;
	ସଂରଚନା winsize ws;

	ବାଛ (sig) {
	ସ୍ଥିତି SIGHUP:
		sighup = true;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି SIGUSR1:
		skip_mark = true;
		ଭାଙ୍ଗ;

	ସ୍ଥିତି SIGCHLD:
		ଯେତେବେଳେ ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			ଯଦି (signal_pipe[1] > -1 && pid == service_pid) {
				ଯଦି (ଲିଖ(signal_pipe[1], &status, ଆକାର(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି SIGWINCH:
		ଯଦି (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି SIGINT:
		ଯଦି (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ସ୍ଥିତି SIGTERM:
		ଯଦି (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ସ୍ଥିତି SIGQUIT:
		ଯଦି (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		ଯଦି (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	ସ୍ଥିତି SIGALRM:
		timedout = true;
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ସ୍ଥିର ଶୂନ୍ୟ
unhotplug(ଶୂନ୍ୟ)
{
	ଯଦି (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ସ୍ଥିର ଶୂନ୍ୟ
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	ଯଦି (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		ଫେରନ୍ତୁ;

	TAILQ_FOREACH(svc, list, entries) {
		ଯଦି (!(rc_service_state(svc->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED))
			ଜାରି;
		ଯଦି (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->ମୂଲ୍ୟ);
			ewarn("WARNING: %s will start when %s has started", svc->ମୂଲ୍ୟ, applet);
		} ଅନ୍ୟଥା {
			service_start(svc->ମୂଲ୍ୟ);
		}
	}
}

ସ୍ଥିର ଶୂନ୍ୟ
restore_state(ଶୂନ୍ୟ)
{
	RC_SERVICE state;

	ଯଦି (rc_in_plugin || exclusive_fd == -1)
		ଫେରନ୍ତୁ;
	state = rc_service_state(applet);
	ଯଦି (state & RC_SERVICE_STOPPING) {
		ଯଦି (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ଅନ୍ୟଥା
			rc_service_mark(applet, RC_SERVICE_STARTED);

		ଯଦି (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_STARTING) {
		ଯଦି (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ଅନ୍ୟଥା
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		ଯଦି (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ସ୍ଥିର ଶୂନ୍ୟ
cleanup(ଶୂନ୍ୟ)
{
	restore_state();

	ଯଦି (!rc_in_plugin) {
		ଯଦି (restart_services)
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
	ମୁକ୍ତ(ibsave);
	ମୁକ୍ତ(prefix);
	ମୁକ୍ତ(runlevel);
	ମୁକ୍ତ(service);
}

ସ୍ଥିର ଶୂନ୍ୟ
svc_getenv(ଅଚଳ ଅକ୍ଷର *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	ଅକ୍ଷର *var = ଖାଲି;

	TAILQ_FOREACH(reexport, reexports, entries) {
		ଯଦି (rc_service_getenv(svc, reexport->ମୂଲ୍ୟ, &var) == -1)
			ଜାରି;
		setenv(reexport->ମୂଲ୍ୟ, var, true);
	}

	ମୁକ୍ତ(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
write_prefix(ଅଚଳ ଅକ୍ଷର *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	ଅଚଳ ଅକ୍ଷର *ec = ecolor(ECOLOR_HILITE);
	ଅଚଳ ଅକ୍ଷର *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	ଯଦି (lock_fd != -1) {
		ଯେତେବେଳେ (flock(lock_fd, LOCK_EX) != 0) {
			ଯଦି (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				ଭାଙ୍ଗ;
			}
		}
	}
	ଅନ୍ୟଥା
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	ପାଇଁ (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		ଯଦି (buffer[i] == '\033' && !*prefixed) {
			ପାଇଁ (j = i + 1; j < bytes; j++) {
				ଯଦି (buffer[j] == 'A')
					*prefixed = true;
				ଯଦି (isalpha((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)buffer[j]))
					ଭାଙ୍ଗ;
			}
		}

		ଯଦି (!*prefixed) {
			ret += ଲିଖ(fd, ec, strlen(ec));
			ret += ଲିଖ(fd, prefix, strlen(prefix));
			ret += ଲିଖ(fd, ec_normal, strlen(ec_normal));
			ret += ଲିଖ(fd, "|", 1);
			*prefixed = true;
		}

		ଯଦି (buffer[i] == '\n')
			*prefixed = false;
		ret += ଲିଖ(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_exec(ଅଚଳ ଅକ୍ଷର *command)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ସଂରଚନା termios tt;
	ସଂରଚନା winsize ws;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;
	ସଂରଚନା pollfd fd[2];
	bool prefixed = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	ଅକ୍ଷର *openrc_sh = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ଖାଲି
	};

	/* Setup our signal pipe */
	ଯଦି (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		ଫେରନ୍ତୁ 1;
	}

	ଯଦି ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		ଫେରନ୍ତୁ 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	ଯଦି (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		ଯଦି (openpty(&master_tty, &slave_tty, ଖାଲି, &tt, &ws) == 0) {
			ଯଦି (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ଯଦି (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ଯଦି ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				ଫେରନ୍ତୁ 1;
			}
		}
	}

	ଯଦି (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	ଯଦି ((errno = posix_spawn(&service_pid, argv[0], &tty, ଖାଲି, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		ଫେରନ୍ତୁ 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	ମୁକ୍ତ(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	ପାଇଁ (;;) {
		ଯଦି (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			ଯଦି (errno == EINTR)
				ଜାରି;
			eerror("%s: poll: %s", applet, strerror(errno));
			ଭାଙ୍ଗ;
		}

		ଯଦି (fd[1].revents & (POLLIN | POLLHUP)) {
			ଅକ୍ଷର buffer[BUFSIZ];
			write_prefix(buffer, ପଠ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		ଯଦି (fd[0].revents & (POLLIN | POLLHUP)) {
			ଯଦି (errno = 0, ପଠ(signal_pipe[0], &ret, ଆକାର(ret)) < (ssize_t)ଆକାର(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			ଅନ୍ୟଥା ଯଦି ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			ଭାଙ୍ଗ;
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

	sigprocmask (SIG_SETMASK, &oldmask, ଖାଲି);

	ଯଦି (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର bool
svc_wait(ଅଚଳ ଅକ୍ଷର *svc)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ଅଚଳ ଅକ୍ଷର *base = basename_c(svc);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	ଯଦି (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	ଯଦି ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		ଫେରନ୍ତୁ errno == ENOENT;

	timedout = false;
	ପାଇଁ (;;) {
		ଯଦି (!forever)
			alarm(WARN_TIMEOUT);

		ଯଦି (flock(fd, LOCK_SH) == 0) {
			retval = true;
			ଭାଙ୍ଗ;
		}

		ଯଦି (errno != EINTR)
			ଭାଙ୍ଗ;

		ଯଦି (!forever && timedout) {
			timedout = false;
			ଯଦି ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			ଅନ୍ୟଥା
				ଭାଙ୍ଗ;
		}
	}

	alarm(0);
	close(fd);
	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ଶୂନ୍ୟ
get_started_services(ଶୂନ୍ୟ)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	ମୁକ୍ତ(tmp);
}

ସ୍ଥିର ଶୂନ୍ୟ
setup_deptypes(ଶୂନ୍ୟ)
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

ସ୍ଥିର ଶୂନ୍ୟ
svc_start_check(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	ଯଦି (in_background) {
		ଯଦି (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			ଯାଅ exit;

		ଯଦି (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		ଯଦି (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	ଯଦି (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		ଯାଅ exit;
	} ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	ଯଦି (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	ଯଦି (exclusive_fd == -1) {
		ଯଦି (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			ଯାଅ exit;
		}

		ଯଦି (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		ଅନ୍ୟଥା
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	ଫେରନ୍ତୁ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

ସ୍ଥିର ଶୂନ୍ୟ
svc_start_deps(ଶୂନ୍ୟ)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା depoptions = RC_DEP_TRACE;
	size_t len, svc_ଗଣନା;
	ଅକ୍ଷର *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	ଯଦି (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ଯଦି (!deptree && ((deptree = _rc_deptree_load(0, ଖାଲି)) == ଖାଲି)) {
		eerror("failed to load deptree");
		ଯାଅ exit;
	}

	ଯଦି (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	ଯଦି (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			ଯଦି (first)
				first = false;
			ଅନ୍ୟଥା
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->ମୂଲ୍ୟ);
		}
		fprintf(stderr, "\n");
		ଯାଅ exit;
	}
	rc_stringlist_free(services);
	services = ଖାଲି;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	ଯଦି (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->ମୂଲ୍ୟ);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			ଯଦି (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				ଜାରି;
			ଯଦି (state & RC_SERVICE_STOPPED) {
				ଯଦି (dry_run) {
					printf(" %s", svc->ମୂଲ୍ୟ);
					ଜାରି;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->ମୂଲ୍ୟ);
				ଯଦି (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	ଯଦି (dry_run)
		ଫେରନ୍ତୁ;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->ମୂଲ୍ୟ);
		ଯଦି (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ମୂଲ୍ୟ);
			ଜାରି;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		ଯଦି (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			ଯଦି (!rc_stringlist_find(need_services, svc->ମୂଲ୍ୟ) &&
					!rc_stringlist_find(want_services, svc->ମୂଲ୍ୟ) &&
					!rc_stringlist_find(use_services, svc->ମୂଲ୍ୟ))
				ଜାରି;
		}

		ଯଦି (!svc_wait(svc->ମୂଲ୍ୟ))
			eerror("%s: timed out waiting for %s", applet, svc->ମୂଲ୍ୟ);
		state = rc_service_state(svc->ମୂଲ୍ୟ);
		ଯଦି (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ମୂଲ୍ୟ);
			ଜାରି;
		}
		ଯଦି (rc_stringlist_find(need_services, svc->ମୂଲ୍ୟ)) {
			ଯଦି (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->ମୂଲ୍ୟ);
			} ଅନ୍ୟଥା ଯଦି (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->ମୂଲ୍ୟ);
				ଯାଅ exit;
			}
		}
	}

	ଯଦି (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ଖାଲି;
		mem = xopen_memstream(&tmp, &len);
		svc_ଗଣନା = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->ମୂଲ୍ୟ, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->ମୂଲ୍ୟ);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->ମୂଲ୍ୟ, applet);
			rc_stringlist_free(use_services);
			use_services = ଖାଲି;
			ଯଦି (svc_ଗଣନା > 0)
				fputs(", ", mem);
			fputs(svc->ମୂଲ୍ୟ, mem);
			++svc_ଗଣନା;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ଖାଲି;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_ଗଣନା > 1 ? "have" : "has");
		ମୁକ୍ତ(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ଖାଲି;
	rc_stringlist_free(services);
	services = ଖାଲି;

	ଫେରନ୍ତୁ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


ସ୍ଥିର ଶୂନ୍ୟ set_reexports(ଶୂନ୍ୟ)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	ଅକ୍ଷର *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->ମୂଲ୍ୟ);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ସ୍ଥିର ଶୂନ୍ୟ svc_start_real(ଶୂନ୍ୟ)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	ଯଦି (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	ଯଦି (ibsave)
		unsetenv("IN_BACKGROUND");

	ଯଦି (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	ଯଦି (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	ଅନ୍ୟଥା ଯଦି (!started)
		eerrorx("ERROR: %s failed to start", applet);

	ଯଦି (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    ଯଦି (rc_service_state(svc->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED)
		    service_start(svc->ମୂଲ୍ୟ);
	rc_stringlist_free(services);
	services = ଖାଲି;

	/* Do the same for any services we provide */
	ଯଦି (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->ମୂଲ୍ୟ);
			TAILQ_FOREACH(svc2, services, entries)
				ଯଦି (rc_service_state(svc2->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED)
					service_start(svc2->ମୂଲ୍ୟ);
			rc_stringlist_free(services);
			services = ଖାଲି;
		}
		rc_stringlist_free(tmplist);
		tmplist = ଖାଲି;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_start(ଶୂନ୍ୟ)
{
	ଯଦି (dry_run)
		einfon("start:");
	ଅନ୍ୟଥା
		svc_start_check();

	ଯଦି (deps)
		svc_start_deps();

	ଯଦି (dry_run)
		printf(" %s\n", applet);
	ଅନ୍ୟଥା
		svc_start_real();

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	ଯଦି (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		ଯାଅ exit;

	ଯଦି (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		ଯାଅ exit;

	ଯଦି (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		ଫେରନ୍ତୁ 1;
	}

	ଯଦି (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	ଯଦି (exclusive_fd == -1) {
		ଯଦି (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		ଅନ୍ୟଥା ଯଦି (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		ଅନ୍ୟଥା
			eerror("ERROR: %s stopped by something else", applet);
		ଯାଅ exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	ଯଦି (!rc_runlevel_stopping()) {
		ଯଦି (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		ଅନ୍ୟଥା ଯଦି (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	ଫେରନ୍ତୁ 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

ସ୍ଥିର ଶୂନ୍ୟ
svc_stop_deps(RC_SERVICE state)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	ଯଦି (state & RC_SERVICE_WASINACTIVE)
		ଫେରନ୍ତୁ;

	errno = 0;
	ଯଦି (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ଯଦି (!deptree && ((deptree = _rc_deptree_load(0, ଖାଲି)) == ଖାଲି)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	ଯଦି (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->ମୂଲ୍ୟ);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		ଯଦି (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			ଜାରି;
		ଯଦି (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			ଜାରି;
		ଯଦି (dry_run) {
			printf(" %s", svc->ମୂଲ୍ୟ);
			ଜାରି;
		}
		svc_wait(svc->ମୂଲ୍ୟ);
		ଯଦି (rc_service_state(svc->ମୂଲ୍ୟ) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->ମୂଲ୍ୟ);
			ଯଦି (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->ମୂଲ୍ୟ);
		}
	}
	rc_stringlist_free(services);
	services = ଖାଲି;
	ଯଦି (dry_run)
		ଫେରନ୍ତୁ;

	TAILQ_FOREACH(svc, tmplist, entries) {
		ଯଦି (rc_service_state(svc->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED) {
			ଜାରି;
		}
		svc_wait(svc->ମୂଲ୍ୟ);
		ଯଦି (rc_service_state(svc->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED)
			ଜାରି;
		ଯଦି (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			ଯଦି (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				ଜାରି;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->ମୂଲ୍ୟ);
	}
	rc_stringlist_free(tmplist);
	tmplist = ଖାଲି;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		ଯଦି (rc_service_state(svc->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED)
			ଜାରି;
		svc_wait(svc->ମୂଲ୍ୟ);
	}
	rc_stringlist_free(services);
	services = ଖାଲି;
}

ସ୍ଥିର ଶୂନ୍ୟ
load_dep_env(ଶୂନ୍ୟ)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->ମୂଲ୍ୟ);

	rc_stringlist_free(depsvcs);
}

ସ୍ଥିର ଶୂନ୍ୟ
svc_stop_real(ଶୂନ୍ୟ)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	ଯଦି (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	ଯଦି (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	ଯଦି (ibsave)
		unsetenv("IN_BACKGROUND");

	ଯଦି (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	ଯଦି (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_stop(ଶୂନ୍ୟ)
{
	RC_SERVICE state = 0;

	ଯଦି (dry_run)
		einfon("stop:");
	ଅନ୍ୟଥା
		ଯଦି (svc_stop_check(&state) == 1)
			ଫେରନ୍ତୁ 1; /* Service has been stopped already */

	ଯଦି (deps)
		svc_stop_deps(state);

	ଯଦି (dry_run)
		printf(" %s\n", applet);
	ଅନ୍ୟଥା
		svc_stop_real();

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
svc_restart(ଶୂନ୍ୟ)
{
	ଯଦି (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		ଯଦି (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ଖାଲି;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର bool
service_plugable(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *list, *p, *token;
	bool allow = true, truefalse;
	ଅକ୍ଷର *match = rc_conf_ମୂଲ୍ୟ("rc_hotplug");

	ଯଦି (!match)
		match = rc_conf_ମୂଲ୍ୟ("rc_plug_services");
	ଯଦି (!match)
		ଫେରନ୍ତୁ false;

	list = xstrdup(match);
	p = list;
	ଯେତେବେଳେ ((token = strsep(&p, " "))) {
		ଯଦି (token[0] == '!') {
			truefalse = false;
			token++;
		} ଅନ୍ୟଥା {
			truefalse = true;
		}

		ଯଦି (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			ଭାଙ୍ଗ;
		}
	}
	ମୁକ୍ତ(list);
	ଫେରନ୍ତୁ allow;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
simple_command(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *save = prefix;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;

	eprefix(ଖାଲି);
	prefix = ଖାଲି;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
resolve_deptype(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	ଯଦି (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ଯଦି (!deptree && ((deptree = _rc_deptree_load(0, ଖାଲି)) == ଖାଲି))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ଖାଲି;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->ମୂଲ୍ୟ);
	printf ("\n");

	rc_stringlist_free(services);
	services = ଖାଲି;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cmd_condrestart(ଶୂନ୍ୟ)
{
	ଯଦି (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cmd_stop(ଶୂନ୍ୟ)
{
	RC_STRING *svc;

	ଯଦି (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	ଯଦି (deps && in_background)
		get_started_services();

	ଯଦି (svc_stop() == 1)
		ଫେରନ୍ତୁ 0; /* Service has been stopped already */

	ଯଦି (deps) {
		ଯଦି (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		ଯଦି (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				ଯଦି (rc_service_state(svc->ମୂଲ୍ୟ) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->ମୂଲ୍ୟ);
		}
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
cmd_zap(ଶୂନ୍ୟ)
{
	einfo("Manually resetting %s to stopped state", applet);
	ଯଦି (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	ଫେରନ୍ତୁ 0;
}

ସଂରଚନା {
	ଅଚଳ ଅକ୍ଷର *name;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*handler)(ଶୂନ୍ୟ);
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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval, opt;
	ଅଚଳ ଅକ୍ଷର *workingdir = "/";
	ଅକ୍ଷର *pidstr = ଖାଲି;
	size_t l = 0, ll;
	ସଂରଚନା stat stbuf;

	/* Show help if insufficient args */
	ଯଦି (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	ଯଦି (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	ଯଦି (!(service = (realpath(argv[1], ଖାଲି))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	ଯଦି (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring, longopts, ଖାଲି)) != -1)
		ବାଛ (opt) {
		ସ୍ଥିତି 'd':
			setenv("RC_DEBUG", "YES", 1);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			ଯଦି (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'S':
			ଯଦି (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'D':
			deps = false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'Z':
			dry_run = true;
			ଭାଙ୍ଗ;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ଯଦି (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ଯଦି (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	ଯଦି ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ଖାଲି) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	ମୁକ୍ତ(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	ଯଦି (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(ଖାଲି);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->ମୂଲ୍ୟ);
			ଯଦି (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ଖାଲି;
		ll = strlen(applet);
		ଯଦି (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(ଆକାର(ଅକ୍ଷର) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	ଯଦି (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	ଯଦି (deps && getenv("RC_PID") == ଖାଲି && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	ଯଦି (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	ଯଦି (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	ଯଦି (rc_yesno(getenv("IN_HOTPLUG"))) {
		ଯଦି (!service_plugable())
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

	ଯଦି (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	ଯେତେବେଳେ (optind < argc) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		ଯଦି (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			ଯଦି (strcmp(optarg, commands[i].name) != 0)
				ଜାରି;

			ଯଦି ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			ଯଦି (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ଖାଲି;
			}

			ଯାଅ next_cmd;
		}

		ଯଦି ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ଖାଲି;
	}

	ଫେରନ୍ତୁ retval;
}
