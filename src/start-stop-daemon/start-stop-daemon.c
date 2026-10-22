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

#ຖ້າ defined(__linux__) && !defined(_GNU_SOURCE)
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

#ຖ້າ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ຖ້າ !defined(__DragonFly__)
static anta_hstha ຈຳນວນເຕັມ ioprio_set(ຈຳນວນເຕັມ which RC_UNUSED,
			     ຈຳນວນເຕັມ who RC_UNUSED,
			     ຈຳນວນເຕັມ ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ສົ່ງຄືນ syscall(SYS_ioprio_set, which, who, ioprio);
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ 0;
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
handle_signal(ຈຳນວນເຕັມ sig)
{
	ຈຳນວນເຕັມ status;
	ຈຳນວນເຕັມ serrno = errno;
	const var_na *signame = NULL;

	switch (sig) {
	pak_sa SIGINT:
		ຖ້າ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	pak_sa SIGTERM:
		ຖ້າ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	pak_sa SIGQUIT:
		ຖ້າ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	pak_sa SIGCHLD:
		ສຳລັບ (;;) {
			ຖ້າ (waitpid(-1, &status, WNOHANG) < 0) {
				ຖ້າ (errno != ECHILD)
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

	ຖ້າ (!path || *path != '~')
		ສົ່ງຄືນ xstrdup(path);

	opath = ppath = xstrdup(path);
	ຖ້າ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ຖ້າ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ຖ້າ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ຖ້າ (ppath)
				*ppath = '/';
		} ບໍ່ດັ່ງນັ້ນ
			home = NULL;
	} ບໍ່ດັ່ງນັ້ນ
		ppath++;

	ຖ້າ (!home) {
	free(opath);
		ສົ່ງຄືນ xstrdup(path);
	}
	ຖ້າ (!ppath) {
		free(opath);
		ສົ່ງຄືນ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	ສົ່ງຄືນ nh;
}

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na **argv)
{
	ຈຳນວນເຕັມ devnull_fd = -1;
#ifdef TIOCNOTTY
	ຈຳນວນເຕັມ tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	ຈຳນວນເຕັມ pamr;
	const var_na *const *pamenv = NULL;
#endif

	ຈຳນວນເຕັມ opt;
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
	ຈຳນວນເຕັມ sig = -1;
	ຈຳນວນເຕັມ nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ຈຳນວນເຕັມ oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	var_na *home = NULL;
	ຈຳນວນເຕັມ tid = 0;
	var_na *redirect_stdin = NULL;
	var_na *redirect_stderr = NULL;
	var_na *redirect_stdout = NULL;
	var_na *stderr_process = NULL;
	var_na *stdout_process = NULL;
	ຈຳນວນເຕັມ stdin_fd;
	ຈຳນວນເຕັມ stdout_fd;
	ຈຳນວນເຕັມ stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ຈຳນວນເຕັມ i;
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
	unsigned ຈຳນວນເຕັມ start_wait = 0;
	const var_na *scheduler = NULL;
	ຈຳນວນເຕັມ sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = NULL;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	ຈຳນວນເຕັມ pipefd[2];
	var_na readbuf[1];
	ssize_t ss;
	sa_nracanaa notify notify = {0};
	ຈຳນວນເຕັມ ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ຖ້າ ((tmp = getenv("SSD_NICELEVEL")))
		ຖ້າ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ຖ້າ ((tmp = getenv("SSD_IONICELEVEL"))) {
		ຈຳນວນເຕັມ n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ຖ້າ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ຖ້າ (ionicec == 0)
			ioniced = 0;
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ຖ້າ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ຖ້າ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ຖ້າ (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		ຖ້າ (pw != NULL) {
			ຖ້າ (p == NULL)
				setenv("USER", pw->pw_name, 1);
			ຖ້າ (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ໃນຂະນະທີ່ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ຈຳນວນເຕັມ *) 0)) != -1)
		switch (opt) {
		pak_sa LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ຖ້າ (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ບໍ່ດັ່ງນັ້ນ
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_SECBITS:
#ifdef __linux__
			ຖ້າ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			ຖ້າ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ບໍ່ດັ່ງນັ້ນ
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ບໍ່ດັ່ງນັ້ນ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			bha_nga;

		pak_sa 'I': /* --ionice */
			ຖ້າ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ຖ້າ (ionicec == 0)
				ioniced = 0;
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (ionicec == 3)
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
			ຖ້າ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ຖ້າ (sscanf(optarg, "%d", &oom_score_adj) != 1)
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
			ຖ້າ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ບໍ່ດັ່ງນັ້ນ
				pw = getpwuid((uid_t)tid);

			ຖ້າ (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ຖ້າ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ຖ້າ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ຖ້າ (gid == 0)
				gid = pw->pw_gid;

			ຖ້າ (p) {
				tmp = strsep (&p, ":");
				ຖ້າ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ບໍ່ດັ່ງນັ້ນ
					gr = getgrgid((gid_t) tid);

				ຖ້າ (gr == NULL)
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
			ຖ້າ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ບໍ່ດັ່ງນັ້ນ
				gr = getgrgid((gid_t)tid);
			ຖ້າ (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			bha_nga;

		pak_sa 'i': /* --interpreted */
			interpreted = true;
			bha_nga;

		pak_sa 'k':
			ຖ້າ (parse_mode(&numask, optarg))
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
			ຖ້າ (sscanf(optarg, "%u", &start_wait) != 1)
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
	ຖ້າ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ຖ້າ (!exec)
		exec = startas;
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (!name)
		name = startas;

	ຖ້າ (!exec) {
		exec = *argv;
		ຖ້າ (!exec)
			exec = name;
		ຖ້າ (name && start)
			*argv = name;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (name) {
		*--argv = name;
		++argc;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (exec) {
		*--argv = exec;
		++argc;
	};

	ຖ້າ (stop || sig != -1) {
		ຖ້າ (sig == -1)
			sig = SIGTERM;
		ຖ້າ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ຖ້າ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ຖ້າ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ຖ້າ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ຖ້າ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ຖ້າ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ບໍ່ດັ່ງນັ້ນ {
		ຖ້າ (!exec)
			eerrorx("%s: nothing to start", applet);
		ຖ້າ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ຖ້າ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ຖ້າ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ຖ້າ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ຖ້າ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ຖ້າ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ຖ້າ (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	ຖ້າ (exec) {
		ຖ້າ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ຖ້າ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ຖ້າ (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			ບໍ່ດັ່ງນັ້ນ
				xasprintf(&exec_file, "%s", exec);
		} ບໍ່ດັ່ງນັ້ນ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = NULL;
			ໃນຂະນະທີ່ ((token = strsep(&p, ":"))) {
				ຖ້າ (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				ບໍ່ດັ່ງນັ້ນ
					xasprintf(&exec_file, "%s/%s", token, exec);
				ຖ້າ (exec_file && access(exec_file, F_OK) == 0)
					bha_nga;
				free(exec_file);
				exec_file = NULL;
			}
			free(tmp);
		}
	}
	ຖ້າ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		free(exec_file);
		exit(EXIT_FAILURE);
	}
	ຖ້າ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ຖ້າ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ຖ້າ (fp) {
			line = NULL;
			ຖ້າ (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			ຖ້າ (p != NULL && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				ໃນຂະນະທີ່ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				free(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				ສຳລັບ (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(parimaa_na(var_na *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ຖ້າ (p)
					nav[len++] = p;
				ສຳລັບ (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = NULL;
			}
		}
	}
	margv = nav ? nav : argv;

	ຖ້າ (stop || sig != -1) {
		ຖ້າ (sig == -1)
			sig = SIGTERM;
		ຖ້າ (!stop)
			oknodo = true;
		ຖ້າ (retry)
			parse_schedule(applet, retry, sig);
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (test || oknodo)
			parse_schedule(applet, "0", sig);
		ບໍ່ດັ່ງນັ້ນ
			parse_schedule(applet, NULL, sig);
		ຖ້າ (pidfile) {
			pid = get_pid(applet, pidfile);
			ຖ້າ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} ບໍ່ດັ່ງນັ້ນ {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (const var_na *const *)margv,
		    pid, uid, stopgroup, test, progress, false);

		ຖ້າ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		ຖ້າ (test || oknodo)
			ສົ່ງຄືນ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ຖ້າ (pidfile)
			unlink(pidfile);
		ຖ້າ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (const var_na *const *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	ຖ້າ (pidfile)
		pid = get_pid(applet, pidfile);
	ບໍ່ດັ່ງນັ້ນ
		pid = 0;

	ຖ້າ (pid)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	ບໍ່ດັ່ງນັ້ນ
		pids = rc_find_pids(exec, (const var_na * const *) argv, uid, 0);
	ຖ້າ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	free(pids);
	ຖ້າ (test) {
		ຖ້າ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		ໃນຂະນະທີ່ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		ຖ້າ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ຖ້າ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ຖ້າ (ch_root)
			einfo("in root `%s'", ch_root);
		ຖ້າ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ຖ້າ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ຖ້າ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ຖ້າ (pidfile)
		unlink(pidfile);

	ຖ້າ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ຖ້າ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ຖ້າ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ຖ້າ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		ຈຳນວນເຕັມ group_count = ARRAY_SIZE(group_buf);
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

		ຖ້າ (nicelevel != INT_MIN) {
			ຖ້າ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ຖ້າ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ບໍ່ດັ່ງນັ້ນ ຖ້າ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ຖ້າ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ຖ້າ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ຖ້າ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		ຖ້າ (changeuser != NULL) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ຖ້າ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ຖ້າ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ຖ້າ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		ຖ້າ (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * parimaa_na(*group_list));
			ຖ້າ (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ຖ້າ (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		ຖ້າ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ຖ້າ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ຖ້າ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ຖ້າ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ຖ້າ (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		ຖ້າ (group_list != group_buf)
			free(group_list);
#ifdef __linux__
		ຖ້າ (uid && cap_setuid(uid))
#ບໍ່ດັ່ງນັ້ນ
		ຖ້າ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		ຖ້າ (cap_iab != NULL) {
			i = cap_iab_set_proc(cap_iab);

			ຖ້າ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ຖ້າ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ຖ້າ (secbits != 0) {
			ຖ້າ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		ຖ້າ (no_new_privs) {
			ຖ້າ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		ໃນຂະນະທີ່ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		ຖ້າ (changeuser != NULL) {
			pamenv = (const var_na *const *)pam_getenvlist(pamh);
			ຖ້າ (pamenv) {
				ໃນຂະນະທີ່ (*pamenv) {
					/* Don't add strings unless they set a var */
					ຖ້າ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ບໍ່ດັ່ງນັ້ນ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ຖ້າ (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ຖ້າ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			ໃນຂະນະທີ່ (token && *token) {
				p = strchr(token, ':');
				ຖ້າ (p) {
					*p++ = '\0';
					ໃນຂະນະທີ່ (*p == ':')
						p++;
				}
				ຖ້າ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					ຖ້າ (np != newpath)
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
		ຖ້າ (redirect_stdin) {
			ຖ້າ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ຖ້າ (redirect_stdout) {
			ຖ້າ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ບໍ່ດັ່ງນັ້ນ ຖ້າ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ຖ້າ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ຖ້າ (redirect_stderr) {
			ຖ້າ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ບໍ່ດັ່ງນັ້ນ ຖ້າ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ຖ້າ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ຖ້າ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ຖ້າ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ຖ້າ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ຖ້າ (notify.type == NOTIFY_FD) {
			ຖ້າ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ຖ້າ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ຖ້າ (notify.pipe[1] == notify.fd) {
				ຈຳນວນເຕັມ flags = fcntl(notify.fd, F_GETFD, 0);
				ຖ້າ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ຖ້າ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ຖ້າ (scheduler != NULL) {
			ຈຳນວນເຕັມ scheduler_index;
			sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			ຖ້າ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			ບໍ່ດັ່ງນັ້ນ ຖ້າ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ຖ້າ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			ຖ້າ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (sched_prio != -1) {
			const sa_nracanaa sched_param sched =  {.sched_priority = sched_prio};
			ຖ້າ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		ຖ້າ (changeuser != NULL && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ໃນຂະນະທີ່ ((ss = read(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ຖ້າ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ຖ້າ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		do {
			pid = waitpid(spid, &i, 0);
			ຖ້າ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ສົ່ງຄືນ -1;
			}
		} ໃນຂະນະທີ່ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ຖ້າ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ຖ້າ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		ຖ້າ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	ຖ້າ (notify.type != NOTIFY_NONE) {
		ຖ້າ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (start_wait > 0) {
		sa_nracanaa timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		ຖ້າ (nanosleep(&ts, NULL) == -1) {
			ຖ້າ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ສົ່ງຄືນ 0;
			}
		}
		ຖ້າ (background) {
			ຖ້າ (kill(pid, 0) == 0)
				alive = true;
		} ບໍ່ດັ່ງນັ້ນ {
			ຖ້າ (pidfile) {
				pid = get_pid(applet, pidfile);
				ຖ້າ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ບໍ່ດັ່ງນັ້ນ
				pid = 0;
			ຖ້າ (do_stop(applet, exec, (const var_na *const *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ຖ້າ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ຖ້າ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (const var_na *const *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
