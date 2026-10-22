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

#समावेश <errno.h>
#समावेश <pwd.h>
#समावेश <signal.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>
#समावेश <fcntl.h>
#समावेश <unistd.h>
#समावेश <sys/stat.h>
#समावेश <sys/reboot.h>
#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <poll.h>
#समावेश <utmp.h>

#अगर_घोषित HAVE_SELINUX
#  include <selinux/selinux.h>
#पूर्ण_अगर

#समावेश "rc.h"
#समावेश "rc_exec.h"
#समावेश "wtmp.h"
#समावेश "helpers.h"
#समावेश "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#अगर defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#पूर्ण_अगर

अटल स्थिर अक्षर *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
अटल स्थिर अक्षर *rc_default_runlevel = "default";
अटल पूर्णांक sigpipe[2] = { -1, -1 };
अटल bool quiet = false;
अटल स्थिर अक्षर *progname;

अटल व्योम do_openrc(स्थिर अक्षर *runlevel)
{
	स्थिर अक्षर *argv[] = { "openrc", runlevel, शून्य };
	काष्ठा exec_परिणाम res;
	काष्ठा exec_args args = exec_init(argv);
	args.setsid = 1;

	अगर (!quiet)
		म_लिखो("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	अगर (res.pid < 0) {
		लिखो_त्रुटि("do_exec");
		निकास(1);
	}
	जबतक (waitpid(res.pid, शून्य, 0) != res.pid)
		अगर (त्रुटि_सं == ECHILD)
			अवरोध;
}

अटल व्योम init(स्थिर अक्षर *default_runlevel)
{
	स्थिर अक्षर *runlevel = शून्य;
	do_openrc("sysinit");
	do_openrc("boot");
	अगर (default_runlevel)
		runlevel = default_runlevel;
	अन्यथा
		runlevel = rc_conf_मूल्य("rc_default_runlevel");
	अगर (!runlevel)
		runlevel = rc_default_runlevel;
	अगर (!rc_runlevel_exists(runlevel)) {
		ख_लिखो(मानक_त्रुटि, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

अटल व्योम handle_reexec(व्योम)
{
	execlp(progname, progname, "reexec", शून्य);
	वापस;
}

अटल व्योम handle_shutdown(स्थिर अक्षर *runlevel, पूर्णांक cmd)
{
	स्थिर दीर्घ ms_to_ns = 1000000L;
	काष्ठा timespec start, now, tmp, remain;

	do_openrc(runlevel);
	अगर (!quiet)
		म_लिखो("Sending the final TERM signal\n");
	समाप्त(-1, संक_इति);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	जबतक (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && त्रुटि_सं == EINTR)
		tmp = remain;

	क्रम (;;) {
		pid_t pid = waitpid(-1, शून्य, WNOHANG);
		अगर (pid == -1 && त्रुटि_सं == ECHILD) {
			/* all children reaped, safe to exit */
			अवरोध;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		अगर (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			अवरोध;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, शून्य);
	}

	अगर (!quiet)
		म_लिखो("Sending the final KILL signal\n");
	समाप्त(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	अगर (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

अटल व्योम run_program(स्थिर अक्षर *prog)
{
	स्थिर अक्षर *argv[] = { prog, शून्य };
	काष्ठा exec_args args = exec_init(argv);
	काष्ठा exec_परिणाम res = do_exec(&args);
	अगर (res.pid < 0 || rc_waitpid(res.pid) == -1)
		लिखो_त्रुटि("init");
}

अटल व्योम open_shell(व्योम)
{
	स्थिर अक्षर *shell;
	काष्ठा passwd *pw;

#अगर_घोषित __linux__
	स्थिर अक्षर *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	अगर (sys && म_भेद(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (अक्षर *) शून्य);
		लिखो_त्रुटि("init");
		वापस;
	}
#पूर्ण_अगर

	shell = rc_conf_मूल्य("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	अगर (!shell) {
		shell = दो_पर्या("SHELL");
		अगर (!shell) {
			pw = getpwuid(getuid());
			अगर (pw)
				shell = pw->pw_shell;
			अगर (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

अटल व्योम handle_single(व्योम)
{
	do_openrc("single");
	open_shell();
	init(शून्य);
}

अटल व्योम dispatch_signal(पूर्णांक sigfd)
{
	पूर्णांक sig;
	ssize_t गिनती = पढ़ो(sigfd, &sig, माप(sig));

	अगर (गिनती < 0) {
		लिखो_त्रुटि("read(sigpipe)");
		वापस;
	}

	अगर (गिनती != माप(sig)) {
		ख_माला_दो("bad read from sigpipe\n", मानक_त्रुटि);
		वापस;
	}

	चयन (sig) {
		हाल संक_विघ्न:
			handle_shutdown("reboot", RB_AUTOBOOT);
			अवरोध;
		हाल संक_इति:
#अगर_घोषित SIGPWR
		हाल SIGPWR:
#पूर्ण_अगर
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			अवरोध;
		शेष:
			ख_लिखो(मानक_त्रुटि, "Unknown signal received, %d\n", sig);
			अवरोध;
	}
}

अटल व्योम dispatch_fifo(पूर्णांक fifo)
{
	अक्षर buf[2048];
	ssize_t गिनती = पढ़ो(fifo, buf, माप(buf) - 1);

	अगर (गिनती < 0) {
		लिखो_त्रुटि("read(fifo)");
		वापस;
	}

	buf[गिनती] = 0;

	अगर (!quiet)
		म_लिखो("PID1: Received \"%s\" from FIFO...\n", buf);

	अगर (म_भेद(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	अन्यथा अगर (म_भेद(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	अन्यथा अगर (म_भेद(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	अन्यथा अगर (म_भेद(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	अन्यथा अगर (म_भेद(buf, "reexec") == 0)
		handle_reexec();
	अन्यथा अगर (म_भेद(buf, "single") == 0)
		handle_single();
}

अटल व्योम signal_handler(पूर्णांक sig)
{
	पूर्णांक saved_errno = त्रुटि_सं;
	ग_लिखो(sigpipe[1], &sig, माप(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	त्रुटि_सं = saved_errno;
}

अटल व्योम reap_zombies(पूर्णांक sig RC_UNUSED)
{
	अक्षर errmsg[] = "waitpid() failed\n";
	पूर्णांक saved_errno = त्रुटि_सं;

	क्रम (;;) {
		pid_t pid = waitpid(-1, शून्य, WNOHANG);
		अगर (pid == 0)
			अवरोध;
		अन्यथा अगर (pid == -1) {
			अगर (त्रुटि_सं == ECHILD)
				अवरोध;
			ग_लिखो(STDERR_FILENO, errmsg, माप(errmsg) - 1);
			जारी;
		}
	}
	त्रुटि_सं = saved_errno;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *cmdline_runlevel = शून्य;
	पूर्णांक fifo;
	bool reexec = false;
	sigset_t signals;
	काष्ठा sigaction sa;
#अगर_घोषित HAVE_SELINUX
	पूर्णांक enforce = 0;
#पूर्ण_अगर

	progname = argv[0];

	अगर (getpid() != 1)
		वापस 1;

#अगर_घोषित HAVE_SELINUX
	अगर (दो_पर्या("SELINUX_INIT") == शून्य) {
		अगर (is_selinux_enabled() != 1) {
			अगर (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} अन्यथा {
				अगर (enforce > 0) {
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
#पूर्ण_अगर

	quiet = rc_yesno(दो_पर्या("EINFO_QUIET"));

	अगर (!quiet)
		म_लिखो("OpenRC init version %s starting\n", VERSION);

	अगर (argc > 1) {
		अगर (म_भेद(argv[1], "reexec") == 0)
			reexec = true;
		अन्यथा
			cmdline_runlevel = argv[1];
	}

	अगर (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		लिखो_त्रुटि("pipe2");
		वापस 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, संक_विघ्न);
	sigdelset(&signals, संक_इति);
#अगर_घोषित SIGPWR
	sigdelset(&signals, SIGPWR);
#पूर्ण_अगर
	sigprocmask(SIG_SETMASK, &signals, शून्य);

	/* install signal  handler */
	स_रखो(&sa, 0, माप(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, शून्य);
	sa.sa_handler = signal_handler;
	sigaction(संक_विघ्न, &sa, शून्य);
	sigaction(संक_इति, &sa, शून्य);
#अगर_घोषित SIGPWR
	sigaction(SIGPWR, &sa, शून्य);
#पूर्ण_अगर
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	अगर (!reexec)
		init(cmdline_runlevel);

	अगर ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		लिखो_त्रुटि("open_rwfifo");

	क्रम (;;) {
		क्रमागत { FD_FIFO, FD_SIG, FD_गिनती };
		काष्ठा pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_गिनती, -1);

		अगर (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		अगर (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	वापस 0;
}
