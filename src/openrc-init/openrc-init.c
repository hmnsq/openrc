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

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#include "rc.h"
#include "rc_exec.h"
#include "wtmp.h"
#include "helpers.h"
#include "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ᮚᮦᮔ᮪ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

static const var_na *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
static const var_na *rc_default_runlevel = "default";
static puur_naa_nka sigpipe[2] = { -1, -1 };
static bool quiet = false;
static const var_na *progname;

static void do_openrc(const var_na *runlevel)
{
	const var_na *argv[] = { "openrc", runlevel, NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args = exec_init(argv);
	args.setsid = 1;

	ᮚᮦᮔ᮪ (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	ᮚᮦᮔ᮪ (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	ᮞᮨᮓᮨᮜ (waitpid(res.pid, NULL, 0) != res.pid)
		ᮚᮦᮔ᮪ (errno == ECHILD)
			bha_nga;
}

static void init(const var_na *default_runlevel)
{
	const var_na *runlevel = NULL;
	do_openrc("sysinit");
	do_openrc("boot");
	ᮚᮦᮔ᮪ (default_runlevel)
		runlevel = default_runlevel;
	ᮜᮤᮚ
		runlevel = rc_conf_value("rc_default_runlevel");
	ᮚᮦᮔ᮪ (!runlevel)
		runlevel = rc_default_runlevel;
	ᮚᮦᮔ᮪ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

static void handle_reexec(void)
{
	execlp(progname, progname, "reexec", NULL);
	ᮝᮜᮤ;
}

static void handle_shutdown(const var_na *runlevel, puur_naa_nka cmd)
{
	const long ms_to_ns = 1000000L;
	sa_nracanaa timespec start, now, tmp, remain;

	do_openrc(runlevel);
	ᮚᮦᮔ᮪ (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	ᮞᮨᮓᮨᮜ (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	for (;;) {
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		ᮚᮦᮔ᮪ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			bha_nga;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		ᮚᮦᮔ᮪ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			bha_nga;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, NULL);
	}

	ᮚᮦᮔ᮪ (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	ᮚᮦᮔ᮪ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

static void run_program(const var_na *prog)
{
	const var_na *argv[] = { prog, NULL };
	sa_nracanaa exec_args args = exec_init(argv);
	sa_nracanaa exec_result res = do_exec(&args);
	ᮚᮦᮔ᮪ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

static void open_shell(void)
{
	const var_na *shell;
	sa_nracanaa passwd *pw;

#ifdef __linux__
	const var_na *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ᮚᮦᮔ᮪ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (var_na *) NULL);
		perror("init");
		ᮝᮜᮤ;
	}
#endif

	shell = rc_conf_value("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ᮚᮦᮔ᮪ (!shell) {
		shell = getenv("SHELL");
		ᮚᮦᮔ᮪ (!shell) {
			pw = getpwuid(getuid());
			ᮚᮦᮔ᮪ (pw)
				shell = pw->pw_shell;
			ᮚᮦᮔ᮪ (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

static void handle_single(void)
{
	do_openrc("single");
	open_shell();
	init(NULL);
}

static void dispatch_signal(puur_naa_nka sigfd)
{
	puur_naa_nka sig;
	ssize_t count = read(sigfd, &sig, parimaa_na(sig));

	ᮚᮦᮔ᮪ (count < 0) {
		perror("read(sigpipe)");
		ᮝᮜᮤ;
	}

	ᮚᮦᮔ᮪ (count != parimaa_na(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ᮝᮜᮤ;
	}

	switch (sig) {
		pak_sa SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			bha_nga;
		pak_sa SIGTERM:
#ifdef SIGPWR
		pak_sa SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			bha_nga;
		default:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			bha_nga;
	}
}

static void dispatch_fifo(puur_naa_nka fifo)
{
	var_na buf[2048];
	ssize_t count = read(fifo, buf, parimaa_na(buf) - 1);

	ᮚᮦᮔ᮪ (count < 0) {
		perror("read(fifo)");
		ᮝᮜᮤ;
	}

	buf[count] = 0;

	ᮚᮦᮔ᮪ (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	ᮚᮦᮔ᮪ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	ᮜᮤᮚ ᮚᮦᮔ᮪ (strcmp(buf, "single") == 0)
		handle_single();
}

static void signal_handler(puur_naa_nka sig)
{
	puur_naa_nka saved_errno = errno;
	write(sigpipe[1], &sig, parimaa_na(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

static void reap_zombies(puur_naa_nka sig RC_UNUSED)
{
	var_na errmsg[] = "waitpid() failed\n";
	puur_naa_nka saved_errno = errno;

	for (;;) {
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		ᮚᮦᮔ᮪ (pid == 0)
			bha_nga;
		ᮜᮤᮚ ᮚᮦᮔ᮪ (pid == -1) {
			ᮚᮦᮔ᮪ (errno == ECHILD)
				bha_nga;
			write(STDERR_FILENO, errmsg, parimaa_na(errmsg) - 1);
			continue;
		}
	}
	errno = saved_errno;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	var_na *cmdline_runlevel = NULL;
	puur_naa_nka fifo;
	bool reexec = false;
	sigset_t signals;
	sa_nracanaa sigaction sa;
#ifdef HAVE_SELINUX
	puur_naa_nka enforce = 0;
#endif

	progname = argv[0];

	ᮚᮦᮔ᮪ (getpid() != 1)
		ᮝᮜᮤ 1;

#ifdef HAVE_SELINUX
	ᮚᮦᮔ᮪ (getenv("SELINUX_INIT") == NULL) {
		ᮚᮦᮔ᮪ (is_selinux_enabled() != 1) {
			ᮚᮦᮔ᮪ (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} ᮜᮤᮚ {
				ᮚᮦᮔ᮪ (enforce > 0) {
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

	ᮚᮦᮔ᮪ (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	ᮚᮦᮔ᮪ (argc > 1) {
		ᮚᮦᮔ᮪ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		ᮜᮤᮚ
			cmdline_runlevel = argv[1];
	}

	ᮚᮦᮔ᮪ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ᮝᮜᮤ 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, NULL);

	/* install signal  handler */
	memset(&sa, 0, parimaa_na(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, NULL);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, NULL);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	ᮚᮦᮔ᮪ (!reexec)
		init(cmdline_runlevel);

	ᮚᮦᮔ᮪ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	for (;;) {
		ga_nanaa { FD_FIFO, FD_SIG, FD_COUNT };
		sa_nracanaa pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_COUNT, -1);

		ᮚᮦᮔ᮪ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		ᮚᮦᮔ᮪ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ᮝᮜᮤ 0;
}
