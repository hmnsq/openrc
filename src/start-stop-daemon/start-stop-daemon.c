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

#ଯଦି defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <grp.h>
#ଅନ୍ତର୍ଭାଵ <pthread.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <sched.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <sys/resource.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#ଅନ୍ତର୍ଭାଵ <termios.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ସ୍ଥିର ସଂରଚନା pam_conv conv = { ଖାଲି, ଖାଲି};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "schedules.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ଗଣନା {
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

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "capabilities", 1, ଖାଲି, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ଖାଲି, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ଖାଲି, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ଖାଲି, 'I'},
	{ "stop",         0, ଖାଲି, 'K'},
	{ "nicelevel",    1, ଖାଲି, 'N'},
	{ "oom-score-adj",1, ଖାଲି, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ଖାଲି, 'R'},
	{ "start",        0, ଖାଲି, 'S'},
	{ "startas",      1, ଖାଲି, 'a'},
	{ "background",   0, ଖାଲି, 'b'},
	{ "chuid",        1, ଖାଲି, 'c'},
	{ "chdir",        1, ଖାଲି, 'd'},
	{ "env",          1, ଖାଲି, 'e'},
	{ "umask",        1, ଖାଲି, 'k'},
	{ "group",        1, ଖାଲି, 'g'},
	{ "stop-group",   0, ଖାଲି, 'G'},
	{ "interpreted",  0, ଖାଲି, 'i'},
	{ "make-pidfile", 0, ଖାଲି, 'm'},
	{ "name",         1, ଖାଲି, 'n'},
	{ "oknodo",       0, ଖାଲି, 'o'},
	{ "pidfile",      1, ଖାଲି, 'p'},
	{ "signal",       1, ଖାଲି, 's'},
	{ "test",         0, ଖାଲି, 't'},
	{ "user",         1, ଖାଲି, 'u'},
	{ "chroot",       1, ଖାଲି, 'r'},
	{ "wait",         1, ଖାଲି, 'w'},
	{ "exec",         1, ଖାଲି, 'x'},
	{ "stdin",        1, ଖାଲି, '0'},
	{ "stdout",       1, ଖାଲି, '1'},
	{ "stderr",       1, ଖାଲି, '2'},
	{ "stdout-logger",1, ଖାଲି, '3'},
	{ "stderr-logger",1, ଖାଲି, '4'},
	{ "progress",     0, ଖାଲି, 'P'},
	{ "scheduler",    1, ଖାଲି, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ଖାଲି, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ଖାଲି, LONGOPT_NOTIFY},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ସ୍ଥିର ଅକ୍ଷର **nav;

ସ୍ଥିର ଅକ୍ଷର *changeuser, *ch_ମୂଳ, *ch_dir;

ବାହ୍ୟ ଅକ୍ଷର **environ;

#ଯଦି !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ଯଦି !defined(__DragonFly__)
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ioprio_set(ପୂର୍ଣ୍ଣସଂଖ୍ୟା which RC_UNUSED,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା who RC_UNUSED,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ଫେରନ୍ତୁ syscall(SYS_ioprio_set, which, who, ioprio);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ
cleanup(ଶୂନ୍ୟ)
{
	ମୁକ୍ତ(changeuser);
	ମୁକ୍ତ(nav);
	free_schedulelist();
}

ସ୍ଥିର ଶୂନ୍ୟ
handle_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno;
	ଅଚଳ ଅକ୍ଷର *signame = ଖାଲି;

	ବାଛ (sig) {
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

	ସ୍ଥିତି SIGCHLD:
		ପାଇଁ (;;) {
			ଯଦି (waitpid(-1, &status, WNOHANG) < 0) {
				ଯଦି (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				ଭାଙ୍ଗ;
			}
		}
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ସ୍ଥିର ଅକ୍ଷର *
expand_home(ଅଚଳ ଅକ୍ଷର *home, ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *opath, *ppath, *p, *nh;
	ସଂରଚନା passwd *pw;

	ଯଦି (!path || *path != '~')
		ଫେରନ୍ତୁ xstrdup(path);

	opath = ppath = xstrdup(path);
	ଯଦି (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ଯଦି (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ଯଦି (pw) {
			home = pw->pw_dir;
			ppath = p;
			ଯଦି (ppath)
				*ppath = '/';
		} ଅନ୍ୟଥା
			home = ଖାଲି;
	} ଅନ୍ୟଥା
		ppath++;

	ଯଦି (!home) {
	ମୁକ୍ତ(opath);
		ଫେରନ୍ତୁ xstrdup(path);
	}
	ଯଦି (!ppath) {
		ମୁକ୍ତ(opath);
		ଫେରନ୍ତୁ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ମୁକ୍ତ(opath);
	ଫେରନ୍ତୁ nh;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnull_fd = -1;
#ifdef TIOCNOTTY
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pamr;
	ଅଚଳ ଅକ୍ଷର *ଅଚଳ *pamenv = ଖାଲି;
#endif

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	ଅକ୍ଷର *exec = ଖାଲି;
	ଅକ୍ଷର *startas = ଖାଲି;
	ଅକ୍ଷର *name = ଖାଲି;
	ଅକ୍ଷର *pidfile = ଖାଲି;
	ଅକ୍ଷର *retry = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = -1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	ଅକ୍ଷର *home = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା tid = 0;
	ଅକ୍ଷର *redirect_stdin = ଖାଲି;
	ଅକ୍ଷର *redirect_stderr = ଖାଲି;
	ଅକ୍ଷର *redirect_stdout = ଖାଲି;
	ଅକ୍ଷର *stderr_process = ଖାଲି;
	ଅକ୍ଷର *stdout_process = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdin_fd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdout_fd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅକ୍ଷର *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ଅକ୍ଷର *tmp, *newpath, *np;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
	ଅକ୍ଷର *exec_file = ଖାଲି;
	ସଂରଚନା passwd *pw;
	ସଂରଚନା group *gr;
	bool stopgroup = false;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	ଅକ୍ଷର **margv;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା start_wait = 0;
	ଅଚଳ ଅକ୍ଷର *scheduler = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = ଖାଲି;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipefd[2];
	ଅକ୍ଷର readbuf[1];
	ssize_t ss;
	ସଂରଚନା notify notify = {0};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ଯଦି ((tmp = getenv("SSD_NICELEVEL")))
		ଯଦି (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ଯଦି ((tmp = getenv("SSD_IONICELEVEL"))) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ଯଦି (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ଯଦି (ionicec == 0)
			ioniced = 0;
		ଅନ୍ୟଥା ଯଦି (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ଯଦି ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ଯଦି (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ଯଦି (home == ଖାଲି || p == ଖାଲି) {
		pw = getpwuid(getuid());
		ଯଦି (pw != ଖାଲି) {
			ଯଦି (p == ଖାଲି)
				setenv("USER", pw->pw_name, 1);
			ଯଦି (home == ଖାଲି) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
		ବାଛ (opt) {
		ସ୍ଥିତି LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ଯଦି (cap_iab == ଖାଲି)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ଅନ୍ୟଥା
			eerrorx("Capabilities support not enabled");
#endif
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_SECBITS:
#ifdef __linux__
			ଯଦି (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ଖାଲି;
			secbits = strtoul(optarg, &tmp, 0);
			ଯଦି (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ଅନ୍ୟଥା
			eerrorx("Capabilities support not enabled");
#endif
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ଅନ୍ୟଥା
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'I': /* --ionice */
			ଯଦି (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ଯଦି (ionicec == 0)
				ioniced = 0;
			ଅନ୍ୟଥା ଯଦି (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'K':  /* --stop */
			stop = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'G':  /* --stop-group */
			stopgroup = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'N':  /* --nice */
			ଯଦି (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ଯଦି (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'P':  /* --progress */
			progress = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'S':  /* --start */
			start = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'b':  /* --background */
			background = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		ସ୍ଥିତି 'u':  /* --user <username>|<uid> */
		{
			ଅକ୍ଷର dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ଯଦି (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ଅନ୍ୟଥା
				pw = getpwuid((uid_t)tid);

			ଯଦି (pw == ଖାଲି)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ଯଦି (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ଯଦି (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ଯଦି (gid == 0)
				gid = pw->pw_gid;

			ଯଦି (p) {
				tmp = strsep (&p, ":");
				ଯଦି (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ଅନ୍ୟଥା
					gr = getgrgid((gid_t) tid);

				ଯଦି (gr == ଖାଲି)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ଭାଙ୍ଗ;

		ସ୍ଥିତି 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'e': /* --env */
			putenv(optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'g':  /* --group <group>|<gid> */
			ଯଦି (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ଅନ୍ୟଥା
				gr = getgrgid((gid_t)tid);
			ଯଦି (gr == ଖାଲି)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'i': /* --interpreted */
			interpreted = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'k':
			ଯଦି (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'm':  /* --make-pidfile */
			makepidfile = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'n':  /* --name <process-name> */
			name = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 't':  /* --test */
			test = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'r':  /* --chroot /new/root */
			ch_ମୂଳ = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'w':
			ଯଦି (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'x':  /* --exec <executable> */
			exec = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	ଯଦି (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ଯଦି (!exec)
		exec = startas;
	ଅନ୍ୟଥା ଯଦି (!name)
		name = startas;

	ଯଦି (!exec) {
		exec = *argv;
		ଯଦି (!exec)
			exec = name;
		ଯଦି (name && start)
			*argv = name;
	} ଅନ୍ୟଥା ଯଦି (name) {
		*--argv = name;
		++argc;
	} ଅନ୍ୟଥା ଯଦି (exec) {
		*--argv = exec;
		++argc;
	};

	ଯଦି (stop || sig != -1) {
		ଯଦି (sig == -1)
			sig = SIGTERM;
		ଯଦି (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ଯଦି (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ଯଦି (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ଯଦି (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ଯଦି (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ଯଦି (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ଅନ୍ୟଥା {
		ଯଦି (!exec)
			eerrorx("%s: nothing to start", applet);
		ଯଦି (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ଯଦି ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ଯଦି ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ଯଦି (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ଯଦି (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ଯଦି (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ଯଦି (ch_ମୂଳ && *ch_ମୂଳ == '~')
		ch_ମୂଳ = expand_home(home, ch_ମୂଳ);
	ଯଦି (exec) {
		ଯଦି (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ଯଦି (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ଯଦି (ch_ମୂଳ)
				xasprintf(&exec_file, "%s/%s", ch_ମୂଳ, exec);
			ଅନ୍ୟଥା
				xasprintf(&exec_file, "%s", exec);
		} ଅନ୍ୟଥା {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ଖାଲି;
			ଯେତେବେଳେ ((token = strsep(&p, ":"))) {
				ଯଦି (ch_ମୂଳ)
					xasprintf(&exec_file, "%s/%s/%s", ch_ମୂଳ, token, exec);
				ଅନ୍ୟଥା
					xasprintf(&exec_file, "%s/%s", token, exec);
				ଯଦି (exec_file && access(exec_file, F_OK) == 0)
					ଭାଙ୍ଗ;
				ମୁକ୍ତ(exec_file);
				exec_file = ଖାଲି;
			}
			ମୁକ୍ତ(tmp);
		}
	}
	ଯଦି (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		ମୁକ୍ତ(exec_file);
		exit(EXIT_FAILURE);
	}
	ଯଦି (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ଯଦି (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ଯଦି (fp) {
			ଧାଡ଼ି = ଖାଲି;
			ଯଦି (xgetline(&ଧାଡ଼ି, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = ଧାଡ଼ି;
			fclose(fp);
			ଯଦି (p != ଖାଲି && ଧାଡ଼ି[0] == '#' && ଧାଡ଼ି[1] == '!') {
				p = ଧାଡ଼ି + 2;
				/* Strip leading spaces */
				ଯେତେବେଳେ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				ମୁକ୍ତ(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				ପାଇଁ (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(ଆକାର(ଅକ୍ଷର *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ଯଦି (p)
					nav[len++] = p;
				ପାଇଁ (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ଖାଲି;
			}
		}
	}
	margv = nav ? nav : argv;

	ଯଦି (stop || sig != -1) {
		ଯଦି (sig == -1)
			sig = SIGTERM;
		ଯଦି (!stop)
			oknodo = true;
		ଯଦି (retry)
			parse_schedule(applet, retry, sig);
		ଅନ୍ୟଥା ଯଦି (test || oknodo)
			parse_schedule(applet, "0", sig);
		ଅନ୍ୟଥା
			parse_schedule(applet, ଖାଲି, sig);
		ଯଦି (pidfile) {
			pid = get_pid(applet, pidfile);
			ଯଦି (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} ଅନ୍ୟଥା {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)margv,
		    pid, uid, stopgroup, test, progress, false);

		ଯଦି (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		ଯଦି (test || oknodo)
			ଫେରନ୍ତୁ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ଯଦି (pidfile)
			unlink(pidfile);
		ଯଦି (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	ଯଦି (pidfile)
		pid = get_pid(applet, pidfile);
	ଅନ୍ୟଥା
		pid = 0;

	ଯଦି (pid)
		pids = rc_find_pids(ଖାଲି, ଖାଲି, 0, pid);
	ଅନ୍ୟଥା
		pids = rc_find_pids(exec, (ଅଚଳ ଅକ୍ଷର * ଅଚଳ *) argv, uid, 0);
	ଯଦି (pids)
		eerrorx("%s: %s is already running", applet, exec);

	ମୁକ୍ତ(pids);
	ଯଦି (test) {
		ଯଦି (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		ଯେତେବେଳେ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		ଯଦି (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ଯଦି (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ଯଦି (ch_ମୂଳ)
			einfo("in root `%s'", ch_ମୂଳ);
		ଯଦି (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ଯଦି (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ଯଦି (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ଯଦି (pidfile)
		unlink(pidfile);

	ଯଦି (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ଯଦି (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ଯଦି ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ଯଦି (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା group_ଗଣନା = ARRAY_SIZE(group_buf);
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

		ଯଦି (nicelevel != INT_MIN) {
			ଯଦି (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ଯଦି (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ଅନ୍ୟଥା ଯଦି (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ଯଦି (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ଯଦି (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ଯଦି (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		ଯଦି (changeuser != ଖାଲି) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ଯଦି (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ଯଦି (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ଯଦି (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		ଯଦି (changeuser && getgrouplist(changeuser, gid, group_list, &group_ଗଣନା) < 0) {
			group_list = xmalloc(group_ଗଣନା * ଆକାର(*group_list));
			ଯଦି (getgrouplist(changeuser, gid, group_list, &group_ଗଣନା) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ଯଦି (ch_ମୂଳ && chroot(ch_ମୂଳ) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_ମୂଳ, strerror(errno));

		ଯଦି (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ଯଦି (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ଯଦି (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ଯଦି (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ଯଦି (changeuser && setgroups(group_ଗଣନା, group_list))
			eerrorx("%s: setgroups() failed", applet);
		ଯଦି (group_list != group_buf)
			ମୁକ୍ତ(group_list);
#ifdef __linux__
		ଯଦି (uid && cap_setuid(uid))
#ଅନ୍ୟଥା
		ଯଦି (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		ଯଦି (cap_iab != ଖାଲି) {
			i = cap_iab_set_proc(cap_iab);

			ଯଦି (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ଯଦି (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ଯଦି (secbits != 0) {
			ଯଦି (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		ଯଦି (no_new_privs) {
			ଯଦି (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		ଯେତେବେଳେ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		ଯଦି (changeuser != ଖାଲି) {
			pamenv = (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)pam_getenvlist(pamh);
			ଯଦି (pamenv) {
				ଯେତେବେଳେ (*pamenv) {
					/* Don't add strings unless they set a var */
					ଯଦି (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ଅନ୍ୟଥା
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ଯଦି (strncmp(env->ମୂଲ୍ୟ, "RC_", 3) == 0
					|| strncmp(env->ମୂଲ୍ୟ, "SSD_", 4) == 0
					|| strncmp(env->ମୂଲ୍ୟ, "EINFO_", 6) == 0) {
				*strchr(env->ମୂଲ୍ୟ, '=') = '\0';
				unsetenv(env->ମୂଲ୍ୟ);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ଯଦି ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			ଯେତେବେଳେ (token && *token) {
				p = strchr(token, ':');
				ଯଦି (p) {
					*p++ = '\0';
					ଯେତେବେଳେ (*p == ':')
						p++;
				}
				ଯଦି (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					ଯଦି (np != newpath)
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
		ଯଦି (redirect_stdin) {
			ଯଦି ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ଯଦି (redirect_stdout) {
			ଯଦି ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ଅନ୍ୟଥା ଯଦି (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ଯଦି (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ଯଦି (redirect_stderr) {
			ଯଦି ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ଅନ୍ୟଥା ଯଦି (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ଯଦି (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ଯଦି (background)
			dup2(stdin_fd, STDIN_FILENO);
		ଯଦି (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ଯଦି (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ଯଦି (notify.type == NOTIFY_FD) {
			ଯଦି (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ଯଦି (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ଯଦି (notify.pipe[1] == notify.fd) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = fcntl(notify.fd, F_GETFD, 0);
				ଯଦି (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ଯଦି (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ଯଦି (scheduler != ଖାଲି) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା scheduler_index;
			ସଂରଚନା sched_param sched =  {.sched_priority = sched_prio};
			ଯଦି (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			ଅନ୍ୟଥା ଯଦି (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			ଅନ୍ୟଥା ଯଦି (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			ଅନ୍ୟଥା ଯଦି (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			ଅନ୍ୟଥା ଯଦି (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			ଅନ୍ୟଥା ଯଦି (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ଯଦି (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			ଯଦି (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ଅନ୍ୟଥା ଯଦି (sched_prio != -1) {
			ଅଚଳ ସଂରଚନା sched_param sched =  {.sched_priority = sched_prio};
			ଯଦି (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		ଯଦି (changeuser != ଖାଲି && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ଯେତେବେଳେ ((ss = ପଠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ଯଦି (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ଯଦି (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		କର {
			pid = waitpid(spid, &i, 0);
			ଯଦି (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ଫେରନ୍ତୁ -1;
			}
		} ଯେତେବେଳେ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ଯଦି (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ଯଦି (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_ମୂଲ୍ୟ("rc_start_wait"))))
	{
		ଯଦି (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	ଯଦି (notify.type != NOTIFY_NONE) {
		ଯଦି (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ଅନ୍ୟଥା ଯଦି (start_wait > 0) {
		ସଂରଚନା timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		ଯଦି (nanosleep(&ts, ଖାଲି) == -1) {
			ଯଦି (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ଫେରନ୍ତୁ 0;
			}
		}
		ଯଦି (background) {
			ଯଦି (kill(pid, 0) == 0)
				alive = true;
		} ଅନ୍ୟଥା {
			ଯଦି (pidfile) {
				pid = get_pid(applet, pidfile);
				ଯଦି (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ଅନ୍ୟଥା
				pid = 0;
			ଯଦି (do_stop(applet, exec, (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ଯଦି (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ଯଦି (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
