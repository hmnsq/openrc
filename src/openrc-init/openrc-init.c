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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/reboot.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <poll.h>
#ଅନ୍ତର୍ଭାଵ <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "wtmp.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ଯଦି defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *rc_default_runlevel = "default";
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sigpipe[2] = { -1, -1 };
ସ୍ଥିର bool quiet = false;
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *progname;

ସ୍ଥିର ଶୂନ୍ୟ do_openrc(ଅଚଳ ଅକ୍ଷର *runlevel)
{
	ଅଚଳ ଅକ୍ଷର *argv[] = { "openrc", runlevel, ଖାଲି };
	ସଂରଚନା exec_result res;
	ସଂରଚନା exec_args args = exec_init(argv);
	args.setsid = 1;

	ଯଦି (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	ଯଦି (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	ଯେତେବେଳେ (waitpid(res.pid, ଖାଲି, 0) != res.pid)
		ଯଦି (errno == ECHILD)
			ଭାଙ୍ଗ;
}

ସ୍ଥିର ଶୂନ୍ୟ init(ଅଚଳ ଅକ୍ଷର *default_runlevel)
{
	ଅଚଳ ଅକ୍ଷର *runlevel = ଖାଲି;
	do_openrc("sysinit");
	do_openrc("boot");
	ଯଦି (default_runlevel)
		runlevel = default_runlevel;
	ଅନ୍ୟଥା
		runlevel = rc_conf_ମୂଲ୍ୟ("rc_default_runlevel");
	ଯଦି (!runlevel)
		runlevel = rc_default_runlevel;
	ଯଦି (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ସ୍ଥିର ଶୂନ୍ୟ handle_reexec(ଶୂନ୍ୟ)
{
	execlp(progname, progname, "reexec", ଖାଲି);
	ଫେରନ୍ତୁ;
}

ସ୍ଥିର ଶୂନ୍ୟ handle_shutdown(ଅଚଳ ଅକ୍ଷର *runlevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd)
{
	ଅଚଳ long ms_to_ns = 1000000L;
	ସଂରଚନା timespec start, now, tmp, remain;

	do_openrc(runlevel);
	ଯଦି (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	ଯେତେବେଳେ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	ପାଇଁ (;;) {
		pid_t pid = waitpid(-1, ଖାଲି, WNOHANG);
		ଯଦି (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			ଭାଙ୍ଗ;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		ଯଦି (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			ଭାଙ୍ଗ;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ଖାଲି);
	}

	ଯଦି (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	ଯଦି (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ସ୍ଥିର ଶୂନ୍ୟ run_program(ଅଚଳ ଅକ୍ଷର *prog)
{
	ଅଚଳ ଅକ୍ଷର *argv[] = { prog, ଖାଲି };
	ସଂରଚନା exec_args args = exec_init(argv);
	ସଂରଚନା exec_result res = do_exec(&args);
	ଯଦି (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ସ୍ଥିର ଶୂନ୍ୟ open_shell(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *shell;
	ସଂରଚନା passwd *pw;

#ifdef __linux__
	ଅଚଳ ଅକ୍ଷର *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ଯଦି (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ଅକ୍ଷର *) ଖାଲି);
		perror("init");
		ଫେରନ୍ତୁ;
	}
#endif

	shell = rc_conf_ମୂଲ୍ୟ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ଯଦି (!shell) {
		shell = getenv("SHELL");
		ଯଦି (!shell) {
			pw = getpwuid(getuid());
			ଯଦି (pw)
				shell = pw->pw_shell;
			ଯଦି (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ସ୍ଥିର ଶୂନ୍ୟ handle_single(ଶୂନ୍ୟ)
{
	do_openrc("single");
	open_shell();
	init(ଖାଲି);
}

ସ୍ଥିର ଶୂନ୍ୟ dispatch_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sigfd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;
	ssize_t ଗଣନା_2 = ପଠ(sigfd, &sig, ଆକାର(sig));

	ଯଦି (ଗଣନା_2 < 0) {
		perror("read(sigpipe)");
		ଫେରନ୍ତୁ;
	}

	ଯଦି (ଗଣନା_2 != ଆକାର(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ଫେରନ୍ତୁ;
	}

	ବାଛ (sig) {
		ସ୍ଥିତି SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି SIGTERM:
#ifdef SIGPWR
		ସ୍ଥିତି SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			ଭାଙ୍ଗ;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ dispatch_fifo(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fifo)
{
	ଅକ୍ଷର buf[2048];
	ssize_t ଗଣନା_2 = ପଠ(fifo, buf, ଆକାର(buf) - 1);

	ଯଦି (ଗଣନା_2 < 0) {
		perror("read(fifo)");
		ଫେରନ୍ତୁ;
	}

	buf[ଗଣନା_2] = 0;

	ଯଦି (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	ଯଦି (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	ଅନ୍ୟଥା ଯଦି (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	ଅନ୍ୟଥା ଯଦି (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	ଅନ୍ୟଥା ଯଦି (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	ଅନ୍ୟଥା ଯଦି (strcmp(buf, "reexec") == 0)
		handle_reexec();
	ଅନ୍ୟଥା ଯଦି (strcmp(buf, "single") == 0)
		handle_single();
}

ସ୍ଥିର ଶୂନ୍ୟ signal_handler(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା saved_errno = errno;
	ଲିଖ(sigpipe[1], &sig, ଆକାର(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ସ୍ଥିର ଶୂନ୍ୟ reap_zombies(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig RC_UNUSED)
{
	ଅକ୍ଷର errmsg[] = "waitpid() failed\n";
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା saved_errno = errno;

	ପାଇଁ (;;) {
		pid_t pid = waitpid(-1, ଖାଲି, WNOHANG);
		ଯଦି (pid == 0)
			ଭାଙ୍ଗ;
		ଅନ୍ୟଥା ଯଦି (pid == -1) {
			ଯଦି (errno == ECHILD)
				ଭାଙ୍ଗ;
			ଲିଖ(STDERR_FILENO, errmsg, ଆକାର(errmsg) - 1);
			ଜାରି;
		}
	}
	errno = saved_errno;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *cmdline_runlevel = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fifo;
	bool reexec = false;
	sigset_t signals;
	ସଂରଚନା sigaction sa;
#ifdef HAVE_SELINUX
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା enforce = 0;
#endif

	progname = argv[0];

	ଯଦି (getpid() != 1)
		ଫେରନ୍ତୁ 1;

#ifdef HAVE_SELINUX
	ଯଦି (getenv("SELINUX_INIT") == ଖାଲି) {
		ଯଦି (is_selinux_enabled() != 1) {
			ଯଦି (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} ଅନ୍ୟଥା {
				ଯଦି (enforce > 0) {
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

	ଯଦି (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	ଯଦି (argc > 1) {
		ଯଦି (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		ଅନ୍ୟଥା
			cmdline_runlevel = argv[1];
	}

	ଯଦି (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ଫେରନ୍ତୁ 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, ଖାଲି);

	/* install signal  handler */
	memset(&sa, 0, ଆକାର(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ଖାଲି);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ଖାଲି);
	sigaction(SIGTERM, &sa, ଖାଲି);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, ଖାଲି);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	ଯଦି (!reexec)
		init(cmdline_runlevel);

	ଯଦି ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	ପାଇଁ (;;) {
		ଗଣନା { FD_FIFO, FD_SIG, FD_ଗଣନା };
		ସଂରଚନା pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_ଗଣନା, -1);

		ଯଦି (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		ଯଦି (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ଫେରନ୍ତୁ 0;
}
