/*
 * rc.c
 * rc - manager for init scripts which control the startup, shutdown
 * and the running of daemons.
 *
 * Also a multicall binary for various commands that can be used in shell
 * scripts to query service state, mark service state and provide the
 * einfo family of informational functions.
 */

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
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
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <libgen.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <termios.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "rc-logger.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "plugin.h"
#ಅನ್ತರ್ಭಾವ "version.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "no-stop", 0, ರಿಕ್ತ, 'n' },
	{ "override",    1, ರಿಕ್ತ, 'o' },
	{ "service",     1, ರಿಕ್ತ, 's' },
	{ "sys",         0, ರಿಕ್ತ, 'S' },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಸ್ಥಿರ RC_STRINGLIST *main_hotplugged_services;
ಸ್ಥಿರ RC_STRINGLIST *main_stop_services;
ಸ್ಥಿರ RC_STRINGLIST *main_start_services;
ಸ್ಥಿರ RC_STRINGLIST *main_types_nw;
ಸ್ಥಿರ RC_STRINGLIST *main_types_nwua;
ಸ್ಥಿರ RC_DEPTREE *main_deptree;
ಸ್ಥಿರ ವರ್ಣ *runlevel;

ಸಂರಚನಾ termios *termios_orig = ರಿಕ್ತ;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ಸ್ಥಿರ ಶೂನ್ಯ
clean_failed(ಶೂನ್ಯ)
{
	DIR *dp;
	ಸಂರಚನಾ dirent *d;

	/* Clean the failed services state dir now */
	ಯದಿ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		ಪ್ರತ್ಯಾಗಮ;

	ಯಾವತ್ ((d = readdir(dp))) {
		ಯದಿ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ಅನುವರ್ತ;

		ಯದಿ (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ಸ್ಥಿರ ಶೂನ್ಯ
cleanup(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	ಯದಿ (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		ಯದಿ (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			ಮುಕ್ತ(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ಮುಕ್ತ(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ಮುಕ್ತ(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	ಮುಕ್ತ(runlevel);
}

ಸ್ಥಿರ ವರ್ಣ
read_ಕೀಲಿ(bool block)
{
	ಸಂರಚನಾ termios termios;
	ವರ್ಣ c = 0;
	ಪೂರ್ಣಾಙ್ಕ fd = STDIN_FILENO;

	ಯದಿ (!isatty(fd))
		ಪ್ರತ್ಯಾಗಮ false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	ಯದಿ (!termios_orig) {
		termios_orig = xmalloc(ಪರಿಮಾಣ(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	ಯದಿ (block)
		termios.c_cc[VMIN] = 1;
	ಅನ್ಯಥಾ {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	ಯದಿ (ಪಠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	ಪ್ರತ್ಯಾಗಮ c;
}

ಸ್ಥಿರ bool
want_interactive(ಶೂನ್ಯ)
{
	ವರ್ಣ c;
	ಸ್ಥಿರ bool gotinteractive;
	ಸ್ಥಿರ bool interactive;

	ಯದಿ (rc_yesno(getenv("EINFO_QUIET")))
		ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	ಯದಿ (!interactive)
		ಪ್ರತ್ಯಾಗಮ false;
	c = read_ಕೀಲಿ(false);
	ಪ್ರತ್ಯಾಗಮ (c == 'I' || c == 'i') ? true : false;
}

ಸ್ಥಿರ ಶೂನ್ಯ
mark_interactive(ಶೂನ್ಯ)
{
	FILE *fp;

	ಯದಿ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ಸ್ಥಿರ ಶೂನ್ಯ
run_program(ಧ್ರುವ ವರ್ಣ *prog)
{
	ಸಂರಚನಾ sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	ಯದಿ (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	ಯದಿ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ರಿಕ್ತ);
		sigaction(SIGHUP, &sa, ರಿಕ್ತ);
		sigaction(SIGINT, &sa, ರಿಕ್ತ);
		sigaction(SIGQUIT, &sa, ರಿಕ್ತ);
		sigaction(SIGTERM, &sa, ರಿಕ್ತ);
		sigaction(SIGUSR1, &sa, ರಿಕ್ತ);
		sigaction(SIGWINCH, &sa, ರಿಕ್ತ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);

		ಯದಿ (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (ವರ್ಣ *)ರಿಕ್ತ);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
	ಯದಿ (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
open_shell(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *shell;
	ಸಂರಚನಾ passwd *pw;

#ifdef __linux__
	ಧ್ರುವ ವರ್ಣ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ಯದಿ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ವರ್ಣ *) ರಿಕ್ತ);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_ಮೌಲ್ಯ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ಯದಿ (shell == ರಿಕ್ತ) {
		shell = getenv("SHELL");
		ಯದಿ (shell == ರಿಕ್ತ) {
			pw = getpwuid(getuid());
			ಯದಿ (pw)
				shell = pw->pw_shell;
			ಯದಿ (shell == ರಿಕ್ತ)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ಸ್ಥಿರ bool
set_krunlevel(ಧ್ರುವ ವರ್ಣ *level)
{
	FILE *fp;

	ಯದಿ (rc_is_user())
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		ಯದಿ (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ವರ್ಣ *get_krunlevel(ಶೂನ್ಯ)
{
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	FILE *fp;
	size_t i = 0;

	ಯದಿ (rc_is_user())
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (access(RC_KRUNLEVEL, F_OK) != 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ (xgetline(&buffer, &i, fp) == -1) {
		ಮುಕ್ತ(buffer);
		buffer = ರಿಕ್ತ;
	}

	fclose(fp);
	ಪ್ರತ್ಯಾಗಮ buffer;
}

ಸ್ಥಿರ ಶೂನ್ಯ
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(ಪರಿಮಾಣ(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_ಸೇರಿಸು_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
}

ಸ್ಥಿರ ಶೂನ್ಯ
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		ಯದಿ (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_ಸೇರಿಸು_HEAD(&free_these_pids, p, entries);
			ಭಙ್ಗ;
		}
	}
	/* only call free if we're not inside a signal handler */
	ಯದಿ (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			ಮುಕ್ತ(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
}

ಸ್ಥಿರ ಶೂನ್ಯ
wait_for_services(ಶೂನ್ಯ)
{
	ಪ್ರತಿ (;;) {
		ಯಾವತ್ (waitpid(0, 0, 0) != -1)
			;
		ಯದಿ (errno != EINTR)
			ಭಙ್ಗ;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_signal(ಪೂರ್ಣಾಙ್ಕ sig)
{
	ಪೂರ್ಣಾಙ್ಕ serrno = errno;
	ಧ್ರುವ ವರ್ಣ *signame = ರಿಕ್ತ;
	pid_t pid;
	RC_PID *pi;
	ಪೂರ್ಣಾಙ್ಕ status = 0;
	ಸಂರಚನಾ winsize ws;
	sigset_t sset;

	ವಿಕಲ್ಪ (sig) {
	ಪಕ್ಷ SIGCHLD:
		ಕುರು {
			pid = waitpid(-1, &status, WNOHANG);
			ಯದಿ (pid < 0) {
				ಯದಿ (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				ಪ್ರತ್ಯಾಗಮ;
			}
		} ಯಾವತ್ (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		ಯದಿ (pid > 0)
			remove_pid(pid, true);
		ಭಙ್ಗ;

	ಪಕ್ಷ SIGWINCH:
		ಯದಿ (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		ಭಙ್ಗ;

	ಪಕ್ಷ SIGINT:
		ಯದಿ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ಪಕ್ಷ SIGTERM:
		ಯದಿ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ಪಕ್ಷ SIGQUIT:
		ಯದಿ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	ಪಕ್ಷ SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, ರಿಕ್ತ);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, ರಿಕ್ತ);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	ಸ್ವಭಾವ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ಶೂನ್ಯ
do_sysinit(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ utsname uts;
	ಧ್ರುವ ವರ್ಣ *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	ಯದಿ (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	ಯದಿ (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#ಅನ್ಯಥಾ
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	ಯದಿ ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	ಯದಿ (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	ಯದಿ ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	ಯದಿ ((main_deptree = _rc_deptree_load(false, ರಿಕ್ತ)) == ರಿಕ್ತ)
		eerrorx("failed to load deptree");
}

ಸ್ಥಿರ bool
runlevel_config(ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *level)
{
	ವರ್ಣ *init = rc_service_resolve(service);
	ವರ್ಣ *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	ಮುಕ್ತ(conf);
	ಮುಕ್ತ(init);
	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಶೂನ್ಯ
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 ಧ್ರುವ RC_STRINGLIST *stop_services, ಧ್ರುವ RC_DEPTREE *deptree,
				 ಧ್ರುವ ವರ್ಣ *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = ರಿಕ್ತ;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	ಯದಿ (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_ಮೌಲ್ಯ("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->ಮೌಲ್ಯ);
		ಯದಿ (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			ಅನುವರ್ತ;

		/* Sometimes we don't ever want to stop a service. */
		ಯದಿ (rc_stringlist_find(nostop, service->ಮೌಲ್ಯ)) {
			rc_service_mark(service->ಮೌಲ್ಯ, RC_SERVICE_FAILED);
			ಅನುವರ್ತ;
		}
		kwords = rc_deptree_depend(deptree, service->ಮೌಲ್ಯ, "keyword");
		ಯದಿ (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		ಅನ್ಯಥಾ
			nstop = false;
		rc_stringlist_free(kwords);
		ಯದಿ (nstop) {
			rc_service_mark(service->ಮೌಲ್ಯ, RC_SERVICE_FAILED);
			ಅನುವರ್ತ;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		ಯದಿ (crashed &&
		    rc_service_daemons_crashed(service->ಮೌಲ್ಯ))
			ಗಚ್ಛ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->ಮೌಲ್ಯ);
		ಯದಿ (svc1) {
			ಯದಿ (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				ಯದಿ (!runlevel_config(service->ಮೌಲ್ಯ,runlevel) &&
				    !runlevel_config(service->ಮೌಲ್ಯ,newlevel))
					ಅನುವರ್ತ;
			}
			ಅನ್ಯಥಾ
				ಅನುವರ್ತ;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		ಯದಿ (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->ಮೌಲ್ಯ);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = ರಿಕ್ತ;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->ಮೌಲ್ಯ);
				ಯದಿ (svc2)
					ಭಙ್ಗ;
			}
			rc_stringlist_free(deporder);

			ಯದಿ (svc2)
				ಅನುವರ್ತ;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->ಮೌಲ್ಯ);
		ಯದಿ (pid > 0) {
			add_pid(pid);
			ಯದಿ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	ಯದಿ (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ಸ್ಥಿರ ಶೂನ್ಯ
do_start_services(ಧ್ರುವ RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	ಯದಿ (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	ಯದಿ (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->ಮೌಲ್ಯ);
		ಯದಿ (state & RC_SERVICE_FAILED)
			ಅನುವರ್ತ;
		ಯದಿ (!(state & RC_SERVICE_STOPPED)) {
			ಯದಿ (crashed &&
			    rc_service_daemons_crashed(service->ಮೌಲ್ಯ))
				rc_service_mark(service->ಮೌಲ್ಯ,
				    RC_SERVICE_STOPPED);
			ಅನ್ಯಥಾ
			    ಅನುವರ್ತ;
		}
		ಯದಿ (!interactive)
			interactive = want_interactive();

		ಯದಿ (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->ಮೌಲ್ಯ);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			ವಿಕಲ್ಪ (read_ಕೀಲಿ(true)) {
			ಪಕ್ಷ '1': ಭಙ್ಗ;
			ಪಕ್ಷ '2': ಅನುವರ್ತ;
			ಪಕ್ಷ '3': interactive = false; ಭಙ್ಗ;
			ಪಕ್ಷ '4': open_shell(); ಗಚ್ಛ interactive_retry;
			ಸ್ವಭಾವ: ಗಚ್ಛ interactive_option;
			}
		}

		pid = service_start(service->ಮೌಲ್ಯ);
		ಯದಿ (pid == -1)
			ಭಙ್ಗ;
		/* Remember the pid if we're running in parallel */
		ಯದಿ (pid > 0) {
			add_pid(pid);
			ಯದಿ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	ಯದಿ (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	ಅನ್ಯಥಾ
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ
handle_bad_signal(ಪೂರ್ಣಾಙ್ಕ sig)
{
	ವರ್ಣ pid[10];
	ಪೂರ್ಣಾಙ್ಕ status;
	pid_t crashed_pid = getpid();

	ವಿಕಲ್ಪ (fork()) {
	ಪಕ್ಷ -1:
		_exit(sig);
		/* NOTREACHED */
	ಪಕ್ಷ 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", ರಿಕ್ತ));
		/* NOTREACHED */
	ಸ್ವಭಾವ:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಧ್ರುವ ವರ್ಣ *bootlevel = ರಿಕ್ತ;
	ವರ್ಣ *newlevel = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *systype = ರಿಕ್ತ;
	RC_STRINGLIST *deporder = ರಿಕ್ತ;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	ಪೂರ್ಣಾಙ್ಕ depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	ವರ್ಣ *krunlevel = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *workingdir = "/";
	ವರ್ಣ *pidstr = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ opt;
	bool parallel;
	ಪೂರ್ಣಾಙ್ಕ regen = 0;
	bool nostop = false;
#ifdef __linux__
	ವರ್ಣ *proc;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	ಯದಿ (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	ಯದಿ (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'n':
			nostop = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'o':
			ಯದಿ (*optarg == '\0')
				optarg = ರಿಕ್ತ;
			ಯದಿ (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			ಯದಿ (rc_is_user())
				eerrorx("user services do not currently support --override");
			ಯದಿ (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		ಪಕ್ಷ 's':
			newlevel = rc_service_resolve(optarg);
			ಯದಿ (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		ಪಕ್ಷ 'S':
			systype = rc_sys();
			ಯದಿ (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ಯದಿ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ಯದಿ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	ಯದಿ (newlevel) {
		ಯದಿ (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		ಯದಿ (strcmp(newlevel, "reboot") == 0) {
			newlevel = UNCONST(RC_LEVEL_SHUTDOWN);
			setenv("RC_REBOOT", "YES", 1);
		}
	}

	atexit(cleanup);

	/* Enable logging */
	setenv("EINFO_LOG", "openrc", 1);

	/* Export our PID */
	xasprintf(&pidstr, "%d", getpid());
	setenv("RC_PID", pidstr, 1);
	ಮುಕ್ತ(pidstr);

	/* Create a list of all services which should be started for the new or
	* current runlevel including those in boot, sysinit and hotplugged
	* runlevels.  Clearly, some of these will already be started so we
	* won't actually be starting them all.
	*/
	bootlevel = getenv("RC_BOOTLEVEL");
	runlevel = default_runlevel();

	rc_logger_open(newlevel ? newlevel : runlevel);

	/* Setup a signal handler */
	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);
	signal_setup(SIGUSR1, handle_signal);
	signal_setup(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	ಯದಿ (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		ಮುಕ್ತ(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	ಯದಿ (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		ಯದಿ (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} ಅನ್ಯಥಾ {
		/* We should not use krunlevel in sysinit or boot runlevels */
		ಯದಿ (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			ಯದಿ (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(ರಿಕ್ತ);
			}
		}

		ಯದಿ (newlevel) {
			ಯದಿ (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			ಯದಿ (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				ಯದಿ (p == ರಿಕ್ತ)
					p = rc_proc_getent("softlevel");
				ಯದಿ (p != ರಿಕ್ತ) {
					set_krunlevel(p);
					ಮುಕ್ತ(p);
				}
			}
#endif
		}
	}

	ಯದಿ (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} ಅನ್ಯಥಾ {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	ಯದಿ (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		ಯದಿ (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	ಯದಿ ((main_deptree = _rc_deptree_load(0, &regen)) == ರಿಕ್ತ) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	ಯದಿ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	ಯದಿ (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), strerror(errno));
	}

	/* Create a list of all services which we could stop (assuming
	* they won't be active in the new or current runlevel) including
	* all those services which have been started, are inactive or
	* are currently starting.  Clearly, some of these will be listed
	* in the new or current runlevel so we won't actually be stopping
	* them all.
	*/
	main_stop_services = rc_services_in_state(RC_SERVICE_STARTED);
	tmplist = rc_services_in_state(RC_SERVICE_INACTIVE);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ಮುಕ್ತ(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ಮುಕ್ತ(tmplist);
	ಯದಿ (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	ಯದಿ (main_stop_services) {
		tmplist = rc_deptree_depends(main_deptree, main_types_nwua, main_stop_services,
		    runlevel, depoptions | RC_DEP_STOP);
		rc_stringlist_free(main_stop_services);
		main_stop_services = tmplist;
	}

	/* Create a list of all services which should be started for the new or
	 * current runlevel including those in boot, sysinit and hotplugged
	 * runlevels.  Clearly, some of these will already be started so we
	 * won't actually be starting them all.
	 */
	main_hotplugged_services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
	main_start_services = rc_services_in_runlevel_stacked(newlevel ?
	    newlevel : runlevel);
	ಯದಿ (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		ಮುಕ್ತ(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		ಯದಿ (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			ಯದಿ (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				ಮುಕ್ತ(tmplist);
			}
			ಯದಿ (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->ಮೌಲ್ಯ);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	ಯದಿ (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	ಯದಿ (newlevel) {
		rc_runlevel_set(newlevel);
		ಮುಕ್ತ(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	ಯದಿ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	ಯದಿ (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->ಮೌಲ್ಯ, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	ಯದಿ (proc) {
		ಯಾವತ್ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		ಮುಕ್ತ(proc);
	}
#endif

	/* If we have a list of services to start then... */
	ಯದಿ (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->ಮೌಲ್ಯ);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->ಮೌಲ್ಯ, depoptions | RC_DEP_START);
			rc_stringlist_free(run_services);
			run_services = deporder;
			do_start_services(run_services, parallel);

			/* Wait for our services to finish */
			wait_for_services();

			/* Free the list of services, we're done with it. */
			rc_stringlist_free(run_services);
		}
		rc_stringlist_free(runlevel_chain);
	}

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	ಯದಿ (proc) {
		ಯಾವತ್ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		ಮುಕ್ತ(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	ಯದಿ (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
