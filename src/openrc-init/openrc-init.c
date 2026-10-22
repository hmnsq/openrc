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
#အကယ်၍ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

static const var_na *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
static const var_na *rc_default_runlevel = "default";
static ကိန်းပြည့် sigpipe[2] = { -1, -1 };
static bool quiet = false;
static const var_na *progname;

static void do_openrc(const var_na *runlevel)
{
	const var_na *argv[] = { "openrc", runlevel, NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args = exec_init(argv);
	args.setsid = 1;

	အကယ်၍ (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	အကယ်၍ (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	နေစဉ် (waitpid(res.pid, NULL, 0) != res.pid)
		အကယ်၍ (errno == ECHILD)
			bha_nga;
}

static void init(const var_na *default_runlevel)
{
	const var_na *runlevel = NULL;
	do_openrc("sysinit");
	do_openrc("boot");
	အကယ်၍ (default_runlevel)
		runlevel = default_runlevel;
	မဟုတ်ပါက
		runlevel = rc_conf_value("rc_default_runlevel");
	အကယ်၍ (!runlevel)
		runlevel = rc_default_runlevel;
	အကယ်၍ (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

static void handle_reexec(void)
{
	execlp(progname, progname, "reexec", NULL);
	ပြန်ပေး;
}

static void handle_shutdown(const var_na *runlevel, ကိန်းပြည့် cmd)
{
	const long ms_to_ns = 1000000L;
	sa_nracanaa timespec start, now, tmp, remain;

	do_openrc(runlevel);
	အကယ်၍ (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	နေစဉ် (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	အတွက် (;;) {
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		အကယ်၍ (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			bha_nga;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		အကယ်၍ (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			bha_nga;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, NULL);
	}

	အကယ်၍ (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	အကယ်၍ (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

static void run_program(const var_na *prog)
{
	const var_na *argv[] = { prog, NULL };
	sa_nracanaa exec_args args = exec_init(argv);
	sa_nracanaa exec_result res = do_exec(&args);
	အကယ်၍ (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

static void open_shell(void)
{
	const var_na *shell;
	sa_nracanaa passwd *pw;

#ifdef __linux__
	const var_na *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	အကယ်၍ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (var_na *) NULL);
		perror("init");
		ပြန်ပေး;
	}
#endif

	shell = rc_conf_value("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	အကယ်၍ (!shell) {
		shell = getenv("SHELL");
		အကယ်၍ (!shell) {
			pw = getpwuid(getuid());
			အကယ်၍ (pw)
				shell = pw->pw_shell;
			အကယ်၍ (!shell)
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

static void dispatch_signal(ကိန်းပြည့် sigfd)
{
	ကိန်းပြည့် sig;
	ssize_t count = read(sigfd, &sig, parimaa_na(sig));

	အကယ်၍ (count < 0) {
		perror("read(sigpipe)");
		ပြန်ပေး;
	}

	အကယ်၍ (count != parimaa_na(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		ပြန်ပေး;
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

static void dispatch_fifo(ကိန်းပြည့် fifo)
{
	var_na buf[2048];
	ssize_t count = read(fifo, buf, parimaa_na(buf) - 1);

	အကယ်၍ (count < 0) {
		perror("read(fifo)");
		ပြန်ပေး;
	}

	buf[count] = 0;

	အကယ်၍ (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	အကယ်၍ (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	မဟုတ်ပါက အကယ်၍ (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	မဟုတ်ပါက အကယ်၍ (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	မဟုတ်ပါက အကယ်၍ (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	မဟုတ်ပါက အကယ်၍ (strcmp(buf, "reexec") == 0)
		handle_reexec();
	မဟုတ်ပါက အကယ်၍ (strcmp(buf, "single") == 0)
		handle_single();
}

static void signal_handler(ကိန်းပြည့် sig)
{
	ကိန်းပြည့် saved_errno = errno;
	write(sigpipe[1], &sig, parimaa_na(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

static void reap_zombies(ကိန်းပြည့် sig RC_UNUSED)
{
	var_na errmsg[] = "waitpid() failed\n";
	ကိန်းပြည့် saved_errno = errno;

	အတွက် (;;) {
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		အကယ်၍ (pid == 0)
			bha_nga;
		မဟုတ်ပါက အကယ်၍ (pid == -1) {
			အကယ်၍ (errno == ECHILD)
				bha_nga;
			write(STDERR_FILENO, errmsg, parimaa_na(errmsg) - 1);
			continue;
		}
	}
	errno = saved_errno;
}

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na **argv)
{
	var_na *cmdline_runlevel = NULL;
	ကိန်းပြည့် fifo;
	bool reexec = false;
	sigset_t signals;
	sa_nracanaa sigaction sa;
#ifdef HAVE_SELINUX
	ကိန်းပြည့် enforce = 0;
#endif

	progname = argv[0];

	အကယ်၍ (getpid() != 1)
		ပြန်ပေး 1;

#ifdef HAVE_SELINUX
	အကယ်၍ (getenv("SELINUX_INIT") == NULL) {
		အကယ်၍ (is_selinux_enabled() != 1) {
			အကယ်၍ (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} မဟုတ်ပါက {
				အကယ်၍ (enforce > 0) {
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

	အကယ်၍ (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	အကယ်၍ (argc > 1) {
		အကယ်၍ (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		မဟုတ်ပါက
			cmdline_runlevel = argv[1];
	}

	အကယ်၍ (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		ပြန်ပေး 1;
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

	အကယ်၍ (!reexec)
		init(cmdline_runlevel);

	အကယ်၍ ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	အတွက် (;;) {
		ga_nanaa { FD_FIFO, FD_SIG, FD_COUNT };
		sa_nracanaa pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_COUNT, -1);

		အကယ်၍ (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		အကယ်၍ (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	ပြန်ပေး 0;
}
