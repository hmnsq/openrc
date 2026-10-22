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
#якщо defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

static const char *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
static const char *rc_default_runlevel = "default";
static ціле sigpipe[2] = { -1, -1 };
static bool quiet = false;
static const char *progname;

static void do_openrc(const char *runlevel)
{
	const char *argv[] = { "openrc", runlevel, NULL };
	struct exec_result res;
	struct exec_args args = exec_init(argv);
	args.setsid = 1;

	якщо (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	якщо (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	поки (waitpid(res.pid, NULL, 0) != res.pid)
		якщо (errno == ECHILD)
			break;
}

static void init(const char *default_runlevel)
{
	const char *runlevel = NULL;
	do_openrc("sysinit");
	do_openrc("boot");
	якщо (default_runlevel)
		runlevel = default_runlevel;
	інакше
		runlevel = rc_conf_value("rc_default_runlevel");
	якщо (!runlevel)
		runlevel = rc_default_runlevel;
	якщо (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

static void handle_reexec(void)
{
	execlp(progname, progname, "reexec", NULL);
	повернути;
}

static void handle_shutdown(const char *runlevel, ціле cmd)
{
	const long ms_to_ns = 1000000L;
	struct timespec start, now, tmp, remain;

	do_openrc(runlevel);
	якщо (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	поки (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	для (;;) {
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		якщо (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			break;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		якщо (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			break;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, NULL);
	}

	якщо (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	якщо (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

static void run_program(const char *prog)
{
	const char *argv[] = { prog, NULL };
	struct exec_args args = exec_init(argv);
	struct exec_result res = do_exec(&args);
	якщо (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

static void open_shell(void)
{
	const char *shell;
	struct passwd *pw;

#ifdef __linux__
	const char *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	якщо (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (char *) NULL);
		perror("init");
		повернути;
	}
#endif

	shell = rc_conf_value("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	якщо (!shell) {
		shell = getenv("SHELL");
		якщо (!shell) {
			pw = getpwuid(getuid());
			якщо (pw)
				shell = pw->pw_shell;
			якщо (!shell)
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

static void dispatch_signal(ціле sigfd)
{
	ціле sig;
	ssize_t count = read(sigfd, &sig, sizeof(sig));

	якщо (count < 0) {
		perror("read(sigpipe)");
		повернути;
	}

	якщо (count != sizeof(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		повернути;
	}

	switch (sig) {
		case SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			break;
		case SIGTERM:
#ifdef SIGPWR
		case SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			break;
		default:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			break;
	}
}

static void dispatch_fifo(ціле fifo)
{
	char buf[2048];
	ssize_t count = read(fifo, buf, sizeof(buf) - 1);

	якщо (count < 0) {
		perror("read(fifo)");
		повернути;
	}

	buf[count] = 0;

	якщо (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	якщо (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	інакше якщо (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	інакше якщо (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	інакше якщо (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	інакше якщо (strcmp(buf, "reexec") == 0)
		handle_reexec();
	інакше якщо (strcmp(buf, "single") == 0)
		handle_single();
}

static void signal_handler(ціле sig)
{
	ціле saved_errno = errno;
	write(sigpipe[1], &sig, sizeof(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

static void reap_zombies(ціле sig RC_UNUSED)
{
	char errmsg[] = "waitpid() failed\n";
	ціле saved_errno = errno;

	для (;;) {
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		якщо (pid == 0)
			break;
		інакше якщо (pid == -1) {
			якщо (errno == ECHILD)
				break;
			write(STDERR_FILENO, errmsg, sizeof(errmsg) - 1);
			continue;
		}
	}
	errno = saved_errno;
}

ціле main(ціле argc, char **argv)
{
	char *cmdline_runlevel = NULL;
	ціле fifo;
	bool reexec = false;
	sigset_t signals;
	struct sigaction sa;
#ifdef HAVE_SELINUX
	ціле enforce = 0;
#endif

	progname = argv[0];

	якщо (getpid() != 1)
		повернути 1;

#ifdef HAVE_SELINUX
	якщо (getenv("SELINUX_INIT") == NULL) {
		якщо (is_selinux_enabled() != 1) {
			якщо (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} інакше {
				якщо (enforce > 0) {
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

	якщо (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	якщо (argc > 1) {
		якщо (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		інакше
			cmdline_runlevel = argv[1];
	}

	якщо (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		повернути 1;
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
	memset(&sa, 0, sizeof(sa));
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

	якщо (!reexec)
		init(cmdline_runlevel);

	якщо ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	для (;;) {
		enum { FD_FIFO, FD_SIG, FD_COUNT };
		struct pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_COUNT, -1);

		якщо (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		якщо (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	повернути 0;
}
