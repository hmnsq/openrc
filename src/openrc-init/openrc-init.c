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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/reboot.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <poll.h>
#అన్తర్భావ <utmp.h>

#నిర్వచితమైతే HAVE_SELINUX
#  include <selinux/selinux.h>
#అయితే_ముగింపు

#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "wtmp.h"
#అన్తర్భావ "helpers.h"
#అన్తర్భావ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ఒకవేళ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#అయితే_ముగింపు

స్థిర స్థిరరాశి అక్షరం *మార్గం_default = "/sbin:/usr/sbin:/bin:/usr/bin";
స్థిర స్థిరరాశి అక్షరం *rc_default_runlevel = "default";
స్థిర పూర్ణాంకం sigpipe[2] = { -1, -1 };
స్థిర bool quiet = false;
స్థిర స్థిరరాశి అక్షరం *progname;

స్థిర శూన్య do_openrc(స్థిరరాశి అక్షరం *runlevel)
{
	స్థిరరాశి అక్షరం *argv[] = { "openrc", runlevel, ఖాళీ };
	నిర్మాణం exec_ఫలితం res;
	నిర్మాణం exec_వాదనలు వాదనలు = exec_init(argv);
	వాదనలు.setsid = 1;

	ఒకవేళ (!quiet)
		రాయి("Starting %s runlevel\n", runlevel);
	res = do_exec(&వాదనలు);
	ఒకవేళ (res.pid < 0) {
		perror("do_exec");
		నిష్క్రమించు(1);
	}
	అయితే (waitpid(res.pid, ఖాళీ, 0) != res.pid)
		ఒకవేళ (errno == ECHILD)
			ఆపు;
}

స్థిర శూన్య init(స్థిరరాశి అక్షరం *default_runlevel)
{
	స్థిరరాశి అక్షరం *runlevel = ఖాళీ;
	do_openrc("sysinit");
	do_openrc("boot");
	ఒకవేళ (default_runlevel)
		runlevel = default_runlevel;
	లేకపోతే
		runlevel = rc_conf_విలువ("rc_default_runlevel");
	ఒకవేళ (!runlevel)
		runlevel = rc_default_runlevel;
	ఒకవేళ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

స్థిర శూన్య handle_reexec(శూన్య)
{
	execlp(progname, progname, "reexec", ఖాళీ);
	తిరిగి;
}

స్థిర శూన్య handle_shutdown(స్థిరరాశి అక్షరం *runlevel, పూర్ణాంకం cmd)
{
	స్థిరరాశి దీర్ఘ ms_గమ్యం_ns = 1000000L;
	నిర్మాణం timespec ప్రారంభం, now, tmp, remain;

	do_openrc(runlevel);
	ఒకవేళ (!quiet)
		రాయి("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &ప్రారంభం);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_గమ్యం_ns;
	అయితే (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	కొరకు (;;) {
		pid_t pid = waitpid(-1, ఖాళీ, WNOHANG);
		ఒకవేళ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			ఆపు;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &ప్రారంభం, &tmp);
		ఒకవేళ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			ఆపు;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_గమ్యం_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ఖాళీ);
	}

	ఒకవేళ (!quiet)
		రాయి("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	ఒకవేళ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

స్థిర శూన్య నిరంతరం_కార్యక్రమం(స్థిరరాశి అక్షరం *prog)
{
	స్థిరరాశి అక్షరం *argv[] = { prog, ఖాళీ };
	నిర్మాణం exec_వాదనలు వాదనలు = exec_init(argv);
	నిర్మాణం exec_ఫలితం res = do_exec(&వాదనలు);
	ఒకవేళ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

స్థిర శూన్య open_shell(శూన్య)
{
	స్థిరరాశి అక్షరం *shell;
	నిర్మాణం passwd *pw;

#నిర్వచితమైతే __linux__
	స్థిరరాశి అక్షరం *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ఒకవేళ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (అక్షరం *) ఖాళీ);
		perror("init");
		తిరిగి;
	}
#అయితే_ముగింపు

	shell = rc_conf_విలువ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ఒకవేళ (!shell) {
		shell = getenv("SHELL");
		ఒకవేళ (!shell) {
			pw = getpwuid(getuid());
			ఒకవేళ (pw)
				shell = pw->pw_shell;
			ఒకవేళ (!shell)
				shell = "/bin/sh";
		}
	}
	నిరంతరం_కార్యక్రమం(shell);
}

స్థిర శూన్య handle_single(శూన్య)
{
	do_openrc("single");
	open_shell();
	init(ఖాళీ);
}

స్థిర శూన్య dispatch_signal(పూర్ణాంకం sigfd)
{
	పూర్ణాంకం sig;
	ssize_t లెక్క = పఠ(sigfd, &sig, పరిమాణం(sig));

	ఒకవేళ (లెక్క < 0) {
		perror("read(sigpipe)");
		తిరిగి;
	}

	ఒకవేళ (లెక్క != పరిమాణం(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		తిరిగి;
	}

	ఎంపిక (sig) {
		స్థితి SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			ఆపు;
		స్థితి SIGTERM:
#నిర్వచితమైతే SIGPWR
		స్థితి SIGPWR:
#అయితే_ముగింపు
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			ఆపు;
		అప్రమేయ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			ఆపు;
	}
}

స్థిర శూన్య dispatch_fifo(పూర్ణాంకం fifo)
{
	అక్షరం buf[2048];
	ssize_t లెక్క = పఠ(fifo, buf, పరిమాణం(buf) - 1);

	ఒకవేళ (లెక్క < 0) {
		perror("read(fifo)");
		తిరిగి;
	}

	buf[లెక్క] = 0;

	ఒకవేళ (!quiet)
		రాయి("PID1: Received \"%s\" from FIFO...\n", buf);

	ఒకవేళ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	లేకపోతే ఒకవేళ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	లేకపోతే ఒకవేళ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	లేకపోతే ఒకవేళ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	లేకపోతే ఒకవేళ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	లేకపోతే ఒకవేళ (strcmp(buf, "single") == 0)
		handle_single();
}

స్థిర శూన్య signal_handler(పూర్ణాంకం sig)
{
	పూర్ణాంకం saved_errno = errno;
	లిఖ(sigpipe[1], &sig, పరిమాణం(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

స్థిర శూన్య reap_zombies(పూర్ణాంకం sig RC_వాడని)
{
	అక్షరం errmsg[] = "waitpid() failed\n";
	పూర్ణాంకం saved_errno = errno;

	కొరకు (;;) {
		pid_t pid = waitpid(-1, ఖాళీ, WNOHANG);
		ఒకవేళ (pid == 0)
			ఆపు;
		లేకపోతే ఒకవేళ (pid == -1) {
			ఒకవేళ (errno == ECHILD)
				ఆపు;
			లిఖ(STDERR_FILENO, errmsg, పరిమాణం(errmsg) - 1);
			కొనసాగు;
		}
	}
	errno = saved_errno;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	అక్షరం *cmdline_runlevel = ఖాళీ;
	పూర్ణాంకం fifo;
	bool reexec = false;
	sigset_t సంకేతాలు;
	నిర్మాణం sigaction sa;
#నిర్వచితమైతే HAVE_SELINUX
	పూర్ణాంకం enforce = 0;
#అయితే_ముగింపు

	progname = argv[0];

	ఒకవేళ (getpid() != 1)
		తిరిగి 1;

#నిర్వచితమైతే HAVE_SELINUX
	ఒకవేళ (getenv("SELINUX_INIT") == ఖాళీ) {
		ఒకవేళ (is_selinux_enabled() != 1) {
			ఒకవేళ (selinux_init_ఎక్కించు_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} లేకపోతే {
				ఒకవేళ (enforce > 0) {
					/*
					 * SELinux in enforcing mode but load_policy failed
					 * At this point, we probably can't open /dev/console,
					 * so log() won't work
					 */
					fprintf(stderr,"Unable to load SELinux Policy.\n");
					fprintf(stderr,"Machine is  in enforcing mode.\n");
					fprintf(stderr,"Halting now.\n");
					నిష్క్రమించు(1);
				}
			}
		}
	}
#అయితే_ముగింపు

	quiet = rc_yesno(getenv("EINFO_QUIET"));

	ఒకవేళ (!quiet)
		రాయి("OpenRC init version %s starting\n", సంస్కరణ);

	ఒకవేళ (argc > 1) {
		ఒకవేళ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		లేకపోతే
			cmdline_runlevel = argv[1];
	}

	ఒకవేళ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		తిరిగి 1;
	}

	/* block all signals we do not handle */
	sigfillset(&సంకేతాలు);
	sigdelset(&సంకేతాలు, SIGCHLD);
	sigdelset(&సంకేతాలు, SIGINT);
	sigdelset(&సంకేతాలు, SIGTERM);
#నిర్వచితమైతే SIGPWR
	sigdelset(&సంకేతాలు, SIGPWR);
#అయితే_ముగింపు
	sigprocmask(SIG_SETMASK, &సంకేతాలు, ఖాళీ);

	/* install signal  handler */
	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ఖాళీ);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ఖాళీ);
	sigaction(SIGTERM, &sa, ఖాళీ);
#నిర్వచితమైతే SIGPWR
	sigaction(SIGPWR, &sa, ఖాళీ);
#అయితే_ముగింపు
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", మార్గం_default, 1);

	ఒకవేళ (!reexec)
		init(cmdline_runlevel);

	ఒకవేళ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	కొరకు (;;) {
		గణన { FD_FIFO, FD_SIG, FD_లెక్క };
		నిర్మాణం pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_లెక్క, -1);

		ఒకవేళ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		ఒకవేళ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	తిరిగి 0;
}
