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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <libgen.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/utsname.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <termios.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "rc_exec.h"
#অন্তর্ভাব "rc-logger.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "plugin.h"
#অন্তর্ভাব "version.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

ধ্রুব বর্ণ *extraopts = রিক্ত;
ধ্রুব বর্ণ getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{ "no-stop", 0, রিক্ত, 'n' },
	{ "override",    1, রিক্ত, 'o' },
	{ "service",     1, রিক্ত, 's' },
	{ "sys",         0, রিক্ত, 'S' },
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ধ্রুব বর্ণ *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

ধ্রুব বর্ণ *applet = রিক্ত;
স্থির RC_STRINGLIST *main_hotplugged_services;
স্থির RC_STRINGLIST *main_stop_services;
স্থির RC_STRINGLIST *main_start_services;
স্থির RC_STRINGLIST *main_types_nw;
স্থির RC_STRINGLIST *main_types_nwua;
স্থির RC_DEPTREE *main_deptree;
স্থির বর্ণ *runlevel;

সংরচনা termios *termios_orig = রিক্ত;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

স্থির শূন্য
clean_failed(শূন্য)
{
	DIR *dp;
	সংরচনা dirent *d;

	/* Clean the failed services state dir now */
	যদি (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		প্রত্যাগম;

	যাবত্ ((d = readdir(dp))) {
		যদি (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			অনুবর্ত;

		যদি (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

স্থির শূন্য
cleanup(শূন্য)
{
	পূর্ণাঙ্ক svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	যদি (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		যদি (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			মুক্ত(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		মুক্ত(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		মুক্ত(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	মুক্ত(runlevel);
}

স্থির বর্ণ
read_চাবি(bool block)
{
	সংরচনা termios termios;
	বর্ণ c = 0;
	পূর্ণাঙ্ক fd = STDIN_FILENO;

	যদি (!isatty(fd))
		প্রত্যাগম false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	যদি (!termios_orig) {
		termios_orig = xmalloc(পরিমাণ(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	যদি (block)
		termios.c_cc[VMIN] = 1;
	অন্যথা {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	যদি (পঠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	প্রত্যাগম c;
}

স্থির bool
want_interactive(শূন্য)
{
	বর্ণ c;
	স্থির bool gotinteractive;
	স্থির bool interactive;

	যদি (rc_yesno(getenv("EINFO_QUIET")))
		প্রত্যাগম false;
	যদি (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	যদি (!interactive)
		প্রত্যাগম false;
	c = read_চাবি(false);
	প্রত্যাগম (c == 'I' || c == 'i') ? true : false;
}

স্থির শূন্য
mark_interactive(শূন্য)
{
	FILE *fp;

	যদি ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

স্থির শূন্য
run_program(ধ্রুব বর্ণ *prog)
{
	সংরচনা sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, পরিমাণ(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	যদি (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	যদি (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, রিক্ত);
		sigaction(SIGHUP, &sa, রিক্ত);
		sigaction(SIGINT, &sa, রিক্ত);
		sigaction(SIGQUIT, &sa, রিক্ত);
		sigaction(SIGTERM, &sa, রিক্ত);
		sigaction(SIGUSR1, &sa, রিক্ত);
		sigaction(SIGWINCH, &sa, রিক্ত);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, রিক্ত);

		যদি (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (বর্ণ *)রিক্ত);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, রিক্ত);
	যদি (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

স্থির শূন্য
open_shell(শূন্য)
{
	ধ্রুব বর্ণ *shell;
	সংরচনা passwd *pw;

#ifdef __linux__
	ধ্রুব বর্ণ *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	যদি (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (বর্ণ *) রিক্ত);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_মান("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	যদি (shell == রিক্ত) {
		shell = getenv("SHELL");
		যদি (shell == রিক্ত) {
			pw = getpwuid(getuid());
			যদি (pw)
				shell = pw->pw_shell;
			যদি (shell == রিক্ত)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

স্থির bool
set_krunlevel(ধ্রুব বর্ণ *level)
{
	FILE *fp;

	যদি (rc_is_user())
		প্রত্যাগম true;

	যদি (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		যদি (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		প্রত্যাগম false;
	}

	যদি (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		প্রত্যাগম false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	প্রত্যাগম true;
}

স্থির বর্ণ *get_krunlevel(শূন্য)
{
	বর্ণ *buffer = রিক্ত;
	FILE *fp;
	size_t i = 0;

	যদি (rc_is_user())
		প্রত্যাগম রিক্ত;

	যদি (access(RC_KRUNLEVEL, F_OK) != 0)
		প্রত্যাগম রিক্ত;
	যদি (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		প্রত্যাগম রিক্ত;
	}

	যদি (xgetline(&buffer, &i, fp) == -1) {
		মুক্ত(buffer);
		buffer = রিক্ত;
	}

	fclose(fp);
	প্রত্যাগম buffer;
}

স্থির শূন্য
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(পরিমাণ(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_ঢোকাও_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, রিক্ত);
}

স্থির শূন্য
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		যদি (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_ঢোকাও_HEAD(&free_these_pids, p, entries);
			ভঙ্গ;
		}
	}
	/* only call free if we're not inside a signal handler */
	যদি (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			মুক্ত(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, রিক্ত);
}

স্থির শূন্য
wait_for_services(শূন্য)
{
	প্রতি (;;) {
		যাবত্ (waitpid(0, 0, 0) != -1)
			;
		যদি (errno != EINTR)
			ভঙ্গ;
	}
}

স্থির শূন্য
handle_signal(পূর্ণাঙ্ক sig)
{
	পূর্ণাঙ্ক serrno = errno;
	ধ্রুব বর্ণ *signame = রিক্ত;
	pid_t pid;
	RC_PID *pi;
	পূর্ণাঙ্ক status = 0;
	সংরচনা winsize ws;
	sigset_t sset;

	বিকল্প (sig) {
	পক্ষ SIGCHLD:
		কুরু {
			pid = waitpid(-1, &status, WNOHANG);
			যদি (pid < 0) {
				যদি (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				প্রত্যাগম;
			}
		} যাবত্ (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		যদি (pid > 0)
			remove_pid(pid, true);
		ভঙ্গ;

	পক্ষ SIGWINCH:
		যদি (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		ভঙ্গ;

	পক্ষ SIGINT:
		যদি (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	পক্ষ SIGTERM:
		যদি (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	পক্ষ SIGQUIT:
		যদি (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	পক্ষ SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, রিক্ত);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, রিক্ত);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	স্বভাব:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

স্থির শূন্য
do_sysinit(শূন্য)
{
	সংরচনা utsname uts;
	ধ্রুব বর্ণ *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	যদি (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	যদি (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#অন্যথা
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	যদি ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	যদি (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	যদি ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	যদি ((main_deptree = _rc_deptree_load(false, রিক্ত)) == রিক্ত)
		eerrorx("failed to load deptree");
}

স্থির bool
runlevel_config(ধ্রুব বর্ণ *service, ধ্রুব বর্ণ *level)
{
	বর্ণ *init = rc_service_resolve(service);
	বর্ণ *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	মুক্ত(conf);
	মুক্ত(init);
	প্রত্যাগম retval;
}

স্থির শূন্য
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 ধ্রুব RC_STRINGLIST *stop_services, ধ্রুব RC_DEPTREE *deptree,
				 ধ্রুব বর্ণ *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = রিক্ত;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	যদি (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_মান("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->মান);
		যদি (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			অনুবর্ত;

		/* Sometimes we don't ever want to stop a service. */
		যদি (rc_stringlist_find(nostop, service->মান)) {
			rc_service_mark(service->মান, RC_SERVICE_FAILED);
			অনুবর্ত;
		}
		kwords = rc_deptree_depend(deptree, service->মান, "keyword");
		যদি (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		অন্যথা
			nstop = false;
		rc_stringlist_free(kwords);
		যদি (nstop) {
			rc_service_mark(service->মান, RC_SERVICE_FAILED);
			অনুবর্ত;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		যদি (crashed &&
		    rc_service_daemons_crashed(service->মান))
			গচ্ছ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->মান);
		যদি (svc1) {
			যদি (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				যদি (!runlevel_config(service->মান,runlevel) &&
				    !runlevel_config(service->মান,newlevel))
					অনুবর্ত;
			}
			অন্যথা
				অনুবর্ত;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		যদি (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->মান);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = রিক্ত;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->মান);
				যদি (svc2)
					ভঙ্গ;
			}
			rc_stringlist_free(deporder);

			যদি (svc2)
				অনুবর্ত;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->মান);
		যদি (pid > 0) {
			add_pid(pid);
			যদি (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	যদি (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

স্থির শূন্য
do_start_services(ধ্রুব RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	যদি (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	যদি (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->মান);
		যদি (state & RC_SERVICE_FAILED)
			অনুবর্ত;
		যদি (!(state & RC_SERVICE_STOPPED)) {
			যদি (crashed &&
			    rc_service_daemons_crashed(service->মান))
				rc_service_mark(service->মান,
				    RC_SERVICE_STOPPED);
			অন্যথা
			    অনুবর্ত;
		}
		যদি (!interactive)
			interactive = want_interactive();

		যদি (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->মান);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			বিকল্প (read_চাবি(true)) {
			পক্ষ '1': ভঙ্গ;
			পক্ষ '2': অনুবর্ত;
			পক্ষ '3': interactive = false; ভঙ্গ;
			পক্ষ '4': open_shell(); গচ্ছ interactive_retry;
			স্বভাব: গচ্ছ interactive_option;
			}
		}

		pid = service_start(service->মান);
		যদি (pid == -1)
			ভঙ্গ;
		/* Remember the pid if we're running in parallel */
		যদি (pid > 0) {
			add_pid(pid);
			যদি (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	যদি (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	অন্যথা
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN স্থির শূন্য
handle_bad_signal(পূর্ণাঙ্ক sig)
{
	বর্ণ pid[10];
	পূর্ণাঙ্ক status;
	pid_t crashed_pid = getpid();

	বিকল্প (fork()) {
	পক্ষ -1:
		_exit(sig);
		/* NOTREACHED */
	পক্ষ 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", রিক্ত));
		/* NOTREACHED */
	স্বভাব:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	ধ্রুব বর্ণ *bootlevel = রিক্ত;
	বর্ণ *newlevel = রিক্ত;
	ধ্রুব বর্ণ *systype = রিক্ত;
	RC_STRINGLIST *deporder = রিক্ত;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	পূর্ণাঙ্ক depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	বর্ণ *krunlevel = রিক্ত;
	ধ্রুব বর্ণ *workingdir = "/";
	বর্ণ *pidstr = রিক্ত;
	পূর্ণাঙ্ক opt;
	bool parallel;
	পূর্ণাঙ্ক regen = 0;
	bool nostop = false;
#ifdef __linux__
	বর্ণ *proc;
	বর্ণ *p;
	বর্ণ *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	যদি (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	যদি (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	যাবত্ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		বিকল্প (opt) {
		পক্ষ 'n':
			nostop = true;
			ভঙ্গ;
		পক্ষ 'o':
			যদি (*optarg == '\0')
				optarg = রিক্ত;
			যদি (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			যদি (rc_is_user())
				eerrorx("user services do not currently support --override");
			যদি (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		পক্ষ 's':
			newlevel = rc_service_resolve(optarg);
			যদি (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		পক্ষ 'S':
			systype = rc_sys();
			যদি (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	যদি (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	যদি (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	যদি (newlevel) {
		যদি (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		যদি (strcmp(newlevel, "reboot") == 0) {
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
	মুক্ত(pidstr);

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
	যদি (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		মুক্ত(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	যদি (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		যদি (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} অন্যথা {
		/* We should not use krunlevel in sysinit or boot runlevels */
		যদি (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			যদি (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(রিক্ত);
			}
		}

		যদি (newlevel) {
			যদি (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			যদি (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				যদি (p == রিক্ত)
					p = rc_proc_getent("softlevel");
				যদি (p != রিক্ত) {
					set_krunlevel(p);
					মুক্ত(p);
				}
			}
#endif
		}
	}

	যদি (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} অন্যথা {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	যদি (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		যদি (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	যদি ((main_deptree = _rc_deptree_load(0, &regen)) == রিক্ত) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	যদি (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	যদি (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	মুক্ত(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	মুক্ত(tmplist);
	যদি (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	যদি (main_stop_services) {
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
	যদি (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		মুক্ত(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		যদি (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			যদি (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				মুক্ত(tmplist);
			}
			যদি (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->মান);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	যদি (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	যদি (newlevel) {
		rc_runlevel_set(newlevel);
		মুক্ত(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	যদি (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	যদি (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->মান, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	যদি (proc) {
		যাবত্ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		মুক্ত(proc);
	}
#endif

	/* If we have a list of services to start then... */
	যদি (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->মান);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->মান, depoptions | RC_DEP_START);
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
	যদি (proc) {
		যাবত্ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		মুক্ত(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	যদি (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	প্রত্যাগম EXIT_SUCCESS;
}
