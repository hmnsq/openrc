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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/reboot.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "wtmp.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ಯದಿ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *rc_default_runlevel = "default";
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sigpipe[2] = { -1, -1 };
ಸ್ಥಿರ bool quiet = false;
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *progname;

ಸ್ಥಿರ ಶೂನ್ಯ do_openrc(ಧ್ರುವ ವರ್ಣ *runlevel)
{
	ಧ್ರುವ ವರ್ಣ *argv[] = { "openrc", runlevel, ರಿಕ್ತ };
	ಸಂರಚನಾ exec_result res;
	ಸಂರಚನಾ exec_args args = exec_init(argv);
	args.setsid = 1;

	ಯದಿ (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	ಯದಿ (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	ಯಾವತ್ (waitpid(res.pid, ರಿಕ್ತ, 0) != res.pid)
		ಯದಿ (errno == ECHILD)
			ಭಙ್ಗ;
}

ಸ್ಥಿರ ಶೂನ್ಯ init(ಧ್ರುವ ವರ್ಣ *default_runlevel)
{
	ಧ್ರುವ ವರ್ಣ *runlevel = ರಿಕ್ತ;
	do_openrc("sysinit");
	do_openrc("boot");
	ಯದಿ (default_runlevel)
		runlevel = default_runlevel;
	ಅನ್ಯಥಾ
		runlevel = rc_conf_ಮೌಲ್ಯ("rc_default_runlevel");
	ಯದಿ (!runlevel)
		runlevel = rc_default_runlevel;
	ಯದಿ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_reexec(ಶೂನ್ಯ)
{
	execlp(progname, progname, "reexec", ರಿಕ್ತ);
	ಪ್ರತ್ಯಾಗಮ;
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_shutdown(ಧ್ರುವ ವರ್ಣ *runlevel, ಪೂರ್ಣಾಙ್ಕ cmd)
{
	ಧ್ರುವ long ms_to_ns = 1000000L;
	ಸಂರಚನಾ timespec start, now, tmp, remain;

	do_openrc(runlevel);
	ಯದಿ (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	ಯಾವತ್ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	ಪ್ರತಿ (;;) {
		pid_t pid = waitpid(-1, ರಿಕ್ತ, WNOHANG);
		ಯದಿ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			ಭಙ್ಗ;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		ಯದಿ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			ಭಙ್ಗ;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ರಿಕ್ತ);
	}

	ಯದಿ (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	ಯದಿ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ಸ್ಥಿರ ಶೂನ್ಯ run_program(ಧ್ರುವ ವರ್ಣ *prog)
{
	ಧ್ರುವ ವರ್ಣ *argv[] = { prog, ರಿಕ್ತ };
	ಸಂರಚನಾ exec_args args = exec_init(argv);
	ಸಂರಚನಾ exec_result res = do_exec(&args);
	ಯದಿ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ಸ್ಥಿರ ಶೂನ್ಯ open_shell(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *shell;
	ಸಂರಚನಾ passwd *pw;

#ifdef __linux__
	ಧ್ರುವ ವರ್ಣ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ಯದಿ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ವರ್ಣ *) ರಿಕ್ತ);
		perror("init");
		ಪ್ರತ್ಯಾಗಮ;
	}
#endif

	shell = rc_conf_ಮೌಲ್ಯ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ಯದಿ (!shell) {
		shell = getenv("SHELL");
		ಯದಿ (!shell) {
			pw = getpwuid(getuid());
			ಯದಿ (pw)
				shell = pw->pw_shell;
			ಯದಿ (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_single(ಶೂನ್ಯ)
{
	do_openrc("single");
	open_shell();
	init(ರಿಕ್ತ);
}

ಸ್ಥಿರ ಶೂನ್ಯ dispatch_signal(ಪೂರ್ಣಾಙ್ಕ sigfd)
{
	ಪೂರ್ಣಾಙ್ಕ sig;
	ssize_t ಎಣಿಕೆ = ಪಠ(sigfd, &sig, ಪರಿಮಾಣ(sig));

	ಯದಿ (ಎಣಿಕೆ < 0) {
		perror("read(sigpipe)");
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (ಎಣಿಕೆ != ಪರಿಮಾಣ(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ಪ್ರತ್ಯಾಗಮ;
	}

	ವಿಕಲ್ಪ (sig) {
		ಪಕ್ಷ SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			ಭಙ್ಗ;
		ಪಕ್ಷ SIGTERM:
#ifdef SIGPWR
		ಪಕ್ಷ SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			ಭಙ್ಗ;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ dispatch_fifo(ಪೂರ್ಣಾಙ್ಕ fifo)
{
	ವರ್ಣ buf[2048];
	ssize_t ಎಣಿಕೆ = ಪಠ(fifo, buf, ಪರಿಮಾಣ(buf) - 1);

	ಯದಿ (ಎಣಿಕೆ < 0) {
		perror("read(fifo)");
		ಪ್ರತ್ಯಾಗಮ;
	}

	buf[ಎಣಿಕೆ] = 0;

	ಯದಿ (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	ಯದಿ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	ಅನ್ಯಥಾ ಯದಿ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	ಅನ್ಯಥಾ ಯದಿ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	ಅನ್ಯಥಾ ಯದಿ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	ಅನ್ಯಥಾ ಯದಿ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	ಅನ್ಯಥಾ ಯದಿ (strcmp(buf, "single") == 0)
		handle_single();
}

ಸ್ಥಿರ ಶೂನ್ಯ signal_handler(ಪೂರ್ಣಾಙ್ಕ sig)
{
	ಪೂರ್ಣಾಙ್ಕ saved_errno = errno;
	ಲಿಖ(sigpipe[1], &sig, ಪರಿಮಾಣ(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ಸ್ಥಿರ ಶೂನ್ಯ reap_zombies(ಪೂರ್ಣಾಙ್ಕ sig RC_UNUSED)
{
	ವರ್ಣ errmsg[] = "waitpid() failed\n";
	ಪೂರ್ಣಾಙ್ಕ saved_errno = errno;

	ಪ್ರತಿ (;;) {
		pid_t pid = waitpid(-1, ರಿಕ್ತ, WNOHANG);
		ಯದಿ (pid == 0)
			ಭಙ್ಗ;
		ಅನ್ಯಥಾ ಯದಿ (pid == -1) {
			ಯದಿ (errno == ECHILD)
				ಭಙ್ಗ;
			ಲಿಖ(STDERR_FILENO, errmsg, ಪರಿಮಾಣ(errmsg) - 1);
			ಅನುವರ್ತ;
		}
	}
	errno = saved_errno;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ವರ್ಣ *cmdline_runlevel = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ fifo;
	bool reexec = false;
	sigset_t signals;
	ಸಂರಚನಾ sigaction sa;
#ifdef HAVE_SELINUX
	ಪೂರ್ಣಾಙ್ಕ enforce = 0;
#endif

	progname = argv[0];

	ಯದಿ (getpid() != 1)
		ಪ್ರತ್ಯಾಗಮ 1;

#ifdef HAVE_SELINUX
	ಯದಿ (getenv("SELINUX_INIT") == ರಿಕ್ತ) {
		ಯದಿ (is_selinux_enabled() != 1) {
			ಯದಿ (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} ಅನ್ಯಥಾ {
				ಯದಿ (enforce > 0) {
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

	ಯದಿ (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	ಯದಿ (argc > 1) {
		ಯದಿ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		ಅನ್ಯಥಾ
			cmdline_runlevel = argv[1];
	}

	ಯದಿ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, ರಿಕ್ತ);

	/* install signal  handler */
	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ರಿಕ್ತ);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ರಿಕ್ತ);
	sigaction(SIGTERM, &sa, ರಿಕ್ತ);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, ರಿಕ್ತ);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	ಯದಿ (!reexec)
		init(cmdline_runlevel);

	ಯದಿ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	ಪ್ರತಿ (;;) {
		ಗಣನಾ { FD_FIFO, FD_SIG, FD_ಎಣಿಕೆ };
		ಸಂರಚನಾ pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_ಎಣಿಕೆ, -1);

		ಯದಿ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		ಯದಿ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}
