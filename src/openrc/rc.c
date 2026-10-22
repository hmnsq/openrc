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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libgen.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <termios.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc-logger.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "plugin.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "version.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "no-stop", 0, ਨਿਰਾ, 'n' },
	{ "override",    1, ਨਿਰਾ, 'o' },
	{ "service",     1, ਨਿਰਾ, 's' },
	{ "sys",         0, ਨਿਰਾ, 'S' },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEVBOOT			"/dev/.rcboot"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਸਥਿਰ RC_STRINGLIST *main_hotplugged_services;
ਸਥਿਰ RC_STRINGLIST *main_stop_services;
ਸਥਿਰ RC_STRINGLIST *main_ਸ਼ੁਰੂ_services;
ਸਥਿਰ RC_STRINGLIST *main_types_nw;
ਸਥਿਰ RC_STRINGLIST *main_types_nwua;
ਸਥਿਰ RC_DEPTREE *main_deptree;
ਸਥਿਰ ਅੱਖਰ *runlevel;

ਬਣਤਰ termios *termios_orig = ਨਿਰਾ;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ਸਥਿਰ ਖਾਲੀ
clean_failed(ਖਾਲੀ)
{
	DIR *dp;
	ਬਣਤਰ dirent *d;

	/* Clean the failed services state dir now */
	ਜੇ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		ਵਾਪਸ;

	ਜਦੋਂ ((d = readdir(dp))) {
		ਜੇ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ਜਾਰੀ;

		ਜੇ (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ਸਥਿਰ ਖਾਲੀ
cleanup(ਖਾਲੀ)
{
	ਪੂਰਨ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	ਜੇ (!rc_ਵਿੱਚ_logger && !rc_ਵਿੱਚ_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		ਜੇ (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			ਮੁਕਤ(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_ਸੁਰੱਖਿਅਤ(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ਮੁਕਤ(p);
	}
	LIST_FOREACH_ਸੁਰੱਖਿਅਤ(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ਮੁਕਤ(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_ਸ਼ੁਰੂ_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	ਮੁਕਤ(runlevel);
}

ਸਥਿਰ ਅੱਖਰ
read_ਕੁੰਜੀ(bool ਖੰਡ)
{
	ਬਣਤਰ termios termios;
	ਅੱਖਰ c = 0;
	ਪੂਰਨ fd = STDIN_FILENO;

	ਜੇ (!isatty(fd))
		ਵਾਪਸ false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	ਜੇ (!termios_orig) {
		termios_orig = xmalloc(ਆਕਾਰ(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	ਜੇ (ਖੰਡ)
		termios.c_cc[VMIN] = 1;
	ਨਹੀਂਤਾਂ {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	ਜੇ (ਪਠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	ਵਾਪਸ c;
}

ਸਥਿਰ bool
want_interactive(ਖਾਲੀ)
{
	ਅੱਖਰ c;
	ਸਥਿਰ bool gotinteractive;
	ਸਥਿਰ bool interactive;

	ਜੇ (rc_yesno(getenv("EINFO_QUIET")))
		ਵਾਪਸ false;
	ਜੇ (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	ਜੇ (!interactive)
		ਵਾਪਸ false;
	c = read_ਕੁੰਜੀ(false);
	ਵਾਪਸ (c == 'I' || c == 'i') ? true : false;
}

ਸਥਿਰ ਖਾਲੀ
mark_interactive(ਖਾਲੀ)
{
	FILE *fp;

	ਜੇ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ਸਥਿਰ ਖਾਲੀ
ਲੜੀਵਾਰ_ਪ੍ਰੋਗਰਾਮ(ਅਚਲ ਅੱਖਰ *prog)
{
	ਬਣਤਰ sigaction sa;
	sigset_t ਭਰਿਆ;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&ਭਰਿਆ);
	sigprocmask(SIG_SETMASK, &ਭਰਿਆ, &old);
	pid = fork();

	ਜੇ (pid == -1) {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	ਜੇ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ਨਿਰਾ);
		sigaction(SIGHUP, &sa, ਨਿਰਾ);
		sigaction(SIGINT, &sa, ਨਿਰਾ);
		sigaction(SIGQUIT, &sa, ਨਿਰਾ);
		sigaction(SIGTERM, &sa, ਨਿਰਾ);
		sigaction(SIGUSR1, &sa, ਨਿਰਾ);
		sigaction(SIGWINCH, &sa, ਨਿਰਾ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);

		ਜੇ (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (ਅੱਖਰ *)ਨਿਰਾ);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
	ਜੇ (rc_waitpid(pid) == -1) {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ਸਥਿਰ ਖਾਲੀ
open_shell(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *shell;
	ਬਣਤਰ passwd *pw;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਅਚਲ ਅੱਖਰ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ਜੇ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ਅੱਖਰ *) ਨਿਰਾ);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#ਜੇ_ਅੰਤ

	shell = rc_conf_ਮੁੱਲ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ਜੇ (shell == ਨਿਰਾ) {
		shell = getenv("SHELL");
		ਜੇ (shell == ਨਿਰਾ) {
			pw = getpwuid(getuid());
			ਜੇ (pw)
				shell = pw->pw_shell;
			ਜੇ (shell == ਨਿਰਾ)
				shell = "/bin/sh";
		}
	}
	ਲੜੀਵਾਰ_ਪ੍ਰੋਗਰਾਮ(shell);
}

ਸਥਿਰ bool
ਸੈੱਟ_krunlevel(ਅਚਲ ਅੱਖਰ *level)
{
	FILE *fp;

	ਜੇ (rc_is_user())
		ਵਾਪਸ true;

	ਜੇ (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		ਜੇ (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ਵਾਪਸ false;
	}

	ਜੇ (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ਵਾਪਸ false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	ਵਾਪਸ true;
}

ਸਥਿਰ ਅੱਖਰ *get_krunlevel(ਖਾਲੀ)
{
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	FILE *fp;
	size_t i = 0;

	ਜੇ (rc_is_user())
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (access(RC_KRUNLEVEL, F_OK) != 0)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (xgetline(&ਬਫਰ, &i, fp) == -1) {
		ਮੁਕਤ(ਬਫਰ);
		ਬਫਰ = ਨਿਰਾ;
	}

	fclose(fp);
	ਵਾਪਸ ਬਫਰ;
}

ਸਥਿਰ ਖਾਲੀ
ਜੋੜੋ_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(ਆਕਾਰ(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_ਪਾਓ_ਸਿਰ(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
}

ਸਥਿਰ ਖਾਲੀ
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		ਜੇ (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_ਪਾਓ_ਸਿਰ(&free_these_pids, p, entries);
			ਰੋਕੋ;
		}
	}
	/* only call free if we're not inside a signal handler */
	ਜੇ (!inside_signal) {
		LIST_FOREACH_ਸੁਰੱਖਿਅਤ(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			ਮੁਕਤ(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
}

ਸਥਿਰ ਖਾਲੀ
wait_for_services(ਖਾਲੀ)
{
	ਲਈ (;;) {
		ਜਦੋਂ (waitpid(0, 0, 0) != -1)
			;
		ਜੇ (errno != EINTR)
			ਰੋਕੋ;
	}
}

ਸਥਿਰ ਖਾਲੀ
handle_signal(ਪੂਰਨ sig)
{
	ਪੂਰਨ serrno = errno;
	ਅਚਲ ਅੱਖਰ *signame = ਨਿਰਾ;
	pid_t pid;
	RC_PID *pi;
	ਪੂਰਨ ਸਥਿਤੀ = 0;
	ਬਣਤਰ winsize ws;
	sigset_t sset;

	ਚੋਣ (sig) {
	ਹਾਲਤ SIGCHLD:
		ਕਰੋ {
			pid = waitpid(-1, &ਸਥਿਤੀ, WNOHANG);
			ਜੇ (pid < 0) {
				ਜੇ (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				ਵਾਪਸ;
			}
		} ਜਦੋਂ (!WIFEXITED(ਸਥਿਤੀ) && !WIFSIGNALED(ਸਥਿਤੀ));

		/* Remove that pid from our list */
		ਜੇ (pid > 0)
			remove_pid(pid, true);
		ਰੋਕੋ;

	ਹਾਲਤ SIGWINCH:
		ਜੇ (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		ਰੋਕੋ;

	ਹਾਲਤ SIGINT:
		ਜੇ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ਹਾਲਤ SIGTERM:
		ਜੇ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ਹਾਲਤ SIGQUIT:
		ਜੇ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	ਹਾਲਤ SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, ਨਿਰਾ);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_ABORT, ਨਿਰਾ);

		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
		/* NOTREACHED */

	ਮੂਲ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ਸਥਿਰ ਖਾਲੀ
do_sysinit(ਖਾਲੀ)
{
	ਬਣਤਰ utsname uts;
	ਅਚਲ ਅੱਖਰ *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	ਜੇ (access(INITEARLYSH, F_OK) == 0)
		ਲੜੀਵਾਰ_ਪ੍ਰੋਗਰਾਮ(INITEARLYSH);

	uname(&uts);
	ਜੇ (!rc_yesno(getenv("EINFO_QUIET"))) {
		ਲਿਖੋ("\n   %sOpenRC %s" ਸੰਸਕਰਣ "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_ਸਾਧਾਰਨ), ecolor(ECOLOR_BRACKET));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BRANDING
		ਲਿਖੋ(BRANDING " (%s)", uts.machine);
#ਨਹੀਂਤਾਂ
		ਲਿਖੋ("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#ਜੇ_ਅੰਤ
	}

	ਜੇ ((sys = rc_sys()))
		ਲਿਖੋ(" [%s]", sys);

	ਲਿਖੋ("%s\n\n", ecolor(ECOLOR_ਸਾਧਾਰਨ));

	ਜੇ (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		ਲਿਖੋ("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_ਸਾਧਾਰਨ));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	ਲੜੀਵਾਰ_ਪ੍ਰੋਗਰਾਮ(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	ਜੇ ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	ਜੇ ((main_deptree = rc_deptree_ਲੱਦੋ_2(false, ਨਿਰਾ)) == ਨਿਰਾ)
		eerrorx("failed to load deptree");
}

ਸਥਿਰ bool
runlevel_config(ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *level)
{
	ਅੱਖਰ *init = rc_service_resolve(service);
	ਅੱਖਰ *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	ਮੁਕਤ(conf);
	ਮੁਕਤ(init);
	ਵਾਪਸ retval;
}

ਸਥਿਰ ਖਾਲੀ
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *ਸ਼ੁਰੂ_services,
				 ਅਚਲ RC_STRINGLIST *stop_services, ਅਚਲ RC_DEPTREE *deptree,
				 ਅਚਲ ਅੱਖਰ *newlevel, bool parallel, bool going_ਥੱਲੇ)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = ਨਿਰਾ;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	ਜੇ (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_ਜੋੜੋ(types_nw, "needsme");
		rc_stringlist_ਜੋੜੋ(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_ਵੰਡੋ(rc_conf_ਮੁੱਲ("rc_nostop"), " ");
	TAILQ_FOREACH_ਉਲਟ(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->ਮੁੱਲ);
		ਜੇ (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			ਜਾਰੀ;

		/* Sometimes we don't ever want to stop a service. */
		ਜੇ (rc_stringlist_ਲੱਭੋ(nostop, service->ਮੁੱਲ)) {
			rc_service_mark(service->ਮੁੱਲ, RC_SERVICE_FAILED);
			ਜਾਰੀ;
		}
		kwords = rc_deptree_depend(deptree, service->ਮੁੱਲ, "keyword");
		ਜੇ (rc_stringlist_ਲੱਭੋ(kwords, "-stop") ||
		    rc_stringlist_ਲੱਭੋ(kwords, "nostop") ||
		    (going_ਥੱਲੇ &&
			(rc_stringlist_ਲੱਭੋ(kwords, "-shutdown") ||
			    rc_stringlist_ਲੱਭੋ(kwords, "noshutdown"))))
			nstop = true;
		ਨਹੀਂਤਾਂ
			nstop = false;
		rc_stringlist_free(kwords);
		ਜੇ (nstop) {
			rc_service_mark(service->ਮੁੱਲ, RC_SERVICE_FAILED);
			ਜਾਰੀ;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		ਜੇ (crashed &&
		    rc_service_daemons_crashed(service->ਮੁੱਲ))
			ਜਾਓ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_ਲੱਭੋ(ਸ਼ੁਰੂ_services, service->ਮੁੱਲ);
		ਜੇ (svc1) {
			ਜੇ (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				ਜੇ (!runlevel_config(service->ਮੁੱਲ,runlevel) &&
				    !runlevel_config(service->ਮੁੱਲ,newlevel))
					ਜਾਰੀ;
			}
			ਨਹੀਂਤਾਂ
				ਜਾਰੀ;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		ਜੇ (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_ਜੋੜੋ(tmplist, service->ਮੁੱਲ);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = ਨਿਰਾ;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_ਲੱਭੋ(ਸ਼ੁਰੂ_services,
				    svc1->ਮੁੱਲ);
				ਜੇ (svc2)
					ਰੋਕੋ;
			}
			rc_stringlist_free(deporder);

			ਜੇ (svc2)
				ਜਾਰੀ;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->ਮੁੱਲ);
		ਜੇ (pid > 0) {
			ਜੋੜੋ_pid(pid);
			ਜੇ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	ਜੇ (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ਸਥਿਰ ਖਾਲੀ
do_ਸ਼ੁਰੂ_services(ਅਚਲ RC_STRINGLIST *ਸ਼ੁਰੂ_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	ਜੇ (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	ਜੇ (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, ਸ਼ੁਰੂ_services, entries) {
		state = rc_service_state(service->ਮੁੱਲ);
		ਜੇ (state & RC_SERVICE_FAILED)
			ਜਾਰੀ;
		ਜੇ (!(state & RC_SERVICE_STOPPED)) {
			ਜੇ (crashed &&
			    rc_service_daemons_crashed(service->ਮੁੱਲ))
				rc_service_mark(service->ਮੁੱਲ,
				    RC_SERVICE_STOPPED);
			ਨਹੀਂਤਾਂ
			    ਜਾਰੀ;
		}
		ਜੇ (!interactive)
			interactive = want_interactive();

		ਜੇ (interactive) {
	interactive_retry:
			ਲਿਖੋ("\n");
			einfo("About to start the service %s",
			    service->ਮੁੱਲ);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			ਚੋਣ (read_ਕੁੰਜੀ(true)) {
			ਹਾਲਤ '1': ਰੋਕੋ;
			ਹਾਲਤ '2': ਜਾਰੀ;
			ਹਾਲਤ '3': interactive = false; ਰੋਕੋ;
			ਹਾਲਤ '4': open_shell(); ਜਾਓ interactive_retry;
			ਮੂਲ: ਜਾਓ interactive_option;
			}
		}

		pid = service_ਸ਼ੁਰੂ(service->ਮੁੱਲ);
		ਜੇ (pid == -1)
			ਰੋਕੋ;
		/* Remember the pid if we're running in parallel */
		ਜੇ (pid > 0) {
			ਜੋੜੋ_pid(pid);
			ਜੇ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	ਜੇ (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	ਨਹੀਂਤਾਂ
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ RC_DEBUG
RC_NORETURN ਸਥਿਰ ਖਾਲੀ
handle_bad_signal(ਪੂਰਨ sig)
{
	ਅੱਖਰ pid[10];
	ਪੂਰਨ ਸਥਿਤੀ;
	pid_t crashed_pid = getpid();

	ਚੋਣ (fork()) {
	ਹਾਲਤ -1:
		_exit(sig);
		/* NOTREACHED */
	ਹਾਲਤ 0:
		sprintf(pid, "%i", crashed_pid);
		ਲਿਖੋ("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", ਨਿਰਾ));
		/* NOTREACHED */
	ਮੂਲ:
		wait(&ਸਥਿਤੀ);
	}
	_exit(1);
	/* NOTREACHED */
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਅਚਲ ਅੱਖਰ *bootlevel = ਨਿਰਾ;
	ਅੱਖਰ *newlevel = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *systype = ਨਿਰਾ;
	RC_STRINGLIST *deporder = ਨਿਰਾ;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_ਥੱਲੇ = false;
	ਪੂਰਨ depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	ਅੱਖਰ *krunlevel = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *workingdir = "/";
	ਅੱਖਰ *pidstr = ਨਿਰਾ;
	ਪੂਰਨ opt;
	bool parallel;
	ਪੂਰਨ regen = 0;
	bool nostop = false;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਅੱਖਰ *proc;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ RC_DEBUG
	signal_ਤਿਆਰੀ(SIGBUS, handle_bad_signal);
	signal_ਤਿਆਰੀ(SIGILL, handle_bad_signal);
	signal_ਤਿਆਰੀ(SIGSEGV, handle_bad_signal);
#ਜੇ_ਅੰਤ

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	ਜੇ (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	ਜੇ (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ 'n':
			nostop = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'o':
			ਜੇ (*optarg == '\0')
				optarg = ਨਿਰਾ;
			ਜੇ (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			}
			ਜੇ (rc_is_user())
				eerrorx("user services do not currently support --override");
			ਜੇ (!ਸੈੱਟ_krunlevel(optarg))
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
			/* NOTREACHED */
		ਹਾਲਤ 's':
			newlevel = rc_service_resolve(optarg);
			ਜੇ (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		ਹਾਲਤ 'S':
			systype = rc_sys();
			ਜੇ (systype)
				ਲਿਖੋ("%s\n", systype);
			ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ਜੇ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ਜੇ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	ਜੇ (newlevel) {
		ਜੇ (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		ਜੇ (strcmp(newlevel, "reboot") == 0) {
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
	ਮੁਕਤ(pidstr);

	/* Create a list of all services which should be started for the new or
	* current runlevel including those in boot, sysinit and hotplugged
	* runlevels.  Clearly, some of these will already be started so we
	* won't actually be starting them all.
	*/
	bootlevel = getenv("RC_BOOTLEVEL");
	runlevel = default_runlevel();

	rc_logger_open(newlevel ? newlevel : runlevel);

	/* Setup a signal handler */
	signal_ਤਿਆਰੀ(SIGINT, handle_signal);
	signal_ਤਿਆਰੀ(SIGQUIT, handle_signal);
	signal_ਤਿਆਰੀ(SIGTERM, handle_signal);
	signal_ਤਿਆਰੀ(SIGUSR1, handle_signal);
	signal_ਤਿਆਰੀ(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	ਜੇ (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		ਮੁਕਤ(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_ਲੱਦੋ();

	/* Now we start handling our children */
	signal_ਤਿਆਰੀ(SIGCHLD, handle_signal);

	ਜੇ (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_ਥੱਲੇ = true;
		ਜੇ (access(RC_KRUNLEVEL, F_OK) != 0)
			ਸੈੱਟ_krunlevel(runlevel);
		rc_runlevel_ਸੈੱਟ(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} ਨਹੀਂਤਾਂ {
		/* We should not use krunlevel in sysinit or boot runlevels */
		ਜੇ (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			ਜੇ (krunlevel) {
				newlevel = krunlevel;
				ਸੈੱਟ_krunlevel(ਨਿਰਾ);
			}
		}

		ਜੇ (newlevel) {
			ਜੇ (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
			ਜੇ (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				ਜੇ (p == ਨਿਰਾ)
					p = rc_proc_getent("softlevel");
				ਜੇ (p != ਨਿਰਾ) {
					ਸੈੱਟ_krunlevel(p);
					ਮੁਕਤ(p);
				}
			}
#ਜੇ_ਅੰਤ
		}
	}

	ਜੇ (going_ਥੱਲੇ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#ਜੇ_ਅੰਤ

		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_STOP_ਵਿੱਚ, newlevel);
	} ਨਹੀਂਤਾਂ {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_STOP_ਵਿੱਚ, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	ਜੇ (newlevel && strcmp(runlevel, newlevel) != 0 && !going_ਥੱਲੇ) {
		ਜੇ (!rc_runlevel_exists(newlevel)) {
			rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_STOP_ਬਾਹਰਲਾ, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	ਜੇ ((main_deptree = rc_deptree_ਲੱਦੋ_2(0, &regen)) == ਨਿਰਾ) {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_STOP_ਬਾਹਰਲਾ, runlevel);
		eerrorx("failed to load deptree");
	}

	ਜੇ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	ਜੇ (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_STOP_ਬਾਹਰਲਾ, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), strerror(errno));
	}

	/* Create a list of all services which we could stop (assuming
	* they won't be active in the new or current runlevel) including
	* all those services which have been started, are inactive or
	* are currently starting.  Clearly, some of these will be listed
	* in the new or current runlevel so we won't actually be stopping
	* them all.
	*/
	main_stop_services = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTED);
	tmplist = rc_services_ਵਿੱਚ_state(RC_SERVICE_INACTIVE);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ਮੁਕਤ(tmplist);
	tmplist = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ਮੁਕਤ(tmplist);
	ਜੇ (main_stop_services)
		rc_stringlist_ਛਾਂਟੋ(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(main_types_nwua, "ineed");
	rc_stringlist_ਜੋੜੋ(main_types_nwua, "iwant");
	rc_stringlist_ਜੋੜੋ(main_types_nwua, "iuse");
	rc_stringlist_ਜੋੜੋ(main_types_nwua, "iafter");

	ਜੇ (main_stop_services) {
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
	main_hotplugged_services = rc_services_ਵਿੱਚ_state(RC_SERVICE_HOTPLUGGED);
	main_ਸ਼ੁਰੂ_services = rc_services_ਵਿੱਚ_runlevel_stacked(newlevel ?
	    newlevel : runlevel);
	ਜੇ (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_ਵਿੱਚ_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_ਸ਼ੁਰੂ_services, tmplist, entries);
		ਮੁਕਤ(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		ਜੇ (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			ਜੇ (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_ਵਿੱਚ_runlevel(bootlevel);
				TAILQ_CONCAT(main_ਸ਼ੁਰੂ_services, tmplist, entries);
				ਮੁਕਤ(tmplist);
			}
			ਜੇ (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_ਸ਼ੁਰੂ_services,
					service->ਮੁੱਲ);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	ਜੇ (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_ਸ਼ੁਰੂ_services, main_stop_services, main_deptree, newlevel, parallel, going_ਥੱਲੇ);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_STOP_ਬਾਹਰਲਾ,
	    going_ਥੱਲੇ ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	ਜੇ (newlevel) {
		rc_runlevel_ਸੈੱਟ(newlevel);
		ਮੁਕਤ(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	ਜੇ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#ਜੇ_ਅੰਤ

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_ਸ਼ੁਰੂ_ਵਿੱਚ, runlevel);

	/* Re-add our hotplugged services if they stopped */
	ਜੇ (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->ਮੁੱਲ, RC_SERVICE_HOTPLUGGED);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	ਜੇ (proc) {
		ਜਦੋਂ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		ਮੁਕਤ(proc);
	}
#ਜੇ_ਅੰਤ

	/* If we have a list of services to start then... */
	ਜੇ (main_ਸ਼ੁਰੂ_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_ਉਲਟ(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *ਲੜੀਵਾਰ_services = rc_services_ਵਿੱਚ_runlevel(rlevel->ਮੁੱਲ);

			/* Start those services. */
			rc_stringlist_ਛਾਂਟੋ(&ਲੜੀਵਾਰ_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, ਲੜੀਵਾਰ_services, rlevel->ਮੁੱਲ, depoptions | RC_DEP_ਸ਼ੁਰੂ);
			rc_stringlist_free(ਲੜੀਵਾਰ_services);
			ਲੜੀਵਾਰ_services = deporder;
			do_ਸ਼ੁਰੂ_services(ਲੜੀਵਾਰ_services, parallel);

			/* Wait for our services to finish */
			wait_for_services();

			/* Free the list of services, we're done with it. */
			rc_stringlist_free(ਲੜੀਵਾਰ_services);
		}
		rc_stringlist_free(runlevel_chain);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	ਜੇ (proc) {
		ਜਦੋਂ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		ਮੁਕਤ(proc);
	}

#ਜੇ_ਅੰਤ

	rc_plugin_ਲੜੀਵਾਰ(RC_HOOK_RUNLEVEL_ਸ਼ੁਰੂ_ਬਾਹਰਲਾ, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	ਜੇ (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	ਵਾਪਸ EXIT_SUCCESS;
}
