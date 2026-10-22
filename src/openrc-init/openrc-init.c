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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/reboot.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <poll.h>
#அந்தர்பாவ <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "wtmp.h"
#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#எனில் defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

நிலையான மாறிலி எழுத்து *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
நிலையான மாறிலி எழுத்து *rc_default_runlevel = "default";
நிலையான முழு sigpipe[2] = { -1, -1 };
நிலையான bool quiet = false;
நிலையான மாறிலி எழுத்து *progname;

நிலையான வெறுமை do_openrc(மாறிலி எழுத்து *runlevel)
{
	மாறிலி எழுத்து *argv[] = { "openrc", runlevel, வெற்று };
	கட்டமைப்பு exec_result res;
	கட்டமைப்பு exec_args args = exec_init(argv);
	args.setsid = 1;

	எனில் (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	எனில் (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	போது (waitpid(res.pid, வெற்று, 0) != res.pid)
		எனில் (errno == ECHILD)
			நிறுத்து;
}

நிலையான வெறுமை init(மாறிலி எழுத்து *default_runlevel)
{
	மாறிலி எழுத்து *runlevel = வெற்று;
	do_openrc("sysinit");
	do_openrc("boot");
	எனில் (default_runlevel)
		runlevel = default_runlevel;
	இல்லையெனில்
		runlevel = rc_conf_மதிப்பு("rc_default_runlevel");
	எனில் (!runlevel)
		runlevel = rc_default_runlevel;
	எனில் (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

நிலையான வெறுமை handle_reexec(வெறுமை)
{
	execlp(progname, progname, "reexec", வெற்று);
	திரும்பு;
}

நிலையான வெறுமை handle_shutdown(மாறிலி எழுத்து *runlevel, முழு cmd)
{
	மாறிலி long ms_to_ns = 1000000L;
	கட்டமைப்பு timespec start, now, tmp, remain;

	do_openrc(runlevel);
	எனில் (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	போது (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	க்கு (;;) {
		pid_t pid = waitpid(-1, வெற்று, WNOHANG);
		எனில் (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			நிறுத்து;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		எனில் (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			நிறுத்து;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, வெற்று);
	}

	எனில் (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	எனில் (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

நிலையான வெறுமை run_program(மாறிலி எழுத்து *prog)
{
	மாறிலி எழுத்து *argv[] = { prog, வெற்று };
	கட்டமைப்பு exec_args args = exec_init(argv);
	கட்டமைப்பு exec_result res = do_exec(&args);
	எனில் (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

நிலையான வெறுமை open_shell(வெறுமை)
{
	மாறிலி எழுத்து *shell;
	கட்டமைப்பு passwd *pw;

#ifdef __linux__
	மாறிலி எழுத்து *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	எனில் (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (எழுத்து *) வெற்று);
		perror("init");
		திரும்பு;
	}
#endif

	shell = rc_conf_மதிப்பு("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	எனில் (!shell) {
		shell = getenv("SHELL");
		எனில் (!shell) {
			pw = getpwuid(getuid());
			எனில் (pw)
				shell = pw->pw_shell;
			எனில் (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

நிலையான வெறுமை handle_single(வெறுமை)
{
	do_openrc("single");
	open_shell();
	init(வெற்று);
}

நிலையான வெறுமை dispatch_signal(முழு sigfd)
{
	முழு sig;
	ssize_t மொத்தம் = பட(sigfd, &sig, அளவு(sig));

	எனில் (மொத்தம் < 0) {
		perror("read(sigpipe)");
		திரும்பு;
	}

	எனில் (மொத்தம் != அளவு(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		திரும்பு;
	}

	தேர்வு (sig) {
		நிலை SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			நிறுத்து;
		நிலை SIGTERM:
#ifdef SIGPWR
		நிலை SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			நிறுத்து;
		இயல்பு:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			நிறுத்து;
	}
}

நிலையான வெறுமை dispatch_fifo(முழு fifo)
{
	எழுத்து buf[2048];
	ssize_t மொத்தம் = பட(fifo, buf, அளவு(buf) - 1);

	எனில் (மொத்தம் < 0) {
		perror("read(fifo)");
		திரும்பு;
	}

	buf[மொத்தம்] = 0;

	எனில் (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	எனில் (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	இல்லையெனில் எனில் (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	இல்லையெனில் எனில் (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	இல்லையெனில் எனில் (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	இல்லையெனில் எனில் (strcmp(buf, "reexec") == 0)
		handle_reexec();
	இல்லையெனில் எனில் (strcmp(buf, "single") == 0)
		handle_single();
}

நிலையான வெறுமை signal_handler(முழு sig)
{
	முழு saved_errno = errno;
	லிக(sigpipe[1], &sig, அளவு(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

நிலையான வெறுமை reap_zombies(முழு sig RC_UNUSED)
{
	எழுத்து errmsg[] = "waitpid() failed\n";
	முழு saved_errno = errno;

	க்கு (;;) {
		pid_t pid = waitpid(-1, வெற்று, WNOHANG);
		எனில் (pid == 0)
			நிறுத்து;
		இல்லையெனில் எனில் (pid == -1) {
			எனில் (errno == ECHILD)
				நிறுத்து;
			லிக(STDERR_FILENO, errmsg, அளவு(errmsg) - 1);
			தொடர்;
		}
	}
	errno = saved_errno;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	எழுத்து *cmdline_runlevel = வெற்று;
	முழு fifo;
	bool reexec = false;
	sigset_t signals;
	கட்டமைப்பு sigaction sa;
#ifdef HAVE_SELINUX
	முழு enforce = 0;
#endif

	progname = argv[0];

	எனில் (getpid() != 1)
		திரும்பு 1;

#ifdef HAVE_SELINUX
	எனில் (getenv("SELINUX_INIT") == வெற்று) {
		எனில் (is_selinux_enabled() != 1) {
			எனில் (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} இல்லையெனில் {
				எனில் (enforce > 0) {
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

	எனில் (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	எனில் (argc > 1) {
		எனில் (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		இல்லையெனில்
			cmdline_runlevel = argv[1];
	}

	எனில் (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		திரும்பு 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, வெற்று);

	/* install signal  handler */
	memset(&sa, 0, அளவு(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, வெற்று);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, வெற்று);
	sigaction(SIGTERM, &sa, வெற்று);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, வெற்று);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	எனில் (!reexec)
		init(cmdline_runlevel);

	எனில் ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	க்கு (;;) {
		எண்ணிக்கை { FD_FIFO, FD_SIG, FD_மொத்தம் };
		கட்டமைப்பு pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_மொத்தம், -1);

		எனில் (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		எனில் (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	திரும்பு 0;
}
