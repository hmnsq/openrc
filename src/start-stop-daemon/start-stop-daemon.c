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

#ᨀᨘ defined(__linux__) && !defined(_GNU_SOURCE)
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

#ᨀᨘ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ᨀᨘ !defined(__DragonFly__)
static anta_hstha puur_naa_nka ioprio_set(puur_naa_nka which RC_UNUSED,
			     puur_naa_nka who RC_UNUSED,
			     puur_naa_nka ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ᨀᨘᨒ syscall(SYS_ioprio_set, which, who, ioprio);
#ᨀᨘᨁ
	ᨀᨘᨒ 0;
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
handle_signal(puur_naa_nka sig)
{
	puur_naa_nka status;
	puur_naa_nka serrno = errno;
	const var_na *signame = NULL;

	switch (sig) {
	pak_sa SIGINT:
		ᨀᨘ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	pak_sa SIGTERM:
		ᨀᨘ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	pak_sa SIGQUIT:
		ᨀᨘ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	pak_sa SIGCHLD:
		for (;;) {
			ᨀᨘ (waitpid(-1, &status, WNOHANG) < 0) {
				ᨀᨘ (errno != ECHILD)
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

	ᨀᨘ (!path || *path != '~')
		ᨀᨘᨒ xstrdup(path);

	opath = ppath = xstrdup(path);
	ᨀᨘ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ᨀᨘ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ᨀᨘ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ᨀᨘ (ppath)
				*ppath = '/';
		} ᨀᨘᨁ
			home = NULL;
	} ᨀᨘᨁ
		ppath++;

	ᨀᨘ (!home) {
	free(opath);
		ᨀᨘᨒ xstrdup(path);
	}
	ᨀᨘ (!ppath) {
		free(opath);
		ᨀᨘᨒ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	ᨀᨘᨒ nh;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	puur_naa_nka devnull_fd = -1;
#ifdef TIOCNOTTY
	puur_naa_nka tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	puur_naa_nka pamr;
	const var_na *const *pamenv = NULL;
#endif

	puur_naa_nka opt;
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
	puur_naa_nka sig = -1;
	puur_naa_nka nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	puur_naa_nka oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	var_na *home = NULL;
	puur_naa_nka tid = 0;
	var_na *redirect_stdin = NULL;
	var_na *redirect_stderr = NULL;
	var_na *redirect_stdout = NULL;
	var_na *stderr_process = NULL;
	var_na *stdout_process = NULL;
	puur_naa_nka stdin_fd;
	puur_naa_nka stdout_fd;
	puur_naa_nka stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	puur_naa_nka i;
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
	unsigned puur_naa_nka start_wait = 0;
	const var_na *scheduler = NULL;
	puur_naa_nka sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = NULL;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	puur_naa_nka pipefd[2];
	var_na readbuf[1];
	ssize_t ss;
	sa_nracanaa notify notify = {0};
	puur_naa_nka ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ᨀᨘ ((tmp = getenv("SSD_NICELEVEL")))
		ᨀᨘ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ᨀᨘ ((tmp = getenv("SSD_IONICELEVEL"))) {
		puur_naa_nka n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ᨀᨘ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ᨀᨘ (ionicec == 0)
			ioniced = 0;
		ᨀᨘᨁ ᨀᨘ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ᨀᨘ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ᨀᨘ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ᨀᨘ (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		ᨀᨘ (pw != NULL) {
			ᨀᨘ (p == NULL)
				setenv("USER", pw->pw_name, 1);
			ᨀᨘ (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ᨀᨛ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (puur_naa_nka *) 0)) != -1)
		switch (opt) {
		pak_sa LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ᨀᨘ (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ᨀᨘᨁ
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_SECBITS:
#ifdef __linux__
			ᨀᨘ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			ᨀᨘ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ᨀᨘᨁ
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ᨀᨘᨁ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			bha_nga;

		pak_sa 'I': /* --ionice */
			ᨀᨘ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ᨀᨘ (ionicec == 0)
				ioniced = 0;
			ᨀᨘᨁ ᨀᨘ (ionicec == 3)
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
			ᨀᨘ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ᨀᨘ (sscanf(optarg, "%d", &oom_score_adj) != 1)
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
			ᨀᨘ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ᨀᨘᨁ
				pw = getpwuid((uid_t)tid);

			ᨀᨘ (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ᨀᨘ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ᨀᨘ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ᨀᨘ (gid == 0)
				gid = pw->pw_gid;

			ᨀᨘ (p) {
				tmp = strsep (&p, ":");
				ᨀᨘ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ᨀᨘᨁ
					gr = getgrgid((gid_t) tid);

				ᨀᨘ (gr == NULL)
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
			ᨀᨘ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ᨀᨘᨁ
				gr = getgrgid((gid_t)tid);
			ᨀᨘ (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			bha_nga;

		pak_sa 'i': /* --interpreted */
			interpreted = true;
			bha_nga;

		pak_sa 'k':
			ᨀᨘ (parse_mode(&numask, optarg))
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
			ᨀᨘ (sscanf(optarg, "%u", &start_wait) != 1)
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
	ᨀᨘ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ᨀᨘ (!exec)
		exec = startas;
	ᨀᨘᨁ ᨀᨘ (!name)
		name = startas;

	ᨀᨘ (!exec) {
		exec = *argv;
		ᨀᨘ (!exec)
			exec = name;
		ᨀᨘ (name && start)
			*argv = name;
	} ᨀᨘᨁ ᨀᨘ (name) {
		*--argv = name;
		++argc;
	} ᨀᨘᨁ ᨀᨘ (exec) {
		*--argv = exec;
		++argc;
	};

	ᨀᨘ (stop || sig != -1) {
		ᨀᨘ (sig == -1)
			sig = SIGTERM;
		ᨀᨘ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ᨀᨘ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ᨀᨘ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ᨀᨘ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ᨀᨘ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ᨀᨘ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ᨀᨘᨁ {
		ᨀᨘ (!exec)
			eerrorx("%s: nothing to start", applet);
		ᨀᨘ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ᨀᨘ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ᨀᨘ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ᨀᨘ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ᨀᨘ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ᨀᨘ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ᨀᨘ (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	ᨀᨘ (exec) {
		ᨀᨘ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ᨀᨘ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ᨀᨘ (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			ᨀᨘᨁ
				xasprintf(&exec_file, "%s", exec);
		} ᨀᨘᨁ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = NULL;
			ᨀᨛ ((token = strsep(&p, ":"))) {
				ᨀᨘ (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				ᨀᨘᨁ
					xasprintf(&exec_file, "%s/%s", token, exec);
				ᨀᨘ (exec_file && access(exec_file, F_OK) == 0)
					bha_nga;
				free(exec_file);
				exec_file = NULL;
			}
			free(tmp);
		}
	}
	ᨀᨘ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		free(exec_file);
		exit(EXIT_FAILURE);
	}
	ᨀᨘ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ᨀᨘ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ᨀᨘ (fp) {
			line = NULL;
			ᨀᨘ (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			ᨀᨘ (p != NULL && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				ᨀᨛ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				free(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				for (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(parimaa_na(var_na *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ᨀᨘ (p)
					nav[len++] = p;
				for (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = NULL;
			}
		}
	}
	margv = nav ? nav : argv;

	ᨀᨘ (stop || sig != -1) {
		ᨀᨘ (sig == -1)
			sig = SIGTERM;
		ᨀᨘ (!stop)
			oknodo = true;
		ᨀᨘ (retry)
			parse_schedule(applet, retry, sig);
		ᨀᨘᨁ ᨀᨘ (test || oknodo)
			parse_schedule(applet, "0", sig);
		ᨀᨘᨁ
			parse_schedule(applet, NULL, sig);
		ᨀᨘ (pidfile) {
			pid = get_pid(applet, pidfile);
			ᨀᨘ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} ᨀᨘᨁ {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (const var_na *const *)margv,
		    pid, uid, stopgroup, test, progress, false);

		ᨀᨘ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		ᨀᨘ (test || oknodo)
			ᨀᨘᨒ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ᨀᨘ (pidfile)
			unlink(pidfile);
		ᨀᨘ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (const var_na *const *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	ᨀᨘ (pidfile)
		pid = get_pid(applet, pidfile);
	ᨀᨘᨁ
		pid = 0;

	ᨀᨘ (pid)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	ᨀᨘᨁ
		pids = rc_find_pids(exec, (const var_na * const *) argv, uid, 0);
	ᨀᨘ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	free(pids);
	ᨀᨘ (test) {
		ᨀᨘ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		ᨀᨛ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		ᨀᨘ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ᨀᨘ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ᨀᨘ (ch_root)
			einfo("in root `%s'", ch_root);
		ᨀᨘ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ᨀᨘ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ᨀᨘ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ᨀᨘ (pidfile)
		unlink(pidfile);

	ᨀᨘ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ᨀᨘ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ᨀᨘ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ᨀᨘ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		puur_naa_nka group_count = ARRAY_SIZE(group_buf);
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

		ᨀᨘ (nicelevel != INT_MIN) {
			ᨀᨘ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ᨀᨘ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ᨀᨘᨁ ᨀᨘ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ᨀᨘ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ᨀᨘ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ᨀᨘ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		ᨀᨘ (changeuser != NULL) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ᨀᨘ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ᨀᨘ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ᨀᨘ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		ᨀᨘ (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * parimaa_na(*group_list));
			ᨀᨘ (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ᨀᨘ (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		ᨀᨘ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ᨀᨘ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ᨀᨘ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ᨀᨘ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ᨀᨘ (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		ᨀᨘ (group_list != group_buf)
			free(group_list);
#ifdef __linux__
		ᨀᨘ (uid && cap_setuid(uid))
#ᨀᨘᨁ
		ᨀᨘ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		ᨀᨘ (cap_iab != NULL) {
			i = cap_iab_set_proc(cap_iab);

			ᨀᨘ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ᨀᨘ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ᨀᨘ (secbits != 0) {
			ᨀᨘ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		ᨀᨘ (no_new_privs) {
			ᨀᨘ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		ᨀᨛ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		ᨀᨘ (changeuser != NULL) {
			pamenv = (const var_na *const *)pam_getenvlist(pamh);
			ᨀᨘ (pamenv) {
				ᨀᨛ (*pamenv) {
					/* Don't add strings unless they set a var */
					ᨀᨘ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ᨀᨘᨁ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ᨀᨘ (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ᨀᨘ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			ᨀᨛ (token && *token) {
				p = strchr(token, ':');
				ᨀᨘ (p) {
					*p++ = '\0';
					ᨀᨛ (*p == ':')
						p++;
				}
				ᨀᨘ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					ᨀᨘ (np != newpath)
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
		ᨀᨘ (redirect_stdin) {
			ᨀᨘ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ᨀᨘ (redirect_stdout) {
			ᨀᨘ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ᨀᨘᨁ ᨀᨘ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ᨀᨘ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ᨀᨘ (redirect_stderr) {
			ᨀᨘ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ᨀᨘᨁ ᨀᨘ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ᨀᨘ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ᨀᨘ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ᨀᨘ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ᨀᨘ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ᨀᨘ (notify.type == NOTIFY_FD) {
			ᨀᨘ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ᨀᨘ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ᨀᨘ (notify.pipe[1] == notify.fd) {
				puur_naa_nka flags = fcntl(notify.fd, F_GETFD, 0);
				ᨀᨘ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ᨀᨘ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ᨀᨘ (scheduler != NULL) {
			puur_naa_nka scheduler_index;
			sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			ᨀᨘ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			ᨀᨘᨁ ᨀᨘ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			ᨀᨘᨁ ᨀᨘ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			ᨀᨘᨁ ᨀᨘ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			ᨀᨘᨁ ᨀᨘ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			ᨀᨘᨁ ᨀᨘ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ᨀᨘ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			ᨀᨘ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ᨀᨘᨁ ᨀᨘ (sched_prio != -1) {
			const sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			ᨀᨘ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		ᨀᨘ (changeuser != NULL && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ᨀᨛ ((ss = read(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ᨀᨘ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ᨀᨘ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		do {
			pid = waitpid(spid, &i, 0);
			ᨀᨘ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ᨀᨘᨒ -1;
			}
		} ᨀᨛ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ᨀᨘ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ᨀᨘ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		ᨀᨘ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	ᨀᨘ (notify.type != NOTIFY_NONE) {
		ᨀᨘ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ᨀᨘᨁ ᨀᨘ (start_wait > 0) {
		sa_nracanaa timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		ᨀᨘ (nanosleep(&ts, NULL) == -1) {
			ᨀᨘ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ᨀᨘᨒ 0;
			}
		}
		ᨀᨘ (background) {
			ᨀᨘ (kill(pid, 0) == 0)
				alive = true;
		} ᨀᨘᨁ {
			ᨀᨘ (pidfile) {
				pid = get_pid(applet, pidfile);
				ᨀᨘ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ᨀᨘᨁ
				pid = 0;
			ᨀᨘ (do_stop(applet, exec, (const var_na *const *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ᨀᨘ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ᨀᨘ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (const var_na *const *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
