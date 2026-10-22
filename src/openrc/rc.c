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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <libgen.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/utsname.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <termios.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "rc-logger.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "plugin.h"
#अन्तर्भाव "version.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

स्थिरांक अक्षर *extraopts = रिक्त;
स्थिरांक अक्षर getoptstring[] = "a:no:s:S" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "no-stop", 0, रिक्त, 'n' },
	{ "override",    1, रिक्त, 'o' },
	{ "service",     1, रिक्त, 's' },
	{ "sys",         0, रिक्त, 'S' },
	longopts_COMMON
};
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
स्थिरांक अक्षर *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

स्थिरांक अक्षर *applet = रिक्त;
स्थिर RC_STRINGLIST *main_hotplugged_services;
स्थिर RC_STRINGLIST *main_stop_services;
स्थिर RC_STRINGLIST *main_start_services;
स्थिर RC_STRINGLIST *main_types_nw;
स्थिर RC_STRINGLIST *main_types_nwua;
स्थिर RC_DEPTREE *main_deptree;
स्थिर अक्षर *runlevel;

संरचना termios *termios_orig = रिक्त;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

स्थिर शून्य
clean_failed(शून्य)
{
	सूची *dp;
	संरचना dirent *d;

	/* Clean the failed services state dir now */
	जर (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		परत;

	पर्यंत ((d = सूची_पढ़ो(dp))) {
		जर (म_भेद(d->d_name, ".") == 0 || म_भेद(d->d_name, "..") == 0)
			सुरूठेवा;

		जर (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, म_त्रुटि(त्रुटि_सं));
	}
	बंद_सूची(dp);
}

स्थिर शून्य
cleanup(शून्य)
{
	पूर्णांक svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	जर (!rc_in_logger && !rc_in_plugin &&
	    applet && (म_भेद(applet, "rc") == 0 || म_भेद(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		जर (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			मुक्त(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		मुक्त(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		मुक्त(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	मुक्त(runlevel);
}

स्थिर अक्षर
read_किल्ली(bool block)
{
	संरचना termios termios;
	अक्षर c = 0;
	पूर्णांक fd = STDIN_FILENO;

	जर (!isatty(fd))
		परत false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	जर (!termios_orig) {
		termios_orig = xmalloc(आकार(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	जर (block)
		termios.c_cc[VMIN] = 1;
	अन्यथा {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	जर (पठ(fd, &c, 1) == -1)
		eerror("read: %s", म_त्रुटि(त्रुटि_सं));
	tcsetattr(fd, TCSANOW, termios_orig);
	परत c;
}

स्थिर bool
want_interactive(शून्य)
{
	अक्षर c;
	स्थिर bool gotinteractive;
	स्थिर bool interactive;

	जर (rc_yesno(दो_पर्या("EINFO_QUIET")))
		परत false;
	जर (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	जर (!interactive)
		परत false;
	c = read_किल्ली(false);
	परत (c == 'I' || c == 'i') ? true : false;
}

स्थिर शून्य
mark_interactive(शून्य)
{
	खाता *fp;

	जर ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		ख_बंद(fp);
}

स्थिर शून्य
run_program(स्थिरांक अक्षर *prog)
{
	संरचना sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	स_रखो(&sa, 0, आकार(sa));
	sa.sa_handler = संक_पूर्व;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = विभाजन();

	जर (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
	}
	जर (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, रिक्त);
		sigaction(SIGHUP, &sa, रिक्त);
		sigaction(संक_विघ्न, &sa, रिक्त);
		sigaction(SIGQUIT, &sa, रिक्त);
		sigaction(संक_इति, &sa, रिक्त);
		sigaction(SIGUSR1, &sa, रिक्त);
		sigaction(SIGWINCH, &sa, रिक्त);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, रिक्त);

		जर (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (अक्षर *)रिक्त);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    म_त्रुटि(त्रुटि_सं));
		_exit(निकास_त्रुटि);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, रिक्त);
	जर (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

स्थिर शून्य
open_shell(शून्य)
{
	स्थिरांक अक्षर *shell;
	संरचना passwd *pw;

#ifdef __linux__
	स्थिरांक अक्षर *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	जर (sys && म_भेद(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (अक्षर *) रिक्त);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, म_त्रुटि(त्रुटि_सं));
	}
#endif

	shell = rc_conf_मूल्य("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	जर (shell == रिक्त) {
		shell = दो_पर्या("SHELL");
		जर (shell == रिक्त) {
			pw = getpwuid(getuid());
			जर (pw)
				shell = pw->pw_shell;
			जर (shell == रिक्त)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

स्थिर bool
set_krunlevel(स्थिरांक अक्षर *level)
{
	खाता *fp;

	जर (rc_is_user())
		परत true;

	जर (!level ||
	    म_भेद(level, दो_पर्या ("RC_BOOTLEVEL")) == 0 ||
	    म_भेद(level, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(level, RC_LEVEL_SYSINIT) == 0)
	{
		जर (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, म_त्रुटि(त्रुटि_सं));
		परत false;
	}

	जर (!(fp = ख_खोलो(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, म_त्रुटि(त्रुटि_सं));
		परत false;
	}

	ख_लिखो(fp, "%s", level);
	ख_बंद(fp);
	परत true;
}

स्थिर अक्षर *get_krunlevel(शून्य)
{
	अक्षर *buffer = रिक्त;
	खाता *fp;
	माप_प्रकार i = 0;

	जर (rc_is_user())
		परत रिक्त;

	जर (access(RC_KRUNLEVEL, F_OK) != 0)
		परत रिक्त;
	जर (!(fp = ख_खोलो(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, म_त्रुटि(त्रुटि_सं));
		परत रिक्त;
	}

	जर (xgetline(&buffer, &i, fp) == -1) {
		मुक्त(buffer);
		buffer = रिक्त;
	}

	ख_बंद(fp);
	परत buffer;
}

स्थिर शून्य
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(आकार(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_घाला_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, रिक्त);
}

स्थिर शून्य
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		जर (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_घाला_HEAD(&free_these_pids, p, entries);
			थांबा;
		}
	}
	/* only call free if we're not inside a signal handler */
	जर (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			मुक्त(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, रिक्त);
}

स्थिर शून्य
wait_for_services(शून्य)
{
	साठी (;;) {
		पर्यंत (waitpid(0, 0, 0) != -1)
			;
		जर (त्रुटि_सं != EINTR)
			थांबा;
	}
}

स्थिर शून्य
handle_signal(पूर्णांक sig)
{
	पूर्णांक serrno = त्रुटि_सं;
	स्थिरांक अक्षर *signame = रिक्त;
	pid_t pid;
	RC_PID *pi;
	पूर्णांक status = 0;
	संरचना winsize ws;
	sigset_t sset;

	निवड (sig) {
	स्थिती SIGCHLD:
		करा {
			pid = waitpid(-1, &status, WNOHANG);
			जर (pid < 0) {
				जर (त्रुटि_सं != ECHILD)
					eerror("waitpid: %s", म_त्रुटि(त्रुटि_सं));
				परत;
			}
		} पर्यंत (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		जर (pid > 0)
			remove_pid(pid, true);
		थांबा;

	स्थिती SIGWINCH:
		जर (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		थांबा;

	स्थिती संक_विघ्न:
		जर (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	स्थिती संक_इति:
		जर (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	स्थिती SIGQUIT:
		जर (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	स्थिती SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, रिक्त);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    समाप्त(pi->pid, संक_इति);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, रिक्त);

		निकास(निकास_त्रुटि);
		/* NOTREACHED */

	पूर्वनिर्धारित:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	त्रुटि_सं = serrno;
}

स्थिर शून्य
do_sysinit(शून्य)
{
	संरचना utsname uts;
	स्थिरांक अक्षर *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	जर (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	जर (!rc_yesno(दो_पर्या("EINFO_QUIET"))) {
		म_लिखो("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		म_लिखो(BRANDING " (%s)", uts.machine);
#अन्यथा
		म_लिखो("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	जर ((sys = rc_sys()))
		म_लिखो(" [%s]", sys);

	म_लिखो("%s\n\n", ecolor(ECOLOR_NORMAL));

	जर (!rc_yesno(दो_पर्या ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		म_लिखो("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	जर ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	जर ((main_deptree = _rc_deptree_load(false, रिक्त)) == रिक्त)
		eerrorx("failed to load deptree");
}

स्थिर bool
runlevel_config(स्थिरांक अक्षर *service, स्थिरांक अक्षर *level)
{
	अक्षर *init = rc_service_resolve(service);
	अक्षर *conf, *dir;
	bool retval;

	dir = स_नाम(स_नाम(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	मुक्त(conf);
	मुक्त(init);
	परत retval;
}

स्थिर शून्य
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 स्थिरांक RC_STRINGLIST *stop_services, स्थिरांक RC_DEPTREE *deptree,
				 स्थिरांक अक्षर *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = रिक्त;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	जर (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_मूल्य("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->मूल्य);
		जर (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			सुरूठेवा;

		/* Sometimes we don't ever want to stop a service. */
		जर (rc_stringlist_find(nostop, service->मूल्य)) {
			rc_service_mark(service->मूल्य, RC_SERVICE_FAILED);
			सुरूठेवा;
		}
		kwords = rc_deptree_depend(deptree, service->मूल्य, "keyword");
		जर (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		अन्यथा
			nstop = false;
		rc_stringlist_free(kwords);
		जर (nstop) {
			rc_service_mark(service->मूल्य, RC_SERVICE_FAILED);
			सुरूठेवा;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		जर (crashed &&
		    rc_service_daemons_crashed(service->मूल्य))
			जा stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->मूल्य);
		जर (svc1) {
			जर (newlevel && म_भेद(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				जर (!runlevel_config(service->मूल्य,runlevel) &&
				    !runlevel_config(service->मूल्य,newlevel))
					सुरूठेवा;
			}
			अन्यथा
				सुरूठेवा;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		जर (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->मूल्य);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = रिक्त;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->मूल्य);
				जर (svc2)
					थांबा;
			}
			rc_stringlist_free(deporder);

			जर (svc2)
				सुरूठेवा;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->मूल्य);
		जर (pid > 0) {
			add_pid(pid);
			जर (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	जर (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

स्थिर शून्य
do_start_services(स्थिरांक RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	जर (!rc_yesno(दो_पर्या("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	त्रुटि_सं = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	जर (त्रुटि_सं == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->मूल्य);
		जर (state & RC_SERVICE_FAILED)
			सुरूठेवा;
		जर (!(state & RC_SERVICE_STOPPED)) {
			जर (crashed &&
			    rc_service_daemons_crashed(service->मूल्य))
				rc_service_mark(service->मूल्य,
				    RC_SERVICE_STOPPED);
			अन्यथा
			    सुरूठेवा;
		}
		जर (!interactive)
			interactive = want_interactive();

		जर (interactive) {
	interactive_retry:
			म_लिखो("\n");
			einfo("About to start the service %s",
			    service->मूल्य);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			निवड (read_किल्ली(true)) {
			स्थिती '1': थांबा;
			स्थिती '2': सुरूठेवा;
			स्थिती '3': interactive = false; थांबा;
			स्थिती '4': open_shell(); जा interactive_retry;
			पूर्वनिर्धारित: जा interactive_option;
			}
		}

		pid = service_start(service->मूल्य);
		जर (pid == -1)
			थांबा;
		/* Remember the pid if we're running in parallel */
		जर (pid > 0) {
			add_pid(pid);
			जर (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	जर (interactive &&
			(म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 म_भेद(runlevel, दो_पर्या("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	अन्यथा
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN स्थिर शून्य
handle_bad_signal(पूर्णांक sig)
{
	अक्षर pid[10];
	पूर्णांक status;
	pid_t crashed_pid = getpid();

	निवड (विभाजन()) {
	स्थिती -1:
		_exit(sig);
		/* NOTREACHED */
	स्थिती 0:
		प्र_लिखो(pid, "%i", crashed_pid);
		म_लिखो("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", रिक्त));
		/* NOTREACHED */
	पूर्वनिर्धारित:
		रुको(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	स्थिरांक अक्षर *bootlevel = रिक्त;
	अक्षर *newlevel = रिक्त;
	स्थिरांक अक्षर *systype = रिक्त;
	RC_STRINGLIST *deporder = रिक्त;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	पूर्णांक depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	अक्षर *krunlevel = रिक्त;
	स्थिरांक अक्षर *workingdir = "/";
	अक्षर *pidstr = रिक्त;
	पूर्णांक opt;
	bool parallel;
	पूर्णांक regen = 0;
	bool nostop = false;
#ifdef __linux__
	अक्षर *proc;
	अक्षर *p;
	अक्षर *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(संक_अवैध, handle_bad_signal);
	signal_setup(संक_अंश, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	जर (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	जर (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
		स्थिती 'n':
			nostop = true;
			थांबा;
		स्थिती 'o':
			जर (*optarg == '\0')
				optarg = रिक्त;
			जर (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				निकास(निकास_त्रुटि);
			}
			जर (rc_is_user())
				eerrorx("user services do not currently support --override");
			जर (!set_krunlevel(optarg))
				निकास(निकास_त्रुटि);
			einfo("Overriding next runlevel to %s", optarg);
			निकास(निकास_सफल);
			/* NOTREACHED */
		स्थिती 's':
			newlevel = rc_service_resolve(optarg);
			जर (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, म_त्रुटि(त्रुटि_सं));
			/* NOTREACHED */
		स्थिती 'S':
			systype = rc_sys();
			जर (systype)
				म_लिखो("%s\n", systype);
			निकास(निकास_सफल);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	जर (rc_is_user() && !(workingdir = दो_पर्या("HOME")))
		eerrorx("HOME is unset.");

	जर (स_बदलो(workingdir) == -1)
		eerror("chdir: %s", म_त्रुटि(त्रुटि_सं));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	जर (newlevel) {
		जर (rc_is_user() && म_भेद(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		जर (म_भेद(newlevel, "reboot") == 0) {
			newlevel = UNCONST(RC_LEVEL_SHUTDOWN);
			setenv("RC_REBOOT", "YES", 1);
		}
	}

	निकास_पर(cleanup);

	/* Enable logging */
	setenv("EINFO_LOG", "openrc", 1);

	/* Export our PID */
	xasprintf(&pidstr, "%d", getpid());
	setenv("RC_PID", pidstr, 1);
	मुक्त(pidstr);

	/* Create a list of all services which should be started for the new or
	* current runlevel including those in boot, sysinit and hotplugged
	* runlevels.  Clearly, some of these will already be started so we
	* won't actually be starting them all.
	*/
	bootlevel = दो_पर्या("RC_BOOTLEVEL");
	runlevel = default_runlevel();

	rc_logger_open(newlevel ? newlevel : runlevel);

	/* Setup a signal handler */
	signal_setup(संक_विघ्न, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(संक_इति, handle_signal);
	signal_setup(SIGUSR1, handle_signal);
	signal_setup(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	जर (newlevel && म_भेद(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		मुक्त(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	जर (newlevel &&
	    (म_भेद(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		म_भेद(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		जर (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} अन्यथा {
		/* We should not use krunlevel in sysinit or boot runlevels */
		जर (!newlevel ||
		    (म_भेद(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			म_भेद(newlevel, दो_पर्या("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			जर (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(रिक्त);
			}
		}

		जर (newlevel) {
			जर (म_भेद(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			जर (म_भेद(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				जर (p == रिक्त)
					p = rc_proc_getent("softlevel");
				जर (p != रिक्त) {
					set_krunlevel(p);
					मुक्त(p);
				}
			}
#endif
		}
	}

	जर (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} अन्यथा {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	जर (newlevel && म_भेद(runlevel, newlevel) != 0 && !going_down) {
		जर (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	जर ((main_deptree = _rc_deptree_load(0, &regen)) == रिक्त) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	जर (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	जर (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), म_त्रुटि(त्रुटि_सं));
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
	मुक्त(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	मुक्त(tmplist);
	जर (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	जर (main_stop_services) {
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
	जर (म_भेद(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    म_भेद(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		मुक्त(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		जर (म_भेद(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			जर (म_भेद(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				मुक्त(tmplist);
			}
			जर (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->मूल्य);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	जर (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	जर (newlevel) {
		rc_runlevel_set(newlevel);
		मुक्त(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	जर (म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	जर (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->मूल्य, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	जर (proc) {
		पर्यंत ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		मुक्त(proc);
	}
#endif

	/* If we have a list of services to start then... */
	जर (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->मूल्य);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->मूल्य, depoptions | RC_DEP_START);
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
	जर (proc) {
		पर्यंत ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		मुक्त(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	जर (regen && म_भेद(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	परत निकास_सफल;
}
