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

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "no-stop", 0, ಖಾಲಿ, 'n' },
	{ "override",    1, ಖಾಲಿ, 'o' },
	{ "service",     1, ಖಾಲಿ, 's' },
	{ "sys",         0, ಖಾಲಿ, 'S' },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#ವ್ಯಾಖ್ಯಾನಿಸು INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#ವ್ಯಾಖ್ಯಾನಿಸು INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#ವ್ಯಾಖ್ಯಾನಿಸು DEVBOOT			"/dev/.rcboot"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರ RC_STRINGLIST *main_hotplugged_services;
ಸ್ಥಿರ RC_STRINGLIST *main_stop_services;
ಸ್ಥಿರ RC_STRINGLIST *main_ಆರಂಭ_services;
ಸ್ಥಿರ RC_STRINGLIST *main_types_nw;
ಸ್ಥಿರ RC_STRINGLIST *main_types_nwua;
ಸ್ಥಿರ RC_DEPTREE *main_deptree;
ಸ್ಥಿರ ಅಕ್ಷರ *runlevel;

ರಚನೆ termios *termios_orig = ಖಾಲಿ;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ಸ್ಥಿರ ಶೂನ್ಯ
clean_failed(ಶೂನ್ಯ)
{
	DIR *dp;
	ರಚನೆ dirent *d;

	/* Clean the failed services state dir now */
	ಆದರೆ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		ಹಿಂತಿರುಗಿ;

	ಆಗ ((d = readdir(dp))) {
		ಆದರೆ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ಮುಂದುವರಿ;

		ಆದರೆ (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ಸ್ಥಿರ ಶೂನ್ಯ
cleanup(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	ಆದರೆ (!rc_ಒಳ_logger && !rc_ಒಳ_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		ಆದರೆ (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			ಬಿಡುಗಡೆ(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_ಸುರಕ್ಷಿತ(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ಬಿಡುಗಡೆ(p);
	}
	LIST_FOREACH_ಸುರಕ್ಷಿತ(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ಬಿಡುಗಡೆ(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_ಆರಂಭ_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	ಬಿಡುಗಡೆ(runlevel);
}

ಸ್ಥಿರ ಅಕ್ಷರ
read_ಕೀಲಿ(bool ಖಂಡ)
{
	ರಚನೆ termios termios;
	ಅಕ್ಷರ c = 0;
	ಪೂರ್ಣಾಂಕ fd = STDIN_FILENO;

	ಆದರೆ (!isatty(fd))
		ಹಿಂತಿರುಗಿ false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	ಆದರೆ (!termios_orig) {
		termios_orig = xmalloc(ಗಾತ್ರ(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	ಆದರೆ (ಖಂಡ)
		termios.c_cc[VMIN] = 1;
	ಇಲ್ಲದಿದ್ದರೆ {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	ಆದರೆ (ಪಠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	ಹಿಂತಿರುಗಿ c;
}

ಸ್ಥಿರ bool
want_interactive(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ c;
	ಸ್ಥಿರ bool gotinteractive;
	ಸ್ಥಿರ bool interactive;

	ಆದರೆ (rc_yesno(getenv("EINFO_QUIET")))
		ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	ಆದರೆ (!interactive)
		ಹಿಂತಿರುಗಿ false;
	c = read_ಕೀಲಿ(false);
	ಹಿಂತಿರುಗಿ (c == 'I' || c == 'i') ? true : false;
}

ಸ್ಥಿರ ಶೂನ್ಯ
mark_interactive(ಶೂನ್ಯ)
{
	FILE *fp;

	ಆದರೆ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಸತತ_ಕಾರ್ಯಕ್ರಮ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prog)
{
	ರಚನೆ sigaction sa;
	sigset_t ತುಂಬಿದ;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&ತುಂಬಿದ);
	sigprocmask(SIG_SETMASK, &ತುಂಬಿದ, &old);
	pid = fork();

	ಆದರೆ (pid == -1) {
		rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_ಆರಂಭ_ಹೊರಹೋಗು, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	ಆದರೆ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ಖಾಲಿ);
		sigaction(SIGHUP, &sa, ಖಾಲಿ);
		sigaction(SIGINT, &sa, ಖಾಲಿ);
		sigaction(SIGQUIT, &sa, ಖಾಲಿ);
		sigaction(SIGTERM, &sa, ಖಾಲಿ);
		sigaction(SIGUSR1, &sa, ಖಾಲಿ);
		sigaction(SIGWINCH, &sa, ಖಾಲಿ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);

		ಆದರೆ (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (ಅಕ್ಷರ *)ಖಾಲಿ);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
	ಆದರೆ (rc_waitpid(pid) == -1) {
		rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_ಆರಂಭ_ಹೊರಹೋಗು, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
open_shell(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shell;
	ರಚನೆ passwd *pw;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ಆದರೆ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ಅಕ್ಷರ *) ಖಾಲಿ);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#ಆದರೆ_ಅಂತ್ಯ

	shell = rc_conf_ಮೌಲ್ಯ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ಆದರೆ (shell == ಖಾಲಿ) {
		shell = getenv("SHELL");
		ಆದರೆ (shell == ಖಾಲಿ) {
			pw = getpwuid(getuid());
			ಆದರೆ (pw)
				shell = pw->pw_shell;
			ಆದರೆ (shell == ಖಾಲಿ)
				shell = "/bin/sh";
		}
	}
	ಸತತ_ಕಾರ್ಯಕ್ರಮ(shell);
}

ಸ್ಥಿರ bool
ಗಣ_krunlevel(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *level)
{
	FILE *fp;

	ಆದರೆ (rc_is_user())
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		ಆದರೆ (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ಹಿಂತಿರುಗಿ false;
	}

	ಆದರೆ (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ಹಿಂತಿರುಗಿ false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	ಹಿಂತಿರುಗಿ true;
}

ಸ್ಥಿರ ಅಕ್ಷರ *get_krunlevel(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	FILE *fp;
	size_t i = 0;

	ಆದರೆ (rc_is_user())
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (access(RC_KRUNLEVEL, F_OK) != 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಆದರೆ (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಆದರೆ (xgetline(&ಬಫರ್, &i, fp) == -1) {
		ಬಿಡುಗಡೆ(ಬಫರ್);
		ಬಫರ್ = ಖಾಲಿ;
	}

	fclose(fp);
	ಹಿಂತಿರುಗಿ ಬಫರ್;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಕೂಡಿಸು_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(ಗಾತ್ರ(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_ಸೇರಿಸು_ತಲೆ(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
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
		ಆದರೆ (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_ಸೇರಿಸು_ತಲೆ(&free_these_pids, p, entries);
			ನಿಲ್ಲಿಸು;
		}
	}
	/* only call free if we're not inside a signal handler */
	ಆದರೆ (!inside_signal) {
		LIST_FOREACH_ಸುರಕ್ಷಿತ(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			ಬಿಡುಗಡೆ(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
}

ಸ್ಥಿರ ಶೂನ್ಯ
wait_for_services(ಶೂನ್ಯ)
{
	ಗಾಗಿ (;;) {
		ಆಗ (waitpid(0, 0, 0) != -1)
			;
		ಆದರೆ (errno != EINTR)
			ನಿಲ್ಲಿಸು;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_signal(ಪೂರ್ಣಾಂಕ sig)
{
	ಪೂರ್ಣಾಂಕ serrno = errno;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *signame = ಖಾಲಿ;
	pid_t pid;
	RC_PID *pi;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 0;
	ರಚನೆ winsize ws;
	sigset_t sset;

	ಆಯ್ಕೆ (sig) {
	ಸ್ಥಿತಿ SIGCHLD:
		ಮಾಡು {
			pid = waitpid(-1, &ಸ್ಥಿತಿ_2, WNOHANG);
			ಆದರೆ (pid < 0) {
				ಆದರೆ (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				ಹಿಂತಿರುಗಿ;
			}
		} ಆಗ (!WIFEXITED(ಸ್ಥಿತಿ_2) && !WIFSIGNALED(ಸ್ಥಿತಿ_2));

		/* Remove that pid from our list */
		ಆದರೆ (pid > 0)
			remove_pid(pid, true);
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ SIGWINCH:
		ಆದರೆ (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ SIGINT:
		ಆದರೆ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ಸ್ಥಿತಿ SIGTERM:
		ಆದರೆ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ಸ್ಥಿತಿ SIGQUIT:
		ಆದರೆ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	ಸ್ಥಿತಿ SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, ಖಾಲಿ);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_ಸತತ(RC_HOOK_ABORT, ಖಾಲಿ);

		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
		/* NOTREACHED */

	ಪೂರ್ವನಿಯೋಜಿತ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ಶೂನ್ಯ
do_sysinit(ಶೂನ್ಯ)
{
	ರಚನೆ utsname uts;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	ಆದರೆ (access(INITEARLYSH, F_OK) == 0)
		ಸತತ_ಕಾರ್ಯಕ್ರಮ(INITEARLYSH);

	uname(&uts);
	ಆದರೆ (!rc_yesno(getenv("EINFO_QUIET"))) {
		ಬರೆ("\n   %sOpenRC %s" ಆವೃತ್ತಿ "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_ಸಾಮಾನ್ಯ), ecolor(ECOLOR_BRACKET));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BRANDING
		ಬರೆ(BRANDING " (%s)", uts.machine);
#ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಆದರೆ ((sys = rc_sys()))
		ಬರೆ(" [%s]", sys);

	ಬರೆ("%s\n\n", ecolor(ECOLOR_ಸಾಮಾನ್ಯ));

	ಆದರೆ (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		ಬರೆ("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_ಸಾಮಾನ್ಯ));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	ಸತತ_ಕಾರ್ಯಕ್ರಮ(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	ಆದರೆ ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	ಆದರೆ ((main_deptree = rc_deptree_ತುಂಬು_2(false, ಖಾಲಿ)) == ಖಾಲಿ)
		eerrorx("failed to load deptree");
}

ಸ್ಥಿರ bool
runlevel_config(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *level)
{
	ಅಕ್ಷರ *init = rc_service_resolve(service);
	ಅಕ್ಷರ *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	ಬಿಡುಗಡೆ(conf);
	ಬಿಡುಗಡೆ(init);
	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಶೂನ್ಯ
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *ಆರಂಭ_services,
				 ಸ್ಥಿರಾಂಕ RC_STRINGLIST *stop_services, ಸ್ಥಿರಾಂಕ RC_DEPTREE *deptree,
				 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *newlevel, bool parallel, bool going_ಕೆಳಕ್ಕೆ)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = ಖಾಲಿ;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	ಆದರೆ (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_ಕೂಡಿಸು(types_nw, "needsme");
		rc_stringlist_ಕೂಡಿಸು(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_ವಿಭಜಿಸು(rc_conf_ಮೌಲ್ಯ("rc_nostop"), " ");
	TAILQ_FOREACH_ಹಿಮ್ಮುಖ(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->ಮೌಲ್ಯ);
		ಆದರೆ (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			ಮುಂದುವರಿ;

		/* Sometimes we don't ever want to stop a service. */
		ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(nostop, service->ಮೌಲ್ಯ)) {
			rc_service_mark(service->ಮೌಲ್ಯ, RC_SERVICE_FAILED);
			ಮುಂದುವರಿ;
		}
		kwords = rc_deptree_depend(deptree, service->ಮೌಲ್ಯ, "keyword");
		ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(kwords, "-stop") ||
		    rc_stringlist_ಕಂಡುಹಿಡಿ(kwords, "nostop") ||
		    (going_ಕೆಳಕ್ಕೆ &&
			(rc_stringlist_ಕಂಡುಹಿಡಿ(kwords, "-shutdown") ||
			    rc_stringlist_ಕಂಡುಹಿಡಿ(kwords, "noshutdown"))))
			nstop = true;
		ಇಲ್ಲದಿದ್ದರೆ
			nstop = false;
		rc_stringlist_free(kwords);
		ಆದರೆ (nstop) {
			rc_service_mark(service->ಮೌಲ್ಯ, RC_SERVICE_FAILED);
			ಮುಂದುವರಿ;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		ಆದರೆ (crashed &&
		    rc_service_daemons_crashed(service->ಮೌಲ್ಯ))
			ಹೋಗು stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_ಕಂಡುಹಿಡಿ(ಆರಂಭ_services, service->ಮೌಲ್ಯ);
		ಆದರೆ (svc1) {
			ಆದರೆ (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				ಆದರೆ (!runlevel_config(service->ಮೌಲ್ಯ,runlevel) &&
				    !runlevel_config(service->ಮೌಲ್ಯ,newlevel))
					ಮುಂದುವರಿ;
			}
			ಇಲ್ಲದಿದ್ದರೆ
				ಮುಂದುವರಿ;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		ಆದರೆ (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_ಕೂಡಿಸು(tmplist, service->ಮೌಲ್ಯ);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = ಖಾಲಿ;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_ಕಂಡುಹಿಡಿ(ಆರಂಭ_services,
				    svc1->ಮೌಲ್ಯ);
				ಆದರೆ (svc2)
					ನಿಲ್ಲಿಸು;
			}
			rc_stringlist_free(deporder);

			ಆದರೆ (svc2)
				ಮುಂದುವರಿ;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->ಮೌಲ್ಯ);
		ಆದರೆ (pid > 0) {
			ಕೂಡಿಸು_pid(pid);
			ಆದರೆ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	ಆದರೆ (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ಸ್ಥಿರ ಶೂನ್ಯ
do_ಆರಂಭ_services(ಸ್ಥಿರಾಂಕ RC_STRINGLIST *ಆರಂಭ_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	ಆದರೆ (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	ಆದರೆ (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, ಆರಂಭ_services, entries) {
		state = rc_service_state(service->ಮೌಲ್ಯ);
		ಆದರೆ (state & RC_SERVICE_FAILED)
			ಮುಂದುವರಿ;
		ಆದರೆ (!(state & RC_SERVICE_STOPPED)) {
			ಆದರೆ (crashed &&
			    rc_service_daemons_crashed(service->ಮೌಲ್ಯ))
				rc_service_mark(service->ಮೌಲ್ಯ,
				    RC_SERVICE_STOPPED);
			ಇಲ್ಲದಿದ್ದರೆ
			    ಮುಂದುವರಿ;
		}
		ಆದರೆ (!interactive)
			interactive = want_interactive();

		ಆದರೆ (interactive) {
	interactive_retry:
			ಬರೆ("\n");
			einfo("About to start the service %s",
			    service->ಮೌಲ್ಯ);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			ಆಯ್ಕೆ (read_ಕೀಲಿ(true)) {
			ಸ್ಥಿತಿ '1': ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '2': ಮುಂದುವರಿ;
			ಸ್ಥಿತಿ '3': interactive = false; ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ '4': open_shell(); ಹೋಗು interactive_retry;
			ಪೂರ್ವನಿಯೋಜಿತ: ಹೋಗು interactive_option;
			}
		}

		pid = service_ಆರಂಭ(service->ಮೌಲ್ಯ);
		ಆದರೆ (pid == -1)
			ನಿಲ್ಲಿಸು;
		/* Remember the pid if we're running in parallel */
		ಆದರೆ (pid > 0) {
			ಕೂಡಿಸು_pid(pid);
			ಆದರೆ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	ಆದರೆ (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	ಇಲ್ಲದಿದ್ದರೆ
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ RC_DEBUG
RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ
handle_bad_signal(ಪೂರ್ಣಾಂಕ sig)
{
	ಅಕ್ಷರ pid[10];
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	pid_t crashed_pid = getpid();

	ಆಯ್ಕೆ (fork()) {
	ಸ್ಥಿತಿ -1:
		_exit(sig);
		/* NOTREACHED */
	ಸ್ಥಿತಿ 0:
		sprintf(pid, "%i", crashed_pid);
		ಬರೆ("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", ಖಾಲಿ));
		/* NOTREACHED */
	ಪೂರ್ವನಿಯೋಜಿತ:
		wait(&ಸ್ಥಿತಿ_2);
	}
	_exit(1);
	/* NOTREACHED */
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bootlevel = ಖಾಲಿ;
	ಅಕ್ಷರ *newlevel = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *systype = ಖಾಲಿ;
	RC_STRINGLIST *deporder = ಖಾಲಿ;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_ಕೆಳಕ್ಕೆ = false;
	ಪೂರ್ಣಾಂಕ depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	ಅಕ್ಷರ *krunlevel = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *workingdir = "/";
	ಅಕ್ಷರ *pidstr = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ opt;
	bool parallel;
	ಪೂರ್ಣಾಂಕ regen = 0;
	bool nostop = false;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಅಕ್ಷರ *proc;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ RC_DEBUG
	signal_ಸಿದ್ಧತೆ(SIGBUS, handle_bad_signal);
	signal_ಸಿದ್ಧತೆ(SIGILL, handle_bad_signal);
	signal_ಸಿದ್ಧತೆ(SIGSEGV, handle_bad_signal);
#ಆದರೆ_ಅಂತ್ಯ

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	ಆದರೆ (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	ಆದರೆ (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'n':
			nostop = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'o':
			ಆದರೆ (*optarg == '\0')
				optarg = ಖಾಲಿ;
			ಆದರೆ (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			}
			ಆದರೆ (rc_is_user())
				eerrorx("user services do not currently support --override");
			ಆದರೆ (!ಗಣ_krunlevel(optarg))
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
			/* NOTREACHED */
		ಸ್ಥಿತಿ 's':
			newlevel = rc_service_resolve(optarg);
			ಆದರೆ (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'S':
			systype = rc_sys();
			ಆದರೆ (systype)
				ಬರೆ("%s\n", systype);
			ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ಆದರೆ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ಆದರೆ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	ಆದರೆ (newlevel) {
		ಆದರೆ (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		ಆದರೆ (strcmp(newlevel, "reboot") == 0) {
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
	ಬಿಡುಗಡೆ(pidstr);

	/* Create a list of all services which should be started for the new or
	* current runlevel including those in boot, sysinit and hotplugged
	* runlevels.  Clearly, some of these will already be started so we
	* won't actually be starting them all.
	*/
	bootlevel = getenv("RC_BOOTLEVEL");
	runlevel = default_runlevel();

	rc_logger_open(newlevel ? newlevel : runlevel);

	/* Setup a signal handler */
	signal_ಸಿದ್ಧತೆ(SIGINT, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGQUIT, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGTERM, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGUSR1, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	ಆದರೆ (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		ಬಿಡುಗಡೆ(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_ತುಂಬು();

	/* Now we start handling our children */
	signal_ಸಿದ್ಧತೆ(SIGCHLD, handle_signal);

	ಆದರೆ (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_ಕೆಳಕ್ಕೆ = true;
		ಆದರೆ (access(RC_KRUNLEVEL, F_OK) != 0)
			ಗಣ_krunlevel(runlevel);
		rc_runlevel_ಗಣ(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* We should not use krunlevel in sysinit or boot runlevels */
		ಆದರೆ (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			ಆದರೆ (krunlevel) {
				newlevel = krunlevel;
				ಗಣ_krunlevel(ಖಾಲಿ);
			}
		}

		ಆದರೆ (newlevel) {
			ಆದರೆ (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
			ಆದರೆ (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				ಆದರೆ (p == ಖಾಲಿ)
					p = rc_proc_getent("softlevel");
				ಆದರೆ (p != ಖಾಲಿ) {
					ಗಣ_krunlevel(p);
					ಬಿಡುಗಡೆ(p);
				}
			}
#ಆದರೆ_ಅಂತ್ಯ
		}
	}

	ಆದರೆ (going_ಕೆಳಕ್ಕೆ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#ಆದರೆ_ಅಂತ್ಯ

		rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_STOP_ಒಳ, newlevel);
	} ಇಲ್ಲದಿದ್ದರೆ {
		rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_STOP_ಒಳ, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	ಆದರೆ (newlevel && strcmp(runlevel, newlevel) != 0 && !going_ಕೆಳಕ್ಕೆ) {
		ಆದರೆ (!rc_runlevel_exists(newlevel)) {
			rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_STOP_ಹೊರಹೋಗು, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	ಆದರೆ ((main_deptree = rc_deptree_ತುಂಬು_2(0, &regen)) == ಖಾಲಿ) {
		rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_STOP_ಹೊರಹೋಗು, runlevel);
		eerrorx("failed to load deptree");
	}

	ಆದರೆ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	ಆದರೆ (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_STOP_ಹೊರಹೋಗು, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), strerror(errno));
	}

	/* Create a list of all services which we could stop (assuming
	* they won't be active in the new or current runlevel) including
	* all those services which have been started, are inactive or
	* are currently starting.  Clearly, some of these will be listed
	* in the new or current runlevel so we won't actually be stopping
	* them all.
	*/
	main_stop_services = rc_services_ಒಳ_state(RC_SERVICE_STARTED);
	tmplist = rc_services_ಒಳ_state(RC_SERVICE_INACTIVE);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ಬಿಡುಗಡೆ(tmplist);
	tmplist = rc_services_ಒಳ_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ಬಿಡುಗಡೆ(tmplist);
	ಆದರೆ (main_stop_services)
		rc_stringlist_ವಿಂಗಡಿಸು(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(main_types_nwua, "ineed");
	rc_stringlist_ಕೂಡಿಸು(main_types_nwua, "iwant");
	rc_stringlist_ಕೂಡಿಸು(main_types_nwua, "iuse");
	rc_stringlist_ಕೂಡಿಸು(main_types_nwua, "iafter");

	ಆದರೆ (main_stop_services) {
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
	main_hotplugged_services = rc_services_ಒಳ_state(RC_SERVICE_HOTPLUGGED);
	main_ಆರಂಭ_services = rc_services_ಒಳ_runlevel_stacked(newlevel ?
	    newlevel : runlevel);
	ಆದರೆ (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_ಒಳ_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_ಆರಂಭ_services, tmplist, entries);
		ಬಿಡುಗಡೆ(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		ಆದರೆ (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			ಆದರೆ (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_ಒಳ_runlevel(bootlevel);
				TAILQ_CONCAT(main_ಆರಂಭ_services, tmplist, entries);
				ಬಿಡುಗಡೆ(tmplist);
			}
			ಆದರೆ (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_ಆರಂಭ_services,
					service->ಮೌಲ್ಯ);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	ಆದರೆ (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_ಆರಂಭ_services, main_stop_services, main_deptree, newlevel, parallel, going_ಕೆಳಕ್ಕೆ);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_STOP_ಹೊರಹೋಗು,
	    going_ಕೆಳಕ್ಕೆ ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	ಆದರೆ (newlevel) {
		rc_runlevel_ಗಣ(newlevel);
		ಬಿಡುಗಡೆ(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	ಆದರೆ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#ಆದರೆ_ಅಂತ್ಯ

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_ಆರಂಭ_ಒಳ, runlevel);

	/* Re-add our hotplugged services if they stopped */
	ಆದರೆ (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->ಮೌಲ್ಯ, RC_SERVICE_HOTPLUGGED);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	ಆದರೆ (proc) {
		ಆಗ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		ಬಿಡುಗಡೆ(proc);
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* If we have a list of services to start then... */
	ಆದರೆ (main_ಆರಂಭ_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_ಹಿಮ್ಮುಖ(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *ಸತತ_services = rc_services_ಒಳ_runlevel(rlevel->ಮೌಲ್ಯ);

			/* Start those services. */
			rc_stringlist_ವಿಂಗಡಿಸು(&ಸತತ_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, ಸತತ_services, rlevel->ಮೌಲ್ಯ, depoptions | RC_DEP_ಆರಂಭ);
			rc_stringlist_free(ಸತತ_services);
			ಸತತ_services = deporder;
			do_ಆರಂಭ_services(ಸತತ_services, parallel);

			/* Wait for our services to finish */
			wait_for_services();

			/* Free the list of services, we're done with it. */
			rc_stringlist_free(ಸತತ_services);
		}
		rc_stringlist_free(runlevel_chain);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	ಆದರೆ (proc) {
		ಆಗ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		ಬಿಡುಗಡೆ(proc);
	}

#ಆದರೆ_ಅಂತ್ಯ

	rc_plugin_ಸತತ(RC_HOOK_RUNLEVEL_ಆರಂಭ_ಹೊರಹೋಗು, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	ಆದರೆ (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
