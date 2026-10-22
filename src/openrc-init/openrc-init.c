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

#اگر_تعریف_شدہ HAVE_SELINUX
#  include <selinux/selinux.h>
#اگر_ختم

#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "wtmp.h"
#شامل "helpers.h"
#شامل "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#اگر defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#اگر_ختم

ساکن ثابت حرف *راستہ_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ساکن ثابت حرف *rc_default_runlevel = "default";
ساکن صحیح sigpipe[2] = { -1, -1 };
ساکن bool quiet = false;
ساکن ثابت حرف *progname;

ساکن خالی do_openrc(ثابت حرف *runlevel)
{
	ثابت حرف *argv[] = { "openrc", runlevel, تہی };
	ساخت exec_نتیجہ res;
	ساخت exec_دلائل دلائل = exec_init(argv);
	دلائل.setsid = 1;

	اگر (!quiet)
		لکھو("Starting %s runlevel\n", runlevel);
	res = do_exec(&دلائل);
	اگر (res.pid < 0) {
		perror("do_exec");
		باہر_نکلو(1);
	}
	جب (waitpid(res.pid, تہی, 0) != res.pid)
		اگر (errno == ECHILD)
			توڑو;
}

ساکن خالی init(ثابت حرف *default_runlevel)
{
	ثابت حرف *runlevel = تہی;
	do_openrc("sysinit");
	do_openrc("boot");
	اگر (default_runlevel)
		runlevel = default_runlevel;
	ورنہ
		runlevel = rc_conf_قدر("rc_default_runlevel");
	اگر (!runlevel)
		runlevel = rc_default_runlevel;
	اگر (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ساکن خالی handle_reexec(خالی)
{
	execlp(progname, progname, "reexec", تہی);
	واپس;
}

ساکن خالی handle_shutdown(ثابت حرف *runlevel, صحیح cmd)
{
	ثابت طویل ms_مقصد_ns = 1000000L;
	ساخت timespec آغاز, now, tmp, remain;

	do_openrc(runlevel);
	اگر (!quiet)
		لکھو("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &آغاز);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_مقصد_ns;
	جب (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	برائے (;;) {
		pid_t pid = waitpid(-1, تہی, WNOHANG);
		اگر (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			توڑو;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &آغاز, &tmp);
		اگر (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			توڑو;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_مقصد_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, تہی);
	}

	اگر (!quiet)
		لکھو("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	اگر (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ساکن خالی تسلسل_پروگرام(ثابت حرف *prog)
{
	ثابت حرف *argv[] = { prog, تہی };
	ساخت exec_دلائل دلائل = exec_init(argv);
	ساخت exec_نتیجہ res = do_exec(&دلائل);
	اگر (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ساکن خالی open_shell(خالی)
{
	ثابت حرف *shell;
	ساخت passwd *pw;

#اگر_تعریف_شدہ __linux__
	ثابت حرف *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	اگر (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (حرف *) تہی);
		perror("init");
		واپس;
	}
#اگر_ختم

	shell = rc_conf_قدر("rc_shell");
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
	تسلسل_پروگرام(shell);
}

ساکن خالی handle_single(خالی)
{
	do_openrc("single");
	open_shell();
	init(تہی);
}

ساکن خالی dispatch_signal(صحیح sigfd)
{
	صحیح sig;
	ssize_t گنتی = اقرأ(sigfd, &sig, حجم(sig));

	اگر (گنتی < 0) {
		perror("read(sigpipe)");
		واپس;
	}

	اگر (گنتی != حجم(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		واپس;
	}

	انتخاب (sig) {
		حالت SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			توڑو;
		حالت SIGTERM:
#اگر_تعریف_شدہ SIGPWR
		حالت SIGPWR:
#اگر_ختم
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			توڑو;
		طےشدہ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			توڑو;
	}
}

ساکن خالی dispatch_fifo(صحیح fifo)
{
	حرف buf[2048];
	ssize_t گنتی = اقرأ(fifo, buf, حجم(buf) - 1);

	اگر (گنتی < 0) {
		perror("read(fifo)");
		واپس;
	}

	buf[گنتی] = 0;

	اگر (!quiet)
		لکھو("PID1: Received \"%s\" from FIFO...\n", buf);

	اگر (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	ورنہ اگر (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	ورنہ اگر (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	ورنہ اگر (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	ورنہ اگر (strcmp(buf, "reexec") == 0)
		handle_reexec();
	ورنہ اگر (strcmp(buf, "single") == 0)
		handle_single();
}

ساکن خالی signal_handler(صحیح sig)
{
	صحیح saved_errno = errno;
	اكتب(sigpipe[1], &sig, حجم(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ساکن خالی reap_zombies(صحیح sig RC_غیر_مستعمل)
{
	حرف errmsg[] = "waitpid() failed\n";
	صحیح saved_errno = errno;

	برائے (;;) {
		pid_t pid = waitpid(-1, تہی, WNOHANG);
		اگر (pid == 0)
			توڑو;
		ورنہ اگر (pid == -1) {
			اگر (errno == ECHILD)
				توڑو;
			اكتب(STDERR_FILENO, errmsg, حجم(errmsg) - 1);
			جاری;
		}
	}
	errno = saved_errno;
}

صحیح مین(صحیح argc, حرف **argv)
{
	حرف *cmdline_runlevel = تہی;
	صحیح fifo;
	bool reexec = false;
	sigset_t اشارات;
	ساخت sigaction sa;
#اگر_تعریف_شدہ HAVE_SELINUX
	صحیح enforce = 0;
#اگر_ختم

	progname = argv[0];

	اگر (getpid() != 1)
		واپس 1;

#اگر_تعریف_شدہ HAVE_SELINUX
	اگر (getenv("SELINUX_INIT") == تہی) {
		اگر (is_selinux_enabled() != 1) {
			اگر (selinux_init_بھرو_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} ورنہ {
				اگر (enforce > 0) {
					/*
					 * SELinux in enforcing mode but load_policy failed
					 * At this point, we probably can't open /dev/console,
					 * so log() won't work
					 */
					fprintf(stderr,"Unable to load SELinux Policy.\n");
					fprintf(stderr,"Machine is  in enforcing mode.\n");
					fprintf(stderr,"Halting now.\n");
					باہر_نکلو(1);
				}
			}
		}
	}
#اگر_ختم

	quiet = rc_yesno(getenv("EINFO_QUIET"));

	اگر (!quiet)
		لکھو("OpenRC init version %s starting\n", نسخہ);

	اگر (argc > 1) {
		اگر (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		ورنہ
			cmdline_runlevel = argv[1];
	}

	اگر (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		واپس 1;
	}

	/* block all signals we do not handle */
	sigfillset(&اشارات);
	sigdelset(&اشارات, SIGCHLD);
	sigdelset(&اشارات, SIGINT);
	sigdelset(&اشارات, SIGTERM);
#اگر_تعریف_شدہ SIGPWR
	sigdelset(&اشارات, SIGPWR);
#اگر_ختم
	sigprocmask(SIG_SETMASK, &اشارات, تہی);

	/* install signal  handler */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, تہی);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, تہی);
	sigaction(SIGTERM, &sa, تہی);
#اگر_تعریف_شدہ SIGPWR
	sigaction(SIGPWR, &sa, تہی);
#اگر_ختم
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", راستہ_default, 1);

	اگر (!reexec)
		init(cmdline_runlevel);

	اگر ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	برائے (;;) {
		شمار { FD_FIFO, FD_SIG, FD_گنتی };
		ساخت pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_گنتی, -1);

		اگر (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		اگر (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	واپس 0;
}
