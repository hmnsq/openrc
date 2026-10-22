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

#అన్తర్భావ <ctype.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <fnmatch.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <libgen.h>
#అన్తర్భావ <poll.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <spawn.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/file.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <termios.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <inttypes.h>

#ఒకవేళ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#లేదావేళ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#లేకపోతే
#  include <libutil.h>
#అయితే_ముగింపు

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "plugin.h"
#అన్తర్భావ "selinux.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

#నిర్వచించు WAIT_TIMEOUT	60		/* seconds until we timeout */
#నిర్వచించు WARN_TIMEOUT	10		/* warn about this every N seconds */

బాహ్య అక్షరం **environ;

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = "stop | start | restart | status | describe | zap";
స్థిరరాశి అక్షరం getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "debug",      0, ఖాళీ, 'd'},
	{ "dry-run",    0, ఖాళీ, 'Z'},
	{ "ifstarted",  0, ఖాళీ, 's'},
	{ "ifstopped",  0, ఖాళీ, 'S'},
	{ "nodeps",     0, ఖాళీ, 'D'},
	{ "lockfd",     1, ఖాళీ, 'l'},
	longopts_COMMON
};
స్థిరరాశి అక్షరం *స్థిరరాశి longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

స్థిర అక్షరం *service, *runlevel, *ibsave, *ఉపసర్గ;
స్థిర RC_DEPTREE *deptree;
స్థిర RC_STRINGLIST *applet_list, *services, *tmplist;
స్థిర RC_STRINGLIST *restart_services;
స్థిర RC_STRINGLIST *need_services;
స్థిర RC_STRINGLIST *use_services;
స్థిర RC_STRINGLIST *want_services;
స్థిర పూర్ణాంకం exclusive_fd = -1, master_tty = -1;
స్థిర bool లోపల_background, deps, dry_నిరంతరం;
స్థిర అస్థిర bool sighup, skip_mark, timedout;
స్థిర pid_t service_pid;
స్థిర పూర్ణాంకం signal_pipe[2] = { -1, -1 };

స్థిర RC_STRINGLIST *deptypes_b;	/* broken deps */
స్థిర RC_STRINGLIST *deptypes_n;	/* needed deps */
స్థిర RC_STRINGLIST *deptypes_nw;	/* need+want deps */
స్థిర RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
స్థిర RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
స్థిర RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
స్థిర RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

స్థిర శూన్య
handle_signal(పూర్ణాంకం sig)
{
	పూర్ణాంకం serrno = errno, స్థితి_2;
	pid_t pid;
	స్థిరరాశి అక్షరం *signame = ఖాళీ;
	నిర్మాణం winsize ws;

	ఎంపిక (sig) {
	స్థితి SIGHUP:
		sighup = true;
		ఆపు;

	స్థితి SIGUSR1:
		skip_mark = true;
		ఆపు;

	స్థితి SIGCHLD:
		అయితే ((pid = waitpid(-1, &స్థితి_2, WNOHANG)) > 0) {
			ఒకవేళ (signal_pipe[1] > -1 && pid == service_pid) {
				ఒకవేళ (లిఖ(signal_pipe[1], &స్థితి_2, పరిమాణం(స్థితి_2)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		ఆపు;

	స్థితి SIGWINCH:
		ఒకవేళ (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		ఆపు;

	స్థితి SIGINT:
		ఒకవేళ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	స్థితి SIGTERM:
		ఒకవేళ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	స్థితి SIGQUIT:
		ఒకవేళ (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		ఒకవేళ (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		నిష్క్రమించు(EXIT_FAILURE);
		/* NOTREACHED */

	స్థితి SIGALRM:
		timedout = true;
		ఆపు;

	అప్రమేయ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

స్థిర శూన్య
unhotplug(శూన్య)
{
	ఒకవేళ (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

స్థిర శూన్య
ప్రారంభం_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	ఒకవేళ (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		తిరిగి;

	TAILQ_FOREACH(svc, list, entries) {
		ఒకవేళ (!(rc_service_state(svc->విలువ) & RC_SERVICE_STOPPED))
			కొనసాగు;
		ఒకవేళ (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_కాలపట్టిక_ప్రారంభం(applet, svc->విలువ);
			ewarn("WARNING: %s will start when %s has started", svc->విలువ, applet);
		} లేకపోతే {
			service_ప్రారంభం(svc->విలువ);
		}
	}
}

స్థిర శూన్య
restore_state(శూన్య)
{
	RC_SERVICE state;

	ఒకవేళ (rc_లోపల_plugin || exclusive_fd == -1)
		తిరిగి;
	state = rc_service_state(applet);
	ఒకవేళ (state & RC_SERVICE_STOPPING) {
		ఒకవేళ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		లేకపోతే
			rc_service_mark(applet, RC_SERVICE_STARTED);

		ఒకవేళ (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} లేకపోతే ఒకవేళ (state & RC_SERVICE_STARTING) {
		ఒకవేళ (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		లేకపోతే
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		ఒకవేళ (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

స్థిర శూన్య
cleanup(శూన్య)
{
	restore_state();

	ఒకవేళ (!rc_లోపల_plugin) {
		ఒకవేళ (restart_services)
			ప్రారంభం_services(restart_services);
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
	విడుదల(ibsave);
	విడుదల(ఉపసర్గ);
	విడుదల(runlevel);
	విడుదల(service);
}

స్థిర శూన్య
svc_getenv(స్థిరరాశి అక్షరం *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	అక్షరం *var = ఖాళీ;

	TAILQ_FOREACH(reexport, reexports, entries) {
		ఒకవేళ (rc_service_getenv(svc, reexport->విలువ, &var) == -1)
			కొనసాగు;
		setenv(reexport->విలువ, var, true);
	}

	విడుదల(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
స్థిర పూర్ణాంకం
write_ఉపసర్గ(స్థిరరాశి అక్షరం *బఫరు, size_t బైట్లు, bool *prefixed)
{
	size_t i, j;
	స్థిరరాశి అక్షరం *ec = ecolor(ECOLOR_HILITE);
	స్థిరరాశి అక్షరం *ec_సాధారణ = ecolor(ECOLOR_సాధారణ);
	ssize_t ret = 0;
	పూర్ణాంకం fd = fileno(stdout), తాళం_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	తాళం_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	ఒకవేళ (తాళం_fd != -1) {
		అయితే (flock(తాళం_fd, LOCK_EX) != 0) {
			ఒకవేళ (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				ఆపు;
			}
		}
	}
	లేకపోతే
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	కొరకు (i = 0; i < బైట్లు; i++) {
		/* We don't prefix eend calls (cursor up) */
		ఒకవేళ (బఫరు[i] == '\033' && !*prefixed) {
			కొరకు (j = i + 1; j < బైట్లు; j++) {
				ఒకవేళ (బఫరు[j] == 'A')
					*prefixed = true;
				ఒకవేళ (అక్షరమా((గుర్తులేని పూర్ణాంకం)బఫరు[j]))
					ఆపు;
			}
		}

		ఒకవేళ (!*prefixed) {
			ret += లిఖ(fd, ec, అక్షరశ్రేణి_పొడవు(ec));
			ret += లిఖ(fd, ఉపసర్గ, అక్షరశ్రేణి_పొడవు(ఉపసర్గ));
			ret += లిఖ(fd, ec_సాధారణ, అక్షరశ్రేణి_పొడవు(ec_సాధారణ));
			ret += లిఖ(fd, "|", 1);
			*prefixed = true;
		}

		ఒకవేళ (బఫరు[i] == '\n')
			*prefixed = false;
		ret += లిఖ(fd, బఫరు + i, 1);
	}

	/* Release the lock */
	close(తాళం_fd);

	తిరిగి ret;
}

స్థిర పూర్ణాంకం
svc_exec(స్థిరరాశి అక్షరం *command)
{
	పూర్ణాంకం ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	నిర్మాణం termios tt;
	నిర్మాణం winsize ws;
	పూర్ణాంకం flags = 0;
	నిర్మాణం pollfd fd[2];
	bool prefixed = false;
	పూర్ణాంకం slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	అక్షరం *openrc_sh = ఖాళీ;
	స్థిరరాశి అక్షరం *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ఖాళీ
	};

	/* Setup our signal pipe */
	ఒకవేళ (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		తిరిగి 1;
	}

	ఒకవేళ ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		తిరిగి 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	ఒకవేళ (ఉపసర్గ && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		ఒకవేళ (openpty(&master_tty, &slave_tty, ఖాళీ, &tt, &ws) == 0) {
			ఒకవేళ (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ఒకవేళ (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			ఒకవేళ ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				తిరిగి 1;
			}
		}
	}

	ఒకవేళ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	ఒకవేళ ((errno = posix_spawn(&service_pid, argv[0], &tty, ఖాళీ, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		తిరిగి 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	విడుదల(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	కొరకు (;;) {
		ఒకవేళ (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			ఒకవేళ (errno == EINTR)
				కొనసాగు;
			eerror("%s: poll: %s", applet, strerror(errno));
			ఆపు;
		}

		ఒకవేళ (fd[1].revents & (POLLIN | POLLHUP)) {
			అక్షరం బఫరు[BUFSIZ];
			write_ఉపసర్గ(బఫరు, పఠ(master_tty, బఫరు, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		ఒకవేళ (fd[0].revents & (POLLIN | POLLHUP)) {
			ఒకవేళ (errno = 0, పఠ(signal_pipe[0], &ret, పరిమాణం(ret)) < (ssize_t)పరిమాణం(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			లేకపోతే ఒకవేళ ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			ఆపు;
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

	sigprocmask (SIG_SETMASK, &oldmask, ఖాళీ);

	ఒకవేళ (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	తిరిగి ret;
}

స్థిర bool
svc_wait(స్థిరరాశి అక్షరం *svc)
{
	పూర్ణాంకం fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	స్థిరరాశి అక్షరం *పునాది = basename_c(svc);
	పూర్ణాంకం timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	ఒకవేళ (rc_stringlist_కనుగొను(keywords, "-timeout") || rc_stringlist_కనుగొను(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	ఒకవేళ ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), పునాది, O_RDONLY | O_NONBLOCK)) == -1)
		తిరిగి errno == ENOENT;

	timedout = false;
	కొరకు (;;) {
		ఒకవేళ (!forever)
			alarm(WARN_TIMEOUT);

		ఒకవేళ (flock(fd, LOCK_SH) == 0) {
			retval = true;
			ఆపు;
		}

		ఒకవేళ (errno != EINTR)
			ఆపు;

		ఒకవేళ (!forever && timedout) {
			timedout = false;
			ఒకవేళ ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, పునాది, timeout);
			లేకపోతే
				ఆపు;
		}
	}

	alarm(0);
	close(fd);
	తిరిగి retval;
}

స్థిర శూన్య
get_started_services(శూన్య)
{
	RC_STRINGLIST *tmp = rc_services_లోపల_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_లోపల_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	విడుదల(tmp);
}

స్థిర శూన్య
అమరిక_deptypes(శూన్య)
{
	deptypes_b = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_b, "broken");

	deptypes_n = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_n, "ineed");

	deptypes_nw = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_nw, "ineed");
	rc_stringlist_కూడు(deptypes_nw, "iwant");

	deptypes_nwu = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_nwu, "ineed");
	rc_stringlist_కూడు(deptypes_nwu, "iwant");
	rc_stringlist_కూడు(deptypes_nwu, "iuse");

	deptypes_nwua = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_nwua, "ineed");
	rc_stringlist_కూడు(deptypes_nwua, "iwant");
	rc_stringlist_కూడు(deptypes_nwua, "iuse");
	rc_stringlist_కూడు(deptypes_nwua, "iafter");

	deptypes_m = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_m, "needsme");

	deptypes_mwua = rc_stringlist_new();
	rc_stringlist_కూడు(deptypes_mwua, "needsme");
	rc_stringlist_కూడు(deptypes_mwua, "wantsme");
	rc_stringlist_కూడు(deptypes_mwua, "usesme");
	rc_stringlist_కూడు(deptypes_mwua, "beforeme");
}

స్థిర శూన్య
svc_ప్రారంభం_check(శూన్య)
{
	పూర్ణాంకం ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	ఒకవేళ (లోపల_background) {
		ఒకవేళ (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			వెళ్ళు నిష్క్రమించు;

		ఒకవేళ (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		ఒకవేళ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	ఒకవేళ (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		వెళ్ళు నిష్క్రమించు;
	} లేకపోతే ఒకవేళ (state & RC_SERVICE_INACTIVE && !లోపల_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	ఒకవేళ (exclusive_fd == -1)
		exclusive_fd = svc_తాళం(applet, !deps);
	ఒకవేళ (exclusive_fd == -1) {
		ఒకవేళ (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			వెళ్ళు నిష్క్రమించు;
		}

		ఒకవేళ (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		లేకపోతే
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_లోపల, applet);

	తిరిగి;

నిష్క్రమించు:
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_బహిర్గతం, applet);
	నిష్క్రమించు(ret);
}

స్థిర శూన్య
svc_ప్రారంభం_deps(శూన్య)
{
	bool మొదటి;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	పూర్ణాంకం depoptions = RC_DEP_TRACE;
	size_t len, svc_లెక్క;
	అక్షరం *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	ఒకవేళ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ఒకవేళ (!deptree && ((deptree = rc_deptree_ఎక్కించు_2(0, ఖాళీ)) == ఖాళీ)) {
		eerror("failed to load deptree");
		వెళ్ళు నిష్క్రమించు;
	}

	ఒకవేళ (!deptypes_b)
		అమరిక_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	ఒకవేళ (TAILQ_మొదటి(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		మొదటి = true;
		TAILQ_FOREACH(svc, services, entries) {
			ఒకవేళ (మొదటి)
				మొదటి = false;
			లేకపోతే
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->విలువ);
		}
		fprintf(stderr, "\n");
		వెళ్ళు నిష్క్రమించు;
	}
	rc_stringlist_free(services);
	services = ఖాళీ;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	ఒకవేళ (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->విలువ);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			ఒకవేళ (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				కొనసాగు;
			ఒకవేళ (state & RC_SERVICE_STOPPED) {
				ఒకవేళ (dry_నిరంతరం) {
					రాయి(" %s", svc->విలువ);
					కొనసాగు;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_ప్రారంభం(svc->విలువ);
				ఒకవేళ (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	ఒకవేళ (dry_నిరంతరం)
		తిరిగి;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->విలువ);
		ఒకవేళ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->విలువ);
			కొనసాగు;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		ఒకవేళ (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			ఒకవేళ (!rc_stringlist_కనుగొను(need_services, svc->విలువ) &&
					!rc_stringlist_కనుగొను(want_services, svc->విలువ) &&
					!rc_stringlist_కనుగొను(use_services, svc->విలువ))
				కొనసాగు;
		}

		ఒకవేళ (!svc_wait(svc->విలువ))
			eerror("%s: timed out waiting for %s", applet, svc->విలువ);
		state = rc_service_state(svc->విలువ);
		ఒకవేళ (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->విలువ);
			కొనసాగు;
		}
		ఒకవేళ (rc_stringlist_కనుగొను(need_services, svc->విలువ)) {
			ఒకవేళ (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_కూడు(tmplist, svc->విలువ);
			} లేకపోతే ఒకవేళ (!TAILQ_మొదటి(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->విలువ);
				వెళ్ళు నిష్క్రమించు;
			}
		}
	}

	ఒకవేళ (TAILQ_మొదటి(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ఖాళీ;
		mem = xopen_memstream(&tmp, &len);
		svc_లెక్క = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_కాలపట్టిక_ప్రారంభం(svc->విలువ, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->విలువ);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_కాలపట్టిక_ప్రారంభం(svc2->విలువ, applet);
			rc_stringlist_free(use_services);
			use_services = ఖాళీ;
			ఒకవేళ (svc_లెక్క > 0)
				fputs(", ", mem);
			fputs(svc->విలువ, mem);
			++svc_లెక్క;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ఖాళీ;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_లెక్క > 1 ? "have" : "has");
		విడుదల(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ఖాళీ;
	rc_stringlist_free(services);
	services = ఖాళీ;

	తిరిగి;

నిష్క్రమించు:
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_బహిర్గతం, applet);
	నిష్క్రమించు(EXIT_FAILURE);
}


స్థిర శూన్య సమితి_reexports(శూన్య)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t కొలత;
	అక్షరం *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &కొలత);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->విలువ);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

స్థిర శూన్య svc_ప్రారంభం_real(శూన్య)
{
	bool started;
	RC_STRING *svc, *svc2;

	సమితి_reexports();

	ఒకవేళ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	ఒకవేళ (ibsave)
		unsetenv("IN_BACKGROUND");

	ఒకవేళ (!started) {
		rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_పూర్తి, applet);
		rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_బహిర్గతం, applet);
	}

	ఒకవేళ (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	లేకపోతే ఒకవేళ (!started)
		eerrorx("ERROR: %s failed to start", applet);

	ఒకవేళ (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_పూర్తి, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    ఒకవేళ (rc_service_state(svc->విలువ) & RC_SERVICE_STOPPED)
		    service_ప్రారంభం(svc->విలువ);
	rc_stringlist_free(services);
	services = ఖాళీ;

	/* Do the same for any services we provide */
	ఒకవేళ (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->విలువ);
			TAILQ_FOREACH(svc2, services, entries)
				ఒకవేళ (rc_service_state(svc2->విలువ) & RC_SERVICE_STOPPED)
					service_ప్రారంభం(svc2->విలువ);
			rc_stringlist_free(services);
			services = ఖాళీ;
		}
		rc_stringlist_free(tmplist);
		tmplist = ఖాళీ;
	}

	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_ప్రారంభం_బహిర్గతం, applet);
}

స్థిర పూర్ణాంకం
svc_ప్రారంభం(శూన్య)
{
	ఒకవేళ (dry_నిరంతరం)
		einfon("start:");
	లేకపోతే
		svc_ప్రారంభం_check();

	ఒకవేళ (deps)
		svc_ప్రారంభం_deps();

	ఒకవేళ (dry_నిరంతరం)
		రాయి(" %s\n", applet);
	లేకపోతే
		svc_ప్రారంభం_real();

	తిరిగి 0;
}

స్థిర పూర్ణాంకం
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	ఒకవేళ (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		వెళ్ళు నిష్క్రమించు;

	ఒకవేళ (లోపల_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		వెళ్ళు నిష్క్రమించు;

	ఒకవేళ (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		తిరిగి 1;
	}

	ఒకవేళ (exclusive_fd == -1)
		exclusive_fd = svc_తాళం(applet, !deps);
	ఒకవేళ (exclusive_fd == -1) {
		ఒకవేళ (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		లేకపోతే ఒకవేళ (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		లేకపోతే
			eerror("ERROR: %s stopped by something else", applet);
		వెళ్ళు నిష్క్రమించు;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_లోపల, applet);

	ఒకవేళ (!rc_runlevel_stopping()) {
		ఒకవేళ (rc_service_లోపల_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		లేకపోతే ఒకవేళ (rc_service_లోపల_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	తిరిగి 0;

నిష్క్రమించు:
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_బహిర్గతం, applet);
	నిష్క్రమించు(EXIT_FAILURE);
}

స్థిర శూన్య
svc_stop_deps(RC_SERVICE state)
{
	పూర్ణాంకం depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	ఒకవేళ (state & RC_SERVICE_WASINACTIVE)
		తిరిగి;

	errno = 0;
	ఒకవేళ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ఒకవేళ (!deptree && ((deptree = rc_deptree_ఎక్కించు_2(0, ఖాళీ)) == ఖాళీ)) {
		rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_బహిర్గతం, applet);
		eerrorx("failed to load deptree");
	}

	ఒకవేళ (!deptypes_m)
		అమరిక_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_తిరగేయి(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->విలువ);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		ఒకవేళ (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			కొనసాగు;
		ఒకవేళ (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			కొనసాగు;
		ఒకవేళ (dry_నిరంతరం) {
			రాయి(" %s", svc->విలువ);
			కొనసాగు;
		}
		svc_wait(svc->విలువ);
		ఒకవేళ (rc_service_state(svc->విలువ) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->విలువ);
			ఒకవేళ (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_కూడు(tmplist, svc->విలువ);
		}
	}
	rc_stringlist_free(services);
	services = ఖాళీ;
	ఒకవేళ (dry_నిరంతరం)
		తిరిగి;

	TAILQ_FOREACH(svc, tmplist, entries) {
		ఒకవేళ (rc_service_state(svc->విలువ) & RC_SERVICE_STOPPED) {
			కొనసాగు;
		}
		svc_wait(svc->విలువ);
		ఒకవేళ (rc_service_state(svc->విలువ) & RC_SERVICE_STOPPED)
			కొనసాగు;
		ఒకవేళ (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			ఒకవేళ (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				కొనసాగు;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_బహిర్గతం, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->విలువ);
	}
	rc_stringlist_free(tmplist);
	tmplist = ఖాళీ;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		ఒకవేళ (rc_service_state(svc->విలువ) & RC_SERVICE_STOPPED)
			కొనసాగు;
		svc_wait(svc->విలువ);
	}
	rc_stringlist_free(services);
	services = ఖాళీ;
}

స్థిర శూన్య
ఎక్కించు_dep_env(శూన్య)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->విలువ);

	rc_stringlist_free(depsvcs);
}

స్థిర శూన్య
svc_stop_real(శూన్య)
{
	bool stopped;

	ఎక్కించు_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	ఒకవేళ (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	ఒకవేళ (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	ఒకవేళ (ibsave)
		unsetenv("IN_BACKGROUND");

	ఒకవేళ (stopped && !skip_mark)
		rc_service_mark(applet, లోపల_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_పూర్తి, applet);
	rc_plugin_నిరంతరం(RC_HOOK_SERVICE_STOP_బహిర్గతం, applet);

	ఒకవేళ (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

స్థిర పూర్ణాంకం
svc_stop(శూన్య)
{
	RC_SERVICE state = 0;

	ఒకవేళ (dry_నిరంతరం)
		einfon("stop:");
	లేకపోతే
		ఒకవేళ (svc_stop_check(&state) == 1)
			తిరిగి 1; /* Service has been stopped already */

	ఒకవేళ (deps)
		svc_stop_deps(state);

	ఒకవేళ (dry_నిరంతరం)
		రాయి(" %s\n", applet);
	లేకపోతే
		svc_stop_real();

	తిరిగి 0;
}

స్థిర పూర్ణాంకం
svc_restart(శూన్య)
{
	ఒకవేళ (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		ఒకవేళ (dry_నిరంతరం)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_ప్రారంభం();
	ప్రారంభం_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ఖాళీ;

	తిరిగి 0;
}

స్థిర bool
service_plugable(శూన్య)
{
	అక్షరం *list, *p, *token;
	bool allow = true, truefalse;
	అక్షరం *సరిపోలిక = rc_conf_విలువ("rc_hotplug");

	ఒకవేళ (!సరిపోలిక)
		సరిపోలిక = rc_conf_విలువ("rc_plug_services");
	ఒకవేళ (!సరిపోలిక)
		తిరిగి false;

	list = xstrdup(సరిపోలిక);
	p = list;
	అయితే ((token = strsep(&p, " "))) {
		ఒకవేళ (token[0] == '!') {
			truefalse = false;
			token++;
		} లేకపోతే {
			truefalse = true;
		}

		ఒకవేళ (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			ఆపు;
		}
	}
	విడుదల(list);
	తిరిగి allow;
}

స్థిర పూర్ణాంకం
simple_command(శూన్య)
{
	అక్షరం *save = ఉపసర్గ;
	పూర్ణాంకం retval;

	eprefix(ఖాళీ);
	ఉపసర్గ = ఖాళీ;

	retval = svc_exec(optarg);

	eprefix(save);
	ఉపసర్గ = save;

	తిరిగి retval;
}

స్థిర పూర్ణాంకం
resolve_deptype(శూన్య)
{
	పూర్ణాంకం depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	ఒకవేళ (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	ఒకవేళ (!deptree && ((deptree = rc_deptree_ఎక్కించు_2(0, ఖాళీ)) == ఖాళీ))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_కూడు(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ఖాళీ;
	TAILQ_FOREACH(svc, services, entries)
		రాయి("%s ", svc->విలువ);
	రాయి ("\n");

	rc_stringlist_free(services);
	services = ఖాళీ;

	తిరిగి 0;
}

స్థిర పూర్ణాంకం
cmd_condrestart(శూన్య)
{
	ఒకవేళ (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	తిరిగి 0;
}

స్థిర పూర్ణాంకం
cmd_stop(శూన్య)
{
	RC_STRING *svc;

	ఒకవేళ (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	ఒకవేళ (deps && లోపల_background)
		get_started_services();

	ఒకవేళ (svc_stop() == 1)
		తిరిగి 0; /* Service has been stopped already */

	ఒకవేళ (deps) {
		ఒకవేళ (!లోపల_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		ఒకవేళ (లోపల_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				ఒకవేళ (rc_service_state(svc->విలువ) & RC_SERVICE_STOPPED)
					rc_service_కాలపట్టిక_ప్రారంభం(applet, svc->విలువ);
		}
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం
cmd_zap(శూన్య)
{
	einfo("Manually resetting %s to stopped state", applet);
	ఒకవేళ (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	తిరిగి 0;
}

నిర్మాణం {
	స్థిరరాశి అక్షరం *name;
	పూర్ణాంకం (*handler)(శూన్య);
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
	{ "start",              svc_ప్రారంభం,       true  },
	{ "stop",               cmd_stop,        true  },
	{ "restart",            svc_restart,     true  },
	{ "condrestart",        cmd_condrestart, true  },
	{ "conditionalrestart", cmd_condrestart, true  },
	{ "zap",                cmd_zap,         true  },
};

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం retval, opt;
	స్థిరరాశి అక్షరం *workingdir = "/";
	అక్షరం *pidstr = ఖాళీ;
	size_t l = 0, ll;
	నిర్మాణం stat stbuf;

	/* Show help if insufficient args */
	ఒకవేళ (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		నిష్క్రమించు(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	ఒకవేళ (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		నిష్క్రమించు(EXIT_FAILURE);
	}

	ఒకవేళ (!(service = (realpath(argv[1], ఖాళీ))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	ఒకవేళ (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_అమరిక(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	అయితే ((opt = getopt_long(argc, argv, getoptstring, longopts, ఖాళీ)) != -1)
		ఎంపిక (opt) {
		స్థితి 'd':
			setenv("RC_DEBUG", "YES", 1);
			ఆపు;
		స్థితి 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			ఆపు;
		స్థితి 's':
			ఒకవేళ (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				నిష్క్రమించు(EXIT_FAILURE);
			ఆపు;
		స్థితి 'S':
			ఒకవేళ (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				నిష్క్రమించు(EXIT_FAILURE);
			ఆపు;
		స్థితి 'D':
			deps = false;
			ఆపు;
		స్థితి 'Z':
			dry_నిరంతరం = true;
			ఆపు;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ఒకవేళ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ఒకవేళ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	ఒకవేళ ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ఖాళీ) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (పూర్ణాంకం) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	విడుదల(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	ఒకవేళ (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_లోపల_runlevel(ఖాళీ);
		TAILQ_FOREACH(svc, services, entries) {
			ll = అక్షరశ్రేణి_పొడవు(svc->విలువ);
			ఒకవేళ (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ఖాళీ;
		ll = అక్షరశ్రేణి_పొడవు(applet);
		ఒకవేళ (ll > l)
			l = ll;

		/* Make our prefix string */
		ఉపసర్గ = xmalloc(పరిమాణం(అక్షరం) * l + 1);
		జ్ఞాపకం_నకలు(ఉపసర్గ, applet, ll);
		memset(ఉపసర్గ + ll, ' ', l - ll);
		memset(ఉపసర్గ + l, 0, 1);
		eprefix(ఉపసర్గ);
	}

	ఒకవేళ (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	ఒకవేళ (deps && getenv("RC_PID") == ఖాళీ && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	ఒకవేళ (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		లోపల_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	ఒకవేళ (rc_yesno(getenv("IN_DRYRUN")))
		dry_నిరంతరం = true;
	ఒకవేళ (rc_yesno(getenv("IN_HOTPLUG"))) {
		ఒకవేళ (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		లోపల_background = true;
	}

	/* Setup a signal handler */
	signal_అమరిక(SIGHUP, handle_signal);
	signal_అమరిక(SIGALRM, handle_signal);
	signal_అమరిక(SIGUSR1, handle_signal);
	signal_అమరిక(SIGINT, handle_signal);
	signal_అమరిక(SIGQUIT, handle_signal);
	signal_అమరిక(SIGTERM, handle_signal);
	signal_అమరిక(SIGCHLD, handle_signal);

	/* Load our plugins */
	rc_plugin_ఎక్కించు();

	applet_list = rc_stringlist_new();
	rc_stringlist_కూడు(applet_list, applet);

	ఒకవేళ (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

తదుపరి_cmd:
	అయితే (optind < argc) {
		పూర్ణాంకం cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		ఒకవేళ (sighup)
			నిష్క్రమించు (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		కొరకు (size_t i = 0; i < ARRAY_కొలత(commands); i++) {
			ఒకవేళ (strcmp(optarg, commands[i].name) != 0)
				కొనసాగు;

			ఒకవేళ ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			ఒకవేళ (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ఖాళీ;
			}

			వెళ్ళు తదుపరి_cmd;
		}

		ఒకవేళ ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ఖాళీ;
	}

	తిరిగి retval;
}
