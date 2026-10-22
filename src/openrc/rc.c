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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <libgen.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/utsname.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <termios.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "rc-logger.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "plugin.h"
#అన్తర్భావ "version.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "a:no:s:S" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "no-stop", 0, ఖాళీ, 'n' },
	{ "override",    1, ఖాళీ, 'o' },
	{ "service",     1, ఖాళీ, 's' },
	{ "sys",         0, ఖాళీ, 'S' },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#నిర్వచించు INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#నిర్వచించు INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#నిర్వచించు DEVBOOT			"/dev/.rcboot"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిర RC_STRINGLIST *main_hotplugged_services;
స్థిర RC_STRINGLIST *main_stop_services;
స్థిర RC_STRINGLIST *main_ప్రారంభం_services;
స్థిర RC_STRINGLIST *main_types_nw;
స్థిర RC_STRINGLIST *main_types_nwua;
స్థిర RC_DEPTREE *main_deptree;
స్థిర అక్షరం *runlevel;

నిర్మాణం termios *termios_orig = ఖాళీ;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

స్థిర శూన్య
clean_failed(శూన్య)
{
	DIR *dp;
	నిర్మాణం dirent *d;

	/* Clean the failed services state dir now */
	ఒకవేళ (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		తిరిగి;

	అయితే ((d = readdir(dp))) {
		ఒకవేళ (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			కొనసాగు;

		ఒకవేళ (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

స్థిర శూన్య
cleanup(శూన్య)
{
	పూర్ణాంకం svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	ఒకవేళ (!rc_లోపల_logger && !rc_లోపల_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		ఒకవేళ (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			విడుదల(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_సురక్షిత(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		విడుదల(p);
	}
	LIST_FOREACH_సురక్షిత(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		విడుదల(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_ప్రారంభం_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	విడుదల(runlevel);
}

స్థిర అక్షరం
read_కీలకం(bool ఖండం)
{
	నిర్మాణం termios termios;
	అక్షరం c = 0;
	పూర్ణాంకం fd = STDIN_FILENO;

	ఒకవేళ (!isatty(fd))
		తిరిగి false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	ఒకవేళ (!termios_orig) {
		termios_orig = xmalloc(పరిమాణం(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	ఒకవేళ (ఖండం)
		termios.c_cc[VMIN] = 1;
	లేకపోతే {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	ఒకవేళ (పఠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	తిరిగి c;
}

స్థిర bool
want_interactive(శూన్య)
{
	అక్షరం c;
	స్థిర bool gotinteractive;
	స్థిర bool interactive;

	ఒకవేళ (rc_yesno(getenv("EINFO_QUIET")))
		తిరిగి false;
	ఒకవేళ (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	ఒకవేళ (!interactive)
		తిరిగి false;
	c = read_కీలకం(false);
	తిరిగి (c == 'I' || c == 'i') ? true : false;
}

స్థిర శూన్య
mark_interactive(శూన్య)
{
	FILE *fp;

	ఒకవేళ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

స్థిర శూన్య
నిరంతరం_కార్యక్రమం(స్థిరరాశి అక్షరం *prog)
{
	నిర్మాణం sigaction sa;
	sigset_t నిండిన;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&నిండిన);
	sigprocmask(SIG_SETMASK, &నిండిన, &old);
	pid = fork();

	ఒకవేళ (pid == -1) {
		rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_ప్రారంభం_బహిర్గతం, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	ఒకవేళ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ఖాళీ);
		sigaction(SIGHUP, &sa, ఖాళీ);
		sigaction(SIGINT, &sa, ఖాళీ);
		sigaction(SIGQUIT, &sa, ఖాళీ);
		sigaction(SIGTERM, &sa, ఖాళీ);
		sigaction(SIGUSR1, &sa, ఖాళీ);
		sigaction(SIGWINCH, &sa, ఖాళీ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ఖాళీ);

		ఒకవేళ (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (అక్షరం *)ఖాళీ);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, ఖాళీ);
	ఒకవేళ (rc_waitpid(pid) == -1) {
		rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_ప్రారంభం_బహిర్గతం, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

స్థిర శూన్య
open_shell(శూన్య)
{
	స్థిరరాశి అక్షరం *shell;
	నిర్మాణం passwd *pw;

#నిర్వచితమైతే __linux__
	స్థిరరాశి అక్షరం *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ఒకవేళ (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (అక్షరం *) ఖాళీ);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#అయితే_ముగింపు

	shell = rc_conf_విలువ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ఒకవేళ (shell == ఖాళీ) {
		shell = getenv("SHELL");
		ఒకవేళ (shell == ఖాళీ) {
			pw = getpwuid(getuid());
			ఒకవేళ (pw)
				shell = pw->pw_shell;
			ఒకవేళ (shell == ఖాళీ)
				shell = "/bin/sh";
		}
	}
	నిరంతరం_కార్యక్రమం(shell);
}

స్థిర bool
సమితి_krunlevel(స్థిరరాశి అక్షరం *level)
{
	FILE *fp;

	ఒకవేళ (rc_is_user())
		తిరిగి true;

	ఒకవేళ (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		ఒకవేళ (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		తిరిగి false;
	}

	ఒకవేళ (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		తిరిగి false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	తిరిగి true;
}

స్థిర అక్షరం *get_krunlevel(శూన్య)
{
	అక్షరం *బఫరు = ఖాళీ;
	FILE *fp;
	size_t i = 0;

	ఒకవేళ (rc_is_user())
		తిరిగి ఖాళీ;

	ఒకవేళ (access(RC_KRUNLEVEL, F_OK) != 0)
		తిరిగి ఖాళీ;
	ఒకవేళ (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (xgetline(&బఫరు, &i, fp) == -1) {
		విడుదల(బఫరు);
		బఫరు = ఖాళీ;
	}

	fclose(fp);
	తిరిగి బఫరు;
}

స్థిర శూన్య
కూడు_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(పరిమాణం(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_చొప్పించు_తల(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, ఖాళీ);
}

స్థిర శూన్య
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		ఒకవేళ (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_చొప్పించు_తల(&free_these_pids, p, entries);
			ఆపు;
		}
	}
	/* only call free if we're not inside a signal handler */
	ఒకవేళ (!inside_signal) {
		LIST_FOREACH_సురక్షిత(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			విడుదల(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, ఖాళీ);
}

స్థిర శూన్య
wait_for_services(శూన్య)
{
	కొరకు (;;) {
		అయితే (waitpid(0, 0, 0) != -1)
			;
		ఒకవేళ (errno != EINTR)
			ఆపు;
	}
}

స్థిర శూన్య
handle_signal(పూర్ణాంకం sig)
{
	పూర్ణాంకం serrno = errno;
	స్థిరరాశి అక్షరం *signame = ఖాళీ;
	pid_t pid;
	RC_PID *pi;
	పూర్ణాంకం స్థితి_2 = 0;
	నిర్మాణం winsize ws;
	sigset_t sset;

	ఎంపిక (sig) {
	స్థితి SIGCHLD:
		చేయి {
			pid = waitpid(-1, &స్థితి_2, WNOHANG);
			ఒకవేళ (pid < 0) {
				ఒకవేళ (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				తిరిగి;
			}
		} అయితే (!WIFEXITED(స్థితి_2) && !WIFSIGNALED(స్థితి_2));

		/* Remove that pid from our list */
		ఒకవేళ (pid > 0)
			remove_pid(pid, true);
		ఆపు;

	స్థితి SIGWINCH:
		ఒకవేళ (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		ఆపు;

	స్థితి SIGINT:
		ఒకవేళ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	స్థితి SIGTERM:
		ఒకవేళ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	స్థితి SIGQUIT:
		ఒకవేళ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	స్థితి SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, ఖాళీ);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_నిరంతరం(RC_HOOK_ABORT, ఖాళీ);

		నిష్క్రమించు(EXIT_FAILURE);
		/* NOTREACHED */

	అప్రమేయ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

స్థిర శూన్య
do_sysinit(శూన్య)
{
	నిర్మాణం utsname uts;
	స్థిరరాశి అక్షరం *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	ఒకవేళ (access(INITEARLYSH, F_OK) == 0)
		నిరంతరం_కార్యక్రమం(INITEARLYSH);

	uname(&uts);
	ఒకవేళ (!rc_yesno(getenv("EINFO_QUIET"))) {
		రాయి("\n   %sOpenRC %s" సంస్కరణ "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_సాధారణ), ecolor(ECOLOR_BRACKET));
#నిర్వచితమైతే BRANDING
		రాయి(BRANDING " (%s)", uts.machine);
#లేకపోతే
		రాయి("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#అయితే_ముగింపు
	}

	ఒకవేళ ((sys = rc_sys()))
		రాయి(" [%s]", sys);

	రాయి("%s\n\n", ecolor(ECOLOR_సాధారణ));

	ఒకవేళ (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		రాయి("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_సాధారణ));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	నిరంతరం_కార్యక్రమం(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	ఒకవేళ ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	ఒకవేళ ((main_deptree = rc_deptree_ఎక్కించు_2(false, ఖాళీ)) == ఖాళీ)
		eerrorx("failed to load deptree");
}

స్థిర bool
runlevel_config(స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *level)
{
	అక్షరం *init = rc_service_resolve(service);
	అక్షరం *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	విడుదల(conf);
	విడుదల(init);
	తిరిగి retval;
}

స్థిర శూన్య
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *ప్రారంభం_services,
				 స్థిరరాశి RC_STRINGLIST *stop_services, స్థిరరాశి RC_DEPTREE *deptree,
				 స్థిరరాశి అక్షరం *newlevel, bool parallel, bool going_కిందికి)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = ఖాళీ;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	ఒకవేళ (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_కూడు(types_nw, "needsme");
		rc_stringlist_కూడు(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_విభజించు(rc_conf_విలువ("rc_nostop"), " ");
	TAILQ_FOREACH_తిరగేయి(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->విలువ);
		ఒకవేళ (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			కొనసాగు;

		/* Sometimes we don't ever want to stop a service. */
		ఒకవేళ (rc_stringlist_కనుగొను(nostop, service->విలువ)) {
			rc_service_mark(service->విలువ, RC_SERVICE_FAILED);
			కొనసాగు;
		}
		kwords = rc_deptree_depend(deptree, service->విలువ, "keyword");
		ఒకవేళ (rc_stringlist_కనుగొను(kwords, "-stop") ||
		    rc_stringlist_కనుగొను(kwords, "nostop") ||
		    (going_కిందికి &&
			(rc_stringlist_కనుగొను(kwords, "-shutdown") ||
			    rc_stringlist_కనుగొను(kwords, "noshutdown"))))
			nstop = true;
		లేకపోతే
			nstop = false;
		rc_stringlist_free(kwords);
		ఒకవేళ (nstop) {
			rc_service_mark(service->విలువ, RC_SERVICE_FAILED);
			కొనసాగు;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		ఒకవేళ (crashed &&
		    rc_service_daemons_crashed(service->విలువ))
			వెళ్ళు stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_కనుగొను(ప్రారంభం_services, service->విలువ);
		ఒకవేళ (svc1) {
			ఒకవేళ (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				ఒకవేళ (!runlevel_config(service->విలువ,runlevel) &&
				    !runlevel_config(service->విలువ,newlevel))
					కొనసాగు;
			}
			లేకపోతే
				కొనసాగు;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		ఒకవేళ (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_కూడు(tmplist, service->విలువ);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = ఖాళీ;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_కనుగొను(ప్రారంభం_services,
				    svc1->విలువ);
				ఒకవేళ (svc2)
					ఆపు;
			}
			rc_stringlist_free(deporder);

			ఒకవేళ (svc2)
				కొనసాగు;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->విలువ);
		ఒకవేళ (pid > 0) {
			కూడు_pid(pid);
			ఒకవేళ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	ఒకవేళ (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

స్థిర శూన్య
do_ప్రారంభం_services(స్థిరరాశి RC_STRINGLIST *ప్రారంభం_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	ఒకవేళ (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	ఒకవేళ (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, ప్రారంభం_services, entries) {
		state = rc_service_state(service->విలువ);
		ఒకవేళ (state & RC_SERVICE_FAILED)
			కొనసాగు;
		ఒకవేళ (!(state & RC_SERVICE_STOPPED)) {
			ఒకవేళ (crashed &&
			    rc_service_daemons_crashed(service->విలువ))
				rc_service_mark(service->విలువ,
				    RC_SERVICE_STOPPED);
			లేకపోతే
			    కొనసాగు;
		}
		ఒకవేళ (!interactive)
			interactive = want_interactive();

		ఒకవేళ (interactive) {
	interactive_retry:
			రాయి("\n");
			einfo("About to start the service %s",
			    service->విలువ);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			ఎంపిక (read_కీలకం(true)) {
			స్థితి '1': ఆపు;
			స్థితి '2': కొనసాగు;
			స్థితి '3': interactive = false; ఆపు;
			స్థితి '4': open_shell(); వెళ్ళు interactive_retry;
			అప్రమేయ: వెళ్ళు interactive_option;
			}
		}

		pid = service_ప్రారంభం(service->విలువ);
		ఒకవేళ (pid == -1)
			ఆపు;
		/* Remember the pid if we're running in parallel */
		ఒకవేళ (pid > 0) {
			కూడు_pid(pid);
			ఒకవేళ (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	ఒకవేళ (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	లేకపోతే
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#నిర్వచితమైతే RC_DEBUG
RC_NORETURN స్థిర శూన్య
handle_bad_signal(పూర్ణాంకం sig)
{
	అక్షరం pid[10];
	పూర్ణాంకం స్థితి_2;
	pid_t crashed_pid = getpid();

	ఎంపిక (fork()) {
	స్థితి -1:
		_exit(sig);
		/* NOTREACHED */
	స్థితి 0:
		sprintf(pid, "%i", crashed_pid);
		రాయి("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", ఖాళీ));
		/* NOTREACHED */
	అప్రమేయ:
		wait(&స్థితి_2);
	}
	_exit(1);
	/* NOTREACHED */
}
#అయితే_ముగింపు

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	స్థిరరాశి అక్షరం *bootlevel = ఖాళీ;
	అక్షరం *newlevel = ఖాళీ;
	స్థిరరాశి అక్షరం *systype = ఖాళీ;
	RC_STRINGLIST *deporder = ఖాళీ;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_కిందికి = false;
	పూర్ణాంకం depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	అక్షరం *krunlevel = ఖాళీ;
	స్థిరరాశి అక్షరం *workingdir = "/";
	అక్షరం *pidstr = ఖాళీ;
	పూర్ణాంకం opt;
	bool parallel;
	పూర్ణాంకం regen = 0;
	bool nostop = false;
#నిర్వచితమైతే __linux__
	అక్షరం *proc;
	అక్షరం *p;
	అక్షరం *token;
#అయితే_ముగింపు

#నిర్వచితమైతే RC_DEBUG
	signal_అమరిక(SIGBUS, handle_bad_signal);
	signal_అమరిక(SIGILL, handle_bad_signal);
	signal_అమరిక(SIGSEGV, handle_bad_signal);
#అయితే_ముగింపు

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	ఒకవేళ (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	ఒకవేళ (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి 'n':
			nostop = true;
			ఆపు;
		స్థితి 'o':
			ఒకవేళ (*optarg == '\0')
				optarg = ఖాళీ;
			ఒకవేళ (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				నిష్క్రమించు(EXIT_FAILURE);
			}
			ఒకవేళ (rc_is_user())
				eerrorx("user services do not currently support --override");
			ఒకవేళ (!సమితి_krunlevel(optarg))
				నిష్క్రమించు(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			నిష్క్రమించు(EXIT_SUCCESS);
			/* NOTREACHED */
		స్థితి 's':
			newlevel = rc_service_resolve(optarg);
			ఒకవేళ (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		స్థితి 'S':
			systype = rc_sys();
			ఒకవేళ (systype)
				రాయి("%s\n", systype);
			నిష్క్రమించు(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ఒకవేళ (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ఒకవేళ (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	ఒకవేళ (newlevel) {
		ఒకవేళ (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		ఒకవేళ (strcmp(newlevel, "reboot") == 0) {
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
	విడుదల(pidstr);

	/* Create a list of all services which should be started for the new or
	* current runlevel including those in boot, sysinit and hotplugged
	* runlevels.  Clearly, some of these will already be started so we
	* won't actually be starting them all.
	*/
	bootlevel = getenv("RC_BOOTLEVEL");
	runlevel = default_runlevel();

	rc_logger_open(newlevel ? newlevel : runlevel);

	/* Setup a signal handler */
	signal_అమరిక(SIGINT, handle_signal);
	signal_అమరిక(SIGQUIT, handle_signal);
	signal_అమరిక(SIGTERM, handle_signal);
	signal_అమరిక(SIGUSR1, handle_signal);
	signal_అమరిక(SIGWINCH, handle_signal);

	/* Run any special sysinit foo */
	ఒకవేళ (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		విడుదల(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_ఎక్కించు();

	/* Now we start handling our children */
	signal_అమరిక(SIGCHLD, handle_signal);

	ఒకవేళ (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_కిందికి = true;
		ఒకవేళ (access(RC_KRUNLEVEL, F_OK) != 0)
			సమితి_krunlevel(runlevel);
		rc_runlevel_సమితి(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} లేకపోతే {
		/* We should not use krunlevel in sysinit or boot runlevels */
		ఒకవేళ (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			ఒకవేళ (krunlevel) {
				newlevel = krunlevel;
				సమితి_krunlevel(ఖాళీ);
			}
		}

		ఒకవేళ (newlevel) {
			ఒకవేళ (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#నిర్వచితమైతే __linux__
			ఒకవేళ (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				ఒకవేళ (p == ఖాళీ)
					p = rc_proc_getent("softlevel");
				ఒకవేళ (p != ఖాళీ) {
					సమితి_krunlevel(p);
					విడుదల(p);
				}
			}
#అయితే_ముగింపు
		}
	}

	ఒకవేళ (going_కిందికి) {
#నిర్వచితమైతే __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#అయితే_ముగింపు

		rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_STOP_లోపల, newlevel);
	} లేకపోతే {
		rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_STOP_లోపల, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	ఒకవేళ (newlevel && strcmp(runlevel, newlevel) != 0 && !going_కిందికి) {
		ఒకవేళ (!rc_runlevel_exists(newlevel)) {
			rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_STOP_బహిర్గతం, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	ఒకవేళ ((main_deptree = rc_deptree_ఎక్కించు_2(0, &regen)) == ఖాళీ) {
		rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_STOP_బహిర్గతం, runlevel);
		eerrorx("failed to load deptree");
	}

	ఒకవేళ (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	ఒకవేళ (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
		rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_STOP_బహిర్గతం, runlevel);
		eerrorx("%s: failed to create stopping dir '%s/rc.stopping': %s", applet, rc_svcdir(), strerror(errno));
	}

	/* Create a list of all services which we could stop (assuming
	* they won't be active in the new or current runlevel) including
	* all those services which have been started, are inactive or
	* are currently starting.  Clearly, some of these will be listed
	* in the new or current runlevel so we won't actually be stopping
	* them all.
	*/
	main_stop_services = rc_services_లోపల_state(RC_SERVICE_STARTED);
	tmplist = rc_services_లోపల_state(RC_SERVICE_INACTIVE);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	విడుదల(tmplist);
	tmplist = rc_services_లోపల_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	విడుదల(tmplist);
	ఒకవేళ (main_stop_services)
		rc_stringlist_క్రమీకరించు(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_కూడు(main_types_nwua, "ineed");
	rc_stringlist_కూడు(main_types_nwua, "iwant");
	rc_stringlist_కూడు(main_types_nwua, "iuse");
	rc_stringlist_కూడు(main_types_nwua, "iafter");

	ఒకవేళ (main_stop_services) {
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
	main_hotplugged_services = rc_services_లోపల_state(RC_SERVICE_HOTPLUGGED);
	main_ప్రారంభం_services = rc_services_లోపల_runlevel_stacked(newlevel ?
	    newlevel : runlevel);
	ఒకవేళ (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_లోపల_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_ప్రారంభం_services, tmplist, entries);
		విడుదల(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		ఒకవేళ (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			ఒకవేళ (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_లోపల_runlevel(bootlevel);
				TAILQ_CONCAT(main_ప్రారంభం_services, tmplist, entries);
				విడుదల(tmplist);
			}
			ఒకవేళ (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_ప్రారంభం_services,
					service->విలువ);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	ఒకవేళ (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_ప్రారంభం_services, main_stop_services, main_deptree, newlevel, parallel, going_కిందికి);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_STOP_బహిర్గతం,
	    going_కిందికి ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	ఒకవేళ (newlevel) {
		rc_runlevel_సమితి(newlevel);
		విడుదల(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#నిర్వచితమైతే __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	ఒకవేళ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#అయితే_ముగింపు

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_ప్రారంభం_లోపల, runlevel);

	/* Re-add our hotplugged services if they stopped */
	ఒకవేళ (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->విలువ, RC_SERVICE_HOTPLUGGED);

#నిర్వచితమైతే __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	ఒకవేళ (proc) {
		అయితే ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		విడుదల(proc);
	}
#అయితే_ముగింపు

	/* If we have a list of services to start then... */
	ఒకవేళ (main_ప్రారంభం_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_తిరగేయి(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *నిరంతరం_services = rc_services_లోపల_runlevel(rlevel->విలువ);

			/* Start those services. */
			rc_stringlist_క్రమీకరించు(&నిరంతరం_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, నిరంతరం_services, rlevel->విలువ, depoptions | RC_DEP_ప్రారంభం);
			rc_stringlist_free(నిరంతరం_services);
			నిరంతరం_services = deporder;
			do_ప్రారంభం_services(నిరంతరం_services, parallel);

			/* Wait for our services to finish */
			wait_for_services();

			/* Free the list of services, we're done with it. */
			rc_stringlist_free(నిరంతరం_services);
		}
		rc_stringlist_free(runlevel_chain);
	}

#నిర్వచితమైతే __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as stopped so that our records reflect
	 * reality.	 */
	proc = p = rc_proc_getent("noinit");
	ఒకవేళ (proc) {
		అయితే ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		విడుదల(proc);
	}

#అయితే_ముగింపు

	rc_plugin_నిరంతరం(RC_HOOK_RUNLEVEL_ప్రారంభం_బహిర్గతం, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	ఒకవేళ (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	తిరిగి EXIT_SUCCESS;
}
