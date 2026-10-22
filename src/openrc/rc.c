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

#include <errno.h>
#include <dirent.h>
#include <getopt.h>
#include <libgen.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "rc_exec.h"
#include "rc-logger.h"
#include "misc.h"
#include "plugin.h"
#include "version.h"
#include "_usage.h"
#include "helpers.h"

const char *extraopts = NULL;
const char getoptstring[] = "a:no:s:S" getoptstring_COMMON;
const struct option longopts[] = {
	{ "no-stop", 0, NULL, 'n' },
	{ "override",    1, NULL, 'o' },
	{ "service",     1, NULL, 's' },
	{ "sys",         0, NULL, 'S' },
	longopts_COMMON
};
const char * const longopts_help[] = {
	"do not stop any services",
	"override the next runlevel to change into\nwhen leaving single user or boot runlevels",
	"runs the service specified with the rest\nof the arguments",
	"output the RC system type, if any",
	longopts_help_COMMON
};
const char *usagestring = ""
	"Usage: openrc [options] [<runlevel>]";

#define INITSH                  RC_LIBEXECDIR "/sh/init.sh"
#define INITEARLYSH             RC_LIBEXECDIR "/sh/init-early.sh"

#define DEVBOOT			"/dev/.rcboot"

const char *applet = NULL;
static RC_STRINGLIST *main_hotplugged_services;
static RC_STRINGLIST *main_stop_services;
static RC_STRINGLIST *main_start_services;
static RC_STRINGLIST *main_types_nw;
static RC_STRINGLIST *main_types_nwua;
static RC_DEPTREE *main_deptree;
static char *runlevel;

struct termios *termios_orig = NULL;

RC_PIDLIST service_pids;
RC_PIDLIST free_these_pids;

static void
clean_failed(void)
{
	DIR *dp;
	struct dirent *d;

	/* Clean the failed services state dir now */
	如果 (!(dp = do_opendirat(rc_dirfd(RC_DIR_SVCDIR), "failed")))
		返回;

	当 ((d = readdir(dp))) {
		如果 (strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0)
			continue;

		如果 (unlinkat(dirfd(dp), d->d_name, 0) == -1)
			eerror("%s: unlink '%s/failed/%s': %s", applet, rc_svcdir(), d->d_name, strerror(errno));
	}
	closedir(dp);
}

static void
cleanup(void)
{
	整数 svcdirfd = rc_dirfd(RC_DIR_SVCDIR);
	RC_PID *p, *tmp;

	如果 (!rc_in_logger && !rc_in_plugin &&
	    applet && (strcmp(applet, "rc") == 0 || strcmp(applet, "openrc") == 0))
	{
		rc_plugin_unload();

		如果 (termios_orig) {
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);
			free(termios_orig);
		}

		/* Clean runlevel start, stop markers */
		unlinkat(svcdirfd, "rc.starting", AT_REMOVEDIR);
		unlinkat(svcdirfd, "rc.stopping", AT_REMOVEDIR);

		clean_failed();
		rc_logger_close();
	}

	LIST_FOREACH_SAFE(p, &service_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		free(p);
	}
	LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
		LIST_REMOVE(p, entries);
		free(p);
	}

	rc_stringlist_free(main_hotplugged_services);
	rc_stringlist_free(main_stop_services);
	rc_stringlist_free(main_start_services);
	rc_stringlist_free(main_types_nw);
	rc_stringlist_free(main_types_nwua);
	rc_deptree_free(main_deptree);
	free(runlevel);
}

static char
read_key(bool block)
{
	struct termios termios;
	char c = 0;
	整数 fd = STDIN_FILENO;

	如果 (!isatty(fd))
		返回 false;

	/* Now save our terminal settings. We need to restore them at exit as
	   we will be changing it for non-blocking reads for Interactive */
	如果 (!termios_orig) {
		termios_orig = xmalloc(sizeof(*termios_orig));
		tcgetattr(fd, termios_orig);
	}

	tcgetattr(fd, &termios);
	termios.c_lflag &= ~(ICANON | ECHO);
	如果 (block)
		termios.c_cc[VMIN] = 1;
	否则 {
		termios.c_cc[VMIN] = 0;
		termios.c_cc[VTIME] = 0;
	}
	tcsetattr(fd, TCSANOW, &termios);
	如果 (read(fd, &c, 1) == -1)
		eerror("read: %s", strerror(errno));
	tcsetattr(fd, TCSANOW, termios_orig);
	返回 c;
}

static bool
want_interactive(void)
{
	char c;
	static bool gotinteractive;
	static bool interactive;

	如果 (rc_yesno(getenv("EINFO_QUIET")))
		返回 false;
	如果 (!gotinteractive) {
		gotinteractive = true;
		interactive = rc_conf_yesno("rc_interactive");
	}
	如果 (!interactive)
		返回 false;
	c = read_key(false);
	返回 (c == 'I' || c == 'i') ? true : false;
}

static void
mark_interactive(void)
{
	FILE *fp;

	如果 ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "interactive", O_WRONLY | O_CREAT | O_TRUNC)))
		fclose(fp);
}

static void
run_program(const char *prog)
{
	struct sigaction sa;
	sigset_t full;
	sigset_t old;
	pid_t pid;

	/* We need to block signals until we have forked */
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	pid = fork();

	如果 (pid == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: fork: %s", applet, strerror(errno));
	}
	如果 (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, NULL);
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);
		sigaction(SIGUSR1, &sa, NULL);
		sigaction(SIGWINCH, &sa, NULL);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, NULL);

		如果 (termios_orig)
			tcsetattr(STDIN_FILENO, TCSANOW, termios_orig);

		execl(prog, prog, (char *)NULL);
		eerror("%s: unable to exec `%s': %s", applet, prog,
		    strerror(errno));
		_exit(EXIT_FAILURE);
	}

	/* Unmask signals and wait for child */
	sigprocmask(SIG_SETMASK, &old, NULL);
	如果 (rc_waitpid(pid) == -1) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);
		eerrorx("%s: failed to exec `%s'", applet, prog);
	}
}

static void
open_shell(void)
{
	const char *shell;
	struct passwd *pw;

#ifdef __linux__
	const char *sys = rc_sys();

	/* VSERVER systems cannot really drop to shells */
	如果 (sys && strcmp(sys, RC_SYS_VSERVER) == 0)
	{
		execlp("halt", "halt", "-f", (char *) NULL);
		eerrorx("%s: unable to exec `halt -f': %s",
		    applet, strerror(errno));
	}
#endif

	shell = rc_conf_value("rc_shell");
	/* No shell set, so obey env, then passwd, then default to /bin/sh */
	如果 (shell == NULL) {
		shell = getenv("SHELL");
		如果 (shell == NULL) {
			pw = getpwuid(getuid());
			如果 (pw)
				shell = pw->pw_shell;
			如果 (shell == NULL)
				shell = "/bin/sh";
		}
	}
	run_program(shell);
}

static bool
set_krunlevel(const char *level)
{
	FILE *fp;

	如果 (rc_is_user())
		返回 true;

	如果 (!level ||
	    strcmp(level, getenv ("RC_BOOTLEVEL")) == 0 ||
	    strcmp(level, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(level, RC_LEVEL_SYSINIT) == 0)
	{
		如果 (access(RC_KRUNLEVEL, F_OK) == 0 && unlink(RC_KRUNLEVEL) != 0)
			eerror("unlink `%s': %s", RC_KRUNLEVEL, strerror(errno));
		返回 false;
	}

	如果 (!(fp = fopen(RC_KRUNLEVEL, "w"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		返回 false;
	}

	fprintf(fp, "%s", level);
	fclose(fp);
	返回 true;
}

static char *get_krunlevel(void)
{
	char *buffer = NULL;
	FILE *fp;
	size_t i = 0;

	如果 (rc_is_user())
		返回 NULL;

	如果 (access(RC_KRUNLEVEL, F_OK) != 0)
		返回 NULL;
	如果 (!(fp = fopen(RC_KRUNLEVEL, "r"))) {
		eerror("fopen `%s': %s", RC_KRUNLEVEL, strerror(errno));
		返回 NULL;
	}

	如果 (xgetline(&buffer, &i, fp) == -1) {
		free(buffer);
		buffer = NULL;
	}

	fclose(fp);
	返回 buffer;
}

static void
add_pid(pid_t pid)
{
	sigset_t sset, old;
	RC_PID *p = xmalloc(sizeof(*p));
	p->pid = pid;

	/* this list will be accessed inside the SIGCHLD signal handler.
	 * so we need to ensure that the SIGCHLD handler doesn't get invoked
	 * while the list is at an inconsistent state.
	 */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_INSERT_HEAD(&service_pids, p, entries);
	sigprocmask(SIG_SETMASK, &old, NULL);
}

static void
remove_pid(pid_t pid, bool inside_signal)
{
	sigset_t sset, old;
	RC_PID *p, *tmp;

	/* same rationale for blocking SIGCHLD as add_pid() */
	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigprocmask(SIG_SETMASK, &sset, &old);
	LIST_FOREACH(p, &service_pids, entries) {
		如果 (p->pid == pid) {
			LIST_REMOVE(p, entries);
			LIST_INSERT_HEAD(&free_these_pids, p, entries);
			break;
		}
	}
	/* only call free if we're not inside a signal handler */
	如果 (!inside_signal) {
		LIST_FOREACH_SAFE(p, &free_these_pids, entries, tmp) {
			LIST_REMOVE(p, entries);
			free(p);
		}
	}
	sigprocmask(SIG_SETMASK, &old, NULL);
}

static void
wait_for_services(void)
{
	为 (;;) {
		当 (waitpid(0, 0, 0) != -1)
			;
		如果 (errno != EINTR)
			break;
	}
}

static void
handle_signal(整数 sig)
{
	整数 serrno = errno;
	const char *signame = NULL;
	pid_t pid;
	RC_PID *pi;
	整数 status = 0;
	struct winsize ws;
	sigset_t sset;

	switch (sig) {
	case SIGCHLD:
		do {
			pid = waitpid(-1, &status, WNOHANG);
			如果 (pid < 0) {
				如果 (errno != ECHILD)
					eerror("waitpid: %s", strerror(errno));
				返回;
			}
		} 当 (!WIFEXITED(status) && !WIFSIGNALED(status));

		/* Remove that pid from our list */
		如果 (pid > 0)
			remove_pid(pid, true);
		break;

	case SIGWINCH:
		如果 (rc_logger_tty >= 0) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
			ioctl(rc_logger_tty, TIOCSWINSZ, &ws);
		}
		break;

	case SIGINT:
		如果 (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	case SIGTERM:
		如果 (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	case SIGQUIT:
		如果 (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */
	case SIGUSR1:
		eerror("rc: Aborting!");

		/* Block child signals */
		sigemptyset(&sset);
		sigaddset(&sset, SIGCHLD);
		sigprocmask(SIG_BLOCK, &sset, NULL);

		/* Kill any running services we have started */
		LIST_FOREACH(pi, &service_pids, entries)
		    kill(pi->pid, SIGTERM);

		/* Notify plugins we are aborting */
		rc_plugin_run(RC_HOOK_ABORT, NULL);

		exit(EXIT_FAILURE);
		/* NOTREACHED */

	default:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

static void
do_sysinit(void)
{
	struct utsname uts;
	const char *sys;

	/* exec init-early.sh if it exists
	 * This should just setup the console to use the correct
	 * font. Maybe it should setup the keyboard too? */
	如果 (access(INITEARLYSH, F_OK) == 0)
		run_program(INITEARLYSH);

	uname(&uts);
	如果 (!rc_yesno(getenv("EINFO_QUIET"))) {
		printf("\n   %sOpenRC %s" VERSION "%s is starting up %s",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_HILITE),
		    ecolor(ECOLOR_NORMAL), ecolor(ECOLOR_BRACKET));
#ifdef BRANDING
		printf(BRANDING " (%s)", uts.machine);
#否则
		printf("%s %s (%s)",
		    uts.sysname,
		    uts.release,
		    uts.machine);
#endif
	}

	如果 ((sys = rc_sys()))
		printf(" [%s]", sys);

	printf("%s\n\n", ecolor(ECOLOR_NORMAL));

	如果 (!rc_yesno(getenv ("EINFO_QUIET")) &&
	    rc_conf_yesno("rc_interactive"))
		printf("Press %sI%s to enter interactive boot mode\n\n",
		    ecolor(ECOLOR_GOOD), ecolor(ECOLOR_NORMAL));

	setenv("RC_RUNLEVEL", RC_LEVEL_SYSINIT, 1);
	run_program(INITSH);

	/* init may have mounted /proc so we can now detect or real
	 * sys */
	如果 ((sys = rc_sys()))
		setenv("RC_SYS", sys, 1);
	如果 ((main_deptree = _rc_deptree_load(false, NULL)) == NULL)
		eerrorx("failed to load deptree");
}

static bool
runlevel_config(const char *service, const char *level)
{
	char *init = rc_service_resolve(service);
	char *conf, *dir;
	bool retval;

	dir = dirname(dirname(init));
	xasprintf(&conf, "%s/conf.d/%s.%s", dir, service, level);
	retval = access(conf, F_OK) == 0;
	free(conf);
	free(init);
	返回 retval;
}

static void
do_stop_services(RC_STRINGLIST *types_nw, RC_STRINGLIST *start_services,
				 const RC_STRINGLIST *stop_services, const RC_DEPTREE *deptree,
				 const char *newlevel, bool parallel, bool going_down)
{
	pid_t pid;
	RC_STRING *service, *svc1, *svc2;
	RC_STRINGLIST *deporder, *tmplist, *kwords;
	RC_STRINGLIST *types_nw_save = NULL;
	RC_SERVICE state;
	RC_STRINGLIST *nostop;
	bool crashed, nstop;

	如果 (!types_nw) {
		types_nw = types_nw_save = rc_stringlist_new();
		rc_stringlist_add(types_nw, "needsme");
		rc_stringlist_add(types_nw, "wantsme");
	}

	crashed = rc_conf_yesno("rc_crashed_stop");

	nostop = rc_stringlist_split(rc_conf_value("rc_nostop"), " ");
	TAILQ_FOREACH_REVERSE(service, stop_services, rc_stringlist, entries)
	{
		state = rc_service_state(service->value);
		如果 (state & RC_SERVICE_STOPPED || state & RC_SERVICE_FAILED)
			continue;

		/* Sometimes we don't ever want to stop a service. */
		如果 (rc_stringlist_find(nostop, service->value)) {
			rc_service_mark(service->value, RC_SERVICE_FAILED);
			continue;
		}
		kwords = rc_deptree_depend(deptree, service->value, "keyword");
		如果 (rc_stringlist_find(kwords, "-stop") ||
		    rc_stringlist_find(kwords, "nostop") ||
		    (going_down &&
			(rc_stringlist_find(kwords, "-shutdown") ||
			    rc_stringlist_find(kwords, "noshutdown"))))
			nstop = true;
		否则
			nstop = false;
		rc_stringlist_free(kwords);
		如果 (nstop) {
			rc_service_mark(service->value, RC_SERVICE_FAILED);
			continue;
		}

		/* If the service has crashed, skip further checks and just stop
		   it */
		如果 (crashed &&
		    rc_service_daemons_crashed(service->value))
			goto stop;

		/* If we're in the start list then don't bother stopping us */
		svc1 = rc_stringlist_find(start_services, service->value);
		如果 (svc1) {
			如果 (newlevel && strcmp(runlevel, newlevel) != 0) {
				/* So we're in the start list. But we should
				 * be stopped if we have a runlevel
				 * configuration file for either the current
				 * or next so we use the correct one. */
				如果 (!runlevel_config(service->value,runlevel) &&
				    !runlevel_config(service->value,newlevel))
					continue;
			}
			否则
				continue;
		}

		/* We got this far. Last check is to see if any service
		 * that going to be started depends on us */
		如果 (!svc1) {
			tmplist = rc_stringlist_new();
			rc_stringlist_add(tmplist, service->value);
			deporder = rc_deptree_depends(deptree, types_nw,
			    tmplist, newlevel ? newlevel : runlevel,
			    RC_DEP_STRICT | RC_DEP_TRACE);
			rc_stringlist_free(tmplist);
			svc2 = NULL;
			TAILQ_FOREACH(svc1, deporder, entries) {
				svc2 = rc_stringlist_find(start_services,
				    svc1->value);
				如果 (svc2)
					break;
			}
			rc_stringlist_free(deporder);

			如果 (svc2)
				continue;
		}

stop:
		/* After all that we can finally stop the blighter! */
		pid = service_stop(service->value);
		如果 (pid > 0) {
			add_pid(pid);
			如果 (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	如果 (types_nw_save)
		rc_stringlist_free(types_nw_save);

	rc_stringlist_free(nostop);
}

static void
do_start_services(const RC_STRINGLIST *start_services, bool parallel)
{
	RC_STRING *service;
	pid_t pid;
	bool interactive = false;
	RC_SERVICE state;
	bool crashed = false;

	如果 (!rc_yesno(getenv("EINFO_QUIET")))
		interactive = faccessat(rc_dirfd(RC_DIR_SVCDIR), "interactive", F_OK, 0) == 0;
	errno = 0;
	crashed = rc_conf_yesno("rc_crashed_start");
	如果 (errno == ENOENT)
		crashed = true;

	TAILQ_FOREACH(service, start_services, entries) {
		state = rc_service_state(service->value);
		如果 (state & RC_SERVICE_FAILED)
			continue;
		如果 (!(state & RC_SERVICE_STOPPED)) {
			如果 (crashed &&
			    rc_service_daemons_crashed(service->value))
				rc_service_mark(service->value,
				    RC_SERVICE_STOPPED);
			否则
			    continue;
		}
		如果 (!interactive)
			interactive = want_interactive();

		如果 (interactive) {
	interactive_retry:
			printf("\n");
			einfo("About to start the service %s",
			    service->value);
			eindent();
			einfo("1) Start the service\t\t2) Skip the service");
			einfo("3) Continue boot process\t\t4) Exit to shell");
			eoutdent();
	interactive_option:
			switch (read_key(true)) {
			case '1': break;
			case '2': continue;
			case '3': interactive = false; break;
			case '4': open_shell(); goto interactive_retry;
			default: goto interactive_option;
			}
		}

		pid = service_start(service->value);
		如果 (pid == -1)
			break;
		/* Remember the pid if we're running in parallel */
		如果 (pid > 0) {
			add_pid(pid);
			如果 (!parallel) {
				rc_waitpid(pid);
				remove_pid(pid, false);
			}
		}
	}

	/* Store our interactive status for boot */
	如果 (interactive &&
			(strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
			 strcmp(runlevel, getenv("RC_BOOTLEVEL")) == 0))
		mark_interactive();
	否则
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "interactive", 0);
}

#ifdef RC_DEBUG
RC_NORETURN static void
handle_bad_signal(整数 sig)
{
	char pid[10];
	整数 status;
	pid_t crashed_pid = getpid();

	switch (fork()) {
	case -1:
		_exit(sig);
		/* NOTREACHED */
	case 0:
		sprintf(pid, "%i", crashed_pid);
		printf("\nAuto launching gdb!\n\n");
		_exit(execlp("gdb", "gdb", "--quiet", "--pid", pid,
			"-ex", "bt full", NULL));
		/* NOTREACHED */
	default:
		wait(&status);
	}
	_exit(1);
	/* NOTREACHED */
}
#endif

整数 主要(整数 argc, char **argv)
{
	const char *bootlevel = NULL;
	char *newlevel = NULL;
	const char *systype = NULL;
	RC_STRINGLIST *deporder = NULL;
	RC_STRINGLIST *tmplist;
	RC_STRING *service;
	bool going_down = false;
	整数 depoptions = RC_DEP_STRICT | RC_DEP_TRACE;
	char *krunlevel = NULL;
	const char *workingdir = "/";
	char *pidstr = NULL;
	整数 opt;
	bool parallel;
	整数 regen = 0;
	bool nostop = false;
#ifdef __linux__
	char *proc;
	char *p;
	char *token;
#endif

#ifdef RC_DEBUG
	signal_setup(SIGBUS, handle_bad_signal);
	signal_setup(SIGILL, handle_bad_signal);
	signal_setup(SIGSEGV, handle_bad_signal);
#endif

	applet = basename_c(argv[0]);
	LIST_INIT(&service_pids);
	LIST_INIT(&free_these_pids);
	如果 (!applet)
		eerrorx("arguments required");

	argc--;
	argv++;

	/* complain about old configuration settings if they exist */
	如果 (access(RC_CONF_OLD, F_OK) == 0) {
		ewarn("%s still exists on your system and should be removed.", RC_CONF_OLD);
		ewarn("Please migrate to the appropriate settings in %s", RC_CONF);
	}

	argc++;
	argv--;
	当 ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (整数 *) 0)) != -1)
	{
		switch (opt) {
		case 'n':
			nostop = true;
			break;
		case 'o':
			如果 (*optarg == '\0')
				optarg = NULL;
			如果 (!rc_runlevel_exists(optarg)) {
				eerror("runlevel `%s' does not exist", optarg);
				exit(EXIT_FAILURE);
			}
			如果 (rc_is_user())
				eerrorx("user services do not currently support --override");
			如果 (!set_krunlevel(optarg))
				exit(EXIT_FAILURE);
			einfo("Overriding next runlevel to %s", optarg);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case 's':
			newlevel = rc_service_resolve(optarg);
			如果 (!newlevel)
				eerrorx("%s: service `%s' does not exist",
				    applet, optarg);
			argv += optind - 1;
			*argv = newlevel;
			execv(*argv, argv);
			eerrorx("%s: %s", applet, strerror(errno));
			/* NOTREACHED */
		case 'S':
			systype = rc_sys();
			如果 (systype)
				printf("%s\n", systype);
			exit(EXIT_SUCCESS);
			/* NOTREACHED */
		case_RC_COMMON_GETOPT
		}
	}

	/* Change dir to / to ensure all init scripts don't use stuff in pwd
	 * For user services, change to the user's HOME instead. */
	如果 (rc_is_user() && !(workingdir = getenv("HOME")))
		eerrorx("HOME is unset.");

	如果 (chdir(workingdir) == -1)
		eerror("chdir: %s", strerror(errno));

	/* Ensure our environment is pure
	 * Also, add our configuration to it */
	env_filter();
	env_config();

	newlevel = argv[optind++];
	/* To make life easier, we only have the shutdown runlevel as
	 * nothing really needs to know that we're rebooting.
	 * But for those that do, you can test against RC_REBOOT. */
	如果 (newlevel) {
		如果 (rc_is_user() && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0)
			eerrorx("%s: the sysinit runlevel can't be used for user services.", applet);

		如果 (strcmp(newlevel, "reboot") == 0) {
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
	free(pidstr);

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
	如果 (newlevel && strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
		do_sysinit();
		free(runlevel);
		runlevel = default_runlevel();
	}

	rc_plugin_load();

	/* Now we start handling our children */
	signal_setup(SIGCHLD, handle_signal);

	如果 (newlevel &&
	    (strcmp(newlevel, RC_LEVEL_SHUTDOWN) == 0 ||
		strcmp(newlevel, RC_LEVEL_SINGLE) == 0))
	{
		going_down = true;
		如果 (access(RC_KRUNLEVEL, F_OK) != 0)
			set_krunlevel(runlevel);
		rc_runlevel_set(newlevel);
		setenv("RC_RUNLEVEL", newlevel, 1);
		setenv("RC_GOINGDOWN", "YES", 1);
	} 否则 {
		/* We should not use krunlevel in sysinit or boot runlevels */
		如果 (!newlevel ||
		    (strcmp(newlevel, RC_LEVEL_SYSINIT) != 0 &&
			strcmp(newlevel, getenv("RC_BOOTLEVEL")) != 0))
		{
			krunlevel = get_krunlevel();
			如果 (krunlevel) {
				newlevel = krunlevel;
				set_krunlevel(NULL);
			}
		}

		如果 (newlevel) {
			如果 (strcmp(runlevel, newlevel) != 0 &&
			    !rc_runlevel_exists(newlevel))
				eerrorx("%s: not a valid runlevel", newlevel);

#ifdef __linux__
			如果 (strcmp(newlevel, RC_LEVEL_SYSINIT) == 0) {
				/* If we requested a runlevel, save it now */
				p = rc_proc_getent("rc_runlevel");
				如果 (p == NULL)
					p = rc_proc_getent("softlevel");
				如果 (p != NULL) {
					set_krunlevel(p);
					free(p);
				}
			}
#endif
		}
	}

	如果 (going_down) {
#ifdef __FreeBSD__
		/* FIXME: we shouldn't have todo this */
		/* For some reason, wait_for_services waits for the logger
		 * process to finish as well, but only on FreeBSD.
		 * We cannot allow this so we stop logging now. */
		rc_logger_close();
#endif

		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, newlevel);
	} 否则 {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_IN, runlevel);
	}

	/* Check if runlevel is valid if we're changing */
	如果 (newlevel && strcmp(runlevel, newlevel) != 0 && !going_down) {
		如果 (!rc_runlevel_exists(newlevel)) {
			rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
			eerrorx("%s: is not a valid runlevel", newlevel);
		}
	}

	/* Load our deptree */
	如果 ((main_deptree = _rc_deptree_load(0, &regen)) == NULL) {
		rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT, runlevel);
		eerrorx("failed to load deptree");
	}

	如果 (faccessat(rc_dirfd(RC_DIR_SVCDIR), "clock-skewed", F_OK, 0) == 0)
		ewarn("WARNING: clock skew detected!");

	/* Clean the failed services state dir */
	clean_failed();

	如果 (mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", 0755) != 0) {
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
	free(tmplist);
	tmplist = rc_services_in_state(RC_SERVICE_STARTING);
	TAILQ_CONCAT(main_stop_services, tmplist, entries);
	free(tmplist);
	如果 (main_stop_services)
		rc_stringlist_sort(&main_stop_services);

	main_types_nwua = rc_stringlist_new();
	rc_stringlist_add(main_types_nwua, "ineed");
	rc_stringlist_add(main_types_nwua, "iwant");
	rc_stringlist_add(main_types_nwua, "iuse");
	rc_stringlist_add(main_types_nwua, "iafter");

	如果 (main_stop_services) {
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
	如果 (strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SHUTDOWN) != 0 &&
	    strcmp(newlevel ? newlevel : runlevel, RC_LEVEL_SYSINIT) != 0)
	{
		tmplist = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		TAILQ_CONCAT(main_start_services, tmplist, entries);
		free(tmplist);
		/* If we are NOT headed for the single-user runlevel... */
		如果 (strcmp(newlevel ? newlevel : runlevel,
			RC_LEVEL_SINGLE) != 0)
		{
			/* If we are NOT headed for the boot runlevel... */
			如果 (strcmp(newlevel ? newlevel : runlevel,
				bootlevel) != 0)
			{
				tmplist = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(main_start_services, tmplist, entries);
				free(tmplist);
			}
			如果 (main_hotplugged_services) {
				TAILQ_FOREACH(service, main_hotplugged_services,
				    entries)
				    rc_stringlist_addu(main_start_services,
					service->value);
			}
		}
	}

	parallel = rc_conf_yesno("rc_parallel");

	/* Now stop the services that shouldn't be running */
	如果 (main_stop_services && !nostop)
		do_stop_services(main_types_nw, main_start_services, main_stop_services, main_deptree, newlevel, parallel, going_down);

	/* Wait for our services to finish */
	wait_for_services();

	/* Notify the plugins we have finished */
	rc_plugin_run(RC_HOOK_RUNLEVEL_STOP_OUT,
	    going_down ? newlevel : runlevel);

	unlinkat(rc_dirfd(RC_DIR_SVCDIR), "rc.stopping", AT_REMOVEDIR);

	/* Store the new runlevel */
	如果 (newlevel) {
		rc_runlevel_set(newlevel);
		free(runlevel);
		runlevel = xstrdup(newlevel);
		setenv("RC_RUNLEVEL", runlevel, 1);
	}

#ifdef __linux__
	/* We can't log beyond this point as the shutdown runlevel
	 * will mount / readonly. */
	如果 (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
		rc_logger_close();
#endif

	mkdirat(rc_dirfd(RC_DIR_SVCDIR), "rc.starting", 0755);

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_IN, runlevel);

	/* Re-add our hotplugged services if they stopped */
	如果 (main_hotplugged_services)
		TAILQ_FOREACH(service, main_hotplugged_services, entries)
		    rc_service_mark(service->value, RC_SERVICE_HOTPLUGGED);

#ifdef __linux__
	/* If the "noinit" parameter was passed on the kernel command line then
	 * mark the specified services as started so they will not be started
	 * by us. */
	proc = p = rc_proc_getent("noinit");
	如果 (proc) {
		当 ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STARTED);
		free(proc);
	}
#endif

	/* If we have a list of services to start then... */
	如果 (main_start_services) {
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
	如果 (proc) {
		当 ((token = strsep(&p, ",")))
			rc_service_mark(token, RC_SERVICE_STOPPED);
		free(proc);
	}

#endif

	rc_plugin_run(RC_HOOK_RUNLEVEL_START_OUT, runlevel);

	/* If we're in the boot runlevel and we regenerated our dependencies
	 * we need to delete them so that they are regenerated again in the
	 * default runlevel as they may depend on things that are now
	 * available */
	如果 (regen && strcmp(runlevel, bootlevel) == 0)
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "deptree", 0);

	返回 EXIT_SUCCESS;
}
