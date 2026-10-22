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

#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <fnmatch.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <libgen.h>
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <spawn.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/file.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <termios.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <inttypes.h>

#എങ്കിൽ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#അല്ലെങ്കിലെങ്കിൽ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#അല്ലെങ്കിൽ
#  include <libutil.h>
#endif

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "timeutils.h"
#അന്തര്ഭാവ "plugin.h"
#അന്തര്ഭാവ "selinux.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

ബാഹ്യ അക്ഷരം **environ;

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = "stop | start | restart | status | describe | zap";
സ്ഥിരം അക്ഷരം getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "debug",      0, ശൂന്യ, 'd'},
	{ "dry-run",    0, ശൂന്യ, 'Z'},
	{ "ifstarted",  0, ശൂന്യ, 's'},
	{ "ifstopped",  0, ശൂന്യ, 'S'},
	{ "nodeps",     0, ശൂന്യ, 'D'},
	{ "lockfd",     1, ശൂന്യ, 'l'},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം *സ്ഥിരം longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

സ്ഥിതം അക്ഷരം *service, *runlevel, *ibsave, *prefix;
സ്ഥിതം RC_DEPTREE *deptree;
സ്ഥിതം RC_STRINGLIST *applet_list, *services, *tmplist;
സ്ഥിതം RC_STRINGLIST *restart_services;
സ്ഥിതം RC_STRINGLIST *need_services;
സ്ഥിതം RC_STRINGLIST *use_services;
സ്ഥിതം RC_STRINGLIST *want_services;
സ്ഥിതം പൂർണ്ണസംഖ്യ exclusive_fd = -1, master_tty = -1;
സ്ഥിതം bool in_background, deps, dry_run;
സ്ഥിതം അസ്ഥിര bool sighup, skip_mark, timedout;
സ്ഥിതം pid_t service_pid;
സ്ഥിതം പൂർണ്ണസംഖ്യ signal_pipe[2] = { -1, -1 };

സ്ഥിതം RC_STRINGLIST *deptypes_b;	/* broken deps */
സ്ഥിതം RC_STRINGLIST *deptypes_n;	/* needed deps */
സ്ഥിതം RC_STRINGLIST *deptypes_nw;	/* need+want deps */
സ്ഥിതം RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
സ്ഥിതം RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
സ്ഥിതം RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
സ്ഥിതം RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

സ്ഥിതം ശൂന്യം
handle_signal(പൂർണ്ണസംഖ്യ sig)
{
	പൂർണ്ണസംഖ്യ serrno = errno, status;
	pid_t pid;
	സ്ഥിരം അക്ഷരം *signame = ശൂന്യ;
	ഘടന winsize ws;

	തിരഞ്ഞെടുപ്പ് (sig) {
	അവസ്ഥ SIGHUP:
		sighup = true;
		നിർത്തുക;

	അവസ്ഥ SIGUSR1:
		skip_mark = true;
		നിർത്തുക;

	അവസ്ഥ SIGCHLD:
		ആകുമ്പോൾ ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			എങ്കിൽ (signal_pipe[1] > -1 && pid == service_pid) {
				എങ്കിൽ (ലിഖ(signal_pipe[1], &status, വലുപ്പം(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		നിർത്തുക;

	അവസ്ഥ SIGWINCH:
		എങ്കിൽ (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		നിർത്തുക;

	അവസ്ഥ SIGINT:
		എങ്കിൽ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	അവസ്ഥ SIGTERM:
		എങ്കിൽ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	അവസ്ഥ SIGQUIT:
		എങ്കിൽ (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		എങ്കിൽ (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	അവസ്ഥ SIGALRM:
		timedout = true;
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

സ്ഥിതം ശൂന്യം
unhotplug(ശൂന്യം)
{
	എങ്കിൽ (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

സ്ഥിതം ശൂന്യം
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	എങ്കിൽ (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		തിരികെ;

	TAILQ_FOREACH(svc, list, entries) {
		എങ്കിൽ (!(rc_service_state(svc->മൂല്യം) & RC_SERVICE_STOPPED))
			തുടരുക;
		എങ്കിൽ (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->മൂല്യം);
			ewarn("WARNING: %s will start when %s has started", svc->മൂല്യം, applet);
		} അല്ലെങ്കിൽ {
			service_start(svc->മൂല്യം);
		}
	}
}

സ്ഥിതം ശൂന്യം
restore_state(ശൂന്യം)
{
	RC_SERVICE state;

	എങ്കിൽ (rc_in_plugin || exclusive_fd == -1)
		തിരികെ;
	state = rc_service_state(applet);
	എങ്കിൽ (state & RC_SERVICE_STOPPING) {
		എങ്കിൽ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		അല്ലെങ്കിൽ
			rc_service_mark(applet, RC_SERVICE_STARTED);

		എങ്കിൽ (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_STARTING) {
		എങ്കിൽ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		അല്ലെങ്കിൽ
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		എങ്കിൽ (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

സ്ഥിതം ശൂന്യം
cleanup(ശൂന്യം)
{
	restore_state();

	എങ്കിൽ (!rc_in_plugin) {
		എങ്കിൽ (restart_services)
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
	സ്വതന്ത്രം(ibsave);
	സ്വതന്ത്രം(prefix);
	സ്വതന്ത്രം(runlevel);
	സ്വതന്ത്രം(service);
}

സ്ഥിതം ശൂന്യം
svc_getenv(സ്ഥിരം അക്ഷരം *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	അക്ഷരം *var = ശൂന്യ;

	TAILQ_FOREACH(reexport, reexports, entries) {
		എങ്കിൽ (rc_service_getenv(svc, reexport->മൂല്യം, &var) == -1)
			തുടരുക;
		setenv(reexport->മൂല്യം, var, true);
	}

	സ്വതന്ത്രം(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ
write_prefix(സ്ഥിരം അക്ഷരം *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	സ്ഥിരം അക്ഷരം *ec = ecolor(ECOLOR_HILITE);
	സ്ഥിരം അക്ഷരം *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	പൂർണ്ണസംഖ്യ fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	എങ്കിൽ (lock_fd != -1) {
		ആകുമ്പോൾ (flock(lock_fd, LOCK_EX) != 0) {
			എങ്കിൽ (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				നിർത്തുക;
			}
		}
	}
	അല്ലെങ്കിൽ
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	വേണ്ടി (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		എങ്കിൽ (buffer[i] == '\033' && !*prefixed) {
			വേണ്ടി (j = i + 1; j < bytes; j++) {
				എങ്കിൽ (buffer[j] == 'A')
					*prefixed = true;
				എങ്കിൽ (isalpha((unsigned പൂർണ്ണസംഖ്യ)buffer[j]))
					നിർത്തുക;
			}
		}

		എങ്കിൽ (!*prefixed) {
			ret += ലിഖ(fd, ec, strlen(ec));
			ret += ലിഖ(fd, prefix, strlen(prefix));
			ret += ലിഖ(fd, ec_normal, strlen(ec_normal));
			ret += ലിഖ(fd, "|", 1);
			*prefixed = true;
		}

		എങ്കിൽ (buffer[i] == '\n')
			*prefixed = false;
		ret += ലിഖ(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
svc_exec(സ്ഥിരം അക്ഷരം *command)
{
	പൂർണ്ണസംഖ്യ ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ഘടന termios tt;
	ഘടന winsize ws;
	പൂർണ്ണസംഖ്യ flags = 0;
	ഘടന pollfd fd[2];
	bool prefixed = false;
	പൂർണ്ണസംഖ്യ slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	അക്ഷരം *openrc_sh = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ശൂന്യ
	};

	/* Setup our signal pipe */
	എങ്കിൽ (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		തിരികെ 1;
	}

	എങ്കിൽ ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		തിരികെ 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	എങ്കിൽ (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		എങ്കിൽ (openpty(&master_tty, &slave_tty, ശൂന്യ, &tt, &ws) == 0) {
			എങ്കിൽ (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			എങ്കിൽ (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			എങ്കിൽ ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				തിരികെ 1;
			}
		}
	}

	എങ്കിൽ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	എങ്കിൽ ((errno = posix_spawn(&service_pid, argv[0], &tty, ശൂന്യ, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		തിരികെ 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	സ്വതന്ത്രം(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	വേണ്ടി (;;) {
		എങ്കിൽ (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			എങ്കിൽ (errno == EINTR)
				തുടരുക;
			eerror("%s: poll: %s", applet, strerror(errno));
			നിർത്തുക;
		}

		എങ്കിൽ (fd[1].revents & (POLLIN | POLLHUP)) {
			അക്ഷരം buffer[BUFSIZ];
			write_prefix(buffer, പഠ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		എങ്കിൽ (fd[0].revents & (POLLIN | POLLHUP)) {
			എങ്കിൽ (errno = 0, പഠ(signal_pipe[0], &ret, വലുപ്പം(ret)) < (ssize_t)വലുപ്പം(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			അല്ലെങ്കിൽ എങ്കിൽ ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			നിർത്തുക;
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

	sigprocmask (SIG_SETMASK, &oldmask, ശൂന്യ);

	എങ്കിൽ (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	തിരികെ ret;
}

സ്ഥിതം bool
svc_wait(സ്ഥിരം അക്ഷരം *svc)
{
	പൂർണ്ണസംഖ്യ fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	സ്ഥിരം അക്ഷരം *base = basename_c(svc);
	പൂർണ്ണസംഖ്യ timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	എങ്കിൽ (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	എങ്കിൽ ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		തിരികെ errno == ENOENT;

	timedout = false;
	വേണ്ടി (;;) {
		എങ്കിൽ (!forever)
			alarm(WARN_TIMEOUT);

		എങ്കിൽ (flock(fd, LOCK_SH) == 0) {
			retval = true;
			നിർത്തുക;
		}

		എങ്കിൽ (errno != EINTR)
			നിർത്തുക;

		എങ്കിൽ (!forever && timedout) {
			timedout = false;
			എങ്കിൽ ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			അല്ലെങ്കിൽ
				നിർത്തുക;
		}
	}

	alarm(0);
	close(fd);
	തിരികെ retval;
}

സ്ഥിതം ശൂന്യം
get_started_services(ശൂന്യം)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	സ്വതന്ത്രം(tmp);
}

സ്ഥിതം ശൂന്യം
setup_deptypes(ശൂന്യം)
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

സ്ഥിതം ശൂന്യം
svc_start_check(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	എങ്കിൽ (in_background) {
		എങ്കിൽ (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			പോകുക exit;

		എങ്കിൽ (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	എങ്കിൽ (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		പോകുക exit;
	} അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	എങ്കിൽ (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	എങ്കിൽ (exclusive_fd == -1) {
		എങ്കിൽ (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			പോകുക exit;
		}

		എങ്കിൽ (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		അല്ലെങ്കിൽ
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	തിരികെ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

സ്ഥിതം ശൂന്യം
svc_start_deps(ശൂന്യം)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	പൂർണ്ണസംഖ്യ depoptions = RC_DEP_TRACE;
	size_t len, svc_എണ്ണം;
	അക്ഷരം *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	എങ്കിൽ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	എങ്കിൽ (!deptree && ((deptree = _rc_deptree_load(0, ശൂന്യ)) == ശൂന്യ)) {
		eerror("failed to load deptree");
		പോകുക exit;
	}

	എങ്കിൽ (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	എങ്കിൽ (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			എങ്കിൽ (first)
				first = false;
			അല്ലെങ്കിൽ
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->മൂല്യം);
		}
		fprintf(stderr, "\n");
		പോകുക exit;
	}
	rc_stringlist_free(services);
	services = ശൂന്യ;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	എങ്കിൽ (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->മൂല്യം);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			എങ്കിൽ (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				തുടരുക;
			എങ്കിൽ (state & RC_SERVICE_STOPPED) {
				എങ്കിൽ (dry_run) {
					printf(" %s", svc->മൂല്യം);
					തുടരുക;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->മൂല്യം);
				എങ്കിൽ (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	എങ്കിൽ (dry_run)
		തിരികെ;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->മൂല്യം);
		എങ്കിൽ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->മൂല്യം);
			തുടരുക;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		എങ്കിൽ (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			എങ്കിൽ (!rc_stringlist_find(need_services, svc->മൂല്യം) &&
					!rc_stringlist_find(want_services, svc->മൂല്യം) &&
					!rc_stringlist_find(use_services, svc->മൂല്യം))
				തുടരുക;
		}

		എങ്കിൽ (!svc_wait(svc->മൂല്യം))
			eerror("%s: timed out waiting for %s", applet, svc->മൂല്യം);
		state = rc_service_state(svc->മൂല്യം);
		എങ്കിൽ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->മൂല്യം);
			തുടരുക;
		}
		എങ്കിൽ (rc_stringlist_find(need_services, svc->മൂല്യം)) {
			എങ്കിൽ (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->മൂല്യം);
			} അല്ലെങ്കിൽ എങ്കിൽ (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->മൂല്യം);
				പോകുക exit;
			}
		}
	}

	എങ്കിൽ (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ശൂന്യ;
		mem = xopen_memstream(&tmp, &len);
		svc_എണ്ണം = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->മൂല്യം, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->മൂല്യം);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->മൂല്യം, applet);
			rc_stringlist_free(use_services);
			use_services = ശൂന്യ;
			എങ്കിൽ (svc_എണ്ണം > 0)
				fputs(", ", mem);
			fputs(svc->മൂല്യം, mem);
			++svc_എണ്ണം;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ശൂന്യ;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_എണ്ണം > 1 ? "have" : "has");
		സ്വതന്ത്രം(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ശൂന്യ;
	rc_stringlist_free(services);
	services = ശൂന്യ;

	തിരികെ;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


സ്ഥിതം ശൂന്യം set_reexports(ശൂന്യം)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	അക്ഷരം *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->മൂല്യം);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

സ്ഥിതം ശൂന്യം svc_start_real(ശൂന്യം)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	എങ്കിൽ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	എങ്കിൽ (ibsave)
		unsetenv("IN_BACKGROUND");

	എങ്കിൽ (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	എങ്കിൽ (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	അല്ലെങ്കിൽ എങ്കിൽ (!started)
		eerrorx("ERROR: %s failed to start", applet);

	എങ്കിൽ (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    എങ്കിൽ (rc_service_state(svc->മൂല്യം) & RC_SERVICE_STOPPED)
		    service_start(svc->മൂല്യം);
	rc_stringlist_free(services);
	services = ശൂന്യ;

	/* Do the same for any services we provide */
	എങ്കിൽ (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->മൂല്യം);
			TAILQ_FOREACH(svc2, services, entries)
				എങ്കിൽ (rc_service_state(svc2->മൂല്യം) & RC_SERVICE_STOPPED)
					service_start(svc2->മൂല്യം);
			rc_stringlist_free(services);
			services = ശൂന്യ;
		}
		rc_stringlist_free(tmplist);
		tmplist = ശൂന്യ;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
svc_start(ശൂന്യം)
{
	എങ്കിൽ (dry_run)
		einfon("start:");
	അല്ലെങ്കിൽ
		svc_start_check();

	എങ്കിൽ (deps)
		svc_start_deps();

	എങ്കിൽ (dry_run)
		printf(" %s\n", applet);
	അല്ലെങ്കിൽ
		svc_start_real();

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	എങ്കിൽ (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		പോകുക exit;

	എങ്കിൽ (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		പോകുക exit;

	എങ്കിൽ (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		തിരികെ 1;
	}

	എങ്കിൽ (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	എങ്കിൽ (exclusive_fd == -1) {
		എങ്കിൽ (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		അല്ലെങ്കിൽ എങ്കിൽ (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		അല്ലെങ്കിൽ
			eerror("ERROR: %s stopped by something else", applet);
		പോകുക exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	എങ്കിൽ (!rc_runlevel_stopping()) {
		എങ്കിൽ (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		അല്ലെങ്കിൽ എങ്കിൽ (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	തിരികെ 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

സ്ഥിതം ശൂന്യം
svc_stop_deps(RC_SERVICE state)
{
	പൂർണ്ണസംഖ്യ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	എങ്കിൽ (state & RC_SERVICE_WASINACTIVE)
		തിരികെ;

	errno = 0;
	എങ്കിൽ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	എങ്കിൽ (!deptree && ((deptree = _rc_deptree_load(0, ശൂന്യ)) == ശൂന്യ)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	എങ്കിൽ (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->മൂല്യം);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		എങ്കിൽ (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			തുടരുക;
		എങ്കിൽ (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			തുടരുക;
		എങ്കിൽ (dry_run) {
			printf(" %s", svc->മൂല്യം);
			തുടരുക;
		}
		svc_wait(svc->മൂല്യം);
		എങ്കിൽ (rc_service_state(svc->മൂല്യം) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->മൂല്യം);
			എങ്കിൽ (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->മൂല്യം);
		}
	}
	rc_stringlist_free(services);
	services = ശൂന്യ;
	എങ്കിൽ (dry_run)
		തിരികെ;

	TAILQ_FOREACH(svc, tmplist, entries) {
		എങ്കിൽ (rc_service_state(svc->മൂല്യം) & RC_SERVICE_STOPPED) {
			തുടരുക;
		}
		svc_wait(svc->മൂല്യം);
		എങ്കിൽ (rc_service_state(svc->മൂല്യം) & RC_SERVICE_STOPPED)
			തുടരുക;
		എങ്കിൽ (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			എങ്കിൽ (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				തുടരുക;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->മൂല്യം);
	}
	rc_stringlist_free(tmplist);
	tmplist = ശൂന്യ;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		എങ്കിൽ (rc_service_state(svc->മൂല്യം) & RC_SERVICE_STOPPED)
			തുടരുക;
		svc_wait(svc->മൂല്യം);
	}
	rc_stringlist_free(services);
	services = ശൂന്യ;
}

സ്ഥിതം ശൂന്യം
load_dep_env(ശൂന്യം)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->മൂല്യം);

	rc_stringlist_free(depsvcs);
}

സ്ഥിതം ശൂന്യം
svc_stop_real(ശൂന്യം)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	എങ്കിൽ (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	എങ്കിൽ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	എങ്കിൽ (ibsave)
		unsetenv("IN_BACKGROUND");

	എങ്കിൽ (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	എങ്കിൽ (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
svc_stop(ശൂന്യം)
{
	RC_SERVICE state = 0;

	എങ്കിൽ (dry_run)
		einfon("stop:");
	അല്ലെങ്കിൽ
		എങ്കിൽ (svc_stop_check(&state) == 1)
			തിരികെ 1; /* Service has been stopped already */

	എങ്കിൽ (deps)
		svc_stop_deps(state);

	എങ്കിൽ (dry_run)
		printf(" %s\n", applet);
	അല്ലെങ്കിൽ
		svc_stop_real();

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
svc_restart(ശൂന്യം)
{
	എങ്കിൽ (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		എങ്കിൽ (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ശൂന്യ;

	തിരികെ 0;
}

സ്ഥിതം bool
service_plugable(ശൂന്യം)
{
	അക്ഷരം *list, *p, *token;
	bool allow = true, truefalse;
	അക്ഷരം *match = rc_conf_മൂല്യം("rc_hotplug");

	എങ്കിൽ (!match)
		match = rc_conf_മൂല്യം("rc_plug_services");
	എങ്കിൽ (!match)
		തിരികെ false;

	list = xstrdup(match);
	p = list;
	ആകുമ്പോൾ ((token = strsep(&p, " "))) {
		എങ്കിൽ (token[0] == '!') {
			truefalse = false;
			token++;
		} അല്ലെങ്കിൽ {
			truefalse = true;
		}

		എങ്കിൽ (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			നിർത്തുക;
		}
	}
	സ്വതന്ത്രം(list);
	തിരികെ allow;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
simple_command(ശൂന്യം)
{
	അക്ഷരം *save = prefix;
	പൂർണ്ണസംഖ്യ retval;

	eprefix(ശൂന്യ);
	prefix = ശൂന്യ;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	തിരികെ retval;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
resolve_deptype(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	എങ്കിൽ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	എങ്കിൽ (!deptree && ((deptree = _rc_deptree_load(0, ശൂന്യ)) == ശൂന്യ))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ശൂന്യ;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->മൂല്യം);
	printf ("\n");

	rc_stringlist_free(services);
	services = ശൂന്യ;

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
cmd_condrestart(ശൂന്യം)
{
	എങ്കിൽ (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
cmd_stop(ശൂന്യം)
{
	RC_STRING *svc;

	എങ്കിൽ (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	എങ്കിൽ (deps && in_background)
		get_started_services();

	എങ്കിൽ (svc_stop() == 1)
		തിരികെ 0; /* Service has been stopped already */

	എങ്കിൽ (deps) {
		എങ്കിൽ (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		എങ്കിൽ (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				എങ്കിൽ (rc_service_state(svc->മൂല്യം) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->മൂല്യം);
		}
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
cmd_zap(ശൂന്യം)
{
	einfo("Manually resetting %s to stopped state", applet);
	എങ്കിൽ (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	തിരികെ 0;
}

ഘടന {
	സ്ഥിരം അക്ഷരം *name;
	പൂർണ്ണസംഖ്യ (*handler)(ശൂന്യം);
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

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ retval, opt;
	സ്ഥിരം അക്ഷരം *workingdir = "/";
	അക്ഷരം *pidstr = ശൂന്യ;
	size_t l = 0, ll;
	ഘടന stat stbuf;

	/* Show help if insufficient args */
	എങ്കിൽ (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	എങ്കിൽ (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	എങ്കിൽ (!(service = (realpath(argv[1], ശൂന്യ))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	എങ്കിൽ (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring, longopts, ശൂന്യ)) != -1)
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'd':
			setenv("RC_DEBUG", "YES", 1);
			നിർത്തുക;
		അവസ്ഥ 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			നിർത്തുക;
		അവസ്ഥ 's':
			എങ്കിൽ (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			നിർത്തുക;
		അവസ്ഥ 'S':
			എങ്കിൽ (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			നിർത്തുക;
		അവസ്ഥ 'D':
			deps = false;
			നിർത്തുക;
		അവസ്ഥ 'Z':
			dry_run = true;
			നിർത്തുക;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	എങ്കിൽ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	എങ്കിൽ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	എങ്കിൽ ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ശൂന്യ) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (പൂർണ്ണസംഖ്യ) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	സ്വതന്ത്രം(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	എങ്കിൽ (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(ശൂന്യ);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->മൂല്യം);
			എങ്കിൽ (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ശൂന്യ;
		ll = strlen(applet);
		എങ്കിൽ (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(വലുപ്പം(അക്ഷരം) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	എങ്കിൽ (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	എങ്കിൽ (deps && getenv("RC_PID") == ശൂന്യ && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	എങ്കിൽ (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	എങ്കിൽ (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	എങ്കിൽ (rc_yesno(getenv("IN_HOTPLUG"))) {
		എങ്കിൽ (!service_plugable())
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

	എങ്കിൽ (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	ആകുമ്പോൾ (optind < argc) {
		പൂർണ്ണസംഖ്യ cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		എങ്കിൽ (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			എങ്കിൽ (strcmp(optarg, commands[i].name) != 0)
				തുടരുക;

			എങ്കിൽ ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			എങ്കിൽ (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ശൂന്യ;
			}

			പോകുക next_cmd;
		}

		എങ്കിൽ ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ശൂന്യ;
	}

	തിരികെ retval;
}
