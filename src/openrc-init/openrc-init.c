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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/reboot.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <utmp.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SELINUX
#  include <selinux/selinux.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "wtmp.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ਜੇ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਰਸਤਾ_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *rc_default_runlevel = "default";
ਸਥਿਰ ਪੂਰਨ sigpipe[2] = { -1, -1 };
ਸਥਿਰ bool quiet = false;
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *progname;

ਸਥਿਰ ਖਾਲੀ do_openrc(ਅਚਲ ਅੱਖਰ *runlevel)
{
	ਅਚਲ ਅੱਖਰ *argv[] = { "openrc", runlevel, ਨਿਰਾ };
	ਬਣਤਰ exec_ਨਤੀਜਾ res;
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ = exec_init(argv);
	ਦਲੀਲਾਂ.setsid = 1;

	ਜੇ (!quiet)
		ਲਿਖੋ("Starting %s runlevel\n", runlevel);
	res = do_exec(&ਦਲੀਲਾਂ);
	ਜੇ (res.pid < 0) {
		perror("do_exec");
		ਬਾਹਰ_ਨਿਕਲੋ(1);
	}
	ਜਦੋਂ (waitpid(res.pid, ਨਿਰਾ, 0) != res.pid)
		ਜੇ (errno == ECHILD)
			ਰੋਕੋ;
}

ਸਥਿਰ ਖਾਲੀ init(ਅਚਲ ਅੱਖਰ *default_runlevel)
{
	ਅਚਲ ਅੱਖਰ *runlevel = ਨਿਰਾ;
	do_openrc("sysinit");
	do_openrc("boot");
	ਜੇ (default_runlevel)
		runlevel = default_runlevel;
	ਨਹੀਂਤਾਂ
		runlevel = rc_conf_ਮੁੱਲ("rc_default_runlevel");
	ਜੇ (!runlevel)
		runlevel = rc_default_runlevel;
	ਜੇ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ਸਥਿਰ ਖਾਲੀ handle_reexec(ਖਾਲੀ)
{
	execlp(progname, progname, "reexec", ਨਿਰਾ);
	ਵਾਪਸ;
}

ਸਥਿਰ ਖਾਲੀ handle_shutdown(ਅਚਲ ਅੱਖਰ *runlevel, ਪੂਰਨ cmd)
{
	ਅਚਲ ਲੰਮਾ ms_ਮੰਜ਼ਿਲ_ns = 1000000L;
	ਬਣਤਰ timespec ਸ਼ੁਰੂ, now, tmp, remain;

	do_openrc(runlevel);
	ਜੇ (!quiet)
		ਲਿਖੋ("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &ਸ਼ੁਰੂ);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_ਮੰਜ਼ਿਲ_ns;
	ਜਦੋਂ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	ਲਈ (;;) {
		pid_t pid = waitpid(-1, ਨਿਰਾ, WNOHANG);
		ਜੇ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			ਰੋਕੋ;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &ਸ਼ੁਰੂ, &tmp);
		ਜੇ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			ਰੋਕੋ;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_ਮੰਜ਼ਿਲ_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ਨਿਰਾ);
	}

	ਜੇ (!quiet)
		ਲਿਖੋ("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	ਜੇ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ਸਥਿਰ ਖਾਲੀ ਲੜੀਵਾਰ_ਪ੍ਰੋਗਰਾਮ(ਅਚਲ ਅੱਖਰ *prog)
{
	ਅਚਲ ਅੱਖਰ *argv[] = { prog, ਨਿਰਾ };
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ = exec_init(argv);
	ਬਣਤਰ exec_ਨਤੀਜਾ res = do_exec(&ਦਲੀਲਾਂ);
	ਜੇ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ਸਥਿਰ ਖਾਲੀ open_shell(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *shell;
	ਬਣਤਰ passwd *pw;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਅਚਲ ਅੱਖਰ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ਜੇ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ਅੱਖਰ *) ਨਿਰਾ);
		perror("init");
		ਵਾਪਸ;
	}
#ਜੇ_ਅੰਤ

	shell = rc_conf_ਮੁੱਲ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ਜੇ (!shell) {
		shell = getenv("SHELL");
		ਜੇ (!shell) {
			pw = getpwuid(getuid());
			ਜੇ (pw)
				shell = pw->pw_shell;
			ਜੇ (!shell)
				shell = "/bin/sh";
		}
	}
	ਲੜੀਵਾਰ_ਪ੍ਰੋਗਰਾਮ(shell);
}

ਸਥਿਰ ਖਾਲੀ handle_single(ਖਾਲੀ)
{
	do_openrc("single");
	open_shell();
	init(ਨਿਰਾ);
}

ਸਥਿਰ ਖਾਲੀ dispatch_signal(ਪੂਰਨ sigfd)
{
	ਪੂਰਨ sig;
	ssize_t ਗਿਣਤੀ_2 = ਪਠ(sigfd, &sig, ਆਕਾਰ(sig));

	ਜੇ (ਗਿਣਤੀ_2 < 0) {
		perror("read(sigpipe)");
		ਵਾਪਸ;
	}

	ਜੇ (ਗਿਣਤੀ_2 != ਆਕਾਰ(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ਵਾਪਸ;
	}

	ਚੋਣ (sig) {
		ਹਾਲਤ SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			ਰੋਕੋ;
		ਹਾਲਤ SIGTERM:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPWR
		ਹਾਲਤ SIGPWR:
#ਜੇ_ਅੰਤ
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			ਰੋਕੋ;
		ਮੂਲ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			ਰੋਕੋ;
	}
}

ਸਥਿਰ ਖਾਲੀ dispatch_fifo(ਪੂਰਨ fifo)
{
	ਅੱਖਰ buf[2048];
	ssize_t ਗਿਣਤੀ_2 = ਪਠ(fifo, buf, ਆਕਾਰ(buf) - 1);

	ਜੇ (ਗਿਣਤੀ_2 < 0) {
		perror("read(fifo)");
		ਵਾਪਸ;
	}

	buf[ਗਿਣਤੀ_2] = 0;

	ਜੇ (!quiet)
		ਲਿਖੋ("PID1: Received \"%s\" from FIFO...\n", buf);

	ਜੇ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(buf, "single") == 0)
		handle_single();
}

ਸਥਿਰ ਖਾਲੀ signal_handler(ਪੂਰਨ sig)
{
	ਪੂਰਨ saved_errno = errno;
	ਲਿਖ(sigpipe[1], &sig, ਆਕਾਰ(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ਸਥਿਰ ਖਾਲੀ reap_zombies(ਪੂਰਨ sig RC_ਅਣਵਰਤਿਆ)
{
	ਅੱਖਰ errmsg[] = "waitpid() failed\n";
	ਪੂਰਨ saved_errno = errno;

	ਲਈ (;;) {
		pid_t pid = waitpid(-1, ਨਿਰਾ, WNOHANG);
		ਜੇ (pid == 0)
			ਰੋਕੋ;
		ਨਹੀਂਤਾਂ ਜੇ (pid == -1) {
			ਜੇ (errno == ECHILD)
				ਰੋਕੋ;
			ਲਿਖ(STDERR_FILENO, errmsg, ਆਕਾਰ(errmsg) - 1);
			ਜਾਰੀ;
		}
	}
	errno = saved_errno;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *cmdline_runlevel = ਨਿਰਾ;
	ਪੂਰਨ fifo;
	bool reexec = false;
	sigset_t ਸੰਕੇਤ_2;
	ਬਣਤਰ sigaction sa;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SELINUX
	ਪੂਰਨ enforce = 0;
#ਜੇ_ਅੰਤ

	progname = argv[0];

	ਜੇ (getpid() != 1)
		ਵਾਪਸ 1;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SELINUX
	ਜੇ (getenv("SELINUX_INIT") == ਨਿਰਾ) {
		ਜੇ (is_selinux_enabled() != 1) {
			ਜੇ (selinux_init_ਲੱਦੋ_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} ਨਹੀਂਤਾਂ {
				ਜੇ (enforce > 0) {
					/*
					 * SELinux in enforcing mode but load_policy failed
					 * At this point, we probably can't open /dev/console,
					 * so log() won't work
					 */
					fprintf(stderr,"Unable to load SELinux Policy.\n");
					fprintf(stderr,"Machine is  in enforcing mode.\n");
					fprintf(stderr,"Halting now.\n");
					ਬਾਹਰ_ਨਿਕਲੋ(1);
				}
			}
		}
	}
#ਜੇ_ਅੰਤ

	quiet = rc_yesno(getenv("EINFO_QUIET"));

	ਜੇ (!quiet)
		ਲਿਖੋ("OpenRC init version %s starting\n", ਸੰਸਕਰਣ);

	ਜੇ (argc > 1) {
		ਜੇ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		ਨਹੀਂਤਾਂ
			cmdline_runlevel = argv[1];
	}

	ਜੇ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ਵਾਪਸ 1;
	}

	/* block all signals we do not handle */
	sigfillset(&ਸੰਕੇਤ_2);
	sigdelset(&ਸੰਕੇਤ_2, SIGCHLD);
	sigdelset(&ਸੰਕੇਤ_2, SIGINT);
	sigdelset(&ਸੰਕੇਤ_2, SIGTERM);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPWR
	sigdelset(&ਸੰਕੇਤ_2, SIGPWR);
#ਜੇ_ਅੰਤ
	sigprocmask(SIG_SETMASK, &ਸੰਕੇਤ_2, ਨਿਰਾ);

	/* install signal  handler */
	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ਨਿਰਾ);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ਨਿਰਾ);
	sigaction(SIGTERM, &sa, ਨਿਰਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPWR
	sigaction(SIGPWR, &sa, ਨਿਰਾ);
#ਜੇ_ਅੰਤ
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", ਰਸਤਾ_default, 1);

	ਜੇ (!reexec)
		init(cmdline_runlevel);

	ਜੇ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	ਲਈ (;;) {
		ਗਿਣਤੀ { FD_FIFO, FD_SIG, FD_ਗਿਣਤੀ };
		ਬਣਤਰ pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_ਗਿਣਤੀ, -1);

		ਜੇ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		ਜੇ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ਵਾਪਸ 0;
}
