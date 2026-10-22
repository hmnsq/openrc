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

#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <fnmatch.h>
#অন্তর্ভাৱ <getopt.h>
#অন্তর্ভাৱ <libgen.h>
#অন্তর্ভাৱ <poll.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <spawn.h>
#অন্তর্ভাৱ <sys/ioctl.h>
#অন্তর্ভাৱ <sys/file.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <termios.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <inttypes.h>

#যদি defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#নোচেদ্যদি defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#অন্যথা
#  include <libutil.h>
#endif

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "rc_exec.h"
#অন্তর্ভাৱ "misc.h"
#অন্তর্ভাৱ "timeutils.h"
#অন্তর্ভাৱ "plugin.h"
#অন্তর্ভাৱ "selinux.h"
#অন্তর্ভাৱ "_usage.h"
#অন্তর্ভাৱ "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

বাহ্য ৱর্ণ **environ;

ধ্রুৱ ৱর্ণ *applet = রিক্ত;
ধ্রুৱ ৱর্ণ *extraopts = "stop | start | restart | status | describe | zap";
ধ্রুৱ ৱর্ণ getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ধ্রুৱ সংরচনা option longopts[] = {
	{ "debug",      0, রিক্ত, 'd'},
	{ "dry-run",    0, রিক্ত, 'Z'},
	{ "ifstarted",  0, রিক্ত, 's'},
	{ "ifstopped",  0, রিক্ত, 'S'},
	{ "nodeps",     0, রিক্ত, 'D'},
	{ "lockfd",     1, রিক্ত, 'l'},
	longopts_COMMON
};
ধ্রুৱ ৱর্ণ *ধ্রুৱ longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ধ্রুৱ ৱর্ণ *usagestring = রিক্ত;

স্থির ৱর্ণ *service, *runlevel, *ibsave, *prefix;
স্থির RC_DEPTREE *deptree;
স্থির RC_STRINGLIST *applet_list, *services, *tmplist;
স্থির RC_STRINGLIST *restart_services;
স্থির RC_STRINGLIST *need_services;
স্থির RC_STRINGLIST *use_services;
স্থির RC_STRINGLIST *want_services;
স্থির পূর্ণাঙ্ক exclusive_fd = -1, master_tty = -1;
স্থির bool in_background, deps, dry_run;
স্থির চঞ্চল bool sighup, skip_mark, timedout;
স্থির pid_t service_pid;
স্থির পূর্ণাঙ্ক signal_pipe[2] = { -1, -1 };

স্থির RC_STRINGLIST *deptypes_b;	/* broken deps */
স্থির RC_STRINGLIST *deptypes_n;	/* needed deps */
স্থির RC_STRINGLIST *deptypes_nw;	/* need+want deps */
স্থির RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
স্থির RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
স্থির RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
স্থির RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

স্থির শূন্য
handle_signal(পূর্ণাঙ্ক sig)
{
	পূর্ণাঙ্ক serrno = errno, status;
	pid_t pid;
	ধ্রুৱ ৱর্ণ *signame = রিক্ত;
	সংরচনা winsize ws;

	ৱিকল্প (sig) {
	পক্ষ SIGHUP:
		sighup = true;
		ভঙ্গ;

	পক্ষ SIGUSR1:
		skip_mark = true;
		ভঙ্গ;

	পক্ষ SIGCHLD:
		যেতিয়ালৈ ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			যদি (signal_pipe[1] > -1 && pid == service_pid) {
				যদি (লিখ(signal_pipe[1], &status, পরিমাণ(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		ভঙ্গ;

	পক্ষ SIGWINCH:
		যদি (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		ভঙ্গ;

	পক্ষ SIGINT:
		যদি (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	পক্ষ SIGTERM:
		যদি (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	পক্ষ SIGQUIT:
		যদি (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		যদি (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	পক্ষ SIGALRM:
		timedout = true;
		ভঙ্গ;

	স্ৱভাৱ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

স্থির শূন্য
unhotplug(শূন্য)
{
	যদি (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

স্থির শূন্য
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	যদি (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		ঘূৰাই;

	TAILQ_FOREACH(svc, list, entries) {
		যদি (!(rc_service_state(svc->মান) & RC_SERVICE_STOPPED))
			অনুৱর্ত;
		যদি (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->মান);
			ewarn("WARNING: %s will start when %s has started", svc->মান, applet);
		} অন্যথা {
			service_start(svc->মান);
		}
	}
}

স্থির শূন্য
restore_state(শূন্য)
{
	RC_SERVICE state;

	যদি (rc_in_plugin || exclusive_fd == -1)
		ঘূৰাই;
	state = rc_service_state(applet);
	যদি (state & RC_SERVICE_STOPPING) {
		যদি (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		অন্যথা
			rc_service_mark(applet, RC_SERVICE_STARTED);

		যদি (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} অন্যথা যদি (state & RC_SERVICE_STARTING) {
		যদি (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		অন্যথা
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		যদি (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

স্থির শূন্য
cleanup(শূন্য)
{
	restore_state();

	যদি (!rc_in_plugin) {
		যদি (restart_services)
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
	মুক্ত(ibsave);
	মুক্ত(prefix);
	মুক্ত(runlevel);
	মুক্ত(service);
}

স্থির শূন্য
svc_getenv(ধ্রুৱ ৱর্ণ *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	ৱর্ণ *var = রিক্ত;

	TAILQ_FOREACH(reexport, reexports, entries) {
		যদি (rc_service_getenv(svc, reexport->মান, &var) == -1)
			অনুৱর্ত;
		setenv(reexport->মান, var, true);
	}

	মুক্ত(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
স্থির পূর্ণাঙ্ক
write_prefix(ধ্রুৱ ৱর্ণ *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	ধ্রুৱ ৱর্ণ *ec = ecolor(ECOLOR_HILITE);
	ধ্রুৱ ৱর্ণ *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	পূর্ণাঙ্ক fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	যদি (lock_fd != -1) {
		যেতিয়ালৈ (flock(lock_fd, LOCK_EX) != 0) {
			যদি (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				ভঙ্গ;
			}
		}
	}
	অন্যথা
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	বাবে (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		যদি (buffer[i] == '\033' && !*prefixed) {
			বাবে (j = i + 1; j < bytes; j++) {
				যদি (buffer[j] == 'A')
					*prefixed = true;
				যদি (isalpha((unsigned পূর্ণাঙ্ক)buffer[j]))
					ভঙ্গ;
			}
		}

		যদি (!*prefixed) {
			ret += লিখ(fd, ec, strlen(ec));
			ret += লিখ(fd, prefix, strlen(prefix));
			ret += লিখ(fd, ec_normal, strlen(ec_normal));
			ret += লিখ(fd, "|", 1);
			*prefixed = true;
		}

		যদি (buffer[i] == '\n')
			*prefixed = false;
		ret += লিখ(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক
svc_exec(ধ্রুৱ ৱর্ণ *command)
{
	পূর্ণাঙ্ক ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	সংরচনা termios tt;
	সংরচনা winsize ws;
	পূর্ণাঙ্ক flags = 0;
	সংরচনা pollfd fd[2];
	bool prefixed = false;
	পূর্ণাঙ্ক slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	ৱর্ণ *openrc_sh = রিক্ত;
	ধ্রুৱ ৱর্ণ *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		রিক্ত
	};

	/* Setup our signal pipe */
	যদি (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		ঘূৰাই 1;
	}

	যদি ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		ঘূৰাই 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	যদি (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		যদি (openpty(&master_tty, &slave_tty, রিক্ত, &tt, &ws) == 0) {
			যদি (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			যদি (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			যদি ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				ঘূৰাই 1;
			}
		}
	}

	যদি (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	যদি ((errno = posix_spawn(&service_pid, argv[0], &tty, রিক্ত, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		ঘূৰাই 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	মুক্ত(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	বাবে (;;) {
		যদি (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			যদি (errno == EINTR)
				অনুৱর্ত;
			eerror("%s: poll: %s", applet, strerror(errno));
			ভঙ্গ;
		}

		যদি (fd[1].revents & (POLLIN | POLLHUP)) {
			ৱর্ণ buffer[BUFSIZ];
			write_prefix(buffer, পঠ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		যদি (fd[0].revents & (POLLIN | POLLHUP)) {
			যদি (errno = 0, পঠ(signal_pipe[0], &ret, পরিমাণ(ret)) < (ssize_t)পরিমাণ(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			অন্যথা যদি ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			ভঙ্গ;
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

	sigprocmask (SIG_SETMASK, &oldmask, রিক্ত);

	যদি (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	ঘূৰাই ret;
}

স্থির bool
svc_wait(ধ্রুৱ ৱর্ণ *svc)
{
	পূর্ণাঙ্ক fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ধ্রুৱ ৱর্ণ *base = basename_c(svc);
	পূর্ণাঙ্ক timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	যদি (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	যদি ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		ঘূৰাই errno == ENOENT;

	timedout = false;
	বাবে (;;) {
		যদি (!forever)
			alarm(WARN_TIMEOUT);

		যদি (flock(fd, LOCK_SH) == 0) {
			retval = true;
			ভঙ্গ;
		}

		যদি (errno != EINTR)
			ভঙ্গ;

		যদি (!forever && timedout) {
			timedout = false;
			যদি ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			অন্যথা
				ভঙ্গ;
		}
	}

	alarm(0);
	close(fd);
	ঘূৰাই retval;
}

স্থির শূন্য
get_started_services(শূন্য)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	মুক্ত(tmp);
}

স্থির শূন্য
setup_deptypes(শূন্য)
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

স্থির শূন্য
svc_start_check(শূন্য)
{
	পূর্ণাঙ্ক ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	যদি (in_background) {
		যদি (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			গচ্ছ exit;

		যদি (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		যদি (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	যদি (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		গচ্ছ exit;
	} অন্যথা যদি (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	যদি (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	যদি (exclusive_fd == -1) {
		যদি (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			গচ্ছ exit;
		}

		যদি (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		অন্যথা
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	ঘূৰাই;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

স্থির শূন্য
svc_start_deps(শূন্য)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	পূর্ণাঙ্ক depoptions = RC_DEP_TRACE;
	size_t len, svc_গুনতি;
	ৱর্ণ *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	যদি (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	যদি (!deptree && ((deptree = _rc_deptree_load(0, রিক্ত)) == রিক্ত)) {
		eerror("failed to load deptree");
		গচ্ছ exit;
	}

	যদি (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	যদি (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			যদি (first)
				first = false;
			অন্যথা
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->মান);
		}
		fprintf(stderr, "\n");
		গচ্ছ exit;
	}
	rc_stringlist_free(services);
	services = রিক্ত;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	যদি (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->মান);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			যদি (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				অনুৱর্ত;
			যদি (state & RC_SERVICE_STOPPED) {
				যদি (dry_run) {
					printf(" %s", svc->মান);
					অনুৱর্ত;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->মান);
				যদি (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	যদি (dry_run)
		ঘূৰাই;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->মান);
		যদি (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->মান);
			অনুৱর্ত;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		যদি (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			যদি (!rc_stringlist_find(need_services, svc->মান) &&
					!rc_stringlist_find(want_services, svc->মান) &&
					!rc_stringlist_find(use_services, svc->মান))
				অনুৱর্ত;
		}

		যদি (!svc_wait(svc->মান))
			eerror("%s: timed out waiting for %s", applet, svc->মান);
		state = rc_service_state(svc->মান);
		যদি (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->মান);
			অনুৱর্ত;
		}
		যদি (rc_stringlist_find(need_services, svc->মান)) {
			যদি (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->মান);
			} অন্যথা যদি (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->মান);
				গচ্ছ exit;
			}
		}
	}

	যদি (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = রিক্ত;
		mem = xopen_memstream(&tmp, &len);
		svc_গুনতি = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->মান, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->মান);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->মান, applet);
			rc_stringlist_free(use_services);
			use_services = রিক্ত;
			যদি (svc_গুনতি > 0)
				fputs(", ", mem);
			fputs(svc->মান, mem);
			++svc_গুনতি;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = রিক্ত;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_গুনতি > 1 ? "have" : "has");
		মুক্ত(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = রিক্ত;
	rc_stringlist_free(services);
	services = রিক্ত;

	ঘূৰাই;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


স্থির শূন্য set_reexports(শূন্য)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	ৱর্ণ *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->মান);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

স্থির শূন্য svc_start_real(শূন্য)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	যদি (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	যদি (ibsave)
		unsetenv("IN_BACKGROUND");

	যদি (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	যদি (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	অন্যথা যদি (!started)
		eerrorx("ERROR: %s failed to start", applet);

	যদি (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    যদি (rc_service_state(svc->মান) & RC_SERVICE_STOPPED)
		    service_start(svc->মান);
	rc_stringlist_free(services);
	services = রিক্ত;

	/* Do the same for any services we provide */
	যদি (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->মান);
			TAILQ_FOREACH(svc2, services, entries)
				যদি (rc_service_state(svc2->মান) & RC_SERVICE_STOPPED)
					service_start(svc2->মান);
			rc_stringlist_free(services);
			services = রিক্ত;
		}
		rc_stringlist_free(tmplist);
		tmplist = রিক্ত;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

স্থির পূর্ণাঙ্ক
svc_start(শূন্য)
{
	যদি (dry_run)
		einfon("start:");
	অন্যথা
		svc_start_check();

	যদি (deps)
		svc_start_deps();

	যদি (dry_run)
		printf(" %s\n", applet);
	অন্যথা
		svc_start_real();

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	যদি (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		গচ্ছ exit;

	যদি (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		গচ্ছ exit;

	যদি (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		ঘূৰাই 1;
	}

	যদি (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	যদি (exclusive_fd == -1) {
		যদি (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		অন্যথা যদি (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		অন্যথা
			eerror("ERROR: %s stopped by something else", applet);
		গচ্ছ exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	যদি (!rc_runlevel_stopping()) {
		যদি (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		অন্যথা যদি (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	ঘূৰাই 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

স্থির শূন্য
svc_stop_deps(RC_SERVICE state)
{
	পূর্ণাঙ্ক depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	যদি (state & RC_SERVICE_WASINACTIVE)
		ঘূৰাই;

	errno = 0;
	যদি (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	যদি (!deptree && ((deptree = _rc_deptree_load(0, রিক্ত)) == রিক্ত)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	যদি (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->মান);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		যদি (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			অনুৱর্ত;
		যদি (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			অনুৱর্ত;
		যদি (dry_run) {
			printf(" %s", svc->মান);
			অনুৱর্ত;
		}
		svc_wait(svc->মান);
		যদি (rc_service_state(svc->মান) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->মান);
			যদি (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->মান);
		}
	}
	rc_stringlist_free(services);
	services = রিক্ত;
	যদি (dry_run)
		ঘূৰাই;

	TAILQ_FOREACH(svc, tmplist, entries) {
		যদি (rc_service_state(svc->মান) & RC_SERVICE_STOPPED) {
			অনুৱর্ত;
		}
		svc_wait(svc->মান);
		যদি (rc_service_state(svc->মান) & RC_SERVICE_STOPPED)
			অনুৱর্ত;
		যদি (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			যদি (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				অনুৱর্ত;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->মান);
	}
	rc_stringlist_free(tmplist);
	tmplist = রিক্ত;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		যদি (rc_service_state(svc->মান) & RC_SERVICE_STOPPED)
			অনুৱর্ত;
		svc_wait(svc->মান);
	}
	rc_stringlist_free(services);
	services = রিক্ত;
}

স্থির শূন্য
load_dep_env(শূন্য)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->মান);

	rc_stringlist_free(depsvcs);
}

স্থির শূন্য
svc_stop_real(শূন্য)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	যদি (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	যদি (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	যদি (ibsave)
		unsetenv("IN_BACKGROUND");

	যদি (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	যদি (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

স্থির পূর্ণাঙ্ক
svc_stop(শূন্য)
{
	RC_SERVICE state = 0;

	যদি (dry_run)
		einfon("stop:");
	অন্যথা
		যদি (svc_stop_check(&state) == 1)
			ঘূৰাই 1; /* Service has been stopped already */

	যদি (deps)
		svc_stop_deps(state);

	যদি (dry_run)
		printf(" %s\n", applet);
	অন্যথা
		svc_stop_real();

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক
svc_restart(শূন্য)
{
	যদি (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		যদি (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = রিক্ত;

	ঘূৰাই 0;
}

স্থির bool
service_plugable(শূন্য)
{
	ৱর্ণ *list, *p, *token;
	bool allow = true, truefalse;
	ৱর্ণ *match = rc_conf_মান("rc_hotplug");

	যদি (!match)
		match = rc_conf_মান("rc_plug_services");
	যদি (!match)
		ঘূৰাই false;

	list = xstrdup(match);
	p = list;
	যেতিয়ালৈ ((token = strsep(&p, " "))) {
		যদি (token[0] == '!') {
			truefalse = false;
			token++;
		} অন্যথা {
			truefalse = true;
		}

		যদি (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			ভঙ্গ;
		}
	}
	মুক্ত(list);
	ঘূৰাই allow;
}

স্থির পূর্ণাঙ্ক
simple_command(শূন্য)
{
	ৱর্ণ *save = prefix;
	পূর্ণাঙ্ক retval;

	eprefix(রিক্ত);
	prefix = রিক্ত;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	ঘূৰাই retval;
}

স্থির পূর্ণাঙ্ক
resolve_deptype(শূন্য)
{
	পূর্ণাঙ্ক depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	যদি (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	যদি (!deptree && ((deptree = _rc_deptree_load(0, রিক্ত)) == রিক্ত))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = রিক্ত;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->মান);
	printf ("\n");

	rc_stringlist_free(services);
	services = রিক্ত;

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক
cmd_condrestart(শূন্য)
{
	যদি (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক
cmd_stop(শূন্য)
{
	RC_STRING *svc;

	যদি (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	যদি (deps && in_background)
		get_started_services();

	যদি (svc_stop() == 1)
		ঘূৰাই 0; /* Service has been stopped already */

	যদি (deps) {
		যদি (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		যদি (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				যদি (rc_service_state(svc->মান) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->মান);
		}
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক
cmd_zap(শূন্য)
{
	einfo("Manually resetting %s to stopped state", applet);
	যদি (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	ঘূৰাই 0;
}

সংরচনা {
	ধ্রুৱ ৱর্ণ *name;
	পূর্ণাঙ্ক (*handler)(শূন্য);
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

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক retval, opt;
	ধ্রুৱ ৱর্ণ *workingdir = "/";
	ৱর্ণ *pidstr = রিক্ত;
	size_t l = 0, ll;
	সংরচনা stat stbuf;

	/* Show help if insufficient args */
	যদি (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	যদি (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	যদি (!(service = (realpath(argv[1], রিক্ত))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	যদি (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	যেতিয়ালৈ ((opt = getopt_long(argc, argv, getoptstring, longopts, রিক্ত)) != -1)
		ৱিকল্প (opt) {
		পক্ষ 'd':
			setenv("RC_DEBUG", "YES", 1);
			ভঙ্গ;
		পক্ষ 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			ভঙ্গ;
		পক্ষ 's':
			যদি (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			ভঙ্গ;
		পক্ষ 'S':
			যদি (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			ভঙ্গ;
		পক্ষ 'D':
			deps = false;
			ভঙ্গ;
		পক্ষ 'Z':
			dry_run = true;
			ভঙ্গ;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	যদি (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	যদি (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	যদি ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == রিক্ত) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (পূর্ণাঙ্ক) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	মুক্ত(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	যদি (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(রিক্ত);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->মান);
			যদি (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = রিক্ত;
		ll = strlen(applet);
		যদি (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(পরিমাণ(ৱর্ণ) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	যদি (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	যদি (deps && getenv("RC_PID") == রিক্ত && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	যদি (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	যদি (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	যদি (rc_yesno(getenv("IN_HOTPLUG"))) {
		যদি (!service_plugable())
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

	যদি (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	যেতিয়ালৈ (optind < argc) {
		পূর্ণাঙ্ক cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		যদি (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		বাবে (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			যদি (strcmp(optarg, commands[i].name) != 0)
				অনুৱর্ত;

			যদি ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			যদি (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = রিক্ত;
			}

			গচ্ছ next_cmd;
		}

		যদি ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = রিক্ত;
	}

	ঘূৰাই retval;
}
