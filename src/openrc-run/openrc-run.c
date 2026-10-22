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

#जर defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#किंवाजर defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#अन्यथा
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

बाह्य अक्षर **environ;

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = "stop | start | restart | status | describe | zap";
स्थिरांक अक्षर getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "debug",      0, रिक्त, 'd'},
	{ "dry-run",    0, रिक्त, 'Z'},
	{ "ifstarted",  0, रिक्त, 's'},
	{ "ifstopped",  0, रिक्त, 'S'},
	{ "nodeps",     0, रिक्त, 'D'},
	{ "lockfd",     1, रिक्त, 'l'},
	longopts_COMMON
};
स्थिरांक अक्षर *स्थिरांक longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
स्थिरांक अक्षर *usagestring = रिक्त;

स्थिर अक्षर *service, *runlevel, *ibsave, *prefix;
स्थिर RC_DEPTREE *deptree;
स्थिर RC_STRINGLIST *applet_list, *services, *tmplist;
स्थिर RC_STRINGLIST *restart_services;
स्थिर RC_STRINGLIST *need_services;
स्थिर RC_STRINGLIST *use_services;
स्थिर RC_STRINGLIST *want_services;
स्थिर पूर्णांक exclusive_fd = -1, master_tty = -1;
स्थिर bool in_background, deps, dry_run;
स्थिर अस्थिर bool sighup, skip_mark, timedout;
स्थिर pid_t service_pid;
स्थिर पूर्णांक signal_pipe[2] = { -1, -1 };

स्थिर RC_STRINGLIST *deptypes_b;	/* broken deps */
स्थिर RC_STRINGLIST *deptypes_n;	/* needed deps */
स्थिर RC_STRINGLIST *deptypes_nw;	/* need+want deps */
स्थिर RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
स्थिर RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
स्थिर RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
स्थिर RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

स्थिर शून्य
handle_signal(पूर्णांक sig)
{
	पूर्णांक serrno = त्रुटि_सं, status;
	pid_t pid;
	स्थिरांक अक्षर *signame = रिक्त;
	संरचना winsize ws;

	निवड (sig) {
	स्थिती SIGHUP:
		sighup = true;
		थांबा;

	स्थिती SIGUSR1:
		skip_mark = true;
		थांबा;

	स्थिती SIGCHLD:
		पर्यंत ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			जर (signal_pipe[1] > -1 && pid == service_pid) {
				जर (लिख(signal_pipe[1], &status, आकार(status)) == -1)
					eerror("%s: send: %s", applet, म_त्रुटि(त्रुटि_सं));
			}
		}
		थांबा;

	स्थिती SIGWINCH:
		जर (master_tty >= 0) {
			ioctl(fileno(मानक_निकास), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		थांबा;

	स्थिती संक_विघ्न:
		जर (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	स्थिती संक_इति:
		जर (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	स्थिती SIGQUIT:
		जर (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		जर (service_pid > 0)
			समाप्त(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		निकास(निकास_त्रुटि);
		/* NOTREACHED */

	स्थिती SIGALRM:
		timedout = true;
		थांबा;

	पूर्वनिर्धारित:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	त्रुटि_सं = serrno;
}

स्थिर शून्य
unhotplug(शून्य)
{
	जर (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && त्रुटि_सं != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, म_त्रुटि(त्रुटि_सं));
}

स्थिर शून्य
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	जर (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		परत;

	TAILQ_FOREACH(svc, list, entries) {
		जर (!(rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED))
			सुरूठेवा;
		जर (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->मूल्य);
			ewarn("WARNING: %s will start when %s has started", svc->मूल्य, applet);
		} अन्यथा {
			service_start(svc->मूल्य);
		}
	}
}

स्थिर शून्य
restore_state(शून्य)
{
	RC_SERVICE state;

	जर (rc_in_plugin || exclusive_fd == -1)
		परत;
	state = rc_service_state(applet);
	जर (state & RC_SERVICE_STOPPING) {
		जर (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		अन्यथा
			rc_service_mark(applet, RC_SERVICE_STARTED);

		जर (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} अन्यथा जर (state & RC_SERVICE_STARTING) {
		जर (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		अन्यथा
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		जर (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

स्थिर शून्य
cleanup(शून्य)
{
	restore_state();

	जर (!rc_in_plugin) {
		जर (restart_services)
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
svc_getenv(स्थिरांक अक्षर *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	अक्षर *var = रिक्त;

	TAILQ_FOREACH(reexport, reexports, entries) {
		जर (rc_service_getenv(svc, reexport->मूल्य, &var) == -1)
			सुरूठेवा;
		setenv(reexport->मूल्य, var, true);
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
स्थिर पूर्णांक
write_prefix(स्थिरांक अक्षर *buffer, माप_प्रकार bytes, bool *prefixed)
{
	माप_प्रकार i, j;
	स्थिरांक अक्षर *ec = ecolor(ECOLOR_HILITE);
	स्थिरांक अक्षर *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	पूर्णांक fd = fileno(मानक_निकास), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	जर (lock_fd != -1) {
		पर्यंत (flock(lock_fd, LOCK_EX) != 0) {
			जर (त्रुटि_सं != EINTR) {
				ewarnv("flock() failed: %s", म_त्रुटि(त्रुटि_सं));
				थांबा;
			}
		}
	}
	अन्यथा
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	साठी (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		जर (buffer[i] == '\033' && !*prefixed) {
			साठी (j = i + 1; j < bytes; j++) {
				जर (buffer[j] == 'A')
					*prefixed = true;
				जर (है_अक्षर((अचिन्हित पूर्णांक)buffer[j]))
					थांबा;
			}
		}

		जर (!*prefixed) {
			ret += लिख(fd, ec, म_माप(ec));
			ret += लिख(fd, prefix, म_माप(prefix));
			ret += लिख(fd, ec_normal, म_माप(ec_normal));
			ret += लिख(fd, "|", 1);
			*prefixed = true;
		}

		जर (buffer[i] == '\n')
			*prefixed = false;
		ret += लिख(fd, buffer + i, 1);
	}

	/* Release the lock */
	बंद(lock_fd);

	परत ret;
}

स्थिर पूर्णांक
svc_exec(स्थिरांक अक्षर *command)
{
	पूर्णांक ret = -1, fdout = fileno(मानक_निकास), tmp;
	posix_spawn_file_actions_t tty;
	संरचना termios tt;
	संरचना winsize ws;
	पूर्णांक flags = 0;
	संरचना pollfd fd[2];
	bool prefixed = false;
	पूर्णांक slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	अक्षर *openrc_sh = रिक्त;
	स्थिरांक अक्षर *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		रिक्त
	};

	/* Setup our signal pipe */
	जर (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		परत 1;
	}

	जर ((त्रुटि_सं = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, म_त्रुटि(त्रुटि_सं));
		परत 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	जर (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		जर (openpty(&master_tty, &slave_tty, रिक्त, &tt, &ws) == 0) {
			जर (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			जर (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			जर ((त्रुटि_सं = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (त्रुटि_सं = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, म_त्रुटि(त्रुटि_सं));
				परत 1;
			}
		}
	}

	जर (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	जर ((त्रुटि_सं = posix_spawn(&service_pid, argv[0], &tty, रिक्त, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], म_त्रुटि(त्रुटि_सं));
		परत 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	मुक्त(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	साठी (;;) {
		जर (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			जर (त्रुटि_सं == EINTR)
				सुरूठेवा;
			eerror("%s: poll: %s", applet, म_त्रुटि(त्रुटि_सं));
			थांबा;
		}

		जर (fd[1].revents & (POLLIN | POLLHUP)) {
			अक्षर buffer[बफ_मान];
			write_prefix(buffer, पठ(master_tty, buffer, बफ_मान), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		जर (fd[0].revents & (POLLIN | POLLHUP)) {
			जर (त्रुटि_सं = 0, पठ(signal_pipe[0], &ret, आकार(ret)) < (ssize_t)आकार(ret))
				eerror("%s: receive failed: %s", applet, त्रुटि_सं ? म_त्रुटि(त्रुटि_सं) : "short read");
			अन्यथा जर ((ret = WEXITSTATUS(ret)) != 0 && त्रुटि_सं == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			थांबा;
		}
	}

	sigemptyset (&sigchldmask);
	sigaddset (&sigchldmask, SIGCHLD);
	sigprocmask (SIG_BLOCK, &sigchldmask, &oldmask);

	बंद(signal_pipe[0]);
	/* We should set signal_pipe[1] to -1 before closing it to avoid invalid write if
	 * sighandler runs between close() and cleaning the variable */
	tmp = signal_pipe[1];
	signal_pipe[0] = signal_pipe[1] = -1;
	बंद(tmp);

	sigprocmask (SIG_SETMASK, &oldmask, रिक्त);

	जर (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		बंद(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	परत ret;
}

स्थिर bool
svc_wait(स्थिरांक अक्षर *svc)
{
	पूर्णांक fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	स्थिरांक अक्षर *base = basename_c(svc);
	पूर्णांक timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	जर (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	जर ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		परत त्रुटि_सं == ENOENT;

	timedout = false;
	साठी (;;) {
		जर (!forever)
			alarm(WARN_TIMEOUT);

		जर (flock(fd, LOCK_SH) == 0) {
			retval = true;
			थांबा;
		}

		जर (त्रुटि_सं != EINTR)
			थांबा;

		जर (!forever && timedout) {
			timedout = false;
			जर ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			अन्यथा
				थांबा;
		}
	}

	alarm(0);
	बंद(fd);
	परत retval;
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
	पूर्णांक ret = निकास_त्रुटि;
	RC_SERVICE state;

	state = rc_service_state(applet);

	जर (in_background) {
		जर (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			जा निकास;

		जर (rc_yesno(दो_पर्या("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		जर (म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	जर (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = निकास_सफल;
		जा निकास;
	} अन्यथा जर (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	जर (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	जर (exclusive_fd == -1) {
		जर (त्रुटि_सं != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, म_त्रुटि(त्रुटि_सं));
			जा निकास;
		}

		जर (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		अन्यथा
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	परत;

निकास:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	निकास(ret);
}

स्थिर शून्य
svc_start_deps(शून्य)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	पूर्णांक depoptions = RC_DEP_TRACE;
	माप_प्रकार len, svc_मोजणी;
	अक्षर *tmp;
	खाता *mem;
	pid_t pid;

	त्रुटि_सं = 0;
	जर (rc_conf_yesno("rc_depend_strict") || त्रुटि_सं == ENOENT)
		depoptions |= RC_DEP_STRICT;

	जर (!deptree && ((deptree = _rc_deptree_load(0, रिक्त)) == रिक्त)) {
		eerror("failed to load deptree");
		जा निकास;
	}

	जर (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	जर (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			जर (first)
				first = false;
			अन्यथा
				ख_लिखो(मानक_त्रुटि, ", ");
			ख_लिखो(मानक_त्रुटि, "%s", svc->मूल्य);
		}
		ख_लिखो(मानक_त्रुटि, "\n");
		जा निकास;
	}
	rc_stringlist_free(services);
	services = रिक्त;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	जर (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->मूल्य);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			जर (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				सुरूठेवा;
			जर (state & RC_SERVICE_STOPPED) {
				जर (dry_run) {
					म_लिखो(" %s", svc->मूल्य);
					सुरूठेवा;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->मूल्य);
				जर (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	जर (dry_run)
		परत;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->मूल्य);
		जर (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->मूल्य);
			सुरूठेवा;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		जर (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			जर (!rc_stringlist_find(need_services, svc->मूल्य) &&
					!rc_stringlist_find(want_services, svc->मूल्य) &&
					!rc_stringlist_find(use_services, svc->मूल्य))
				सुरूठेवा;
		}

		जर (!svc_wait(svc->मूल्य))
			eerror("%s: timed out waiting for %s", applet, svc->मूल्य);
		state = rc_service_state(svc->मूल्य);
		जर (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->मूल्य);
			सुरूठेवा;
		}
		जर (rc_stringlist_find(need_services, svc->मूल्य)) {
			जर (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->मूल्य);
			} अन्यथा जर (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->मूल्य);
				जा निकास;
			}
		}
	}

	जर (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = रिक्त;
		mem = xopen_memstream(&tmp, &len);
		svc_मोजणी = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->मूल्य, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->मूल्य);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->मूल्य, applet);
			rc_stringlist_free(use_services);
			use_services = रिक्त;
			जर (svc_मोजणी > 0)
				ख_माला_दो(", ", mem);
			ख_माला_दो(svc->मूल्य, mem);
			++svc_मोजणी;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = रिक्त;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_मोजणी > 1 ? "have" : "has");
		मुक्त(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = रिक्त;
	rc_stringlist_free(services);
	services = रिक्त;

	परत;

निकास:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	निकास(निकास_त्रुटि);
}


स्थिर शून्य set_reexports(शून्य)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	माप_प्रकार size;
	अक्षर *buf;
	खाता *fp;

	fp = xopen_memstream(&buf, &size);

	ख_माला_दो("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		ख_लिखो(fp, "%s ", envvar->मूल्य);

	ख_बंद(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

स्थिर शून्य svc_start_real(शून्य)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	जर (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	जर (ibsave)
		unsetenv("IN_BACKGROUND");

	जर (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	जर (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	अन्यथा जर (!started)
		eerrorx("ERROR: %s failed to start", applet);

	जर (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    जर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
		    service_start(svc->मूल्य);
	rc_stringlist_free(services);
	services = रिक्त;

	/* Do the same for any services we provide */
	जर (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->मूल्य);
			TAILQ_FOREACH(svc2, services, entries)
				जर (rc_service_state(svc2->मूल्य) & RC_SERVICE_STOPPED)
					service_start(svc2->मूल्य);
			rc_stringlist_free(services);
			services = रिक्त;
		}
		rc_stringlist_free(tmplist);
		tmplist = रिक्त;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

स्थिर पूर्णांक
svc_start(शून्य)
{
	जर (dry_run)
		einfon("start:");
	अन्यथा
		svc_start_check();

	जर (deps)
		svc_start_deps();

	जर (dry_run)
		म_लिखो(" %s\n", applet);
	अन्यथा
		svc_start_real();

	परत 0;
}

स्थिर पूर्णांक
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	जर (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		जा निकास;

	जर (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		जा निकास;

	जर (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		परत 1;
	}

	जर (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	जर (exclusive_fd == -1) {
		जर (त्रुटि_सं != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, म_त्रुटि(त्रुटि_सं));
		अन्यथा जर (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		अन्यथा
			eerror("ERROR: %s stopped by something else", applet);
		जा निकास;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	जर (!rc_runlevel_stopping()) {
		जर (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		अन्यथा जर (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	परत 0;

निकास:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	निकास(निकास_त्रुटि);
}

स्थिर शून्य
svc_stop_deps(RC_SERVICE state)
{
	पूर्णांक depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	जर (state & RC_SERVICE_WASINACTIVE)
		परत;

	त्रुटि_सं = 0;
	जर (rc_conf_yesno("rc_depend_strict") || त्रुटि_सं == ENOENT)
		depoptions |= RC_DEP_STRICT;

	जर (!deptree && ((deptree = _rc_deptree_load(0, रिक्त)) == रिक्त)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	जर (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->मूल्य);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		जर (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			सुरूठेवा;
		जर (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			सुरूठेवा;
		जर (dry_run) {
			म_लिखो(" %s", svc->मूल्य);
			सुरूठेवा;
		}
		svc_wait(svc->मूल्य);
		जर (rc_service_state(svc->मूल्य) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->मूल्य);
			जर (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->मूल्य);
		}
	}
	rc_stringlist_free(services);
	services = रिक्त;
	जर (dry_run)
		परत;

	TAILQ_FOREACH(svc, tmplist, entries) {
		जर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED) {
			सुरूठेवा;
		}
		svc_wait(svc->मूल्य);
		जर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
			सुरूठेवा;
		जर (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			जर (runlevel && (म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0 || म_भेद(runlevel, RC_LEVEL_SINGLE) == 0))
				सुरूठेवा;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->मूल्य);
	}
	rc_stringlist_free(tmplist);
	tmplist = रिक्त;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		जर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
			सुरूठेवा;
		svc_wait(svc->मूल्य);
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
		svc_getenv(svc->मूल्य);

	rc_stringlist_free(depsvcs);
}

स्थिर शून्य
svc_stop_real(शून्य)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	जर (म_भेद(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	जर (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	जर (ibsave)
		unsetenv("IN_BACKGROUND");

	जर (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	जर (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

स्थिर पूर्णांक
svc_stop(शून्य)
{
	RC_SERVICE state = 0;

	जर (dry_run)
		einfon("stop:");
	अन्यथा
		जर (svc_stop_check(&state) == 1)
			परत 1; /* Service has been stopped already */

	जर (deps)
		svc_stop_deps(state);

	जर (dry_run)
		म_लिखो(" %s\n", applet);
	अन्यथा
		svc_stop_real();

	परत 0;
}

स्थिर पूर्णांक
svc_restart(शून्य)
{
	जर (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		जर (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = रिक्त;

	परत 0;
}

स्थिर bool
service_plugable(शून्य)
{
	अक्षर *list, *p, *token;
	bool allow = true, truefalse;
	अक्षर *match = rc_conf_मूल्य("rc_hotplug");

	जर (!match)
		match = rc_conf_मूल्य("rc_plug_services");
	जर (!match)
		परत false;

	list = xstrdup(match);
	p = list;
	पर्यंत ((token = strsep(&p, " "))) {
		जर (token[0] == '!') {
			truefalse = false;
			token++;
		} अन्यथा {
			truefalse = true;
		}

		जर (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			थांबा;
		}
	}
	मुक्त(list);
	परत allow;
}

स्थिर पूर्णांक
simple_command(शून्य)
{
	अक्षर *save = prefix;
	पूर्णांक retval;

	eprefix(रिक्त);
	prefix = रिक्त;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	परत retval;
}

स्थिर पूर्णांक
resolve_deptype(शून्य)
{
	पूर्णांक depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	त्रुटि_सं = 0;
	जर (rc_conf_yesno("rc_depend_strict") || त्रुटि_सं == ENOENT)
		depoptions |= RC_DEP_STRICT;

	जर (!deptree && ((deptree = _rc_deptree_load(0, रिक्त)) == रिक्त))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = रिक्त;
	TAILQ_FOREACH(svc, services, entries)
		म_लिखो("%s ", svc->मूल्य);
	म_लिखो ("\n");

	rc_stringlist_free(services);
	services = रिक्त;

	परत 0;
}

स्थिर पूर्णांक
cmd_condrestart(शून्य)
{
	जर (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	परत 0;
}

स्थिर पूर्णांक
cmd_stop(शून्य)
{
	RC_STRING *svc;

	जर (म_भेद(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	जर (deps && in_background)
		get_started_services();

	जर (svc_stop() == 1)
		परत 0; /* Service has been stopped already */

	जर (deps) {
		जर (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		जर (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				जर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->मूल्य);
		}
	}

	परत 0;
}

स्थिर पूर्णांक
cmd_zap(शून्य)
{
	einfo("Manually resetting %s to stopped state", applet);
	जर (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", म_त्रुटि(त्रुटि_सं));
	unhotplug();

	परत 0;
}

संरचना {
	स्थिरांक अक्षर *name;
	पूर्णांक (*handler)(शून्य);
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

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक retval, opt;
	स्थिरांक अक्षर *workingdir = "/";
	अक्षर *pidstr = रिक्त;
	माप_प्रकार l = 0, ll;
	संरचना stat stbuf;

	/* Show help if insufficient args */
	जर (argc < 2 || access(argv[1], F_OK) != 0) {
		ख_लिखो(मानक_त्रुटि, "openrc-run should not be run directly\n");
		निकास(निकास_त्रुटि);
	}

	applet = basename_c(argv[0]);

	जर (stat(argv[1], &stbuf) != 0) {
		ख_लिखो(मानक_त्रुटि, "openrc-run '%s': %s\n", argv[1], म_त्रुटि(त्रुटि_सं));
		निकास(निकास_त्रुटि);
	}

	जर (!(service = (realpath(argv[1], रिक्त))))
		eerrorx("readpath: %s", म_त्रुटि(त्रुटि_सं));
	applet = basename_c(argv[1]);

	जर (argc < 3)
		usage(निकास_त्रुटि);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring, longopts, रिक्त)) != -1)
		निवड (opt) {
		स्थिती 'd':
			setenv("RC_DEBUG", "YES", 1);
			थांबा;
		स्थिती 'l':
			exclusive_fd = म_से_प(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			थांबा;
		स्थिती 's':
			जर (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				निकास(निकास_त्रुटि);
			थांबा;
		स्थिती 'S':
			जर (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				निकास(निकास_त्रुटि);
			थांबा;
		स्थिती 'D':
			deps = false;
			थांबा;
		स्थिती 'Z':
			dry_run = true;
			थांबा;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	जर (rc_is_user() && !(workingdir = दो_पर्या("HOME")))
		eerrorx("HOME is unset.");

	जर (स_बदलो(workingdir) == -1)
		eerror("chdir: %s", म_त्रुटि(त्रुटि_सं));

	निकास_पर(cleanup);

	जर ((runlevel = xstrdup(दो_पर्या("RC_RUNLEVEL"))) == रिक्त) {
		env_filter();
		env_config();
		runlevel = default_runlevel();
	}

	setenv("EINFO_LOG", applet, 1);
	setenv("RC_SVCNAME", applet, 1);

	/* Set an env var so that we always know our pid regardless of any
	   subshells the init script may create so that our mark_service_*
	   functions can always instruct us of this change */
	xasprintf(&pidstr, "%d", (पूर्णांक) getpid());
	setenv("RC_OPENRC_PID", pidstr, 1);
	/*
	 * RC_RUNSCRIPT_PID is deprecated, but we will keep it for a while
	 * for safety.
	 */
	setenv("RC_RUNSCRIPT_PID", pidstr, 1);
	मुक्त(pidstr);

	/* eprefix is kinda klunky, but it works for our purposes */
	जर (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(रिक्त);
		TAILQ_FOREACH(svc, services, entries) {
			ll = म_माप(svc->मूल्य);
			जर (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = रिक्त;
		ll = म_माप(applet);
		जर (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(आकार(अक्षर) * l + 1);
		स_नकल(prefix, applet, ll);
		स_रखो(prefix + ll, ' ', l - ll);
		स_रखो(prefix + l, 0, 1);
		eprefix(prefix);
	}

	जर (rc_yesno(दो_पर्या("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	जर (deps && दो_पर्या("RC_PID") == रिक्त && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	जर (दो_पर्या("IN_BACKGROUND")) {
		ibsave = xstrdup(दो_पर्या("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	जर (rc_yesno(दो_पर्या("IN_DRYRUN")))
		dry_run = true;
	जर (rc_yesno(दो_पर्या("IN_HOTPLUG"))) {
		जर (!service_plugable())
			eerrorx("%s: not allowed to be hotplugged", applet);
		in_background = true;
	}

	/* Setup a signal handler */
	signal_setup(SIGHUP, handle_signal);
	signal_setup(SIGALRM, handle_signal);
	signal_setup(SIGUSR1, handle_signal);
	signal_setup(संक_विघ्न, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(संक_इति, handle_signal);
	signal_setup(SIGCHLD, handle_signal);

	/* Load our plugins */
	rc_plugin_load();

	applet_list = rc_stringlist_new();
	rc_stringlist_add(applet_list, applet);

	जर (optind >= argc)
		usage(निकास_त्रुटि);

	/* Now run each option */
	retval = निकास_सफल;

next_cmd:
	पर्यंत (optind < argc) {
		पूर्णांक cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		जर (sighup)
			निकास (निकास_त्रुटि);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		साठी (माप_प्रकार i = 0; i < ARRAY_SIZE(commands); i++) {
			जर (म_भेद(optarg, commands[i].name) != 0)
				सुरूठेवा;

			जर ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			जर (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = रिक्त;
			}

			जा next_cmd;
		}

		जर ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = रिक्त;
	}

	परत retval;
}
