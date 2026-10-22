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

#समावेश <ctype.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <fnmatch.h>
#समावेश <getopt.h>
#समावेश <libgen.h>
#समावेश <poll.h>
#समावेश <signal.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <spawn.h>
#समावेश <sys/ioctl.h>
#समावेश <sys/file.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <termios.h>
#समावेश <time.h>
#समावेश <unistd.h>
#समावेश <stdbool.h>
#समावेश <inttypes.h>

#अगर defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#या_अगर defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#अन्यथा
#  include <libutil.h>
#पूर्ण_अगर

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "rc_exec.h"
#समावेश "misc.h"
#समावेश "timeutils.h"
#समावेश "plugin.h"
#समावेश "selinux.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

#घोषणा WAIT_TIMEOUT	60		/* seconds until we timeout */
#घोषणा WARN_TIMEOUT	10		/* warn about this every N seconds */

बाह्य अक्षर **environ;

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = "stop | start | restart | status | describe | zap";
स्थिर अक्षर getoptstring[] = "dDsSvl:Z" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "debug",      0, शून्य, 'd'},
	{ "dry-run",    0, शून्य, 'Z'},
	{ "ifstarted",  0, शून्य, 's'},
	{ "ifstopped",  0, शून्य, 'S'},
	{ "nodeps",     0, शून्य, 'D'},
	{ "lockfd",     1, शून्य, 'l'},
	longopts_COMMON
};
स्थिर अक्षर *स्थिर longopts_help[] = {
	"set xtrace when running the script",
	"show what would be done",
	"only run commands when started",
	"only run commands when stopped",
	"ignore dependencies",
	"fd of the exclusive lock from rc",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = शून्य;

अटल अक्षर *service, *runlevel, *ibsave, *prefix;
अटल RC_DEPTREE *deptree;
अटल RC_STRINGLIST *applet_list, *services, *tmplist;
अटल RC_STRINGLIST *restart_services;
अटल RC_STRINGLIST *need_services;
अटल RC_STRINGLIST *use_services;
अटल RC_STRINGLIST *want_services;
अटल पूर्णांक exclusive_fd = -1, master_tty = -1;
अटल bool in_background, deps, dry_run;
अटल अस्थिर bool sighup, skip_mark, timedout;
अटल pid_t service_pid;
अटल पूर्णांक signal_pipe[2] = { -1, -1 };

अटल RC_STRINGLIST *deptypes_b;	/* broken deps */
अटल RC_STRINGLIST *deptypes_n;	/* needed deps */
अटल RC_STRINGLIST *deptypes_nw;	/* need+want deps */
अटल RC_STRINGLIST *deptypes_nwu;	/* need+want+use deps */
अटल RC_STRINGLIST *deptypes_nwua;	/* need+want+use+after deps */
अटल RC_STRINGLIST *deptypes_m;	/* needed deps for stopping */
अटल RC_STRINGLIST *deptypes_mwua;	/* need+want+use+after deps for stopping */

अटल व्योम
handle_signal(पूर्णांक sig)
{
	पूर्णांक serrno = त्रुटि_सं, status;
	pid_t pid;
	स्थिर अक्षर *signame = शून्य;
	काष्ठा winsize ws;

	चयन (sig) {
	हाल SIGHUP:
		sighup = true;
		अवरोध;

	हाल SIGUSR1:
		skip_mark = true;
		अवरोध;

	हाल SIGCHLD:
		जबतक ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			अगर (signal_pipe[1] > -1 && pid == service_pid) {
				अगर (ग_लिखो(signal_pipe[1], &status, माप(status)) == -1)
					eerror("%s: send: %s", applet, म_त्रुटि(त्रुटि_सं));
			}
		}
		अवरोध;

	हाल SIGWINCH:
		अगर (master_tty >= 0) {
			ioctl(fileno(मानक_निकास), TIOCGWINSZ, &ws);
			ioctl(master_tty, TIOCSWINSZ, &ws);
		}
		अवरोध;

	हाल संक_विघ्न:
		अगर (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	हाल संक_इति:
		अगर (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	हाल SIGQUIT:
		अगर (!signame)
			signame = "SIGQUIT";
		/* Send the signal to our children too */
		अगर (service_pid > 0)
			समाप्त(service_pid, sig);
		eerror("%s: caught %s, aborting", applet, signame);
		निकास(निकास_त्रुटि);
		/* NOTREACHED */

	हाल SIGALRM:
		timedout = true;
		अवरोध;

	शेष:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	त्रुटि_सं = serrno;
}

अटल व्योम
unhotplug(व्योम)
{
	अगर (unlinkat(rc_dirfd(RC_DIR_HOTPLUGGED), applet, 0) != 0 && त्रुटि_सं != ENOENT)
		eerror("%s: unlink '%s/hotplugged/%s': %s", applet, rc_svcdir(), applet, म_त्रुटि(त्रुटि_सं));
}

अटल व्योम
start_services(RC_STRINGLIST *list)
{
	RC_STRING *svc;
	RC_SERVICE state = rc_service_state(applet);

	अगर (!list || !(state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE | RC_SERVICE_STARTING | RC_SERVICE_STARTED)))
		वापस;

	TAILQ_FOREACH(svc, list, entries) {
		अगर (!(rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED))
			जारी;
		अगर (state & RC_SERVICE_INACTIVE || state & RC_SERVICE_WASINACTIVE) {
			rc_service_schedule_start(applet, svc->मूल्य);
			ewarn("WARNING: %s will start when %s has started", svc->मूल्य, applet);
		} अन्यथा {
			service_start(svc->मूल्य);
		}
	}
}

अटल व्योम
restore_state(व्योम)
{
	RC_SERVICE state;

	अगर (rc_in_plugin || exclusive_fd == -1)
		वापस;
	state = rc_service_state(applet);
	अगर (state & RC_SERVICE_STOPPING) {
		अगर (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		अन्यथा
			rc_service_mark(applet, RC_SERVICE_STARTED);

		अगर (rc_runlevel_stopping())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	} अन्यथा अगर (state & RC_SERVICE_STARTING) {
		अगर (state & RC_SERVICE_WASINACTIVE)
			rc_service_mark(applet, RC_SERVICE_INACTIVE);
		अन्यथा
			rc_service_mark(applet, RC_SERVICE_STOPPED);

		अगर (rc_runlevel_starting())
			rc_service_mark(applet, RC_SERVICE_FAILED);
	}
	exclusive_fd = svc_unlock(applet, exclusive_fd);
}

अटल व्योम
cleanup(व्योम)
{
	restore_state();

	अगर (!rc_in_plugin) {
		अगर (restart_services)
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

अटल व्योम
svc_getenv(स्थिर अक्षर *svc)
{
	RC_STRINGLIST *reexports = rc_deptree_depend(deptree, svc, "reexport");
	RC_STRING *reexport;
	अक्षर *var = शून्य;

	TAILQ_FOREACH(reexport, reexports, entries) {
		अगर (rc_service_getenv(svc, reexport->मूल्य, &var) == -1)
			जारी;
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
अटल पूर्णांक
write_prefix(स्थिर अक्षर *बफर, माप_प्रकार bytes, bool *prefixed)
{
	माप_प्रकार i, j;
	स्थिर अक्षर *ec = ecolor(ECOLOR_HILITE);
	स्थिर अक्षर *ec_normal = ecolor(ECOLOR_NORMAL);
	ssize_t ret = 0;
	पूर्णांक fd = fileno(मानक_निकास), lock_fd = -1;

	/*
	 * Lock the prefix.
	 * open() may fail here when running as user, as RC_SVCDIR may not be writable.
	 */
	lock_fd = openat(rc_dirfd(RC_DIR_SVCDIR), "prefix.lock", O_WRONLY | O_CREAT, 0664);

	अगर (lock_fd != -1) {
		जबतक (flock(lock_fd, LOCK_EX) != 0) {
			अगर (त्रुटि_सं != EINTR) {
				ewarnv("flock() failed: %s", म_त्रुटि(त्रुटि_सं));
				अवरोध;
			}
		}
	}
	अन्यथा
		ewarnv("Couldn't open the prefix lock, please make sure you have enough permissions");

	क्रम (i = 0; i < bytes; i++) {
		/* We don't prefix eend calls (cursor up) */
		अगर (बफर[i] == '\033' && !*prefixed) {
			क्रम (j = i + 1; j < bytes; j++) {
				अगर (बफर[j] == 'A')
					*prefixed = true;
				अगर (है_अक्षर((अचिन्हित पूर्णांक)बफर[j]))
					अवरोध;
			}
		}

		अगर (!*prefixed) {
			ret += ग_लिखो(fd, ec, म_माप(ec));
			ret += ग_लिखो(fd, prefix, म_माप(prefix));
			ret += ग_लिखो(fd, ec_normal, म_माप(ec_normal));
			ret += ग_लिखो(fd, "|", 1);
			*prefixed = true;
		}

		अगर (बफर[i] == '\n')
			*prefixed = false;
		ret += ग_लिखो(fd, बफर + i, 1);
	}

	/* Release the lock */
	बंद(lock_fd);

	वापस ret;
}

अटल पूर्णांक
svc_exec(स्थिर अक्षर *command)
{
	पूर्णांक ret = -1, fdout = fileno(मानक_निकास), tmp;
	posix_spawn_file_actions_t tty;
	काष्ठा termios tt;
	काष्ठा winsize ws;
	पूर्णांक flags = 0;
	काष्ठा pollfd fd[2];
	bool prefixed = false;
	पूर्णांक slave_tty;
	sigset_t sigchldmask;
	sigset_t oldmask;
	अक्षर *openrc_sh = शून्य;
	स्थिर अक्षर *argv[] = {
		RC_LIBEXECDIR "/sh/openrc-run.sh",
		service,
		command,
		शून्य
	};

	/* Setup our signal pipe */
	अगर (pipe2(signal_pipe, O_CLOEXEC) == -1) {
		eerror("%s: pipe2: %s", applet, applet);
		वापस 1;
	}

	अगर ((त्रुटि_सं = posix_spawn_file_actions_init(&tty))) {
		eerror("%s: posix_spawn_file_actions_init: %s", applet, म_त्रुटि(त्रुटि_सं));
		वापस 1;
	}

	/* Open a pty for our prefixed output
	 * We do this instead of mapping pipes to stdout, stderr so that
	 * programs can tell if they're attached to a tty or not.
	 * The only loss is that we can no longer tell the difference
	 * between the child's stdout or stderr */
	master_tty = slave_tty = -1;
	अगर (prefix && isatty(fdout)) {
		tcgetattr(fdout, &tt);
		ioctl(fdout, TIOCGWINSZ, &ws);

		/* If the below call fails due to not enough ptys then we don't
		 * prefix the output, but we still work */
		अगर (openpty(&master_tty, &slave_tty, शून्य, &tt, &ws) == 0) {
			अगर (master_tty >= 0 &&
			    (flags = fcntl(master_tty, F_GETFD, 0)) == 0)
				fcntl(master_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			अगर (slave_tty >= 0 &&
			    (flags = fcntl(slave_tty, F_GETFD, 0)) == 0)
				fcntl(slave_tty, F_SETFD,
				    flags | FD_CLOEXEC);

			अगर ((त्रुटि_सं = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDOUT_FILENO)) ||
			    (त्रुटि_सं = posix_spawn_file_actions_adddup2(&tty,
				    slave_tty, STDERR_FILENO))) {
				eerror("%s: posix_spawn_file_actions_adddup2: %s",
				    applet, म_त्रुटि(त्रुटि_सं));
				वापस 1;
			}
		}
	}

	अगर (faccessat(rc_dirfd(RC_DIR_SVCDIR), "openrc-run.sh", F_OK, 0) == 0) {
		xasprintf(&openrc_sh, "%s/openrc-run.sh", rc_svcdir());
		argv[0] = openrc_sh;
	}

	einfov("Executing: %s %s %s", argv[0], service, command);
	अगर ((त्रुटि_सं = posix_spawn(&service_pid, argv[0], &tty, शून्य, UNCONST(argv), environ))) {
		eerror("%s: exec '%s': %s", service, argv[0], म_त्रुटि(त्रुटि_सं));
		वापस 1;
	}

	posix_spawn_file_actions_destroy(&tty);
	मुक्त(openrc_sh);

	fd[0].fd = signal_pipe[0];
	fd[1].fd = master_tty;
	fd[0].events = fd[1].events = POLLIN;
	fd[0].revents = fd[1].revents = 0;

	क्रम (;;) {
		अगर (poll(fd, master_tty >= 0 ? 2 : 1, -1) == -1) {
			अगर (त्रुटि_सं == EINTR)
				जारी;
			eerror("%s: poll: %s", applet, म_त्रुटि(त्रुटि_सं));
			अवरोध;
		}

		अगर (fd[1].revents & (POLLIN | POLLHUP)) {
			अक्षर बफर[बफ_मान];
			write_prefix(बफर, पढ़ो(master_tty, बफर, बफ_मान), &prefixed);
		}

		/* signal_pipe receives service_pid's exit status */
		अगर (fd[0].revents & (POLLIN | POLLHUP)) {
			अगर (त्रुटि_सं = 0, पढ़ो(signal_pipe[0], &ret, माप(ret)) < (ssize_t)माप(ret))
				eerror("%s: receive failed: %s", applet, त्रुटि_सं ? म_त्रुटि(त्रुटि_सं) : "short read");
			अन्यथा अगर ((ret = WEXITSTATUS(ret)) != 0 && त्रुटि_सं == ECHILD) /* killall5 -9 could cause this */
				ret = 0;
			अवरोध;
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

	sigprocmask (SIG_SETMASK, &oldmask, शून्य);

	अगर (master_tty >= 0) {
		/* Why did we do this? */
		/* signal (SIGWINCH, SIG_IGN); */
		बंद(master_tty);
		master_tty = -1;
	}

	service_pid = 0;

	वापस ret;
}

अटल bool
svc_wait(स्थिर अक्षर *svc)
{
	पूर्णांक fd;
	bool forever = false;
	RC_STRINGLIST *keywords;
	स्थिर अक्षर *base = basename_c(svc);
	पूर्णांक timeout = WAIT_TIMEOUT;
	bool retval = false;

	/* Some services don't have a timeout, like fsck */
	keywords = rc_deptree_depend(deptree, svc, "keyword");
	अगर (rc_stringlist_find(keywords, "-timeout") || rc_stringlist_find(keywords, "notimeout"))
		forever = true;
	rc_stringlist_free(keywords);

	अगर ((fd = openat(rc_dirfd(RC_DIR_EXCLUSIVE), base, O_RDONLY | O_NONBLOCK)) == -1)
		वापस त्रुटि_सं == ENOENT;

	timedout = false;
	क्रम (;;) {
		अगर (!forever)
			alarm(WARN_TIMEOUT);

		अगर (flock(fd, LOCK_SH) == 0) {
			retval = true;
			अवरोध;
		}

		अगर (त्रुटि_सं != EINTR)
			अवरोध;

		अगर (!forever && timedout) {
			timedout = false;
			अगर ((timeout -= WARN_TIMEOUT) > 0)
				ewarn("%s: waiting for %s (%d seconds)", applet, base, timeout);
			अन्यथा
				अवरोध;
		}
	}

	alarm(0);
	बंद(fd);
	वापस retval;
}

अटल व्योम
get_started_services(व्योम)
{
	RC_STRINGLIST *tmp = rc_services_in_state(RC_SERVICE_INACTIVE);

	rc_stringlist_free(restart_services);
	restart_services = rc_services_in_state(RC_SERVICE_STARTED);
	TAILQ_CONCAT(restart_services, tmp, entries);
	मुक्त(tmp);
}

अटल व्योम
setup_deptypes(व्योम)
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

अटल व्योम
svc_start_check(व्योम)
{
	पूर्णांक ret = निकास_त्रुटि;
	RC_SERVICE state;

	state = rc_service_state(applet);

	अगर (in_background) {
		अगर (!(state & (RC_SERVICE_INACTIVE | RC_SERVICE_STOPPED)))
			जाओ निकास;

		अगर (rc_yesno(दो_पर्या("IN_HOTPLUG")))
			rc_service_mark(applet, RC_SERVICE_HOTPLUGGED);
		अगर (म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0)
			ewarnx("WARNING: %s will be started in the next runlevel", applet);
	}

	अगर (state & RC_SERVICE_STARTED) {
		ewarn("WARNING: %s has already been started", applet);
		ret = निकास_सफल;
		जाओ निकास;
	} अन्यथा अगर (state & RC_SERVICE_INACTIVE && !in_background) {
		ewarnx("WARNING: %s has already started, but is inactive", applet);
	}

	अगर (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	अगर (exclusive_fd == -1) {
		अगर (त्रुटि_सं != EWOULDBLOCK) {
			eerror("%s: failed to acquire lock: %s", applet, म_त्रुटि(त्रुटि_सं));
			जाओ निकास;
		}

		अगर (state & RC_SERVICE_STOPPING)
			ewarnx("WARNING: %s is stopping", applet);
		अन्यथा
			ewarnx("WARNING: %s is already starting", applet);
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STARTING);
	rc_plugin_run(RC_HOOK_SERVICE_START_IN, applet);

	वापस;

निकास:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	निकास(ret);
}

अटल व्योम
svc_start_deps(व्योम)
{
	bool first;
	RC_STRING *svc, *svc2;
	RC_SERVICE state;
	पूर्णांक depoptions = RC_DEP_TRACE;
	माप_प्रकार len, svc_गिनती;
	अक्षर *tmp;
	खाता *mem;
	pid_t pid;

	त्रुटि_सं = 0;
	अगर (rc_conf_yesno("rc_depend_strict") || त्रुटि_सं == ENOENT)
		depoptions |= RC_DEP_STRICT;

	अगर (!deptree && ((deptree = _rc_deptree_load(0, शून्य)) == शून्य)) {
		eerror("failed to load deptree");
		जाओ निकास;
	}

	अगर (!deptypes_b)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_b, applet_list,
	    runlevel, 0);
	अगर (TAILQ_FIRST(services)) {
		eerrorn("ERROR: %s needs service(s) ", applet);
		first = true;
		TAILQ_FOREACH(svc, services, entries) {
			अगर (first)
				first = false;
			अन्यथा
				ख_लिखो(मानक_त्रुटि, ", ");
			ख_लिखो(मानक_त्रुटि, "%s", svc->मूल्य);
		}
		ख_लिखो(मानक_त्रुटि, "\n");
		जाओ निकास;
	}
	rc_stringlist_free(services);
	services = शून्य;

	need_services = rc_deptree_depends(deptree, deptypes_n, applet_list, runlevel, depoptions);
	want_services = rc_deptree_depends(deptree, deptypes_nw, applet_list, runlevel, depoptions);
	use_services = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, depoptions);

	अगर (!rc_runlevel_starting()) {
		TAILQ_FOREACH(svc, use_services, entries) {
			state = rc_service_state(svc->मूल्य);
			/* Don't stop failed services again.
			 * If you remove this check, ensure that the exclusive file isn't created. */
			अगर (state & RC_SERVICE_FAILED && rc_runlevel_starting())
				जारी;
			अगर (state & RC_SERVICE_STOPPED) {
				अगर (dry_run) {
					म_लिखो(" %s", svc->मूल्य);
					जारी;
				}
				/* FIXME(NRK): service_start() can return -1 on
				 * failure and feeding that to rc_waitpid seems
				 * very suspicious. needs investigation.
				 */
				pid = service_start(svc->मूल्य);
				अगर (!rc_conf_yesno("rc_parallel"))
					rc_waitpid(pid);
			}
		}
	}

	अगर (dry_run)
		वापस;

	/* Now wait for them to start */
	services = rc_deptree_depends(deptree, deptypes_nwua, applet_list, runlevel, depoptions);
	/* We use tmplist to hold our scheduled by list */
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH(svc, services, entries) {
		state = rc_service_state(svc->मूल्य);
		अगर (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->मूल्य);
			जारी;
		}

		/* Don't wait for services which went inactive but are
		 * now in starting state which we are after */
		अगर (state & RC_SERVICE_STARTING && state & RC_SERVICE_WASINACTIVE)
		{
			अगर (!rc_stringlist_find(need_services, svc->मूल्य) &&
					!rc_stringlist_find(want_services, svc->मूल्य) &&
					!rc_stringlist_find(use_services, svc->मूल्य))
				जारी;
		}

		अगर (!svc_wait(svc->मूल्य))
			eerror("%s: timed out waiting for %s", applet, svc->मूल्य);
		state = rc_service_state(svc->मूल्य);
		अगर (state & RC_SERVICE_STARTED) {
			svc_getenv(svc->मूल्य);
			जारी;
		}
		अगर (rc_stringlist_find(need_services, svc->मूल्य)) {
			अगर (state & (RC_SERVICE_INACTIVE | RC_SERVICE_WASINACTIVE)) {
				rc_stringlist_add(tmplist, svc->मूल्य);
			} अन्यथा अगर (!TAILQ_FIRST(tmplist)) {
				eerror("ERROR: cannot start %s as %s would not start", applet, svc->मूल्य);
				जाओ निकास;
			}
		}
	}

	अगर (TAILQ_FIRST(tmplist)) {
		/* Set the state now, then unlink our exclusive so that
		   our scheduled list is preserved */
		rc_service_mark(applet, RC_SERVICE_STOPPED);

		rc_stringlist_free(use_services);
		use_services = शून्य;
		mem = xopen_memstream(&tmp, &len);
		svc_गिनती = 0;
		TAILQ_FOREACH(svc, tmplist, entries) {
			rc_service_schedule_start(svc->मूल्य, applet);
			use_services = rc_deptree_depend(deptree, "iprovide", svc->मूल्य);
			TAILQ_FOREACH(svc2, use_services, entries)
			    rc_service_schedule_start(svc2->मूल्य, applet);
			rc_stringlist_free(use_services);
			use_services = शून्य;
			अगर (svc_गिनती > 0)
				ख_माला_दो(", ", mem);
			ख_माला_दो(svc->मूल्य, mem);
			++svc_गिनती;
		}
		xclose_memstream(mem);
		rc_stringlist_free(tmplist);
		tmplist = शून्य;
		ewarnx("WARNING: %s will start when %s %s started", applet, tmp, svc_गिनती > 1 ? "have" : "has");
		मुक्त(tmp);
	}

	rc_stringlist_free(tmplist);
	tmplist = शून्य;
	rc_stringlist_free(services);
	services = शून्य;

	वापस;

निकास:
	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	निकास(निकास_त्रुटि);
}


अटल व्योम set_reexports(व्योम)
{
	RC_STRINGLIST *list = rc_deptree_depend(deptree, applet, "reexport");
	RC_STRING *envvar;
	माप_प्रकार आकार;
	अक्षर *buf;
	खाता *fp;

	fp = xopen_memstream(&buf, &आकार);

	ख_माला_दो("RC_REEXPORT=", fp);
	TAILQ_FOREACH(envvar, list, entries)
		ख_लिखो(fp, "%s ", envvar->मूल्य);

	ख_बंद(fp);
	putenv(buf);
	rc_stringlist_free(list);
}

अटल व्योम svc_start_real(व्योम)
{
	bool started;
	RC_STRING *svc, *svc2;

	set_reexports();

	अगर (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_START_NOW, applet);
	skip_mark = false;
	started = (svc_exec("start") == 0);
	अगर (ibsave)
		unsetenv("IN_BACKGROUND");

	अगर (!started) {
		rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);
		rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
	}

	अगर (rc_service_state(applet) & RC_SERVICE_INACTIVE)
		ewarnx("WARNING: %s has started, but is inactive", applet);
	अन्यथा अगर (!started)
		eerrorx("ERROR: %s failed to start", applet);

	अगर (!skip_mark)
		rc_service_mark(applet, RC_SERVICE_STARTED);
	exclusive_fd = svc_unlock(applet, exclusive_fd);
	rc_plugin_run(RC_HOOK_SERVICE_START_DONE, applet);

	/* Now start any scheduled services */
	services = rc_services_scheduled(applet);
	TAILQ_FOREACH(svc, services, entries)
	    अगर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
		    service_start(svc->मूल्य);
	rc_stringlist_free(services);
	services = शून्य;

	/* Do the same for any services we provide */
	अगर (deptree) {
		tmplist = rc_deptree_depend(deptree, "iprovide", applet);
		TAILQ_FOREACH(svc, tmplist, entries) {
			services = rc_services_scheduled(svc->मूल्य);
			TAILQ_FOREACH(svc2, services, entries)
				अगर (rc_service_state(svc2->मूल्य) & RC_SERVICE_STOPPED)
					service_start(svc2->मूल्य);
			rc_stringlist_free(services);
			services = शून्य;
		}
		rc_stringlist_free(tmplist);
		tmplist = शून्य;
	}

	rc_plugin_run(RC_HOOK_SERVICE_START_OUT, applet);
}

अटल पूर्णांक
svc_start(व्योम)
{
	अगर (dry_run)
		einfon("start:");
	अन्यथा
		svc_start_check();

	अगर (deps)
		svc_start_deps();

	अगर (dry_run)
		म_लिखो(" %s\n", applet);
	अन्यथा
		svc_start_real();

	वापस 0;
}

अटल पूर्णांक
svc_stop_check(RC_SERVICE *state)
{
	*state = rc_service_state(applet);

	अगर (rc_runlevel_stopping() && *state & RC_SERVICE_FAILED)
		जाओ निकास;

	अगर (in_background && !(*state & (RC_SERVICE_STARTED) && !(*state & RC_SERVICE_INACTIVE)))
		जाओ निकास;

	अगर (*state & RC_SERVICE_STOPPED) {
		ewarn("WARNING: %s is already stopped", applet);
		वापस 1;
	}

	अगर (exclusive_fd == -1)
		exclusive_fd = svc_lock(applet, !deps);
	अगर (exclusive_fd == -1) {
		अगर (त्रुटि_सं != EWOULDBLOCK)
			eerror("%s: failed to acquire lock: %s", applet, म_त्रुटि(त्रुटि_सं));
		अन्यथा अगर (*state & RC_SERVICE_STOPPING)
			ewarn("WARNING: %s is already stopping", applet);
		अन्यथा
			eerror("ERROR: %s stopped by something else", applet);
		जाओ निकास;
	}
	fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);

	rc_service_mark(applet, RC_SERVICE_STOPPING);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_IN, applet);

	अगर (!rc_runlevel_stopping()) {
		अगर (rc_service_in_runlevel(applet, RC_LEVEL_SYSINIT))
			ewarn("WARNING: you are stopping a sysinit service");
		अन्यथा अगर (rc_service_in_runlevel(applet, RC_LEVEL_BOOT))
			ewarn("WARNING: you are stopping a boot service");
	}

	वापस 0;

निकास:
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
	निकास(निकास_त्रुटि);
}

अटल व्योम
svc_stop_deps(RC_SERVICE state)
{
	पूर्णांक depoptions = RC_DEP_TRACE;
	RC_STRING *svc;
	pid_t pid;

	अगर (state & RC_SERVICE_WASINACTIVE)
		वापस;

	त्रुटि_सं = 0;
	अगर (rc_conf_yesno("rc_depend_strict") || त्रुटि_सं == ENOENT)
		depoptions |= RC_DEP_STRICT;

	अगर (!deptree && ((deptree = _rc_deptree_load(0, शून्य)) == शून्य)) {
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("failed to load deptree");
	}

	अगर (!deptypes_m)
		setup_deptypes();

	services = rc_deptree_depends(deptree, deptypes_m, applet_list, runlevel, depoptions);
	tmplist = rc_stringlist_new();
	TAILQ_FOREACH_REVERSE(svc, services, rc_stringlist, entries) {
		state = rc_service_state(svc->मूल्य);
		/* Don't stop failed services again.
		 * If you remove this check, ensure that the exclusive file isn't created. */
		अगर (state & RC_SERVICE_FAILED && rc_runlevel_stopping())
			जारी;
		अगर (!(state & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)))
			जारी;
		अगर (dry_run) {
			म_लिखो(" %s", svc->मूल्य);
			जारी;
		}
		svc_wait(svc->मूल्य);
		अगर (rc_service_state(svc->मूल्य) & (RC_SERVICE_STARTED | RC_SERVICE_INACTIVE)) {
			pid = service_stop(svc->मूल्य);
			अगर (!rc_conf_yesno("rc_parallel"))
				rc_waitpid(pid);
			rc_stringlist_add(tmplist, svc->मूल्य);
		}
	}
	rc_stringlist_free(services);
	services = शून्य;
	अगर (dry_run)
		वापस;

	TAILQ_FOREACH(svc, tmplist, entries) {
		अगर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED) {
			जारी;
		}
		svc_wait(svc->मूल्य);
		अगर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
			जारी;
		अगर (rc_runlevel_stopping()) {
			/* If shutting down, we should stop even
			 * if a dependent failed */
			अगर (runlevel && (म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0 || म_भेद(runlevel, RC_LEVEL_SINGLE) == 0))
				जारी;
			rc_service_mark(applet, RC_SERVICE_FAILED);
		}
		rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);
		eerrorx("ERROR: cannot stop %s as %s is still up", applet, svc->मूल्य);
	}
	rc_stringlist_free(tmplist);
	tmplist = शून्य;

	/* We now wait for other services that may use us and are
	 * stopping. This is important when a runlevel stops */
	services = rc_deptree_depends(deptree, deptypes_mwua, applet_list, runlevel, depoptions);
	TAILQ_FOREACH(svc, services, entries) {
		अगर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
			जारी;
		svc_wait(svc->मूल्य);
	}
	rc_stringlist_free(services);
	services = शून्य;
}

अटल व्योम
load_dep_env(व्योम)
{
	RC_STRINGLIST *depsvcs;
	RC_STRING *svc;

	depsvcs = rc_deptree_depends(deptree, deptypes_nwu, applet_list, runlevel, 0);

	TAILQ_FOREACH(svc, depsvcs, entries)
		svc_getenv(svc->मूल्य);

	rc_stringlist_free(depsvcs);
}

अटल व्योम
svc_stop_real(व्योम)
{
	bool stopped;

	load_dep_env();

	/* If we're stopping localmount, set LC_ALL=C so that
	 * bash doesn't load anything blocking the unmounting of /usr */
	अगर (म_भेद(applet, "localmount") == 0)
		setenv("LC_ALL", "C", 1);

	अगर (ibsave)
		setenv("IN_BACKGROUND", ibsave, 1);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_NOW, applet);
	skip_mark = false;
	stopped = (svc_exec("stop") == 0);
	अगर (ibsave)
		unsetenv("IN_BACKGROUND");

	अगर (stopped && !skip_mark)
		rc_service_mark(applet, in_background ? RC_SERVICE_INACTIVE : RC_SERVICE_STOPPED);

	rc_plugin_run(RC_HOOK_SERVICE_STOP_DONE, applet);
	rc_plugin_run(RC_HOOK_SERVICE_STOP_OUT, applet);

	अगर (!stopped)
		eerrorx("ERROR: %s failed to stop", applet);
}

अटल पूर्णांक
svc_stop(व्योम)
{
	RC_SERVICE state = 0;

	अगर (dry_run)
		einfon("stop:");
	अन्यथा
		अगर (svc_stop_check(&state) == 1)
			वापस 1; /* Service has been stopped already */

	अगर (deps)
		svc_stop_deps(state);

	अगर (dry_run)
		म_लिखो(" %s\n", applet);
	अन्यथा
		svc_stop_real();

	वापस 0;
}

अटल पूर्णांक
svc_restart(व्योम)
{
	अगर (!(rc_service_state(applet) & RC_SERVICE_STOPPED)) {
		get_started_services();
		svc_stop();
		अगर (dry_run)
			ewarn("Cannot calculate restart start dependencies on a dry-run");
	}

	svc_start();
	start_services(restart_services);
	rc_stringlist_free(restart_services);
	restart_services = शून्य;

	वापस 0;
}

अटल bool
service_plugable(व्योम)
{
	अक्षर *list, *p, *token;
	bool allow = true, truefalse;
	अक्षर *match = rc_conf_मूल्य("rc_hotplug");

	अगर (!match)
		match = rc_conf_मूल्य("rc_plug_services");
	अगर (!match)
		वापस false;

	list = xstrdup(match);
	p = list;
	जबतक ((token = strsep(&p, " "))) {
		अगर (token[0] == '!') {
			truefalse = false;
			token++;
		} अन्यथा {
			truefalse = true;
		}

		अगर (fnmatch(token, applet, 0) == 0) {
			allow = truefalse;
			अवरोध;
		}
	}
	मुक्त(list);
	वापस allow;
}

अटल पूर्णांक
simple_command(व्योम)
{
	अक्षर *save = prefix;
	पूर्णांक retval;

	eprefix(शून्य);
	prefix = शून्य;

	retval = svc_exec(optarg);

	eprefix(save);
	prefix = save;

	वापस retval;
}

अटल पूर्णांक
resolve_deptype(व्योम)
{
	पूर्णांक depoptions = RC_DEP_TRACE;
	RC_STRING *svc;

	त्रुटि_सं = 0;
	अगर (rc_conf_yesno("rc_depend_strict") || त्रुटि_सं == ENOENT)
		depoptions |= RC_DEP_STRICT;

	अगर (!deptree && ((deptree = _rc_deptree_load(0, शून्य)) == शून्य))
		eerrorx("failed to load deptree");

	tmplist = rc_stringlist_new();
	rc_stringlist_add(tmplist, optarg);
	services = rc_deptree_depends(deptree, tmplist, applet_list, runlevel, depoptions);
	rc_stringlist_free(tmplist);

	tmplist = शून्य;
	TAILQ_FOREACH(svc, services, entries)
		म_लिखो("%s ", svc->मूल्य);
	म_लिखो ("\n");

	rc_stringlist_free(services);
	services = शून्य;

	वापस 0;
}

अटल पूर्णांक
cmd_condrestart(व्योम)
{
	अगर (rc_service_state(applet) & RC_SERVICE_STARTED)
		svc_restart();
	वापस 0;
}

अटल पूर्णांक
cmd_stop(व्योम)
{
	RC_STRING *svc;

	अगर (म_भेद(optarg, "pause") == 0) {
		ewarn("WARNING: 'pause' is deprecated; please use '--nodeps stop'");
		deps = false;
	}

	अगर (deps && in_background)
		get_started_services();

	अगर (svc_stop() == 1)
		वापस 0; /* Service has been stopped already */

	अगर (deps) {
		अगर (!in_background && !rc_runlevel_stopping() && rc_service_state(applet) & RC_SERVICE_STOPPED)
			unhotplug();

		अगर (in_background && rc_service_state(applet) & RC_SERVICE_INACTIVE) {
			TAILQ_FOREACH(svc, restart_services, entries)
				अगर (rc_service_state(svc->मूल्य) & RC_SERVICE_STOPPED)
					rc_service_schedule_start(applet, svc->मूल्य);
		}
	}

	वापस 0;
}

अटल पूर्णांक
cmd_zap(व्योम)
{
	einfo("Manually resetting %s to stopped state", applet);
	अगर (!rc_service_mark(applet, RC_SERVICE_STOPPED))
		eerrorx("rc_service_mark: %s", म_त्रुटि(त्रुटि_सं));
	unhotplug();

	वापस 0;
}

काष्ठा {
	स्थिर अक्षर *name;
	पूर्णांक (*handler)(व्योम);
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

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक retval, opt;
	स्थिर अक्षर *workingdir = "/";
	अक्षर *pidstr = शून्य;
	माप_प्रकार l = 0, ll;
	काष्ठा stat stbuf;

	/* Show help if insufficient args */
	अगर (argc < 2 || access(argv[1], F_OK) != 0) {
		ख_लिखो(मानक_त्रुटि, "openrc-run should not be run directly\n");
		निकास(निकास_त्रुटि);
	}

	applet = basename_c(argv[0]);

	अगर (stat(argv[1], &stbuf) != 0) {
		ख_लिखो(मानक_त्रुटि, "openrc-run '%s': %s\n", argv[1], म_त्रुटि(त्रुटि_सं));
		निकास(निकास_त्रुटि);
	}

	अगर (!(service = (realpath(argv[1], शून्य))))
		eerrorx("readpath: %s", म_त्रुटि(त्रुटि_सं));
	applet = basename_c(argv[1]);

	अगर (argc < 3)
		usage(निकास_त्रुटि);

	/* Ok, we are ready to go, so setup selinux if applicable */
	selinux_setup(argv);

	deps = true;

	/* Punt the first arg as its our service name */
	argc--;
	argv++;

	/* Right then, parse any options there may be */
	जबतक ((opt = getopt_long(argc, argv, getoptstring, longopts, शून्य)) != -1)
		चयन (opt) {
		हाल 'd':
			setenv("RC_DEBUG", "YES", 1);
			अवरोध;
		हाल 'l':
			exclusive_fd = म_से_प(optarg);
			fcntl(exclusive_fd, F_SETFD, fcntl(exclusive_fd, F_GETFD, 0) | FD_CLOEXEC);
			अवरोध;
		हाल 's':
			अगर (!(rc_service_state(applet) & RC_SERVICE_STARTED))
				निकास(निकास_त्रुटि);
			अवरोध;
		हाल 'S':
			अगर (!(rc_service_state(applet) & RC_SERVICE_STOPPED))
				निकास(निकास_त्रुटि);
			अवरोध;
		हाल 'D':
			deps = false;
			अवरोध;
		हाल 'Z':
			dry_run = true;
			अवरोध;
		case_RC_COMMON_GETOPT
		}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	अगर (rc_is_user() && !(workingdir = दो_पर्या("HOME")))
		eerrorx("HOME is unset.");

	अगर (स_बदलो(workingdir) == -1)
		eerror("chdir: %s", म_त्रुटि(त्रुटि_सं));

	निकास_पर(cleanup);

	अगर ((runlevel = xstrdup(दो_पर्या("RC_RUNLEVEL"))) == शून्य) {
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
	अगर (rc_conf_yesno("rc_parallel")) {
		RC_STRING *svc;
		/* Get the longest service name */
		services = rc_services_in_runlevel(शून्य);
		TAILQ_FOREACH(svc, services, entries) {
			ll = म_माप(svc->मूल्य);
			अगर (ll > l)
				l = ll;
		}
		rc_stringlist_free(services);
		services = शून्य;
		ll = म_माप(applet);
		अगर (ll > l)
			l = ll;

		/* Make our prefix string */
		prefix = xmalloc(माप(अक्षर) * l + 1);
		स_नकल(prefix, applet, ll);
		स_रखो(prefix + ll, ' ', l - ll);
		स_रखो(prefix + l, 0, 1);
		eprefix(prefix);
	}

	अगर (rc_yesno(दो_पर्या("RC_NODEPS")))
		deps = false;

	/* If we're changing runlevels and not called by rc then we cannot
	   work with any dependencies */
	अगर (deps && दो_पर्या("RC_PID") == शून्य && (rc_runlevel_starting() || rc_runlevel_stopping()))
		deps = false;

	/* Save the IN_BACKGROUND env flag so it's ONLY passed to the service
	   that is being called and not any dependents */
	अगर (दो_पर्या("IN_BACKGROUND")) {
		ibsave = xstrdup(दो_पर्या("IN_BACKGROUND"));
		in_background = rc_yesno(ibsave);
		unsetenv("IN_BACKGROUND");
	}

	अगर (rc_yesno(दो_पर्या("IN_DRYRUN")))
		dry_run = true;
	अगर (rc_yesno(दो_पर्या("IN_HOTPLUG"))) {
		अगर (!service_plugable())
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

	अगर (optind >= argc)
		usage(निकास_त्रुटि);

	/* Now run each option */
	retval = निकास_सफल;

अग्र_cmd:
	जबतक (optind < argc) {
		पूर्णांक cmdval;
		optarg = argv[optind++];

		/* Abort on a sighup here */
		अगर (sighup)
			निकास (निकास_त्रुटि);

		/* Export the command we're running.
		   This is important as we stamp on the restart function now but
		   some start/stop routines still need to behave differently if
		   restarting. */
		setenv("RC_CMD", optarg, true);

		क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(commands); i++) {
			अगर (म_भेद(optarg, commands[i].name) != 0)
				जारी;

			अगर ((cmdval = commands[i].handler()) != 0)
				retval = cmdval;

			अगर (commands[i].reset_restart_list) {
				/* We should ensure this list is empty after
				 * an action is done */
				rc_stringlist_free(restart_services);
				restart_services = शून्य;
			}

			जाओ अग्र_cmd;
		}

		अगर ((cmdval = svc_exec(optarg)) != 0)
			retval = cmdval;
		rc_stringlist_free(restart_services);
		restart_services = शून्य;
	}

	वापस retval;
}
