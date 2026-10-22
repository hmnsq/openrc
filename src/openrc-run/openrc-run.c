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

#شامل <ctype.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <fnmatch.h>
#شامل <getopt.h>
#شامل <libgen.h>
#شامل <poll.h>
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <spawn.h>
#شامل <sys/ioctl.h>
#شامل <sys/file.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <termios.h>
#شامل <time.h>
#شامل <unistd.h>
#شامل <stdbool.h>
#شامل <inttypes.h>

#اگر defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#وگرنهاگر defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#وگرنه
#  include <libutil.h>
#endif

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "misc.h"
#شامل "timeutils.h"
#شامل "plugin.h"
#شامل "selinux.h"
#شامل "_usage.h"
#شامل "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

بیرونی نویسه **environ;

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = "stop | start | restart | status | describe | zap";
ثابت نویسه getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "debug",      0, پوچ, 'd'},
	{ "dry-run",    0, پوچ, 'Z'},
	{ "ifstarted",  0, پوچ, 's'},
	{ "ifstopped",  0, پوچ, 'S'},
	{ "nodeps",     0, پوچ, 'D'},
	{ "lockfd",     1, پوچ, 'l'},
	longopts_COMMON
};
ثابت نویسه *ثابت longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ثابت نویسه *usagestring = پوچ;

ایستا نویسه *service, *runlevel, *ibsave, *prefix;
ایستا RC_DEPTREE *deptree;
ایستا RC_STRINGLIST *applet_list, *services, *tmplist;
ایستا RC_STRINGLIST *restart_services;
ایستا RC_STRINGLIST *need_services;
ایستا RC_STRINGLIST *use_services;
ایستا RC_STRINGLIST *want_services;
ایستا صحیح exclusive_fd = -1, master_tty = -1;
ایستا bool in_background, deps, dry_run;
ایستا فرار bool sighup, skip_mark, timedout;
ایستا pid_t service_pid;
ایستا صحیح signal_pipe[2] = { -1, -1 };

ایستا RC_STRINGLIST *deptypes_b;	/* broken deps */
ایستا RC_STRINGLIST *deptypes_n;	/* needed deps */
ایستا RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ایستا RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ایستا RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ایستا RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ایستا RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ایستا تهی
handle_signal(صحیح sig)
{
	صحیح serrno = errno, status;
	pid_t pid;
	ثابت نویسه *signame = پوچ;
	ساختار winsize ws;

	انتخاب (sig) {
	حالت SIGHUP:
		sighup = true;
		بشکن;

	حالت SIGUSR1:
		skip_mark = true;
		بشکن;

	حالت SIGCHLD:
		وقتی ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			اگر (signal_pipe[1] > -1 && pid == service_pid) {
				اگر (اكتب(signal_pipe[1], &status, اندازه(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		بشکن;

	حالت SIGWINCH:
		اگر (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		بشکن;

	حالت SIGINT:
		اگر (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالت SIGTERM:
		اگر (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالت SIGQUIT:
		اگر (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		اگر (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	حالت SIGALRM:
		timedout = true;
		بشکن;

	پیشفرض:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ایستا تهی
unhotplug(تهی)
{
	اگر (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ایستا تهی
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	اگر (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		بازگشت;

	TAILQ_FOREACH(svc, list, entries) {
		اگر (!(rc_service_state(svc->مقدار) & RC_SERVICE_STOPPED))
			ادامه;
		اگر (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->مقدار);
			ewarn("WARNING: %s will start when %s has started", svc->مقدار, applet);
		} وگرنه {
			service_start(svc->مقدار);
		}
	}
}

ایستا تهی
restore_state(تهی)
{
	RC_SERVICE state;

	اگر (rc_in_plugin || exclusive_fd == -1)
		بازگشت;
	state = rc_service_state(applet);
	اگر (state & RC_SERVICE_STOPPING) {
		اگر (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		وگرنه
			rc_service_mark(applet, RC_SERVICE_STARTED);

		اگر (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} وگرنه اگر (state & RC_SERVICE_STARTING) {
		اگر (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		وگرنه
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		اگر (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ایستا تهی
cleanup(تهی)
{
	restore_state();

	اگر (!rc_in_plugin) {
		اگر (restart_services)
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
	آزاد(ibsave);
	آزاد(prefix);
	آزاد(runlevel);
	آزاد(service);
}

ایستا تهی
svc_getenv(ثابت نویسه *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	نویسه *var = پوچ;

	TAILQ_FOREACH(reexport, reexports, entries) {
		اگر (rc_service_getenv(svc, reexport->مقدار, &var) == -1)
			ادامه;
		setenv(reexport->مقدار, var, true);
	}

	آزاد(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
ایستا صحیح
write_prefix(ثابت نویسه *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	ثابت نویسه *ec = ecolor(ECOLOR_HILITE);
	ثابت نویسه *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	صحیح fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	اگر (lock_fd != -1) {
		وقتی (flock(lock_fd, LOCK_EX) != 0) {
			اگر (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				بشکن;
			}
		}
	}
	وگرنه
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	برای (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		اگر (buffer[i] == '\033' && !*prefixed) {
			برای (j = i + 1; j < bytes; j++) {
				اگر (buffer[j] == 'A')
					*prefixed = true;
				اگر (isalpha((unsigned صحیح)buffer[j]))
					بشکن;
			}
		}

		اگر (!*prefixed) {
			ret += اكتب(fd, ec, strlen(ec));
			ret += اكتب(fd, prefix, strlen(prefix));
			ret += اكتب(fd, ec_normal, strlen(ec_normal));
			ret += اكتب(fd, "|", 1);
			*prefixed = true;
		}

		اگر (buffer[i] == '\n')
			*prefixed = false;
		ret += اكتب(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	بازگشت ret;
}

ایستا صحیح
svc_exec(ثابت نویسه *command)
{
	صحیح ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ساختار termios tt;
	ساختار winsize ws;
	صحیح flags = 0;
	ساختار pollfd fd[2];
	bool prefixed = false;
	صحیح slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	نویسه *openrc_sh = پوچ;
	ثابت نویسه *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		پوچ
	};

	/* Setup our signal pipe */
	اگر (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		بازگشت 1;
	}

	اگر ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		بازگشت 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	اگر (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		اگر (openpty(&master_tty, &slave_tty, پوچ, &tt, &ws) == 0) {
			اگر (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			اگر (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			اگر ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				بازگشت 1;
			}
		}
	}

	اگر (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	اگر ((errno = posix_spawn(&service_pid, argv[0], &tty, پوچ, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		بازگشت 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	آزاد(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	برای (;;) {
		اگر (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			اگر (errno == EINTR)
				ادامه;
			eerror("%s: poll: %s", applet, strerror(errno));
			بشکن;
		}

		اگر (fd[1].revents & (POLLIN | POLLHUP)) {
			نویسه buffer[BUFSIZ];
			write_prefix(buffer, اقرأ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		اگر (fd[0].revents & (POLLIN | POLLHUP)) {
			اگر (errno = 0, اقرأ(signal_pipe[0], &ret, اندازه(ret)) < (ssize_t)اندازه(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			وگرنه اگر ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			بشکن;
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

	sigprocmask (SIG_SETMASK, &oldmask, پوچ);

	اگر (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	بازگشت ret;
}

ایستا bool
svc_wait(ثابت نویسه *svc)
{
	صحیح fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ثابت نویسه *base = basename_c(svc);
	صحیح timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	اگر (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	اگر ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		بازگشت errno == ENOENT;

	timedout = false;
	برای (;;) {
		اگر (!forever)
			alarm(WARN_TIMEOUT);

		اگر (flock(fd, LOCK_SH) == 0) {
			retval = true;
			بشکن;
		}

		اگر (errno != EINTR)
			بشکن;

		اگر (!forever && timedout) {
			timedout = false;
			اگر ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			وگرنه
				بشکن;
		}
	}

	alarm(0);
	close(fd);
	بازگشت retval;
}

ایستا تهی
get_started_services(تهی)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	آزاد(tmp);
}

ایستا تهی
setup_deptypes(تهی)
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

ایستا تهی
svc_start_check(تهی)
{
	صحیح ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	اگر (in_background) {
		اگر (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			برو exit;

		اگر (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	اگر (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		برو exit;
	} وگرنه اگر (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	اگر (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	اگر (exclusive_fd == -1) {
		اگر (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			برو exit;
		}

		اگر (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		وگرنه
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	بازگشت;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

ایستا تهی
svc_start_deps(تهی)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	صحیح depoptions = RC_DEP_TRACE;
	size_t len, svc_شمار;
	نویسه *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	اگر (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	اگر (!deptree && ((deptree = _rc_deptree_load(0, پوچ)) == پوچ)) {
		eerror("failed to load deptree");
		برو exit;
	}

	اگر (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	اگر (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			اگر (first)
				first = false;
			وگرنه
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->مقدار);
		}
		fprintf(stderr, "\n");
		برو exit;
	}
	rc_stringlist_free(services);
	services = پوچ;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	اگر (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->مقدار);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			اگر (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				ادامه;
			اگر (state & RC_SERVICE_STOPPED) {
				اگر (dry_run) {
					printf(" %s", svc->مقدار);
					ادامه;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->مقدار);
				اگر (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	اگر (dry_run)
		بازگشت;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->مقدار);
		اگر (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->مقدار);
			ادامه;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		اگر (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			اگر (!rc_stringlist_find(need_services, svc->مقدار) &&
					!rc_stringlist_find(want_services, svc->مقدار) &&
					!rc_stringlist_find(use_services, svc->مقدار))
				ادامه;
		}

		اگر (!svc_wait(svc->مقدار))
			eerror("%s: timed out waiting for %s", applet, svc->مقدار);
		state = rc_service_state(svc->مقدار);
		اگر (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->مقدار);
			ادامه;
		}
		اگر (rc_stringlist_find(need_services, svc->مقدار)) {
			اگر (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->مقدار);
			} وگرنه اگر (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->مقدار);
				برو exit;
			}
		}
	}

	اگر (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = پوچ;
		mem = xopen_memstream(&tmp, &len);
		svc_شمار = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->مقدار, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->مقدار);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->مقدار, applet);
			rc_stringlist_free(use_services);
			use_services = پوچ;
			اگر (svc_شمار > 0)
				fputs(", ", mem);
			fputs(svc->مقدار, mem);
			++svc_شمار;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = پوچ;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_شمار > 1 ? "have" : "has");
		آزاد(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = پوچ;
	rc_stringlist_free(services);
	services = پوچ;

	بازگشت;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


ایستا تهی set_reexports(تهی)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	نویسه *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->مقدار);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ایستا تهی svc_start_real(تهی)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	اگر (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	اگر (ibsave)
		unsetenv("IN_BACKGROUND");

	اگر (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	اگر (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	وگرنه اگر (!started)
		eerrorx("ERROR: %s failed to start", applet);

	اگر (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    اگر (rc_service_state(svc->مقدار) & RC_SERVICE_STOPPED)
		    service_start(svc->مقدار);
	rc_stringlist_free(services);
	services = پوچ;

	/* Do the same for any services we provide */
	اگر (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->مقدار);
			TAILQ_FOREACH(svc2, services, entries)
				اگر (rc_service_state(svc2->مقدار) & RC_SERVICE_STOPPED)
					service_start(svc2->مقدار);
			rc_stringlist_free(services);
			services = پوچ;
		}
		rc_stringlist_free(tmplist);
		tmplist = پوچ;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

ایستا صحیح
svc_start(تهی)
{
	اگر (dry_run)
		einfon("start:");
	وگرنه
		svc_start_check();

	اگر (deps)
		svc_start_deps();

	اگر (dry_run)
		printf(" %s\n", applet);
	وگرنه
		svc_start_real();

	بازگشت 0;
}

ایستا صحیح
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	اگر (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		برو exit;

	اگر (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		برو exit;

	اگر (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		بازگشت 1;
	}

	اگر (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	اگر (exclusive_fd == -1) {
		اگر (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		وگرنه اگر (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		وگرنه
			eerror("ERROR: %s stopped by something else", applet);
		برو exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	اگر (!rc_runlevel_stopping()) {
		اگر (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		وگرنه اگر (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	بازگشت 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

ایستا تهی
svc_stop_deps(RC_SERVICE state)
{
	صحیح depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	اگر (state & RC_SERVICE_WASINACTIVE)
		بازگشت;

	errno = 0;
	اگر (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	اگر (!deptree && ((deptree = _rc_deptree_load(0, پوچ)) == پوچ)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	اگر (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->مقدار);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		اگر (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			ادامه;
		اگر (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			ادامه;
		اگر (dry_run) {
			printf(" %s", svc->مقدار);
			ادامه;
		}
		svc_wait(svc->مقدار);
		اگر (rc_service_state(svc->مقدار) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->مقدار);
			اگر (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->مقدار);
		}
	}
	rc_stringlist_free(services);
	services = پوچ;
	اگر (dry_run)
		بازگشت;

	TAILQ_FOREACH(svc, tmplist, entries) {
		اگر (rc_service_state(svc->مقدار) & RC_SERVICE_STOPPED) {
			ادامه;
		}
		svc_wait(svc->مقدار);
		اگر (rc_service_state(svc->مقدار) & RC_SERVICE_STOPPED)
			ادامه;
		اگر (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			اگر (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				ادامه;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->مقدار);
	}
	rc_stringlist_free(tmplist);
	tmplist = پوچ;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		اگر (rc_service_state(svc->مقدار) & RC_SERVICE_STOPPED)
			ادامه;
		svc_wait(svc->مقدار);
	}
	rc_stringlist_free(services);
	services = پوچ;
}

ایستا تهی
load_dep_env(تهی)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->مقدار);

	rc_stringlist_free(depsvcs);
}

ایستا تهی
svc_stop_real(تهی)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	اگر (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	اگر (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	اگر (ibsave)
		unsetenv("IN_BACKGROUND");

	اگر (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	اگر (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ایستا صحیح
svc_stop(تهی)
{
	RC_SERVICE state = 0;

	اگر (dry_run)
		einfon("stop:");
	وگرنه
		اگر (svc_stop_check(&state) == 1)
			بازگشت 1; /* Service has been stopped already */

	اگر (deps)
		svc_stop_deps(state);

	اگر (dry_run)
		printf(" %s\n", applet);
	وگرنه
		svc_stop_real();

	بازگشت 0;
}

ایستا صحیح
svc_restart(تهی)
{
	اگر (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		اگر (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = پوچ;

	بازگشت 0;
}

ایستا bool
service_plugable(تهی)
{
	نویسه *list, *p, *token;
	bool allow = true, truefalse;
	نویسه *match = rc_conf_مقدار("rc_hotplug");

	اگر (!match)
		match = rc_conf_مقدار("rc_plug_services");
	اگر (!match)
		بازگشت false;

	list = xstrdup(match);
	p = list;
	وقتی ((token = strsep(&p, " "))) {
		اگر (token[0] == '!') {
			truefalse = false;
			token++;
		} وگرنه {
			truefalse = true;
		}

		اگر (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			بشکن;
		}
	}
	آزاد(list);
	بازگشت allow;
}

ایستا صحیح
simple_command(تهی)
{
	نویسه *save = prefix;
	صحیح retval;

	eprefix(پوچ);
	prefix = پوچ;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	بازگشت retval;
}

ایستا صحیح
resolve_deptype(تهی)
{
	صحیح depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	اگر (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	اگر (!deptree && ((deptree = _rc_deptree_load(0, پوچ)) == پوچ))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = پوچ;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->مقدار);
	printf ("\n");

	rc_stringlist_free(services);
	services = پوچ;

	بازگشت 0;
}

ایستا صحیح
cmd_condrestart(تهی)
{
	اگر (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	بازگشت 0;
}

ایستا صحیح
cmd_stop(تهی)
{
	RC_STRING *svc;

	اگر (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	اگر (deps && in_background)
		get_started_services();

	اگر (svc_stop() == 1)
		بازگشت 0; /* Service has been stopped already */

	اگر (deps) {
		اگر (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		اگر (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				اگر (rc_service_state(svc->مقدار) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->مقدار);
		}
	}

	بازگشت 0;
}

ایستا صحیح
cmd_zap(تهی)
{
	einfo("Manually resetting %s to stopped state", applet);
	اگر (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	بازگشت 0;
}

ساختار {
	ثابت نویسه *name;
	صحیح (*handler)(تهی);
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

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح retval, opt;
	ثابت نویسه *workingdir = "/";
	نویسه *pidstr = پوچ;
	size_t l = 0, ll;
	ساختار stat stbuf;

	/* Show help if insufficient args */
	اگر (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	اگر (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	اگر (!(service = (realpath(argv[1], پوچ))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	اگر (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	وقتی ((opt = getopt_long(argc, argv, getoptstring, longopts, پوچ)) != -1)
		انتخاب (opt) {
		حالت 'd':
			setenv("RC_DEBUG", "YES", 1);
			بشکن;
		حالت 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			بشکن;
		حالت 's':
			اگر (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			بشکن;
		حالت 'S':
			اگر (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			بشکن;
		حالت 'D':
			deps = false;
			بشکن;
		حالت 'Z':
			dry_run = true;
			بشکن;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	اگر (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	اگر (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	اگر ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == پوچ) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (صحیح) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	آزاد(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	اگر (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(پوچ);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->مقدار);
			اگر (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = پوچ;
		ll = strlen(applet);
		اگر (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(اندازه(نویسه) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	اگر (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	اگر (deps && getenv("RC_PID") == پوچ && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	اگر (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	اگر (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	اگر (rc_yesno(getenv("IN_HOTPLUG"))) {
		اگر (!service_plugable())
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

	اگر (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	وقتی (optind < argc) {
		صحیح cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		اگر (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		برای (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			اگر (strcmp(optarg, commands[i].name) != 0)
				ادامه;

			اگر ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			اگر (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = پوچ;
			}

			برو next_cmd;
		}

		اگر ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = پوچ;
	}

	بازگشت retval;
}
