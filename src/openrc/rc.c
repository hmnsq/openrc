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

ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "no-stop", 0, रिक्त, 'n' },
	{ "override",    1, रिक्त, 'o' },
	{ "service",     1, रिक्त, 's' },
	{ "sys",         0, रिक्त, 'S' },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ध्रुव वर्ण *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

ध्रुव वर्ण *applet = रिक्त;
स्थिर RC_STRINGLIST *main_hotplugged_services;
स्थिर RC_STRINGLIST *main_stop_services;
स्थिर RC_STRINGLIST *main_start_services;
स्थिर RC_STRINGLIST *main_types_nw;
स्थिर RC_STRINGLIST *main_types_nwua;
स्थिर RC_DEPTREE *main_deptree;
स्थिर वर्ण *runlevel;

संरचना termios *termios_orig = रिक्त;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

स्थिर शून्य
clean_failed(शून्य)
{
	DIR *dp;
	संरचना dirent *d;

	/* Clean the failed services state dir now */
	सचे (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		पटिनिवत्त;

	याव ((d = readdir(dp))) {
		सचे (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			अनुवर्त;

		सचे (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

स्थिर शून्य
cleanup(शून्य)
{
	पूर्णाङ्क svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	सचे (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		सचे (termios_orig) {
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

स्थिर वर्ण
read_कुञ्चिका(bool block)
{
	संरचना termios termios;
	वर्ण c = 0;
	पूर्णाङ्क fd = STDIN_FILENO;

	सचे (!isatty(fd))
		पटिनिवत्त false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	सचे (!termios_orig) {
		termios_orig = xmalloc(परिमाण(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	सचे (block)
		termios.c_cc[VMIN] = 1;
	अञ्ञथा {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	सचे (पठ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	पटिनिवत्त c;
}

स्थिर bool
want_interactive(शून्य)
{
	वर्ण c;
	स्थिर bool gotinteractive;
	स्थिर bool interactive;

	सचे (rc_yesno(getenv("EINFO_QUIET")))
		पटिनिवत्त false;
	सचे (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	सचे (!interactive)
		पटिनिवत्त false;
	c = read_कुञ्चिका(false);
	पटिनिवत्त (c == 'I' || c == 'i') ? true : false;
}

स्थिर शून्य
mark_interactive(शून्य)
{
	FILE *fp;

	सचे ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

स्थिर शून्य
run_program(ध्रुव वर्ण *prog)
{
	संरचना sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, परिमाण(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	सचे (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	सचे (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, रिक्त);
		sigaction(SIGHUP, &sa, रिक्त);
		sigaction(SIGINT, &sa, रिक्त);
		sigaction(SIGQUIT, &sa, रिक्त);
		sigaction(SIGTERM, &sa, रिक्त);
		sigaction(SIGUSR1, &sa, रिक्त);
		sigaction(SIGWINCH, &sa, रिक्त);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, रिक्त);

		सचे (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (वर्ण *)रिक्त);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, रिक्त);
	सचे (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

स्थिर शून्य
open_shell(शून्य)
{
	ध्रुव वर्ण *shell;
	संरचना passwd *pw;

#ifdef __linux__
	ध्रुव वर्ण *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	सचे (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (वर्ण *) रिक्त);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_मानम्("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	सचे (shell == रिक्त) {
		shell = getenv("SHELL");
		सचे (shell == रिक्त) {
			pw = getpwuid(getuid());
			सचे (pw)
				shell = pw->pw_shell;
			सचे (shell == रिक्त)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

स्थिर bool
set_krunlevel(ध्रुव वर्ण *level)
{
	FILE *fp;

	सचे (rc_is_user())
		पटिनिवत्त true;

	सचे (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		सचे (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		पटिनिवत्त false;
	}

	सचे (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		पटिनिवत्त false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	पटिनिवत्त true;
}

स्थिर वर्ण *get_krunlevel(शून्य)
{
	वर्ण *buffer = रिक्त;
	FILE *fp;
	size_t i = 0;

	सचे (rc_is_user())
		पटिनिवत्त रिक्त;

	सचे (access(RC_KRUNLEVEL, F_OK) != 0)
		पटिनिवत्त रिक्त;
	सचे (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		पटिनिवत्त रिक्त;
	}

	सचे (xgetline(&buffer, &i, fp) == -1) {
		मुक्त(buffer);
		buffer = रिक्त;
	}

	fclose(fp);
	पटिनिवत्त buffer;
}

स्थिर शून्य
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(परिमाण(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_निवेशय_HEAD(&service_pids, p, entries);
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
		सचे (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_निवेशय_HEAD(&free_these_pids, p, entries);
			भङ्ग;
		}
	}
	/* only call free if we're not inside a signal handler */
	सचे (!inside_signal) {
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
	अत्थाय (;;) {
		याव (waitpid(0, 0, 0) != -1)
			;
		सचे (errno != EINTR)
			भङ्ग;
	}
}

स्थिर शून्य
handle_signal(पूर्णाङ्क sig)
{
	पूर्णाङ्क serrno = errno;
	ध्रुव वर्ण *signame = रिक्त;
	pid_t pid;
	RC_PID *pi;
	पूर्णाङ्क status = 0;
	संरचना winsize ws;
	sigset_t sset;

	विकल्प (sig) {
	पक्ष SIGCHLD:
		करोहि {
			pid = waitpid(-1, &status, WNOHANG);
			सचे (pid < 0) {
				सचे (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				पटिनिवत्त;
			}
		} याव (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		सचे (pid > 0)
			remove_pid(pid, true);
		भङ्ग;

	पक्ष SIGWINCH:
		सचे (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		भङ्ग;

	पक्ष SIGINT:
		सचे (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	पक्ष SIGTERM:
		सचे (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	पक्ष SIGQUIT:
		सचे (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	पक्ष SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, रिक्त);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, रिक्त);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	स्वभाव:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

स्थिर शून्य
do_sysinit(शून्य)
{
	संरचना utsname uts;
	ध्रुव वर्ण *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	सचे (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	सचे (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#अञ्ञथा
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	सचे ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	सचे (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	सचे ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	सचे ((main_deptree = _rc_deptree_load(false, रिक्त)) == रिक्त)
		eerrorx("failed to load deptree");
}

स्थिर bool
runlevel_config(ध्रुव वर्ण *service, ध्रुव वर्ण *level)
{
	वर्ण *init = rc_service_resolve(service);
	वर्ण *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	मुक्त(conf);
	मुक्त(init);
	पटिनिवत्त retval;
}

स्थिर शून्य
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 ध्रुव RC_STRINGLIST *stop_services, ध्रुव RC_DEPTREE *deptree,
				 ध्रुव वर्ण *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = रिक्त;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	सचे (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_मानम्("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->मानम्);
		सचे (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			अनुवर्त;

		/* Sometimes we don't ever want to stop a service. */
		सचे (rc_stringlist_find(nostop, service->मानम्)) {
			rc_service_mark(service->मानम्, RC_SERVICE_FAILED);
			अनुवर्त;
		}
		kwords = rc_deptree_depend(deptree, service->मानम्, "keyword");
		सचे (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		अञ्ञथा
			nstop = false;
		rc_stringlist_free(kwords);
		सचे (nstop) {
			rc_service_mark(service->मानम्, RC_SERVICE_FAILED);
			अनुवर्त;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		सचे (crashed &&
		    rc_service_daemons_crashed(service->मानम्))
			गच्छ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->मानम्);
		सचे (svc1) {
			सचे (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				सचे (!runlevel_config(service->मानम्,runlevel) &&
				    !runlevel_config(service->मानम्,newlevel))
					अनुवर्त;
			}
			अञ्ञथा
				अनुवर्त;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		सचे (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->मानम्);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = रिक्त;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->मानम्);
				सचे (svc2)
					भङ्ग;
			}
			rc_stringlist_free(deporder);

			सचे (svc2)
				अनुवर्त;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->मानम्);
		सचे (pid > 0) {
			add_pid(pid);
			सचे (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	सचे (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

स्थिर शून्य
do_start_services(ध्रुव RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	सचे (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	सचे (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->मानम्);
		सचे (state & RC_SERVICE_FAILED)
			अनुवर्त;
		सचे (!(state & RC_SERVICE_STOPPED)) {
			सचे (crashed &&
			    rc_service_daemons_crashed(service->मानम्))
				rc_service_mark(service->मानम्,
				    RC_SERVICE_STOPPED);
			अञ्ञथा
			    अनुवर्त;
		}
		सचे (!interactive)
			interactive = want_interactive();

		सचे (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->मानम्);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			विकल्प (read_कुञ्चिका(true)) {
			पक्ष '1': भङ्ग;
			पक्ष '2': अनुवर्त;
			पक्ष '3': interactive = false; भङ्ग;
			पक्ष '4': open_shell(); गच्छ interactive_retry;
			स्वभाव: गच्छ interactive_option;
			}
		}

		pid = service_start(service->मानम्);
		सचे (pid == -1)
			भङ्ग;
		/* Remember the pid if we're running in parallel */
		सचे (pid > 0) {
			add_pid(pid);
			सचे (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	सचे (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	अञ्ञथा
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN स्थिर शून्य
handle_bad_signal(पूर्णाङ्क sig)
{
	वर्ण pid[10];
	पूर्णाङ्क status;
	pid_t crashed_pid = getpid();

	विकल्प (fork()) {
	पक्ष -1:
		_exit(sig);
		/* NOTREACHED */
	पक्ष 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", रिक्त));
		/* NOTREACHED */
	स्वभाव:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	ध्रुव वर्ण *bootlevel = रिक्त;
	वर्ण *newlevel = रिक्त;
	ध्रुव वर्ण *systype = रिक्त;
	RC_STRINGLIST *deporder = रिक्त;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	पूर्णाङ्क depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	वर्ण *krunlevel = रिक्त;
	ध्रुव वर्ण *workingdir = "/";
	वर्ण *pidstr = रिक्त;
	पूर्णाङ्क opt;
	bool parallel;
	पूर्णाङ्क regen = 0;
	bool nostop = false;
#ifdef __linux__
	वर्ण *proc;
	वर्ण *p;
	वर्ण *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	सचे (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	सचे (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	याव ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष 'n':
			nostop = true;
			भङ्ग;
		पक्ष 'o':
			सचे (*optarg == '\0')
				optarg = रिक्त;
			सचे (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			सचे (rc_is_user())
				eerrorx("user services do not currently support --override");
			सचे (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		पक्ष 's':
			newlevel = rc_service_resolve(optarg);
			सचे (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		पक्ष 'S':
			systype = rc_sys();
			सचे (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	सचे (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	सचे (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	सचे (newlevel) {
		सचे (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		सचे (strcmp(newlevel, "reboot") == 0) {
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
	मुक्त(pidstr);

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
	सचे (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		मुक्त(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	सचे (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		सचे (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} अञ्ञथा {
		/* We should not use krunlevel in sysinit or boot runlevels */
		सचे (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			सचे (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(रिक्त);
			}
		}

		सचे (newlevel) {
			सचे (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			सचे (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				सचे (p == रिक्त)
					p = rc_proc_getent("softlevel");
				सचे (p != रिक्त) {
					set_krunlevel(p);
					मुक्त(p);
				}
			}
#endif
		}
	}

	सचे (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} अञ्ञथा {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	सचे (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		सचे (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	सचे ((main_deptree = _rc_deptree_load(0, &regen)) == रिक्त) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	सचे (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	सचे (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	मुक्त(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	मुक्त(tmplist);
	सचे (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	सचे (main_stop_services) {
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
	सचे (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		मुक्त(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		सचे (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			सचे (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				मुक्त(tmplist);
			}
			सचे (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->मानम्);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	सचे (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	सचे (newlevel) {
		rc_runlevel_set(newlevel);
		मुक्त(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	सचे (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	सचे (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->मानम्, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	सचे (proc) {
		याव ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		मुक्त(proc);
	}
#endif

	/* If we have a list of services to start then... */
	सचे (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->मानम्);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->मानम्, depoptions | RC_DEP_START);
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
	सचे (proc) {
		याव ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		मुक्त(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	सचे (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	पटिनिवत्त EXIT_SUCCESS;
}
