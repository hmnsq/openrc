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

#ضمن <errno.h>
#ضمن <dirent.h>
#ضمن <getopt.h>
#ضمن <libgen.h>
#ضمن <pwd.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <signal.h>
#ضمن <string.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/stat.h>
#ضمن <sys/types.h>
#ضمن <sys/utsname.h>
#ضمن <sys/wait.h>
#ضمن <termios.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"
#ضمن "rc-logger.h"
#ضمن "misc.h"
#ضمن "plugin.h"
#ضمن "version.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "no-stop", 0, عدم, 'n' },
	{ "override",    1, عدم, 'o' },
	{ "service",     1, عدم, 's' },
	{ "sys",         0, عدم, 'S' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ثابت حرف *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

ثابت حرف *applet = عدم;
ساكن RC_STRINGLIST *main_hotplugged_services;
ساكن RC_STRINGLIST *main_stop_services;
ساكن RC_STRINGLIST *main_start_services;
ساكن RC_STRINGLIST *main_types_nw;
ساكن RC_STRINGLIST *main_types_nwua;
ساكن RC_DEPTREE *main_deptree;
ساكن حرف *runlevel;

بنية termios *termios_orig = عدم;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ساكن فراغ
clean_failed(فراغ)
{
	DIR *dp;
	بنية dirent *d;

	/* Clean the failed services state dir now */
	אם (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		החזר;

	כלעוד ((d = readdir(dp))) {
		אם (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			استمر;

		אם (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ساكن فراغ
cleanup(فراغ)
{
	صحيح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	אם (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		אם (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			حرر(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		حرر(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		حرر(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	حرر(runlevel);
}

ساكن حرف
read_key(bool block)
{
	بنية termios termios;
	حرف c = 0;
	صحيح fd = STDIN_FILENO;

	אם (!isatty(fd))
		החזר false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	אם (!termios_orig) {
		termios_orig = xmalloc(حجم(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	אם (block)
		termios.c_cc[VMIN] = 1;
	אחרת {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	אם (اقرأ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	החזר c;
}

ساكن bool
want_interactive(فراغ)
{
	حرف c;
	ساكن bool gotinteractive;
	ساكن bool interactive;

	אם (rc_yesno(getenv("EINFO_QUIET")))
		החזר false;
	אם (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	אם (!interactive)
		החזר false;
	c = read_key(false);
	החזר (c == 'I' || c == 'i') ? true : false;
}

ساكن فراغ
mark_interactive(فراغ)
{
	FILE *fp;

	אם ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ساكن فراغ
run_program(ثابت حرف *prog)
{
	بنية sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	אם (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	אם (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, عدم);
		sigaction(SIGHUP, &sa, عدم);
		sigaction(SIGINT, &sa, عدم);
		sigaction(SIGQUIT, &sa, عدم);
		sigaction(SIGTERM, &sa, عدم);
		sigaction(SIGUSR1, &sa, عدم);
		sigaction(SIGWINCH, &sa, عدم);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, عدم);

		אם (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (حرف *)عدم);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, عدم);
	אם (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ساكن فراغ
open_shell(فراغ)
{
	ثابت حرف *shell;
	بنية passwd *pw;

#ifdef __linux__
	ثابت حرف *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	אם (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (حرف *) عدم);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_value("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	אם (shell == عدم) {
		shell = getenv("SHELL");
		אם (shell == عدم) {
			pw = getpwuid(getuid());
			אם (pw)
				shell = pw->pw_shell;
			אם (shell == عدم)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ساكن bool
set_krunlevel(ثابت حرف *level)
{
	FILE *fp;

	אם (rc_is_user())
		החזר true;

	אם (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		אם (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		החזר false;
	}

	אם (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		החזר false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	החזר true;
}

ساكن حرف *get_krunlevel(فراغ)
{
	حرف *buffer = عدم;
	FILE *fp;
	size_t i = 0;

	אם (rc_is_user())
		החזר عدم;

	אם (access(RC_KRUNLEVEL, F_OK) != 0)
		החזר عدم;
	אם (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		החזר عدم;
	}

	אם (xgetline(&buffer, &i, fp) == -1) {
		حرر(buffer);
		buffer = عدم;
	}

	fclose(fp);
	החזר buffer;
}

ساكن فراغ
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(حجم(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_INSERT_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, عدم);
}

ساكن فراغ
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		אם (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_INSERT_HEAD(&free_these_pids, p, entries);
			اكسر;
		}
	}
	/* only call free if we're not inside a signal handler */
	אם (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			حرر(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, عدم);
}

ساكن فراغ
wait_for_services(فراغ)
{
	עבור (;;) {
		כלעוד (waitpid(0, 0, 0) != -1)
			;
		אם (errno != EINTR)
			اكسر;
	}
}

ساكن فراغ
handle_signal(صحيح sig)
{
	صحيح serrno = errno;
	ثابت حرف *signame = عدم;
	pid_t pid;
	RC_PID *pi;
	صحيح status = 0;
	بنية winsize ws;
	sigset_t sset;

	بدل (sig) {
	حالة SIGCHLD:
		עשה {
			pid = waitpid(-1, &status, WNOHANG);
			אם (pid < 0) {
				אם (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				החזר;
			}
		} כלעוד (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		אם (pid > 0)
			remove_pid(pid, true);
		اكسر;

	حالة SIGWINCH:
		אם (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		اكسر;

	حالة SIGINT:
		אם (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالة SIGTERM:
		אם (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالة SIGQUIT:
		אם (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	حالة SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, عدم);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, عدم);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	افتراضي:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ساكن فراغ
do_sysinit(فراغ)
{
	بنية utsname uts;
	ثابت حرف *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	אם (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	אם (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#אחרת
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	אם ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	אם (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	אם ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	אם ((main_deptree = _rc_deptree_load(false, عدم)) == عدم)
		eerrorx("failed to load deptree");
}

ساكن bool
runlevel_config(ثابت حرف *service, ثابت حرف *level)
{
	حرف *init = rc_service_resolve(service);
	حرف *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	حرر(conf);
	حرر(init);
	החזר retval;
}

ساكن فراغ
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 ثابت RC_STRINGLIST *stop_services, ثابت RC_DEPTREE *deptree,
				 ثابت حرف *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = عدم;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	אם (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_value("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->value);
		אם (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			استمر;

		/* Sometimes we don't ever want to stop a service. */
		אם (rc_stringlist_find(nostop, service->value)) {
			rc_service_mark(service->value, RC_SERVICE_FAILED);
			استمر;
		}
		kwords = rc_deptree_depend(deptree, service->value, "keyword");
		אם (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		אחרת
			nstop = false;
		rc_stringlist_free(kwords);
		אם (nstop) {
			rc_service_mark(service->value, RC_SERVICE_FAILED);
			استمر;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		אם (crashed &&
		    rc_service_daemons_crashed(service->value))
			اذهب stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->value);
		אם (svc1) {
			אם (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				אם (!runlevel_config(service->value,runlevel) &&
				    !runlevel_config(service->value,newlevel))
					استمر;
			}
			אחרת
				استمر;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		אם (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->value);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = عدم;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->value);
				אם (svc2)
					اكسر;
			}
			rc_stringlist_free(deporder);

			אם (svc2)
				استمر;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->value);
		אם (pid > 0) {
			add_pid(pid);
			אם (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	אם (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ساكن فراغ
do_start_services(ثابت RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	אם (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	אם (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->value);
		אם (state & RC_SERVICE_FAILED)
			استمر;
		אם (!(state & RC_SERVICE_STOPPED)) {
			אם (crashed &&
			    rc_service_daemons_crashed(service->value))
				rc_service_mark(service->value,
				    RC_SERVICE_STOPPED);
			אחרת
			    استمر;
		}
		אם (!interactive)
			interactive = want_interactive();

		אם (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->value);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			بدل (read_key(true)) {
			حالة '1': اكسر;
			حالة '2': استمر;
			حالة '3': interactive = false; اكسر;
			حالة '4': open_shell(); اذهب interactive_retry;
			افتراضي: اذهب interactive_option;
			}
		}

		pid = service_start(service->value);
		אם (pid == -1)
			اكسر;
		/* Remember the pid if we're running in parallel */
		אם (pid > 0) {
			add_pid(pid);
			אם (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	אם (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	אחרת
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN ساكن فراغ
handle_bad_signal(صحيح sig)
{
	حرف pid[10];
	صحيح status;
	pid_t crashed_pid = getpid();

	بدل (fork()) {
	حالة -1:
		_exit(sig);
		/* NOTREACHED */
	حالة 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", عدم));
		/* NOTREACHED */
	افتراضي:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

صحيح main(صحيح argc, حرف **argv)
{
	ثابت حرف *bootlevel = عدم;
	حرف *newlevel = عدم;
	ثابت حرف *systype = عدم;
	RC_STRINGLIST *deporder = عدم;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	صحيح depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	حرف *krunlevel = عدم;
	ثابت حرف *workingdir = "/";
	حرف *pidstr = عدم;
	صحيح opt;
	bool parallel;
	صحيح regen = 0;
	bool nostop = false;
#ifdef __linux__
	حرف *proc;
	حرف *p;
	حرف *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	אם (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	אם (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	כלעוד ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'n':
			nostop = true;
			اكسر;
		حالة 'o':
			אם (*optarg == '\0')
				optarg = عدم;
			אם (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			אם (rc_is_user())
				eerrorx("user services do not currently support --override");
			אם (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		حالة 's':
			newlevel = rc_service_resolve(optarg);
			אם (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		حالة 'S':
			systype = rc_sys();
			אם (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	אם (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	אם (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	אם (newlevel) {
		אם (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		אם (strcmp(newlevel, "reboot") == 0) {
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
	حرر(pidstr);

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
	אם (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		حرر(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	אם (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		אם (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} אחרת {
		/* We should not use krunlevel in sysinit or boot runlevels */
		אם (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			אם (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(عدم);
			}
		}

		אם (newlevel) {
			אם (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			אם (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				אם (p == عدم)
					p = rc_proc_getent("softlevel");
				אם (p != عدم) {
					set_krunlevel(p);
					حرر(p);
				}
			}
#endif
		}
	}

	אם (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} אחרת {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	אם (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		אם (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	אם ((main_deptree = _rc_deptree_load(0, &regen)) == عدم) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	אם (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	אם (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	حرر(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	حرر(tmplist);
	אם (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	אם (main_stop_services) {
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
	אם (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		حرر(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		אם (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			אם (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				حرر(tmplist);
			}
			אם (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->value);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	אם (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	אם (newlevel) {
		rc_runlevel_set(newlevel);
		حرر(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	אם (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	אם (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->value, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	אם (proc) {
		כלעוד ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		حرر(proc);
	}
#endif

	/* If we have a list of services to start then... */
	אם (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->value);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->value, depoptions | RC_DEP_START);
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
	אם (proc) {
		כלעוד ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		حرر(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	אם (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	החזר EXIT_SUCCESS;
}
