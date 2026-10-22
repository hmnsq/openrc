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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <libgen.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/utsname.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <termios.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "rc-logger.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "plugin.h"
#அந்தர்பாவ "version.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "a:no:s:S" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "no-stop", 0, வெற்று, 'n' },
	{ "override",    1, வெற்று, 'o' },
	{ "service",     1, வெற்று, 's' },
	{ "sys",         0, வெற்று, 'S' },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

மாறிலி எழுத்து *applet = வெற்று;
நிலையான RC_STRINGLIST *main_hotplugged_services;
நிலையான RC_STRINGLIST *main_stop_services;
நிலையான RC_STRINGLIST *main_start_services;
நிலையான RC_STRINGLIST *main_types_nw;
நிலையான RC_STRINGLIST *main_types_nwua;
நிலையான RC_DEPTREE *main_deptree;
நிலையான எழுத்து *runlevel;

கட்டமைப்பு termios *termios_orig = வெற்று;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

நிலையான வெறுமை
clean_failed(வெறுமை)
{
	DIR *dp;
	கட்டமைப்பு dirent *d;

	/* Clean the failed services state dir now */
	எனில் (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		திரும்பு;

	போது ((d = readdir(dp))) {
		எனில் (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			தொடர்;

		எனில் (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

நிலையான வெறுமை
cleanup(வெறுமை)
{
	முழு svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	எனில் (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		எனில் (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			விடுவி(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		விடுவி(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		விடுவி(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	விடுவி(runlevel);
}

நிலையான எழுத்து
read_சாவி(bool block)
{
	கட்டமைப்பு termios termios;
	எழுத்து c = 0;
	முழு fd = STDIN_FILENO;

	எனில் (!isatty(fd))
		திரும்பு false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	எனில் (!termios_orig) {
		termios_orig = xmalloc(அளவு(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	எனில் (block)
		termios.c_cc[VMIN] = 1;
	இல்லையெனில் {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	எனில் (பட(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	திரும்பு c;
}

நிலையான bool
want_interactive(வெறுமை)
{
	எழுத்து c;
	நிலையான bool gotinteractive;
	நிலையான bool interactive;

	எனில் (rc_yesno(getenv("EINFO_QUIET")))
		திரும்பு false;
	எனில் (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	எனில் (!interactive)
		திரும்பு false;
	c = read_சாவி(false);
	திரும்பு (c == 'I' || c == 'i') ? true : false;
}

நிலையான வெறுமை
mark_interactive(வெறுமை)
{
	FILE *fp;

	எனில் ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

நிலையான வெறுமை
run_program(மாறிலி எழுத்து *prog)
{
	கட்டமைப்பு sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, அளவு(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	எனில் (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	எனில் (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, வெற்று);
		sigaction(SIGHUP, &sa, வெற்று);
		sigaction(SIGINT, &sa, வெற்று);
		sigaction(SIGQUIT, &sa, வெற்று);
		sigaction(SIGTERM, &sa, வெற்று);
		sigaction(SIGUSR1, &sa, வெற்று);
		sigaction(SIGWINCH, &sa, வெற்று);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, வெற்று);

		எனில் (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (எழுத்து *)வெற்று);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, வெற்று);
	எனில் (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

நிலையான வெறுமை
open_shell(வெறுமை)
{
	மாறிலி எழுத்து *shell;
	கட்டமைப்பு passwd *pw;

#ifdef __linux__
	மாறிலி எழுத்து *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	எனில் (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (எழுத்து *) வெற்று);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_மதிப்பு("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	எனில் (shell == வெற்று) {
		shell = getenv("SHELL");
		எனில் (shell == வெற்று) {
			pw = getpwuid(getuid());
			எனில் (pw)
				shell = pw->pw_shell;
			எனில் (shell == வெற்று)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

நிலையான bool
set_krunlevel(மாறிலி எழுத்து *level)
{
	FILE *fp;

	எனில் (rc_is_user())
		திரும்பு true;

	எனில் (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		எனில் (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		திரும்பு false;
	}

	எனில் (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		திரும்பு false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	திரும்பு true;
}

நிலையான எழுத்து *get_krunlevel(வெறுமை)
{
	எழுத்து *buffer = வெற்று;
	FILE *fp;
	size_t i = 0;

	எனில் (rc_is_user())
		திரும்பு வெற்று;

	எனில் (access(RC_KRUNLEVEL, F_OK) != 0)
		திரும்பு வெற்று;
	எனில் (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		திரும்பு வெற்று;
	}

	எனில் (xgetline(&buffer, &i, fp) == -1) {
		விடுவி(buffer);
		buffer = வெற்று;
	}

	fclose(fp);
	திரும்பு buffer;
}

நிலையான வெறுமை
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(அளவு(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_செருகு_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, வெற்று);
}

நிலையான வெறுமை
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		எனில் (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_செருகு_HEAD(&free_these_pids, p, entries);
			நிறுத்து;
		}
	}
	/* only call free if we're not inside a signal handler */
	எனில் (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			விடுவி(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, வெற்று);
}

நிலையான வெறுமை
wait_for_services(வெறுமை)
{
	க்கு (;;) {
		போது (waitpid(0, 0, 0) != -1)
			;
		எனில் (errno != EINTR)
			நிறுத்து;
	}
}

நிலையான வெறுமை
handle_signal(முழு sig)
{
	முழு serrno = errno;
	மாறிலி எழுத்து *signame = வெற்று;
	pid_t pid;
	RC_PID *pi;
	முழு status = 0;
	கட்டமைப்பு winsize ws;
	sigset_t sset;

	தேர்வு (sig) {
	நிலை SIGCHLD:
		செய் {
			pid = waitpid(-1, &status, WNOHANG);
			எனில் (pid < 0) {
				எனில் (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				திரும்பு;
			}
		} போது (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		எனில் (pid > 0)
			remove_pid(pid, true);
		நிறுத்து;

	நிலை SIGWINCH:
		எனில் (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		நிறுத்து;

	நிலை SIGINT:
		எனில் (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	நிலை SIGTERM:
		எனில் (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	நிலை SIGQUIT:
		எனில் (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	நிலை SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, வெற்று);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, வெற்று);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	இயல்பு:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

நிலையான வெறுமை
do_sysinit(வெறுமை)
{
	கட்டமைப்பு utsname uts;
	மாறிலி எழுத்து *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	எனில் (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	எனில் (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#இல்லையெனில்
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	எனில் ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	எனில் (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	எனில் ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	எனில் ((main_deptree = _rc_deptree_load(false, வெற்று)) == வெற்று)
		eerrorx("failed to load deptree");
}

நிலையான bool
runlevel_config(மாறிலி எழுத்து *service, மாறிலி எழுத்து *level)
{
	எழுத்து *init = rc_service_resolve(service);
	எழுத்து *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	விடுவி(conf);
	விடுவி(init);
	திரும்பு retval;
}

நிலையான வெறுமை
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 மாறிலி RC_STRINGLIST *stop_services, மாறிலி RC_DEPTREE *deptree,
				 மாறிலி எழுத்து *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = வெற்று;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	எனில் (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_மதிப்பு("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->மதிப்பு);
		எனில் (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			தொடர்;

		/* Sometimes we don't ever want to stop a service. */
		எனில் (rc_stringlist_find(nostop, service->மதிப்பு)) {
			rc_service_mark(service->மதிப்பு, RC_SERVICE_FAILED);
			தொடர்;
		}
		kwords = rc_deptree_depend(deptree, service->மதிப்பு, "keyword");
		எனில் (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		இல்லையெனில்
			nstop = false;
		rc_stringlist_free(kwords);
		எனில் (nstop) {
			rc_service_mark(service->மதிப்பு, RC_SERVICE_FAILED);
			தொடர்;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		எனில் (crashed &&
		    rc_service_daemons_crashed(service->மதிப்பு))
			செல் stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->மதிப்பு);
		எனில் (svc1) {
			எனில் (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				எனில் (!runlevel_config(service->மதிப்பு,runlevel) &&
				    !runlevel_config(service->மதிப்பு,newlevel))
					தொடர்;
			}
			இல்லையெனில்
				தொடர்;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		எனில் (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->மதிப்பு);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = வெற்று;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->மதிப்பு);
				எனில் (svc2)
					நிறுத்து;
			}
			rc_stringlist_free(deporder);

			எனில் (svc2)
				தொடர்;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->மதிப்பு);
		எனில் (pid > 0) {
			add_pid(pid);
			எனில் (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	எனில் (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

நிலையான வெறுமை
do_start_services(மாறிலி RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	எனில் (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	எனில் (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->மதிப்பு);
		எனில் (state & RC_SERVICE_FAILED)
			தொடர்;
		எனில் (!(state & RC_SERVICE_STOPPED)) {
			எனில் (crashed &&
			    rc_service_daemons_crashed(service->மதிப்பு))
				rc_service_mark(service->மதிப்பு,
				    RC_SERVICE_STOPPED);
			இல்லையெனில்
			    தொடர்;
		}
		எனில் (!interactive)
			interactive = want_interactive();

		எனில் (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->மதிப்பு);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			தேர்வு (read_சாவி(true)) {
			நிலை '1': நிறுத்து;
			நிலை '2': தொடர்;
			நிலை '3': interactive = false; நிறுத்து;
			நிலை '4': open_shell(); செல் interactive_retry;
			இயல்பு: செல் interactive_option;
			}
		}

		pid = service_start(service->மதிப்பு);
		எனில் (pid == -1)
			நிறுத்து;
		/* Remember the pid if we're running in parallel */
		எனில் (pid > 0) {
			add_pid(pid);
			எனில் (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	எனில் (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	இல்லையெனில்
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN நிலையான வெறுமை
handle_bad_signal(முழு sig)
{
	எழுத்து pid[10];
	முழு status;
	pid_t crashed_pid = getpid();

	தேர்வு (fork()) {
	நிலை -1:
		_exit(sig);
		/* NOTREACHED */
	நிலை 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", வெற்று));
		/* NOTREACHED */
	இயல்பு:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	மாறிலி எழுத்து *bootlevel = வெற்று;
	எழுத்து *newlevel = வெற்று;
	மாறிலி எழுத்து *systype = வெற்று;
	RC_STRINGLIST *deporder = வெற்று;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	முழு depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	எழுத்து *krunlevel = வெற்று;
	மாறிலி எழுத்து *workingdir = "/";
	எழுத்து *pidstr = வெற்று;
	முழு opt;
	bool parallel;
	முழு regen = 0;
	bool nostop = false;
#ifdef __linux__
	எழுத்து *proc;
	எழுத்து *p;
	எழுத்து *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	எனில் (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	எனில் (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை 'n':
			nostop = true;
			நிறுத்து;
		நிலை 'o':
			எனில் (*optarg == '\0')
				optarg = வெற்று;
			எனில் (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			எனில் (rc_is_user())
				eerrorx("user services do not currently support --override");
			எனில் (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		நிலை 's':
			newlevel = rc_service_resolve(optarg);
			எனில் (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		நிலை 'S':
			systype = rc_sys();
			எனில் (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	எனில் (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	எனில் (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	எனில் (newlevel) {
		எனில் (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		எனில் (strcmp(newlevel, "reboot") == 0) {
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
	விடுவி(pidstr);

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
	எனில் (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		விடுவி(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	எனில் (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		எனில் (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} இல்லையெனில் {
		/* We should not use krunlevel in sysinit or boot runlevels */
		எனில் (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			எனில் (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(வெற்று);
			}
		}

		எனில் (newlevel) {
			எனில் (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			எனில் (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				எனில் (p == வெற்று)
					p = rc_proc_getent("softlevel");
				எனில் (p != வெற்று) {
					set_krunlevel(p);
					விடுவி(p);
				}
			}
#endif
		}
	}

	எனில் (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} இல்லையெனில் {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	எனில் (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		எனில் (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	எனில் ((main_deptree = _rc_deptree_load(0, &regen)) == வெற்று) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	எனில் (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	எனில் (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	விடுவி(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	விடுவி(tmplist);
	எனில் (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	எனில் (main_stop_services) {
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
	எனில் (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		விடுவி(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		எனில் (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			எனில் (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				விடுவி(tmplist);
			}
			எனில் (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->மதிப்பு);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	எனில் (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	எனில் (newlevel) {
		rc_runlevel_set(newlevel);
		விடுவி(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	எனில் (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	எனில் (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->மதிப்பு, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	எனில் (proc) {
		போது ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		விடுவி(proc);
	}
#endif

	/* If we have a list of services to start then... */
	எனில் (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->மதிப்பு);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->மதிப்பு, depoptions | RC_DEP_START);
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
	எனில் (proc) {
		போது ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		விடுவி(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	எனில் (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	திரும்பு EXIT_SUCCESS;
}
