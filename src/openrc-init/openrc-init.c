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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/reboot.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "wtmp.h"
#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#എങ്കിൽ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

സ്ഥിതം സ്ഥിരം അക്ഷരം *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
സ്ഥിതം സ്ഥിരം അക്ഷരം *rc_default_runlevel = "default";
സ്ഥിതം പൂർണ്ണസംഖ്യ sigpipe[2] = { -1, -1 };
സ്ഥിതം bool quiet = false;
സ്ഥിതം സ്ഥിരം അക്ഷരം *progname;

സ്ഥിതം ശൂന്യം do_openrc(സ്ഥിരം അക്ഷരം *runlevel)
{
	സ്ഥിരം അക്ഷരം *argv[] = { "openrc", runlevel, ശൂന്യ };
	ഘടന exec_result res;
	ഘടന exec_args args = exec_init(argv);
	args.setsid = 1;

	എങ്കിൽ (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	എങ്കിൽ (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	ആകുമ്പോൾ (waitpid(res.pid, ശൂന്യ, 0) != res.pid)
		എങ്കിൽ (errno == ECHILD)
			നിർത്തുക;
}

സ്ഥിതം ശൂന്യം init(സ്ഥിരം അക്ഷരം *default_runlevel)
{
	സ്ഥിരം അക്ഷരം *runlevel = ശൂന്യ;
	do_openrc("sysinit");
	do_openrc("boot");
	എങ്കിൽ (default_runlevel)
		runlevel = default_runlevel;
	അല്ലെങ്കിൽ
		runlevel = rc_conf_മൂല്യം("rc_default_runlevel");
	എങ്കിൽ (!runlevel)
		runlevel = rc_default_runlevel;
	എങ്കിൽ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

സ്ഥിതം ശൂന്യം handle_reexec(ശൂന്യം)
{
	execlp(progname, progname, "reexec", ശൂന്യ);
	തിരികെ;
}

സ്ഥിതം ശൂന്യം handle_shutdown(സ്ഥിരം അക്ഷരം *runlevel, പൂർണ്ണസംഖ്യ cmd)
{
	സ്ഥിരം long ms_to_ns = 1000000L;
	ഘടന timespec start, now, tmp, remain;

	do_openrc(runlevel);
	എങ്കിൽ (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	ആകുമ്പോൾ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	വേണ്ടി (;;) {
		pid_t pid = waitpid(-1, ശൂന്യ, WNOHANG);
		എങ്കിൽ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			നിർത്തുക;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		എങ്കിൽ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			നിർത്തുക;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ശൂന്യ);
	}

	എങ്കിൽ (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	എങ്കിൽ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

സ്ഥിതം ശൂന്യം run_program(സ്ഥിരം അക്ഷരം *prog)
{
	സ്ഥിരം അക്ഷരം *argv[] = { prog, ശൂന്യ };
	ഘടന exec_args args = exec_init(argv);
	ഘടന exec_result res = do_exec(&args);
	എങ്കിൽ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

സ്ഥിതം ശൂന്യം open_shell(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *shell;
	ഘടന passwd *pw;

#ifdef __linux__
	സ്ഥിരം അക്ഷരം *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	എങ്കിൽ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (അക്ഷരം *) ശൂന്യ);
		perror("init");
		തിരികെ;
	}
#endif

	shell = rc_conf_മൂല്യം("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	എങ്കിൽ (!shell) {
		shell = getenv("SHELL");
		എങ്കിൽ (!shell) {
			pw = getpwuid(getuid());
			എങ്കിൽ (pw)
				shell = pw->pw_shell;
			എങ്കിൽ (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

സ്ഥിതം ശൂന്യം handle_single(ശൂന്യം)
{
	do_openrc("single");
	open_shell();
	init(ശൂന്യ);
}

സ്ഥിതം ശൂന്യം dispatch_signal(പൂർണ്ണസംഖ്യ sigfd)
{
	പൂർണ്ണസംഖ്യ sig;
	ssize_t എണ്ണം = പഠ(sigfd, &sig, വലുപ്പം(sig));

	എങ്കിൽ (എണ്ണം < 0) {
		perror("read(sigpipe)");
		തിരികെ;
	}

	എങ്കിൽ (എണ്ണം != വലുപ്പം(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		തിരികെ;
	}

	തിരഞ്ഞെടുപ്പ് (sig) {
		അവസ്ഥ SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			നിർത്തുക;
		അവസ്ഥ SIGTERM:
#ifdef SIGPWR
		അവസ്ഥ SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			നിർത്തുക;
	}
}

സ്ഥിതം ശൂന്യം dispatch_fifo(പൂർണ്ണസംഖ്യ fifo)
{
	അക്ഷരം buf[2048];
	ssize_t എണ്ണം = പഠ(fifo, buf, വലുപ്പം(buf) - 1);

	എങ്കിൽ (എണ്ണം < 0) {
		perror("read(fifo)");
		തിരികെ;
	}

	buf[എണ്ണം] = 0;

	എങ്കിൽ (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	എങ്കിൽ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	അല്ലെങ്കിൽ എങ്കിൽ (strcmp(buf, "single") == 0)
		handle_single();
}

സ്ഥിതം ശൂന്യം signal_handler(പൂർണ്ണസംഖ്യ sig)
{
	പൂർണ്ണസംഖ്യ saved_errno = errno;
	ലിഖ(sigpipe[1], &sig, വലുപ്പം(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

സ്ഥിതം ശൂന്യം reap_zombies(പൂർണ്ണസംഖ്യ sig RC_UNUSED)
{
	അക്ഷരം errmsg[] = "waitpid() failed\n";
	പൂർണ്ണസംഖ്യ saved_errno = errno;

	വേണ്ടി (;;) {
		pid_t pid = waitpid(-1, ശൂന്യ, WNOHANG);
		എങ്കിൽ (pid == 0)
			നിർത്തുക;
		അല്ലെങ്കിൽ എങ്കിൽ (pid == -1) {
			എങ്കിൽ (errno == ECHILD)
				നിർത്തുക;
			ലിഖ(STDERR_FILENO, errmsg, വലുപ്പം(errmsg) - 1);
			തുടരുക;
		}
	}
	errno = saved_errno;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	അക്ഷരം *cmdline_runlevel = ശൂന്യ;
	പൂർണ്ണസംഖ്യ fifo;
	bool reexec = false;
	sigset_t signals;
	ഘടന sigaction sa;
#ifdef HAVE_SELINUX
	പൂർണ്ണസംഖ്യ enforce = 0;
#endif

	progname = argv[0];

	എങ്കിൽ (getpid() != 1)
		തിരികെ 1;

#ifdef HAVE_SELINUX
	എങ്കിൽ (getenv("SELINUX_INIT") == ശൂന്യ) {
		എങ്കിൽ (is_selinux_enabled() != 1) {
			എങ്കിൽ (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (enforce > 0) {
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

	എങ്കിൽ (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	എങ്കിൽ (argc > 1) {
		എങ്കിൽ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		അല്ലെങ്കിൽ
			cmdline_runlevel = argv[1];
	}

	എങ്കിൽ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		തിരികെ 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, ശൂന്യ);

	/* install signal  handler */
	memset(&sa, 0, വലുപ്പം(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ശൂന്യ);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ശൂന്യ);
	sigaction(SIGTERM, &sa, ശൂന്യ);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, ശൂന്യ);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	എങ്കിൽ (!reexec)
		init(cmdline_runlevel);

	എങ്കിൽ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	വേണ്ടി (;;) {
		എണ്ണൽ { FD_FIFO, FD_SIG, FD_എണ്ണം };
		ഘടന pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_എണ്ണം, -1);

		എങ്കിൽ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		എങ്കിൽ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	തിരികെ 0;
}
