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
#सचे defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

स्थिर ध्रुव वर्ण *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
स्थिर ध्रुव वर्ण *rc_default_runlevel = "default";
स्थिर पूर्णाङ्क sigpipe[2] = { -1, -1 };
स्थिर bool quiet = false;
स्थिर ध्रुव वर्ण *progname;

स्थिर शून्य do_openrc(ध्रुव वर्ण *runlevel)
{
	ध्रुव वर्ण *argv[] = { "openrc", runlevel, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);
	args.setsid = 1;

	सचे (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	सचे (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	याव (waitpid(res.pid, रिक्त, 0) != res.pid)
		सचे (errno == ECHILD)
			भङ्ग;
}

स्थिर शून्य init(ध्रुव वर्ण *default_runlevel)
{
	ध्रुव वर्ण *runlevel = रिक्त;
	do_openrc("sysinit");
	do_openrc("boot");
	सचे (default_runlevel)
		runlevel = default_runlevel;
	अञ्ञथा
		runlevel = rc_conf_मानम्("rc_default_runlevel");
	सचे (!runlevel)
		runlevel = rc_default_runlevel;
	सचे (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

स्थिर शून्य handle_reexec(शून्य)
{
	execlp(progname, progname, "reexec", रिक्त);
	पटिनिवत्त;
}

स्थिर शून्य handle_shutdown(ध्रुव वर्ण *runlevel, पूर्णाङ्क cmd)
{
	ध्रुव long ms_to_ns = 1000000L;
	संरचना timespec start, now, tmp, remain;

	do_openrc(runlevel);
	सचे (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	याव (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	अत्थाय (;;) {
		pid_t pid = waitpid(-1, रिक्त, WNOHANG);
		सचे (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			भङ्ग;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		सचे (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			भङ्ग;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, रिक्त);
	}

	सचे (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	सचे (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

स्थिर शून्य run_program(ध्रुव वर्ण *prog)
{
	ध्रुव वर्ण *argv[] = { prog, रिक्त };
	संरचना exec_args args = exec_init(argv);
	संरचना exec_result res = do_exec(&args);
	सचे (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

स्थिर शून्य open_shell(शून्य)
{
	ध्रुव वर्ण *shell;
	संरचना passwd *pw;

#ifdef __linux__
	ध्रुव वर्ण *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	सचे (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (वर्ण *) रिक्त);
		perror("init");
		पटिनिवत्त;
	}
#endif

	shell = rc_conf_मानम्("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	सचे (!shell) {
		shell = getenv("SHELL");
		सचे (!shell) {
			pw = getpwuid(getuid());
			सचे (pw)
				shell = pw->pw_shell;
			सचे (!shell)
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

स्थिर शून्य dispatch_signal(पूर्णाङ्क sigfd)
{
	पूर्णाङ्क sig;
	ssize_t गणनम् = पठ(sigfd, &sig, परिमाण(sig));

	सचे (गणनम् < 0) {
		perror("read(sigpipe)");
		पटिनिवत्त;
	}

	सचे (गणनम् != परिमाण(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		पटिनिवत्त;
	}

	विकल्प (sig) {
		पक्ष SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			भङ्ग;
		पक्ष SIGTERM:
#ifdef SIGPWR
		पक्ष SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			भङ्ग;
		स्वभाव:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			भङ्ग;
	}
}

स्थिर शून्य dispatch_fifo(पूर्णाङ्क fifo)
{
	वर्ण buf[2048];
	ssize_t गणनम् = पठ(fifo, buf, परिमाण(buf) - 1);

	सचे (गणनम् < 0) {
		perror("read(fifo)");
		पटिनिवत्त;
	}

	buf[गणनम्] = 0;

	सचे (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	सचे (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	अञ्ञथा सचे (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	अञ्ञथा सचे (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	अञ्ञथा सचे (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	अञ्ञथा सचे (strcmp(buf, "reexec") == 0)
		handle_reexec();
	अञ्ञथा सचे (strcmp(buf, "single") == 0)
		handle_single();
}

स्थिर शून्य signal_handler(पूर्णाङ्क sig)
{
	पूर्णाङ्क saved_errno = errno;
	लिख(sigpipe[1], &sig, परिमाण(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

स्थिर शून्य reap_zombies(पूर्णाङ्क sig RC_UNUSED)
{
	वर्ण errmsg[] = "waitpid() failed\n";
	पूर्णाङ्क saved_errno = errno;

	अत्थाय (;;) {
		pid_t pid = waitpid(-1, रिक्त, WNOHANG);
		सचे (pid == 0)
			भङ्ग;
		अञ्ञथा सचे (pid == -1) {
			सचे (errno == ECHILD)
				भङ्ग;
			लिख(STDERR_FILENO, errmsg, परिमाण(errmsg) - 1);
			अनुवर्त;
		}
	}
	errno = saved_errno;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *cmdline_runlevel = रिक्त;
	पूर्णाङ्क fifo;
	bool reexec = false;
	sigset_t signals;
	संरचना sigaction sa;
#ifdef HAVE_SELINUX
	पूर्णाङ्क enforce = 0;
#endif

	progname = argv[0];

	सचे (getpid() != 1)
		पटिनिवत्त 1;

#ifdef HAVE_SELINUX
	सचे (getenv("SELINUX_INIT") == रिक्त) {
		सचे (is_selinux_enabled() != 1) {
			सचे (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} अञ्ञथा {
				सचे (enforce > 0) {
					/*
					 * SELinux in enforcing mode but load_policy failed
					 * At this point, we probably can't open /dev/console,
					 * so log() won't work
					 */
					fprintf(stderr,"Unable to load SELinux Policy.\n");
					fprintf(stderr,"Machine is  in enforcing mode.\n");
					fprintf(stderr,"Halting now.\n");
					exit(1);
				}
			}
		}
	}
#endif

	quiet = rc_yesno(getenv("EINFO_QUIET"));

	सचे (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	सचे (argc > 1) {
		सचे (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		अञ्ञथा
			cmdline_runlevel = argv[1];
	}

	सचे (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		पटिनिवत्त 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, रिक्त);

	/* install signal  handler */
	memset(&sa, 0, परिमाण(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, रिक्त);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, रिक्त);
	sigaction(SIGTERM, &sa, रिक्त);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, रिक्त);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	सचे (!reexec)
		init(cmdline_runlevel);

	सचे ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	अत्थाय (;;) {
		गणना { FD_FIFO, FD_SIG, FD_गणनम् };
		संरचना pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_गणनम्, -1);

		सचे (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		सचे (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	पटिनिवत्त 0;
}
