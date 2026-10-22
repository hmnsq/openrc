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
#وگرنہ_اگر defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ورنہ
#  include <libutil.h>
#اگر_ختم

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

#تعریف_کرو WAIT_TIMEOUT	60		/* seconds until we timeout */
#تعریف_کرو WARN_TIMEOUT	10		/* warn about this every N seconds */

خارجی حرف **environ;

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = "stop | start | restart | status | describe | zap";
ثابت حرف getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "debug",      0, تہی, 'd'},
	{ "dry-run",    0, تہی, 'Z'},
	{ "ifstarted",  0, تہی, 's'},
	{ "ifstopped",  0, تہی, 'S'},
	{ "nodeps",     0, تہی, 'D'},
	{ "lockfd",     1, تہی, 'l'},
	longopts_COMMON
};
ثابت حرف *ثابت longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ثابت حرف *usagestring = تہی;

ساکن حرف *service, *runlevel, *ibsave, *سابقہ;
ساکن RC_DEPTREE *deptree;
ساکن RC_STRINGLIST *applet_list, *services, *tmplist;
ساکن RC_STRINGLIST *restart_services;
ساکن RC_STRINGLIST *need_services;
ساکن RC_STRINGLIST *use_services;
ساکن RC_STRINGLIST *want_services;
ساکن صحیح exclusive_fd = -1, master_tty = -1;
ساکن bool اندر_background, deps, dry_تسلسل;
ساکن متلون bool sighup, skip_mark, timedout;
ساکن pid_t service_pid;
ساکن صحیح signal_pipe[2] = { -1, -1 };

ساکن RC_STRINGLIST *deptypes_b;	/* broken deps */
ساکن RC_STRINGLIST *deptypes_n;	/* needed deps */
ساکن RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ساکن RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ساکن RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ساکن RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ساکن RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ساکن خالی
handle_signal(صحیح sig)
{
	صحیح serrno = errno, حالت_2;
	pid_t pid;
	ثابت حرف *signame = تہی;
	ساخت winsize ws;

	انتخاب (sig) {
	حالت SIGHUP:
		sighup = true;
		توڑو;

	حالت SIGUSR1:
		skip_mark = true;
		توڑو;

	حالت SIGCHLD:
		جب ((pid = waitpid(-1, &حالت_2, WNOHANG)) > 0) {
			اگر (signal_pipe[1] > -1 && pid == service_pid) {
				اگر (اكتب(signal_pipe[1], &حالت_2, حجم(حالت_2)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		توڑو;

	حالت SIGWINCH:
		اگر (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		توڑو;

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
		باہر_نکلو(EXIT_FAILURE);
		/* NOTREACHED */

	حالت SIGALRM:
		timedout = true;
		توڑو;

	طےشدہ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ساکن خالی
unhotplug(خالی)
{
	اگر (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ساکن خالی
آغاز_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	اگر (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		واپس;

	TAILQ_FOREACH(svc, list, entries) {
		اگر (!(rc_service_state(svc->قدر) & RC_SERVICE_STOPPED))
			جاری;
		اگر (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_جدول_وقت_آغاز(applet, svc->قدر);
			ewarn("WARNING: %s will start when %s has started", svc->قدر, applet);
		} ورنہ {
			service_آغاز(svc->قدر);
		}
	}
}

ساکن خالی
restore_state(خالی)
{
	RC_SERVICE state;

	اگر (rc_اندر_plugin || exclusive_fd == -1)
		واپس;
	state = rc_service_state(applet);
	اگر (state & RC_SERVICE_STOPPING) {
		اگر (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ورنہ
			rc_service_mark(applet, RC_SERVICE_STARTED);

		اگر (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} ورنہ اگر (state & RC_SERVICE_STARTING) {
		اگر (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		ورنہ
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		اگر (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ساکن خالی
cleanup(خالی)
{
	restore_state();

	اگر (!rc_اندر_plugin) {
		اگر (restart_services)
			آغاز_services(restart_services);
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
	آزاد(سابقہ);
	آزاد(runlevel);
	آزاد(service);
}

ساکن خالی
svc_getenv(ثابت حرف *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	حرف *var = تہی;

	TAILQ_FOREACH(reexport, reexports, entries) {
		اگر (rc_service_getenv(svc, reexport->قدر, &var) == -1)
			جاری;
		setenv(reexport->قدر, var, true);
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
ساکن صحیح
write_سابقہ(ثابت حرف *بفر, size_t بائٹیں, bool *prefixed)
{
	size_t i, j;
	ثابت حرف *ec = ecolor(ECOLOR_HILITE);
	ثابت حرف *ec_معمول = ecolor(ECOLOR_معمول);
	ssize_t ret = 0;
	صحیح fd = fileno(stdout), تالا_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	تالا_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	اگر (تالا_fd != -1) {
		جب (flock(تالا_fd, LOCK_EX) != 0) {
			اگر (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				توڑو;
			}
		}
	}
	ورنہ
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	برائے (i = 0; i < بائٹیں; i++) {
		/* We don't prefix eend calls (cursor up) */
		اگر (بفر[i] == '\033' && !*prefixed) {
			برائے (j = i + 1; j < بائٹیں; j++) {
				اگر (بفر[j] == 'A')
					*prefixed = true;
				اگر (حرف_ہے((غیرعلامتی صحیح)بفر[j]))
					توڑو;
			}
		}

		اگر (!*prefixed) {
			ret += اكتب(fd, ec, لڑی_لمبائی(ec));
			ret += اكتب(fd, سابقہ, لڑی_لمبائی(سابقہ));
			ret += اكتب(fd, ec_معمول, لڑی_لمبائی(ec_معمول));
			ret += اكتب(fd, "|", 1);
			*prefixed = true;
		}

		اگر (بفر[i] == '\n')
			*prefixed = false;
		ret += اكتب(fd, بفر + i, 1);
	}

	/* Release the lock */
	close(تالا_fd);

	واپس ret;
}

ساکن صحیح
svc_exec(ثابت حرف *command)
{
	صحیح ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	ساخت termios tt;
	ساخت winsize ws;
	صحیح flags = 0;
	ساخت pollfd fd[2];
	bool prefixed = false;
	صحیح slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	حرف *openrc_sh = تہی;
	ثابت حرف *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		تہی
	};

	/* Setup our signal pipe */
	اگر (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		واپس 1;
	}

	اگر ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		واپس 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	اگر (سابقہ && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		اگر (openpty(&master_tty, &slave_tty, تہی, &tt, &ws) == 0) {
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
				واپس 1;
			}
		}
	}

	اگر (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	اگر ((errno = posix_spawn(&service_pid, argv[0], &tty, تہی, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		واپس 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	آزاد(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	برائے (;;) {
		اگر (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			اگر (errno == EINTR)
				جاری;
			eerror("%s: poll: %s", applet, strerror(errno));
			توڑو;
		}

		اگر (fd[1].revents & (POLLIN | POLLHUP)) {
			حرف بفر[BUFSIZ];
			write_سابقہ(بفر, اقرأ(master_tty, بفر, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		اگر (fd[0].revents & (POLLIN | POLLHUP)) {
			اگر (errno = 0, اقرأ(signal_pipe[0], &ret, حجم(ret)) < (ssize_t)حجم(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			ورنہ اگر ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			توڑو;
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

	sigprocmask (SIG_SETMASK, &oldmask, تہی);

	اگر (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	واپس ret;
}

ساکن bool
svc_wait(ثابت حرف *svc)
{
	صحیح fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ثابت حرف *بنیاد = basename_c(svc);
	صحیح timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	اگر (rc_stringlist_ڈھونڈو(keywords, "-timeout") || rc_stringlist_ڈھونڈو(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	اگر ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), بنیاد, O_RDONLY | O_NONBLOCK)) == -1)
		واپس errno == ENOENT;

	timedout = false;
	برائے (;;) {
		اگر (!forever)
			alarm(WARN_TIMEOUT);

		اگر (flock(fd, LOCK_SH) == 0) {
			retval = true;
			توڑو;
		}

		اگر (errno != EINTR)
			توڑو;

		اگر (!forever && timedout) {
			timedout = false;
			اگر ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, بنیاد, timeout);
			ورنہ
				توڑو;
		}
	}

	alarm(0);
	close(fd);
	واپس retval;
}

ساکن خالی
get_started_services(خالی)
{
	RC_STRINGLIST *tmp = rc_services_اندر_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_اندر_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	آزاد(tmp);
}

ساکن خالی
تیاری_deptypes(خالی)
{
	deptypes_b = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_b, "broken");

	deptypes_n = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_n, "ineed");

	deptypes_nw = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_nw, "ineed");
	rc_stringlist_جوڑو(deptypes_nw, "iwant");

	deptypes_nwu = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_nwu, "ineed");
	rc_stringlist_جوڑو(deptypes_nwu, "iwant");
	rc_stringlist_جوڑو(deptypes_nwu, "iuse");

	deptypes_nwua = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_nwua, "ineed");
	rc_stringlist_جوڑو(deptypes_nwua, "iwant");
	rc_stringlist_جوڑو(deptypes_nwua, "iuse");
	rc_stringlist_جوڑو(deptypes_nwua, "iafter");

	deptypes_m = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_m, "needsme");

	deptypes_mwua = rc_stringlist_new();
	rc_stringlist_جوڑو(deptypes_mwua, "needsme");
	rc_stringlist_جوڑو(deptypes_mwua, "wantsme");
	rc_stringlist_جوڑو(deptypes_mwua, "usesme");
	rc_stringlist_جوڑو(deptypes_mwua, "beforeme");
}

ساکن خالی
svc_آغاز_check(خالی)
{
	صحیح ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	اگر (اندر_background) {
		اگر (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			جاؤ باہر_نکلو;

		اگر (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	اگر (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		جاؤ باہر_نکلو;
	} ورنہ اگر (state & RC_SERVICE_INACTIVE && !اندر_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	اگر (exclusive_fd == -1)
		exclusive_fd = svc_تالا(applet, !deps);
	اگر (exclusive_fd == -1) {
		اگر (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			جاؤ باہر_نکلو;
		}

		اگر (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		ورنہ
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_اندر, applet);

	واپس;

باہر_نکلو:
	rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_برآمد, applet);
	باہر_نکلو(ret);
}

ساکن خالی
svc_آغاز_deps(خالی)
{
	bool پہلا;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	صحیح depoptions = RC_DEP_TRACE;
	size_t len, svc_گنتی;
	حرف *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	اگر (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	اگر (!deptree && ((deptree = rc_deptree_بھرو_2(0, تہی)) == تہی)) {
		eerror("failed to load deptree");
		جاؤ باہر_نکلو;
	}

	اگر (!deptypes_b)
		تیاری_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	اگر (TAILQ_پہلا(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		پہلا = true;
		TAILQ_FOREACH(svc, services, entries) {
			اگر (پہلا)
				پہلا = false;
			ورنہ
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->قدر);
		}
		fprintf(stderr, "\n");
		جاؤ باہر_نکلو;
	}
	rc_stringlist_free(services);
	services = تہی;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	اگر (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->قدر);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			اگر (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				جاری;
			اگر (state & RC_SERVICE_STOPPED) {
				اگر (dry_تسلسل) {
					لکھو(" %s", svc->قدر);
					جاری;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_آغاز(svc->قدر);
				اگر (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	اگر (dry_تسلسل)
		واپس;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->قدر);
		اگر (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->قدر);
			جاری;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		اگر (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			اگر (!rc_stringlist_ڈھونڈو(need_services, svc->قدر) &&
					!rc_stringlist_ڈھونڈو(want_services, svc->قدر) &&
					!rc_stringlist_ڈھونڈو(use_services, svc->قدر))
				جاری;
		}

		اگر (!svc_wait(svc->قدر))
			eerror("%s: timed out waiting for %s", applet, svc->قدر);
		state = rc_service_state(svc->قدر);
		اگر (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->قدر);
			جاری;
		}
		اگر (rc_stringlist_ڈھونڈو(need_services, svc->قدر)) {
			اگر (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_جوڑو(tmplist, svc->قدر);
			} ورنہ اگر (!TAILQ_پہلا(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->قدر);
				جاؤ باہر_نکلو;
			}
		}
	}

	اگر (TAILQ_پہلا(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = تہی;
		mem = xopen_memstream(&tmp, &len);
		svc_گنتی = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_جدول_وقت_آغاز(svc->قدر, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->قدر);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_جدول_وقت_آغاز(svc2->قدر, applet);
			rc_stringlist_free(use_services);
			use_services = تہی;
			اگر (svc_گنتی > 0)
				fputs(", ", mem);
			fputs(svc->قدر, mem);
			++svc_گنتی;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = تہی;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_گنتی > 1 ? "have" : "has");
		آزاد(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = تہی;
	rc_stringlist_free(services);
	services = تہی;

	واپس;

باہر_نکلو:
	rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_برآمد, applet);
	باہر_نکلو(EXIT_FAILURE);
}


ساکن خالی سیٹ_reexports(خالی)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t سائز;
	حرف *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &سائز);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->قدر);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ساکن خالی svc_آغاز_real(خالی)
{
	bool started;
	RC_STRING *svc, *svc2;

	سیٹ_reexports();

	اگر (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	اگر (ibsave)
		unsetenv("IN_BACKGROUND");

	اگر (!started) {
		rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_مکمل, applet);
		rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_برآمد, applet);
	}

	اگر (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	ورنہ اگر (!started)
		eerrorx("ERROR: %s failed to start", applet);

	اگر (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_مکمل, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    اگر (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
		    service_آغاز(svc->قدر);
	rc_stringlist_free(services);
	services = تہی;

	/* Do the same for any services we provide */
	اگر (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->قدر);
			TAILQ_FOREACH(svc2, services, entries)
				اگر (rc_service_state(svc2->قدر) & RC_SERVICE_STOPPED)
					service_آغاز(svc2->قدر);
			rc_stringlist_free(services);
			services = تہی;
		}
		rc_stringlist_free(tmplist);
		tmplist = تہی;
	}

	rc_plugin_تسلسل(RC_HOOK_SERVICE_آغاز_برآمد, applet);
}

ساکن صحیح
svc_آغاز(خالی)
{
	اگر (dry_تسلسل)
		einfon("start:");
	ورنہ
		svc_آغاز_check();

	اگر (deps)
		svc_آغاز_deps();

	اگر (dry_تسلسل)
		لکھو(" %s\n", applet);
	ورنہ
		svc_آغاز_real();

	واپس 0;
}

ساکن صحیح
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	اگر (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		جاؤ باہر_نکلو;

	اگر (اندر_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		جاؤ باہر_نکلو;

	اگر (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		واپس 1;
	}

	اگر (exclusive_fd == -1)
		exclusive_fd = svc_تالا(applet, !deps);
	اگر (exclusive_fd == -1) {
		اگر (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		ورنہ اگر (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		ورنہ
			eerror("ERROR: %s stopped by something else", applet);
		جاؤ باہر_نکلو;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_اندر, applet);

	اگر (!rc_runlevel_stopping()) {
		اگر (rc_service_اندر_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		ورنہ اگر (rc_service_اندر_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	واپس 0;

باہر_نکلو:
	rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_برآمد, applet);
	باہر_نکلو(EXIT_FAILURE);
}

ساکن خالی
svc_stop_deps(RC_SERVICE state)
{
	صحیح depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	اگر (state & RC_SERVICE_WASINACTIVE)
		واپس;

	errno = 0;
	اگر (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	اگر (!deptree && ((deptree = rc_deptree_بھرو_2(0, تہی)) == تہی)) {
		rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_برآمد, applet);
		eerrorx("failed to load deptree");
	}

	اگر (!deptypes_m)
		تیاری_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_الٹ(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->قدر);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		اگر (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			جاری;
		اگر (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			جاری;
		اگر (dry_تسلسل) {
			لکھو(" %s", svc->قدر);
			جاری;
		}
		svc_wait(svc->قدر);
		اگر (rc_service_state(svc->قدر) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->قدر);
			اگر (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_جوڑو(tmplist, svc->قدر);
		}
	}
	rc_stringlist_free(services);
	services = تہی;
	اگر (dry_تسلسل)
		واپس;

	TAILQ_FOREACH(svc, tmplist, entries) {
		اگر (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED) {
			جاری;
		}
		svc_wait(svc->قدر);
		اگر (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
			جاری;
		اگر (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			اگر (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				جاری;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_برآمد, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->قدر);
	}
	rc_stringlist_free(tmplist);
	tmplist = تہی;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		اگر (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
			جاری;
		svc_wait(svc->قدر);
	}
	rc_stringlist_free(services);
	services = تہی;
}

ساکن خالی
بھرو_dep_env(خالی)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->قدر);

	rc_stringlist_free(depsvcs);
}

ساکن خالی
svc_stop_real(خالی)
{
	bool stopped;

	بھرو_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	اگر (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	اگر (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	اگر (ibsave)
		unsetenv("IN_BACKGROUND");

	اگر (stopped && !skip_mark)
		rc_service_mark(applet, اندر_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_مکمل, applet);
	rc_plugin_تسلسل(RC_HOOK_SERVICE_STOP_برآمد, applet);

	اگر (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ساکن صحیح
svc_stop(خالی)
{
	RC_SERVICE state = 0;

	اگر (dry_تسلسل)
		einfon("stop:");
	ورنہ
		اگر (svc_stop_check(&state) == 1)
			واپس 1; /* Service has been stopped already */

	اگر (deps)
		svc_stop_deps(state);

	اگر (dry_تسلسل)
		لکھو(" %s\n", applet);
	ورنہ
		svc_stop_real();

	واپس 0;
}

ساکن صحیح
svc_restart(خالی)
{
	اگر (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		اگر (dry_تسلسل)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_آغاز();
	آغاز_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = تہی;

	واپس 0;
}

ساکن bool
service_plugable(خالی)
{
	حرف *list, *p, *token;
	bool allow = true, truefalse;
	حرف *مطابقت = rc_conf_قدر("rc_hotplug");

	اگر (!مطابقت)
		مطابقت = rc_conf_قدر("rc_plug_services");
	اگر (!مطابقت)
		واپس false;

	list = xstrdup(مطابقت);
	p = list;
	جب ((token = strsep(&p, " "))) {
		اگر (token[0] == '!') {
			truefalse = false;
			token++;
		} ورنہ {
			truefalse = true;
		}

		اگر (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			توڑو;
		}
	}
	آزاد(list);
	واپس allow;
}

ساکن صحیح
simple_command(خالی)
{
	حرف *save = سابقہ;
	صحیح retval;

	eprefix(تہی);
	سابقہ = تہی;

	retval = svc_exec(optarg);

	eprefix(save);
	سابقہ = save;

	واپس retval;
}

ساکن صحیح
resolve_deptype(خالی)
{
	صحیح depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	اگر (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	اگر (!deptree && ((deptree = rc_deptree_بھرو_2(0, تہی)) == تہی))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_جوڑو(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = تہی;
	TAILQ_FOREACH(svc, services, entries)
		لکھو("%s ", svc->قدر);
	لکھو ("\n");

	rc_stringlist_free(services);
	services = تہی;

	واپس 0;
}

ساکن صحیح
cmd_condrestart(خالی)
{
	اگر (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	واپس 0;
}

ساکن صحیح
cmd_stop(خالی)
{
	RC_STRING *svc;

	اگر (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	اگر (deps && اندر_background)
		get_started_services();

	اگر (svc_stop() == 1)
		واپس 0; /* Service has been stopped already */

	اگر (deps) {
		اگر (!اندر_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		اگر (اندر_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				اگر (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
					rc_service_جدول_وقت_آغاز(applet, svc->قدر);
		}
	}

	واپس 0;
}

ساکن صحیح
cmd_zap(خالی)
{
	einfo("Manually resetting %s to stopped state", applet);
	اگر (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	واپس 0;
}

ساخت {
	ثابت حرف *name;
	صحیح (*handler)(خالی);
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
	{ "start",              svc_آغاز,       true  },
	{ "stop",               cmd_stop,        true  },
	{ "restart",            svc_restart,     true  },
	{ "condrestart",        cmd_condrestart, true  },
	{ "conditionalrestart", cmd_condrestart, true  },
	{ "zap",                cmd_zap,         true  },
};

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح retval, opt;
	ثابت حرف *workingdir = "/";
	حرف *pidstr = تہی;
	size_t l = 0, ll;
	ساخت stat stbuf;

	/* Show help if insufficient args */
	اگر (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		باہر_نکلو(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	اگر (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		باہر_نکلو(EXIT_FAILURE);
	}

	اگر (!(service = (realpath(argv[1], تہی))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	اگر (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_تیاری(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	جب ((opt = getopt_long(argc, argv, getoptstring, longopts, تہی)) != -1)
		انتخاب (opt) {
		حالت 'd':
			setenv("RC_DEBUG", "YES", 1);
			توڑو;
		حالت 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			توڑو;
		حالت 's':
			اگر (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				باہر_نکلو(EXIT_FAILURE);
			توڑو;
		حالت 'S':
			اگر (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				باہر_نکلو(EXIT_FAILURE);
			توڑو;
		حالت 'D':
			deps = false;
			توڑو;
		حالت 'Z':
			dry_تسلسل = true;
			توڑو;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	اگر (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	اگر (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	اگر ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == تہی) {
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
		services = rc_services_اندر_runlevel(تہی);
		TAILQ_FOREACH(svc, services, entries) {
			ll = لڑی_لمبائی(svc->قدر);
			اگر (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = تہی;
		ll = لڑی_لمبائی(applet);
		اگر (ll > l)
			l = ll;

		/* Make our prefix string */
		سابقہ = xmalloc(حجم(حرف) * l + 1);
		حافظہ_نقل(سابقہ, applet, ll);
		memset(سابقہ + ll, ' ', l - ll);
		memset(سابقہ + l, 0, 1);
		eprefix(سابقہ);
	}

	اگر (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	اگر (deps && getenv("RC_PID") == تہی && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	اگر (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		اندر_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	اگر (rc_yesno(getenv("IN_DRYRUN")))
		dry_تسلسل = true;
	اگر (rc_yesno(getenv("IN_HOTPLUG"))) {
		اگر (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		اندر_background = true;
	}

	/* Setup a signal handler */
	signal_تیاری(SIGHUP, handle_signal);
	signal_تیاری(SIGALRM, handle_signal);
	signal_تیاری(SIGUSR1, handle_signal);
	signal_تیاری(SIGINT, handle_signal);
	signal_تیاری(SIGQUIT, handle_signal);
	signal_تیاری(SIGTERM, handle_signal);
	signal_تیاری(SIGCHLD, handle_signal);

	/* Load our plugins */
	rc_plugin_بھرو();

	applet_list = rc_stringlist_new();
	rc_stringlist_جوڑو(applet_list, applet);

	اگر (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

اگلا_cmd:
	جب (optind < argc) {
		صحیح cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		اگر (sighup)
			باہر_نکلو (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		برائے (size_t i = 0; i < ARRAY_سائز(commands); i++) {
			اگر (strcmp(optarg, commands[i].name) != 0)
				جاری;

			اگر ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			اگر (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = تہی;
			}

			جاؤ اگلا_cmd;
		}

		اگر ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = تہی;
	}

	واپس retval;
}
