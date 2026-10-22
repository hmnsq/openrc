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

#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <fnmatch.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <libgen.h>
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <spawn.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <termios.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>

#ಯದಿ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ನೋಚೇದ್ಯದಿ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ಅನ್ಯಥಾ
#  include <libutil.h>
#endif

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "plugin.h"
#ಅನ್ತರ್ಭಾವ "selinux.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

ಬಾಹ್ಯ ವರ್ಣ **environ;

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = "stop | start | restart | status | describe | zap";
ಧ್ರುವ ವರ್ಣ getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "debug",      0, ರಿಕ್ತ, 'd'},
	{ "dry-run",    0, ರಿಕ್ತ, 'Z'},
	{ "ifstarted",  0, ರಿಕ್ತ, 's'},
	{ "ifstopped",  0, ರಿಕ್ತ, 'S'},
	{ "nodeps",     0, ರಿಕ್ತ, 'D'},
	{ "lockfd",     1, ರಿಕ್ತ, 'l'},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಸ್ಥಿರ ವರ್ಣ *service, *runlevel, *ibsave, *prefix;
ಸ್ಥಿರ RC_DEPTREE *deptree;
ಸ್ಥಿರ RC_STRINGLIST *applet_list, *services, *tmplist;
ಸ್ಥಿರ RC_STRINGLIST *restart_services;
ಸ್ಥಿರ RC_STRINGLIST *need_services;
ಸ್ಥಿರ RC_STRINGLIST *use_services;
ಸ್ಥಿರ RC_STRINGLIST *want_services;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ exclusive_fd = -1, master_tty = -1;
ಸ್ಥಿರ bool in_background, deps, dry_run;
ಸ್ಥಿರ ಚಞ್ಚಲ bool sighup, skip_mark, timedout;
ಸ್ಥಿರ pid_t service_pid;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_pipe[2] = { -1, -1 };

ಸ್ಥಿರ RC_STRINGLIST *deptypes_b;	/* broken deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_n;	/* needed deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ಸ್ಥಿರ ಶೂನ್ಯ
handle_signal(ಪೂರ್ಣಾಙ್ಕ sig)
{
	ಪೂರ್ಣಾಙ್ಕ serrno = errno, status;
	pid_t pid;
	ಧ್ರುವ ವರ್ಣ *signame = ರಿಕ್ತ;
	ಸಂರಚನಾ winsize ws;

	ವಿಕಲ್ಪ (sig) {
	ಪಕ್ಷ SIGHUP:
		sighup = true;
		ಭಙ್ಗ;

	ಪಕ್ಷ SIGUSR1:
		skip_mark = true;
		ಭಙ್ಗ;

	ಪಕ್ಷ SIGCHLD:
		ಯಾವತ್ ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			ಯದಿ (signal_pipe[1] > -1 && pid == service_pid) {
				ಯದಿ (ಲಿಖ(signal_pipe[1], &status, ಪರಿಮಾಣ(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ SIGWINCH:
		ಯದಿ (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ SIGINT:
		ಯದಿ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ಪಕ್ಷ SIGTERM:
		ಯದಿ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ಪಕ್ಷ SIGQUIT:
		ಯದಿ (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		ಯದಿ (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	ಪಕ್ಷ SIGALRM:
		timedout = true;
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ಶೂನ್ಯ
unhotplug(ಶೂನ್ಯ)
{
	ಯದಿ (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ಸ್ಥಿರ ಶೂನ್ಯ
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	ಯದಿ (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		ಪ್ರತ್ಯಾಗಮ;

	TAILQ_FOREACH(svc, list, entries) {
		ಯದಿ (!(rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED))
			ಅನುವರ್ತ;
		ಯದಿ (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->ಮೌಲ್ಯ);
			ewarn("WARNING: %s will start when %s has started", svc->ಮೌಲ್ಯ, applet);
		} ಅನ್ಯಥಾ {
			service_start(svc->ಮೌಲ್ಯ);
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
restore_state(ಶೂನ್ಯ)
{
	RC_SERVICE state;

	ಯದಿ (rc_in_plugin || exclusive_fd == -1)
		ಪ್ರತ್ಯಾಗಮ;
	state = rc_service_state(applet);
	ಯದಿ (state & RC_SERVICE_STOPPING) {
		ಯದಿ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ಅನ್ಯಥಾ
			rc_service_mark(applet, RC_SERVICE_STARTED);

		ಯದಿ (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_STARTING) {
		ಯದಿ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ಅನ್ಯಥಾ
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		ಯದಿ (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ಸ್ಥಿರ ಶೂನ್ಯ
cleanup(ಶೂನ್ಯ)
{
	restore_state();

	ಯದಿ (!rc_in_plugin) {
		ಯದಿ (restart_services)
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
	ಮುಕ್ತ(ibsave);
	ಮುಕ್ತ(prefix);
	ಮುಕ್ತ(runlevel);
	ಮುಕ್ತ(service);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_getenv(ಧ್ರುವ ವರ್ಣ *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	ವರ್ಣ *var = ರಿಕ್ತ;

	TAILQ_FOREACH(reexport, reexports, entries) {
		ಯದಿ (rc_service_getenv(svc, reexport->ಮೌಲ್ಯ, &var) == -1)
			ಅನುವರ್ತ;
		setenv(reexport->ಮೌಲ್ಯ, var, true);
	}

	ಮುಕ್ತ(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
write_prefix(ಧ್ರುವ ವರ್ಣ *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	ಧ್ರುವ ವರ್ಣ *ec = ecolor(ECOLOR_HILITE);
	ಧ್ರುವ ವರ್ಣ *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	ಪೂರ್ಣಾಙ್ಕ fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	ಯದಿ (lock_fd != -1) {
		ಯಾವತ್ (flock(lock_fd, LOCK_EX) != 0) {
			ಯದಿ (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				ಭಙ್ಗ;
			}
		}
	}
	ಅನ್ಯಥಾ
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	ಪ್ರತಿ (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		ಯದಿ (buffer[i] == '\033' && !*prefixed) {
			ಪ್ರತಿ (j = i + 1; j < bytes; j++) {
				ಯದಿ (buffer[j] == 'A')
					*prefixed = true;
				ಯದಿ (isalpha((unsigned ಪೂರ್ಣಾಙ್ಕ)buffer[j]))
					ಭಙ್ಗ;
			}
		}

		ಯದಿ (!*prefixed) {
			ret += ಲಿಖ(fd, ec, strlen(ec));
			ret += ಲಿಖ(fd, prefix, strlen(prefix));
			ret += ಲಿಖ(fd, ec_normal, strlen(ec_normal));
			ret += ಲಿಖ(fd, "|", 1);
			*prefixed = true;
		}

		ಯದಿ (buffer[i] == '\n')
			*prefixed = false;
		ret += ಲಿಖ(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
svc_exec(ಧ್ರುವ ವರ್ಣ *command)
{
	ಪೂರ್ಣಾಙ್ಕ ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ಸಂರಚನಾ termios tt;
	ಸಂರಚನಾ winsize ws;
	ಪೂರ್ಣಾಙ್ಕ flags = 0;
	ಸಂರಚನಾ pollfd fd[2];
	bool prefixed = false;
	ಪೂರ್ಣಾಙ್ಕ slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	ವರ್ಣ *openrc_sh = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ರಿಕ್ತ
	};

	/* Setup our signal pipe */
	ಯದಿ (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಯದಿ ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	ಯದಿ (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		ಯದಿ (openpty(&master_tty, &slave_tty, ರಿಕ್ತ, &tt, &ws) == 0) {
			ಯದಿ (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ಯದಿ (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ಯದಿ ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				ಪ್ರತ್ಯಾಗಮ 1;
			}
		}
	}

	ಯದಿ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	ಯದಿ ((errno = posix_spawn(&service_pid, argv[0], &tty, ರಿಕ್ತ, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	ಮುಕ್ತ(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	ಪ್ರತಿ (;;) {
		ಯದಿ (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			ಯದಿ (errno == EINTR)
				ಅನುವರ್ತ;
			eerror("%s: poll: %s", applet, strerror(errno));
			ಭಙ್ಗ;
		}

		ಯದಿ (fd[1].revents & (POLLIN | POLLHUP)) {
			ವರ್ಣ buffer[BUFSIZ];
			write_prefix(buffer, ಪಠ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		ಯದಿ (fd[0].revents & (POLLIN | POLLHUP)) {
			ಯದಿ (errno = 0, ಪಠ(signal_pipe[0], &ret, ಪರಿಮಾಣ(ret)) < (ssize_t)ಪರಿಮಾಣ(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			ಅನ್ಯಥಾ ಯದಿ ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			ಭಙ್ಗ;
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

	sigprocmask (SIG_SETMASK, &oldmask, ರಿಕ್ತ);

	ಯದಿ (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ bool
svc_wait(ಧ್ರುವ ವರ್ಣ *svc)
{
	ಪೂರ್ಣಾಙ್ಕ fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ಧ್ರುವ ವರ್ಣ *base = basename_c(svc);
	ಪೂರ್ಣಾಙ್ಕ timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	ಯದಿ (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	ಯದಿ ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		ಪ್ರತ್ಯಾಗಮ errno == ENOENT;

	timedout = false;
	ಪ್ರತಿ (;;) {
		ಯದಿ (!forever)
			alarm(WARN_TIMEOUT);

		ಯದಿ (flock(fd, LOCK_SH) == 0) {
			retval = true;
			ಭಙ್ಗ;
		}

		ಯದಿ (errno != EINTR)
			ಭಙ್ಗ;

		ಯದಿ (!forever && timedout) {
			timedout = false;
			ಯದಿ ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			ಅನ್ಯಥಾ
				ಭಙ್ಗ;
		}
	}

	alarm(0);
	close(fd);
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಶೂನ್ಯ
get_started_services(ಶೂನ್ಯ)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	ಮುಕ್ತ(tmp);
}

ಸ್ಥಿರ ಶೂನ್ಯ
setup_deptypes(ಶೂನ್ಯ)
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

ಸ್ಥಿರ ಶೂನ್ಯ
svc_start_check(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	ಯದಿ (in_background) {
		ಯದಿ (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			ಗಚ್ಛ exit;

		ಯದಿ (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		ಯದಿ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	ಯದಿ (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		ಗಚ್ಛ exit;
	} ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	ಯದಿ (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	ಯದಿ (exclusive_fd == -1) {
		ಯದಿ (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			ಗಚ್ಛ exit;
		}

		ಯದಿ (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		ಅನ್ಯಥಾ
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	ಪ್ರತ್ಯಾಗಮ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_start_deps(ಶೂನ್ಯ)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	ಪೂರ್ಣಾಙ್ಕ depoptions = RC_DEP_TRACE;
	size_t len, svc_ಎಣಿಕೆ;
	ವರ್ಣ *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	ಯದಿ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ಯದಿ (!deptree && ((deptree = _rc_deptree_load(0, ರಿಕ್ತ)) == ರಿಕ್ತ)) {
		eerror("failed to load deptree");
		ಗಚ್ಛ exit;
	}

	ಯದಿ (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	ಯದಿ (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			ಯದಿ (first)
				first = false;
			ಅನ್ಯಥಾ
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->ಮೌಲ್ಯ);
		}
		fprintf(stderr, "\n");
		ಗಚ್ಛ exit;
	}
	rc_stringlist_free(services);
	services = ರಿಕ್ತ;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	ಯದಿ (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->ಮೌಲ್ಯ);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			ಯದಿ (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				ಅನುವರ್ತ;
			ಯದಿ (state & RC_SERVICE_STOPPED) {
				ಯದಿ (dry_run) {
					printf(" %s", svc->ಮೌಲ್ಯ);
					ಅನುವರ್ತ;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->ಮೌಲ್ಯ);
				ಯದಿ (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	ಯದಿ (dry_run)
		ಪ್ರತ್ಯಾಗಮ;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->ಮೌಲ್ಯ);
		ಯದಿ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ಮೌಲ್ಯ);
			ಅನುವರ್ತ;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		ಯದಿ (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			ಯದಿ (!rc_stringlist_find(need_services, svc->ಮೌಲ್ಯ) &&
					!rc_stringlist_find(want_services, svc->ಮೌಲ್ಯ) &&
					!rc_stringlist_find(use_services, svc->ಮೌಲ್ಯ))
				ಅನುವರ್ತ;
		}

		ಯದಿ (!svc_wait(svc->ಮೌಲ್ಯ))
			eerror("%s: timed out waiting for %s", applet, svc->ಮೌಲ್ಯ);
		state = rc_service_state(svc->ಮೌಲ್ಯ);
		ಯದಿ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ಮೌಲ್ಯ);
			ಅನುವರ್ತ;
		}
		ಯದಿ (rc_stringlist_find(need_services, svc->ಮೌಲ್ಯ)) {
			ಯದಿ (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->ಮೌಲ್ಯ);
			} ಅನ್ಯಥಾ ಯದಿ (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->ಮೌಲ್ಯ);
				ಗಚ್ಛ exit;
			}
		}
	}

	ಯದಿ (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ರಿಕ್ತ;
		mem = xopen_memstream(&tmp, &len);
		svc_ಎಣಿಕೆ = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->ಮೌಲ್ಯ, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->ಮೌಲ್ಯ);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->ಮೌಲ್ಯ, applet);
			rc_stringlist_free(use_services);
			use_services = ರಿಕ್ತ;
			ಯದಿ (svc_ಎಣಿಕೆ > 0)
				fputs(", ", mem);
			fputs(svc->ಮೌಲ್ಯ, mem);
			++svc_ಎಣಿಕೆ;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ರಿಕ್ತ;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_ಎಣಿಕೆ > 1 ? "have" : "has");
		ಮುಕ್ತ(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ರಿಕ್ತ;
	rc_stringlist_free(services);
	services = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


ಸ್ಥಿರ ಶೂನ್ಯ set_reexports(ಶೂನ್ಯ)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	ವರ್ಣ *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->ಮೌಲ್ಯ);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ಸ್ಥಿರ ಶೂನ್ಯ svc_start_real(ಶೂನ್ಯ)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	ಯದಿ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	ಯದಿ (ibsave)
		unsetenv("IN_BACKGROUND");

	ಯದಿ (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	ಯದಿ (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	ಅನ್ಯಥಾ ಯದಿ (!started)
		eerrorx("ERROR: %s failed to start", applet);

	ಯದಿ (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    ಯದಿ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
		    service_start(svc->ಮೌಲ್ಯ);
	rc_stringlist_free(services);
	services = ರಿಕ್ತ;

	/* Do the same for any services we provide */
	ಯದಿ (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->ಮೌಲ್ಯ);
			TAILQ_FOREACH(svc2, services, entries)
				ಯದಿ (rc_service_state(svc2->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
					service_start(svc2->ಮೌಲ್ಯ);
			rc_stringlist_free(services);
			services = ರಿಕ್ತ;
		}
		rc_stringlist_free(tmplist);
		tmplist = ರಿಕ್ತ;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
svc_start(ಶೂನ್ಯ)
{
	ಯದಿ (dry_run)
		einfon("start:");
	ಅನ್ಯಥಾ
		svc_start_check();

	ಯದಿ (deps)
		svc_start_deps();

	ಯದಿ (dry_run)
		printf(" %s\n", applet);
	ಅನ್ಯಥಾ
		svc_start_real();

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	ಯದಿ (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		ಗಚ್ಛ exit;

	ಯದಿ (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		ಗಚ್ಛ exit;

	ಯದಿ (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಯದಿ (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	ಯದಿ (exclusive_fd == -1) {
		ಯದಿ (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		ಅನ್ಯಥಾ ಯದಿ (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		ಅನ್ಯಥಾ
			eerror("ERROR: %s stopped by something else", applet);
		ಗಚ್ಛ exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	ಯದಿ (!rc_runlevel_stopping()) {
		ಯದಿ (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		ಅನ್ಯಥಾ ಯದಿ (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	ಪ್ರತ್ಯಾಗಮ 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_stop_deps(RC_SERVICE state)
{
	ಪೂರ್ಣಾಙ್ಕ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	ಯದಿ (state & RC_SERVICE_WASINACTIVE)
		ಪ್ರತ್ಯಾಗಮ;

	errno = 0;
	ಯದಿ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ಯದಿ (!deptree && ((deptree = _rc_deptree_load(0, ರಿಕ್ತ)) == ರಿಕ್ತ)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	ಯದಿ (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->ಮೌಲ್ಯ);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		ಯದಿ (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			ಅನುವರ್ತ;
		ಯದಿ (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			ಅನುವರ್ತ;
		ಯದಿ (dry_run) {
			printf(" %s", svc->ಮೌಲ್ಯ);
			ಅನುವರ್ತ;
		}
		svc_wait(svc->ಮೌಲ್ಯ);
		ಯದಿ (rc_service_state(svc->ಮೌಲ್ಯ) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->ಮೌಲ್ಯ);
			ಯದಿ (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->ಮೌಲ್ಯ);
		}
	}
	rc_stringlist_free(services);
	services = ರಿಕ್ತ;
	ಯದಿ (dry_run)
		ಪ್ರತ್ಯಾಗಮ;

	TAILQ_FOREACH(svc, tmplist, entries) {
		ಯದಿ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED) {
			ಅನುವರ್ತ;
		}
		svc_wait(svc->ಮೌಲ್ಯ);
		ಯದಿ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
			ಅನುವರ್ತ;
		ಯದಿ (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			ಯದಿ (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				ಅನುವರ್ತ;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->ಮೌಲ್ಯ);
	}
	rc_stringlist_free(tmplist);
	tmplist = ರಿಕ್ತ;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		ಯದಿ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
			ಅನುವರ್ತ;
		svc_wait(svc->ಮೌಲ್ಯ);
	}
	rc_stringlist_free(services);
	services = ರಿಕ್ತ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
load_dep_env(ಶೂನ್ಯ)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->ಮೌಲ್ಯ);

	rc_stringlist_free(depsvcs);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_stop_real(ಶೂನ್ಯ)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	ಯದಿ (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	ಯದಿ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	ಯದಿ (ibsave)
		unsetenv("IN_BACKGROUND");

	ಯದಿ (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	ಯದಿ (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
svc_stop(ಶೂನ್ಯ)
{
	RC_SERVICE state = 0;

	ಯದಿ (dry_run)
		einfon("stop:");
	ಅನ್ಯಥಾ
		ಯದಿ (svc_stop_check(&state) == 1)
			ಪ್ರತ್ಯಾಗಮ 1; /* Service has been stopped already */

	ಯದಿ (deps)
		svc_stop_deps(state);

	ಯದಿ (dry_run)
		printf(" %s\n", applet);
	ಅನ್ಯಥಾ
		svc_stop_real();

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
svc_restart(ಶೂನ್ಯ)
{
	ಯದಿ (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		ಯದಿ (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ bool
service_plugable(ಶೂನ್ಯ)
{
	ವರ್ಣ *list, *p, *token;
	bool allow = true, truefalse;
	ವರ್ಣ *match = rc_conf_ಮೌಲ್ಯ("rc_hotplug");

	ಯದಿ (!match)
		match = rc_conf_ಮೌಲ್ಯ("rc_plug_services");
	ಯದಿ (!match)
		ಪ್ರತ್ಯಾಗಮ false;

	list = xstrdup(match);
	p = list;
	ಯಾವತ್ ((token = strsep(&p, " "))) {
		ಯದಿ (token[0] == '!') {
			truefalse = false;
			token++;
		} ಅನ್ಯಥಾ {
			truefalse = true;
		}

		ಯದಿ (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			ಭಙ್ಗ;
		}
	}
	ಮುಕ್ತ(list);
	ಪ್ರತ್ಯಾಗಮ allow;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
simple_command(ಶೂನ್ಯ)
{
	ವರ್ಣ *save = prefix;
	ಪೂರ್ಣಾಙ್ಕ retval;

	eprefix(ರಿಕ್ತ);
	prefix = ರಿಕ್ತ;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
resolve_deptype(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	ಯದಿ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ಯದಿ (!deptree && ((deptree = _rc_deptree_load(0, ರಿಕ್ತ)) == ರಿಕ್ತ))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ರಿಕ್ತ;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->ಮೌಲ್ಯ);
	printf ("\n");

	rc_stringlist_free(services);
	services = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cmd_condrestart(ಶೂನ್ಯ)
{
	ಯದಿ (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cmd_stop(ಶೂನ್ಯ)
{
	RC_STRING *svc;

	ಯದಿ (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	ಯದಿ (deps && in_background)
		get_started_services();

	ಯದಿ (svc_stop() == 1)
		ಪ್ರತ್ಯಾಗಮ 0; /* Service has been stopped already */

	ಯದಿ (deps) {
		ಯದಿ (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		ಯದಿ (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				ಯದಿ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->ಮೌಲ್ಯ);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
cmd_zap(ಶೂನ್ಯ)
{
	einfo("Manually resetting %s to stopped state", applet);
	ಯದಿ (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸಂರಚನಾ {
	ಧ್ರುವ ವರ್ಣ *name;
	ಪೂರ್ಣಾಙ್ಕ (*handler)(ಶೂನ್ಯ);
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

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ retval, opt;
	ಧ್ರುವ ವರ್ಣ *workingdir = "/";
	ವರ್ಣ *pidstr = ರಿಕ್ತ;
	size_t l = 0, ll;
	ಸಂರಚನಾ stat stbuf;

	/* Show help if insufficient args */
	ಯದಿ (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	ಯದಿ (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	ಯದಿ (!(service = (realpath(argv[1], ರಿಕ್ತ))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	ಯದಿ (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring, longopts, ರಿಕ್ತ)) != -1)
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'd':
			setenv("RC_DEBUG", "YES", 1);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			ಭಙ್ಗ;
		ಪಕ್ಷ 's':
			ಯದಿ (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'S':
			ಯದಿ (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'D':
			deps = false;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'Z':
			dry_run = true;
			ಭಙ್ಗ;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ಯದಿ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ಯದಿ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	ಯದಿ ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ರಿಕ್ತ) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (ಪೂರ್ಣಾಙ್ಕ) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	ಮುಕ್ತ(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	ಯದಿ (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(ರಿಕ್ತ);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->ಮೌಲ್ಯ);
			ಯದಿ (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ರಿಕ್ತ;
		ll = strlen(applet);
		ಯದಿ (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(ಪರಿಮಾಣ(ವರ್ಣ) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	ಯದಿ (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	ಯದಿ (deps && getenv("RC_PID") == ರಿಕ್ತ && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	ಯದಿ (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	ಯದಿ (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	ಯದಿ (rc_yesno(getenv("IN_HOTPLUG"))) {
		ಯದಿ (!service_plugable())
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

	ಯದಿ (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	ಯಾವತ್ (optind < argc) {
		ಪೂರ್ಣಾಙ್ಕ cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		ಯದಿ (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			ಯದಿ (strcmp(optarg, commands[i].name) != 0)
				ಅನುವರ್ತ;

			ಯದಿ ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			ಯದಿ (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ರಿಕ್ತ;
			}

			ಗಚ್ಛ next_cmd;
		}

		ಯದಿ ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ retval;
}
