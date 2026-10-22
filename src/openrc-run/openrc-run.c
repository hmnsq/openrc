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

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fnmatch.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libgen.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spawn.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/file.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <termios.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>

#ਜੇ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ਜਾਂਜੇ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ਨਹੀਂਤਾਂ
#  include <libutil.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "plugin.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "selinux.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAIT_TIMEOUT	60		/* seconds until we timeout */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WARN_TIMEOUT	10		/* warn about this every N seconds */

ਬਾਹਰੀ ਅੱਖਰ **environ;

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = "stop | start | restart | status | describe | zap";
ਅਚਲ ਅੱਖਰ getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "debug",      0, ਨਿਰਾ, 'd'},
	{ "dry-run",    0, ਨਿਰਾ, 'Z'},
	{ "ifstarted",  0, ਨਿਰਾ, 's'},
	{ "ifstopped",  0, ਨਿਰਾ, 'S'},
	{ "nodeps",     0, ਨਿਰਾ, 'D'},
	{ "lockfd",     1, ਨਿਰਾ, 'l'},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ *ਅਚਲ longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਸਥਿਰ ਅੱਖਰ *service, *runlevel, *ibsave, *ਅਗੇਤਰ;
ਸਥਿਰ RC_DEPTREE *deptree;
ਸਥਿਰ RC_STRINGLIST *applet_list, *services, *tmplist;
ਸਥਿਰ RC_STRINGLIST *restart_services;
ਸਥਿਰ RC_STRINGLIST *need_services;
ਸਥਿਰ RC_STRINGLIST *use_services;
ਸਥਿਰ RC_STRINGLIST *want_services;
ਸਥਿਰ ਪੂਰਨ exclusive_fd = -1, master_tty = -1;
ਸਥਿਰ bool ਵਿੱਚ_background, deps, dry_ਲੜੀਵਾਰ;
ਸਥਿਰ ਅਸਥਿਰ bool sighup, skip_mark, timedout;
ਸਥਿਰ pid_t service_pid;
ਸਥਿਰ ਪੂਰਨ signal_pipe[2] = { -1, -1 };

ਸਥਿਰ RC_STRINGLIST *deptypes_b;	/* broken deps */
ਸਥਿਰ RC_STRINGLIST *deptypes_n;	/* needed deps */
ਸਥਿਰ RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ਸਥਿਰ RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ਸਥਿਰ RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ਸਥਿਰ RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ਸਥਿਰ RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ਸਥਿਰ ਖਾਲੀ
handle_signal(ਪੂਰਨ sig)
{
	ਪੂਰਨ serrno = errno, ਸਥਿਤੀ;
	pid_t pid;
	ਅਚਲ ਅੱਖਰ *signame = ਨਿਰਾ;
	ਬਣਤਰ winsize ws;

	ਚੋਣ (sig) {
	ਹਾਲਤ SIGHUP:
		sighup = true;
		ਰੋਕੋ;

	ਹਾਲਤ SIGUSR1:
		skip_mark = true;
		ਰੋਕੋ;

	ਹਾਲਤ SIGCHLD:
		ਜਦੋਂ ((pid = waitpid(-1, &ਸਥਿਤੀ, WNOHANG)) > 0) {
			ਜੇ (signal_pipe[1] > -1 && pid == service_pid) {
				ਜੇ (ਲਿਖ(signal_pipe[1], &ਸਥਿਤੀ, ਆਕਾਰ(ਸਥਿਤੀ)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		ਰੋਕੋ;

	ਹਾਲਤ SIGWINCH:
		ਜੇ (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		ਰੋਕੋ;

	ਹਾਲਤ SIGINT:
		ਜੇ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ਹਾਲਤ SIGTERM:
		ਜੇ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ਹਾਲਤ SIGQUIT:
		ਜੇ (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		ਜੇ (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
		/* NOTREACHED */

	ਹਾਲਤ SIGALRM:
		timedout = true;
		ਰੋਕੋ;

	ਮੂਲ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ਸਥਿਰ ਖਾਲੀ
unhotplug(ਖਾਲੀ)
{
	ਜੇ (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ਸਥਿਰ ਖਾਲੀ
ਸ਼ੁਰੂ_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	ਜੇ (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		ਵਾਪਸ;

	TAILQ_FOREACH(svc, list, entries) {
		ਜੇ (!(rc_service_state(svc->ਮੁੱਲ) & RC_SERVICE_STOPPED))
			ਜਾਰੀ;
		ਜੇ (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_ਸਮਾਂ_ਸਾਰਣੀ_ਸ਼ੁਰੂ(applet, svc->ਮੁੱਲ);
			ewarn("WARNING: %s will start when %s has started", svc->ਮੁੱਲ, applet);
		} ਨਹੀਂਤਾਂ {
			service_ਸ਼ੁਰੂ(svc->ਮੁੱਲ);
		}
	}
}

ਸਥਿਰ ਖਾਲੀ
restore_state(ਖਾਲੀ)
{
	RC_SERVICE state;

	ਜੇ (rc_ਵਿੱਚ_plugin || exclusive_fd == -1)
		ਵਾਪਸ;
	state = rc_service_state(applet);
	ਜੇ (state & RC_SERVICE_STOPPING) {
		ਜੇ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ਨਹੀਂਤਾਂ
			rc_service_mark(applet, RC_SERVICE_STARTED);

		ਜੇ (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_STARTING) {
		ਜੇ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ਨਹੀਂਤਾਂ
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		ਜੇ (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ਸਥਿਰ ਖਾਲੀ
cleanup(ਖਾਲੀ)
{
	restore_state();

	ਜੇ (!rc_ਵਿੱਚ_plugin) {
		ਜੇ (restart_services)
			ਸ਼ੁਰੂ_services(restart_services);
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
	ਮੁਕਤ(ibsave);
	ਮੁਕਤ(ਅਗੇਤਰ);
	ਮੁਕਤ(runlevel);
	ਮੁਕਤ(service);
}

ਸਥਿਰ ਖਾਲੀ
svc_getenv(ਅਚਲ ਅੱਖਰ *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	ਅੱਖਰ *var = ਨਿਰਾ;

	TAILQ_FOREACH(reexport, reexports, entries) {
		ਜੇ (rc_service_getenv(svc, reexport->ਮੁੱਲ, &var) == -1)
			ਜਾਰੀ;
		setenv(reexport->ਮੁੱਲ, var, true);
	}

	ਮੁਕਤ(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
ਸਥਿਰ ਪੂਰਨ
write_ਅਗੇਤਰ(ਅਚਲ ਅੱਖਰ *ਬਫਰ, size_t ਬਾਈਟਾਂ, bool *prefixed)
{
	size_t i, j;
	ਅਚਲ ਅੱਖਰ *ec = ecolor(ECOLOR_HILITE);
	ਅਚਲ ਅੱਖਰ *ec_ਸਾਧਾਰਨ = ecolor(ECOLOR_ਸਾਧਾਰਨ);
	ssize_t ret = 0;
	ਪੂਰਨ fd = fileno(stdout), ਤਾਲਾ_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	ਤਾਲਾ_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	ਜੇ (ਤਾਲਾ_fd != -1) {
		ਜਦੋਂ (flock(ਤਾਲਾ_fd, LOCK_EX) != 0) {
			ਜੇ (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				ਰੋਕੋ;
			}
		}
	}
	ਨਹੀਂਤਾਂ
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	ਲਈ (i = 0; i < ਬਾਈਟਾਂ; i++) {
		/* We don't prefix eend calls (cursor up) */
		ਜੇ (ਬਫਰ[i] == '\033' && !*prefixed) {
			ਲਈ (j = i + 1; j < ਬਾਈਟਾਂ; j++) {
				ਜੇ (ਬਫਰ[j] == 'A')
					*prefixed = true;
				ਜੇ (ਅੱਖਰ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਬਫਰ[j]))
					ਰੋਕੋ;
			}
		}

		ਜੇ (!*prefixed) {
			ret += ਲਿਖ(fd, ec, ਲੜੀ_ਲੰਬਾਈ(ec));
			ret += ਲਿਖ(fd, ਅਗੇਤਰ, ਲੜੀ_ਲੰਬਾਈ(ਅਗੇਤਰ));
			ret += ਲਿਖ(fd, ec_ਸਾਧਾਰਨ, ਲੜੀ_ਲੰਬਾਈ(ec_ਸਾਧਾਰਨ));
			ret += ਲਿਖ(fd, "|", 1);
			*prefixed = true;
		}

		ਜੇ (ਬਫਰ[i] == '\n')
			*prefixed = false;
		ret += ਲਿਖ(fd, ਬਫਰ + i, 1);
	}

	/* Release the lock */
	close(ਤਾਲਾ_fd);

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ
svc_exec(ਅਚਲ ਅੱਖਰ *command)
{
	ਪੂਰਨ ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ਬਣਤਰ termios tt;
	ਬਣਤਰ winsize ws;
	ਪੂਰਨ flags = 0;
	ਬਣਤਰ pollfd fd[2];
	bool prefixed = false;
	ਪੂਰਨ slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	ਅੱਖਰ *openrc_sh = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ਨਿਰਾ
	};

	/* Setup our signal pipe */
	ਜੇ (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		ਵਾਪਸ 1;
	}

	ਜੇ ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		ਵਾਪਸ 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	ਜੇ (ਅਗੇਤਰ && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		ਜੇ (openpty(&master_tty, &slave_tty, ਨਿਰਾ, &tt, &ws) == 0) {
			ਜੇ (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ਜੇ (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ਜੇ ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				ਵਾਪਸ 1;
			}
		}
	}

	ਜੇ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	ਜੇ ((errno = posix_spawn(&service_pid, argv[0], &tty, ਨਿਰਾ, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		ਵਾਪਸ 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	ਮੁਕਤ(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	ਲਈ (;;) {
		ਜੇ (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			ਜੇ (errno == EINTR)
				ਜਾਰੀ;
			eerror("%s: poll: %s", applet, strerror(errno));
			ਰੋਕੋ;
		}

		ਜੇ (fd[1].revents & (POLLIN | POLLHUP)) {
			ਅੱਖਰ ਬਫਰ[BUFSIZ];
			write_ਅਗੇਤਰ(ਬਫਰ, ਪਠ(master_tty, ਬਫਰ, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		ਜੇ (fd[0].revents & (POLLIN | POLLHUP)) {
			ਜੇ (errno = 0, ਪਠ(signal_pipe[0], &ret, ਆਕਾਰ(ret)) < (ssize_t)ਆਕਾਰ(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			ਨਹੀਂਤਾਂ ਜੇ ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			ਰੋਕੋ;
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

	sigprocmask (SIG_SETMASK, &oldmask, ਨਿਰਾ);

	ਜੇ (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	ਵਾਪਸ ret;
}

ਸਥਿਰ bool
svc_wait(ਅਚਲ ਅੱਖਰ *svc)
{
	ਪੂਰਨ fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = basename_c(svc);
	ਪੂਰਨ timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	ਜੇ (rc_stringlist_ਲੱਭੋ(keywords, "-timeout") || rc_stringlist_ਲੱਭੋ(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	ਜੇ ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), ਅਧਾਰ, O_RDONLY | O_NONBLOCK)) == -1)
		ਵਾਪਸ errno == ENOENT;

	timedout = false;
	ਲਈ (;;) {
		ਜੇ (!forever)
			alarm(WARN_TIMEOUT);

		ਜੇ (flock(fd, LOCK_SH) == 0) {
			retval = true;
			ਰੋਕੋ;
		}

		ਜੇ (errno != EINTR)
			ਰੋਕੋ;

		ਜੇ (!forever && timedout) {
			timedout = false;
			ਜੇ ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, ਅਧਾਰ, timeout);
			ਨਹੀਂਤਾਂ
				ਰੋਕੋ;
		}
	}

	alarm(0);
	close(fd);
	ਵਾਪਸ retval;
}

ਸਥਿਰ ਖਾਲੀ
get_started_services(ਖਾਲੀ)
{
	RC_STRINGLIST *tmp = rc_services_ਵਿੱਚ_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	ਮੁਕਤ(tmp);
}

ਸਥਿਰ ਖਾਲੀ
ਤਿਆਰੀ_deptypes(ਖਾਲੀ)
{
	deptypes_b = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_b, "broken");

	deptypes_n = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_n, "ineed");

	deptypes_nw = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_nw, "ineed");
	rc_stringlist_ਜੋੜੋ(deptypes_nw, "iwant");

	deptypes_nwu = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_nwu, "ineed");
	rc_stringlist_ਜੋੜੋ(deptypes_nwu, "iwant");
	rc_stringlist_ਜੋੜੋ(deptypes_nwu, "iuse");

	deptypes_nwua = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_nwua, "ineed");
	rc_stringlist_ਜੋੜੋ(deptypes_nwua, "iwant");
	rc_stringlist_ਜੋੜੋ(deptypes_nwua, "iuse");
	rc_stringlist_ਜੋੜੋ(deptypes_nwua, "iafter");

	deptypes_m = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_m, "needsme");

	deptypes_mwua = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(deptypes_mwua, "needsme");
	rc_stringlist_ਜੋੜੋ(deptypes_mwua, "wantsme");
	rc_stringlist_ਜੋੜੋ(deptypes_mwua, "usesme");
	rc_stringlist_ਜੋੜੋ(deptypes_mwua, "beforeme");
}

ਸਥਿਰ ਖਾਲੀ
svc_ਸ਼ੁਰੂ_check(ਖਾਲੀ)
{
	ਪੂਰਨ ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	ਜੇ (ਵਿੱਚ_background) {
		ਜੇ (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;

		ਜੇ (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		ਜੇ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	ਜੇ (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	} ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_INACTIVE && !ਵਿੱਚ_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	ਜੇ (exclusive_fd == -1)
		exclusive_fd = svc_ਤਾਲਾ(applet, !deps);
	ਜੇ (exclusive_fd == -1) {
		ਜੇ (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
		}

		ਜੇ (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		ਨਹੀਂਤਾਂ
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਵਿੱਚ, applet);

	ਵਾਪਸ;

ਬਾਹਰ_ਨਿਕਲੋ:
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, applet);
	ਬਾਹਰ_ਨਿਕਲੋ(ret);
}

ਸਥਿਰ ਖਾਲੀ
svc_ਸ਼ੁਰੂ_deps(ਖਾਲੀ)
{
	bool ਪਹਿਲਾ;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	ਪੂਰਨ depoptions = RC_DEP_TRACE;
	size_t len, svc_ਗਿਣਤੀ;
	ਅੱਖਰ *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	ਜੇ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ਜੇ (!deptree && ((deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ)) == ਨਿਰਾ)) {
		eerror("failed to load deptree");
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	}

	ਜੇ (!deptypes_b)
		ਤਿਆਰੀ_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	ਜੇ (TAILQ_ਪਹਿਲਾ(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		ਪਹਿਲਾ = true;
		TAILQ_FOREACH(svc, services, entries) {
			ਜੇ (ਪਹਿਲਾ)
				ਪਹਿਲਾ = false;
			ਨਹੀਂਤਾਂ
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->ਮੁੱਲ);
		}
		fprintf(stderr, "\n");
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	}
	rc_stringlist_free(services);
	services = ਨਿਰਾ;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	ਜੇ (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->ਮੁੱਲ);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			ਜੇ (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				ਜਾਰੀ;
			ਜੇ (state & RC_SERVICE_STOPPED) {
				ਜੇ (dry_ਲੜੀਵਾਰ) {
					ਲਿਖੋ(" %s", svc->ਮੁੱਲ);
					ਜਾਰੀ;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_ਸ਼ੁਰੂ(svc->ਮੁੱਲ);
				ਜੇ (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	ਜੇ (dry_ਲੜੀਵਾਰ)
		ਵਾਪਸ;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->ਮੁੱਲ);
		ਜੇ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ਮੁੱਲ);
			ਜਾਰੀ;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		ਜੇ (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			ਜੇ (!rc_stringlist_ਲੱਭੋ(need_services, svc->ਮੁੱਲ) &&
					!rc_stringlist_ਲੱਭੋ(want_services, svc->ਮੁੱਲ) &&
					!rc_stringlist_ਲੱਭੋ(use_services, svc->ਮੁੱਲ))
				ਜਾਰੀ;
		}

		ਜੇ (!svc_wait(svc->ਮੁੱਲ))
			eerror("%s: timed out waiting for %s", applet, svc->ਮੁੱਲ);
		state = rc_service_state(svc->ਮੁੱਲ);
		ਜੇ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->ਮੁੱਲ);
			ਜਾਰੀ;
		}
		ਜੇ (rc_stringlist_ਲੱਭੋ(need_services, svc->ਮੁੱਲ)) {
			ਜੇ (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_ਜੋੜੋ(tmplist, svc->ਮੁੱਲ);
			} ਨਹੀਂਤਾਂ ਜੇ (!TAILQ_ਪਹਿਲਾ(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->ਮੁੱਲ);
				ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			}
		}
	}

	ਜੇ (TAILQ_ਪਹਿਲਾ(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ਨਿਰਾ;
		mem = xopen_memstream(&tmp, &len);
		svc_ਗਿਣਤੀ = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_ਸਮਾਂ_ਸਾਰਣੀ_ਸ਼ੁਰੂ(svc->ਮੁੱਲ, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->ਮੁੱਲ);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_ਸਮਾਂ_ਸਾਰਣੀ_ਸ਼ੁਰੂ(svc2->ਮੁੱਲ, applet);
			rc_stringlist_free(use_services);
			use_services = ਨਿਰਾ;
			ਜੇ (svc_ਗਿਣਤੀ > 0)
				fputs(", ", mem);
			fputs(svc->ਮੁੱਲ, mem);
			++svc_ਗਿਣਤੀ;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ਨਿਰਾ;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_ਗਿਣਤੀ > 1 ? "have" : "has");
		ਮੁਕਤ(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ਨਿਰਾ;
	rc_stringlist_free(services);
	services = ਨਿਰਾ;

	ਵਾਪਸ;

ਬਾਹਰ_ਨਿਕਲੋ:
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, applet);
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}


ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_reexports(ਖਾਲੀ)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t ਮਾਪ;
	ਅੱਖਰ *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &ਮਾਪ);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->ਮੁੱਲ);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ਸਥਿਰ ਖਾਲੀ svc_ਸ਼ੁਰੂ_real(ਖਾਲੀ)
{
	bool started;
	RC_STRING *svc, *svc2;

	ਸੈੱਟ_reexports();

	ਜੇ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	ਜੇ (ibsave)
		unsetenv("IN_BACKGROUND");

	ਜੇ (!started) {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਪੂਰਾ, applet);
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, applet);
	}

	ਜੇ (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	ਨਹੀਂਤਾਂ ਜੇ (!started)
		eerrorx("ERROR: %s failed to start", applet);

	ਜੇ (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਪੂਰਾ, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    ਜੇ (rc_service_state(svc->ਮੁੱਲ) & RC_SERVICE_STOPPED)
		    service_ਸ਼ੁਰੂ(svc->ਮੁੱਲ);
	rc_stringlist_free(services);
	services = ਨਿਰਾ;

	/* Do the same for any services we provide */
	ਜੇ (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->ਮੁੱਲ);
			TAILQ_FOREACH(svc2, services, entries)
				ਜੇ (rc_service_state(svc2->ਮੁੱਲ) & RC_SERVICE_STOPPED)
					service_ਸ਼ੁਰੂ(svc2->ਮੁੱਲ);
			rc_stringlist_free(services);
			services = ਨਿਰਾ;
		}
		rc_stringlist_free(tmplist);
		tmplist = ਨਿਰਾ;
	}

	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, applet);
}

ਸਥਿਰ ਪੂਰਨ
svc_ਸ਼ੁਰੂ(ਖਾਲੀ)
{
	ਜੇ (dry_ਲੜੀਵਾਰ)
		einfon("start:");
	ਨਹੀਂਤਾਂ
		svc_ਸ਼ੁਰੂ_check();

	ਜੇ (deps)
		svc_ਸ਼ੁਰੂ_deps();

	ਜੇ (dry_ਲੜੀਵਾਰ)
		ਲਿਖੋ(" %s\n", applet);
	ਨਹੀਂਤਾਂ
		svc_ਸ਼ੁਰੂ_real();

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	ਜੇ (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;

	ਜੇ (ਵਿੱਚ_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;

	ਜੇ (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		ਵਾਪਸ 1;
	}

	ਜੇ (exclusive_fd == -1)
		exclusive_fd = svc_ਤਾਲਾ(applet, !deps);
	ਜੇ (exclusive_fd == -1) {
		ਜੇ (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		ਨਹੀਂਤਾਂ ਜੇ (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		ਨਹੀਂਤਾਂ
			eerror("ERROR: %s stopped by something else", applet);
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_ਵਿੱਚ, applet);

	ਜੇ (!rc_runlevel_stopping()) {
		ਜੇ (rc_service_ਵਿੱਚ_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		ਨਹੀਂਤਾਂ ਜੇ (rc_service_ਵਿੱਚ_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	ਵਾਪਸ 0;

ਬਾਹਰ_ਨਿਕਲੋ:
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_ਬਾਹਰਲਾ, applet);
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}

ਸਥਿਰ ਖਾਲੀ
svc_stop_deps(RC_SERVICE state)
{
	ਪੂਰਨ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	ਜੇ (state & RC_SERVICE_WASINACTIVE)
		ਵਾਪਸ;

	errno = 0;
	ਜੇ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ਜੇ (!deptree && ((deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ)) == ਨਿਰਾ)) {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_ਬਾਹਰਲਾ, applet);
		eerrorx("failed to load deptree");
	}

	ਜੇ (!deptypes_m)
		ਤਿਆਰੀ_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_ਉਲਟ(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->ਮੁੱਲ);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		ਜੇ (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			ਜਾਰੀ;
		ਜੇ (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			ਜਾਰੀ;
		ਜੇ (dry_ਲੜੀਵਾਰ) {
			ਲਿਖੋ(" %s", svc->ਮੁੱਲ);
			ਜਾਰੀ;
		}
		svc_wait(svc->ਮੁੱਲ);
		ਜੇ (rc_service_state(svc->ਮੁੱਲ) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->ਮੁੱਲ);
			ਜੇ (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_ਜੋੜੋ(tmplist, svc->ਮੁੱਲ);
		}
	}
	rc_stringlist_free(services);
	services = ਨਿਰਾ;
	ਜੇ (dry_ਲੜੀਵਾਰ)
		ਵਾਪਸ;

	TAILQ_FOREACH(svc, tmplist, entries) {
		ਜੇ (rc_service_state(svc->ਮੁੱਲ) & RC_SERVICE_STOPPED) {
			ਜਾਰੀ;
		}
		svc_wait(svc->ਮੁੱਲ);
		ਜੇ (rc_service_state(svc->ਮੁੱਲ) & RC_SERVICE_STOPPED)
			ਜਾਰੀ;
		ਜੇ (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			ਜੇ (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				ਜਾਰੀ;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_ਬਾਹਰਲਾ, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->ਮੁੱਲ);
	}
	rc_stringlist_free(tmplist);
	tmplist = ਨਿਰਾ;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		ਜੇ (rc_service_state(svc->ਮੁੱਲ) & RC_SERVICE_STOPPED)
			ਜਾਰੀ;
		svc_wait(svc->ਮੁੱਲ);
	}
	rc_stringlist_free(services);
	services = ਨਿਰਾ;
}

ਸਥਿਰ ਖਾਲੀ
ਲੱਦੋ_dep_env(ਖਾਲੀ)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->ਮੁੱਲ);

	rc_stringlist_free(depsvcs);
}

ਸਥਿਰ ਖਾਲੀ
svc_stop_real(ਖਾਲੀ)
{
	bool stopped;

	ਲੱਦੋ_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	ਜੇ (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	ਜੇ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	ਜੇ (ibsave)
		unsetenv("IN_BACKGROUND");

	ਜੇ (stopped && !skip_mark)
		rc_service_mark(applet, ਵਿੱਚ_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_ਪੂਰਾ, applet);
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_SERVICE_STOP_ਬਾਹਰਲਾ, applet);

	ਜੇ (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ਸਥਿਰ ਪੂਰਨ
svc_stop(ਖਾਲੀ)
{
	RC_SERVICE state = 0;

	ਜੇ (dry_ਲੜੀਵਾਰ)
		einfon("stop:");
	ਨਹੀਂਤਾਂ
		ਜੇ (svc_stop_check(&state) == 1)
			ਵਾਪਸ 1; /* Service has been stopped already */

	ਜੇ (deps)
		svc_stop_deps(state);

	ਜੇ (dry_ਲੜੀਵਾਰ)
		ਲਿਖੋ(" %s\n", applet);
	ਨਹੀਂਤਾਂ
		svc_stop_real();

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
svc_restart(ਖਾਲੀ)
{
	ਜੇ (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		ਜੇ (dry_ਲੜੀਵਾਰ)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_ਸ਼ੁਰੂ();
	ਸ਼ੁਰੂ_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ਨਿਰਾ;

	ਵਾਪਸ 0;
}

ਸਥਿਰ bool
service_plugable(ਖਾਲੀ)
{
	ਅੱਖਰ *list, *p, *token;
	bool allow = true, truefalse;
	ਅੱਖਰ *ਮੇਲ = rc_conf_ਮੁੱਲ("rc_hotplug");

	ਜੇ (!ਮੇਲ)
		ਮੇਲ = rc_conf_ਮੁੱਲ("rc_plug_services");
	ਜੇ (!ਮੇਲ)
		ਵਾਪਸ false;

	list = xstrdup(ਮੇਲ);
	p = list;
	ਜਦੋਂ ((token = strsep(&p, " "))) {
		ਜੇ (token[0] == '!') {
			truefalse = false;
			token++;
		} ਨਹੀਂਤਾਂ {
			truefalse = true;
		}

		ਜੇ (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			ਰੋਕੋ;
		}
	}
	ਮੁਕਤ(list);
	ਵਾਪਸ allow;
}

ਸਥਿਰ ਪੂਰਨ
simple_command(ਖਾਲੀ)
{
	ਅੱਖਰ *save = ਅਗੇਤਰ;
	ਪੂਰਨ retval;

	eprefix(ਨਿਰਾ);
	ਅਗੇਤਰ = ਨਿਰਾ;

	retval = svc_exec(optarg);

	eprefix(save);
	ਅਗੇਤਰ = save;

	ਵਾਪਸ retval;
}

ਸਥਿਰ ਪੂਰਨ
resolve_deptype(ਖਾਲੀ)
{
	ਪੂਰਨ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	ਜੇ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ਜੇ (!deptree && ((deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ)) == ਨਿਰਾ))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ਨਿਰਾ;
	TAILQ_FOREACH(svc, services, entries)
		ਲਿਖੋ("%s ", svc->ਮੁੱਲ);
	ਲਿਖੋ ("\n");

	rc_stringlist_free(services);
	services = ਨਿਰਾ;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
cmd_condrestart(ਖਾਲੀ)
{
	ਜੇ (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
cmd_stop(ਖਾਲੀ)
{
	RC_STRING *svc;

	ਜੇ (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	ਜੇ (deps && ਵਿੱਚ_background)
		get_started_services();

	ਜੇ (svc_stop() == 1)
		ਵਾਪਸ 0; /* Service has been stopped already */

	ਜੇ (deps) {
		ਜੇ (!ਵਿੱਚ_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		ਜੇ (ਵਿੱਚ_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				ਜੇ (rc_service_state(svc->ਮੁੱਲ) & RC_SERVICE_STOPPED)
					rc_service_ਸਮਾਂ_ਸਾਰਣੀ_ਸ਼ੁਰੂ(applet, svc->ਮੁੱਲ);
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ
cmd_zap(ਖਾਲੀ)
{
	einfo("Manually resetting %s to stopped state", applet);
	ਜੇ (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	ਵਾਪਸ 0;
}

ਬਣਤਰ {
	ਅਚਲ ਅੱਖਰ *name;
	ਪੂਰਨ (*handler)(ਖਾਲੀ);
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
	{ "start",              svc_ਸ਼ੁਰੂ,       true  },
	{ "stop",               cmd_stop,        true  },
	{ "restart",            svc_restart,     true  },
	{ "condrestart",        cmd_condrestart, true  },
	{ "conditionalrestart", cmd_condrestart, true  },
	{ "zap",                cmd_zap,         true  },
};

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ retval, opt;
	ਅਚਲ ਅੱਖਰ *workingdir = "/";
	ਅੱਖਰ *pidstr = ਨਿਰਾ;
	size_t l = 0, ll;
	ਬਣਤਰ stat stbuf;

	/* Show help if insufficient args */
	ਜੇ (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	ਜੇ (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}

	ਜੇ (!(service = (realpath(argv[1], ਨਿਰਾ))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	ਜੇ (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_ਤਿਆਰੀ(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring, longopts, ਨਿਰਾ)) != -1)
		ਚੋਣ (opt) {
		ਹਾਲਤ 'd':
			setenv("RC_DEBUG", "YES", 1);
			ਰੋਕੋ;
		ਹਾਲਤ 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			ਜੇ (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			ਰੋਕੋ;
		ਹਾਲਤ 'S':
			ਜੇ (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			ਰੋਕੋ;
		ਹਾਲਤ 'D':
			deps = false;
			ਰੋਕੋ;
		ਹਾਲਤ 'Z':
			dry_ਲੜੀਵਾਰ = true;
			ਰੋਕੋ;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ਜੇ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ਜੇ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	ਜੇ ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ਨਿਰਾ) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (ਪੂਰਨ) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	ਮੁਕਤ(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	ਜੇ (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
		TAILQ_FOREACH(svc, services, entries) {
			ll = ਲੜੀ_ਲੰਬਾਈ(svc->ਮੁੱਲ);
			ਜੇ (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ਨਿਰਾ;
		ll = ਲੜੀ_ਲੰਬਾਈ(applet);
		ਜੇ (ll > l)
			l = ll;

		/* Make our prefix string */
		ਅਗੇਤਰ = xmalloc(ਆਕਾਰ(ਅੱਖਰ) * l + 1);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅਗੇਤਰ, applet, ll);
		memset(ਅਗੇਤਰ + ll, ' ', l - ll);
		memset(ਅਗੇਤਰ + l, 0, 1);
		eprefix(ਅਗੇਤਰ);
	}

	ਜੇ (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	ਜੇ (deps && getenv("RC_PID") == ਨਿਰਾ && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	ਜੇ (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		ਵਿੱਚ_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	ਜੇ (rc_yesno(getenv("IN_DRYRUN")))
		dry_ਲੜੀਵਾਰ = true;
	ਜੇ (rc_yesno(getenv("IN_HOTPLUG"))) {
		ਜੇ (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		ਵਿੱਚ_background = true;
	}

	/* Setup a signal handler */
	signal_ਤਿਆਰੀ(SIGHUP, handle_signal);
	signal_ਤਿਆਰੀ(SIGALRM, handle_signal);
	signal_ਤਿਆਰੀ(SIGUSR1, handle_signal);
	signal_ਤਿਆਰੀ(SIGINT, handle_signal);
	signal_ਤਿਆਰੀ(SIGQUIT, handle_signal);
	signal_ਤਿਆਰੀ(SIGTERM, handle_signal);
	signal_ਤਿਆਰੀ(SIGCHLD, handle_signal);

	/* Load our plugins */
	rc_plugin_ਲੱਦੋ();

	applet_list = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(applet_list, applet);

	ਜੇ (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

ਅਗਲਾ_cmd:
	ਜਦੋਂ (optind < argc) {
		ਪੂਰਨ cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		ਜੇ (sighup)
			ਬਾਹਰ_ਨਿਕਲੋ (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(commands); i++) {
			ਜੇ (strcmp(optarg, commands[i].name) != 0)
				ਜਾਰੀ;

			ਜੇ ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			ਜੇ (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ਨਿਰਾ;
			}

			ਜਾਓ ਅਗਲਾ_cmd;
		}

		ਜੇ ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ਨਿਰਾ;
	}

	ਵਾਪਸ retval;
}
