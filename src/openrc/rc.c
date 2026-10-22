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

#شامل <errno.h>
#شامل <dirent.h>
#شامل <getopt.h>
#شامل <libgen.h>
#شامل <pwd.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <signal.h>
#شامل <string.h>
#شامل <sys/ioctl.h>
#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <sys/utsname.h>
#شامل <sys/wait.h>
#شامل <termios.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "rc-logger.h"
#شامل "misc.h"
#شامل "plugin.h"
#شامل "version.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "no-stop", 0, پوچ, 'n' },
	{ "override",    1, پوچ, 'o' },
	{ "service",     1, پوچ, 's' },
	{ "sys",         0, پوچ, 'S' },
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ثابت نویسه *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

ثابت نویسه *applet = پوچ;
ایستا RC_STRINGLIST *main_hotplugged_services;
ایستا RC_STRINGLIST *main_stop_services;
ایستا RC_STRINGLIST *main_start_services;
ایستا RC_STRINGLIST *main_types_nw;
ایستا RC_STRINGLIST *main_types_nwua;
ایستا RC_DEPTREE *main_deptree;
ایستا نویسه *runlevel;

ساختار termios *termios_orig = پوچ;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ایستا تهی
clean_failed(تهی)
{
	DIR *dp;
	ساختار dirent *d;

	/* Clean the failed services state dir now */
	اگر (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		بازگشت;

	وقتی ((d = readdir(dp))) {
		اگر (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ادامه;

		اگر (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ایستا تهی
cleanup(تهی)
{
	صحیح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	اگر (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		اگر (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			آزاد(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		آزاد(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		آزاد(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	آزاد(runlevel);
}

ایستا نویسه
read_کلید(bool block)
{
	ساختار termios termios;
	نویسه c = 0;
	صحیح fd = STDIN_FILENO;

	اگر (!isatty(fd))
		بازگشت false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	اگر (!termios_orig) {
		termios_orig = xmalloc(اندازه(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	اگر (block)
		termios.c_cc[VMIN] = 1;
	وگرنه {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	اگر (اقرأ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	بازگشت c;
}

ایستا bool
want_interactive(تهی)
{
	نویسه c;
	ایستا bool gotinteractive;
	ایستا bool interactive;

	اگر (rc_yesno(getenv("EINFO_QUIET")))
		بازگشت false;
	اگر (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	اگر (!interactive)
		بازگشت false;
	c = read_کلید(false);
	بازگشت (c == 'I' || c == 'i') ? true : false;
}

ایستا تهی
mark_interactive(تهی)
{
	FILE *fp;

	اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ایستا تهی
run_program(ثابت نویسه *prog)
{
	ساختار sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, اندازه(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	اگر (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	اگر (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, پوچ);
		sigaction(SIGHUP, &sa, پوچ);
		sigaction(SIGINT, &sa, پوچ);
		sigaction(SIGQUIT, &sa, پوچ);
		sigaction(SIGTERM, &sa, پوچ);
		sigaction(SIGUSR1, &sa, پوچ);
		sigaction(SIGWINCH, &sa, پوچ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, پوچ);

		اگر (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (نویسه *)پوچ);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, پوچ);
	اگر (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ایستا تهی
open_shell(تهی)
{
	ثابت نویسه *shell;
	ساختار passwd *pw;

#ifdef __linux__
	ثابت نویسه *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	اگر (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (نویسه *) پوچ);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_مقدار("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	اگر (shell == پوچ) {
		shell = getenv("SHELL");
		اگر (shell == پوچ) {
			pw = getpwuid(getuid());
			اگر (pw)
				shell = pw->pw_shell;
			اگر (shell == پوچ)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ایستا bool
set_krunlevel(ثابت نویسه *level)
{
	FILE *fp;

	اگر (rc_is_user())
		بازگشت true;

	اگر (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		اگر (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		بازگشت false;
	}

	اگر (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		بازگشت false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	بازگشت true;
}

ایستا نویسه *get_krunlevel(تهی)
{
	نویسه *buffer = پوچ;
	FILE *fp;
	size_t i = 0;

	اگر (rc_is_user())
		بازگشت پوچ;

	اگر (access(RC_KRUNLEVEL, F_OK) != 0)
		بازگشت پوچ;
	اگر (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		بازگشت پوچ;
	}

	اگر (xgetline(&buffer, &i, fp) == -1) {
		آزاد(buffer);
		buffer = پوچ;
	}

	fclose(fp);
	بازگشت buffer;
}

ایستا تهی
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(اندازه(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_درج_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, پوچ);
}

ایستا تهی
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		اگر (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_درج_HEAD(&free_these_pids, p, entries);
			بشکن;
		}
	}
	/* only call free if we're not inside a signal handler */
	اگر (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			آزاد(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, پوچ);
}

ایستا تهی
wait_for_services(تهی)
{
	برای (;;) {
		وقتی (waitpid(0, 0, 0) != -1)
			;
		اگر (errno != EINTR)
			بشکن;
	}
}

ایستا تهی
handle_signal(صحیح sig)
{
	صحیح serrno = errno;
	ثابت نویسه *signame = پوچ;
	pid_t pid;
	RC_PID *pi;
	صحیح status = 0;
	ساختار winsize ws;
	sigset_t sset;

	انتخاب (sig) {
	حالت SIGCHLD:
		انجامبده {
			pid = waitpid(-1, &status, WNOHANG);
			اگر (pid < 0) {
				اگر (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				بازگشت;
			}
		} وقتی (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		اگر (pid > 0)
			remove_pid(pid, true);
		بشکن;

	حالت SIGWINCH:
		اگر (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		بشکن;

	حالت SIGINT:
		اگر (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالت SIGTERM:
		اگر (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالت SIGQUIT:
		اگر (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	حالت SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, پوچ);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, پوچ);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	پیشفرض:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ایستا تهی
do_sysinit(تهی)
{
	ساختار utsname uts;
	ثابت نویسه *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	اگر (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	اگر (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#وگرنه
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	اگر ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	اگر (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	اگر ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	اگر ((main_deptree = _rc_deptree_load(false, پوچ)) == پوچ)
		eerrorx("failed to load deptree");
}

ایستا bool
runlevel_config(ثابت نویسه *service, ثابت نویسه *level)
{
	نویسه *init = rc_service_resolve(service);
	نویسه *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	آزاد(conf);
	آزاد(init);
	بازگشت retval;
}

ایستا تهی
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 ثابت RC_STRINGLIST *stop_services, ثابت RC_DEPTREE *deptree,
				 ثابت نویسه *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = پوچ;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	اگر (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_مقدار("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->مقدار);
		اگر (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			ادامه;

		/* Sometimes we don't ever want to stop a service. */
		اگر (rc_stringlist_find(nostop, service->مقدار)) {
			rc_service_mark(service->مقدار, RC_SERVICE_FAILED);
			ادامه;
		}
		kwords = rc_deptree_depend(deptree, service->مقدار, "keyword");
		اگر (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		وگرنه
			nstop = false;
		rc_stringlist_free(kwords);
		اگر (nstop) {
			rc_service_mark(service->مقدار, RC_SERVICE_FAILED);
			ادامه;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		اگر (crashed &&
		    rc_service_daemons_crashed(service->مقدار))
			برو stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->مقدار);
		اگر (svc1) {
			اگر (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				اگر (!runlevel_config(service->مقدار,runlevel) &&
				    !runlevel_config(service->مقدار,newlevel))
					ادامه;
			}
			وگرنه
				ادامه;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		اگر (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->مقدار);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = پوچ;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->مقدار);
				اگر (svc2)
					بشکن;
			}
			rc_stringlist_free(deporder);

			اگر (svc2)
				ادامه;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->مقدار);
		اگر (pid > 0) {
			add_pid(pid);
			اگر (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	اگر (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ایستا تهی
do_start_services(ثابت RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	اگر (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	اگر (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->مقدار);
		اگر (state & RC_SERVICE_FAILED)
			ادامه;
		اگر (!(state & RC_SERVICE_STOPPED)) {
			اگر (crashed &&
			    rc_service_daemons_crashed(service->مقدار))
				rc_service_mark(service->مقدار,
				    RC_SERVICE_STOPPED);
			وگرنه
			    ادامه;
		}
		اگر (!interactive)
			interactive = want_interactive();

		اگر (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->مقدار);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			انتخاب (read_کلید(true)) {
			حالت '1': بشکن;
			حالت '2': ادامه;
			حالت '3': interactive = false; بشکن;
			حالت '4': open_shell(); برو interactive_retry;
			پیشفرض: برو interactive_option;
			}
		}

		pid = service_start(service->مقدار);
		اگر (pid == -1)
			بشکن;
		/* Remember the pid if we're running in parallel */
		اگر (pid > 0) {
			add_pid(pid);
			اگر (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	اگر (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	وگرنه
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN ایستا تهی
handle_bad_signal(صحیح sig)
{
	نویسه pid[10];
	صحیح status;
	pid_t crashed_pid = getpid();

	انتخاب (fork()) {
	حالت -1:
		_exit(sig);
		/* NOTREACHED */
	حالت 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", پوچ));
		/* NOTREACHED */
	پیشفرض:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

صحیح اصلی(صحیح argc, نویسه **argv)
{
	ثابت نویسه *bootlevel = پوچ;
	نویسه *newlevel = پوچ;
	ثابت نویسه *systype = پوچ;
	RC_STRINGLIST *deporder = پوچ;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	صحیح depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	نویسه *krunlevel = پوچ;
	ثابت نویسه *workingdir = "/";
	نویسه *pidstr = پوچ;
	صحیح opt;
	bool parallel;
	صحیح regen = 0;
	bool nostop = false;
#ifdef __linux__
	نویسه *proc;
	نویسه *p;
	نویسه *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	اگر (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	اگر (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'n':
			nostop = true;
			بشکن;
		حالت 'o':
			اگر (*optarg == '\0')
				optarg = پوچ;
			اگر (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			اگر (rc_is_user())
				eerrorx("user services do not currently support --override");
			اگر (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		حالت 's':
			newlevel = rc_service_resolve(optarg);
			اگر (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		حالت 'S':
			systype = rc_sys();
			اگر (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	اگر (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	اگر (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	اگر (newlevel) {
		اگر (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		اگر (strcmp(newlevel, "reboot") == 0) {
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
	آزاد(pidstr);

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
	اگر (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		آزاد(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	اگر (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		اگر (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} وگرنه {
		/* We should not use krunlevel in sysinit or boot runlevels */
		اگر (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			اگر (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(پوچ);
			}
		}

		اگر (newlevel) {
			اگر (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			اگر (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				اگر (p == پوچ)
					p = rc_proc_getent("softlevel");
				اگر (p != پوچ) {
					set_krunlevel(p);
					آزاد(p);
				}
			}
#endif
		}
	}

	اگر (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} وگرنه {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	اگر (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		اگر (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	اگر ((main_deptree = _rc_deptree_load(0, &regen)) == پوچ) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	اگر (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	اگر (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	آزاد(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	آزاد(tmplist);
	اگر (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	اگر (main_stop_services) {
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
	اگر (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		آزاد(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		اگر (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			اگر (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				آزاد(tmplist);
			}
			اگر (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->مقدار);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	اگر (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	اگر (newlevel) {
		rc_runlevel_set(newlevel);
		آزاد(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	اگر (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	اگر (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->مقدار, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	اگر (proc) {
		وقتی ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		آزاد(proc);
	}
#endif

	/* If we have a list of services to start then... */
	اگر (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->مقدار);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->مقدار, depoptions | RC_DEP_START);
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
	اگر (proc) {
		وقتی ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		آزاد(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	اگر (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	بازگشت EXIT_SUCCESS;
}
