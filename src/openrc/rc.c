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

#تعریف_کړه INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#تعریف_کړه INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#تعریف_کړه DEVBOOT			"/dev/.rcboot"

ثابت حرف *applet = عدم;
ساكن RC_STRINGLIST *main_hotplugged_services;
ساكن RC_STRINGLIST *main_stop_services;
ساكن RC_STRINGLIST *main_پیل_services;
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
	إذا (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		أرجع;

	طالما ((d = readdir(dp))) {
		إذا (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			استمر;

		إذا (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ساكن فراغ
cleanup(فراغ)
{
	صحيح svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	إذا (!rc_دننه_logger && !rc_دننه_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		إذا (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			حرر(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_خوندي(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		حرر(p);
	}
	LIST_FOREACH_خوندي(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		حرر(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_پیل_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	حرر(runlevel);
}

ساكن حرف
read_کلی(bool بلاک)
{
	بنية termios termios;
	حرف c = 0;
	صحيح fd = STDIN_FILENO;

	إذا (!isatty(fd))
		أرجع false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	إذا (!termios_orig) {
		termios_orig = xmalloc(حجم(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	إذا (بلاک)
		termios.c_cc[VMIN] = 1;
	وإلا {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	إذا (اقرأ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	أرجع c;
}

ساكن bool
want_interactive(فراغ)
{
	حرف c;
	ساكن bool gotinteractive;
	ساكن bool interactive;

	إذا (rc_yesno(getenv("EINFO_QUIET")))
		أرجع false;
	إذا (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	إذا (!interactive)
		أرجع false;
	c = read_کلی(false);
	أرجع (c == 'I' || c == 'i') ? true : false;
}

ساكن فراغ
mark_interactive(فراغ)
{
	FILE *fp;

	إذا ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ساكن فراغ
پرلپسې_پروګرام(ثابت حرف *prog)
{
	بنية sigaction sa;
	sigset_t ډک;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&ډک);
	sigprocmask(SIG_SETMASK, &ډک, &old);
	pid = fork();

	إذا (pid == -1) {
		rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_پیل_بهر, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	إذا (pid == 0) {
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

		إذا (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (حرف *)عدم);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, عدم);
	إذا (rc_waitpid(pid) == -1) {
		rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_پیل_بهر, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ساكن فراغ
open_shell(فراغ)
{
	ثابت حرف *shell;
	بنية passwd *pw;

#که_تعریف_شوی __linux__
	ثابت حرف *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	إذا (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (حرف *) عدم);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#د_که_پای

	shell = rc_conf_ارزښت("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	إذا (shell == عدم) {
		shell = getenv("SHELL");
		إذا (shell == عدم) {
			pw = getpwuid(getuid());
			إذا (pw)
				shell = pw->pw_shell;
			إذا (shell == عدم)
				shell = "/bin/sh";
		}
	}
	پرلپسې_پروګرام(shell);
}

ساكن bool
سېټ_krunlevel(ثابت حرف *level)
{
	FILE *fp;

	إذا (rc_is_user())
		أرجع true;

	إذا (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		إذا (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		أرجع false;
	}

	إذا (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		أرجع false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	أرجع true;
}

ساكن حرف *get_krunlevel(فراغ)
{
	حرف *بفر = عدم;
	FILE *fp;
	size_t i = 0;

	إذا (rc_is_user())
		أرجع عدم;

	إذا (access(RC_KRUNLEVEL, F_OK) != 0)
		أرجع عدم;
	إذا (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		أرجع عدم;
	}

	إذا (xgetline(&بفر, &i, fp) == -1) {
		حرر(بفر);
		بفر = عدم;
	}

	fclose(fp);
	أرجع بفر;
}

ساكن فراغ
جمع_کړه_pid(pid_t pid)
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
	LIST_ورننباسه_سر(&service_pids, p, entries);
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
		إذا (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_ورننباسه_سر(&free_these_pids, p, entries);
			اكسر;
		}
	}
	/* only call free if we're not inside a signal handler */
	إذا (!inside_signal) {
		LIST_FOREACH_خوندي(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			حرر(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, عدم);
}

ساكن فراغ
wait_for_services(فراغ)
{
	لكل (;;) {
		طالما (waitpid(0, 0, 0) != -1)
			;
		إذا (errno != EINTR)
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
	صحيح حالت = 0;
	بنية winsize ws;
	sigset_t sset;

	بدل (sig) {
	حالة SIGCHLD:
		افعل {
			pid = waitpid(-1, &حالت, WNOHANG);
			إذا (pid < 0) {
				إذا (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				أرجع;
			}
		} طالما (!WIFEXITED(حالت) && !WIFSIGNALED(حالت));

		/* Remove that pid from our list */
		إذا (pid > 0)
			remove_pid(pid, true);
		اكسر;

	حالة SIGWINCH:
		إذا (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		اكسر;

	حالة SIGINT:
		إذا (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالة SIGTERM:
		إذا (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالة SIGQUIT:
		إذا (!signame)
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
		rc_plugin_پرلپسې(RC_HOOK_ABORT, عدم);

		وتل(EXIT_FAILURE);
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
	إذا (access(INITEARLYSH, F_OK) == 0)
		پرلپسې_پروګرام(INITEARLYSH);

	uname(&uts);
	إذا (!rc_yesno(getenv("EINFO_QUIET"))) {
		چاپ_کړه("\n   %sOpenRC %s" نسخه "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_نورمال), ecolor(ECOLOR_BRACKET));
#که_تعریف_شوی BRANDING
		چاپ_کړه(BRANDING " (%s)", uts.machine);
#وإلا
		چاپ_کړه("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#د_که_پای
	}

	إذا ((sys = rc_sys()))
		چاپ_کړه(" [%s]", sys);

	چاپ_کړه("%s\n\n", ecolor(ECOLOR_نورمال));

	إذا (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		چاپ_کړه("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_نورمال));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	پرلپسې_پروګرام(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	إذا ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	إذا ((main_deptree = rc_deptree_بارول_2(false, عدم)) == عدم)
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
	أرجع retval;
}

ساكن فراغ
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *پیل_services,
				 ثابت RC_STRINGLIST *stop_services, ثابت RC_DEPTREE *deptree,
				 ثابت حرف *newlevel, bool parallel, bool going_ښکته)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = عدم;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	إذا (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_جمع_کړه(types_nw, "needsme");
		rc_stringlist_جمع_کړه(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_وېشل(rc_conf_ارزښت("rc_nostop"), " ");
	TAILQ_FOREACH_سرچپه(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->ارزښت);
		إذا (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			استمر;

		/* Sometimes we don't ever want to stop a service. */
		إذا (rc_stringlist_پیدا_کړه(nostop, service->ارزښت)) {
			rc_service_mark(service->ارزښت, RC_SERVICE_FAILED);
			استمر;
		}
		kwords = rc_deptree_depend(deptree, service->ارزښت, "keyword");
		إذا (rc_stringlist_پیدا_کړه(kwords, "-stop") ||
		    rc_stringlist_پیدا_کړه(kwords, "nostop") ||
		    (going_ښکته &&
			(rc_stringlist_پیدا_کړه(kwords, "-shutdown") ||
			    rc_stringlist_پیدا_کړه(kwords, "noshutdown"))))
			nstop = true;
		وإلا
			nstop = false;
		rc_stringlist_free(kwords);
		إذا (nstop) {
			rc_service_mark(service->ارزښت, RC_SERVICE_FAILED);
			استمر;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		إذا (crashed &&
		    rc_service_daemons_crashed(service->ارزښت))
			اذهب stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_پیدا_کړه(پیل_services, service->ارزښت);
		إذا (svc1) {
			إذا (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				إذا (!runlevel_config(service->ارزښت,runlevel) &&
				    !runlevel_config(service->ارزښت,newlevel))
					استمر;
			}
			وإلا
				استمر;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		إذا (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_جمع_کړه(tmplist, service->ارزښت);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = عدم;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_پیدا_کړه(پیل_services,
				    svc1->ارزښت);
				إذا (svc2)
					اكسر;
			}
			rc_stringlist_free(deporder);

			إذا (svc2)
				استمر;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->ارزښت);
		إذا (pid > 0) {
			جمع_کړه_pid(pid);
			إذا (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	إذا (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ساكن فراغ
do_پیل_services(ثابت RC_STRINGLIST *پیل_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	إذا (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	إذا (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, پیل_services, entries) {
		state = rc_service_state(service->ارزښت);
		إذا (state & RC_SERVICE_FAILED)
			استمر;
		إذا (!(state & RC_SERVICE_STOPPED)) {
			إذا (crashed &&
			    rc_service_daemons_crashed(service->ارزښت))
				rc_service_mark(service->ارزښت,
				    RC_SERVICE_STOPPED);
			وإلا
			    استمر;
		}
		إذا (!interactive)
			interactive = want_interactive();

		إذا (interactive) {
	interactive_retry:
			چاپ_کړه("\n");
			einfo("About to start the service %s",
			    service->ارزښت);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			بدل (read_کلی(true)) {
			حالة '1': اكسر;
			حالة '2': استمر;
			حالة '3': interactive = false; اكسر;
			حالة '4': open_shell(); اذهب interactive_retry;
			افتراضي: اذهب interactive_option;
			}
		}

		pid = service_پیل(service->ارزښت);
		إذا (pid == -1)
			اكسر;
		/* Remember the pid if we're running in parallel */
		إذا (pid > 0) {
			جمع_کړه_pid(pid);
			إذا (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	إذا (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	وإلا
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#که_تعریف_شوی RC_DEBUG
RC_NORETURN ساكن فراغ
handle_bad_signal(صحيح sig)
{
	حرف pid[10];
	صحيح حالت;
	pid_t crashed_pid = getpid();

	بدل (fork()) {
	حالة -1:
		_exit(sig);
		/* NOTREACHED */
	حالة 0:
		sprintf(pid, "%i", crashed_pid);
		چاپ_کړه("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", عدم));
		/* NOTREACHED */
	افتراضي:
		wait(&حالت);
	}
	_exit(1);
	/* NOTREACHED */
}
#د_که_پای

صحيح اصلي(صحيح argc, حرف **argv)
{
	ثابت حرف *bootlevel = عدم;
	حرف *newlevel = عدم;
	ثابت حرف *systype = عدم;
	RC_STRINGLIST *deporder = عدم;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_ښکته = false;
	صحيح depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	حرف *krunlevel = عدم;
	ثابت حرف *workingdir = "/";
	حرف *pidstr = عدم;
	صحيح opt;
	bool parallel;
	صحيح regen = 0;
	bool nostop = false;
#که_تعریف_شوی __linux__
	حرف *proc;
	حرف *p;
	حرف *token;
#د_که_پای

#که_تعریف_شوی RC_DEBUG
	signal_تیاری(SIGBUS, handle_bad_signal);
	signal_تیاری(SIGILL, handle_bad_signal);
	signal_تیاری(SIGSEGV, handle_bad_signal);
#د_که_پای

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	إذا (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	إذا (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'n':
			nostop = true;
			اكسر;
		حالة 'o':
			إذا (*optarg == '\0')
				optarg = عدم;
			إذا (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				وتل(EXIT_FAILURE);
			}
			إذا (rc_is_user())
				eerrorx("user services do not currently support --override");
			إذا (!سېټ_krunlevel(optarg))
				وتل(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			وتل(EXIT_SUCCESS);
			/* NOTREACHED */
		حالة 's':
			newlevel = rc_service_resolve(optarg);
			إذا (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		حالة 'S':
			systype = rc_sys();
			إذا (systype)
				چاپ_کړه("%s\n", systype);
			وتل(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	إذا (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	إذا (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	إذا (newlevel) {
		إذا (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		إذا (strcmp(newlevel, "reboot") == 0) {
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
	signal_تیاری(SIGINT, handle_signal);
	signal_تیاری(SIGQUIT, handle_signal);
	signal_تیاری(SIGTERM, handle_signal);
	signal_تیاری(SIGUSR1, handle_signal);
	signal_تیاری(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	إذا (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		حرر(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_بارول();

	/* Now we start handling our children */
	signal_تیاری(SIGCHLD, handle_signal);

	إذا (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_ښکته = true;
		إذا (access(RC_KRUNLEVEL, F_OK) != 0)
			سېټ_krunlevel(runlevel);
		rc_runlevel_سېټ(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} وإلا {
		/* We should not use krunlevel in sysinit or boot runlevels */
		إذا (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			إذا (krunlevel) {
				newlevel = krunlevel;
				سېټ_krunlevel(عدم);
			}
		}

		إذا (newlevel) {
			إذا (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#که_تعریف_شوی __linux__
			إذا (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				إذا (p == عدم)
					p = rc_proc_getent("softlevel");
				إذا (p != عدم) {
					سېټ_krunlevel(p);
					حرر(p);
				}
			}
#د_که_پای
		}
	}

	إذا (going_ښکته) {
#که_تعریف_شوی __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#د_که_پای

		rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_STOP_دننه, newlevel);
	} وإلا {
		rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_STOP_دننه, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	إذا (newlevel && strcmp(runlevel, newlevel) != 0 && !going_ښکته) {
		إذا (!rc_runlevel_exists(newlevel)) {
			rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_STOP_بهر, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	إذا ((main_deptree = rc_deptree_بارول_2(0, &regen)) == عدم) {
		rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_STOP_بهر, runlevel);
		eerrorx("failed to load deptree");
	}

	إذا (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	إذا (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_STOP_بهر, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), strerror(errno));
	}

	/* Create a list of all services which we could stop (assuming
	* they won't be active in the new or current runlevel) including
	* all those services which have been started, are inactive or
	* are currently starting.  Clearly, some of these will be listed
	* in the new or current runlevel so we won't actually be stopping
	* them all.
	*/
	main_stop_services = rc_services_دننه_state(RC_SERVICE_STARTED);
	tmplist = rc_services_دننه_state(RC_SERVICE_INACTIVE);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	حرر(tmplist);
	tmplist = rc_services_دننه_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	حرر(tmplist);
	إذا (main_stop_services)
		rc_stringlist_ترتیبول(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_جمع_کړه(main_types_nwua, "ineed");
	rc_stringlist_جمع_کړه(main_types_nwua, "iwant");
	rc_stringlist_جمع_کړه(main_types_nwua, "iuse");
	rc_stringlist_جمع_کړه(main_types_nwua, "iafter");

	إذا (main_stop_services) {
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
	main_hotplugged_services = rc_services_دننه_state(RC_SERVICE_HOTPLUGGED);
	main_پیل_services = rc_services_دننه_runlevel_stacked(newlevel ?
	    newlevel : runlevel);
	إذا (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_دننه_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_پیل_services, tmplist, entries);
		حرر(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		إذا (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			إذا (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_دننه_runlevel(bootlevel);
				TAILQ_CONCAT(main_پیل_services, tmplist, entries);
				حرر(tmplist);
			}
			إذا (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_پیل_services,
					service->ارزښت);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	إذا (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_پیل_services, main_stop_services, main_deptree, newlevel, parallel, going_ښکته);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_STOP_بهر,
	    going_ښکته ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	إذا (newlevel) {
		rc_runlevel_سېټ(newlevel);
		حرر(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#که_تعریف_شوی __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	إذا (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#د_که_پای

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_پیل_دننه, runlevel);

	/* Re-add our hotplugged services if they stopped */
	إذا (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->ارزښت, RC_SERVICE_HOTPLUGGED);

#که_تعریف_شوی __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	إذا (proc) {
		طالما ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		حرر(proc);
	}
#د_که_پای

	/* If we have a list of services to start then... */
	إذا (main_پیل_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_سرچپه(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *پرلپسې_services = rc_services_دننه_runlevel(rlevel->ارزښت);

			/* Start those services. */
			rc_stringlist_ترتیبول(&پرلپسې_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, پرلپسې_services, rlevel->ارزښت, depoptions | RC_DEP_پیل);
			rc_stringlist_free(پرلپسې_services);
			پرلپسې_services = deporder;
			do_پیل_services(پرلپسې_services, parallel);

			/* Wait for our services to finish */
			wait_for_services();

			/* Free the list of services, we're done with it. */
			rc_stringlist_free(پرلپسې_services);
		}
		rc_stringlist_free(runlevel_chain);
	}

#که_تعریف_شوی __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	إذا (proc) {
		طالما ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		حرر(proc);
	}

#د_که_پای

	rc_plugin_پرلپسې(RC_HOOK_RUNLEVEL_پیل_بهر, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	إذا (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	أرجع EXIT_SUCCESS;
}
