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

ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "no-stop", 0, تہی, 'n' },
	{ "override",    1, تہی, 'o' },
	{ "service",     1, تہی, 's' },
	{ "sys",         0, تہی, 'S' },
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

#تعریف_کرو INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#تعریف_کرو INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#تعریف_کرو DEVBOOT			"/dev/.rcboot"

ثابت حرف *applet = تہی;
ساکن RC_STRINGLIST *main_hotplugged_services;
ساکن RC_STRINGLIST *main_stop_services;
ساکن RC_STRINGLIST *main_آغاز_services;
ساکن RC_STRINGLIST *main_types_nw;
ساکن RC_STRINGLIST *main_types_nwua;
ساکن RC_DEPTREE *main_deptree;
ساکن حرف *runlevel;

ساخت termios *termios_orig = تہی;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ساکن خالی
clean_failed(خالی)
{
	DIR *dp;
	ساخت dirent *d;

	/* Clean the failed services state dir now */
	اگر (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		واپس;

	جب ((d = readdir(dp))) {
		اگر (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			جاری;

		اگر (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ساکن خالی
cleanup(خالی)
{
	صحیح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	اگر (!rc_اندر_logger && !rc_اندر_plugin &&
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

	LIST_FOREACH_محفوظ(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		آزاد(p);
	}
	LIST_FOREACH_محفوظ(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		آزاد(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_آغاز_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	آزاد(runlevel);
}

ساکن حرف
read_کلید(bool بلاک)
{
	ساخت termios termios;
	حرف c = 0;
	صحیح fd = STDIN_FILENO;

	اگر (!isatty(fd))
		واپس false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	اگر (!termios_orig) {
		termios_orig = xmalloc(حجم(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	اگر (بلاک)
		termios.c_cc[VMIN] = 1;
	ورنہ {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	اگر (اقرأ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	واپس c;
}

ساکن bool
want_interactive(خالی)
{
	حرف c;
	ساکن bool gotinteractive;
	ساکن bool interactive;

	اگر (rc_yesno(getenv("EINFO_QUIET")))
		واپس false;
	اگر (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	اگر (!interactive)
		واپس false;
	c = read_کلید(false);
	واپس (c == 'I' || c == 'i') ? true : false;
}

ساکن خالی
mark_interactive(خالی)
{
	FILE *fp;

	اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ساکن خالی
تسلسل_پروگرام(ثابت حرف *prog)
{
	ساخت sigaction sa;
	sigset_t بھرا;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&بھرا);
	sigprocmask(SIG_SETMASK, &بھرا, &old);
	pid = fork();

	اگر (pid == -1) {
		rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_آغاز_برآمد, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	اگر (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, تہی);
		sigaction(SIGHUP, &sa, تہی);
		sigaction(SIGINT, &sa, تہی);
		sigaction(SIGQUIT, &sa, تہی);
		sigaction(SIGTERM, &sa, تہی);
		sigaction(SIGUSR1, &sa, تہی);
		sigaction(SIGWINCH, &sa, تہی);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, تہی);

		اگر (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (حرف *)تہی);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, تہی);
	اگر (rc_waitpid(pid) == -1) {
		rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_آغاز_برآمد, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ساکن خالی
open_shell(خالی)
{
	ثابت حرف *shell;
	ساخت passwd *pw;

#اگر_تعریف_شدہ __linux__
	ثابت حرف *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	اگر (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (حرف *) تہی);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#اگر_ختم

	shell = rc_conf_قدر("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	اگر (shell == تہی) {
		shell = getenv("SHELL");
		اگر (shell == تہی) {
			pw = getpwuid(getuid());
			اگر (pw)
				shell = pw->pw_shell;
			اگر (shell == تہی)
				shell = "/bin/sh";
		}
	}
	تسلسل_پروگرام(shell);
}

ساکن bool
سیٹ_krunlevel(ثابت حرف *level)
{
	FILE *fp;

	اگر (rc_is_user())
		واپس true;

	اگر (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		اگر (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		واپس false;
	}

	اگر (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		واپس false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	واپس true;
}

ساکن حرف *get_krunlevel(خالی)
{
	حرف *بفر = تہی;
	FILE *fp;
	size_t i = 0;

	اگر (rc_is_user())
		واپس تہی;

	اگر (access(RC_KRUNLEVEL, F_OK) != 0)
		واپس تہی;
	اگر (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		واپس تہی;
	}

	اگر (xgetline(&بفر, &i, fp) == -1) {
		آزاد(بفر);
		بفر = تہی;
	}

	fclose(fp);
	واپس بفر;
}

ساکن خالی
جوڑو_pid(pid_t pid)
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
	LIST_داخل_کرو_سر(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, تہی);
}

ساکن خالی
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
			LIST_داخل_کرو_سر(&free_these_pids, p, entries);
			توڑو;
		}
	}
	/* only call free if we're not inside a signal handler */
	اگر (!inside_signal) {
		LIST_FOREACH_محفوظ(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			آزاد(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, تہی);
}

ساکن خالی
wait_for_services(خالی)
{
	برائے (;;) {
		جب (waitpid(0, 0, 0) != -1)
			;
		اگر (errno != EINTR)
			توڑو;
	}
}

ساکن خالی
handle_signal(صحیح sig)
{
	صحیح serrno = errno;
	ثابت حرف *signame = تہی;
	pid_t pid;
	RC_PID *pi;
	صحیح حالت_2 = 0;
	ساخت winsize ws;
	sigset_t sset;

	انتخاب (sig) {
	حالت SIGCHLD:
		کرو {
			pid = waitpid(-1, &حالت_2, WNOHANG);
			اگر (pid < 0) {
				اگر (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				واپس;
			}
		} جب (!WIFEXITED(حالت_2) && !WIFSIGNALED(حالت_2));

		/* Remove that pid from our list */
		اگر (pid > 0)
			remove_pid(pid, true);
		توڑو;

	حالت SIGWINCH:
		اگر (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		توڑو;

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
		sigprocmask(SIG_BLOCK, &sset, تہی);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_تسلسل(RC_HOOK_ABORT, تہی);

		باہر_نکلو(EXIT_FAILURE);
		/* NOTREACHED */

	طےشدہ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ساکن خالی
do_sysinit(خالی)
{
	ساخت utsname uts;
	ثابت حرف *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	اگر (access(INITEARLYSH, F_OK) == 0)
		تسلسل_پروگرام(INITEARLYSH);

	uname(&uts);
	اگر (!rc_yesno(getenv("EINFO_QUIET"))) {
		لکھو("\n   %sOpenRC %s" نسخہ "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_معمول), ecolor(ECOLOR_BRACKET));
#اگر_تعریف_شدہ BRANDING
		لکھو(BRANDING " (%s)", uts.machine);
#ورنہ
		لکھو("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#اگر_ختم
	}

	اگر ((sys = rc_sys()))
		لکھو(" [%s]", sys);

	لکھو("%s\n\n", ecolor(ECOLOR_معمول));

	اگر (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		لکھو("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_معمول));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	تسلسل_پروگرام(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	اگر ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	اگر ((main_deptree = rc_deptree_بھرو_2(false, تہی)) == تہی)
		eerrorx("failed to load deptree");
}

ساکن bool
runlevel_config(ثابت حرف *service, ثابت حرف *level)
{
	حرف *init = rc_service_resolve(service);
	حرف *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	آزاد(conf);
	آزاد(init);
	واپس retval;
}

ساکن خالی
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *آغاز_services,
				 ثابت RC_STRINGLIST *stop_services, ثابت RC_DEPTREE *deptree,
				 ثابت حرف *newlevel, bool parallel, bool going_نیچے_کو)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = تہی;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	اگر (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_جوڑو(types_nw, "needsme");
		rc_stringlist_جوڑو(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_بانٹو(rc_conf_قدر("rc_nostop"), " ");
	TAILQ_FOREACH_الٹ(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->قدر);
		اگر (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			جاری;

		/* Sometimes we don't ever want to stop a service. */
		اگر (rc_stringlist_ڈھونڈو(nostop, service->قدر)) {
			rc_service_mark(service->قدر, RC_SERVICE_FAILED);
			جاری;
		}
		kwords = rc_deptree_depend(deptree, service->قدر, "keyword");
		اگر (rc_stringlist_ڈھونڈو(kwords, "-stop") ||
		    rc_stringlist_ڈھونڈو(kwords, "nostop") ||
		    (going_نیچے_کو &&
			(rc_stringlist_ڈھونڈو(kwords, "-shutdown") ||
			    rc_stringlist_ڈھونڈو(kwords, "noshutdown"))))
			nstop = true;
		ورنہ
			nstop = false;
		rc_stringlist_free(kwords);
		اگر (nstop) {
			rc_service_mark(service->قدر, RC_SERVICE_FAILED);
			جاری;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		اگر (crashed &&
		    rc_service_daemons_crashed(service->قدر))
			جاؤ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_ڈھونڈو(آغاز_services, service->قدر);
		اگر (svc1) {
			اگر (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				اگر (!runlevel_config(service->قدر,runlevel) &&
				    !runlevel_config(service->قدر,newlevel))
					جاری;
			}
			ورنہ
				جاری;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		اگر (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_جوڑو(tmplist, service->قدر);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = تہی;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_ڈھونڈو(آغاز_services,
				    svc1->قدر);
				اگر (svc2)
					توڑو;
			}
			rc_stringlist_free(deporder);

			اگر (svc2)
				جاری;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->قدر);
		اگر (pid > 0) {
			جوڑو_pid(pid);
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

ساکن خالی
do_آغاز_services(ثابت RC_STRINGLIST *آغاز_services, bool parallel)
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

	TAILQ_FOREACH(service, آغاز_services, entries) {
		state = rc_service_state(service->قدر);
		اگر (state & RC_SERVICE_FAILED)
			جاری;
		اگر (!(state & RC_SERVICE_STOPPED)) {
			اگر (crashed &&
			    rc_service_daemons_crashed(service->قدر))
				rc_service_mark(service->قدر,
				    RC_SERVICE_STOPPED);
			ورنہ
			    جاری;
		}
		اگر (!interactive)
			interactive = want_interactive();

		اگر (interactive) {
	interactive_retry:
			لکھو("\n");
			einfo("About to start the service %s",
			    service->قدر);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			انتخاب (read_کلید(true)) {
			حالت '1': توڑو;
			حالت '2': جاری;
			حالت '3': interactive = false; توڑو;
			حالت '4': open_shell(); جاؤ interactive_retry;
			طےشدہ: جاؤ interactive_option;
			}
		}

		pid = service_آغاز(service->قدر);
		اگر (pid == -1)
			توڑو;
		/* Remember the pid if we're running in parallel */
		اگر (pid > 0) {
			جوڑو_pid(pid);
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
	ورنہ
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#اگر_تعریف_شدہ RC_DEBUG
RC_NORETURN ساکن خالی
handle_bad_signal(صحیح sig)
{
	حرف pid[10];
	صحیح حالت_2;
	pid_t crashed_pid = getpid();

	انتخاب (fork()) {
	حالت -1:
		_exit(sig);
		/* NOTREACHED */
	حالت 0:
		sprintf(pid, "%i", crashed_pid);
		لکھو("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", تہی));
		/* NOTREACHED */
	طےشدہ:
		wait(&حالت_2);
	}
	_exit(1);
	/* NOTREACHED */
}
#اگر_ختم

صحیح مین(صحیح argc, حرف **argv)
{
	ثابت حرف *bootlevel = تہی;
	حرف *newlevel = تہی;
	ثابت حرف *systype = تہی;
	RC_STRINGLIST *deporder = تہی;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_نیچے_کو = false;
	صحیح depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	حرف *krunlevel = تہی;
	ثابت حرف *workingdir = "/";
	حرف *pidstr = تہی;
	صحیح opt;
	bool parallel;
	صحیح regen = 0;
	bool nostop = false;
#اگر_تعریف_شدہ __linux__
	حرف *proc;
	حرف *p;
	حرف *token;
#اگر_ختم

#اگر_تعریف_شدہ RC_DEBUG
	signal_تیاری(SIGBUS, handle_bad_signal);
	signal_تیاری(SIGILL, handle_bad_signal);
	signal_تیاری(SIGSEGV, handle_bad_signal);
#اگر_ختم

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
	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'n':
			nostop = true;
			توڑو;
		حالت 'o':
			اگر (*optarg == '\0')
				optarg = تہی;
			اگر (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				باہر_نکلو(EXIT_FAILURE);
			}
			اگر (rc_is_user())
				eerrorx("user services do not currently support --override");
			اگر (!سیٹ_krunlevel(optarg))
				باہر_نکلو(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			باہر_نکلو(EXIT_SUCCESS);
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
				لکھو("%s\n", systype);
			باہر_نکلو(EXIT_SUCCESS);
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
	signal_تیاری(SIGINT, handle_signal);
	signal_تیاری(SIGQUIT, handle_signal);
	signal_تیاری(SIGTERM, handle_signal);
	signal_تیاری(SIGUSR1, handle_signal);
	signal_تیاری(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	اگر (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		آزاد(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_بھرو();

	/* Now we start handling our children */
	signal_تیاری(SIGCHLD, handle_signal);

	اگر (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_نیچے_کو = true;
		اگر (access(RC_KRUNLEVEL, F_OK) != 0)
			سیٹ_krunlevel(runlevel);
		rc_runlevel_سیٹ(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} ورنہ {
		/* We should not use krunlevel in sysinit or boot runlevels */
		اگر (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			اگر (krunlevel) {
				newlevel = krunlevel;
				سیٹ_krunlevel(تہی);
			}
		}

		اگر (newlevel) {
			اگر (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#اگر_تعریف_شدہ __linux__
			اگر (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				اگر (p == تہی)
					p = rc_proc_getent("softlevel");
				اگر (p != تہی) {
					سیٹ_krunlevel(p);
					آزاد(p);
				}
			}
#اگر_ختم
		}
	}

	اگر (going_نیچے_کو) {
#اگر_تعریف_شدہ __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#اگر_ختم

		rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_STOP_اندر, newlevel);
	} ورنہ {
		rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_STOP_اندر, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	اگر (newlevel && strcmp(runlevel, newlevel) != 0 && !going_نیچے_کو) {
		اگر (!rc_runlevel_exists(newlevel)) {
			rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_STOP_برآمد, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	اگر ((main_deptree = rc_deptree_بھرو_2(0, &regen)) == تہی) {
		rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_STOP_برآمد, runlevel);
		eerrorx("failed to load deptree");
	}

	اگر (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	اگر (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_STOP_برآمد, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), strerror(errno));
	}

	/* Create a list of all services which we could stop (assuming
	* they won't be active in the new or current runlevel) including
	* all those services which have been started, are inactive or
	* are currently starting.  Clearly, some of these will be listed
	* in the new or current runlevel so we won't actually be stopping
	* them all.
	*/
	main_stop_services = rc_services_اندر_state(RC_SERVICE_STARTED);
	tmplist = rc_services_اندر_state(RC_SERVICE_INACTIVE);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	آزاد(tmplist);
	tmplist = rc_services_اندر_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	آزاد(tmplist);
	اگر (main_stop_services)
		rc_stringlist_ترتیب_دو(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_جوڑو(main_types_nwua, "ineed");
	rc_stringlist_جوڑو(main_types_nwua, "iwant");
	rc_stringlist_جوڑو(main_types_nwua, "iuse");
	rc_stringlist_جوڑو(main_types_nwua, "iafter");

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
	main_hotplugged_services = rc_services_اندر_state(RC_SERVICE_HOTPLUGGED);
	main_آغاز_services = rc_services_اندر_runlevel_stacked(newlevel ?
	    newlevel : runlevel);
	اگر (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_اندر_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_آغاز_services, tmplist, entries);
		آزاد(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		اگر (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			اگر (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_اندر_runlevel(bootlevel);
				TAILQ_CONCAT(main_آغاز_services, tmplist, entries);
				آزاد(tmplist);
			}
			اگر (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_آغاز_services,
					service->قدر);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	اگر (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_آغاز_services, main_stop_services, main_deptree, newlevel, parallel, going_نیچے_کو);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_STOP_برآمد,
	    going_نیچے_کو ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	اگر (newlevel) {
		rc_runlevel_سیٹ(newlevel);
		آزاد(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#اگر_تعریف_شدہ __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	اگر (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#اگر_ختم

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_آغاز_اندر, runlevel);

	/* Re-add our hotplugged services if they stopped */
	اگر (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->قدر, RC_SERVICE_HOTPLUGGED);

#اگر_تعریف_شدہ __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	اگر (proc) {
		جب ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		آزاد(proc);
	}
#اگر_ختم

	/* If we have a list of services to start then... */
	اگر (main_آغاز_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_الٹ(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *تسلسل_services = rc_services_اندر_runlevel(rlevel->قدر);

			/* Start those services. */
			rc_stringlist_ترتیب_دو(&تسلسل_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, تسلسل_services, rlevel->قدر, depoptions | RC_DEP_آغاز);
			rc_stringlist_free(تسلسل_services);
			تسلسل_services = deporder;
			do_آغاز_services(تسلسل_services, parallel);

			/* Wait for our services to finish */
			wait_for_services();

			/* Free the list of services, we're done with it. */
			rc_stringlist_free(تسلسل_services);
		}
		rc_stringlist_free(runlevel_chain);
	}

#اگر_تعریف_شدہ __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	اگر (proc) {
		جب ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		آزاد(proc);
	}

#اگر_ختم

	rc_plugin_تسلسل(RC_HOOK_RUNLEVEL_آغاز_برآمد, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	اگر (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	واپس EXIT_SUCCESS;
}
