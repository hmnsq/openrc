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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <libgen.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/utsname.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <termios.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "rc-logger.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "plugin.h"
#ଅନ୍ତର୍ଭାଵ "version.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "a:no:s:S" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "no-stop", 0, ଖାଲି, 'n' },
	{ "override",    1, ଖାଲି, 'o' },
	{ "service",     1, ଖାଲି, 's' },
	{ "sys",         0, ଖାଲି, 'S' },
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
ଅଚଳ ଅକ୍ଷର *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ସ୍ଥିର RC_STRINGLIST *main_hotplugged_services;
ସ୍ଥିର RC_STRINGLIST *main_stop_services;
ସ୍ଥିର RC_STRINGLIST *main_start_services;
ସ୍ଥିର RC_STRINGLIST *main_types_nw;
ସ୍ଥିର RC_STRINGLIST *main_types_nwua;
ସ୍ଥିର RC_DEPTREE *main_deptree;
ସ୍ଥିର ଅକ୍ଷର *runlevel;

ସଂରଚନା termios *termios_orig = ଖାଲି;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

ସ୍ଥିର ଶୂନ୍ୟ
clean_failed(ଶୂନ୍ୟ)
{
	DIR *dp;
	ସଂରଚନା dirent *d;

	/* Clean the failed services state dir now */
	ଯଦି (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		ଫେରନ୍ତୁ;

	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଯଦି (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			ଜାରି;

		ଯଦି (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

ସ୍ଥିର ଶୂନ୍ୟ
cleanup(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	ଯଦି (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		ଯଦି (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			ମୁକ୍ତ(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ମୁକ୍ତ(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		ମୁକ୍ତ(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	ମୁକ୍ତ(runlevel);
}

ସ୍ଥିର ଅକ୍ଷର
read_ଚାବି(bool block)
{
	ସଂରଚନା termios termios;
	ଅକ୍ଷର c = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = STDIN_FILENO;

	ଯଦି (!isatty(fd))
		ଫେରନ୍ତୁ false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	ଯଦି (!termios_orig) {
		termios_orig = xmalloc(ଆକାର(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	ଯଦି (block)
		termios.c_cc[VMIN] = 1;
	ଅନ୍ୟଥା {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	ଯଦି (ପଠ(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	ଫେରନ୍ତୁ c;
}

ସ୍ଥିର bool
want_interactive(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର c;
	ସ୍ଥିର bool gotinteractive;
	ସ୍ଥିର bool interactive;

	ଯଦି (rc_yesno(getenv("EINFO_QUIET")))
		ଫେରନ୍ତୁ false;
	ଯଦି (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	ଯଦି (!interactive)
		ଫେରନ୍ତୁ false;
	c = read_ଚାବି(false);
	ଫେରନ୍ତୁ (c == 'I' || c == 'i') ? true : false;
}

ସ୍ଥିର ଶୂନ୍ୟ
mark_interactive(ଶୂନ୍ୟ)
{
	FILE *fp;

	ଯଦି ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

ସ୍ଥିର ଶୂନ୍ୟ
run_program(ଅଚଳ ଅକ୍ଷର *prog)
{
	ସଂରଚନା sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, ଆକାର(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	ଯଦି (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	ଯଦି (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ଖାଲି);
		sigaction(SIGHUP, &sa, ଖାଲି);
		sigaction(SIGINT, &sa, ଖାଲି);
		sigaction(SIGQUIT, &sa, ଖାଲି);
		sigaction(SIGTERM, &sa, ଖାଲି);
		sigaction(SIGUSR1, &sa, ଖାଲି);
		sigaction(SIGWINCH, &sa, ଖାଲି);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ଖାଲି);

		ଯଦି (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (ଅକ୍ଷର *)ଖାଲି);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, ଖାଲି);
	ଯଦି (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ
open_shell(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *shell;
	ସଂରଚନା passwd *pw;

#ifdef __linux__
	ଅଚଳ ଅକ୍ଷର *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	ଯଦି (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (ଅକ୍ଷର *) ଖାଲି);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_ମୂଲ୍ୟ("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	ଯଦି (shell == ଖାଲି) {
		shell = getenv("SHELL");
		ଯଦି (shell == ଖାଲି) {
			pw = getpwuid(getuid());
			ଯଦି (pw)
				shell = pw->pw_shell;
			ଯଦି (shell == ଖାଲି)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

ସ୍ଥିର bool
set_krunlevel(ଅଚଳ ଅକ୍ଷର *level)
{
	FILE *fp;

	ଯଦି (rc_is_user())
		ଫେରନ୍ତୁ true;

	ଯଦି (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		ଯଦି (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ଫେରନ୍ତୁ false;
	}

	ଯଦି (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ଫେରନ୍ତୁ false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	ଫେରନ୍ତୁ true;
}

ସ୍ଥିର ଅକ୍ଷର *get_krunlevel(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *buffer = ଖାଲି;
	FILE *fp;
	size_t i = 0;

	ଯଦି (rc_is_user())
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (access(RC_KRUNLEVEL, F_OK) != 0)
		ଫେରନ୍ତୁ ଖାଲି;
	ଯଦି (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (xgetline(&buffer, &i, fp) == -1) {
		ମୁକ୍ତ(buffer);
		buffer = ଖାଲି;
	}

	fclose(fp);
	ଫେରନ୍ତୁ buffer;
}

ସ୍ଥିର ଶୂନ୍ୟ
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(ଆକାର(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_ଭର୍ତ୍ତି_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, ଖାଲି);
}

ସ୍ଥିର ଶୂନ୍ୟ
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		ଯଦି (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_ଭର୍ତ୍ତି_HEAD(&free_these_pids, p, entries);
			ଭାଙ୍ଗ;
		}
	}
	/* only call free if we're not inside a signal handler */
	ଯଦି (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			ମୁକ୍ତ(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, ଖାଲି);
}

ସ୍ଥିର ଶୂନ୍ୟ
wait_for_services(ଶୂନ୍ୟ)
{
	ପାଇଁ (;;) {
		ଯେତେବେଳେ (waitpid(0, 0, 0) != -1)
			;
		ଯଦି (errno != EINTR)
			ଭାଙ୍ଗ;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno;
	ଅଚଳ ଅକ୍ଷର *signame = ଖାଲି;
	pid_t pid;
	RC_PID *pi;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 0;
	ସଂରଚନା winsize ws;
	sigset_t sset;

	ବାଛ (sig) {
	ସ୍ଥିତି SIGCHLD:
		କର {
			pid = waitpid(-1, &status, WNOHANG);
			ଯଦି (pid < 0) {
				ଯଦି (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				ଫେରନ୍ତୁ;
			}
		} ଯେତେବେଳେ (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		ଯଦି (pid > 0)
			remove_pid(pid, true);
		ଭାଙ୍ଗ;

	ସ୍ଥିତି SIGWINCH:
		ଯଦି (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		ଭାଙ୍ଗ;

	ସ୍ଥିତି SIGINT:
		ଯଦି (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ସ୍ଥିତି SIGTERM:
		ଯଦି (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ସ୍ଥିତି SIGQUIT:
		ଯଦି (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	ସ୍ଥିତି SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, ଖାଲି);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, ଖାଲି);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ସ୍ଥିର ଶୂନ୍ୟ
do_sysinit(ଶୂନ୍ୟ)
{
	ସଂରଚନା utsname uts;
	ଅଚଳ ଅକ୍ଷର *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	ଯଦି (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	ଯଦି (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#ଅନ୍ୟଥା
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	ଯଦି ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	ଯଦି (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	ଯଦି ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	ଯଦି ((main_deptree = _rc_deptree_load(false, ଖାଲି)) == ଖାଲି)
		eerrorx("failed to load deptree");
}

ସ୍ଥିର bool
runlevel_config(ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *level)
{
	ଅକ୍ଷର *init = rc_service_resolve(service);
	ଅକ୍ଷର *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	ମୁକ୍ତ(conf);
	ମୁକ୍ତ(init);
	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ଶୂନ୍ୟ
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 ଅଚଳ RC_STRINGLIST *stop_services, ଅଚଳ RC_DEPTREE *deptree,
				 ଅଚଳ ଅକ୍ଷର *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = ଖାଲି;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	ଯଦି (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_ମୂଲ୍ୟ("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->ମୂଲ୍ୟ);
		ଯଦି (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			ଜାରି;

		/* Sometimes we don't ever want to stop a service. */
		ଯଦି (rc_stringlist_find(nostop, service->ମୂଲ୍ୟ)) {
			rc_service_mark(service->ମୂଲ୍ୟ, RC_SERVICE_FAILED);
			ଜାରି;
		}
		kwords = rc_deptree_depend(deptree, service->ମୂଲ୍ୟ, "keyword");
		ଯଦି (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		ଅନ୍ୟଥା
			nstop = false;
		rc_stringlist_free(kwords);
		ଯଦି (nstop) {
			rc_service_mark(service->ମୂଲ୍ୟ, RC_SERVICE_FAILED);
			ଜାରି;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		ଯଦି (crashed &&
		    rc_service_daemons_crashed(service->ମୂଲ୍ୟ))
			ଯାଅ stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->ମୂଲ୍ୟ);
		ଯଦି (svc1) {
			ଯଦି (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				ଯଦି (!runlevel_config(service->ମୂଲ୍ୟ,runlevel) &&
				    !runlevel_config(service->ମୂଲ୍ୟ,newlevel))
					ଜାରି;
			}
			ଅନ୍ୟଥା
				ଜାରି;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		ଯଦି (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->ମୂଲ୍ୟ);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = ଖାଲି;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->ମୂଲ୍ୟ);
				ଯଦି (svc2)
					ଭାଙ୍ଗ;
			}
			rc_stringlist_free(deporder);

			ଯଦି (svc2)
				ଜାରି;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->ମୂଲ୍ୟ);
		ଯଦି (pid > 0) {
			add_pid(pid);
			ଯଦି (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	ଯଦି (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

ସ୍ଥିର ଶୂନ୍ୟ
do_start_services(ଅଚଳ RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	ଯଦି (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	ଯଦି (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->ମୂଲ୍ୟ);
		ଯଦି (state & RC_SERVICE_FAILED)
			ଜାରି;
		ଯଦି (!(state & RC_SERVICE_STOPPED)) {
			ଯଦି (crashed &&
			    rc_service_daemons_crashed(service->ମୂଲ୍ୟ))
				rc_service_mark(service->ମୂଲ୍ୟ,
				    RC_SERVICE_STOPPED);
			ଅନ୍ୟଥା
			    ଜାରି;
		}
		ଯଦି (!interactive)
			interactive = want_interactive();

		ଯଦି (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->ମୂଲ୍ୟ);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			ବାଛ (read_ଚାବି(true)) {
			ସ୍ଥିତି '1': ଭାଙ୍ଗ;
			ସ୍ଥିତି '2': ଜାରି;
			ସ୍ଥିତି '3': interactive = false; ଭାଙ୍ଗ;
			ସ୍ଥିତି '4': open_shell(); ଯାଅ interactive_retry;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଯାଅ interactive_option;
			}
		}

		pid = service_start(service->ମୂଲ୍ୟ);
		ଯଦି (pid == -1)
			ଭାଙ୍ଗ;
		/* Remember the pid if we're running in parallel */
		ଯଦି (pid > 0) {
			add_pid(pid);
			ଯଦି (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	ଯଦି (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	ଅନ୍ୟଥା
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN ସ୍ଥିର ଶୂନ୍ୟ
handle_bad_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	ଅକ୍ଷର pid[10];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	pid_t crashed_pid = getpid();

	ବାଛ (fork()) {
	ସ୍ଥିତି -1:
		_exit(sig);
		/* NOTREACHED */
	ସ୍ଥିତି 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", ଖାଲି));
		/* NOTREACHED */
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଅଚଳ ଅକ୍ଷର *bootlevel = ଖାଲି;
	ଅକ୍ଷର *newlevel = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *systype = ଖାଲି;
	RC_STRINGLIST *deporder = ଖାଲି;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	ଅକ୍ଷର *krunlevel = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *workingdir = "/";
	ଅକ୍ଷର *pidstr = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	bool parallel;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା regen = 0;
	bool nostop = false;
#ifdef __linux__
	ଅକ୍ଷର *proc;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	ଯଦି (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	ଯଦି (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
		ସ୍ଥିତି 'n':
			nostop = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'o':
			ଯଦି (*optarg == '\0')
				optarg = ଖାଲି;
			ଯଦି (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			ଯଦି (rc_is_user())
				eerrorx("user services do not currently support --override");
			ଯଦି (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		ସ୍ଥିତି 's':
			newlevel = rc_service_resolve(optarg);
			ଯଦି (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		ସ୍ଥିତି 'S':
			systype = rc_sys();
			ଯଦି (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	ଯଦି (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	ଯଦି (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	ଯଦି (newlevel) {
		ଯଦି (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		ଯଦି (strcmp(newlevel, "reboot") == 0) {
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
	ମୁକ୍ତ(pidstr);

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
	ଯଦି (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		ମୁକ୍ତ(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	ଯଦି (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		ଯଦି (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} ଅନ୍ୟଥା {
		/* We should not use krunlevel in sysinit or boot runlevels */
		ଯଦି (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			ଯଦି (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(ଖାଲି);
			}
		}

		ଯଦି (newlevel) {
			ଯଦି (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			ଯଦି (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				ଯଦି (p == ଖାଲି)
					p = rc_proc_getent("softlevel");
				ଯଦି (p != ଖାଲି) {
					set_krunlevel(p);
					ମୁକ୍ତ(p);
				}
			}
#endif
		}
	}

	ଯଦି (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} ଅନ୍ୟଥା {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	ଯଦି (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		ଯଦି (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	ଯଦି ((main_deptree = _rc_deptree_load(0, &regen)) == ଖାଲି) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	ଯଦି (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	ଯଦି (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	ମୁକ୍ତ(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	ମୁକ୍ତ(tmplist);
	ଯଦି (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	ଯଦି (main_stop_services) {
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
	ଯଦି (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		ମୁକ୍ତ(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		ଯଦି (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			ଯଦି (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				ମୁକ୍ତ(tmplist);
			}
			ଯଦି (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->ମୂଲ୍ୟ);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	ଯଦି (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	ଯଦି (newlevel) {
		rc_runlevel_set(newlevel);
		ମୁକ୍ତ(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	ଯଦି (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	ଯଦି (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->ମୂଲ୍ୟ, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	ଯଦି (proc) {
		ଯେତେବେଳେ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		ମୁକ୍ତ(proc);
	}
#endif

	/* If we have a list of services to start then... */
	ଯଦି (main_start_services) {
		/* Get a list of the chained runlevels which compose the target runlevel */
		RC_STRINGLIST *runlevel_chain = rc_runlevel_stacks(runlevel);

		/* Loop through them in reverse order. */
		RC_STRING *rlevel;
		TAILQ_FOREACH_REVERSE(rlevel, runlevel_chain, rc_stringlist, entries)
		{
			/* Get a list of all the services in that runlevel */
			RC_STRINGLIST *run_services = rc_services_in_runlevel(rlevel->ମୂଲ୍ୟ);

			/* Start those services. */
			rc_stringlist_sort(&run_services);
			deporder = rc_deptree_depends(main_deptree, main_types_nwua, run_services, rlevel->ମୂଲ୍ୟ, depoptions | RC_DEP_START);
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
	ଯଦି (proc) {
		ଯେତେବେଳେ ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		ମୁକ୍ତ(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	ଯଦି (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
