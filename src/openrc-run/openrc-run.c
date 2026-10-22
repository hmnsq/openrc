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

#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <fnmatch.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <libgen.h>
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <spawn.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <sys/file.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <termios.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <inttypes.h>

#જો defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#અથવાજો defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#નહીંતર
#  include <libutil.h>
#endif

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "timeutils.h"
#અન્તર્ભાવ "plugin.h"
#અન્તર્ભાવ "selinux.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

બાહ્ય અક્ષર **environ;

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = "stop | start | restart | status | describe | zap";
અચલ અક્ષર getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "debug",      0, ખાલી, 'd'},
	{ "dry-run",    0, ખાલી, 'Z'},
	{ "ifstarted",  0, ખાલી, 's'},
	{ "ifstopped",  0, ખાલી, 'S'},
	{ "nodeps",     0, ખાલી, 'D'},
	{ "lockfd",     1, ખાલી, 'l'},
	longopts_COMMON
};
અચલ અક્ષર *અચલ longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
અચલ અક્ષર *usagestring = ખાલી;

સ્થિર અક્ષર *service, *runlevel, *ibsave, *prefix;
સ્થિર RC_DEPTREE *deptree;
સ્થિર RC_STRINGLIST *applet_list, *services, *tmplist;
સ્થિર RC_STRINGLIST *restart_services;
સ્થિર RC_STRINGLIST *need_services;
સ્થિર RC_STRINGLIST *use_services;
સ્થિર RC_STRINGLIST *want_services;
સ્થિર પૂર્ણાંક exclusive_fd = -1, master_tty = -1;
સ્થિર bool in_background, deps, dry_run;
સ્થિર અસ્થિર bool sighup, skip_mark, timedout;
સ્થિર pid_t service_pid;
સ્થિર પૂર્ણાંક signal_pipe[2] = { -1, -1 };

સ્થિર RC_STRINGLIST *deptypes_b;	/* broken deps */
સ્થિર RC_STRINGLIST *deptypes_n;	/* needed deps */
સ્થિર RC_STRINGLIST *deptypes_nw;	/* need+want deps */
સ્થિર RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
સ્થિર RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
સ્થિર RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
સ્થિર RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

સ્થિર શૂન્ય
handle_signal(પૂર્ણાંક sig)
{
	પૂર્ણાંક serrno = errno, status;
	pid_t pid;
	અચલ અક્ષર *signame = ખાલી;
	સંરચના winsize ws;

	પસંદગી (sig) {
	સ્થિતિ SIGHUP:
		sighup = true;
		તોડો;

	સ્થિતિ SIGUSR1:
		skip_mark = true;
		તોડો;

	સ્થિતિ SIGCHLD:
		જ્યારે ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			જો (signal_pipe[1] > -1 && pid == service_pid) {
				જો (લિખ(signal_pipe[1], &status, કદ(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		તોડો;

	સ્થિતિ SIGWINCH:
		જો (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		તોડો;

	સ્થિતિ SIGINT:
		જો (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	સ્થિતિ SIGTERM:
		જો (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	સ્થિતિ SIGQUIT:
		જો (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		જો (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	સ્થિતિ SIGALRM:
		timedout = true;
		તોડો;

	મૂળભૂત:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

સ્થિર શૂન્ય
unhotplug(શૂન્ય)
{
	જો (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

સ્થિર શૂન્ય
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	જો (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		પાછા;

	TAILQ_FOREACH(svc, list, entries) {
		જો (!(rc_service_state(svc->કિંમત) & RC_SERVICE_STOPPED))
			ચાલુ;
		જો (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->કિંમત);
			ewarn("WARNING: %s will start when %s has started", svc->કિંમત, applet);
		} નહીંતર {
			service_start(svc->કિંમત);
		}
	}
}

સ્થિર શૂન્ય
restore_state(શૂન્ય)
{
	RC_SERVICE state;

	જો (rc_in_plugin || exclusive_fd == -1)
		પાછા;
	state = rc_service_state(applet);
	જો (state & RC_SERVICE_STOPPING) {
		જો (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		નહીંતર
			rc_service_mark(applet, RC_SERVICE_STARTED);

		જો (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} નહીંતર જો (state & RC_SERVICE_STARTING) {
		જો (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		નહીંતર
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		જો (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

સ્થિર શૂન્ય
cleanup(શૂન્ય)
{
	restore_state();

	જો (!rc_in_plugin) {
		જો (restart_services)
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
	મુક્ત(ibsave);
	મુક્ત(prefix);
	મુક્ત(runlevel);
	મુક્ત(service);
}

સ્થિર શૂન્ય
svc_getenv(અચલ અક્ષર *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	અક્ષર *var = ખાલી;

	TAILQ_FOREACH(reexport, reexports, entries) {
		જો (rc_service_getenv(svc, reexport->કિંમત, &var) == -1)
			ચાલુ;
		setenv(reexport->કિંમત, var, true);
	}

	મુક્ત(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
સ્થિર પૂર્ણાંક
write_prefix(અચલ અક્ષર *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	અચલ અક્ષર *ec = ecolor(ECOLOR_HILITE);
	અચલ અક્ષર *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	પૂર્ણાંક fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	જો (lock_fd != -1) {
		જ્યારે (flock(lock_fd, LOCK_EX) != 0) {
			જો (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				તોડો;
			}
		}
	}
	નહીંતર
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	માટે (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		જો (buffer[i] == '\033' && !*prefixed) {
			માટે (j = i + 1; j < bytes; j++) {
				જો (buffer[j] == 'A')
					*prefixed = true;
				જો (isalpha((unsigned પૂર્ણાંક)buffer[j]))
					તોડો;
			}
		}

		જો (!*prefixed) {
			ret += લિખ(fd, ec, strlen(ec));
			ret += લિખ(fd, prefix, strlen(prefix));
			ret += લિખ(fd, ec_normal, strlen(ec_normal));
			ret += લિખ(fd, "|", 1);
			*prefixed = true;
		}

		જો (buffer[i] == '\n')
			*prefixed = false;
		ret += લિખ(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	પાછા ret;
}

સ્થિર પૂર્ણાંક
svc_exec(અચલ અક્ષર *command)
{
	પૂર્ણાંક ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	સંરચના termios tt;
	સંરચના winsize ws;
	પૂર્ણાંક flags = 0;
	સંરચના pollfd fd[2];
	bool prefixed = false;
	પૂર્ણાંક slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	અક્ષર *openrc_sh = ખાલી;
	અચલ અક્ષર *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		ખાલી
	};

	/* Setup our signal pipe */
	જો (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		પાછા 1;
	}

	જો ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		પાછા 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	જો (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		જો (openpty(&master_tty, &slave_tty, ખાલી, &tt, &ws) == 0) {
			જો (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			જો (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			જો ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				પાછા 1;
			}
		}
	}

	જો (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	જો ((errno = posix_spawn(&service_pid, argv[0], &tty, ખાલી, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		પાછા 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	મુક્ત(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	માટે (;;) {
		જો (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			જો (errno == EINTR)
				ચાલુ;
			eerror("%s: poll: %s", applet, strerror(errno));
			તોડો;
		}

		જો (fd[1].revents & (POLLIN | POLLHUP)) {
			અક્ષર buffer[BUFSIZ];
			write_prefix(buffer, પઠ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		જો (fd[0].revents & (POLLIN | POLLHUP)) {
			જો (errno = 0, પઠ(signal_pipe[0], &ret, કદ(ret)) < (ssize_t)કદ(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			નહીંતર જો ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			તોડો;
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

	sigprocmask (SIG_SETMASK, &oldmask, ખાલી);

	જો (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	પાછા ret;
}

સ્થિર bool
svc_wait(અચલ અક્ષર *svc)
{
	પૂર્ણાંક fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	અચલ અક્ષર *base = basename_c(svc);
	પૂર્ણાંક timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	જો (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	જો ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		પાછા errno == ENOENT;

	timedout = false;
	માટે (;;) {
		જો (!forever)
			alarm(WARN_TIMEOUT);

		જો (flock(fd, LOCK_SH) == 0) {
			retval = true;
			તોડો;
		}

		જો (errno != EINTR)
			તોડો;

		જો (!forever && timedout) {
			timedout = false;
			જો ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			નહીંતર
				તોડો;
		}
	}

	alarm(0);
	close(fd);
	પાછા retval;
}

સ્થિર શૂન્ય
get_started_services(શૂન્ય)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	મુક્ત(tmp);
}

સ્થિર શૂન્ય
setup_deptypes(શૂન્ય)
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

સ્થિર શૂન્ય
svc_start_check(શૂન્ય)
{
	પૂર્ણાંક ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	જો (in_background) {
		જો (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			જાઓ exit;

		જો (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		જો (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	જો (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		જાઓ exit;
	} નહીંતર જો (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	જો (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	જો (exclusive_fd == -1) {
		જો (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			જાઓ exit;
		}

		જો (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		નહીંતર
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	પાછા;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

સ્થિર શૂન્ય
svc_start_deps(શૂન્ય)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	પૂર્ણાંક depoptions = RC_DEP_TRACE;
	size_t len, svc_ગણતરી;
	અક્ષર *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	જો (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	જો (!deptree && ((deptree = _rc_deptree_load(0, ખાલી)) == ખાલી)) {
		eerror("failed to load deptree");
		જાઓ exit;
	}

	જો (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	જો (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			જો (first)
				first = false;
			નહીંતર
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->કિંમત);
		}
		fprintf(stderr, "\n");
		જાઓ exit;
	}
	rc_stringlist_free(services);
	services = ખાલી;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	જો (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->કિંમત);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			જો (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				ચાલુ;
			જો (state & RC_SERVICE_STOPPED) {
				જો (dry_run) {
					printf(" %s", svc->કિંમત);
					ચાલુ;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->કિંમત);
				જો (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	જો (dry_run)
		પાછા;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->કિંમત);
		જો (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->કિંમત);
			ચાલુ;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		જો (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			જો (!rc_stringlist_find(need_services, svc->કિંમત) &&
					!rc_stringlist_find(want_services, svc->કિંમત) &&
					!rc_stringlist_find(use_services, svc->કિંમત))
				ચાલુ;
		}

		જો (!svc_wait(svc->કિંમત))
			eerror("%s: timed out waiting for %s", applet, svc->કિંમત);
		state = rc_service_state(svc->કિંમત);
		જો (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->કિંમત);
			ચાલુ;
		}
		જો (rc_stringlist_find(need_services, svc->કિંમત)) {
			જો (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->કિંમત);
			} નહીંતર જો (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->કિંમત);
				જાઓ exit;
			}
		}
	}

	જો (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = ખાલી;
		mem = xopen_memstream(&tmp, &len);
		svc_ગણતરી = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->કિંમત, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->કિંમત);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->કિંમત, applet);
			rc_stringlist_free(use_services);
			use_services = ખાલી;
			જો (svc_ગણતરી > 0)
				fputs(", ", mem);
			fputs(svc->કિંમત, mem);
			++svc_ગણતરી;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = ખાલી;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_ગણતરી > 1 ? "have" : "has");
		મુક્ત(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = ખાલી;
	rc_stringlist_free(services);
	services = ખાલી;

	પાછા;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


સ્થિર શૂન્ય set_reexports(શૂન્ય)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	અક્ષર *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->કિંમત);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

સ્થિર શૂન્ય svc_start_real(શૂન્ય)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	જો (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	જો (ibsave)
		unsetenv("IN_BACKGROUND");

	જો (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	જો (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	નહીંતર જો (!started)
		eerrorx("ERROR: %s failed to start", applet);

	જો (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    જો (rc_service_state(svc->કિંમત) & RC_SERVICE_STOPPED)
		    service_start(svc->કિંમત);
	rc_stringlist_free(services);
	services = ખાલી;

	/* Do the same for any services we provide */
	જો (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->કિંમત);
			TAILQ_FOREACH(svc2, services, entries)
				જો (rc_service_state(svc2->કિંમત) & RC_SERVICE_STOPPED)
					service_start(svc2->કિંમત);
			rc_stringlist_free(services);
			services = ખાલી;
		}
		rc_stringlist_free(tmplist);
		tmplist = ખાલી;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

સ્થિર પૂર્ણાંક
svc_start(શૂન્ય)
{
	જો (dry_run)
		einfon("start:");
	નહીંતર
		svc_start_check();

	જો (deps)
		svc_start_deps();

	જો (dry_run)
		printf(" %s\n", applet);
	નહીંતર
		svc_start_real();

	પાછા 0;
}

સ્થિર પૂર્ણાંક
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	જો (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		જાઓ exit;

	જો (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		જાઓ exit;

	જો (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		પાછા 1;
	}

	જો (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	જો (exclusive_fd == -1) {
		જો (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		નહીંતર જો (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		નહીંતર
			eerror("ERROR: %s stopped by something else", applet);
		જાઓ exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	જો (!rc_runlevel_stopping()) {
		જો (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		નહીંતર જો (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	પાછા 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

સ્થિર શૂન્ય
svc_stop_deps(RC_SERVICE state)
{
	પૂર્ણાંક depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	જો (state & RC_SERVICE_WASINACTIVE)
		પાછા;

	errno = 0;
	જો (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	જો (!deptree && ((deptree = _rc_deptree_load(0, ખાલી)) == ખાલી)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	જો (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->કિંમત);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		જો (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			ચાલુ;
		જો (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			ચાલુ;
		જો (dry_run) {
			printf(" %s", svc->કિંમત);
			ચાલુ;
		}
		svc_wait(svc->કિંમત);
		જો (rc_service_state(svc->કિંમત) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->કિંમત);
			જો (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->કિંમત);
		}
	}
	rc_stringlist_free(services);
	services = ખાલી;
	જો (dry_run)
		પાછા;

	TAILQ_FOREACH(svc, tmplist, entries) {
		જો (rc_service_state(svc->કિંમત) & RC_SERVICE_STOPPED) {
			ચાલુ;
		}
		svc_wait(svc->કિંમત);
		જો (rc_service_state(svc->કિંમત) & RC_SERVICE_STOPPED)
			ચાલુ;
		જો (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			જો (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				ચાલુ;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->કિંમત);
	}
	rc_stringlist_free(tmplist);
	tmplist = ખાલી;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		જો (rc_service_state(svc->કિંમત) & RC_SERVICE_STOPPED)
			ચાલુ;
		svc_wait(svc->કિંમત);
	}
	rc_stringlist_free(services);
	services = ખાલી;
}

સ્થિર શૂન્ય
load_dep_env(શૂન્ય)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->કિંમત);

	rc_stringlist_free(depsvcs);
}

સ્થિર શૂન્ય
svc_stop_real(શૂન્ય)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	જો (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	જો (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	જો (ibsave)
		unsetenv("IN_BACKGROUND");

	જો (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	જો (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

સ્થિર પૂર્ણાંક
svc_stop(શૂન્ય)
{
	RC_SERVICE state = 0;

	જો (dry_run)
		einfon("stop:");
	નહીંતર
		જો (svc_stop_check(&state) == 1)
			પાછા 1; /* Service has been stopped already */

	જો (deps)
		svc_stop_deps(state);

	જો (dry_run)
		printf(" %s\n", applet);
	નહીંતર
		svc_stop_real();

	પાછા 0;
}

સ્થિર પૂર્ણાંક
svc_restart(શૂન્ય)
{
	જો (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		જો (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = ખાલી;

	પાછા 0;
}

સ્થિર bool
service_plugable(શૂન્ય)
{
	અક્ષર *list, *p, *token;
	bool allow = true, truefalse;
	અક્ષર *match = rc_conf_કિંમત("rc_hotplug");

	જો (!match)
		match = rc_conf_કિંમત("rc_plug_services");
	જો (!match)
		પાછા false;

	list = xstrdup(match);
	p = list;
	જ્યારે ((token = strsep(&p, " "))) {
		જો (token[0] == '!') {
			truefalse = false;
			token++;
		} નહીંતર {
			truefalse = true;
		}

		જો (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			તોડો;
		}
	}
	મુક્ત(list);
	પાછા allow;
}

સ્થિર પૂર્ણાંક
simple_command(શૂન્ય)
{
	અક્ષર *save = prefix;
	પૂર્ણાંક retval;

	eprefix(ખાલી);
	prefix = ખાલી;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	પાછા retval;
}

સ્થિર પૂર્ણાંક
resolve_deptype(શૂન્ય)
{
	પૂર્ણાંક depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	જો (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	જો (!deptree && ((deptree = _rc_deptree_load(0, ખાલી)) == ખાલી))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = ખાલી;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->કિંમત);
	printf ("\n");

	rc_stringlist_free(services);
	services = ખાલી;

	પાછા 0;
}

સ્થિર પૂર્ણાંક
cmd_condrestart(શૂન્ય)
{
	જો (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	પાછા 0;
}

સ્થિર પૂર્ણાંક
cmd_stop(શૂન્ય)
{
	RC_STRING *svc;

	જો (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	જો (deps && in_background)
		get_started_services();

	જો (svc_stop() == 1)
		પાછા 0; /* Service has been stopped already */

	જો (deps) {
		જો (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		જો (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				જો (rc_service_state(svc->કિંમત) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->કિંમત);
		}
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક
cmd_zap(શૂન્ય)
{
	einfo("Manually resetting %s to stopped state", applet);
	જો (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	પાછા 0;
}

સંરચના {
	અચલ અક્ષર *name;
	પૂર્ણાંક (*handler)(શૂન્ય);
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

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક retval, opt;
	અચલ અક્ષર *workingdir = "/";
	અક્ષર *pidstr = ખાલી;
	size_t l = 0, ll;
	સંરચના stat stbuf;

	/* Show help if insufficient args */
	જો (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	જો (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	જો (!(service = (realpath(argv[1], ખાલી))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	જો (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring, longopts, ખાલી)) != -1)
		પસંદગી (opt) {
		સ્થિતિ 'd':
			setenv("RC_DEBUG", "YES", 1);
			તોડો;
		સ્થિતિ 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			તોડો;
		સ્થિતિ 's':
			જો (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			તોડો;
		સ્થિતિ 'S':
			જો (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			તોડો;
		સ્થિતિ 'D':
			deps = false;
			તોડો;
		સ્થિતિ 'Z':
			dry_run = true;
			તોડો;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	જો (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	જો (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	જો ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == ખાલી) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (પૂર્ણાંક) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	મુક્ત(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	જો (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(ખાલી);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->કિંમત);
			જો (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = ખાલી;
		ll = strlen(applet);
		જો (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(કદ(અક્ષર) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	જો (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	જો (deps && getenv("RC_PID") == ખાલી && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	જો (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	જો (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	જો (rc_yesno(getenv("IN_HOTPLUG"))) {
		જો (!service_plugable())
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

	જો (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	જ્યારે (optind < argc) {
		પૂર્ણાંક cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		જો (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		માટે (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			જો (strcmp(optarg, commands[i].name) != 0)
				ચાલુ;

			જો ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			જો (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = ખાલી;
			}

			જાઓ next_cmd;
		}

		જો ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = ખાલી;
	}

	પાછા retval;
}
