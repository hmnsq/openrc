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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/reboot.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <utmp.h>

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif

#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "wtmp.h"
#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "version.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#જો defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

સ્થિર અચલ અક્ષર *path_default = "/sbin:/usr/sbin:/bin:/usr/bin";
સ્થિર અચલ અક્ષર *rc_default_runlevel = "default";
સ્થિર પૂર્ણાંક sigpipe[2] = { -1, -1 };
સ્થિર bool quiet = false;
સ્થિર અચલ અક્ષર *progname;

સ્થિર શૂન્ય do_openrc(અચલ અક્ષર *runlevel)
{
	અચલ અક્ષર *argv[] = { "openrc", runlevel, ખાલી };
	સંરચના exec_result res;
	સંરચના exec_args args = exec_init(argv);
	args.setsid = 1;

	જો (!quiet)
		printf("Starting %s runlevel\n", runlevel);
	res = do_exec(&args);
	જો (res.pid < 0) {
		perror("do_exec");
		exit(1);
	}
	જ્યારે (waitpid(res.pid, ખાલી, 0) != res.pid)
		જો (errno == ECHILD)
			તોડો;
}

સ્થિર શૂન્ય init(અચલ અક્ષર *default_runlevel)
{
	અચલ અક્ષર *runlevel = ખાલી;
	do_openrc("sysinit");
	do_openrc("boot");
	જો (default_runlevel)
		runlevel = default_runlevel;
	નહીંતર
		runlevel = rc_conf_કિંમત("rc_default_runlevel");
	જો (!runlevel)
		runlevel = rc_default_runlevel;
	જો (!rc_runlevel_exists(runlevel)) {
		fprintf(stderr, "%s is an invalid runlevel\n", runlevel);
		runlevel = rc_default_runlevel;
	}
	do_openrc(runlevel);
	log_wtmp("reboot", "~~", 0, RUN_LVL, "~~");
}

સ્થિર શૂન્ય handle_reexec(શૂન્ય)
{
	execlp(progname, progname, "reexec", ખાલી);
	પાછા;
}

સ્થિર શૂન્ય handle_shutdown(અચલ અક્ષર *runlevel, પૂર્ણાંક cmd)
{
	અચલ long ms_to_ns = 1000000L;
	સંરચના timespec start, now, tmp, remain;

	do_openrc(runlevel);
	જો (!quiet)
		printf("Sending the final TERM signal\n");
	kill(-1, SIGTERM);
	clock_gettime(CLOCK_MONOTONIC, &start);

	/* ensure *at least* half a second of grace time before KILL */
	tmp.tv_sec = 0;
	tmp.tv_nsec = 500 * ms_to_ns;
	જ્યારે (clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, &remain) < 0 && errno == EINTR)
		tmp = remain;

	માટે (;;) {
		pid_t pid = waitpid(-1, ખાલી, WNOHANG);
		જો (pid == -1 && errno == ECHILD) {
			/* all children reaped, safe to exit */
			તોડો;
		}

		clock_gettime(CLOCK_MONOTONIC, &now);
		timespecsub(&now, &start, &tmp);
		જો (tmp.tv_sec >= 3) {
			/* it's been 3 seconds already, go for the KILL hammer */
			તોડો;
		}

		tmp.tv_sec = 0;
		tmp.tv_nsec = 100 * ms_to_ns;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &tmp, ખાલી);
	}

	જો (!quiet)
		printf("Sending the final KILL signal\n");
	kill(-1, SIGKILL);
	sync();
	reboot(cmd);
	/* reboot normally if kexec fails */
	જો (cmd == RB_KEXEC)
		reboot(RB_AUTOBOOT);
}

સ્થિર શૂન્ય run_program(અચલ અક્ષર *prog)
{
	અચલ અક્ષર *argv[] = { prog, ખાલી };
	સંરચના exec_args args = exec_init(argv);
	સંરચના exec_result res = do_exec(&args);
	જો (res.pid < 0 || rc_waitpid(res.pid) == -1)
		perror("init");
}

સ્થિર શૂન્ય open_shell(શૂન્ય)
{
	અચલ અક્ષર *shell;
	સંરચના passwd *pw;

#ifdef __linux__
	અચલ અક્ષર *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	જો (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (અક્ષર *) ખાલી);
		perror("init");
		પાછા;
	}
#endif

	shell = rc_conf_કિંમત("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	જો (!shell) {
		shell = getenv("SHELL");
		જો (!shell) {
			pw = getpwuid(getuid());
			જો (pw)
				shell = pw->pw_shell;
			જો (!shell)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

સ્થિર શૂન્ય handle_single(શૂન્ય)
{
	do_openrc("single");
	open_shell();
	init(ખાલી);
}

સ્થિર શૂન્ય dispatch_signal(પૂર્ણાંક sigfd)
{
	પૂર્ણાંક sig;
	ssize_t ગણતરી = પઠ(sigfd, &sig, કદ(sig));

	જો (ગણતરી < 0) {
		perror("read(sigpipe)");
		પાછા;
	}

	જો (ગણતરી != કદ(sig)) {
		fputs("bad read from sigpipe\n", stderr);
		પાછા;
	}

	પસંદગી (sig) {
		સ્થિતિ SIGINT:
			handle_shutdown("reboot", RB_AUTOBOOT);
			તોડો;
		સ્થિતિ SIGTERM:
#ifdef SIGPWR
		સ્થિતિ SIGPWR:
#endif
			handle_shutdown("shutdown", RB_HALT_SYSTEM);
			તોડો;
		મૂળભૂત:
			fprintf(stderr, "Unknown signal received, %d\n", sig);
			તોડો;
	}
}

સ્થિર શૂન્ય dispatch_fifo(પૂર્ણાંક fifo)
{
	અક્ષર buf[2048];
	ssize_t ગણતરી = પઠ(fifo, buf, કદ(buf) - 1);

	જો (ગણતરી < 0) {
		perror("read(fifo)");
		પાછા;
	}

	buf[ગણતરી] = 0;

	જો (!quiet)
		printf("PID1: Received \"%s\" from FIFO...\n", buf);

	જો (strcmp(buf, "halt") == 0)
		handle_shutdown("shutdown", RB_HALT_SYSTEM);
	નહીંતર જો (strcmp(buf, "kexec") == 0)
		handle_shutdown("reboot", RB_KEXEC);
	નહીંતર જો (strcmp(buf, "poweroff") == 0)
		handle_shutdown("shutdown", RB_POWER_OFF);
	નહીંતર જો (strcmp(buf, "reboot") == 0)
		handle_shutdown("reboot", RB_AUTOBOOT);
	નહીંતર જો (strcmp(buf, "reexec") == 0)
		handle_reexec();
	નહીંતર જો (strcmp(buf, "single") == 0)
		handle_single();
}

સ્થિર શૂન્ય signal_handler(પૂર્ણાંક sig)
{
	પૂર્ણાંક saved_errno = errno;
	લિખ(sigpipe[1], &sig, કદ(sig));
	/* ensure the errno doesn't get clobbered in the rare (impossible?)
	 * case that the write above fails */
	errno = saved_errno;
}

સ્થિર શૂન્ય reap_zombies(પૂર્ણાંક sig RC_UNUSED)
{
	અક્ષર errmsg[] = "waitpid() failed\n";
	પૂર્ણાંક saved_errno = errno;

	માટે (;;) {
		pid_t pid = waitpid(-1, ખાલી, WNOHANG);
		જો (pid == 0)
			તોડો;
		નહીંતર જો (pid == -1) {
			જો (errno == ECHILD)
				તોડો;
			લિખ(STDERR_FILENO, errmsg, કદ(errmsg) - 1);
			ચાલુ;
		}
	}
	errno = saved_errno;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	અક્ષર *cmdline_runlevel = ખાલી;
	પૂર્ણાંક fifo;
	bool reexec = false;
	sigset_t signals;
	સંરચના sigaction sa;
#ifdef HAVE_SELINUX
	પૂર્ણાંક enforce = 0;
#endif

	progname = argv[0];

	જો (getpid() != 1)
		પાછા 1;

#ifdef HAVE_SELINUX
	જો (getenv("SELINUX_INIT") == ખાલી) {
		જો (is_selinux_enabled() != 1) {
			જો (selinux_init_load_policy(&enforce) == 0) {
				setenv("SELINUX_INIT", "YES", 1);
				execv(argv[0], argv);
			} નહીંતર {
				જો (enforce > 0) {
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

	જો (!quiet)
		printf("OpenRC init version %s starting\n", VERSION);

	જો (argc > 1) {
		જો (strcmp(argv[1], "reexec") == 0)
			reexec = true;
		નહીંતર
			cmdline_runlevel = argv[1];
	}

	જો (pipe2(sigpipe, O_NONBLOCK | O_CLOEXEC) == -1) {
		perror("pipe2");
		પાછા 1;
	}

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGINT);
	sigdelset(&signals, SIGTERM);
#ifdef SIGPWR
	sigdelset(&signals, SIGPWR);
#endif
	sigprocmask(SIG_SETMASK, &signals, ખાલી);

	/* install signal  handler */
	memset(&sa, 0, કદ(sa));
	sa.sa_handler = reap_zombies;
	sigaction(SIGCHLD, &sa, ખાલી);
	sa.sa_handler = signal_handler;
	sigaction(SIGINT, &sa, ખાલી);
	sigaction(SIGTERM, &sa, ખાલી);
#ifdef SIGPWR
	sigaction(SIGPWR, &sa, ખાલી);
#endif
	reboot(RB_DISABLE_CAD);

	/* set default path */
	setenv("PATH", path_default, 1);

	જો (!reexec)
		init(cmdline_runlevel);

	જો ((fifo = open_rwfifo(RC_INIT_FIFO, true)) == -1)
		perror("open_rwfifo");

	માટે (;;) {
		ગણના { FD_FIFO, FD_SIG, FD_ગણતરી };
		સંરચના pollfd pfd[] = {
			[FD_FIFO] = { .fd = fifo, .events = POLLIN },
			[FD_SIG] = { .fd = sigpipe[0], .events = POLLIN },
		};

		poll(pfd, FD_ગણતરી, -1);

		જો (pfd[FD_SIG].revents & POLLIN) /* handle signals first */
			dispatch_signal(sigpipe[0]);

		જો (pfd[FD_FIFO].revents & POLLIN)
			dispatch_fifo(fifo);
	}
	પાછા 0;
}
