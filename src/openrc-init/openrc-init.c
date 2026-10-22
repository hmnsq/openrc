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

#شامل <errno.h>
#شامل <pwd.h>
#شامل <signal.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <fcntl.h>
#شامل <unistd.h>
#شامل <sys/stat.h>
#شامل <sys/reboot.h>
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <poll.h>
#شامل <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "wtmp.h"
#شامل "helpers.h"
#شامل "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#اگر defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

ایستا ثابت نویسه *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ایستا ثابت نویسه *rc_default_runlevel = "default";
ایستا صحیح sigpipe[2] = { -1, -1 };
ایستا bool quiet = false;
ایستا ثابت نویسه *progname;

ایستا تهی do_openrc(ثابت نویسه *runlevel)
{
	ثابت نویسه *argv[] = { "openrc", runlevel, پوچ };
	ساختار exec_result res;
	ساختار exec_args args = exec_init(argv);
	args.setsid = 1;

	اگر (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	اگر (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	وقتی (waitpid(res.pid, پوچ, 0) != res.pid)
		اگر (errno == ECHILD)
			بشکن;
}

ایستا تهی init(ثابت نویسه *default_runlevel)
{
	ثابت نویسه *runlevel = پوچ;
	do_openrc("sysinit");
	do_openrc("boot");
	اگر (default_runlevel)
		runlevel = default_runlevel;
	وگرنه
		runlevel = rc_conf_مقدار("rc_default_runlevel");
	اگر (!runlevel)
		runlevel = rc_default_runlevel;
	اگر (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ایستا تهی handle_reexec(تهی)
{
	execlp(progname, progname, "reexec", پوچ);
	بازگشت;
}

ایستا تهی handle_shutdown(ثابت نویسه *runlevel, صحیح cmd)
{
	ثابت long ms_to_ns = 1000000L;
	ساختار timespec start, now, tmp, remain;

	do_openrc(runlevel);
	اگر (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	وقتی (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	برای (;;) {
		pid_t pid = waitpid(-1, پوچ, WNOHANG);
		اگر (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			بشکن;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		اگر (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			بشکن;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, پوچ);
	}

	اگر (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	اگر (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ایستا تهی run_program(ثابت نویسه *prog)
{
	ثابت نویسه *argv[] = { prog, پوچ };
	ساختار exec_args args = exec_init(argv);
	ساختار exec_result res = do_exec(&args);
	اگر (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ایستا تهی open_shell(تهی)
{
	ثابت نویسه *shell;
	ساختار passwd *pw;

#ifdef __linux__
	ثابت نویسه *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	اگر (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (نویسه *) پوچ);
		perror("init");
		بازگشت;
	}
#endif

	shell = rc_conf_مقدار("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	اگر (!shell) {
		shell = getenv("SHELL");
		اگر (!shell) {
			pw = getpwuid(getuid());
			اگر (pw)
				shell = pw->pw_shell;
			اگر (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ایستا تهی handle_single(تهی)
{
	do_openrc("single");
	open_shell();
	init(پوچ);
}

ایستا تهی dispatch_signal(صحیح sigfd)
{
	صحیح sig;
	ssize_t شمار = اقرأ(sigfd, &sig, اندازه(sig));

	اگر (شمار < 0) {
		perror("read(sigpipe)");
		بازگشت;
	}

	اگر (شمار != اندازه(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		بازگشت;
	}

	انتخاب (sig) {
		حالت SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			بشکن;
		حالت SIGTERM:
#ifdef SIGPWR
		حالت SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			بشکن;
		پیشفرض:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			بشکن;
	}
}

ایستا تهی dispatch_fifo(صحیح fifo)
{
	نویسه buf[2048];
	ssize_t شمار = اقرأ(fifo, buf, اندازه(buf) - 1);

	اگر (شمار < 0) {
		perror("read(fifo)");
		بازگشت;
	}

	buf[شمار] = 0;

	اگر (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	اگر (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	وگرنه اگر (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	وگرنه اگر (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	وگرنه اگر (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	وگرنه اگر (strcmp(buf, "reexec") == 0)
		handle_reexec();
	وگرنه اگر (strcmp(buf, "single") == 0)
		handle_single();
}

ایستا تهی signal_handler(صحیح sig)
{
	صحیح saved_errno = errno;
	اكتب(sigpipe[1], &sig, اندازه(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ایستا تهی reap_zombies(صحیح sig RC_UNUSED)
{
	نویسه errmsg[] = "waitpid() failed\n";
	صحیح saved_errno = errno;

	برای (;;) {
		pid_t pid = waitpid(-1, پوچ, WNOHANG);
		اگر (pid == 0)
			بشکن;
		وگرنه اگر (pid == -1) {
			اگر (errno == ECHILD)
				بشکن;
			اكتب(STDERR_FILENO, errmsg, اندازه(errmsg) - 1);
			ادامه;
		}
	}
	errno = saved_errno;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	نویسه *cmdline_runlevel = پوچ;
	صحیح fifo;
	bool reexec = false;
	sigset_t signals;
	ساختار sigaction sa;
#ifdef HAVE_SELINUX
	صحیح enforce = 0;
#endif

	progname = argv[0];

	اگر (getpid() != 1)
		بازگشت 1;

#ifdef HAVE_SELINUX
	اگر (getenv("SELINUX_INIT") == پوچ) {
		اگر (is_selinux_enabled() != 1) {
			اگر (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} وگرنه {
				اگر (enforce > 0) {
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

	اگر (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	اگر (argc > 1) {
		اگر (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		وگرنه
			cmdline_runlevel = argv[1];
	}

	اگر (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		بازگشت 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, پوچ);

	/* install signal  handler */
	memset(&sa, 0, اندازه(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, پوچ);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, پوچ);
	sigaction(SIGTERM, &sa, پوچ);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, پوچ);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	اگر (!reexec)
		init(cmdline_runlevel);

	اگر ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	برای (;;) {
		شمارش { FD_FIFO, FD_SIG, FD_شمار };
		ساختار pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_شمار, -1);

		اگر (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		اگر (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	بازگشت 0;
}
