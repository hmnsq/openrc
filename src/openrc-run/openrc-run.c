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

#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <fnmatch.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <libgen.h>
#அந்தர்பாவ <poll.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <spawn.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <sys/file.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <termios.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <inttypes.h>

#எனில் defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#அல்லதெனில் defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#இல்லையெனில்
#  include <libutil.h>
#endif

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "timeutils.h"
#அந்தர்பாவ "plugin.h"
#அந்தர்பாவ "selinux.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

வெளி எழுத்து **environ;

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = "stop | start | restart | status | describe | zap";
மாறிலி எழுத்து getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "debug",      0, வெற்று, 'd'},
	{ "dry-run",    0, வெற்று, 'Z'},
	{ "ifstarted",  0, வெற்று, 's'},
	{ "ifstopped",  0, வெற்று, 'S'},
	{ "nodeps",     0, வெற்று, 'D'},
	{ "lockfd",     1, வெற்று, 'l'},
	longopts_COMMON
};
மாறிலி எழுத்து *மாறிலி longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = வெற்று;

நிலையான எழுத்து *service, *runlevel, *ibsave, *prefix;
நிலையான RC_DEPTREE *deptree;
நிலையான RC_STRINGLIST *applet_list, *services, *tmplist;
நிலையான RC_STRINGLIST *restart_services;
நிலையான RC_STRINGLIST *need_services;
நிலையான RC_STRINGLIST *use_services;
நிலையான RC_STRINGLIST *want_services;
நிலையான முழு exclusive_fd = -1, master_tty = -1;
நிலையான bool in_background, deps, dry_run;
நிலையான நிலையற்ற bool sighup, skip_mark, timedout;
நிலையான pid_t service_pid;
நிலையான முழு signal_pipe[2] = { -1, -1 };

நிலையான RC_STRINGLIST *deptypes_b;	/* broken deps */
நிலையான RC_STRINGLIST *deptypes_n;	/* needed deps */
நிலையான RC_STRINGLIST *deptypes_nw;	/* need+want deps */
நிலையான RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
நிலையான RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
நிலையான RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
நிலையான RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

நிலையான வெறுமை
handle_signal(முழு sig)
{
	முழு serrno = errno, status;
	pid_t pid;
	மாறிலி எழுத்து *signame = வெற்று;
	கட்டமைப்பு winsize ws;

	தேர்வு (sig) {
	நிலை SIGHUP:
		sighup = true;
		நிறுத்து;

	நிலை SIGUSR1:
		skip_mark = true;
		நிறுத்து;

	நிலை SIGCHLD:
		போது ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			எனில் (signal_pipe[1] > -1 && pid == service_pid) {
				எனில் (லிக(signal_pipe[1], &status, அளவு(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		நிறுத்து;

	நிலை SIGWINCH:
		எனில் (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		நிறுத்து;

	நிலை SIGINT:
		எனில் (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	நிலை SIGTERM:
		எனில் (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	நிலை SIGQUIT:
		எனில் (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		எனில் (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	நிலை SIGALRM:
		timedout = true;
		நிறுத்து;

	இயல்பு:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

நிலையான வெறுமை
unhotplug(வெறுமை)
{
	எனில் (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

நிலையான வெறுமை
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	எனில் (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		திரும்பு;

	TAILQ_FOREACH(svc, list, entries) {
		எனில் (!(rc_service_state(svc->மதிப்பு) & RC_SERVICE_STOPPED))
			தொடர்;
		எனில் (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->மதிப்பு);
			ewarn("WARNING: %s will start when %s has started", svc->மதிப்பு, applet);
		} இல்லையெனில் {
			service_start(svc->மதிப்பு);
		}
	}
}

நிலையான வெறுமை
restore_state(வெறுமை)
{
	RC_SERVICE state;

	எனில் (rc_in_plugin || exclusive_fd == -1)
		திரும்பு;
	state = rc_service_state(applet);
	எனில் (state & RC_SERVICE_STOPPING) {
		எனில் (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		இல்லையெனில்
			rc_service_mark(applet, RC_SERVICE_STARTED);

		எனில் (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} இல்லையெனில் எனில் (state & RC_SERVICE_STARTING) {
		எனில் (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		இல்லையெனில்
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		எனில் (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

நிலையான வெறுமை
cleanup(வெறுமை)
{
	restore_state();

	எனில் (!rc_in_plugin) {
		எனில் (restart_services)
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
	விடுவி(ibsave);
	விடுவி(prefix);
	விடுவி(runlevel);
	விடுவி(service);
}

நிலையான வெறுமை
svc_getenv(மாறிலி எழுத்து *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	எழுத்து *var = வெற்று;

	TAILQ_FOREACH(reexport, reexports, entries) {
		எனில் (rc_service_getenv(svc, reexport->மதிப்பு, &var) == -1)
			தொடர்;
		setenv(reexport->மதிப்பு, var, true);
	}

	விடுவி(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
நிலையான முழு
write_prefix(மாறிலி எழுத்து *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	மாறிலி எழுத்து *ec = ecolor(ECOLOR_HILITE);
	மாறிலி எழுத்து *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	முழு fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	எனில் (lock_fd != -1) {
		போது (flock(lock_fd, LOCK_EX) != 0) {
			எனில் (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				நிறுத்து;
			}
		}
	}
	இல்லையெனில்
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	க்கு (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		எனில் (buffer[i] == '\033' && !*prefixed) {
			க்கு (j = i + 1; j < bytes; j++) {
				எனில் (buffer[j] == 'A')
					*prefixed = true;
				எனில் (isalpha((unsigned முழு)buffer[j]))
					நிறுத்து;
			}
		}

		எனில் (!*prefixed) {
			ret += லிக(fd, ec, strlen(ec));
			ret += லிக(fd, prefix, strlen(prefix));
			ret += லிக(fd, ec_normal, strlen(ec_normal));
			ret += லிக(fd, "|", 1);
			*prefixed = true;
		}

		எனில் (buffer[i] == '\n')
			*prefixed = false;
		ret += லிக(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	திரும்பு ret;
}

நிலையான முழு
svc_exec(மாறிலி எழுத்து *command)
{
	முழு ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	கட்டமைப்பு termios tt;
	கட்டமைப்பு winsize ws;
	முழு flags = 0;
	கட்டமைப்பு pollfd fd[2];
	bool prefixed = false;
	முழு slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	எழுத்து *openrc_sh = வெற்று;
	மாறிலி எழுத்து *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		வெற்று
	};

	/* Setup our signal pipe */
	எனில் (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		திரும்பு 1;
	}

	எனில் ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		திரும்பு 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	எனில் (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		எனில் (openpty(&master_tty, &slave_tty, வெற்று, &tt, &ws) == 0) {
			எனில் (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			எனில் (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			எனில் ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				திரும்பு 1;
			}
		}
	}

	எனில் (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	எனில் ((errno = posix_spawn(&service_pid, argv[0], &tty, வெற்று, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		திரும்பு 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	விடுவி(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	க்கு (;;) {
		எனில் (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			எனில் (errno == EINTR)
				தொடர்;
			eerror("%s: poll: %s", applet, strerror(errno));
			நிறுத்து;
		}

		எனில் (fd[1].revents & (POLLIN | POLLHUP)) {
			எழுத்து buffer[BUFSIZ];
			write_prefix(buffer, பட(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		எனில் (fd[0].revents & (POLLIN | POLLHUP)) {
			எனில் (errno = 0, பட(signal_pipe[0], &ret, அளவு(ret)) < (ssize_t)அளவு(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			இல்லையெனில் எனில் ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			நிறுத்து;
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

	sigprocmask (SIG_SETMASK, &oldmask, வெற்று);

	எனில் (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	திரும்பு ret;
}

நிலையான bool
svc_wait(மாறிலி எழுத்து *svc)
{
	முழு fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	மாறிலி எழுத்து *base = basename_c(svc);
	முழு timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	எனில் (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	எனில் ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		திரும்பு errno == ENOENT;

	timedout = false;
	க்கு (;;) {
		எனில் (!forever)
			alarm(WARN_TIMEOUT);

		எனில் (flock(fd, LOCK_SH) == 0) {
			retval = true;
			நிறுத்து;
		}

		எனில் (errno != EINTR)
			நிறுத்து;

		எனில் (!forever && timedout) {
			timedout = false;
			எனில் ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			இல்லையெனில்
				நிறுத்து;
		}
	}

	alarm(0);
	close(fd);
	திரும்பு retval;
}

நிலையான வெறுமை
get_started_services(வெறுமை)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	விடுவி(tmp);
}

நிலையான வெறுமை
setup_deptypes(வெறுமை)
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

நிலையான வெறுமை
svc_start_check(வெறுமை)
{
	முழு ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	எனில் (in_background) {
		எனில் (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			செல் exit;

		எனில் (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		எனில் (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	எனில் (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		செல் exit;
	} இல்லையெனில் எனில் (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	எனில் (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	எனில் (exclusive_fd == -1) {
		எனில் (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			செல் exit;
		}

		எனில் (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		இல்லையெனில்
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	திரும்பு;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

நிலையான வெறுமை
svc_start_deps(வெறுமை)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	முழு depoptions = RC_DEP_TRACE;
	size_t len, svc_மொத்தம்;
	எழுத்து *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	எனில் (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	எனில் (!deptree && ((deptree = _rc_deptree_load(0, வெற்று)) == வெற்று)) {
		eerror("failed to load deptree");
		செல் exit;
	}

	எனில் (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	எனில் (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			எனில் (first)
				first = false;
			இல்லையெனில்
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->மதிப்பு);
		}
		fprintf(stderr, "\n");
		செல் exit;
	}
	rc_stringlist_free(services);
	services = வெற்று;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	எனில் (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->மதிப்பு);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			எனில் (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				தொடர்;
			எனில் (state & RC_SERVICE_STOPPED) {
				எனில் (dry_run) {
					printf(" %s", svc->மதிப்பு);
					தொடர்;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->மதிப்பு);
				எனில் (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	எனில் (dry_run)
		திரும்பு;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->மதிப்பு);
		எனில் (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->மதிப்பு);
			தொடர்;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		எனில் (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			எனில் (!rc_stringlist_find(need_services, svc->மதிப்பு) &&
					!rc_stringlist_find(want_services, svc->மதிப்பு) &&
					!rc_stringlist_find(use_services, svc->மதிப்பு))
				தொடர்;
		}

		எனில் (!svc_wait(svc->மதிப்பு))
			eerror("%s: timed out waiting for %s", applet, svc->மதிப்பு);
		state = rc_service_state(svc->மதிப்பு);
		எனில் (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->மதிப்பு);
			தொடர்;
		}
		எனில் (rc_stringlist_find(need_services, svc->மதிப்பு)) {
			எனில் (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->மதிப்பு);
			} இல்லையெனில் எனில் (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->மதிப்பு);
				செல் exit;
			}
		}
	}

	எனில் (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = வெற்று;
		mem = xopen_memstream(&tmp, &len);
		svc_மொத்தம் = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->மதிப்பு, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->மதிப்பு);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->மதிப்பு, applet);
			rc_stringlist_free(use_services);
			use_services = வெற்று;
			எனில் (svc_மொத்தம் > 0)
				fputs(", ", mem);
			fputs(svc->மதிப்பு, mem);
			++svc_மொத்தம்;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = வெற்று;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_மொத்தம் > 1 ? "have" : "has");
		விடுவி(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = வெற்று;
	rc_stringlist_free(services);
	services = வெற்று;

	திரும்பு;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


நிலையான வெறுமை set_reexports(வெறுமை)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	எழுத்து *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->மதிப்பு);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

நிலையான வெறுமை svc_start_real(வெறுமை)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	எனில் (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	எனில் (ibsave)
		unsetenv("IN_BACKGROUND");

	எனில் (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	எனில் (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	இல்லையெனில் எனில் (!started)
		eerrorx("ERROR: %s failed to start", applet);

	எனில் (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    எனில் (rc_service_state(svc->மதிப்பு) & RC_SERVICE_STOPPED)
		    service_start(svc->மதிப்பு);
	rc_stringlist_free(services);
	services = வெற்று;

	/* Do the same for any services we provide */
	எனில் (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->மதிப்பு);
			TAILQ_FOREACH(svc2, services, entries)
				எனில் (rc_service_state(svc2->மதிப்பு) & RC_SERVICE_STOPPED)
					service_start(svc2->மதிப்பு);
			rc_stringlist_free(services);
			services = வெற்று;
		}
		rc_stringlist_free(tmplist);
		tmplist = வெற்று;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

நிலையான முழு
svc_start(வெறுமை)
{
	எனில் (dry_run)
		einfon("start:");
	இல்லையெனில்
		svc_start_check();

	எனில் (deps)
		svc_start_deps();

	எனில் (dry_run)
		printf(" %s\n", applet);
	இல்லையெனில்
		svc_start_real();

	திரும்பு 0;
}

நிலையான முழு
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	எனில் (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		செல் exit;

	எனில் (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		செல் exit;

	எனில் (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		திரும்பு 1;
	}

	எனில் (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	எனில் (exclusive_fd == -1) {
		எனில் (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		இல்லையெனில் எனில் (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		இல்லையெனில்
			eerror("ERROR: %s stopped by something else", applet);
		செல் exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	எனில் (!rc_runlevel_stopping()) {
		எனில் (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		இல்லையெனில் எனில் (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	திரும்பு 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

நிலையான வெறுமை
svc_stop_deps(RC_SERVICE state)
{
	முழு depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	எனில் (state & RC_SERVICE_WASINACTIVE)
		திரும்பு;

	errno = 0;
	எனில் (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	எனில் (!deptree && ((deptree = _rc_deptree_load(0, வெற்று)) == வெற்று)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	எனில் (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->மதிப்பு);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		எனில் (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			தொடர்;
		எனில் (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			தொடர்;
		எனில் (dry_run) {
			printf(" %s", svc->மதிப்பு);
			தொடர்;
		}
		svc_wait(svc->மதிப்பு);
		எனில் (rc_service_state(svc->மதிப்பு) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->மதிப்பு);
			எனில் (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->மதிப்பு);
		}
	}
	rc_stringlist_free(services);
	services = வெற்று;
	எனில் (dry_run)
		திரும்பு;

	TAILQ_FOREACH(svc, tmplist, entries) {
		எனில் (rc_service_state(svc->மதிப்பு) & RC_SERVICE_STOPPED) {
			தொடர்;
		}
		svc_wait(svc->மதிப்பு);
		எனில் (rc_service_state(svc->மதிப்பு) & RC_SERVICE_STOPPED)
			தொடர்;
		எனில் (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			எனில் (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				தொடர்;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->மதிப்பு);
	}
	rc_stringlist_free(tmplist);
	tmplist = வெற்று;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		எனில் (rc_service_state(svc->மதிப்பு) & RC_SERVICE_STOPPED)
			தொடர்;
		svc_wait(svc->மதிப்பு);
	}
	rc_stringlist_free(services);
	services = வெற்று;
}

நிலையான வெறுமை
load_dep_env(வெறுமை)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->மதிப்பு);

	rc_stringlist_free(depsvcs);
}

நிலையான வெறுமை
svc_stop_real(வெறுமை)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	எனில் (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	எனில் (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	எனில் (ibsave)
		unsetenv("IN_BACKGROUND");

	எனில் (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	எனில் (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

நிலையான முழு
svc_stop(வெறுமை)
{
	RC_SERVICE state = 0;

	எனில் (dry_run)
		einfon("stop:");
	இல்லையெனில்
		எனில் (svc_stop_check(&state) == 1)
			திரும்பு 1; /* Service has been stopped already */

	எனில் (deps)
		svc_stop_deps(state);

	எனில் (dry_run)
		printf(" %s\n", applet);
	இல்லையெனில்
		svc_stop_real();

	திரும்பு 0;
}

நிலையான முழு
svc_restart(வெறுமை)
{
	எனில் (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		எனில் (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = வெற்று;

	திரும்பு 0;
}

நிலையான bool
service_plugable(வெறுமை)
{
	எழுத்து *list, *p, *token;
	bool allow = true, truefalse;
	எழுத்து *match = rc_conf_மதிப்பு("rc_hotplug");

	எனில் (!match)
		match = rc_conf_மதிப்பு("rc_plug_services");
	எனில் (!match)
		திரும்பு false;

	list = xstrdup(match);
	p = list;
	போது ((token = strsep(&p, " "))) {
		எனில் (token[0] == '!') {
			truefalse = false;
			token++;
		} இல்லையெனில் {
			truefalse = true;
		}

		எனில் (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			நிறுத்து;
		}
	}
	விடுவி(list);
	திரும்பு allow;
}

நிலையான முழு
simple_command(வெறுமை)
{
	எழுத்து *save = prefix;
	முழு retval;

	eprefix(வெற்று);
	prefix = வெற்று;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	திரும்பு retval;
}

நிலையான முழு
resolve_deptype(வெறுமை)
{
	முழு depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	எனில் (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	எனில் (!deptree && ((deptree = _rc_deptree_load(0, வெற்று)) == வெற்று))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = வெற்று;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->மதிப்பு);
	printf ("\n");

	rc_stringlist_free(services);
	services = வெற்று;

	திரும்பு 0;
}

நிலையான முழு
cmd_condrestart(வெறுமை)
{
	எனில் (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	திரும்பு 0;
}

நிலையான முழு
cmd_stop(வெறுமை)
{
	RC_STRING *svc;

	எனில் (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	எனில் (deps && in_background)
		get_started_services();

	எனில் (svc_stop() == 1)
		திரும்பு 0; /* Service has been stopped already */

	எனில் (deps) {
		எனில் (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		எனில் (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				எனில் (rc_service_state(svc->மதிப்பு) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->மதிப்பு);
		}
	}

	திரும்பு 0;
}

நிலையான முழு
cmd_zap(வெறுமை)
{
	einfo("Manually resetting %s to stopped state", applet);
	எனில் (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	திரும்பு 0;
}

கட்டமைப்பு {
	மாறிலி எழுத்து *name;
	முழு (*handler)(வெறுமை);
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

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு retval, opt;
	மாறிலி எழுத்து *workingdir = "/";
	எழுத்து *pidstr = வெற்று;
	size_t l = 0, ll;
	கட்டமைப்பு stat stbuf;

	/* Show help if insufficient args */
	எனில் (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	எனில் (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	எனில் (!(service = (realpath(argv[1], வெற்று))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	எனில் (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	போது ((opt = getopt_long(argc, argv, getoptstring, longopts, வெற்று)) != -1)
		தேர்வு (opt) {
		நிலை 'd':
			setenv("RC_DEBUG", "YES", 1);
			நிறுத்து;
		நிலை 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			நிறுத்து;
		நிலை 's':
			எனில் (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			நிறுத்து;
		நிலை 'S':
			எனில் (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			நிறுத்து;
		நிலை 'D':
			deps = false;
			நிறுத்து;
		நிலை 'Z':
			dry_run = true;
			நிறுத்து;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	எனில் (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	எனில் (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	எனில் ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == வெற்று) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (முழு) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	விடுவி(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	எனில் (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(வெற்று);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->மதிப்பு);
			எனில் (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = வெற்று;
		ll = strlen(applet);
		எனில் (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(அளவு(எழுத்து) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	எனில் (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	எனில் (deps && getenv("RC_PID") == வெற்று && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	எனில் (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	எனில் (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	எனில் (rc_yesno(getenv("IN_HOTPLUG"))) {
		எனில் (!service_plugable())
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

	எனில் (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	போது (optind < argc) {
		முழு cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		எனில் (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		க்கு (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			எனில் (strcmp(optarg, commands[i].name) != 0)
				தொடர்;

			எனில் ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			எனில் (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = வெற்று;
			}

			செல் next_cmd;
		}

		எனில் ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = வெற்று;
	}

	திரும்பு retval;
}
