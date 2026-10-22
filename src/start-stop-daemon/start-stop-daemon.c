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

#ಯದಿ defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#ಅನ್ತರ್ಭಾವ <termios.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ಸ್ಥಿರ ಸಂರಚನಾ pam_conv conv = { ರಿಕ್ತ, ರಿಕ್ತ};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "schedules.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ಗಣನಾ {
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

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "capabilities", 1, ರಿಕ್ತ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ರಿಕ್ತ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ರಿಕ್ತ, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ರಿಕ್ತ, 'I'},
	{ "stop",         0, ರಿಕ್ತ, 'K'},
	{ "nicelevel",    1, ರಿಕ್ತ, 'N'},
	{ "oom-score-adj",1, ರಿಕ್ತ, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ರಿಕ್ತ, 'R'},
	{ "start",        0, ರಿಕ್ತ, 'S'},
	{ "startas",      1, ರಿಕ್ತ, 'a'},
	{ "background",   0, ರಿಕ್ತ, 'b'},
	{ "chuid",        1, ರಿಕ್ತ, 'c'},
	{ "chdir",        1, ರಿಕ್ತ, 'd'},
	{ "env",          1, ರಿಕ್ತ, 'e'},
	{ "umask",        1, ರಿಕ್ತ, 'k'},
	{ "group",        1, ರಿಕ್ತ, 'g'},
	{ "stop-group",   0, ರಿಕ್ತ, 'G'},
	{ "interpreted",  0, ರಿಕ್ತ, 'i'},
	{ "make-pidfile", 0, ರಿಕ್ತ, 'm'},
	{ "name",         1, ರಿಕ್ತ, 'n'},
	{ "oknodo",       0, ರಿಕ್ತ, 'o'},
	{ "pidfile",      1, ರಿಕ್ತ, 'p'},
	{ "signal",       1, ರಿಕ್ತ, 's'},
	{ "test",         0, ರಿಕ್ತ, 't'},
	{ "user",         1, ರಿಕ್ತ, 'u'},
	{ "chroot",       1, ರಿಕ್ತ, 'r'},
	{ "wait",         1, ರಿಕ್ತ, 'w'},
	{ "exec",         1, ರಿಕ್ತ, 'x'},
	{ "stdin",        1, ರಿಕ್ತ, '0'},
	{ "stdout",       1, ರಿಕ್ತ, '1'},
	{ "stderr",       1, ರಿಕ್ತ, '2'},
	{ "stdout-logger",1, ರಿಕ್ತ, '3'},
	{ "stderr-logger",1, ರಿಕ್ತ, '4'},
	{ "progress",     0, ರಿಕ್ತ, 'P'},
	{ "scheduler",    1, ರಿಕ್ತ, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ರಿಕ್ತ, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ರಿಕ್ತ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಸ್ಥಿರ ವರ್ಣ **nav;

ಸ್ಥಿರ ವರ್ಣ *changeuser, *ch_ಬೇರು, *ch_dir;

ಬಾಹ್ಯ ವರ್ಣ **environ;

#ಯದಿ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ಯದಿ !defined(__DragonFly__)
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ ioprio_set(ಪೂರ್ಣಾಙ್ಕ which RC_UNUSED,
			     ಪೂರ್ಣಾಙ್ಕ who RC_UNUSED,
			     ಪೂರ್ಣಾಙ್ಕ ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ಪ್ರತ್ಯಾಗಮ syscall(SYS_ioprio_set, which, who, ioprio);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ
cleanup(ಶೂನ್ಯ)
{
	ಮುಕ್ತ(changeuser);
	ಮುಕ್ತ(nav);
	free_schedulelist();
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_signal(ಪೂರ್ಣಾಙ್ಕ sig)
{
	ಪೂರ್ಣಾಙ್ಕ status;
	ಪೂರ್ಣಾಙ್ಕ serrno = errno;
	ಧ್ರುವ ವರ್ಣ *signame = ರಿಕ್ತ;

	ವಿಕಲ್ಪ (sig) {
	ಪಕ್ಷ SIGINT:
		ಯದಿ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ಪಕ್ಷ SIGTERM:
		ಯದಿ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ಪಕ್ಷ SIGQUIT:
		ಯದಿ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	ಪಕ್ಷ SIGCHLD:
		ಪ್ರತಿ (;;) {
			ಯದಿ (waitpid(-1, &status, WNOHANG) < 0) {
				ಯದಿ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				ಭಙ್ಗ;
			}
		}
		ಭಙ್ಗ;

	ಸ್ವಭಾವ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ವರ್ಣ *
expand_home(ಧ್ರುವ ವರ್ಣ *home, ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ *opath, *ppath, *p, *nh;
	ಸಂರಚನಾ passwd *pw;

	ಯದಿ (!path || *path != '~')
		ಪ್ರತ್ಯಾಗಮ xstrdup(path);

	opath = ppath = xstrdup(path);
	ಯದಿ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ಯದಿ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ಯದಿ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ಯದಿ (ppath)
				*ppath = '/';
		} ಅನ್ಯಥಾ
			home = ರಿಕ್ತ;
	} ಅನ್ಯಥಾ
		ppath++;

	ಯದಿ (!home) {
	ಮುಕ್ತ(opath);
		ಪ್ರತ್ಯಾಗಮ xstrdup(path);
	}
	ಯದಿ (!ppath) {
		ಮುಕ್ತ(opath);
		ಪ್ರತ್ಯಾಗಮ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ಮುಕ್ತ(opath);
	ಪ್ರತ್ಯಾಗಮ nh;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ devnull_fd = -1;
#ifdef TIOCNOTTY
	ಪೂರ್ಣಾಙ್ಕ tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ pamr;
	ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *pamenv = ರಿಕ್ತ;
#endif

	ಪೂರ್ಣಾಙ್ಕ opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	ವರ್ಣ *exec = ರಿಕ್ತ;
	ವರ್ಣ *startas = ರಿಕ್ತ;
	ವರ್ಣ *name = ರಿಕ್ತ;
	ವರ್ಣ *pidfile = ರಿಕ್ತ;
	ವರ್ಣ *retry = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ sig = -1;
	ಪೂರ್ಣಾಙ್ಕ nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ಪೂರ್ಣಾಙ್ಕ oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	ವರ್ಣ *home = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ tid = 0;
	ವರ್ಣ *redirect_stdin = ರಿಕ್ತ;
	ವರ್ಣ *redirect_stderr = ರಿಕ್ತ;
	ವರ್ಣ *redirect_stdout = ರಿಕ್ತ;
	ವರ್ಣ *stderr_process = ರಿಕ್ತ;
	ವರ್ಣ *stdout_process = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ stdin_fd;
	ಪೂರ್ಣಾಙ್ಕ stdout_fd;
	ಪೂರ್ಣಾಙ್ಕ stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ಪೂರ್ಣಾಙ್ಕ i;
	ವರ್ಣ *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ವರ್ಣ *tmp, *newpath, *np;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
	ವರ್ಣ *exec_file = ರಿಕ್ತ;
	ಸಂರಚನಾ passwd *pw;
	ಸಂರಚನಾ group *gr;
	bool stopgroup = false;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	ವರ್ಣ **margv;
	unsigned ಪೂರ್ಣಾಙ್ಕ start_wait = 0;
	ಧ್ರುವ ವರ್ಣ *scheduler = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = ರಿಕ್ತ;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	ಪೂರ್ಣಾಙ್ಕ pipefd[2];
	ವರ್ಣ readbuf[1];
	ssize_t ss;
	ಸಂರಚನಾ notify notify = {0};
	ಪೂರ್ಣಾಙ್ಕ ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ಯದಿ ((tmp = getenv("SSD_NICELEVEL")))
		ಯದಿ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ಯದಿ ((tmp = getenv("SSD_IONICELEVEL"))) {
		ಪೂರ್ಣಾಙ್ಕ n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ಯದಿ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ಯದಿ (ionicec == 0)
			ioniced = 0;
		ಅನ್ಯಥಾ ಯದಿ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ಯದಿ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ಯದಿ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ಯದಿ (home == ರಿಕ್ತ || p == ರಿಕ್ತ) {
		pw = getpwuid(getuid());
		ಯದಿ (pw != ರಿಕ್ತ) {
			ಯದಿ (p == ರಿಕ್ತ)
				setenv("USER", pw->pw_name, 1);
			ಯದಿ (home == ರಿಕ್ತ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ಯದಿ (cap_iab == ರಿಕ್ತ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ಅನ್ಯಥಾ
			eerrorx("Capabilities support not enabled");
#endif
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_SECBITS:
#ifdef __linux__
			ಯದಿ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ರಿಕ್ತ;
			secbits = strtoul(optarg, &tmp, 0);
			ಯದಿ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ಅನ್ಯಥಾ
			eerrorx("Capabilities support not enabled");
#endif
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ಅನ್ಯಥಾ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			ಭಙ್ಗ;

		ಪಕ್ಷ 'I': /* --ionice */
			ಯದಿ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ಯದಿ (ionicec == 0)
				ioniced = 0;
			ಅನ್ಯಥಾ ಯದಿ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ಭಙ್ಗ;

		ಪಕ್ಷ 'K':  /* --stop */
			stop = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'G':  /* --stop-group */
			stopgroup = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'N':  /* --nice */
			ಯದಿ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ಯದಿ (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'P':  /* --progress */
			progress = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'S':  /* --start */
			start = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'b':  /* --background */
			background = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		ಪಕ್ಷ 'u':  /* --user <username>|<uid> */
		{
			ವರ್ಣ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ಯದಿ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ಅನ್ಯಥಾ
				pw = getpwuid((uid_t)tid);

			ಯದಿ (pw == ರಿಕ್ತ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ಯದಿ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ಯದಿ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ಯದಿ (gid == 0)
				gid = pw->pw_gid;

			ಯದಿ (p) {
				tmp = strsep (&p, ":");
				ಯದಿ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ಅನ್ಯಥಾ
					gr = getgrgid((gid_t) tid);

				ಯದಿ (gr == ರಿಕ್ತ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ಭಙ್ಗ;

		ಪಕ್ಷ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'e': /* --env */
			putenv(optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'g':  /* --group <group>|<gid> */
			ಯದಿ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ಅನ್ಯಥಾ
				gr = getgrgid((gid_t)tid);
			ಯದಿ (gr == ರಿಕ್ತ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'i': /* --interpreted */
			interpreted = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'k':
			ಯದಿ (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'm':  /* --make-pidfile */
			makepidfile = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'n':  /* --name <process-name> */
			name = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 't':  /* --test */
			test = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'r':  /* --chroot /new/root */
			ch_ಬೇರು = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'w':
			ಯದಿ (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'x':  /* --exec <executable> */
			exec = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			ಭಙ್ಗ;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	ಯದಿ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ಯದಿ (!exec)
		exec = startas;
	ಅನ್ಯಥಾ ಯದಿ (!name)
		name = startas;

	ಯದಿ (!exec) {
		exec = *argv;
		ಯದಿ (!exec)
			exec = name;
		ಯದಿ (name && start)
			*argv = name;
	} ಅನ್ಯಥಾ ಯದಿ (name) {
		*--argv = name;
		++argc;
	} ಅನ್ಯಥಾ ಯದಿ (exec) {
		*--argv = exec;
		++argc;
	};

	ಯದಿ (stop || sig != -1) {
		ಯದಿ (sig == -1)
			sig = SIGTERM;
		ಯದಿ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ಯದಿ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ಯದಿ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ಯದಿ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ಯದಿ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ಯದಿ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ಅನ್ಯಥಾ {
		ಯದಿ (!exec)
			eerrorx("%s: nothing to start", applet);
		ಯದಿ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ಯದಿ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ಯದಿ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ಯದಿ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ಯದಿ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ಯದಿ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ಯದಿ (ch_ಬೇರು && *ch_ಬೇರು == '~')
		ch_ಬೇರು = expand_home(home, ch_ಬೇರು);
	ಯದಿ (exec) {
		ಯದಿ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ಯದಿ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ಯದಿ (ch_ಬೇರು)
				xasprintf(&exec_file, "%s/%s", ch_ಬೇರು, exec);
			ಅನ್ಯಥಾ
				xasprintf(&exec_file, "%s", exec);
		} ಅನ್ಯಥಾ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ರಿಕ್ತ;
			ಯಾವತ್ ((token = strsep(&p, ":"))) {
				ಯದಿ (ch_ಬೇರು)
					xasprintf(&exec_file, "%s/%s/%s", ch_ಬೇರು, token, exec);
				ಅನ್ಯಥಾ
					xasprintf(&exec_file, "%s/%s", token, exec);
				ಯದಿ (exec_file && access(exec_file, F_OK) == 0)
					ಭಙ್ಗ;
				ಮುಕ್ತ(exec_file);
				exec_file = ರಿಕ್ತ;
			}
			ಮುಕ್ತ(tmp);
		}
	}
	ಯದಿ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		ಮುಕ್ತ(exec_file);
		exit(EXIT_FAILURE);
	}
	ಯದಿ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ಯದಿ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ಯದಿ (fp) {
			ಸಾಲು = ರಿಕ್ತ;
			ಯದಿ (xgetline(&ಸಾಲು, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = ಸಾಲು;
			fclose(fp);
			ಯದಿ (p != ರಿಕ್ತ && ಸಾಲು[0] == '#' && ಸಾಲು[1] == '!') {
				p = ಸಾಲು + 2;
				/* Strip leading spaces */
				ಯಾವತ್ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				ಮುಕ್ತ(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				ಪ್ರತಿ (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(ಪರಿಮಾಣ(ವರ್ಣ *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ಯದಿ (p)
					nav[len++] = p;
				ಪ್ರತಿ (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ರಿಕ್ತ;
			}
		}
	}
	margv = nav ? nav : argv;

	ಯದಿ (stop || sig != -1) {
		ಯದಿ (sig == -1)
			sig = SIGTERM;
		ಯದಿ (!stop)
			oknodo = true;
		ಯದಿ (retry)
			parse_schedule(applet, retry, sig);
		ಅನ್ಯಥಾ ಯದಿ (test || oknodo)
			parse_schedule(applet, "0", sig);
		ಅನ್ಯಥಾ
			parse_schedule(applet, ರಿಕ್ತ, sig);
		ಯದಿ (pidfile) {
			pid = get_pid(applet, pidfile);
			ಯದಿ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} ಅನ್ಯಥಾ {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)margv,
		    pid, uid, stopgroup, test, progress, false);

		ಯದಿ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		ಯದಿ (test || oknodo)
			ಪ್ರತ್ಯಾಗಮ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ಯದಿ (pidfile)
			unlink(pidfile);
		ಯದಿ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	ಯದಿ (pidfile)
		pid = get_pid(applet, pidfile);
	ಅನ್ಯಥಾ
		pid = 0;

	ಯದಿ (pid)
		pids = rc_find_pids(ರಿಕ್ತ, ರಿಕ್ತ, 0, pid);
	ಅನ್ಯಥಾ
		pids = rc_find_pids(exec, (ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ *) argv, uid, 0);
	ಯದಿ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	ಮುಕ್ತ(pids);
	ಯದಿ (test) {
		ಯದಿ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		ಯಾವತ್ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		ಯದಿ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ಯದಿ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ಯದಿ (ch_ಬೇರು)
			einfo("in root `%s'", ch_ಬೇರು);
		ಯದಿ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ಯದಿ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ಯದಿ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ಯದಿ (pidfile)
		unlink(pidfile);

	ಯದಿ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ಯದಿ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ಯದಿ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ಯದಿ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		ಪೂರ್ಣಾಙ್ಕ group_ಎಣಿಕೆ = ARRAY_SIZE(group_buf);
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

		ಯದಿ (nicelevel != INT_MIN) {
			ಯದಿ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ಯದಿ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ಅನ್ಯಥಾ ಯದಿ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ಯದಿ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ಯದಿ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ಯದಿ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		ಯದಿ (changeuser != ರಿಕ್ತ) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ಯದಿ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ಯದಿ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ಯದಿ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		ಯದಿ (changeuser && getgrouplist(changeuser, gid, group_list, &group_ಎಣಿಕೆ) < 0) {
			group_list = xmalloc(group_ಎಣಿಕೆ * ಪರಿಮಾಣ(*group_list));
			ಯದಿ (getgrouplist(changeuser, gid, group_list, &group_ಎಣಿಕೆ) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ಯದಿ (ch_ಬೇರು && chroot(ch_ಬೇರು) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_ಬೇರು, strerror(errno));

		ಯದಿ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ಯದಿ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ಯದಿ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ಯದಿ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ಯದಿ (changeuser && setgroups(group_ಎಣಿಕೆ, group_list))
			eerrorx("%s: setgroups() failed", applet);
		ಯದಿ (group_list != group_buf)
			ಮುಕ್ತ(group_list);
#ifdef __linux__
		ಯದಿ (uid && cap_setuid(uid))
#ಅನ್ಯಥಾ
		ಯದಿ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		ಯದಿ (cap_iab != ರಿಕ್ತ) {
			i = cap_iab_set_proc(cap_iab);

			ಯದಿ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ಯದಿ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ಯದಿ (secbits != 0) {
			ಯದಿ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		ಯದಿ (no_new_privs) {
			ಯದಿ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		ಯಾವತ್ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		ಯದಿ (changeuser != ರಿಕ್ತ) {
			pamenv = (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)pam_getenvlist(pamh);
			ಯದಿ (pamenv) {
				ಯಾವತ್ (*pamenv) {
					/* Don't add strings unless they set a var */
					ಯದಿ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ಅನ್ಯಥಾ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ಯದಿ (strncmp(env->ಮೌಲ್ಯ, "RC_", 3) == 0
					|| strncmp(env->ಮೌಲ್ಯ, "SSD_", 4) == 0
					|| strncmp(env->ಮೌಲ್ಯ, "EINFO_", 6) == 0) {
				*strchr(env->ಮೌಲ್ಯ, '=') = '\0';
				unsetenv(env->ಮೌಲ್ಯ);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ಯದಿ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			ಯಾವತ್ (token && *token) {
				p = strchr(token, ':');
				ಯದಿ (p) {
					*p++ = '\0';
					ಯಾವತ್ (*p == ':')
						p++;
				}
				ಯದಿ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					ಯದಿ (np != newpath)
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
		ಯದಿ (redirect_stdin) {
			ಯದಿ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ಯದಿ (redirect_stdout) {
			ಯದಿ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ಅನ್ಯಥಾ ಯದಿ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ಯದಿ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ಯದಿ (redirect_stderr) {
			ಯದಿ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ಅನ್ಯಥಾ ಯದಿ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ಯದಿ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ಯದಿ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ಯದಿ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ಯದಿ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ಯದಿ (notify.type == NOTIFY_FD) {
			ಯದಿ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ಯದಿ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ಯದಿ (notify.pipe[1] == notify.fd) {
				ಪೂರ್ಣಾಙ್ಕ flags = fcntl(notify.fd, F_GETFD, 0);
				ಯದಿ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ಯದಿ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ಯದಿ (scheduler != ರಿಕ್ತ) {
			ಪೂರ್ಣಾಙ್ಕ scheduler_index;
			ಸಂರಚನಾ sched_param sched =  {.sched_priority = sched_prio};
			ಯದಿ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			ಅನ್ಯಥಾ ಯದಿ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			ಅನ್ಯಥಾ ಯದಿ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			ಅನ್ಯಥಾ ಯದಿ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			ಅನ್ಯಥಾ ಯದಿ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			ಅನ್ಯಥಾ ಯದಿ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ಯದಿ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			ಯದಿ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ಅನ್ಯಥಾ ಯದಿ (sched_prio != -1) {
			ಧ್ರುವ ಸಂರಚನಾ sched_param sched =  {.sched_priority = sched_prio};
			ಯದಿ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		ಯದಿ (changeuser != ರಿಕ್ತ && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ಯಾವತ್ ((ss = ಪಠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ಯದಿ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ಯದಿ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		ಕುರು {
			pid = waitpid(spid, &i, 0);
			ಯದಿ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ಪ್ರತ್ಯಾಗಮ -1;
			}
		} ಯಾವತ್ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ಯದಿ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ಯದಿ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_ಮೌಲ್ಯ("rc_start_wait"))))
	{
		ಯದಿ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	ಯದಿ (notify.type != NOTIFY_NONE) {
		ಯದಿ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ಅನ್ಯಥಾ ಯದಿ (start_wait > 0) {
		ಸಂರಚನಾ timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		ಯದಿ (nanosleep(&ts, ರಿಕ್ತ) == -1) {
			ಯದಿ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ಪ್ರತ್ಯಾಗಮ 0;
			}
		}
		ಯದಿ (background) {
			ಯದಿ (kill(pid, 0) == 0)
				alive = true;
		} ಅನ್ಯಥಾ {
			ಯದಿ (pidfile) {
				pid = get_pid(applet, pidfile);
				ಯದಿ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ಅನ್ಯಥಾ
				pid = 0;
			ಯದಿ (do_stop(applet, exec, (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ಯದಿ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ಯದಿ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
