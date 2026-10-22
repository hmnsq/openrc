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

#समावेश <errno.h>
#समावेश <dirent.h>
#समावेश <getopt.h>
#समावेश <libgen.h>
#समावेश <pwd.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <signal.h>
#समावेश <string.h>
#समावेश <sys/ioctl.h>
#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <sys/utsname.h>
#समावेश <sys/wait.h>
#समावेश <termios.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "rc_exec.h"
#समावेश "rc-logger.h"
#समावेश "misc.h"
#समावेश "plugin.h"
#समावेश "version.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "a:no:s:S" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "no-stop", 0, शून्य, 'n' },
	{ "override",    1, शून्य, 'o' },
	{ "service",     1, शून्य, 's' },
	{ "sys",         0, शून्य, 'S' },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#घोषणा INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#घोषणा INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#घोषणा DEVBOOT			"/dev/.rcboot"

स्थिर अक्षर *applet = शून्य;
अटल RC_STRINGLIST *main_hotplugged_services;
अटल RC_STRINGLIST *main_stop_services;
अटल RC_STRINGLIST *main_start_services;
अटल RC_STRINGLIST *main_types_nw;
अटल RC_STRINGLIST *main_types_nwua;
अटल RC_DEPTREE *main_deptree;
अटल अक्षर *runlevel;

काष्ठा termios *termios_orig = शून्य;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

अटल व्योम
clean_failed(व्योम)
{
	सूची *dp;
	काष्ठा dirent *d;

	/* Clean the failed services state dir now */
	अगर (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		वापस;

	जबतक ((d = सूची_पढ़ो(dp))) {
		अगर (म_भेद(d->d_name, ".") == 0 || म_भेद(d->d_name, "..") == 0)
			जारी;

		अगर (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, म_त्रुटि(त्रुटि_सं));
	}
	बंद_सूची(dp);
}

अटल व्योम
cleanup(व्योम)
{
	पूर्णांक svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	अगर (!rc_in_logger && !rc_in_plugin &&
	    applet && (म_भेद(applet, "rc") == 0 || म_भेद(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		अगर (termios_orig) {
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

अटल अक्षर
read_कुंजी(bool block)
{
	काष्ठा termios termios;
	अक्षर c = 0;
	पूर्णांक fd = STDIN_FILENO;

	अगर (!isatty(fd))
		वापस false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	अगर (!termios_orig) {
		termios_orig = xmalloc(माप(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	अगर (block)
		termios.c_cc[VMIN] = 1;
	अन्यथा {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	अगर (पढ़ो(fd, &c, 1) == -1)
		eerror("read: %s", म_त्रुटि(त्रुटि_सं));
	tcsetattr(fd, TCSANOW, termios_orig);
	वापस c;
}

अटल bool
want_interactive(व्योम)
{
	अक्षर c;
	अटल bool gotinteractive;
	अटल bool interactive;

	अगर (rc_yesno(दो_पर्या("EINFO_QUIET")))
		वापस false;
	अगर (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	अगर (!interactive)
		वापस false;
	c = read_कुंजी(false);
	वापस (c == 'I' || c == 'i') ? true : false;
}

अटल व्योम
mark_interactive(व्योम)
{
	खाता *fp;

	अगर ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		ख_बंद(fp);
}

अटल व्योम
run_program(स्थिर अक्षर *prog)
{
	काष्ठा sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	स_रखो(&sa, 0, माप(sa));
	sa.sa_handler = संक_पूर्व;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = विभाजन();

	अगर (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));
	}
	अगर (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, शून्य);
		sigaction(SIGHUP, &sa, शून्य);
		sigaction(संक_विघ्न, &sa, शून्य);
		sigaction(SIGQUIT, &sa, शून्य);
		sigaction(संक_इति, &sa, शून्य);
		sigaction(SIGUSR1, &sa, शून्य);
		sigaction(SIGWINCH, &sa, शून्य);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, शून्य);

		अगर (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (अक्षर *)शून्य);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    म_त्रुटि(त्रुटि_सं));
		_निकास(निकास_त्रुटि);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, शून्य);
	अगर (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

अटल व्योम
open_shell(व्योम)
{
	स्थिर अक्षर *shell;
	काष्ठा passwd *pw;

#अगर_घोषित __linux__
	स्थिर अक्षर *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	अगर (sys && म_भेद(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (अक्षर *) शून्य);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, म_त्रुटि(त्रुटि_सं));
	}
#पूर्ण_अगर

	shell = rc_conf_मूल्य("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	अगर (shell == शून्य) {
		shell = दो_पर्या("SHELL");
		अगर (shell == शून्य) {
			pw = getpwuid(getuid());
			अगर (pw)
				shell = pw->pw_shell;
			अगर (shell == शून्य)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

अटल bool
set_krunlevel(स्थिर अक्षर *level)
{
	खाता *fp;

	अगर (rc_is_user())
		वापस true;

	अगर (!level ||
	    म_भेद(level, दो_पर्या ("RC_BOOTLEVEL")) == 0 ||
	    म_भेद(level, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(level, RC_LEVEL_SYSINIT) == 0)
	{
		अगर (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, म_त्रुटि(त्रुटि_सं));
		वापस false;
	}

	अगर (!(fp = ख_खोलो(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, म_त्रुटि(त्रुटि_सं));
		वापस false;
	}

	ख_लिखो(fp, "%s", level);
	ख_बंद(fp);
	वापस true;
}

अटल अक्षर *get_krunlevel(व्योम)
{
	अक्षर *बफर = शून्य;
	खाता *fp;
	माप_प्रकार i = 0;

	अगर (rc_is_user())
		वापस शून्य;

	अगर (access(RC_KRUNLEVEL, F_OK) != 0)
		वापस शून्य;
	अगर (!(fp = ख_खोलो(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, म_त्रुटि(त्रुटि_सं));
		वापस शून्य;
	}

	अगर (xgetline(&बफर, &i, fp) == -1) {
		मुक्त(बफर);
		बफर = शून्य;
	}

	ख_बंद(fp);
	वापस बफर;
}

अटल व्योम
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(माप(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_जोड़ो_शीर्ष(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, शून्य);
}

अटल व्योम
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		अगर (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_जोड़ो_शीर्ष(&free_these_pids, p, entries);
			अवरोध;
		}
	}
	/* only call free if we're not inside a signal handler */
	अगर (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			मुक्त(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, शून्य);
}

अटल व्योम
wait_for_services(व्योम)
{
	क्रम (;;) {
		जबतक (waitpid(0, 0, 0) != -1)
			;
		अगर (त्रुटि_सं != EINTR)
			अवरोध;
	}
}

अटल व्योम
handle_signal(पूर्णांक sig)
{
	पूर्णांक serrno = त्रुटि_सं;
	स्थिर अक्षर *signame = शून्य;
	pid_t pid;
	RC_PID *pi;
	पूर्णांक status = 0;
	काष्ठा winsize ws;
	sigset_t sset;

	चयन (sig) {
	हाल SIGCHLD:
		करो {
			pid = waitpid(-1, &status, WNOHANG);
			अगर (pid < 0) {
				अगर (त्रुटि_सं != ECHILD)
					eerror("waitpid: %s", म_त्रुटि(त्रुटि_सं));
				वापस;
			}
		} जबतक (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		अगर (pid > 0)
			remove_pid(pid, true);
		अवरोध;

	हाल SIGWINCH:
		अगर (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		अवरोध;

	हाल संक_विघ्न:
		अगर (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	हाल संक_इति:
		अगर (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	हाल SIGQUIT:
		अगर (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	हाल SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, शून्य);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    समाप्त(pi->pid, संक_इति);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, शून्य);

		निकास(निकास_त्रुटि);
		/* NOTREACHED */

	शेष:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	त्रुटि_सं = serrno;
}

अटल व्योम
do_sysinit(व्योम)
{
	काष्ठा utsname uts;
	स्थिर अक्षर *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	अगर (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	अगर (!rc_yesno(दो_पर्या("EINFO_QUIET"))) {
		म_लिखो("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#अगर_घोषित BRANDING
		म_लिखो(BRANDING " (%s)", uts.machine);
#अन्यथा
		म_लिखो("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#पूर्ण_अगर
	}

	अगर ((sys = rc_sys()))
		म_लिखो(" [%s]", sys);

	म_लिखो("%s\n\n", ecolor(ECOLOR_NORMAL));

	अगर (!rc_yesno(दो_पर्या ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		म_लिखो("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	अगर ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	अगर ((main_deptree = _rc_deptree_load(false, शून्य)) == शून्य)
		eerrorx("failed to load deptree");
}

अटल bool
runlevel_config(स्थिर अक्षर *service, स्थिर अक्षर *level)
{
	अक्षर *init = rc_service_resolve(service);
	अक्षर *conf, *dir;
	bool retval;

	dir = स_नाम(स_नाम(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	मुक्त(conf);
	मुक्त(init);
	वापस retval;
}

अटल व्योम
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 स्थिर RC_STRINGLIST *stop_services, स्थिर RC_DEPTREE *deptree,
				 स्थिर अक्षर *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = शून्य;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	अगर (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_मूल्य("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->मूल्य);
		अगर (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			जारी;

		/* Sometimes we don't ever want to stop a service. */
		अगर (rc_stringlist_find(nostop, service->मूल्य)) {
			rc_service_mark(service->मूल्य, RC_SERVICE_FAILED);
			जारी;
		}
		kwords = rc_deptree_depend(deptree, service->मूल्य, "keyword");
		अगर (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		अन्यथा
			nstop = false;
		rc_stringlist_free(kwords);
		अगर (nstop) {
			rc_service_mark(service->मूल्य, RC_SERVICE_FAILED);
			जारी;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		अगर (crashed &&
		    rc_service_daemons_crashed(service->मूल्य))
			जाओ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->मूल्य);
		अगर (svc1) {
			अगर (newlevel && म_भेद(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				अगर (!runlevel_config(service->मूल्य,runlevel) &&
				    !runlevel_config(service->मूल्य,newlevel))
					जारी;
			}
			अन्यथा
				जारी;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		अगर (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->मूल्य);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = शून्य;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->मूल्य);
				अगर (svc2)
					अवरोध;
			}
			rc_stringlist_free(deporder);

			अगर (svc2)
				जारी;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->मूल्य);
		अगर (pid > 0) {
			add_pid(pid);
			अगर (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	अगर (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

अटल व्योम
do_start_services(स्थिर RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	अगर (!rc_yesno(दो_पर्या("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	त्रुटि_सं = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	अगर (त्रुटि_सं == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->मूल्य);
		अगर (state & RC_SERVICE_FAILED)
			जारी;
		अगर (!(state & RC_SERVICE_STOPPED)) {
			अगर (crashed &&
			    rc_service_daemons_crashed(service->मूल्य))
				rc_service_mark(service->मूल्य,
				    RC_SERVICE_STOPPED);
			अन्यथा
			    जारी;
		}
		अगर (!interactive)
			interactive = want_interactive();

		अगर (interactive) {
	interactive_retry:
			म_लिखो("\n");
			einfo("About to start the service %s",
			    service->मूल्य);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			चयन (read_कुंजी(true)) {
			हाल '1': अवरोध;
			हाल '2': जारी;
			हाल '3': interactive = false; अवरोध;
			हाल '4': open_shell(); जाओ interactive_retry;
			शेष: जाओ interactive_option;
			}
		}

		pid = service_start(service->मूल्य);
		अगर (pid == -1)
			अवरोध;
		/* Remember the pid if we're running in parallel */
		अगर (pid > 0) {
			add_pid(pid);
			अगर (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	अगर (interactive &&
			(म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 म_भेद(runlevel, दो_पर्या("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	अन्यथा
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#अगर_घोषित RC_DEBUG
RC_NORETURN अटल व्योम
handle_bad_signal(पूर्णांक sig)
{
	अक्षर pid[10];
	पूर्णांक status;
	pid_t crashed_pid = getpid();

	चयन (विभाजन()) {
	हाल -1:
		_निकास(sig);
		/* NOTREACHED */
	हाल 0:
		प्र_लिखो(pid, "%i", crashed_pid);
		म_लिखो("\nAuto launching gdb!\n\n");
		_निकास(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", शून्य));
		/* NOTREACHED */
	शेष:
		रुको(&status);
	}
	_निकास(1);
	/* NOTREACHED */
}
#पूर्ण_अगर

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	स्थिर अक्षर *bootlevel = शून्य;
	अक्षर *newlevel = शून्य;
	स्थिर अक्षर *systype = शून्य;
	RC_STRINGLIST *deporder = शून्य;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	पूर्णांक depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	अक्षर *krunlevel = शून्य;
	स्थिर अक्षर *workingdir = "/";
	अक्षर *pidstr = शून्य;
	पूर्णांक opt;
	bool parallel;
	पूर्णांक regen = 0;
	bool nostop = false;
#अगर_घोषित __linux__
	अक्षर *proc;
	अक्षर *p;
	अक्षर *token;
#पूर्ण_अगर

#अगर_घोषित RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(संक_अवैध, handle_bad_signal);
	signal_setup(संक_अंश, handle_bad_signal);
#पूर्ण_अगर

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	अगर (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	अगर (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल 'n':
			nostop = true;
			अवरोध;
		हाल 'o':
			अगर (*optarg == '\0')
				optarg = शून्य;
			अगर (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				निकास(निकास_त्रुटि);
			}
			अगर (rc_is_user())
				eerrorx("user services do not currently support --override");
			अगर (!set_krunlevel(optarg))
				निकास(निकास_त्रुटि);
			einfo("Overriding next runlevel to %s", optarg);
			निकास(निकास_सफल);
			/* NOTREACHED */
		हाल 's':
			newlevel = rc_service_resolve(optarg);
			अगर (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, म_त्रुटि(त्रुटि_सं));
			/* NOTREACHED */
		हाल 'S':
			systype = rc_sys();
			अगर (systype)
				म_लिखो("%s\n", systype);
			निकास(निकास_सफल);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	अगर (rc_is_user() && !(workingdir = दो_पर्या("HOME")))
		eerrorx("HOME is unset.");

	अगर (स_बदलो(workingdir) == -1)
		eerror("chdir: %s", म_त्रुटि(त्रुटि_सं));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	अगर (newlevel) {
		अगर (rc_is_user() && म_भेद(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		अगर (म_भेद(newlevel, "reboot") == 0) {
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
	अगर (newlevel && म_भेद(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		मुक्त(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	अगर (newlevel &&
	    (म_भेद(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		म_भेद(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		अगर (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} अन्यथा {
		/* We should not use krunlevel in sysinit or boot runlevels */
		अगर (!newlevel ||
		    (म_भेद(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			म_भेद(newlevel, दो_पर्या("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			अगर (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(शून्य);
			}
		}

		अगर (newlevel) {
			अगर (म_भेद(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#अगर_घोषित __linux__
			अगर (म_भेद(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				अगर (p == शून्य)
					p = rc_proc_getent("softlevel");
				अगर (p != शून्य) {
					set_krunlevel(p);
					मुक्त(p);
				}
			}
#पूर्ण_अगर
		}
	}

	अगर (going_down) {
#अगर_घोषित __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#पूर्ण_अगर

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} अन्यथा {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	अगर (newlevel && म_भेद(runlevel, newlevel) != 0 && !going_down) {
		अगर (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	अगर ((main_deptree = _rc_deptree_load(0, &regen)) == शून्य) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	अगर (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	अगर (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	अगर (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	अगर (main_stop_services) {
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
	अगर (म_भेद(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    म_भेद(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		मुक्त(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		अगर (म_भेद(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			अगर (म_भेद(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				मुक्त(tmplist);
			}
			अगर (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->मूल्य);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	अगर (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	अगर (newlevel) {
		rc_runlevel_set(newlevel);
		मुक्त(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#अगर_घोषित __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	अगर (म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#पूर्ण_अगर

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	अगर (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->मूल्य, RC_SERVICE_HOTPLUGGED);

#अगर_घोषित __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	अगर (proc) {
		जबतक ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		मुक्त(proc);
	}
#पूर्ण_अगर

	/* If we have a list of services to start then... */
	अगर (main_start_services) {
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

#अगर_घोषित __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	अगर (proc) {
		जबतक ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		मुक्त(proc);
	}

#पूर्ण_अगर

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	अगर (regen && म_भेद(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	वापस निकास_सफल;
}
