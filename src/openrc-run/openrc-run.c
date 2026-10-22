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

#ضمن <ctype.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <fnmatch.h>
#ضمن <getopt.h>
#ضمن <libgen.h>
#ضمن <poll.h>
#ضمن <signal.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <spawn.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/file.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <termios.h>
#ضمن <time.h>
#ضمن <unistd.h>
#ضمن <stdbool.h>
#ضمن <inttypes.h>

#إذا defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#وإلاإذا defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#وإلا
#  include <libutil.h>
#جے_انت

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"
#ضمن "misc.h"
#ضمن "timeutils.h"
#ضمن "plugin.h"
#ضمن "selinux.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

#تعریف_کرو WAIT_TIMEOUT	60		/* seconds until we timeout */
#تعریف_کرو WARN_TIMEOUT	10		/* warn about this every N seconds */

خارجي حرف **environ;

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = "stop | start | restart | status | describe | zap";
ثابت حرف getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "debug",      0, عدم, 'd'},
	{ "dry-run",    0, عدم, 'Z'},
	{ "ifstarted",  0, عدم, 's'},
	{ "ifstopped",  0, عدم, 'S'},
	{ "nodeps",     0, عدم, 'D'},
	{ "lockfd",     1, عدم, 'l'},
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
ثابت حرف *usagestring = عدم;

ساكن حرف *service, *runlevel, *ibsave, *اگیتر;
ساكن RC_DEPTREE *deptree;
ساكن RC_STRINGLIST *applet_list, *services, *tmplist;
ساكن RC_STRINGLIST *restart_services;
ساكن RC_STRINGLIST *need_services;
ساكن RC_STRINGLIST *use_services;
ساكن RC_STRINGLIST *want_services;
ساكن صحيح exclusive_fd = -1, master_tty = -1;
ساكن bool وچ_background, deps, dry_لگاتار;
ساكن متطاير bool sighup, skip_mark, timedout;
ساكن pid_t service_pid;
ساكن صحيح signal_pipe[2] = { -1, -1 };

ساكن RC_STRINGLIST *deptypes_b;	/* broken deps */
ساكن RC_STRINGLIST *deptypes_n;	/* needed deps */
ساكن RC_STRINGLIST *deptypes_nw;	/* need+want deps */
ساكن RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
ساكن RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
ساكن RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
ساكن RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

ساكن فراغ
handle_signal(صحيح sig)
{
	صحيح serrno = errno, حالت;
	pid_t pid;
	ثابت حرف *signame = عدم;
	بنية winsize ws;

	بدل (sig) {
	حالة SIGHUP:
		sighup = true;
		اكسر;

	حالة SIGUSR1:
		skip_mark = true;
		اكسر;

	حالة SIGCHLD:
		طالما ((pid = waitpid(-1, &حالت, WNOHANG)) > 0) {
			إذا (signal_pipe[1] > -1 && pid == service_pid) {
				إذا (اكتب(signal_pipe[1], &حالت, حجم(حالت)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		اكسر;

	حالة SIGWINCH:
		إذا (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		اكسر;

	حالة SIGINT:
		إذا (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالة SIGTERM:
		إذا (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالة SIGQUIT:
		إذا (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		إذا (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		باہر_نکلو(EXIT_FAILURE);
		/* NOTREACHED */

	حالة SIGALRM:
		timedout = true;
		اكسر;

	افتراضي:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ساكن فراغ
unhotplug(فراغ)
{
	إذا (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

ساكن فراغ
شروع_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	إذا (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		أرجع;

	TAILQ_FOREACH(svc, list, entries) {
		إذا (!(rc_service_state(svc->قدر) & RC_SERVICE_STOPPED))
			استمر;
		إذا (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_ویلا_سارنی_شروع(applet, svc->قدر);
			ewarn("WARNING: %s will start when %s has started", svc->قدر, applet);
		} وإلا {
			service_شروع(svc->قدر);
		}
	}
}

ساكن فراغ
restore_state(فراغ)
{
	RC_SERVICE state;

	إذا (rc_وچ_plugin || exclusive_fd == -1)
		أرجع;
	state = rc_service_state(applet);
	إذا (state & RC_SERVICE_STOPPING) {
		إذا (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		وإلا
			rc_service_mark(applet, RC_SERVICE_STARTED);

		إذا (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} وإلا إذا (state & RC_SERVICE_STARTING) {
		إذا (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		وإلا
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		إذا (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

ساكن فراغ
cleanup(فراغ)
{
	restore_state();

	إذا (!rc_وچ_plugin) {
		إذا (restart_services)
			شروع_services(restart_services);
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
	حرر(ibsave);
	حرر(اگیتر);
	حرر(runlevel);
	حرر(service);
}

ساكن فراغ
svc_getenv(ثابت حرف *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	حرف *var = عدم;

	TAILQ_FOREACH(reexport, reexports, entries) {
		إذا (rc_service_getenv(svc, reexport->قدر, &var) == -1)
			استمر;
		setenv(reexport->قدر, var, true);
	}

	حرر(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
ساكن صحيح
write_اگیتر(ثابت حرف *بفر, size_t بائٹاں, bool *prefixed)
{
	size_t i, j;
	ثابت حرف *ec = ecolor(ECOLOR_HILITE);
	ثابت حرف *ec_عام = ecolor(ECOLOR_عام);
	ssize_t ret = 0;
	صحيح fd = fileno(stdout), جندرا_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	جندرا_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	إذا (جندرا_fd != -1) {
		طالما (flock(جندرا_fd, LOCK_EX) != 0) {
			إذا (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				اكسر;
			}
		}
	}
	وإلا
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	لكل (i = 0; i < بائٹاں; i++) {
		/* We don't prefix eend calls (cursor up) */
		إذا (بفر[i] == '\033' && !*prefixed) {
			لكل (j = i + 1; j < بائٹاں; j++) {
				إذا (بفر[j] == 'A')
					*prefixed = true;
				إذا (حرف_اے((بنانشانی صحيح)بفر[j]))
					اكسر;
			}
		}

		إذا (!*prefixed) {
			ret += اكتب(fd, ec, لڑی_لمبائی(ec));
			ret += اكتب(fd, اگیتر, لڑی_لمبائی(اگیتر));
			ret += اكتب(fd, ec_عام, لڑی_لمبائی(ec_عام));
			ret += اكتب(fd, "|", 1);
			*prefixed = true;
		}

		إذا (بفر[i] == '\n')
			*prefixed = false;
		ret += اكتب(fd, بفر + i, 1);
	}

	/* Release the lock */
	close(جندرا_fd);

	أرجع ret;
}

ساكن صحيح
svc_exec(ثابت حرف *command)
{
	صحيح ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	بنية termios tt;
	بنية winsize ws;
	صحيح flags = 0;
	بنية pollfd fd[2];
	bool prefixed = false;
	صحيح slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	حرف *openrc_sh = عدم;
	ثابت حرف *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		عدم
	};

	/* Setup our signal pipe */
	إذا (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		أرجع 1;
	}

	إذا ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		أرجع 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	إذا (اگیتر && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		إذا (openpty(&master_tty, &slave_tty, عدم, &tt, &ws) == 0) {
			إذا (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			إذا (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			إذا ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				أرجع 1;
			}
		}
	}

	إذا (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	إذا ((errno = posix_spawn(&service_pid, argv[0], &tty, عدم, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		أرجع 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	حرر(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	لكل (;;) {
		إذا (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			إذا (errno == EINTR)
				استمر;
			eerror("%s: poll: %s", applet, strerror(errno));
			اكسر;
		}

		إذا (fd[1].revents & (POLLIN | POLLHUP)) {
			حرف بفر[BUFSIZ];
			write_اگیتر(بفر, اقرأ(master_tty, بفر, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		إذا (fd[0].revents & (POLLIN | POLLHUP)) {
			إذا (errno = 0, اقرأ(signal_pipe[0], &ret, حجم(ret)) < (ssize_t)حجم(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			وإلا إذا ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			اكسر;
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

	sigprocmask (SIG_SETMASK, &oldmask, عدم);

	إذا (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	أرجع ret;
}

ساكن bool
svc_wait(ثابت حرف *svc)
{
	صحيح fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ثابت حرف *ادھار = basename_c(svc);
	صحيح timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	إذا (rc_stringlist_لبھو(keywords, "-timeout") || rc_stringlist_لبھو(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	إذا ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), ادھار, O_RDONLY | O_NONBLOCK)) == -1)
		أرجع errno == ENOENT;

	timedout = false;
	لكل (;;) {
		إذا (!forever)
			alarm(WARN_TIMEOUT);

		إذا (flock(fd, LOCK_SH) == 0) {
			retval = true;
			اكسر;
		}

		إذا (errno != EINTR)
			اكسر;

		إذا (!forever && timedout) {
			timedout = false;
			إذا ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, ادھار, timeout);
			وإلا
				اكسر;
		}
	}

	alarm(0);
	close(fd);
	أرجع retval;
}

ساكن فراغ
get_started_services(فراغ)
{
	RC_STRINGLIST *tmp = rc_services_وچ_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_وچ_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	حرر(tmp);
}

ساكن فراغ
تیاری_deptypes(فراغ)
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

ساكن فراغ
svc_شروع_check(فراغ)
{
	صحيح ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	إذا (وچ_background) {
		إذا (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			اذهب باہر_نکلو;

		إذا (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		إذا (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	إذا (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		اذهب باہر_نکلو;
	} وإلا إذا (state & RC_SERVICE_INACTIVE && !وچ_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	إذا (exclusive_fd == -1)
		exclusive_fd = svc_جندرا(applet, !deps);
	إذا (exclusive_fd == -1) {
		إذا (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			اذهب باہر_نکلو;
		}

		إذا (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		وإلا
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_وچ, applet);

	أرجع;

باہر_نکلو:
	rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_باہرلا, applet);
	باہر_نکلو(ret);
}

ساكن فراغ
svc_شروع_deps(فراغ)
{
	bool پہلا;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	صحيح depoptions = RC_DEP_TRACE;
	size_t len, svc_گنتی;
	حرف *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	إذا (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	إذا (!deptree && ((deptree = rc_deptree_لدو_2(0, عدم)) == عدم)) {
		eerror("failed to load deptree");
		اذهب باہر_نکلو;
	}

	إذا (!deptypes_b)
		تیاری_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	إذا (TAILQ_پہلا(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		پہلا = true;
		TAILQ_FOREACH(svc, services, entries) {
			إذا (پہلا)
				پہلا = false;
			وإلا
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->قدر);
		}
		fprintf(stderr, "\n");
		اذهب باہر_نکلو;
	}
	rc_stringlist_free(services);
	services = عدم;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	إذا (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->قدر);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			إذا (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				استمر;
			إذا (state & RC_SERVICE_STOPPED) {
				إذا (dry_لگاتار) {
					لکھو(" %s", svc->قدر);
					استمر;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_شروع(svc->قدر);
				إذا (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	إذا (dry_لگاتار)
		أرجع;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->قدر);
		إذا (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->قدر);
			استمر;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		إذا (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			إذا (!rc_stringlist_لبھو(need_services, svc->قدر) &&
					!rc_stringlist_لبھو(want_services, svc->قدر) &&
					!rc_stringlist_لبھو(use_services, svc->قدر))
				استمر;
		}

		إذا (!svc_wait(svc->قدر))
			eerror("%s: timed out waiting for %s", applet, svc->قدر);
		state = rc_service_state(svc->قدر);
		إذا (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->قدر);
			استمر;
		}
		إذا (rc_stringlist_لبھو(need_services, svc->قدر)) {
			إذا (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_جوڑو(tmplist, svc->قدر);
			} وإلا إذا (!TAILQ_پہلا(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->قدر);
				اذهب باہر_نکلو;
			}
		}
	}

	إذا (TAILQ_پہلا(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = عدم;
		mem = xopen_memstream(&tmp, &len);
		svc_گنتی = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_ویلا_سارنی_شروع(svc->قدر, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->قدر);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_ویلا_سارنی_شروع(svc2->قدر, applet);
			rc_stringlist_free(use_services);
			use_services = عدم;
			إذا (svc_گنتی > 0)
				fputs(", ", mem);
			fputs(svc->قدر, mem);
			++svc_گنتی;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = عدم;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_گنتی > 1 ? "have" : "has");
		حرر(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = عدم;
	rc_stringlist_free(services);
	services = عدم;

	أرجع;

باہر_نکلو:
	rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_باہرلا, applet);
	باہر_نکلو(EXIT_FAILURE);
}


ساكن فراغ سیٹ_reexports(فراغ)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t ناپ;
	حرف *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &ناپ);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->قدر);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

ساكن فراغ svc_شروع_real(فراغ)
{
	bool started;
	RC_STRING *svc, *svc2;

	سیٹ_reexports();

	إذا (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	إذا (ibsave)
		unsetenv("IN_BACKGROUND");

	إذا (!started) {
		rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_پورا, applet);
		rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_باہرلا, applet);
	}

	إذا (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	وإلا إذا (!started)
		eerrorx("ERROR: %s failed to start", applet);

	إذا (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_پورا, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    إذا (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
		    service_شروع(svc->قدر);
	rc_stringlist_free(services);
	services = عدم;

	/* Do the same for any services we provide */
	إذا (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->قدر);
			TAILQ_FOREACH(svc2, services, entries)
				إذا (rc_service_state(svc2->قدر) & RC_SERVICE_STOPPED)
					service_شروع(svc2->قدر);
			rc_stringlist_free(services);
			services = عدم;
		}
		rc_stringlist_free(tmplist);
		tmplist = عدم;
	}

	rc_plugin_لگاتار(RC_HOOK_SERVICE_شروع_باہرلا, applet);
}

ساكن صحيح
svc_شروع(فراغ)
{
	إذا (dry_لگاتار)
		einfon("start:");
	وإلا
		svc_شروع_check();

	إذا (deps)
		svc_شروع_deps();

	إذا (dry_لگاتار)
		لکھو(" %s\n", applet);
	وإلا
		svc_شروع_real();

	أرجع 0;
}

ساكن صحيح
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	إذا (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		اذهب باہر_نکلو;

	إذا (وچ_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		اذهب باہر_نکلو;

	إذا (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		أرجع 1;
	}

	إذا (exclusive_fd == -1)
		exclusive_fd = svc_جندرا(applet, !deps);
	إذا (exclusive_fd == -1) {
		إذا (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		وإلا إذا (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		وإلا
			eerror("ERROR: %s stopped by something else", applet);
		اذهب باہر_نکلو;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_وچ, applet);

	إذا (!rc_runlevel_stopping()) {
		إذا (rc_service_وچ_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		وإلا إذا (rc_service_وچ_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	أرجع 0;

باہر_نکلو:
	rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_باہرلا, applet);
	باہر_نکلو(EXIT_FAILURE);
}

ساكن فراغ
svc_stop_deps(RC_SERVICE state)
{
	صحيح depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	إذا (state & RC_SERVICE_WASINACTIVE)
		أرجع;

	errno = 0;
	إذا (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	إذا (!deptree && ((deptree = rc_deptree_لدو_2(0, عدم)) == عدم)) {
		rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_باہرلا, applet);
		eerrorx("failed to load deptree");
	}

	إذا (!deptypes_m)
		تیاری_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_الٹ(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->قدر);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		إذا (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			استمر;
		إذا (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			استمر;
		إذا (dry_لگاتار) {
			لکھو(" %s", svc->قدر);
			استمر;
		}
		svc_wait(svc->قدر);
		إذا (rc_service_state(svc->قدر) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->قدر);
			إذا (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_جوڑو(tmplist, svc->قدر);
		}
	}
	rc_stringlist_free(services);
	services = عدم;
	إذا (dry_لگاتار)
		أرجع;

	TAILQ_FOREACH(svc, tmplist, entries) {
		إذا (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED) {
			استمر;
		}
		svc_wait(svc->قدر);
		إذا (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
			استمر;
		إذا (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			إذا (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				استمر;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_باہرلا, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->قدر);
	}
	rc_stringlist_free(tmplist);
	tmplist = عدم;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		إذا (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
			استمر;
		svc_wait(svc->قدر);
	}
	rc_stringlist_free(services);
	services = عدم;
}

ساكن فراغ
لدو_dep_env(فراغ)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->قدر);

	rc_stringlist_free(depsvcs);
}

ساكن فراغ
svc_stop_real(فراغ)
{
	bool stopped;

	لدو_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	إذا (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	إذا (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	إذا (ibsave)
		unsetenv("IN_BACKGROUND");

	إذا (stopped && !skip_mark)
		rc_service_mark(applet, وچ_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_پورا, applet);
	rc_plugin_لگاتار(RC_HOOK_SERVICE_STOP_باہرلا, applet);

	إذا (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

ساكن صحيح
svc_stop(فراغ)
{
	RC_SERVICE state = 0;

	إذا (dry_لگاتار)
		einfon("stop:");
	وإلا
		إذا (svc_stop_check(&state) == 1)
			أرجع 1; /* Service has been stopped already */

	إذا (deps)
		svc_stop_deps(state);

	إذا (dry_لگاتار)
		لکھو(" %s\n", applet);
	وإلا
		svc_stop_real();

	أرجع 0;
}

ساكن صحيح
svc_restart(فراغ)
{
	إذا (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		إذا (dry_لگاتار)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_شروع();
	شروع_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = عدم;

	أرجع 0;
}

ساكن bool
service_plugable(فراغ)
{
	حرف *list, *p, *token;
	bool allow = true, truefalse;
	حرف *میل = rc_conf_قدر("rc_hotplug");

	إذا (!میل)
		میل = rc_conf_قدر("rc_plug_services");
	إذا (!میل)
		أرجع false;

	list = xstrdup(میل);
	p = list;
	طالما ((token = strsep(&p, " "))) {
		إذا (token[0] == '!') {
			truefalse = false;
			token++;
		} وإلا {
			truefalse = true;
		}

		إذا (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			اكسر;
		}
	}
	حرر(list);
	أرجع allow;
}

ساكن صحيح
simple_command(فراغ)
{
	حرف *save = اگیتر;
	صحيح retval;

	eprefix(عدم);
	اگیتر = عدم;

	retval = svc_exec(optarg);

	eprefix(save);
	اگیتر = save;

	أرجع retval;
}

ساكن صحيح
resolve_deptype(فراغ)
{
	صحيح depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	إذا (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	إذا (!deptree && ((deptree = rc_deptree_لدو_2(0, عدم)) == عدم))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_جوڑو(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = عدم;
	TAILQ_FOREACH(svc, services, entries)
		لکھو("%s ", svc->قدر);
	لکھو ("\n");

	rc_stringlist_free(services);
	services = عدم;

	أرجع 0;
}

ساكن صحيح
cmd_condrestart(فراغ)
{
	إذا (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	أرجع 0;
}

ساكن صحيح
cmd_stop(فراغ)
{
	RC_STRING *svc;

	إذا (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	إذا (deps && وچ_background)
		get_started_services();

	إذا (svc_stop() == 1)
		أرجع 0; /* Service has been stopped already */

	إذا (deps) {
		إذا (!وچ_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		إذا (وچ_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				إذا (rc_service_state(svc->قدر) & RC_SERVICE_STOPPED)
					rc_service_ویلا_سارنی_شروع(applet, svc->قدر);
		}
	}

	أرجع 0;
}

ساكن صحيح
cmd_zap(فراغ)
{
	einfo("Manually resetting %s to stopped state", applet);
	إذا (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	أرجع 0;
}

بنية {
	ثابت حرف *name;
	صحيح (*handler)(فراغ);
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
	{ "start",              svc_شروع,       true  },
	{ "stop",               cmd_stop,        true  },
	{ "restart",            svc_restart,     true  },
	{ "condrestart",        cmd_condrestart, true  },
	{ "conditionalrestart", cmd_condrestart, true  },
	{ "zap",                cmd_zap,         true  },
};

صحيح مکھ(صحيح argc, حرف **argv)
{
	صحيح retval, opt;
	ثابت حرف *workingdir = "/";
	حرف *pidstr = عدم;
	size_t l = 0, ll;
	بنية stat stbuf;

	/* Show help if insufficient args */
	إذا (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		باہر_نکلو(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	إذا (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		باہر_نکلو(EXIT_FAILURE);
	}

	إذا (!(service = (realpath(argv[1], عدم))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	إذا (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_تیاری(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts, عدم)) != -1)
		بدل (opt) {
		حالة 'd':
			setenv("RC_DEBUG", "YES", 1);
			اكسر;
		حالة 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			اكسر;
		حالة 's':
			إذا (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				باہر_نکلو(EXIT_FAILURE);
			اكسر;
		حالة 'S':
			إذا (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				باہر_نکلو(EXIT_FAILURE);
			اكسر;
		حالة 'D':
			deps = false;
			اكسر;
		حالة 'Z':
			dry_لگاتار = true;
			اكسر;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	إذا (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	إذا (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	إذا ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == عدم) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (صحيح) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	حرر(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	إذا (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_وچ_runlevel(عدم);
		TAILQ_FOREACH(svc, services, entries) {
			ll = لڑی_لمبائی(svc->قدر);
			إذا (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = عدم;
		ll = لڑی_لمبائی(applet);
		إذا (ll > l)
			l = ll;

		/* Make our prefix string */
		اگیتر = xmalloc(حجم(حرف) * l + 1);
		حافظہ_نقل(اگیتر, applet, ll);
		memset(اگیتر + ll, ' ', l - ll);
		memset(اگیتر + l, 0, 1);
		eprefix(اگیتر);
	}

	إذا (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	إذا (deps && getenv("RC_PID") == عدم && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	إذا (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		وچ_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	إذا (rc_yesno(getenv("IN_DRYRUN")))
		dry_لگاتار = true;
	إذا (rc_yesno(getenv("IN_HOTPLUG"))) {
		إذا (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		وچ_background = true;
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
	rc_plugin_لدو();

	applet_list = rc_stringlist_new();
	rc_stringlist_جوڑو(applet_list, applet);

	إذا (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

اگلا_cmd:
	طالما (optind < argc) {
		صحيح cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		إذا (sighup)
			باہر_نکلو (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		لكل (size_t i = 0; i < ARRAY_ناپ(commands); i++) {
			إذا (strcmp(optarg, commands[i].name) != 0)
				استمر;

			إذا ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			إذا (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = عدم;
			}

			اذهب اگلا_cmd;
		}

		إذا ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = عدم;
	}

	أرجع retval;
}
