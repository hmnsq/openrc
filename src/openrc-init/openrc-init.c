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

#ضمن <errno.h>
#ضمن <pwd.h>
#ضمن <signal.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <time.h>
#ضمن <fcntl.h>
#ضمن <unistd.h>
#ضمن <sys/stat.h>
#ضمن <sys/reboot.h>
#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <poll.h>
#ضمن <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#ضمن "rc.h"
#ضمن "rc_exec.h"
#ضمن "wtmp.h"
#ضمن "helpers.h"
#ضمن "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#إذا defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

ساكن ثابت حرف *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ساكن ثابت حرف *rc_default_runlevel = "default";
ساكن صحيح sigpipe[2] = { -1, -1 };
ساكن bool quiet = false;
ساكن ثابت حرف *progname;

ساكن فراغ do_openrc(ثابت حرف *runlevel)
{
	ثابت حرف *argv[] = { "openrc", runlevel, عدم };
	بنية exec_result res;
	بنية exec_args args = exec_init(argv);
	args.setsid = 1;

	إذا (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	إذا (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	طالما (waitpid(res.pid, عدم, 0) != res.pid)
		إذا (errno == ECHILD)
			اكسر;
}

ساكن فراغ init(ثابت حرف *default_runlevel)
{
	ثابت حرف *runlevel = عدم;
	do_openrc("sysinit");
	do_openrc("boot");
	إذا (default_runlevel)
		runlevel = default_runlevel;
	وإلا
		runlevel = rc_conf_بەها("rc_default_runlevel");
	إذا (!runlevel)
		runlevel = rc_default_runlevel;
	إذا (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ساكن فراغ handle_reexec(فراغ)
{
	execlp(progname, progname, "reexec", عدم);
	أرجع;
}

ساكن فراغ handle_shutdown(ثابت حرف *runlevel, صحيح cmd)
{
	ثابت long ms_to_ns = 1000000L;
	بنية timespec start, now, tmp, remain;

	do_openrc(runlevel);
	إذا (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	طالما (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	لكل (;;) {
		pid_t pid = waitpid(-1, عدم, WNOHANG);
		إذا (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			اكسر;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		إذا (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			اكسر;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, عدم);
	}

	إذا (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	إذا (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ساكن فراغ run_program(ثابت حرف *prog)
{
	ثابت حرف *argv[] = { prog, عدم };
	بنية exec_args args = exec_init(argv);
	بنية exec_result res = do_exec(&args);
	إذا (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ساكن فراغ open_shell(فراغ)
{
	ثابت حرف *shell;
	بنية passwd *pw;

#ifdef __linux__
	ثابت حرف *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	إذا (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (حرف *) عدم);
		perror("init");
		أرجع;
	}
#endif

	shell = rc_conf_بەها("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	إذا (!shell) {
		shell = getenv("SHELL");
		إذا (!shell) {
			pw = getpwuid(getuid());
			إذا (pw)
				shell = pw->pw_shell;
			إذا (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ساكن فراغ handle_single(فراغ)
{
	do_openrc("single");
	open_shell();
	init(عدم);
}

ساكن فراغ dispatch_signal(صحيح sigfd)
{
	صحيح sig;
	ssize_t ژماردن = اقرأ(sigfd, &sig, حجم(sig));

	إذا (ژماردن < 0) {
		perror("read(sigpipe)");
		أرجع;
	}

	إذا (ژماردن != حجم(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		أرجع;
	}

	بدل (sig) {
		حالة SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			اكسر;
		حالة SIGTERM:
#ifdef SIGPWR
		حالة SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			اكسر;
		افتراضي:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			اكسر;
	}
}

ساكن فراغ dispatch_fifo(صحيح fifo)
{
	حرف buf[2048];
	ssize_t ژماردن = اقرأ(fifo, buf, حجم(buf) - 1);

	إذا (ژماردن < 0) {
		perror("read(fifo)");
		أرجع;
	}

	buf[ژماردن] = 0;

	إذا (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	إذا (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	وإلا إذا (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	وإلا إذا (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	وإلا إذا (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	وإلا إذا (strcmp(buf, "reexec") == 0)
		handle_reexec();
	وإلا إذا (strcmp(buf, "single") == 0)
		handle_single();
}

ساكن فراغ signal_handler(صحيح sig)
{
	صحيح saved_errno = errno;
	اكتب(sigpipe[1], &sig, حجم(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ساكن فراغ reap_zombies(صحيح sig RC_UNUSED)
{
	حرف errmsg[] = "waitpid() failed\n";
	صحيح saved_errno = errno;

	لكل (;;) {
		pid_t pid = waitpid(-1, عدم, WNOHANG);
		إذا (pid == 0)
			اكسر;
		وإلا إذا (pid == -1) {
			إذا (errno == ECHILD)
				اكسر;
			اكتب(STDERR_FILENO, errmsg, حجم(errmsg) - 1);
			استمر;
		}
	}
	errno = saved_errno;
}

صحيح main(صحيح argc, حرف **argv)
{
	حرف *cmdline_runlevel = عدم;
	صحيح fifo;
	bool reexec = false;
	sigset_t signals;
	بنية sigaction sa;
#ifdef HAVE_SELINUX
	صحيح enforce = 0;
#endif

	progname = argv[0];

	إذا (getpid() != 1)
		أرجع 1;

#ifdef HAVE_SELINUX
	إذا (getenv("SELINUX_INIT") == عدم) {
		إذا (is_selinux_enabled() != 1) {
			إذا (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} وإلا {
				إذا (enforce > 0) {
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

	إذا (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	إذا (argc > 1) {
		إذا (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		وإلا
			cmdline_runlevel = argv[1];
	}

	إذا (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		أرجع 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, عدم);

	/* install signal  handler */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, عدم);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, عدم);
	sigaction(SIGTERM, &sa, عدم);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, عدم);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	إذا (!reexec)
		init(cmdline_runlevel);

	إذا ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	لكل (;;) {
		تعداد { FD_FIFO, FD_SIG, FD_ژماردن };
		بنية pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_ژماردن, -1);

		إذا (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		إذا (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	أرجع 0;
}
