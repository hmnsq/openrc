/*
 * openrc-init.c
 * This is the init process (pid 1) for OpenRC.
 *
 * This is based on code written by James Hammons <jlhamm@acm.org>, so
 * I would like to publically thank him for his work.
 */

/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#अन्तर्भाव <errno.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/reboot.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <poll.h>
#अन्तर्भाव <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "wtmp.h"
#अन्तर्भाव "helpers.h"
#अन्तर्भाव "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#जर defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

स्थिर स्थिरांक अक्षर *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
स्थिर स्थिरांक अक्षर *rc_default_runlevel = "default";
स्थिर पूर्णांक sigpipe[2] = { -1, -1 };
स्थिर bool quiet = false;
स्थिर स्थिरांक अक्षर *progname;

स्थिर शून्य do_openrc(स्थिरांक अक्षर *runlevel)
{
	स्थिरांक अक्षर *argv[] = { "openrc", runlevel, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);
	args.setsid = 1;

	जर (!quiet)
		म_लिखो("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	जर (res.pid < 0) {
		लिखो_त्रुटि("do_exec");
		निकास(1);
	}
	पर्यंत (waitpid(res.pid, रिक्त, 0) != res.pid)
		जर (त्रुटि_सं == ECHILD)
			थांबा;
}

स्थिर शून्य init(स्थिरांक अक्षर *default_runlevel)
{
	स्थिरांक अक्षर *runlevel = रिक्त;
	do_openrc("sysinit");
	do_openrc("boot");
	जर (default_runlevel)
		runlevel = default_runlevel;
	अन्यथा
		runlevel = rc_conf_मूल्य("rc_default_runlevel");
	जर (!runlevel)
		runlevel = rc_default_runlevel;
	जर (!rc_runlevel_exists(runlevel)) {
		ख_लिखो(मानक_त्रुटि, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

स्थिर शून्य handle_reexec(शून्य)
{
	execlp(progname, progname, "reexec", रिक्त);
	परत;
}

स्थिर शून्य handle_shutdown(स्थिरांक अक्षर *runlevel, पूर्णांक cmd)
{
	स्थिरांक दीर्घ ms_to_ns = 1000000L;
	संरचना timespec start, now, tmp, remain;

	do_openrc(runlevel);
	जर (!quiet)
		म_लिखो("Sending the final TERM signal\n");
	समाप्त(-1, संक_इति);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	पर्यंत (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && त्रुटि_सं == EINTR)
		tmp = remain;

	साठी (;;) {
		pid_t pid = waitpid(-1, रिक्त, WNOHANG);
		जर (pid == -1 && त्रुटि_सं == ECHILD) {
			/* all children reaped, safe to exit */
			थांबा;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		जर (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			थांबा;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, रिक्त);
	}

	जर (!quiet)
		म_लिखो("Sending the final KILL signal\n");
	समाप्त(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	जर (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

स्थिर शून्य run_program(स्थिरांक अक्षर *prog)
{
	स्थिरांक अक्षर *argv[] = { prog, रिक्त };
	संरचना exec_args args = exec_init(argv);
	संरचना exec_result res = do_exec(&args);
	जर (res.pid < 0 || rc_waitpid(res.pid) == -1)
		लिखो_त्रुटि("init");
}

स्थिर शून्य open_shell(शून्य)
{
	स्थिरांक अक्षर *shell;
	संरचना passwd *pw;

#ifdef __linux__
	स्थिरांक अक्षर *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	जर (sys && म_भेद(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (अक्षर *) रिक्त);
		लिखो_त्रुटि("init");
		परत;
	}
#endif

	shell = rc_conf_मूल्य("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	जर (!shell) {
		shell = दो_पर्या("SHELL");
		जर (!shell) {
			pw = getpwuid(getuid());
			जर (pw)
				shell = pw->pw_shell;
			जर (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

स्थिर शून्य handle_single(शून्य)
{
	do_openrc("single");
	open_shell();
	init(रिक्त);
}

स्थिर शून्य dispatch_signal(पूर्णांक sigfd)
{
	पूर्णांक sig;
	ssize_t मोजणी = पठ(sigfd, &sig, आकार(sig));

	जर (मोजणी < 0) {
		लिखो_त्रुटि("read(sigpipe)");
		परत;
	}

	जर (मोजणी != आकार(sig)) {
		ख_माला_दो("bad read from sigpipe\n", मानक_त्रुटि);
		परत;
	}

	निवड (sig) {
		स्थिती संक_विघ्न:
			handle_shutdown("reboot", RB_AUTOBOOT);
			थांबा;
		स्थिती संक_इति:
#ifdef SIGPWR
		स्थिती SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			थांबा;
		पूर्वनिर्धारित:
			ख_लिखो(मानक_त्रुटि, "Unknown signal received, %d\n", sig);
			थांबा;
	}
}

स्थिर शून्य dispatch_fifo(पूर्णांक fifo)
{
	अक्षर buf[2048];
	ssize_t मोजणी = पठ(fifo, buf, आकार(buf) - 1);

	जर (मोजणी < 0) {
		लिखो_त्रुटि("read(fifo)");
		परत;
	}

	buf[मोजणी] = 0;

	जर (!quiet)
		म_लिखो("PID1: Received \"%s\" from FIFO...\n", buf);

	जर (म_भेद(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	अन्यथा जर (म_भेद(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	अन्यथा जर (म_भेद(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	अन्यथा जर (म_भेद(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	अन्यथा जर (म_भेद(buf, "reexec") == 0)
		handle_reexec();
	अन्यथा जर (म_भेद(buf, "single") == 0)
		handle_single();
}

स्थिर शून्य signal_handler(पूर्णांक sig)
{
	पूर्णांक saved_errno = त्रुटि_सं;
	लिख(sigpipe[1], &sig, आकार(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	त्रुटि_सं = saved_errno;
}

स्थिर शून्य reap_zombies(पूर्णांक sig RC_UNUSED)
{
	अक्षर errmsg[] = "waitpid() failed\n";
	पूर्णांक saved_errno = त्रुटि_सं;

	साठी (;;) {
		pid_t pid = waitpid(-1, रिक्त, WNOHANG);
		जर (pid == 0)
			थांबा;
		अन्यथा जर (pid == -1) {
			जर (त्रुटि_सं == ECHILD)
				थांबा;
			लिख(STDERR_FILENO, errmsg, आकार(errmsg) - 1);
			सुरूठेवा;
		}
	}
	त्रुटि_सं = saved_errno;
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *cmdline_runlevel = रिक्त;
	पूर्णांक fifo;
	bool reexec = false;
	sigset_t signals;
	संरचना sigaction sa;
#ifdef HAVE_SELINUX
	पूर्णांक enforce = 0;
#endif

	progname = argv[0];

	जर (getpid() != 1)
		परत 1;

#ifdef HAVE_SELINUX
	जर (दो_पर्या("SELINUX_INIT") == रिक्त) {
		जर (is_selinux_enabled() != 1) {
			जर (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} अन्यथा {
				जर (enforce > 0) {
					/*
					 * SELinux in enforcing mode but load_policy failed
					 * At this point, we probably can't open /dev/console,
					 * so log() won't work
					 */
					ख_लिखो(मानक_त्रुटि,"Unable to load SELinux Policy.\n");
					ख_लिखो(मानक_त्रुटि,"Machine is  in enforcing mode.\n");
					ख_लिखो(मानक_त्रुटि,"Halting now.\n");
					निकास(1);
				}
			}
		}
	}
#endif

	quiet = rc_yesno(दो_पर्या("EINFO_QUIET"));

	जर (!quiet)
		म_लिखो("OpenRC init version %s starting\n", VERSION);

	जर (argc > 1) {
		जर (म_भेद(argv[1], "reexec") == 0)
			reexec = true;
		अन्यथा
			cmdline_runlevel = argv[1];
	}

	जर (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		लिखो_त्रुटि("pipe2");
		परत 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, संक_विघ्न);
	sigdelset(&signals, संक_इति);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, रिक्त);

	/* install signal  handler */
	स_रखो(&sa, 0, आकार(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, रिक्त);
	sa.sa_handler = signal_handler;
	sigaction(संक_विघ्न, &sa, रिक्त);
	sigaction(संक_इति, &sa, रिक्त);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, रिक्त);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	जर (!reexec)
		init(cmdline_runlevel);

	जर ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		लिखो_त्रुटि("open_rwfifo");

	साठी (;;) {
		गणना { FD_FIFO, FD_SIG, FD_मोजणी };
		संरचना pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_मोजणी, -1);

		जर (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		जर (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	परत 0;
}
