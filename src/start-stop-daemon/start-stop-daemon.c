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

#གལ་ཏེ defined(__linux__) && !defined(_GNU_SOURCE)
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

#གལ་ཏེ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#གལ་ཏེ !defined(__DragonFly__)
static anta_hstha ཧྲིལ ioprio_set(ཧྲིལ which RC_UNUSED,
			     ཧྲིལ who RC_UNUSED,
			     ཧྲིལ ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ཕྱིར syscall(SYS_ioprio_set, which, who, ioprio);
#དེ་མིན
	ཕྱིར 0;
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
handle_signal(ཧྲིལ sig)
{
	ཧྲིལ status;
	ཧྲིལ serrno = errno;
	const var_na *signame = NULL;

	switch (sig) {
	pak_sa SIGINT:
		གལ་ཏེ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	pak_sa SIGTERM:
		གལ་ཏེ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	pak_sa SIGQUIT:
		གལ་ཏེ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	pak_sa SIGCHLD:
		ཆེད་དུ (;;) {
			གལ་ཏེ (waitpid(-1, &status, WNOHANG) < 0) {
				གལ་ཏེ (errno != ECHILD)
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

	གལ་ཏེ (!path || *path != '~')
		ཕྱིར xstrdup(path);

	opath = ppath = xstrdup(path);
	གལ་ཏེ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		གལ་ཏེ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		གལ་ཏེ (pw) {
			home = pw->pw_dir;
			ppath = p;
			གལ་ཏེ (ppath)
				*ppath = '/';
		} དེ་མིན
			home = NULL;
	} དེ་མིན
		ppath++;

	གལ་ཏེ (!home) {
	free(opath);
		ཕྱིར xstrdup(path);
	}
	གལ་ཏེ (!ppath) {
		free(opath);
		ཕྱིར xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	ཕྱིར nh;
}

ཧྲིལ གཙོ(ཧྲིལ argc, var_na **argv)
{
	ཧྲིལ devnull_fd = -1;
#ifdef TIOCNOTTY
	ཧྲིལ tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	ཧྲིལ pamr;
	const var_na *const *pamenv = NULL;
#endif

	ཧྲིལ opt;
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
	ཧྲིལ sig = -1;
	ཧྲིལ nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ཧྲིལ oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	var_na *home = NULL;
	ཧྲིལ tid = 0;
	var_na *redirect_stdin = NULL;
	var_na *redirect_stderr = NULL;
	var_na *redirect_stdout = NULL;
	var_na *stderr_process = NULL;
	var_na *stdout_process = NULL;
	ཧྲིལ stdin_fd;
	ཧྲིལ stdout_fd;
	ཧྲིལ stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ཧྲིལ i;
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
	unsigned ཧྲིལ start_wait = 0;
	const var_na *scheduler = NULL;
	ཧྲིལ sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = NULL;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	ཧྲིལ pipefd[2];
	var_na readbuf[1];
	ssize_t ss;
	sa_nracanaa notify notify = {0};
	ཧྲིལ ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	གལ་ཏེ ((tmp = getenv("SSD_NICELEVEL")))
		གལ་ཏེ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	གལ་ཏེ ((tmp = getenv("SSD_IONICELEVEL"))) {
		ཧྲིལ n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		གལ་ཏེ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		གལ་ཏེ (ionicec == 0)
			ioniced = 0;
		དེ་མིན གལ་ཏེ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	གལ་ཏེ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		གལ་ཏེ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	གལ་ཏེ (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		གལ་ཏེ (pw != NULL) {
			གལ་ཏེ (p == NULL)
				setenv("USER", pw->pw_name, 1);
			གལ་ཏེ (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	སྐབས ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ཧྲིལ *) 0)) != -1)
		switch (opt) {
		pak_sa LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			གལ་ཏེ (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#དེ་མིན
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_SECBITS:
#ifdef __linux__
			གལ་ཏེ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			གལ་ཏེ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#དེ་མིན
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#དེ་མིན
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			bha_nga;

		pak_sa 'I': /* --ionice */
			གལ་ཏེ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			གལ་ཏེ (ionicec == 0)
				ioniced = 0;
			དེ་མིན གལ་ཏེ (ionicec == 3)
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
			གལ་ཏེ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			གལ་ཏེ (sscanf(optarg, "%d", &oom_score_adj) != 1)
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
			གལ་ཏེ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			དེ་མིན
				pw = getpwuid((uid_t)tid);

			གལ་ཏེ (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			གལ་ཏེ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			གལ་ཏེ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			གལ་ཏེ (gid == 0)
				gid = pw->pw_gid;

			གལ་ཏེ (p) {
				tmp = strsep (&p, ":");
				གལ་ཏེ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				དེ་མིན
					gr = getgrgid((gid_t) tid);

				གལ་ཏེ (gr == NULL)
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
			གལ་ཏེ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			དེ་མིན
				gr = getgrgid((gid_t)tid);
			གལ་ཏེ (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			bha_nga;

		pak_sa 'i': /* --interpreted */
			interpreted = true;
			bha_nga;

		pak_sa 'k':
			གལ་ཏེ (parse_mode(&numask, optarg))
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
			གལ་ཏེ (sscanf(optarg, "%u", &start_wait) != 1)
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
	གལ་ཏེ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	གལ་ཏེ (!exec)
		exec = startas;
	དེ་མིན གལ་ཏེ (!name)
		name = startas;

	གལ་ཏེ (!exec) {
		exec = *argv;
		གལ་ཏེ (!exec)
			exec = name;
		གལ་ཏེ (name && start)
			*argv = name;
	} དེ་མིན གལ་ཏེ (name) {
		*--argv = name;
		++argc;
	} དེ་མིན གལ་ཏེ (exec) {
		*--argv = exec;
		++argc;
	};

	གལ་ཏེ (stop || sig != -1) {
		གལ་ཏེ (sig == -1)
			sig = SIGTERM;
		གལ་ཏེ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		གལ་ཏེ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		གལ་ཏེ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		གལ་ཏེ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		གལ་ཏེ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		གལ་ཏེ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} དེ་མིན {
		གལ་ཏེ (!exec)
			eerrorx("%s: nothing to start", applet);
		གལ་ཏེ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		གལ་ཏེ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		གལ་ཏེ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		གལ་ཏེ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		གལ་ཏེ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	གལ་ཏེ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	གལ་ཏེ (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	གལ་ཏེ (exec) {
		གལ་ཏེ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		གལ་ཏེ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			གལ་ཏེ (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			དེ་མིན
				xasprintf(&exec_file, "%s", exec);
		} དེ་མིན {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = NULL;
			སྐབས ((token = strsep(&p, ":"))) {
				གལ་ཏེ (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				དེ་མིན
					xasprintf(&exec_file, "%s/%s", token, exec);
				གལ་ཏེ (exec_file && access(exec_file, F_OK) == 0)
					bha_nga;
				free(exec_file);
				exec_file = NULL;
			}
			free(tmp);
		}
	}
	གལ་ཏེ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		free(exec_file);
		exit(EXIT_FAILURE);
	}
	གལ་ཏེ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	གལ་ཏེ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		གལ་ཏེ (fp) {
			line = NULL;
			གལ་ཏེ (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			གལ་ཏེ (p != NULL && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				སྐབས (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				free(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				ཆེད་དུ (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(parimaa_na(var_na *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				གལ་ཏེ (p)
					nav[len++] = p;
				ཆེད་དུ (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = NULL;
			}
		}
	}
	margv = nav ? nav : argv;

	གལ་ཏེ (stop || sig != -1) {
		གལ་ཏེ (sig == -1)
			sig = SIGTERM;
		གལ་ཏེ (!stop)
			oknodo = true;
		གལ་ཏེ (retry)
			parse_schedule(applet, retry, sig);
		དེ་མིན གལ་ཏེ (test || oknodo)
			parse_schedule(applet, "0", sig);
		དེ་མིན
			parse_schedule(applet, NULL, sig);
		གལ་ཏེ (pidfile) {
			pid = get_pid(applet, pidfile);
			གལ་ཏེ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} དེ་མིན {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (const var_na *const *)margv,
		    pid, uid, stopgroup, test, progress, false);

		གལ་ཏེ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		གལ་ཏེ (test || oknodo)
			ཕྱིར i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		གལ་ཏེ (pidfile)
			unlink(pidfile);
		གལ་ཏེ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (const var_na *const *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	གལ་ཏེ (pidfile)
		pid = get_pid(applet, pidfile);
	དེ་མིན
		pid = 0;

	གལ་ཏེ (pid)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	དེ་མིན
		pids = rc_find_pids(exec, (const var_na * const *) argv, uid, 0);
	གལ་ཏེ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	free(pids);
	གལ་ཏེ (test) {
		གལ་ཏེ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		སྐབས (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		གལ་ཏེ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		གལ་ཏེ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		གལ་ཏེ (ch_root)
			einfo("in root `%s'", ch_root);
		གལ་ཏེ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		གལ་ཏེ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		གལ་ཏེ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	གལ་ཏེ (pidfile)
		unlink(pidfile);

	གལ་ཏེ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	གལ་ཏེ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	གལ་ཏེ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	གལ་ཏེ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		ཧྲིལ group_count = ARRAY_SIZE(group_buf);
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

		གལ་ཏེ (nicelevel != INT_MIN) {
			གལ་ཏེ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			གལ་ཏེ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} དེ་མིན གལ་ཏེ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		གལ་ཏེ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		གལ་ཏེ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			གལ་ཏེ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		གལ་ཏེ (changeuser != NULL) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			གལ་ཏེ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			གལ་ཏེ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			གལ་ཏེ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		གལ་ཏེ (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * parimaa_na(*group_list));
			གལ་ཏེ (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		གལ་ཏེ (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		གལ་ཏེ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		གལ་ཏེ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			གལ་ཏེ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		གལ་ཏེ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		གལ་ཏེ (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		གལ་ཏེ (group_list != group_buf)
			free(group_list);
#ifdef __linux__
		གལ་ཏེ (uid && cap_setuid(uid))
#དེ་མིན
		གལ་ཏེ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		གལ་ཏེ (cap_iab != NULL) {
			i = cap_iab_set_proc(cap_iab);

			གལ་ཏེ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			གལ་ཏེ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		གལ་ཏེ (secbits != 0) {
			གལ་ཏེ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		གལ་ཏེ (no_new_privs) {
			གལ་ཏེ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		སྐབས (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		གལ་ཏེ (changeuser != NULL) {
			pamenv = (const var_na *const *)pam_getenvlist(pamh);
			གལ་ཏེ (pamenv) {
				སྐབས (*pamenv) {
					/* Don't add strings unless they set a var */
					གལ་ཏེ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					དེ་མིན
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			གལ་ཏེ (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		གལ་ཏེ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			སྐབས (token && *token) {
				p = strchr(token, ':');
				གལ་ཏེ (p) {
					*p++ = '\0';
					སྐབས (*p == ':')
						p++;
				}
				གལ་ཏེ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					གལ་ཏེ (np != newpath)
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
		གལ་ཏེ (redirect_stdin) {
			གལ་ཏེ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		གལ་ཏེ (redirect_stdout) {
			གལ་ཏེ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}དེ་མིན གལ་ཏེ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			གལ་ཏེ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		གལ་ཏེ (redirect_stderr) {
			གལ་ཏེ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}དེ་མིན གལ་ཏེ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			གལ་ཏེ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		གལ་ཏེ (background)
			dup2(stdin_fd, STDIN_FILENO);
		གལ་ཏེ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		གལ་ཏེ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		གལ་ཏེ (notify.type == NOTIFY_FD) {
			གལ་ཏེ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			གལ་ཏེ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			གལ་ཏེ (notify.pipe[1] == notify.fd) {
				ཧྲིལ flags = fcntl(notify.fd, F_GETFD, 0);
				གལ་ཏེ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				གལ་ཏེ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		གལ་ཏེ (scheduler != NULL) {
			ཧྲིལ scheduler_index;
			sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			གལ་ཏེ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			དེ་མིན གལ་ཏེ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			དེ་མིན གལ་ཏེ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			དེ་མིན གལ་ཏེ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			དེ་མིན གལ་ཏེ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			དེ་མིན གལ་ཏེ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			གལ་ཏེ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			གལ་ཏེ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} དེ་མིན གལ་ཏེ (sched_prio != -1) {
			const sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			གལ་ཏེ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		གལ་ཏེ (changeuser != NULL && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	སྐབས ((ss = read(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	གལ་ཏེ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	གལ་ཏེ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		do {
			pid = waitpid(spid, &i, 0);
			གལ་ཏེ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ཕྱིར -1;
			}
		} སྐབས (!WIFEXITED(i) && !WIFSIGNALED(i));
		གལ་ཏེ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	གལ་ཏེ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		གལ་ཏེ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	གལ་ཏེ (notify.type != NOTIFY_NONE) {
		གལ་ཏེ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} དེ་མིན གལ་ཏེ (start_wait > 0) {
		sa_nracanaa timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		གལ་ཏེ (nanosleep(&ts, NULL) == -1) {
			གལ་ཏེ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ཕྱིར 0;
			}
		}
		གལ་ཏེ (background) {
			གལ་ཏེ (kill(pid, 0) == 0)
				alive = true;
		} དེ་མིན {
			གལ་ཏེ (pidfile) {
				pid = get_pid(applet, pidfile);
				གལ་ཏེ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} དེ་མིན
				pid = 0;
			གལ་ཏེ (do_stop(applet, exec, (const var_na *const *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		གལ་ཏེ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	གལ་ཏེ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (const var_na *const *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
