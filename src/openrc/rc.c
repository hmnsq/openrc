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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <libgen.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/utsname.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <termios.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "rc-logger.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "plugin.h"
#അന്തര്ഭാവ "version.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "a:no:s:S" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "no-stop", 0, ശൂന്യ, 'n' },
	{ "override",    1, ശൂന്യ, 'o' },
	{ "service",     1, ശൂന്യ, 's' },
	{ "sys",         0, ശൂന്യ, 'S' },
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
സ്ഥിരം അക്ഷരം *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിതം RC_STRINGLIST *main_hotplugged_services;
സ്ഥിതം RC_STRINGLIST *main_stop_services;
സ്ഥിതം RC_STRINGLIST *main_start_services;
സ്ഥിതം RC_STRINGLIST *main_types_nw;
സ്ഥിതം RC_STRINGLIST *main_types_nwua;
സ്ഥിതം RC_DEPTREE *main_deptree;
സ്ഥിതം അക്ഷരം *runlevel;

ഘടന termios *termios_orig = ശൂന്യ;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

സ്ഥിതം ശൂന്യം
clean_failed(ശൂന്യം)
{
	DIR *dp;
	ഘടന dirent *d;

	/* Clean the failed services state dir now */
	എങ്കിൽ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		തിരികെ;

	ആകുമ്പോൾ ((d = readdir(dp))) {
		എങ്കിൽ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			തുടരുക;

		എങ്കിൽ (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

സ്ഥിതം ശൂന്യം
cleanup(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	എങ്കിൽ (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		എങ്കിൽ (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			സ്വതന്ത്രം(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		സ്വതന്ത്രം(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		സ്വതന്ത്രം(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	സ്വതന്ത്രം(runlevel);
}

സ്ഥിതം അക്ഷരം
read_താക്കോൽ(bool block)
{
	ഘടന termios termios;
	അക്ഷരം c = 0;
	പൂർണ്ണസംഖ്യ fd = STDIN_FILENO;

	എങ്കിൽ (!isatty(fd))
		തിരികെ false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	എങ്കിൽ (!termios_orig) {
		termios_orig = xmalloc(വലുപ്പം(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	എങ്കിൽ (block)
		termios.c_cc[VMIN] = 1;
	അല്ലെങ്കിൽ {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	എങ്കിൽ (പഠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	തിരികെ c;
}

സ്ഥിതം bool
want_interactive(ശൂന്യം)
{
	അക്ഷരം c;
	സ്ഥിതം bool gotinteractive;
	സ്ഥിതം bool interactive;

	എങ്കിൽ (rc_yesno(getenv("EINFO_QUIET")))
		തിരികെ false;
	എങ്കിൽ (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	എങ്കിൽ (!interactive)
		തിരികെ false;
	c = read_താക്കോൽ(false);
	തിരികെ (c == 'I' || c == 'i') ? true : false;
}

സ്ഥിതം ശൂന്യം
mark_interactive(ശൂന്യം)
{
	FILE *fp;

	എങ്കിൽ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

സ്ഥിതം ശൂന്യം
run_program(സ്ഥിരം അക്ഷരം *prog)
{
	ഘടന sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, വലുപ്പം(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	എങ്കിൽ (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	എങ്കിൽ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ശൂന്യ);
		sigaction(SIGHUP, &sa, ശൂന്യ);
		sigaction(SIGINT, &sa, ശൂന്യ);
		sigaction(SIGQUIT, &sa, ശൂന്യ);
		sigaction(SIGTERM, &sa, ശൂന്യ);
		sigaction(SIGUSR1, &sa, ശൂന്യ);
		sigaction(SIGWINCH, &sa, ശൂന്യ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ശൂന്യ);

		എങ്കിൽ (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (അക്ഷരം *)ശൂന്യ);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
	എങ്കിൽ (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

സ്ഥിതം ശൂന്യം
open_shell(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *shell;
	ഘടന passwd *pw;

#ifdef __linux__
	സ്ഥിരം അക്ഷരം *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	എങ്കിൽ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (അക്ഷരം *) ശൂന്യ);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_മൂല്യം("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	എങ്കിൽ (shell == ശൂന്യ) {
		shell = getenv("SHELL");
		എങ്കിൽ (shell == ശൂന്യ) {
			pw = getpwuid(getuid());
			എങ്കിൽ (pw)
				shell = pw->pw_shell;
			എങ്കിൽ (shell == ശൂന്യ)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

സ്ഥിതം bool
set_krunlevel(സ്ഥിരം അക്ഷരം *level)
{
	FILE *fp;

	എങ്കിൽ (rc_is_user())
		തിരികെ true;

	എങ്കിൽ (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		എങ്കിൽ (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		തിരികെ false;
	}

	എങ്കിൽ (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		തിരികെ false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	തിരികെ true;
}

സ്ഥിതം അക്ഷരം *get_krunlevel(ശൂന്യം)
{
	അക്ഷരം *buffer = ശൂന്യ;
	FILE *fp;
	size_t i = 0;

	എങ്കിൽ (rc_is_user())
		തിരികെ ശൂന്യ;

	എങ്കിൽ (access(RC_KRUNLEVEL, F_OK) != 0)
		തിരികെ ശൂന്യ;
	എങ്കിൽ (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		തിരികെ ശൂന്യ;
	}

	എങ്കിൽ (xgetline(&buffer, &i, fp) == -1) {
		സ്വതന്ത്രം(buffer);
		buffer = ശൂന്യ;
	}

	fclose(fp);
	തിരികെ buffer;
}

സ്ഥിതം ശൂന്യം
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(വലുപ്പം(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_തിരുകുക_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
}

സ്ഥിതം ശൂന്യം
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		എങ്കിൽ (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_തിരുകുക_HEAD(&free_these_pids, p, entries);
			നിർത്തുക;
		}
	}
	/* only call free if we're not inside a signal handler */
	എങ്കിൽ (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			സ്വതന്ത്രം(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
}

സ്ഥിതം ശൂന്യം
wait_for_services(ശൂന്യം)
{
	വേണ്ടി (;;) {
		ആകുമ്പോൾ (waitpid(0, 0, 0) != -1)
			;
		എങ്കിൽ (errno != EINTR)
			നിർത്തുക;
	}
}

സ്ഥിതം ശൂന്യം
handle_signal(പൂർണ്ണസംഖ്യ sig)
{
	പൂർണ്ണസംഖ്യ serrno = errno;
	സ്ഥിരം അക്ഷരം *signame = ശൂന്യ;
	pid_t pid;
	RC_PID *pi;
	പൂർണ്ണസംഖ്യ status = 0;
	ഘടന winsize ws;
	sigset_t sset;

	തിരഞ്ഞെടുപ്പ് (sig) {
	അവസ്ഥ SIGCHLD:
		ചെയ്യുക {
			pid = waitpid(-1, &status, WNOHANG);
			എങ്കിൽ (pid < 0) {
				എങ്കിൽ (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				തിരികെ;
			}
		} ആകുമ്പോൾ (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		എങ്കിൽ (pid > 0)
			remove_pid(pid, true);
		നിർത്തുക;

	അവസ്ഥ SIGWINCH:
		എങ്കിൽ (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		നിർത്തുക;

	അവസ്ഥ SIGINT:
		എങ്കിൽ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	അവസ്ഥ SIGTERM:
		എങ്കിൽ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	അവസ്ഥ SIGQUIT:
		എങ്കിൽ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	അവസ്ഥ SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, ശൂന്യ);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, ശൂന്യ);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	സ്ഥിരസ്ഥിതി:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

സ്ഥിതം ശൂന്യം
do_sysinit(ശൂന്യം)
{
	ഘടന utsname uts;
	സ്ഥിരം അക്ഷരം *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	എങ്കിൽ (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	എങ്കിൽ (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#അല്ലെങ്കിൽ
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	എങ്കിൽ ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	എങ്കിൽ (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	എങ്കിൽ ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	എങ്കിൽ ((main_deptree = _rc_deptree_load(false, ശൂന്യ)) == ശൂന്യ)
		eerrorx("failed to load deptree");
}

സ്ഥിതം bool
runlevel_config(സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *level)
{
	അക്ഷരം *init = rc_service_resolve(service);
	അക്ഷരം *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	സ്വതന്ത്രം(conf);
	സ്വതന്ത്രം(init);
	തിരികെ retval;
}

സ്ഥിതം ശൂന്യം
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 സ്ഥിരം RC_STRINGLIST *stop_services, സ്ഥിരം RC_DEPTREE *deptree,
				 സ്ഥിരം അക്ഷരം *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = ശൂന്യ;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	എങ്കിൽ (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_മൂല്യം("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->മൂല്യം);
		എങ്കിൽ (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			തുടരുക;

		/* Sometimes we don't ever want to stop a service. */
		എങ്കിൽ (rc_stringlist_find(nostop, service->മൂല്യം)) {
			rc_service_mark(service->മൂല്യം, RC_SERVICE_FAILED);
			തുടരുക;
		}
		kwords = rc_deptree_depend(deptree, service->മൂല്യം, "keyword");
		എങ്കിൽ (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		അല്ലെങ്കിൽ
			nstop = false;
		rc_stringlist_free(kwords);
		എങ്കിൽ (nstop) {
			rc_service_mark(service->മൂല്യം, RC_SERVICE_FAILED);
			തുടരുക;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		എങ്കിൽ (crashed &&
		    rc_service_daemons_crashed(service->മൂല്യം))
			പോകുക stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->മൂല്യം);
		എങ്കിൽ (svc1) {
			എങ്കിൽ (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				എങ്കിൽ (!runlevel_config(service->മൂല്യം,runlevel) &&
				    !runlevel_config(service->മൂല്യം,newlevel))
					തുടരുക;
			}
			അല്ലെങ്കിൽ
				തുടരുക;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		എങ്കിൽ (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->മൂല്യം);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = ശൂന്യ;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->മൂല്യം);
				എങ്കിൽ (svc2)
					നിർത്തുക;
			}
			rc_stringlist_free(deporder);

			എങ്കിൽ (svc2)
				തുടരുക;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->മൂല്യം);
		എങ്കിൽ (pid > 0) {
			add_pid(pid);
			എങ്കിൽ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	എങ്കിൽ (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

സ്ഥിതം ശൂന്യം
do_start_services(സ്ഥിരം RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	എങ്കിൽ (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	എങ്കിൽ (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->മൂല്യം);
		എങ്കിൽ (state & RC_SERVICE_FAILED)
			തുടരുക;
		എങ്കിൽ (!(state & RC_SERVICE_STOPPED)) {
			എങ്കിൽ (crashed &&
			    rc_service_daemons_crashed(service->മൂല്യം))
				rc_service_mark(service->മൂല്യം,
				    RC_SERVICE_STOPPED);
			അല്ലെങ്കിൽ
			    തുടരുക;
		}
		എങ്കിൽ (!interactive)
			interactive = want_interactive();

		എങ്കിൽ (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->മൂല്യം);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			തിരഞ്ഞെടുപ്പ് (read_താക്കോൽ(true)) {
			അവസ്ഥ '1': നിർത്തുക;
			അവസ്ഥ '2': തുടരുക;
			അവസ്ഥ '3': interactive = false; നിർത്തുക;
			അവസ്ഥ '4': open_shell(); പോകുക interactive_retry;
			സ്ഥിരസ്ഥിതി: പോകുക interactive_option;
			}
		}

		pid = service_start(service->മൂല്യം);
		എങ്കിൽ (pid == -1)
			നിർത്തുക;
		/* Remember the pid if we're running in parallel */
		എങ്കിൽ (pid > 0) {
			add_pid(pid);
			എങ്കിൽ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	എങ്കിൽ (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	അല്ലെങ്കിൽ
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN സ്ഥിതം ശൂന്യം
handle_bad_signal(പൂർണ്ണസംഖ്യ sig)
{
	അക്ഷരം pid[10];
	പൂർണ്ണസംഖ്യ status;
	pid_t crashed_pid = getpid();

	തിരഞ്ഞെടുപ്പ് (fork()) {
	അവസ്ഥ -1:
		_exit(sig);
		/* NOTREACHED */
	അവസ്ഥ 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", ശൂന്യ));
		/* NOTREACHED */
	സ്ഥിരസ്ഥിതി:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	സ്ഥിരം അക്ഷരം *bootlevel = ശൂന്യ;
	അക്ഷരം *newlevel = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *systype = ശൂന്യ;
	RC_STRINGLIST *deporder = ശൂന്യ;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	പൂർണ്ണസംഖ്യ depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	അക്ഷരം *krunlevel = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *workingdir = "/";
	അക്ഷരം *pidstr = ശൂന്യ;
	പൂർണ്ണസംഖ്യ opt;
	bool parallel;
	പൂർണ്ണസംഖ്യ regen = 0;
	bool nostop = false;
#ifdef __linux__
	അക്ഷരം *proc;
	അക്ഷരം *p;
	അക്ഷരം *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	എങ്കിൽ (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	എങ്കിൽ (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'n':
			nostop = true;
			നിർത്തുക;
		അവസ്ഥ 'o':
			എങ്കിൽ (*optarg == '\0')
				optarg = ശൂന്യ;
			എങ്കിൽ (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			എങ്കിൽ (rc_is_user())
				eerrorx("user services do not currently support --override");
			എങ്കിൽ (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		അവസ്ഥ 's':
			newlevel = rc_service_resolve(optarg);
			എങ്കിൽ (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		അവസ്ഥ 'S':
			systype = rc_sys();
			എങ്കിൽ (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	എങ്കിൽ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	എങ്കിൽ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	എങ്കിൽ (newlevel) {
		എങ്കിൽ (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		എങ്കിൽ (strcmp(newlevel, "reboot") == 0) {
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
	സ്വതന്ത്രം(pidstr);

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
	എങ്കിൽ (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		സ്വതന്ത്രം(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	എങ്കിൽ (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		എങ്കിൽ (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} അല്ലെങ്കിൽ {
		/* We should not use krunlevel in sysinit or boot runlevels */
		എങ്കിൽ (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			എങ്കിൽ (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(ശൂന്യ);
			}
		}

		എങ്കിൽ (newlevel) {
			എങ്കിൽ (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			എങ്കിൽ (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				എങ്കിൽ (p == ശൂന്യ)
					p = rc_proc_getent("softlevel");
				എങ്കിൽ (p != ശൂന്യ) {
					set_krunlevel(p);
					സ്വതന്ത്രം(p);
				}
			}
#endif
		}
	}

	എങ്കിൽ (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} അല്ലെങ്കിൽ {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	എങ്കിൽ (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		എങ്കിൽ (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	എങ്കിൽ ((main_deptree = _rc_deptree_load(0, &regen)) == ശൂന്യ) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	എങ്കിൽ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	എങ്കിൽ (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	സ്വതന്ത്രം(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	സ്വതന്ത്രം(tmplist);
	എങ്കിൽ (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	എങ്കിൽ (main_stop_services) {
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
	എങ്കിൽ (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		സ്വതന്ത്രം(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		എങ്കിൽ (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			എങ്കിൽ (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				സ്വതന്ത്രം(tmplist);
			}
			എങ്കിൽ (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->മൂല്യം);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	എങ്കിൽ (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	എങ്കിൽ (newlevel) {
		rc_runlevel_set(newlevel);
		സ്വതന്ത്രം(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	എങ്കിൽ (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->മൂല്യം, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	എങ്കിൽ (proc) {
		ആകുമ്പോൾ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		സ്വതന്ത്രം(proc);
	}
#endif

	/* If we have a list of services to start then... */
	എങ്കിൽ (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->മൂല്യം);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->മൂല്യം, depoptions | RC_DEP_START);
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
	എങ്കിൽ (proc) {
		ആകുമ്പോൾ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		സ്വതന്ത്രം(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	എങ്കിൽ (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	തിരികെ EXIT_SUCCESS;
}
