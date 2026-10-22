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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/reboot.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <poll.h>
#অন্তর্ভাৱ <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "rc_exec.h"
#অন্তর্ভাৱ "wtmp.h"
#অন্তর্ভাৱ "helpers.h"
#অন্তর্ভাৱ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#যদি defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

স্থির ধ্রুৱ ৱর্ণ *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
স্থির ধ্রুৱ ৱর্ণ *rc_default_runlevel = "default";
স্থির পূর্ণাঙ্ক sigpipe[2] = { -1, -1 };
স্থির bool quiet = false;
স্থির ধ্রুৱ ৱর্ণ *progname;

স্থির শূন্য do_openrc(ধ্রুৱ ৱর্ণ *runlevel)
{
	ধ্রুৱ ৱর্ণ *argv[] = { "openrc", runlevel, রিক্ত };
	সংরচনা exec_result res;
	সংরচনা exec_args args = exec_init(argv);
	args.setsid = 1;

	যদি (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	যদি (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	যেতিয়ালৈ (waitpid(res.pid, রিক্ত, 0) != res.pid)
		যদি (errno == ECHILD)
			ভঙ্গ;
}

স্থির শূন্য init(ধ্রুৱ ৱর্ণ *default_runlevel)
{
	ধ্রুৱ ৱর্ণ *runlevel = রিক্ত;
	do_openrc("sysinit");
	do_openrc("boot");
	যদি (default_runlevel)
		runlevel = default_runlevel;
	অন্যথা
		runlevel = rc_conf_মান("rc_default_runlevel");
	যদি (!runlevel)
		runlevel = rc_default_runlevel;
	যদি (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

স্থির শূন্য handle_reexec(শূন্য)
{
	execlp(progname, progname, "reexec", রিক্ত);
	ঘূৰাই;
}

স্থির শূন্য handle_shutdown(ধ্রুৱ ৱর্ণ *runlevel, পূর্ণাঙ্ক cmd)
{
	ধ্রুৱ long ms_to_ns = 1000000L;
	সংরচনা timespec start, now, tmp, remain;

	do_openrc(runlevel);
	যদি (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	যেতিয়ালৈ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	বাবে (;;) {
		pid_t pid = waitpid(-1, রিক্ত, WNOHANG);
		যদি (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			ভঙ্গ;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		যদি (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			ভঙ্গ;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, রিক্ত);
	}

	যদি (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	যদি (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

স্থির শূন্য run_program(ধ্রুৱ ৱর্ণ *prog)
{
	ধ্রুৱ ৱর্ণ *argv[] = { prog, রিক্ত };
	সংরচনা exec_args args = exec_init(argv);
	সংরচনা exec_result res = do_exec(&args);
	যদি (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

স্থির শূন্য open_shell(শূন্য)
{
	ধ্রুৱ ৱর্ণ *shell;
	সংরচনা passwd *pw;

#ifdef __linux__
	ধ্রুৱ ৱর্ণ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	যদি (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ৱর্ণ *) রিক্ত);
		perror("init");
		ঘূৰাই;
	}
#endif

	shell = rc_conf_মান("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	যদি (!shell) {
		shell = getenv("SHELL");
		যদি (!shell) {
			pw = getpwuid(getuid());
			যদি (pw)
				shell = pw->pw_shell;
			যদি (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

স্থির শূন্য handle_single(শূন্য)
{
	do_openrc("single");
	open_shell();
	init(রিক্ত);
}

স্থির শূন্য dispatch_signal(পূর্ণাঙ্ক sigfd)
{
	পূর্ণাঙ্ক sig;
	ssize_t গুনতি = পঠ(sigfd, &sig, পরিমাণ(sig));

	যদি (গুনতি < 0) {
		perror("read(sigpipe)");
		ঘূৰাই;
	}

	যদি (গুনতি != পরিমাণ(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ঘূৰাই;
	}

	ৱিকল্প (sig) {
		পক্ষ SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			ভঙ্গ;
		পক্ষ SIGTERM:
#ifdef SIGPWR
		পক্ষ SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			ভঙ্গ;
		স্ৱভাৱ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			ভঙ্গ;
	}
}

স্থির শূন্য dispatch_fifo(পূর্ণাঙ্ক fifo)
{
	ৱর্ণ buf[2048];
	ssize_t গুনতি = পঠ(fifo, buf, পরিমাণ(buf) - 1);

	যদি (গুনতি < 0) {
		perror("read(fifo)");
		ঘূৰাই;
	}

	buf[গুনতি] = 0;

	যদি (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	যদি (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	অন্যথা যদি (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	অন্যথা যদি (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	অন্যথা যদি (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	অন্যথা যদি (strcmp(buf, "reexec") == 0)
		handle_reexec();
	অন্যথা যদি (strcmp(buf, "single") == 0)
		handle_single();
}

স্থির শূন্য signal_handler(পূর্ণাঙ্ক sig)
{
	পূর্ণাঙ্ক saved_errno = errno;
	লিখ(sigpipe[1], &sig, পরিমাণ(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

স্থির শূন্য reap_zombies(পূর্ণাঙ্ক sig RC_UNUSED)
{
	ৱর্ণ errmsg[] = "waitpid() failed\n";
	পূর্ণাঙ্ক saved_errno = errno;

	বাবে (;;) {
		pid_t pid = waitpid(-1, রিক্ত, WNOHANG);
		যদি (pid == 0)
			ভঙ্গ;
		অন্যথা যদি (pid == -1) {
			যদি (errno == ECHILD)
				ভঙ্গ;
			লিখ(STDERR_FILENO, errmsg, পরিমাণ(errmsg) - 1);
			অনুৱর্ত;
		}
	}
	errno = saved_errno;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	ৱর্ণ *cmdline_runlevel = রিক্ত;
	পূর্ণাঙ্ক fifo;
	bool reexec = false;
	sigset_t signals;
	সংরচনা sigaction sa;
#ifdef HAVE_SELINUX
	পূর্ণাঙ্ক enforce = 0;
#endif

	progname = argv[0];

	যদি (getpid() != 1)
		ঘূৰাই 1;

#ifdef HAVE_SELINUX
	যদি (getenv("SELINUX_INIT") == রিক্ত) {
		যদি (is_selinux_enabled() != 1) {
			যদি (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} অন্যথা {
				যদি (enforce > 0) {
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

	যদি (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	যদি (argc > 1) {
		যদি (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		অন্যথা
			cmdline_runlevel = argv[1];
	}

	যদি (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ঘূৰাই 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, রিক্ত);

	/* install signal  handler */
	memset(&sa, 0, পরিমাণ(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, রিক্ত);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, রিক্ত);
	sigaction(SIGTERM, &sa, রিক্ত);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, রিক্ত);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	যদি (!reexec)
		init(cmdline_runlevel);

	যদি ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	বাবে (;;) {
		গণনা { FD_FIFO, FD_SIG, FD_গুনতি };
		সংরচনা pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_গুনতি, -1);

		যদি (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		যদি (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ঘূৰাই 0;
}
