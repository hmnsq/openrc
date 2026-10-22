/*
  start-stop-daemon
 * Starts, stops, tests and signals daemons
 *
 * This is essentially a ground up re-write of Debian's
 * start-stop-daemon for cleaner code and to integrate into our RC
 * system so we can monitor daemons a little.
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

#define ONE_MS           1000000

#නම් defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <inttypes.h>
#include <grp.h>
#include <pthread.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <inttypes.h>
#include <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
static sa_nracanaa pam_conv conv = { NULL, NULL};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "rc_exec.h"
#include "schedules.h"
#include "_usage.h"
#include "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ga_nanaa {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_BASE = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SCHEDULER,
  LONGOPT_SCHEDULER_PRIO,
  LONGOPT_SECBITS,
  LONGOPT_NOTIFY,
};

const var_na *applet = NULL;
const var_na *extraopts = NULL;
const var_na getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "capabilities", 1, NULL, LONGOPT_CAPABILITIES},
	{ "secbits",      1, NULL, LONGOPT_SECBITS},
	{ "no-new-privs", 0, NULL, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, NULL, 'I'},
	{ "stop",         0, NULL, 'K'},
	{ "nicelevel",    1, NULL, 'N'},
	{ "oom-score-adj",1, NULL, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, NULL, 'R'},
	{ "start",        0, NULL, 'S'},
	{ "startas",      1, NULL, 'a'},
	{ "background",   0, NULL, 'b'},
	{ "chuid",        1, NULL, 'c'},
	{ "chdir",        1, NULL, 'd'},
	{ "env",          1, NULL, 'e'},
	{ "umask",        1, NULL, 'k'},
	{ "group",        1, NULL, 'g'},
	{ "stop-group",   0, NULL, 'G'},
	{ "interpreted",  0, NULL, 'i'},
	{ "make-pidfile", 0, NULL, 'm'},
	{ "name",         1, NULL, 'n'},
	{ "oknodo",       0, NULL, 'o'},
	{ "pidfile",      1, NULL, 'p'},
	{ "signal",       1, NULL, 's'},
	{ "test",         0, NULL, 't'},
	{ "user",         1, NULL, 'u'},
	{ "chroot",       1, NULL, 'r'},
	{ "wait",         1, NULL, 'w'},
	{ "exec",         1, NULL, 'x'},
	{ "stdin",        1, NULL, '0'},
	{ "stdout",       1, NULL, '1'},
	{ "stderr",       1, NULL, '2'},
	{ "stdout-logger",1, NULL, '3'},
	{ "stderr-logger",1, NULL, '4'},
	{ "progress",     0, NULL, 'P'},
	{ "scheduler",    1, NULL, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, NULL, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, NULL, LONGOPT_NOTIFY},
	longopts_COMMON
};
const var_na * const longopts_help[] = {
	"Set the inheritable, ambient and bounding capabilities",
	"Set the security-bits for the program",
	"Set the No New Privs flag for the program",
	"Set an ionice class:data when starting",
	"Stop daemon",
	"Set a nicelevel when starting",
	"Set OOM score adjustment when starting",
	"Retry schedule to use when stopping",
	"Start daemon",
	"deprecated, use --exec or --name",
	"Force daemon to background",
	"deprecated, use --user",
	"Change the PWD",
	"Set an environment string",
	"Set the umask for the daemon",
	"Change the process group",
	"Stop the whole process group",
	"Match process name by interpreter",
	"Create a pidfile",
	"Match process name",
	"deprecated",
	"Match pid found in this file",
	"Send a different signal",
	"Test actions, don't do them",
	"Change the process user",
	"Chroot to this directory",
	"Milliseconds to wait for daemon start",
	"Binary to start/stop",
	"Redirect stdin to file",
	"Redirect stdout to file",
	"Redirect stderr to file",
	"Redirect stdout to process",
	"Redirect stderr to process",
	"Print dots each second while waiting",
	"Set process scheduler",
	"Set process scheduler priority",
	"Configures experimental notification behaviour",
	longopts_help_COMMON
};
const var_na *usagestring = NULL;

static var_na **nav;

static var_na *changeuser, *ch_root, *ch_dir;

extern var_na **environ;

#නම් !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#නම් !defined(__DragonFly__)
static anta_hstha පූර්ණ_සංඛ්‍යා ioprio_set(පූර්ණ_සංඛ්‍යා which RC_UNUSED,
			     පූර්ණ_සංඛ්‍යා who RC_UNUSED,
			     පූර්ණ_සංඛ්‍යා ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ආපසු syscall(SYS_ioprio_set, which, who, ioprio);
#නැතහොත්
	ආපසු 0;
#endif
}
#endif

static void
cleanup(void)
{
	free(changeuser);
	free(nav);
	free_schedulelist();
}

static void
handle_signal(පූර්ණ_සංඛ්‍යා sig)
{
	පූර්ණ_සංඛ්‍යා status;
	පූර්ණ_සංඛ්‍යා serrno = errno;
	const var_na *signame = NULL;

	switch (sig) {
	pak_sa SIGINT:
		නම් (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	pak_sa SIGTERM:
		නම් (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	pak_sa SIGQUIT:
		නම් (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	pak_sa SIGCHLD:
		සඳහා (;;) {
			නම් (waitpid(-1, &status, WNOHANG) < 0) {
				නම් (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				bha_nga;
			}
		}
		bha_nga;

	default:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

static var_na *
expand_home(const var_na *home, const var_na *path)
{
	var_na *opath, *ppath, *p, *nh;
	sa_nracanaa passwd *pw;

	නම් (!path || *path != '~')
		ආපසු xstrdup(path);

	opath = ppath = xstrdup(path);
	නම් (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		නම් (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		නම් (pw) {
			home = pw->pw_dir;
			ppath = p;
			නම් (ppath)
				*ppath = '/';
		} නැතහොත්
			home = NULL;
	} නැතහොත්
		ppath++;

	නම් (!home) {
	free(opath);
		ආපසු xstrdup(path);
	}
	නම් (!ppath) {
		free(opath);
		ආපසු xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	ආපසු nh;
}

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na **argv)
{
	පූර්ණ_සංඛ්‍යා devnull_fd = -1;
#ifdef TIOCNOTTY
	පූර්ණ_සංඛ්‍යා tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	පූර්ණ_සංඛ්‍යා pamr;
	const var_na *const *pamenv = NULL;
#endif

	පූර්ණ_සංඛ්‍යා opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	var_na *exec = NULL;
	var_na *startas = NULL;
	var_na *name = NULL;
	var_na *pidfile = NULL;
	var_na *retry = NULL;
	පූර්ණ_සංඛ්‍යා sig = -1;
	පූර්ණ_සංඛ්‍යා nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	පූර්ණ_සංඛ්‍යා oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	var_na *home = NULL;
	පූර්ණ_සංඛ්‍යා tid = 0;
	var_na *redirect_stdin = NULL;
	var_na *redirect_stderr = NULL;
	var_na *redirect_stdout = NULL;
	var_na *stderr_process = NULL;
	var_na *stdout_process = NULL;
	පූර්ණ_සංඛ්‍යා stdin_fd;
	පූර්ණ_සංඛ්‍යා stdout_fd;
	පූර්ණ_සංඛ්‍යා stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	පූර්ණ_සංඛ්‍යා i;
	var_na *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	var_na *tmp, *newpath, *np;
	var_na *p;
	var_na *token;
	var_na *exec_file = NULL;
	sa_nracanaa passwd *pw;
	sa_nracanaa group *gr;
	bool stopgroup = false;
	var_na *line = NULL;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	var_na **margv;
	unsigned පූර්ණ_සංඛ්‍යා start_wait = 0;
	const var_na *scheduler = NULL;
	පූර්ණ_සංඛ්‍යා sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = NULL;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	පූර්ණ_සංඛ්‍යා pipefd[2];
	var_na readbuf[1];
	ssize_t ss;
	sa_nracanaa notify notify = {0};
	පූර්ණ_සංඛ්‍යා ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	නම් ((tmp = getenv("SSD_NICELEVEL")))
		නම් (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	නම් ((tmp = getenv("SSD_IONICELEVEL"))) {
		පූර්ණ_සංඛ්‍යා n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		නම් (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		නම් (ionicec == 0)
			ioniced = 0;
		නැතහොත් නම් (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	නම් ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		නම් (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	නම් (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		නම් (pw != NULL) {
			නම් (p == NULL)
				setenv("USER", pw->pw_name, 1);
			නම් (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	අතරතුර ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (පූර්ණ_සංඛ්‍යා *) 0)) != -1)
		switch (opt) {
		pak_sa LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			නම් (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#නැතහොත්
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_SECBITS:
#ifdef __linux__
			නම් (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			නම් (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#නැතහොත්
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#නැතහොත්
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			bha_nga;

		pak_sa 'I': /* --ionice */
			නම් (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			නම් (ionicec == 0)
				ioniced = 0;
			නැතහොත් නම් (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			bha_nga;

		pak_sa 'K':  /* --stop */
			stop = true;
			bha_nga;

		pak_sa 'G':  /* --stop-group */
			stopgroup = true;
			bha_nga;

		pak_sa 'N':  /* --nice */
			නම් (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			නම් (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa 'P':  /* --progress */
			progress = true;
			bha_nga;

		pak_sa 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			bha_nga;

		pak_sa 'S':  /* --start */
			start = true;
			bha_nga;

		pak_sa 'b':  /* --background */
			background = true;
			bha_nga;

		pak_sa 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		pak_sa 'u':  /* --user <username>|<uid> */
		{
			var_na dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			නම් (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			නැතහොත්
				pw = getpwuid((uid_t)tid);

			නම් (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			නම් (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			නම් (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			නම් (gid == 0)
				gid = pw->pw_gid;

			නම් (p) {
				tmp = strsep (&p, ":");
				නම් (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				නැතහොත්
					gr = getgrgid((gid_t) tid);

				නම් (gr == NULL)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		bha_nga;

		pak_sa 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			bha_nga;

		pak_sa 'e': /* --env */
			putenv(optarg);
			bha_nga;

		pak_sa 'g':  /* --group <group>|<gid> */
			නම් (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			නැතහොත්
				gr = getgrgid((gid_t)tid);
			නම් (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			bha_nga;

		pak_sa 'i': /* --interpreted */
			interpreted = true;
			bha_nga;

		pak_sa 'k':
			නම් (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa 'm':  /* --make-pidfile */
			makepidfile = true;
			bha_nga;

		pak_sa 'n':  /* --name <process-name> */
			name = optarg;
			bha_nga;

		pak_sa 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			bha_nga;

		pak_sa 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			bha_nga;

		pak_sa 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			bha_nga;

		pak_sa 't':  /* --test */
			test = true;
			bha_nga;

		pak_sa 'r':  /* --chroot /new/root */
			ch_root = optarg;
			bha_nga;

		pak_sa 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			bha_nga;
		pak_sa 'w':
			නම් (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			bha_nga;
		pak_sa 'x':  /* --exec <executable> */
			exec = optarg;
			bha_nga;

		pak_sa '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			bha_nga;

		pak_sa '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			bha_nga;

		pak_sa '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			bha_nga;

		pak_sa '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			bha_nga;

		pak_sa '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			bha_nga;

		pak_sa LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			bha_nga;

		pak_sa LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			bha_nga;

		pak_sa LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			bha_nga;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	නම් (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	නම් (!exec)
		exec = startas;
	නැතහොත් නම් (!name)
		name = startas;

	නම් (!exec) {
		exec = *argv;
		නම් (!exec)
			exec = name;
		නම් (name && start)
			*argv = name;
	} නැතහොත් නම් (name) {
		*--argv = name;
		++argc;
	} නැතහොත් නම් (exec) {
		*--argv = exec;
		++argc;
	};

	නම් (stop || sig != -1) {
		නම් (sig == -1)
			sig = SIGTERM;
		නම් (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		නම් (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		නම් (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		නම් (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		නම් (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		නම් (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} නැතහොත් {
		නම් (!exec)
			eerrorx("%s: nothing to start", applet);
		නම් (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		නම් ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		නම් ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		නම් (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		නම් (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	නම් (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	නම් (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	නම් (exec) {
		නම් (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		නම් (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			නම් (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			නැතහොත්
				xasprintf(&exec_file, "%s", exec);
		} නැතහොත් {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = NULL;
			අතරතුර ((token = strsep(&p, ":"))) {
				නම් (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				නැතහොත්
					xasprintf(&exec_file, "%s/%s", token, exec);
				නම් (exec_file && access(exec_file, F_OK) == 0)
					bha_nga;
				free(exec_file);
				exec_file = NULL;
			}
			free(tmp);
		}
	}
	නම් (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		free(exec_file);
		exit(EXIT_FAILURE);
	}
	නම් (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	නම් (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		නම් (fp) {
			line = NULL;
			නම් (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			නම් (p != NULL && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				අතරතුර (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				free(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				සඳහා (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(parimaa_na(var_na *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				නම් (p)
					nav[len++] = p;
				සඳහා (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = NULL;
			}
		}
	}
	margv = nav ? nav : argv;

	නම් (stop || sig != -1) {
		නම් (sig == -1)
			sig = SIGTERM;
		නම් (!stop)
			oknodo = true;
		නම් (retry)
			parse_schedule(applet, retry, sig);
		නැතහොත් නම් (test || oknodo)
			parse_schedule(applet, "0", sig);
		නැතහොත්
			parse_schedule(applet, NULL, sig);
		නම් (pidfile) {
			pid = get_pid(applet, pidfile);
			නම් (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} නැතහොත් {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (const var_na *const *)margv,
		    pid, uid, stopgroup, test, progress, false);

		නම් (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		නම් (test || oknodo)
			ආපසු i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		නම් (pidfile)
			unlink(pidfile);
		නම් (svcname)
			rc_service_daemon_set(svcname, exec,
			    (const var_na *const *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	නම් (pidfile)
		pid = get_pid(applet, pidfile);
	නැතහොත්
		pid = 0;

	නම් (pid)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	නැතහොත්
		pids = rc_find_pids(exec, (const var_na * const *) argv, uid, 0);
	නම් (pids)
		eerrorx("%s: %s is already running", applet, exec);

	free(pids);
	නම් (test) {
		නම් (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		අතරතුර (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		නම් (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		නම් (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		නම් (ch_root)
			einfo("in root `%s'", ch_root);
		නම් (ch_dir)
			einfo("in dir `%s'", ch_dir);
		නම් (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		නම් (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	නම් (pidfile)
		unlink(pidfile);

	නම් (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	නම් (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	නම් ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	නම් (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		පූර්ණ_සංඛ්‍යා group_count = ARRAY_SIZE(group_buf);
		pid_t mypid = getpid();
		close(pipefd[0]); /* Close the read end of the pipe. */
		umask(numask);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif

		devnull_fd = open("/dev/null", O_RDWR);

		/* Must call setsid() before setting autogroup nicelevel
		 * but after opening tty_fd. */
		setsid();

		නම් (nicelevel != INT_MIN) {
			නම් (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			නම් (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} නැතහොත් නම් (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		නම් (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		නම් (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			නම් (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		නම් (changeuser != NULL) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			නම් (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			නම් (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			නම් (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		නම් (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * parimaa_na(*group_list));
			නම් (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		නම් (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		නම් (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		නම් (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			නම් (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		නම් (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		නම් (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		නම් (group_list != group_buf)
			free(group_list);
#ifdef __linux__
		නම් (uid && cap_setuid(uid))
#නැතහොත්
		නම් (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		නම් (cap_iab != NULL) {
			i = cap_iab_set_proc(cap_iab);

			නම් (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			නම් (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		නම් (secbits != 0) {
			නම් (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		නම් (no_new_privs) {
			නම් (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
				eerrorx("Could not set No New Privs flag: %s", strerror(errno));
		}
#endif


#ifdef TIOCNOTTY
		ioctl(tty_fd, TIOCNOTTY, 0);
		close(tty_fd);
#endif

		/* Clean the environment of any RC_ variables */
		env_list = rc_stringlist_new();
		i = 0;
		අතරතුර (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		නම් (changeuser != NULL) {
			pamenv = (const var_na *const *)pam_getenvlist(pamh);
			නම් (pamenv) {
				අතරතුර (*pamenv) {
					/* Don't add strings unless they set a var */
					නම් (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					නැතහොත්
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			නම් (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		නම් ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			අතරතුර (token && *token) {
				p = strchr(token, ':');
				නම් (p) {
					*p++ = '\0';
					අතරතුර (*p == ':')
						p++;
				}
				නම් (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					නම් (np != newpath)
						*np++ = ':';
					memcpy(np, token, len);
					np += len;
				}
				token = p;
			}
			*np = '\0';
			unsetenv("PATH");
			setenv("PATH", newpath, 1);
		}

		stdin_fd = devnull_fd;
		stdout_fd = devnull_fd;
		stderr_fd = devnull_fd;
		නම් (redirect_stdin) {
			නම් ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		නම් (redirect_stdout) {
			නම් ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}නැතහොත් නම් (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			නම් (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		නම් (redirect_stderr) {
			නම් ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}නැතහොත් නම් (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			නම් (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		නම් (background)
			dup2(stdin_fd, STDIN_FILENO);
		නම් (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		නම් (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		නම් (notify.type == NOTIFY_FD) {
			නම් (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			නම් (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			නම් (notify.pipe[1] == notify.fd) {
				පූර්ණ_සංඛ්‍යා flags = fcntl(notify.fd, F_GETFD, 0);
				නම් (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				නම් (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		නම් (scheduler != NULL) {
			පූර්ණ_සංඛ්‍යා scheduler_index;
			sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			නම් (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			නැතහොත් නම් (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			නැතහොත් නම් (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			නැතහොත් නම් (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			නැතහොත් නම් (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			නැතහොත් නම් (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			නම් (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			නම් (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} නැතහොත් නම් (sched_prio != -1) {
			const sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			නම් (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		නම් (changeuser != NULL && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	අතරතුර ((ss = read(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	නම් (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	නම් (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		do {
			pid = waitpid(spid, &i, 0);
			නම් (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ආපසු -1;
			}
		} අතරතුර (!WIFEXITED(i) && !WIFSIGNALED(i));
		නම් (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	නම් (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		නම් (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	නම් (notify.type != NOTIFY_NONE) {
		නම් (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} නැතහොත් නම් (start_wait > 0) {
		sa_nracanaa timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		නම් (nanosleep(&ts, NULL) == -1) {
			නම් (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ආපසු 0;
			}
		}
		නම් (background) {
			නම් (kill(pid, 0) == 0)
				alive = true;
		} නැතහොත් {
			නම් (pidfile) {
				pid = get_pid(applet, pidfile);
				නම් (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} නැතහොත්
				pid = 0;
			නම් (do_stop(applet, exec, (const var_na *const *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		නම් (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	නම් (svcname)
		rc_service_daemon_set(svcname, exec,
		    (const var_na *const *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
