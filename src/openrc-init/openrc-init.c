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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SELINUX
#  include <selinux/selinux.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "wtmp.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ಆದರೆ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ_default = "/sbin:/usr/sbin:/bin:/usr/bin";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rc_default_runlevel = "default";
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sigpipe[2] = { -1, -1 };
ಸ್ಥಿರ bool quiet = false;
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *progname;

ಸ್ಥಿರ ಶೂನ್ಯ do_openrc(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = { "openrc", runlevel, ಖಾಲಿ };
	ರಚನೆ exec_ಫಲಿತಾಂಶ res;
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು = exec_init(argv);
	ವಾದಗಳು.setsid = 1;

	ಆದರೆ (!quiet)
		ಬರೆ("Starting %s runlevel\n", runlevel);
	res = do_exec(&ವಾದಗಳು);
	ಆದರೆ (res.pid < 0) {
		perror("do_exec");
		ನಿರ್ಗಮಿಸು(1);
	}
	ಆಗ (waitpid(res.pid, ಖಾಲಿ, 0) != res.pid)
		ಆದರೆ (errno == ECHILD)
			ನಿಲ್ಲಿಸು;
}

ಸ್ಥಿರ ಶೂನ್ಯ init(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *default_runlevel)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel = ಖಾಲಿ;
	do_openrc("sysinit");
	do_openrc("boot");
	ಆದರೆ (default_runlevel)
		runlevel = default_runlevel;
	ಇಲ್ಲದಿದ್ದರೆ
		runlevel = rc_conf_ಮೌಲ್ಯ("rc_default_runlevel");
	ಆದರೆ (!runlevel)
		runlevel = rc_default_runlevel;
	ಆದರೆ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_reexec(ಶೂನ್ಯ)
{
	execlp(progname, progname, "reexec", ಖಾಲಿ);
	ಹಿಂತಿರುಗಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_shutdown(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಪೂರ್ಣಾಂಕ cmd)
{
	ಸ್ಥಿರಾಂಕ ದೀರ್ಘ ms_ಗಮ್ಯ_ns = 1000000L;
	ರಚನೆ timespec ಆರಂಭ, now, tmp, remain;

	do_openrc(runlevel);
	ಆದರೆ (!quiet)
		ಬರೆ("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &ಆರಂಭ);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_ಗಮ್ಯ_ns;
	ಆಗ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	ಗಾಗಿ (;;) {
		pid_t pid = waitpid(-1, ಖಾಲಿ, WNOHANG);
		ಆದರೆ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			ನಿಲ್ಲಿಸು;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &ಆರಂಭ, &tmp);
		ಆದರೆ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			ನಿಲ್ಲಿಸು;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_ಗಮ್ಯ_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ಖಾಲಿ);
	}

	ಆದರೆ (!quiet)
		ಬರೆ("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	ಆದರೆ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

ಸ್ಥಿರ ಶೂನ್ಯ ಸತತ_ಕಾರ್ಯಕ್ರಮ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prog)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = { prog, ಖಾಲಿ };
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು = exec_init(argv);
	ರಚನೆ exec_ಫಲಿತಾಂಶ res = do_exec(&ವಾದಗಳು);
	ಆದರೆ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

ಸ್ಥಿರ ಶೂನ್ಯ open_shell(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell;
	ರಚನೆ passwd *pw;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ಆದರೆ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ಅಕ್ಷರ *) ಖಾಲಿ);
		perror("init");
		ಹಿಂತಿರುಗಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ

	shell = rc_conf_ಮೌಲ್ಯ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ಆದರೆ (!shell) {
		shell = getenv("SHELL");
		ಆದರೆ (!shell) {
			pw = getpwuid(getuid());
			ಆದರೆ (pw)
				shell = pw->pw_shell;
			ಆದರೆ (!shell)
				shell = "/bin/sh";
		}
	}
	ಸತತ_ಕಾರ್ಯಕ್ರಮ(shell);
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_single(ಶೂನ್ಯ)
{
	do_openrc("single");
	open_shell();
	init(ಖಾಲಿ);
}

ಸ್ಥಿರ ಶೂನ್ಯ dispatch_signal(ಪೂರ್ಣಾಂಕ sigfd)
{
	ಪೂರ್ಣಾಂಕ sig;
	ssize_t ಎಣಿಕೆ_2 = ಪಠ(sigfd, &sig, ಗಾತ್ರ(sig));

	ಆದರೆ (ಎಣಿಕೆ_2 < 0) {
		perror("read(sigpipe)");
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (ಎಣಿಕೆ_2 != ಗಾತ್ರ(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ಹಿಂತಿರುಗಿ;
	}

	ಆಯ್ಕೆ (sig) {
		ಸ್ಥಿತಿ SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ SIGTERM:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPWR
		ಸ್ಥಿತಿ SIGPWR:
#ಆದರೆ_ಅಂತ್ಯ
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			ನಿಲ್ಲಿಸು;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ dispatch_fifo(ಪೂರ್ಣಾಂಕ fifo)
{
	ಅಕ್ಷರ buf[2048];
	ssize_t ಎಣಿಕೆ_2 = ಪಠ(fifo, buf, ಗಾತ್ರ(buf) - 1);

	ಆದರೆ (ಎಣಿಕೆ_2 < 0) {
		perror("read(fifo)");
		ಹಿಂತಿರುಗಿ;
	}

	buf[ಎಣಿಕೆ_2] = 0;

	ಆದರೆ (!quiet)
		ಬರೆ("PID1: Received \"%s\" from FIFO...\n", buf);

	ಆದರೆ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(buf, "single") == 0)
		handle_single();
}

ಸ್ಥಿರ ಶೂನ್ಯ signal_handler(ಪೂರ್ಣಾಂಕ sig)
{
	ಪೂರ್ಣಾಂಕ saved_errno = errno;
	ಲಿಖ(sigpipe[1], &sig, ಗಾತ್ರ(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

ಸ್ಥಿರ ಶೂನ್ಯ reap_zombies(ಪೂರ್ಣಾಂಕ sig RC_ಬಳಸದ)
{
	ಅಕ್ಷರ errmsg[] = "waitpid() failed\n";
	ಪೂರ್ಣಾಂಕ saved_errno = errno;

	ಗಾಗಿ (;;) {
		pid_t pid = waitpid(-1, ಖಾಲಿ, WNOHANG);
		ಆದರೆ (pid == 0)
			ನಿಲ್ಲಿಸು;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pid == -1) {
			ಆದರೆ (errno == ECHILD)
				ನಿಲ್ಲಿಸು;
			ಲಿಖ(STDERR_FILENO, errmsg, ಗಾತ್ರ(errmsg) - 1);
			ಮುಂದುವರಿ;
		}
	}
	errno = saved_errno;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *cmdline_runlevel = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ fifo;
	bool reexec = false;
	sigset_t ಸಂಕೇತಗಳು;
	ರಚನೆ sigaction sa;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SELINUX
	ಪೂರ್ಣಾಂಕ enforce = 0;
#ಆದರೆ_ಅಂತ್ಯ

	progname = argv[0];

	ಆದರೆ (getpid() != 1)
		ಹಿಂತಿರುಗಿ 1;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SELINUX
	ಆದರೆ (getenv("SELINUX_INIT") == ಖಾಲಿ) {
		ಆದರೆ (is_selinux_enabled() != 1) {
			ಆದರೆ (selinux_init_ತುಂಬು_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (enforce > 0) {
					/*
					 * SELinux in enforcing mode but load_policy failed
					 * At this point, we probably can't open /dev/console,
					 * so log() won't work
					 */
					fprintf(stderr,"Unable to load SELinux Policy.\n");
					fprintf(stderr,"Machine is  in enforcing mode.\n");
					fprintf(stderr,"Halting now.\n");
					ನಿರ್ಗಮಿಸು(1);
				}
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	quiet = rc_yesno(getenv("EINFO_QUIET"));

	ಆದರೆ (!quiet)
		ಬರೆ("OpenRC init version %s starting\n", ಆವೃತ್ತಿ);

	ಆದರೆ (argc > 1) {
		ಆದರೆ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		ಇಲ್ಲದಿದ್ದರೆ
			cmdline_runlevel = argv[1];
	}

	ಆದರೆ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ಹಿಂತಿರುಗಿ 1;
	}

	/* block all signals we do not handle */
	sigfillset(&ಸಂಕೇತಗಳು);
	sigdelset(&ಸಂಕೇತಗಳು, SIGCHLD);
	sigdelset(&ಸಂಕೇತಗಳು, SIGINT);
	sigdelset(&ಸಂಕೇತಗಳು, SIGTERM);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPWR
	sigdelset(&ಸಂಕೇತಗಳು, SIGPWR);
#ಆದರೆ_ಅಂತ್ಯ
	sigprocmask(SIG_SETMASK, &ಸಂಕೇತಗಳು, ಖಾಲಿ);

	/* install signal  handler */
	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ಖಾಲಿ);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ಖಾಲಿ);
	sigaction(SIGTERM, &sa, ಖಾಲಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPWR
	sigaction(SIGPWR, &sa, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", ಮಾರ್ಗ_default, 1);

	ಆದರೆ (!reexec)
		init(cmdline_runlevel);

	ಆದರೆ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	ಗಾಗಿ (;;) {
		ಎಣಿಕೆ { FD_FIFO, FD_SIG, FD_ಎಣಿಕೆ };
		ರಚನೆ pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_ಎಣಿಕೆ, -1);

		ಆದರೆ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		ಆದರೆ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ಹಿಂತಿರುಗಿ 0;
}
