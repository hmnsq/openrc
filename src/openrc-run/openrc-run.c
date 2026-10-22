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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <fnmatch.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <libgen.h>
#अन्तर्भाव <poll.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <spawn.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <termios.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <inttypes.h>

#सचे defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#नोचेद्यदि defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#अञ्ञथा
#  include <libutil.h>
#endif

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "timeutils.h"
#अन्तर्भाव "plugin.h"
#अन्तर्भाव "selinux.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

#define WAIT_TIMEOUT	60		/* seconds until we timeout */
#define WARN_TIMEOUT	10		/* warn about this every N seconds */

बाह्य वर्ण **environ;

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = "stop | start | restart | status | describe | zap";
ध्रुव वर्ण getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "debug",      0, रिक्त, 'd'},
	{ "dry-run",    0, रिक्त, 'Z'},
	{ "ifstarted",  0, रिक्त, 's'},
	{ "ifstopped",  0, रिक्त, 'S'},
	{ "nodeps",     0, रिक्त, 'D'},
	{ "lockfd",     1, रिक्त, 'l'},
	longopts_COMMON
};
ध्रुव वर्ण *ध्रुव longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
ध्रुव वर्ण *usagestring = रिक्त;

स्थिर वर्ण *service, *runlevel, *ibsave, *prefix;
स्थिर RC_DEPTREE *deptree;
स्थिर RC_STRINGLIST *applet_list, *services, *tmplist;
स्थिर RC_STRINGLIST *restart_services;
स्थिर RC_STRINGLIST *need_services;
स्थिर RC_STRINGLIST *use_services;
स्थिर RC_STRINGLIST *want_services;
स्थिर पूर्णाङ्क exclusive_fd = -1, master_tty = -1;
स्थिर bool in_background, deps, dry_run;
स्थिर चञ्चल bool sighup, skip_mark, timedout;
स्थिर pid_t service_pid;
स्थिर पूर्णाङ्क signal_pipe[2] = { -1, -1 };

स्थिर RC_STRINGLIST *deptypes_b;	/* broken deps */
स्थिर RC_STRINGLIST *deptypes_n;	/* needed deps */
स्थिर RC_STRINGLIST *deptypes_nw;	/* need+want deps */
स्थिर RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
स्थिर RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
स्थिर RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
स्थिर RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

स्थिर शून्य
handle_signal(पूर्णाङ्क sig)
{
	पूर्णाङ्क serrno = errno, status;
	pid_t pid;
	ध्रुव वर्ण *signame = रिक्त;
	संरचना winsize ws;

	विकल्प (sig) {
	पक्ष SIGHUP:
		sighup = true;
		भङ्ग;

	पक्ष SIGUSR1:
		skip_mark = true;
		भङ्ग;

	पक्ष SIGCHLD:
		याव ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			सचे (signal_pipe[1] > -1 && pid == service_pid) {
				सचे (लिख(signal_pipe[1], &status, परिमाण(status)) == -1)
					eerror("%s: send: %s", applet, strerror(errno));
			}
		}
		भङ्ग;

	पक्ष SIGWINCH:
		सचे (master_tty >= 0) {
			ioctl(fileno(stdout), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		भङ्ग;

	पक्ष SIGINT:
		सचे (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	पक्ष SIGTERM:
		सचे (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	पक्ष SIGQUIT:
		सचे (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		सचे (service_pid > 0)
			kill(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		exit(EXIT_FAILURE);
		/* NOTREACHED */

	पक्ष SIGALRM:
		timedout = true;
		भङ्ग;

	स्वभाव:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

स्थिर शून्य
unhotplug(शून्य)
{
	सचे (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && errno != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, strerror(errno));
}

स्थिर शून्य
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	सचे (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		पटिनिवत्त;

	TAILQ_FOREACH(svc, list, entries) {
		सचे (!(rc_service_state(svc->मानम्) & RC_SERVICE_STOPPED))
			अनुवर्त;
		सचे (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->मानम्);
			ewarn("WARNING: %s will start when %s has started", svc->मानम्, applet);
		} अञ्ञथा {
			service_start(svc->मानम्);
		}
	}
}

स्थिर शून्य
restore_state(शून्य)
{
	RC_SERVICE state;

	सचे (rc_in_plugin || exclusive_fd == -1)
		पटिनिवत्त;
	state = rc_service_state(applet);
	सचे (state & RC_SERVICE_STOPPING) {
		सचे (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		अञ्ञथा
			rc_service_mark(applet, RC_SERVICE_STARTED);

		सचे (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} अञ्ञथा सचे (state & RC_SERVICE_STARTING) {
		सचे (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		अञ्ञथा
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		सचे (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

स्थिर शून्य
cleanup(शून्य)
{
	restore_state();

	सचे (!rc_in_plugin) {
		सचे (restart_services)
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
	मुक्त(ibsave);
	मुक्त(prefix);
	मुक्त(runlevel);
	मुक्त(service);
}

स्थिर शून्य
svc_getenv(ध्रुव वर्ण *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	वर्ण *var = रिक्त;

	TAILQ_FOREACH(reexport, reexports, entries) {
		सचे (rc_service_getenv(svc, reexport->मानम्, &var) == -1)
			अनुवर्त;
		setenv(reexport->मानम्, var, true);
	}

	मुक्त(var);
	rc_stringlist_free(reexports);
}

/* Buffer and lock all output messages so that we get readable content */
/* FIXME: Use a dynamic lock file that contains the tty/pts as well.
 * For example openrc-pts8.lock or openrc-tty1.lock.
 * Using a static lock file makes no sense, esp. in multi-user environments.
 * Why don't we use (f)printf, as it is thread-safe through POSIX already?
 * Bug: 360013
 */
स्थिर पूर्णाङ्क
write_prefix(ध्रुव वर्ण *buffer, size_t bytes, bool *prefixed)
{
	size_t i, j;
	ध्रुव वर्ण *ec = ecolor(ECOLOR_HILITE);
	ध्रुव वर्ण *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	पूर्णाङ्क fd = fileno(stdout), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	सचे (lock_fd != -1) {
		याव (flock(lock_fd, LOCK_EX) != 0) {
			सचे (errno != EINTR) {
				ewarnv("flock() failed: %s", strerror(errno));
				भङ्ग;
			}
		}
	}
	अञ्ञथा
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	अत्थाय (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		सचे (buffer[i] == '\033' && !*prefixed) {
			अत्थाय (j = i + 1; j < bytes; j++) {
				सचे (buffer[j] == 'A')
					*prefixed = true;
				सचे (isalpha((unsigned पूर्णाङ्क)buffer[j]))
					भङ्ग;
			}
		}

		सचे (!*prefixed) {
			ret += लिख(fd, ec, strlen(ec));
			ret += लिख(fd, prefix, strlen(prefix));
			ret += लिख(fd, ec_normal, strlen(ec_normal));
			ret += लिख(fd, "|", 1);
			*prefixed = true;
		}

		सचे (buffer[i] == '\n')
			*prefixed = false;
		ret += लिख(fd, buffer + i, 1);
	}

	/* Release the lock */
	close(lock_fd);

	पटिनिवत्त ret;
}

स्थिर पूर्णाङ्क
svc_exec(ध्रुव वर्ण *command)
{
	पूर्णाङ्क ret = -1, fdout = fileno(stdout), tmp;
	posix_spawn_file_actions_t tty;
	संरचना termios tt;
	संरचना winsize ws;
	पूर्णाङ्क flags = 0;
	संरचना pollfd fd[2];
	bool prefixed = false;
	पूर्णाङ्क slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	वर्ण *openrc_sh = रिक्त;
	ध्रुव वर्ण *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		रिक्त
	};

	/* Setup our signal pipe */
	सचे (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		पटिनिवत्त 1;
	}

	सचे ((errno = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, strerror(errno));
		पटिनिवत्त 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	सचे (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		सचे (openpty(&master_tty, &slave_tty, रिक्त, &tt, &ws) == 0) {
			सचे (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			सचे (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			सचे ((errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (errno = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, strerror(errno));
				पटिनिवत्त 1;
			}
		}
	}

	सचे (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	सचे ((errno = posix_spawn(&service_pid, argv[0], &tty, रिक्त, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], strerror(errno));
		पटिनिवत्त 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	मुक्त(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	अत्थाय (;;) {
		सचे (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			सचे (errno == EINTR)
				अनुवर्त;
			eerror("%s: poll: %s", applet, strerror(errno));
			भङ्ग;
		}

		सचे (fd[1].revents & (POLLIN | POLLHUP)) {
			वर्ण buffer[BUFSIZ];
			write_prefix(buffer, पठ(master_tty, buffer, BUFSIZ), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		सचे (fd[0].revents & (POLLIN | POLLHUP)) {
			सचे (errno = 0, पठ(signal_pipe[0], &ret, परिमाण(ret)) < (ssize_t)परिमाण(ret))
				eerror("%s: receive failed: %s", applet, errno ? strerror(errno) : "short read");
			अञ्ञथा सचे ((ret = WEXITSTATUS(ret)) != 0 && errno == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			भङ्ग;
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

	sigprocmask (SIG_SETMASK, &oldmask, रिक्त);

	सचे (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		close(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	पटिनिवत्त ret;
}

स्थिर bool
svc_wait(ध्रुव वर्ण *svc)
{
	पूर्णाङ्क fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	ध्रुव वर्ण *base = basename_c(svc);
	पूर्णाङ्क timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	सचे (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	सचे ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		पटिनिवत्त errno == ENOENT;

	timedout = false;
	अत्थाय (;;) {
		सचे (!forever)
			alarm(WARN_TIMEOUT);

		सचे (flock(fd, LOCK_SH) == 0) {
			retval = true;
			भङ्ग;
		}

		सचे (errno != EINTR)
			भङ्ग;

		सचे (!forever && timedout) {
			timedout = false;
			सचे ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			अञ्ञथा
				भङ्ग;
		}
	}

	alarm(0);
	close(fd);
	पटिनिवत्त retval;
}

स्थिर शून्य
get_started_services(शून्य)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	मुक्त(tmp);
}

स्थिर शून्य
setup_deptypes(शून्य)
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

स्थिर शून्य
svc_start_check(शून्य)
{
	पूर्णाङ्क ret = EXIT_FAILURE;
	RC_SERVICE state;

	state = rc_service_state(applet);

	सचे (in_background) {
		सचे (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			गच्छ exit;

		सचे (rc_yesno(getenv("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		सचे (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	सचे (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = EXIT_SUCCESS;
		गच्छ exit;
	} अञ्ञथा सचे (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	सचे (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	सचे (exclusive_fd == -1) {
		सचे (errno != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
			गच्छ exit;
		}

		सचे (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		अञ्ञथा
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	पटिनिवत्त;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(ret);
}

स्थिर शून्य
svc_start_deps(शून्य)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	पूर्णाङ्क depoptions = RC_DEP_TRACE;
	size_t len, svc_गणनम्;
	वर्ण *tmp;
	FILE *mem;
	pid_t pid;

	errno = 0;
	सचे (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	सचे (!deptree && ((deptree = _rc_deptree_load(0, रिक्त)) == रिक्त)) {
		eerror("failed to load deptree");
		गच्छ exit;
	}

	सचे (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	सचे (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			सचे (first)
				first = false;
			अञ्ञथा
				fprintf(stderr, ", ");
			fprintf(stderr, "%s", svc->मानम्);
		}
		fprintf(stderr, "\n");
		गच्छ exit;
	}
	rc_stringlist_free(services);
	services = रिक्त;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	सचे (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->मानम्);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			सचे (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				अनुवर्त;
			सचे (state & RC_SERVICE_STOPPED) {
				सचे (dry_run) {
					printf(" %s", svc->मानम्);
					अनुवर्त;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->मानम्);
				सचे (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	सचे (dry_run)
		पटिनिवत्त;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->मानम्);
		सचे (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->मानम्);
			अनुवर्त;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		सचे (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			सचे (!rc_stringlist_find(need_services, svc->मानम्) &&
					!rc_stringlist_find(want_services, svc->मानम्) &&
					!rc_stringlist_find(use_services, svc->मानम्))
				अनुवर्त;
		}

		सचे (!svc_wait(svc->मानम्))
			eerror("%s: timed out waiting for %s", applet, svc->मानम्);
		state = rc_service_state(svc->मानम्);
		सचे (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->मानम्);
			अनुवर्त;
		}
		सचे (rc_stringlist_find(need_services, svc->मानम्)) {
			सचे (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->मानम्);
			} अञ्ञथा सचे (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->मानम्);
				गच्छ exit;
			}
		}
	}

	सचे (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = रिक्त;
		mem = xopen_memstream(&tmp, &len);
		svc_गणनम् = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->मानम्, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->मानम्);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->मानम्, applet);
			rc_stringlist_free(use_services);
			use_services = रिक्त;
			सचे (svc_गणनम् > 0)
				fputs(", ", mem);
			fputs(svc->मानम्, mem);
			++svc_गणनम्;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = रिक्त;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_गणनम् > 1 ? "have" : "has");
		मुक्त(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = रिक्त;
	rc_stringlist_free(services);
	services = रिक्त;

	पटिनिवत्त;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	exit(EXIT_FAILURE);
}


स्थिर शून्य set_reexports(शून्य)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	size_t size;
	वर्ण *buf;
	FILE *fp;

	fp = xopen_memstream(&buf, &size);

	fputs("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		fprintf(fp, "%s ", envvar->मानम्);

	fclose(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

स्थिर शून्य svc_start_real(शून्य)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	सचे (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	सचे (ibsave)
		unsetenv("IN_BACKGROUND");

	सचे (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	सचे (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	अञ्ञथा सचे (!started)
		eerrorx("ERROR: %s failed to start", applet);

	सचे (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    सचे (rc_service_state(svc->मानम्) & RC_SERVICE_STOPPED)
		    service_start(svc->मानम्);
	rc_stringlist_free(services);
	services = रिक्त;

	/* Do the same for any services we provide */
	सचे (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->मानम्);
			TAILQ_FOREACH(svc2, services, entries)
				सचे (rc_service_state(svc2->मानम्) & RC_SERVICE_STOPPED)
					service_start(svc2->मानम्);
			rc_stringlist_free(services);
			services = रिक्त;
		}
		rc_stringlist_free(tmplist);
		tmplist = रिक्त;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

स्थिर पूर्णाङ्क
svc_start(शून्य)
{
	सचे (dry_run)
		einfon("start:");
	अञ्ञथा
		svc_start_check();

	सचे (deps)
		svc_start_deps();

	सचे (dry_run)
		printf(" %s\n", applet);
	अञ्ञथा
		svc_start_real();

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	सचे (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		गच्छ exit;

	सचे (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		गच्छ exit;

	सचे (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		पटिनिवत्त 1;
	}

	सचे (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	सचे (exclusive_fd == -1) {
		सचे (errno != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, strerror(errno));
		अञ्ञथा सचे (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		अञ्ञथा
			eerror("ERROR: %s stopped by something else", applet);
		गच्छ exit;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	सचे (!rc_runlevel_stopping()) {
		सचे (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		अञ्ञथा सचे (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	पटिनिवत्त 0;

exit:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	exit(EXIT_FAILURE);
}

स्थिर शून्य
svc_stop_deps(RC_SERVICE state)
{
	पूर्णाङ्क depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	सचे (state & RC_SERVICE_WASINACTIVE)
		पटिनिवत्त;

	errno = 0;
	सचे (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	सचे (!deptree && ((deptree = _rc_deptree_load(0, रिक्त)) == रिक्त)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	सचे (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->मानम्);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		सचे (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			अनुवर्त;
		सचे (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			अनुवर्त;
		सचे (dry_run) {
			printf(" %s", svc->मानम्);
			अनुवर्त;
		}
		svc_wait(svc->मानम्);
		सचे (rc_service_state(svc->मानम्) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->मानम्);
			सचे (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->मानम्);
		}
	}
	rc_stringlist_free(services);
	services = रिक्त;
	सचे (dry_run)
		पटिनिवत्त;

	TAILQ_FOREACH(svc, tmplist, entries) {
		सचे (rc_service_state(svc->मानम्) & RC_SERVICE_STOPPED) {
			अनुवर्त;
		}
		svc_wait(svc->मानम्);
		सचे (rc_service_state(svc->मानम्) & RC_SERVICE_STOPPED)
			अनुवर्त;
		सचे (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			सचे (runlevel && (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 || strcmp(runlevel, RC_LEVEL_SINGLE) == 0))
				अनुवर्त;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->मानम्);
	}
	rc_stringlist_free(tmplist);
	tmplist = रिक्त;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		सचे (rc_service_state(svc->मानम्) & RC_SERVICE_STOPPED)
			अनुवर्त;
		svc_wait(svc->मानम्);
	}
	rc_stringlist_free(services);
	services = रिक्त;
}

स्थिर शून्य
load_dep_env(शून्य)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->मानम्);

	rc_stringlist_free(depsvcs);
}

स्थिर शून्य
svc_stop_real(शून्य)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	सचे (strcmp(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	सचे (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	सचे (ibsave)
		unsetenv("IN_BACKGROUND");

	सचे (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	सचे (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

स्थिर पूर्णाङ्क
svc_stop(शून्य)
{
	RC_SERVICE state = 0;

	सचे (dry_run)
		einfon("stop:");
	अञ्ञथा
		सचे (svc_stop_check(&state) == 1)
			पटिनिवत्त 1; /* Service has been stopped already */

	सचे (deps)
		svc_stop_deps(state);

	सचे (dry_run)
		printf(" %s\n", applet);
	अञ्ञथा
		svc_stop_real();

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क
svc_restart(शून्य)
{
	सचे (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		सचे (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = रिक्त;

	पटिनिवत्त 0;
}

स्थिर bool
service_plugable(शून्य)
{
	वर्ण *list, *p, *token;
	bool allow = true, truefalse;
	वर्ण *match = rc_conf_मानम्("rc_hotplug");

	सचे (!match)
		match = rc_conf_मानम्("rc_plug_services");
	सचे (!match)
		पटिनिवत्त false;

	list = xstrdup(match);
	p = list;
	याव ((token = strsep(&p, " "))) {
		सचे (token[0] == '!') {
			truefalse = false;
			token++;
		} अञ्ञथा {
			truefalse = true;
		}

		सचे (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			भङ्ग;
		}
	}
	मुक्त(list);
	पटिनिवत्त allow;
}

स्थिर पूर्णाङ्क
simple_command(शून्य)
{
	वर्ण *save = prefix;
	पूर्णाङ्क retval;

	eprefix(रिक्त);
	prefix = रिक्त;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	पटिनिवत्त retval;
}

स्थिर पूर्णाङ्क
resolve_deptype(शून्य)
{
	पूर्णाङ्क depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	errno = 0;
	सचे (rc_conf_yesno("rc_depend_strict") || errno == ENOENT)
		depoptions |= RC_DEP_STRICT;

	सचे (!deptree && ((deptree = _rc_deptree_load(0, रिक्त)) == रिक्त))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = रिक्त;
	TAILQ_FOREACH(svc, services, entries)
		printf("%s ", svc->मानम्);
	printf ("\n");

	rc_stringlist_free(services);
	services = रिक्त;

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क
cmd_condrestart(शून्य)
{
	सचे (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क
cmd_stop(शून्य)
{
	RC_STRING *svc;

	सचे (strcmp(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	सचे (deps && in_background)
		get_started_services();

	सचे (svc_stop() == 1)
		पटिनिवत्त 0; /* Service has been stopped already */

	सचे (deps) {
		सचे (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		सचे (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				सचे (rc_service_state(svc->मानम्) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->मानम्);
		}
	}

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क
cmd_zap(शून्य)
{
	einfo("Manually resetting %s to stopped state", applet);
	सचे (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", strerror(errno));
	unhotplug();

	पटिनिवत्त 0;
}

संरचना {
	ध्रुव वर्ण *name;
	पूर्णाङ्क (*handler)(शून्य);
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

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क retval, opt;
	ध्रुव वर्ण *workingdir = "/";
	वर्ण *pidstr = रिक्त;
	size_t l = 0, ll;
	संरचना stat stbuf;

	/* Show help if insufficient args */
	सचे (argc < 2 || access(argv[1], F_OK) != 0) {
		fprintf(stderr, "openrc-run should not be run directly\n");
		exit(EXIT_FAILURE);
	}

	applet = basename_c(argv[0]);

	सचे (stat(argv[1], &stbuf) != 0) {
		fprintf(stderr, "openrc-run '%s': %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	सचे (!(service = (realpath(argv[1], रिक्त))))
		eerrorx("readpath: %s", strerror(errno));
	applet = basename_c(argv[1]);

	सचे (argc < 3)
		usage(EXIT_FAILURE);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	याव ((opt = getopt_long(argc, argv, getoptstring, longopts, रिक्त)) != -1)
		विकल्प (opt) {
		पक्ष 'd':
			setenv("RC_DEBUG", "YES", 1);
			भङ्ग;
		पक्ष 'l':
			exclusive_fd = atoi(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			भङ्ग;
		पक्ष 's':
			सचे (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				exit(EXIT_FAILURE);
			भङ्ग;
		पक्ष 'S':
			सचे (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				exit(EXIT_FAILURE);
			भङ्ग;
		पक्ष 'D':
			deps = false;
			भङ्ग;
		पक्ष 'Z':
			dry_run = true;
			भङ्ग;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	सचे (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	सचे (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	atexit(cleanup);

	सचे ((runlevel = xstrdup(getenv("RC_RUNLEVEL"))) == रिक्त) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (पूर्णाङ्क) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	मुक्त(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	सचे (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(रिक्त);
		TAILQ_FOREACH(svc, services, entries) {
			ll = strlen(svc->मानम्);
			सचे (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = रिक्त;
		ll = strlen(applet);
		सचे (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(परिमाण(वर्ण) * l + 1);
		memcpy(prefix, applet, ll);
		memset(prefix + ll, ' ', l - ll);
		memset(prefix + l, 0, 1);
		eprefix(prefix);
	}

	सचे (rc_yesno(getenv("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	सचे (deps && getenv("RC_PID") == रिक्त && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	सचे (getenv("IN_BACKGROUND")) {
		ibsave = xstrdup(getenv("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	सचे (rc_yesno(getenv("IN_DRYRUN")))
		dry_run = true;
	सचे (rc_yesno(getenv("IN_HOTPLUG"))) {
		सचे (!service_plugable())
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

	सचे (optind >= argc)
		usage(EXIT_FAILURE);

	/* Now run each option */
	retval = EXIT_SUCCESS;

next_cmd:
	याव (optind < argc) {
		पूर्णाङ्क cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		सचे (sighup)
			exit (EXIT_FAILURE);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		अत्थाय (size_t i = 0; i < ARRAY_SIZE(commands); i++) {
			सचे (strcmp(optarg, commands[i].name) != 0)
				अनुवर्त;

			सचे ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			सचे (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = रिक्त;
			}

			गच्छ next_cmd;
		}

		सचे ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = रिक्त;
	}

	पटिनिवत्त retval;
}
