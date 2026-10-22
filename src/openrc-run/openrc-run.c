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

#ಆದರೆ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ಅಥವಾಆದರೆ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ಇಲ್ಲದಿದ್ದರೆ
#  include <libutil.h>
#ಆದರೆ_ಅಂತ್ಯ

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

#ವ್ಯಾಖ್ಯಾನಿಸು WAIT_TIMEOUT	60		/* seconds until we timeout */
#ವ್ಯಾಖ್ಯಾನಿಸು WARN_TIMEOUT	10		/* warn about this every N seconds */

ಬಾಹ್ಯ ಅಕ್ಷರ **environ;

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = "stop | start | restart | status | describe | zap";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "debug",      0, ಖಾಲಿ, 'd'},
	{ "dry-run",    0, ಖಾಲಿ, 'Z'},
	{ "ifstarted",  0, ಖಾಲಿ, 's'},
	{ "ifstopped",  0, ಖಾಲಿ, 'S'},
	{ "nodeps",     0, ಖಾಲಿ, 'D'},
	{ "lockfd",     1, ಖಾಲಿ, 'l'},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಸ್ಥಿರ ಅಕ್ಷರ *service, *runlevel, *ibsave, *ಪೂರ್ವಪ್ರತ್ಯಯ;
ಸ್ಥಿರ RC_DEPTREE *deptree;
ಸ್ಥಿರ RC_STRINGLIST *applet_list, *services, *tmplist;
ಸ್ಥಿರ RC_STRINGLIST *restart_services;
ಸ್ಥಿರ RC_STRINGLIST *need_services;
ಸ್ಥಿರ RC_STRINGLIST *use_services;
ಸ್ಥಿರ RC_STRINGLIST *want_services;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ exclusive_fd = -1, master_tty = -1;
ಸ್ಥಿರ bool ಒಳ_background, deps, dry_ಸತತ;
ಸ್ಥಿರ ಅಸ್ಥಿರ bool sighup, skip_mark, timedout;
ಸ್ಥಿರ pid_t service_pid;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_pipe[2] = { -1, -1 };

ಸ್ಥಿರ RC_STRINGLIST *deptypes_b;	/* broken deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_n;	/* needed deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ಸ್ಥಿರ RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ಸ್ಥಿರ ಶೂನ್ಯ
handle_signal(ಪೂರ್ಣಾಂಕ sig)
{
	ಪೂರ್ಣಾಂಕ serrno = errno, ಸ್ಥಿತಿ_2;
	pid_t pid;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *signame = ಖಾಲಿ;
	ರಚನೆ winsize ws;

	ಆಯ್ಕೆ (sig) {
	ಸ್ಥಿತಿ SIGHUP:
		sighup = true;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ SIGUSR1:
		skip_mark = true;
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ SIGCHLD:
		ಆಗ ((pid = waitpid(-1, &ಸ್ಥಿತಿ_2, WNOHANG)) > 0) {
			ಆದರೆ (signal_pipe[1] > -1 && pid == service_pid) {
				ಆದರೆ (ಲಿಖ(signal_pipe[1], &ಸ್ಥಿತಿ_2, ಗಾತ್ರ(ಸ್ಥಿತಿ_2)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ SIGWINCH:
		ಆದರೆ (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ SIGINT:
		ಆದರೆ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ಸ್ಥಿತಿ SIGTERM:
		ಆದರೆ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ಸ್ಥಿತಿ SIGQUIT:
		ಆದರೆ (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		ಆದರೆ (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
		/* NOTREACHED */

	ಸ್ಥಿತಿ SIGALRM:
		timedout = true;
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ಶೂನ್ಯ
unhotplug(ಶೂನ್ಯ)
{
	ಆದರೆ (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಆರಂಭ_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	ಆದರೆ (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		ಹಿಂತಿರುಗಿ;

	TAILQ_FOREACH(svc, list, entries) {
		ಆದರೆ (!(rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED))
			ಮುಂದುವರಿ;
		ಆದರೆ (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_ವೇಳಾಪಟ್ಟಿ_ಆರಂಭ(applet, svc->ಮೌಲ್ಯ);
			ewarn("WARNING: %s will start when %s has started", svc->ಮೌಲ್ಯ, applet);
		} ಇಲ್ಲದಿದ್ದರೆ {
			service_ಆರಂಭ(svc->ಮೌಲ್ಯ);
		}
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
restore_state(ಶೂನ್ಯ)
{
	RC_SERVICE state;

	ಆದರೆ (rc_ಒಳ_plugin || exclusive_fd == -1)
		ಹಿಂತಿರುಗಿ;
	state = rc_service_state(applet);
	ಆದರೆ (state & RC_SERVICE_STOPPING) {
		ಆದರೆ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ಇಲ್ಲದಿದ್ದರೆ
			rc_service_mark(applet, RC_SERVICE_STARTED);

		ಆದರೆ (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_STARTING) {
		ಆದರೆ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ಇಲ್ಲದಿದ್ದರೆ
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		ಆದರೆ (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ಸ್ಥಿರ ಶೂನ್ಯ
cleanup(ಶೂನ್ಯ)
{
	restore_state();

	ಆದರೆ (!rc_ಒಳ_plugin) {
		ಆದರೆ (restart_services)
			ಆರಂಭ_services(restart_services);
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
	ಬಿಡುಗಡೆ(ibsave);
	ಬಿಡುಗಡೆ(ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಬಿಡುಗಡೆ(runlevel);
	ಬಿಡುಗಡೆ(service);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_getenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	ಅಕ್ಷರ *var = ಖಾಲಿ;

	TAILQ_FOREACH(reexport, reexports, entries) {
		ಆದರೆ (rc_service_getenv(svc, reexport->ಮೌಲ್ಯ, &var) == -1)
			ಮುಂದುವರಿ;
		setenv(reexport->ಮೌಲ್ಯ, var, true);
	}

	ಬಿಡುಗಡೆ(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
write_ಪೂರ್ವಪ್ರತ್ಯಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಫರ್, size_t ಬೈಟುಗಳು, bool *prefixed)
{
	size_t i, j;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ec = ecolor(ECOLOR_HILITE);
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ec_ಸಾಮಾನ್ಯ = ecolor(ECOLOR_ಸಾಮಾನ್ಯ);
	ssize_t ret = 0;
	ಪೂರ್ಣಾಂಕ fd = fileno(stdout), ಬೀಗ_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	ಬೀಗ_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	ಆದರೆ (ಬೀಗ_fd != -1) {
		ಆಗ (flock(ಬೀಗ_fd, LOCK_EX) != 0) {
			ಆದರೆ (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				ನಿಲ್ಲಿಸು;
			}
		}
	}
	ಇಲ್ಲದಿದ್ದರೆ
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	ಗಾಗಿ (i = 0; i < ಬೈಟುಗಳು; i++) {
		/* We don't prefix eend calls (cursor up) */
		ಆದರೆ (ಬಫರ್[i] == '\033' && !*prefixed) {
			ಗಾಗಿ (j = i + 1; j < ಬೈಟುಗಳು; j++) {
				ಆದರೆ (ಬಫರ್[j] == 'A')
					*prefixed = true;
				ಆದರೆ (ಅಕ್ಷರವೇ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಬಫರ್[j]))
					ನಿಲ್ಲಿಸು;
			}
		}

		ಆದರೆ (!*prefixed) {
			ret += ಲಿಖ(fd, ec, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ec));
			ret += ಲಿಖ(fd, ಪೂರ್ವಪ್ರತ್ಯಯ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಪೂರ್ವಪ್ರತ್ಯಯ));
			ret += ಲಿಖ(fd, ec_ಸಾಮಾನ್ಯ, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ec_ಸಾಮಾನ್ಯ));
			ret += ಲಿಖ(fd, "|", 1);
			*prefixed = true;
		}

		ಆದರೆ (ಬಫರ್[i] == '\n')
			*prefixed = false;
		ret += ಲಿಖ(fd, ಬಫರ್ + i, 1);
	}

	/* Release the lock */
	close(ಬೀಗ_fd);

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
svc_exec(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *command)
{
	ಪೂರ್ಣಾಂಕ ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ರಚನೆ termios tt;
	ರಚನೆ winsize ws;
	ಪೂರ್ಣಾಂಕ flags = 0;
	ರಚನೆ pollfd fd[2];
	bool prefixed = false;
	ಪೂರ್ಣಾಂಕ slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	ಅಕ್ಷರ *openrc_sh = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ಖಾಲಿ
	};

	/* Setup our signal pipe */
	ಆದರೆ (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಆದರೆ ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		ಹಿಂತಿರುಗಿ 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		ಆದರೆ (openpty(&master_tty, &slave_tty, ಖಾಲಿ, &tt, &ws) == 0) {
			ಆದರೆ (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ಆದರೆ (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ಆದರೆ ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				ಹಿಂತಿರುಗಿ 1;
			}
		}
	}

	ಆದರೆ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	ಆದರೆ ((errno = posix_spawn(&service_pid, argv[0], &tty, ಖಾಲಿ, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		ಹಿಂತಿರುಗಿ 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	ಬಿಡುಗಡೆ(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	ಗಾಗಿ (;;) {
		ಆದರೆ (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			ಆದರೆ (errno == EINTR)
				ಮುಂದುವರಿ;
			eerror("%s: poll: %s", applet, strerror(errno));
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (fd[1].revents & (POLLIN | POLLHUP)) {
			ಅಕ್ಷರ ಬಫರ್[BUFSIZ];
			write_ಪೂರ್ವಪ್ರತ್ಯಯ(ಬಫರ್, ಪಠ(master_tty, ಬಫರ್, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		ಆದರೆ (fd[0].revents & (POLLIN | POLLHUP)) {
			ಆದರೆ (errno = 0, ಪಠ(signal_pipe[0], &ret, ಗಾತ್ರ(ret)) < (ssize_t)ಗಾತ್ರ(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			ನಿಲ್ಲಿಸು;
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

	sigprocmask (SIG_SETMASK, &oldmask, ಖಾಲಿ);

	ಆದರೆ (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ bool
svc_wait(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svc)
{
	ಪೂರ್ಣಾಂಕ fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = basename_c(svc);
	ಪೂರ್ಣಾಂಕ timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(keywords, "-timeout") || rc_stringlist_ಕಂಡುಹಿಡಿ(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	ಆದರೆ ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), ತಳ, O_RDONLY | O_NONBLOCK)) == -1)
		ಹಿಂತಿರುಗಿ errno == ENOENT;

	timedout = false;
	ಗಾಗಿ (;;) {
		ಆದರೆ (!forever)
			alarm(WARN_TIMEOUT);

		ಆದರೆ (flock(fd, LOCK_SH) == 0) {
			retval = true;
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (errno != EINTR)
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (!forever && timedout) {
			timedout = false;
			ಆದರೆ ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, ತಳ, timeout);
			ಇಲ್ಲದಿದ್ದರೆ
				ನಿಲ್ಲಿಸು;
		}
	}

	alarm(0);
	close(fd);
	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಶೂನ್ಯ
get_started_services(ಶೂನ್ಯ)
{
	RC_STRINGLIST *tmp = rc_services_ಒಳ_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_ಒಳ_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	ಬಿಡುಗಡೆ(tmp);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಸಿದ್ಧತೆ_deptypes(ಶೂನ್ಯ)
{
	deptypes_b = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_b, "broken");

	deptypes_n = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_n, "ineed");

	deptypes_nw = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_nw, "ineed");
	rc_stringlist_ಕೂಡಿಸು(deptypes_nw, "iwant");

	deptypes_nwu = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwu, "ineed");
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwu, "iwant");
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwu, "iuse");

	deptypes_nwua = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwua, "ineed");
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwua, "iwant");
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwua, "iuse");
	rc_stringlist_ಕೂಡಿಸು(deptypes_nwua, "iafter");

	deptypes_m = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_m, "needsme");

	deptypes_mwua = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(deptypes_mwua, "needsme");
	rc_stringlist_ಕೂಡಿಸು(deptypes_mwua, "wantsme");
	rc_stringlist_ಕೂಡಿಸು(deptypes_mwua, "usesme");
	rc_stringlist_ಕೂಡಿಸು(deptypes_mwua, "beforeme");
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_ಆರಂಭ_check(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	ಆದರೆ (ಒಳ_background) {
		ಆದರೆ (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			ಹೋಗು ನಿರ್ಗಮಿಸು;

		ಆದರೆ (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		ಆದರೆ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	ಆದರೆ (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_INACTIVE && !ಒಳ_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	ಆದರೆ (exclusive_fd == -1)
		exclusive_fd = svc_ಬೀಗ(applet, !deps);
	ಆದರೆ (exclusive_fd == -1) {
		ಆದರೆ (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			ಹೋಗು ನಿರ್ಗಮಿಸು;
		}

		ಆದರೆ (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		ಇಲ್ಲದಿದ್ದರೆ
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಒಳ, applet);

	ಹಿಂತಿರುಗಿ;

ನಿರ್ಗಮಿಸು:
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಹೊರಹೋಗು, applet);
	ನಿರ್ಗಮಿಸು(ret);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_ಆರಂಭ_deps(ಶೂನ್ಯ)
{
	bool ಮೊದಲ;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	ಪೂರ್ಣಾಂಕ depoptions = RC_DEP_TRACE;
	size_t len, svc_ಎಣಿಕೆ;
	ಅಕ್ಷರ *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	ಆದರೆ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ಆದರೆ (!deptree && ((deptree = rc_deptree_ತುಂಬು_2(0, ಖಾಲಿ)) == ಖಾಲಿ)) {
		eerror("failed to load deptree");
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	}

	ಆದರೆ (!deptypes_b)
		ಸಿದ್ಧತೆ_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	ಆದರೆ (TAILQ_ಮೊದಲ(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		ಮೊದಲ = true;
		TAILQ_FOREACH(svc, services, entries) {
			ಆದರೆ (ಮೊದಲ)
				ಮೊದಲ = false;
			ಇಲ್ಲದಿದ್ದರೆ
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->ಮೌಲ್ಯ);
		}
		fprintf(stderr, "\n");
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	}
	rc_stringlist_free(services);
	services = ಖಾಲಿ;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	ಆದರೆ (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->ಮೌಲ್ಯ);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			ಆದರೆ (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				ಮುಂದುವರಿ;
			ಆದರೆ (state & RC_SERVICE_STOPPED) {
				ಆದರೆ (dry_ಸತತ) {
					ಬರೆ(" %s", svc->ಮೌಲ್ಯ);
					ಮುಂದುವರಿ;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_ಆರಂಭ(svc->ಮೌಲ್ಯ);
				ಆದರೆ (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	ಆದರೆ (dry_ಸತತ)
		ಹಿಂತಿರುಗಿ;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->ಮೌಲ್ಯ);
		ಆದರೆ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ಮೌಲ್ಯ);
			ಮುಂದುವರಿ;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		ಆದರೆ (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			ಆದರೆ (!rc_stringlist_ಕಂಡುಹಿಡಿ(need_services, svc->ಮೌಲ್ಯ) &&
					!rc_stringlist_ಕಂಡುಹಿಡಿ(want_services, svc->ಮೌಲ್ಯ) &&
					!rc_stringlist_ಕಂಡುಹಿಡಿ(use_services, svc->ಮೌಲ್ಯ))
				ಮುಂದುವರಿ;
		}

		ಆದರೆ (!svc_wait(svc->ಮೌಲ್ಯ))
			eerror("%s: timed out waiting for %s", applet, svc->ಮೌಲ್ಯ);
		state = rc_service_state(svc->ಮೌಲ್ಯ);
		ಆದರೆ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ಮೌಲ್ಯ);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(need_services, svc->ಮೌಲ್ಯ)) {
			ಆದರೆ (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_ಕೂಡಿಸು(tmplist, svc->ಮೌಲ್ಯ);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!TAILQ_ಮೊದಲ(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->ಮೌಲ್ಯ);
				ಹೋಗು ನಿರ್ಗಮಿಸು;
			}
		}
	}

	ಆದರೆ (TAILQ_ಮೊದಲ(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ಖಾಲಿ;
		mem = xopen_memstream(&tmp, &len);
		svc_ಎಣಿಕೆ = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_ವೇಳಾಪಟ್ಟಿ_ಆರಂಭ(svc->ಮೌಲ್ಯ, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->ಮೌಲ್ಯ);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_ವೇಳಾಪಟ್ಟಿ_ಆರಂಭ(svc2->ಮೌಲ್ಯ, applet);
			rc_stringlist_free(use_services);
			use_services = ಖಾಲಿ;
			ಆದರೆ (svc_ಎಣಿಕೆ > 0)
				fputs(", ", mem);
			fputs(svc->ಮೌಲ್ಯ, mem);
			++svc_ಎಣಿಕೆ;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ಖಾಲಿ;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_ಎಣಿಕೆ > 1 ? "have" : "has");
		ಬಿಡುಗಡೆ(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ಖಾಲಿ;
	rc_stringlist_free(services);
	services = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ;

ನಿರ್ಗಮಿಸು:
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಹೊರಹೋಗು, applet);
	ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}


ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_reexports(ಶೂನ್ಯ)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &ಗಾತ್ರ_2);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->ಮೌಲ್ಯ);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ಸ್ಥಿರ ಶೂನ್ಯ svc_ಆರಂಭ_real(ಶೂನ್ಯ)
{
	bool started;
	RC_STRING *svc, *svc2;

	ಗಣ_reexports();

	ಆದರೆ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	ಆದರೆ (ibsave)
		unsetenv("IN_BACKGROUND");

	ಆದರೆ (!started) {
		rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಮುಗಿದ, applet);
		rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಹೊರಹೋಗು, applet);
	}

	ಆದರೆ (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!started)
		eerrorx("ERROR: %s failed to start", applet);

	ಆದರೆ (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಮುಗಿದ, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    ಆದರೆ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
		    service_ಆರಂಭ(svc->ಮೌಲ್ಯ);
	rc_stringlist_free(services);
	services = ಖಾಲಿ;

	/* Do the same for any services we provide */
	ಆದರೆ (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->ಮೌಲ್ಯ);
			TAILQ_FOREACH(svc2, services, entries)
				ಆದರೆ (rc_service_state(svc2->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
					service_ಆರಂಭ(svc2->ಮೌಲ್ಯ);
			rc_stringlist_free(services);
			services = ಖಾಲಿ;
		}
		rc_stringlist_free(tmplist);
		tmplist = ಖಾಲಿ;
	}

	rc_plugin_ಸತತ(RC_HOOK_SERVICE_ಆರಂಭ_ಹೊರಹೋಗು, applet);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
svc_ಆರಂಭ(ಶೂನ್ಯ)
{
	ಆದರೆ (dry_ಸತತ)
		einfon("start:");
	ಇಲ್ಲದಿದ್ದರೆ
		svc_ಆರಂಭ_check();

	ಆದರೆ (deps)
		svc_ಆರಂಭ_deps();

	ಆದರೆ (dry_ಸತತ)
		ಬರೆ(" %s\n", applet);
	ಇಲ್ಲದಿದ್ದರೆ
		svc_ಆರಂಭ_real();

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	ಆದರೆ (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		ಹೋಗು ನಿರ್ಗಮಿಸು;

	ಆದರೆ (ಒಳ_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		ಹೋಗು ನಿರ್ಗಮಿಸು;

	ಆದರೆ (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಆದರೆ (exclusive_fd == -1)
		exclusive_fd = svc_ಬೀಗ(applet, !deps);
	ಆದರೆ (exclusive_fd == -1) {
		ಆದರೆ (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		ಇಲ್ಲದಿದ್ದರೆ
			eerror("ERROR: %s stopped by something else", applet);
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_ಒಳ, applet);

	ಆದರೆ (!rc_runlevel_stopping()) {
		ಆದರೆ (rc_service_ಒಳ_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rc_service_ಒಳ_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	ಹಿಂತಿರುಗಿ 0;

ನಿರ್ಗಮಿಸು:
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_ಹೊರಹೋಗು, applet);
	ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}

ಸ್ಥಿರ ಶೂನ್ಯ
svc_stop_deps(RC_SERVICE state)
{
	ಪೂರ್ಣಾಂಕ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	ಆದರೆ (state & RC_SERVICE_WASINACTIVE)
		ಹಿಂತಿರುಗಿ;

	errno = 0;
	ಆದರೆ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ಆದರೆ (!deptree && ((deptree = rc_deptree_ತುಂಬು_2(0, ಖಾಲಿ)) == ಖಾಲಿ)) {
		rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_ಹೊರಹೋಗು, applet);
		eerrorx("failed to load deptree");
	}

	ಆದರೆ (!deptypes_m)
		ಸಿದ್ಧತೆ_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_ಹಿಮ್ಮುಖ(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->ಮೌಲ್ಯ);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		ಆದರೆ (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			ಮುಂದುವರಿ;
		ಆದರೆ (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			ಮುಂದುವರಿ;
		ಆದರೆ (dry_ಸತತ) {
			ಬರೆ(" %s", svc->ಮೌಲ್ಯ);
			ಮುಂದುವರಿ;
		}
		svc_wait(svc->ಮೌಲ್ಯ);
		ಆದರೆ (rc_service_state(svc->ಮೌಲ್ಯ) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->ಮೌಲ್ಯ);
			ಆದರೆ (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_ಕೂಡಿಸು(tmplist, svc->ಮೌಲ್ಯ);
		}
	}
	rc_stringlist_free(services);
	services = ಖಾಲಿ;
	ಆದರೆ (dry_ಸತತ)
		ಹಿಂತಿರುಗಿ;

	TAILQ_FOREACH(svc, tmplist, entries) {
		ಆದರೆ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED) {
			ಮುಂದುವರಿ;
		}
		svc_wait(svc->ಮೌಲ್ಯ);
		ಆದರೆ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
			ಮುಂದುವರಿ;
		ಆದರೆ (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			ಆದರೆ (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				ಮುಂದುವರಿ;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_ಹೊರಹೋಗು, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->ಮೌಲ್ಯ);
	}
	rc_stringlist_free(tmplist);
	tmplist = ಖಾಲಿ;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		ಆದರೆ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
			ಮುಂದುವರಿ;
		svc_wait(svc->ಮೌಲ್ಯ);
	}
	rc_stringlist_free(services);
	services = ಖಾಲಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ತುಂಬು_dep_env(ಶೂನ್ಯ)
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

	ತುಂಬು_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	ಆದರೆ (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	ಆದರೆ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	ಆದರೆ (ibsave)
		unsetenv("IN_BACKGROUND");

	ಆದರೆ (stopped && !skip_mark)
		rc_service_mark(applet, ಒಳ_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_ಮುಗಿದ, applet);
	rc_plugin_ಸತತ(RC_HOOK_SERVICE_STOP_ಹೊರಹೋಗು, applet);

	ಆದರೆ (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
svc_stop(ಶೂನ್ಯ)
{
	RC_SERVICE state = 0;

	ಆದರೆ (dry_ಸತತ)
		einfon("stop:");
	ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (svc_stop_check(&state) == 1)
			ಹಿಂತಿರುಗಿ 1; /* Service has been stopped already */

	ಆದರೆ (deps)
		svc_stop_deps(state);

	ಆದರೆ (dry_ಸತತ)
		ಬರೆ(" %s\n", applet);
	ಇಲ್ಲದಿದ್ದರೆ
		svc_stop_real();

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
svc_restart(ಶೂನ್ಯ)
{
	ಆದರೆ (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		ಆದರೆ (dry_ಸತತ)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_ಆರಂಭ();
	ಆರಂಭ_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ bool
service_plugable(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *list, *p, *token;
	bool allow = true, truefalse;
	ಅಕ್ಷರ *ಹೊಂದಿಕೆ = rc_conf_ಮೌಲ್ಯ("rc_hotplug");

	ಆದರೆ (!ಹೊಂದಿಕೆ)
		ಹೊಂದಿಕೆ = rc_conf_ಮೌಲ್ಯ("rc_plug_services");
	ಆದರೆ (!ಹೊಂದಿಕೆ)
		ಹಿಂತಿರುಗಿ false;

	list = xstrdup(ಹೊಂದಿಕೆ);
	p = list;
	ಆಗ ((token = strsep(&p, " "))) {
		ಆದರೆ (token[0] == '!') {
			truefalse = false;
			token++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			truefalse = true;
		}

		ಆದರೆ (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			ನಿಲ್ಲಿಸು;
		}
	}
	ಬಿಡುಗಡೆ(list);
	ಹಿಂತಿರುಗಿ allow;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
simple_command(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *save = ಪೂರ್ವಪ್ರತ್ಯಯ;
	ಪೂರ್ಣಾಂಕ retval;

	eprefix(ಖಾಲಿ);
	ಪೂರ್ವಪ್ರತ್ಯಯ = ಖಾಲಿ;

	retval = svc_exec(optarg);

	eprefix(save);
	ಪೂರ್ವಪ್ರತ್ಯಯ = save;

	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
resolve_deptype(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	ಆದರೆ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ಆದರೆ (!deptree && ((deptree = rc_deptree_ತುಂಬು_2(0, ಖಾಲಿ)) == ಖಾಲಿ))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ಖಾಲಿ;
	TAILQ_FOREACH(svc, services, entries)
		ಬರೆ("%s ", svc->ಮೌಲ್ಯ);
	ಬರೆ ("\n");

	rc_stringlist_free(services);
	services = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cmd_condrestart(ಶೂನ್ಯ)
{
	ಆದರೆ (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cmd_stop(ಶೂನ್ಯ)
{
	RC_STRING *svc;

	ಆದರೆ (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	ಆದರೆ (deps && ಒಳ_background)
		get_started_services();

	ಆದರೆ (svc_stop() == 1)
		ಹಿಂತಿರುಗಿ 0; /* Service has been stopped already */

	ಆದರೆ (deps) {
		ಆದರೆ (!ಒಳ_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		ಆದರೆ (ಒಳ_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				ಆದರೆ (rc_service_state(svc->ಮೌಲ್ಯ) & RC_SERVICE_STOPPED)
					rc_service_ವೇಳಾಪಟ್ಟಿ_ಆರಂಭ(applet, svc->ಮೌಲ್ಯ);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
cmd_zap(ಶೂನ್ಯ)
{
	einfo("Manually resetting %s to stopped state", applet);
	ಆದರೆ (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	ಹಿಂತಿರುಗಿ 0;
}

ರಚನೆ {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಪೂರ್ಣಾಂಕ (*handler)(ಶೂನ್ಯ);
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
	{ "start",              svc_ಆರಂಭ,       true  },
	{ "stop",               cmd_stop,        true  },
	{ "restart",            svc_restart,     true  },
	{ "condrestart",        cmd_condrestart, true  },
	{ "conditionalrestart", cmd_condrestart, true  },
	{ "zap",                cmd_zap,         true  },
};

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ retval, opt;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *workingdir = "/";
	ಅಕ್ಷರ *pidstr = ಖಾಲಿ;
	size_t l = 0, ll;
	ರಚನೆ stat stbuf;

	/* Show help if insufficient args */
	ಆದರೆ (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	ಆದರೆ (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}

	ಆದರೆ (!(service = (realpath(argv[1], ಖಾಲಿ))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	ಆದರೆ (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_ಸಿದ್ಧತೆ(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring, longopts, ಖಾಲಿ)) != -1)
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'd':
			setenv("RC_DEBUG", "YES", 1);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
			ಆದರೆ (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'S':
			ಆದರೆ (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'D':
			deps = false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'Z':
			dry_ಸತತ = true;
			ನಿಲ್ಲಿಸು;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ಆದರೆ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ಆದರೆ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	ಆದರೆ ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ಖಾಲಿ) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (ಪೂರ್ಣಾಂಕ) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	ಬಿಡುಗಡೆ(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	ಆದರೆ (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
		TAILQ_FOREACH(svc, services, entries) {
			ll = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(svc->ಮೌಲ್ಯ);
			ಆದರೆ (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ಖಾಲಿ;
		ll = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(applet);
		ಆದರೆ (ll > l)
			l = ll;

		/* Make our prefix string */
		ಪೂರ್ವಪ್ರತ್ಯಯ = xmalloc(ಗಾತ್ರ(ಅಕ್ಷರ) * l + 1);
		ಸ್ಮರಣೆ_ನಕಲು(ಪೂರ್ವಪ್ರತ್ಯಯ, applet, ll);
		memset(ಪೂರ್ವಪ್ರತ್ಯಯ + ll, ' ', l - ll);
		memset(ಪೂರ್ವಪ್ರತ್ಯಯ + l, 0, 1);
		eprefix(ಪೂರ್ವಪ್ರತ್ಯಯ);
	}

	ಆದರೆ (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	ಆದರೆ (deps && getenv("RC_PID") == ಖಾಲಿ && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	ಆದರೆ (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		ಒಳ_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	ಆದರೆ (rc_yesno(getenv("IN_DRYRUN")))
		dry_ಸತತ = true;
	ಆದರೆ (rc_yesno(getenv("IN_HOTPLUG"))) {
		ಆದರೆ (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		ಒಳ_background = true;
	}

	/* Setup a signal handler */
	signal_ಸಿದ್ಧತೆ(SIGHUP, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGALRM, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGUSR1, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGINT, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGQUIT, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGTERM, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGCHLD, handle_signal);

	/* Load our plugins */
	rc_plugin_ತುಂಬು();

	applet_list = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(applet_list, applet);

	ಆದರೆ (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

ಮುಂದಿನ_cmd:
	ಆಗ (optind < argc) {
		ಪೂರ್ಣಾಂಕ cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		ಆದರೆ (sighup)
			ನಿರ್ಗಮಿಸು (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(commands); i++) {
			ಆದರೆ (strcmp(optarg, commands[i].name) != 0)
				ಮುಂದುವರಿ;

			ಆದರೆ ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			ಆದರೆ (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ಖಾಲಿ;
			}

			ಹೋಗು ಮುಂದಿನ_cmd;
		}

		ಆದರೆ ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ಖಾಲಿ;
	}

	ಹಿಂತಿರುಗಿ retval;
}
