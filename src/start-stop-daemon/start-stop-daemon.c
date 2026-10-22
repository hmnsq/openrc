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

#যদি defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <getopt.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <grp.h>
#অন্তর্ভাৱ <pthread.h>
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <sched.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <sys/ioctl.h>
#অন্তর্ভাৱ <sys/resource.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <sys/time.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <inttypes.h>
#অন্তর্ভাৱ <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#অন্তর্ভাৱ <termios.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
স্থির সংরচনা pam_conv conv = { রিক্ত, রিক্ত};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "misc.h"
#অন্তর্ভাৱ "rc_exec.h"
#অন্তর্ভাৱ "schedules.h"
#অন্তর্ভাৱ "_usage.h"
#অন্তর্ভাৱ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
গণনা {
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

ধ্রুৱ ৱর্ণ *applet = রিক্ত;
ধ্রুৱ ৱর্ণ *extraopts = রিক্ত;
ধ্রুৱ ৱর্ণ getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ধ্রুৱ সংরচনা option longopts[] = {
	{ "capabilities", 1, রিক্ত, LONGOPT_CAPABILITIES},
	{ "secbits",      1, রিক্ত, LONGOPT_SECBITS},
	{ "no-new-privs", 0, রিক্ত, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, রিক্ত, 'I'},
	{ "stop",         0, রিক্ত, 'K'},
	{ "nicelevel",    1, রিক্ত, 'N'},
	{ "oom-score-adj",1, রিক্ত, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, রিক্ত, 'R'},
	{ "start",        0, রিক্ত, 'S'},
	{ "startas",      1, রিক্ত, 'a'},
	{ "background",   0, রিক্ত, 'b'},
	{ "chuid",        1, রিক্ত, 'c'},
	{ "chdir",        1, রিক্ত, 'd'},
	{ "env",          1, রিক্ত, 'e'},
	{ "umask",        1, রিক্ত, 'k'},
	{ "group",        1, রিক্ত, 'g'},
	{ "stop-group",   0, রিক্ত, 'G'},
	{ "interpreted",  0, রিক্ত, 'i'},
	{ "make-pidfile", 0, রিক্ত, 'm'},
	{ "name",         1, রিক্ত, 'n'},
	{ "oknodo",       0, রিক্ত, 'o'},
	{ "pidfile",      1, রিক্ত, 'p'},
	{ "signal",       1, রিক্ত, 's'},
	{ "test",         0, রিক্ত, 't'},
	{ "user",         1, রিক্ত, 'u'},
	{ "chroot",       1, রিক্ত, 'r'},
	{ "wait",         1, রিক্ত, 'w'},
	{ "exec",         1, রিক্ত, 'x'},
	{ "stdin",        1, রিক্ত, '0'},
	{ "stdout",       1, রিক্ত, '1'},
	{ "stderr",       1, রিক্ত, '2'},
	{ "stdout-logger",1, রিক্ত, '3'},
	{ "stderr-logger",1, রিক্ত, '4'},
	{ "progress",     0, রিক্ত, 'P'},
	{ "scheduler",    1, রিক্ত, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, রিক্ত, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, রিক্ত, LONGOPT_NOTIFY},
	longopts_COMMON
};
ধ্রুৱ ৱর্ণ * ধ্রুৱ longopts_help[] = {
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
ধ্রুৱ ৱর্ণ *usagestring = রিক্ত;

স্থির ৱর্ণ **nav;

স্থির ৱর্ণ *changeuser, *ch_মূল, *ch_dir;

বাহ্য ৱর্ণ **environ;

#যদি !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#যদি !defined(__DragonFly__)
স্থির অন্তঃস্থ পূর্ণাঙ্ক ioprio_set(পূর্ণাঙ্ক which RC_UNUSED,
			     পূর্ণাঙ্ক who RC_UNUSED,
			     পূর্ণাঙ্ক ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ঘূৰাই syscall(SYS_ioprio_set, which, who, ioprio);
#অন্যথা
	ঘূৰাই 0;
#endif
}
#endif

স্থির শূন্য
cleanup(শূন্য)
{
	মুক্ত(changeuser);
	মুক্ত(nav);
	free_schedulelist();
}

স্থির শূন্য
handle_signal(পূর্ণাঙ্ক sig)
{
	পূর্ণাঙ্ক status;
	পূর্ণাঙ্ক serrno = errno;
	ধ্রুৱ ৱর্ণ *signame = রিক্ত;

	ৱিকল্প (sig) {
	পক্ষ SIGINT:
		যদি (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	পক্ষ SIGTERM:
		যদি (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	পক্ষ SIGQUIT:
		যদি (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	পক্ষ SIGCHLD:
		বাবে (;;) {
			যদি (waitpid(-1, &status, WNOHANG) < 0) {
				যদি (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				ভঙ্গ;
			}
		}
		ভঙ্গ;

	স্ৱভাৱ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

স্থির ৱর্ণ *
expand_home(ধ্রুৱ ৱর্ণ *home, ধ্রুৱ ৱর্ণ *path)
{
	ৱর্ণ *opath, *ppath, *p, *nh;
	সংরচনা passwd *pw;

	যদি (!path || *path != '~')
		ঘূৰাই xstrdup(path);

	opath = ppath = xstrdup(path);
	যদি (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		যদি (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		যদি (pw) {
			home = pw->pw_dir;
			ppath = p;
			যদি (ppath)
				*ppath = '/';
		} অন্যথা
			home = রিক্ত;
	} অন্যথা
		ppath++;

	যদি (!home) {
	মুক্ত(opath);
		ঘূৰাই xstrdup(path);
	}
	যদি (!ppath) {
		মুক্ত(opath);
		ঘূৰাই xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	মুক্ত(opath);
	ঘূৰাই nh;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	পূর্ণাঙ্ক devnull_fd = -1;
#ifdef TIOCNOTTY
	পূর্ণাঙ্ক tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = রিক্ত;
	পূর্ণাঙ্ক pamr;
	ধ্রুৱ ৱর্ণ *ধ্রুৱ *pamenv = রিক্ত;
#endif

	পূর্ণাঙ্ক opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	ৱর্ণ *exec = রিক্ত;
	ৱর্ণ *startas = রিক্ত;
	ৱর্ণ *name = রিক্ত;
	ৱর্ণ *pidfile = রিক্ত;
	ৱর্ণ *retry = রিক্ত;
	পূর্ণাঙ্ক sig = -1;
	পূর্ণাঙ্ক nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	পূর্ণাঙ্ক oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	ৱর্ণ *home = রিক্ত;
	পূর্ণাঙ্ক tid = 0;
	ৱর্ণ *redirect_stdin = রিক্ত;
	ৱর্ণ *redirect_stderr = রিক্ত;
	ৱর্ণ *redirect_stdout = রিক্ত;
	ৱর্ণ *stderr_process = রিক্ত;
	ৱর্ণ *stdout_process = রিক্ত;
	পূর্ণাঙ্ক stdin_fd;
	পূর্ণাঙ্ক stdout_fd;
	পূর্ণাঙ্ক stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	পূর্ণাঙ্ক i;
	ৱর্ণ *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ৱর্ণ *tmp, *newpath, *np;
	ৱর্ণ *p;
	ৱর্ণ *token;
	ৱর্ণ *exec_file = রিক্ত;
	সংরচনা passwd *pw;
	সংরচনা group *gr;
	bool stopgroup = false;
	ৱর্ণ *সাৰি = রিক্ত;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	ৱর্ণ **margv;
	unsigned পূর্ণাঙ্ক start_wait = 0;
	ধ্রুৱ ৱর্ণ *scheduler = রিক্ত;
	পূর্ণাঙ্ক sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = রিক্ত;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	পূর্ণাঙ্ক pipefd[2];
	ৱর্ণ readbuf[1];
	ssize_t ss;
	সংরচনা notify notify = {0};
	পূর্ণাঙ্ক ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	যদি ((tmp = getenv("SSD_NICELEVEL")))
		যদি (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	যদি ((tmp = getenv("SSD_IONICELEVEL"))) {
		পূর্ণাঙ্ক n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		যদি (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		যদি (ionicec == 0)
			ioniced = 0;
		অন্যথা যদি (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	যদি ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		যদি (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	যদি (home == রিক্ত || p == রিক্ত) {
		pw = getpwuid(getuid());
		যদি (pw != রিক্ত) {
			যদি (p == রিক্ত)
				setenv("USER", pw->pw_name, 1);
			যদি (home == রিক্ত) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	যেতিয়ালৈ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (পূর্ণাঙ্ক *) 0)) != -1)
		ৱিকল্প (opt) {
		পক্ষ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			যদি (cap_iab == রিক্ত)
				eerrorx("Could not parse iab: %s", strerror(errno));
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			ভঙ্গ;

		পক্ষ LONGOPT_SECBITS:
#ifdef __linux__
			যদি (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = রিক্ত;
			secbits = strtoul(optarg, &tmp, 0);
			যদি (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			ভঙ্গ;

		পক্ষ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#অন্যথা
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			ভঙ্গ;

		পক্ষ 'I': /* --ionice */
			যদি (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			যদি (ionicec == 0)
				ioniced = 0;
			অন্যথা যদি (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ভঙ্গ;

		পক্ষ 'K':  /* --stop */
			stop = true;
			ভঙ্গ;

		পক্ষ 'G':  /* --stop-group */
			stopgroup = true;
			ভঙ্গ;

		পক্ষ 'N':  /* --nice */
			যদি (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ভঙ্গ;

		পক্ষ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			যদি (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ভঙ্গ;

		পক্ষ 'P':  /* --progress */
			progress = true;
			ভঙ্গ;

		পক্ষ 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			ভঙ্গ;

		পক্ষ 'S':  /* --start */
			start = true;
			ভঙ্গ;

		পক্ষ 'b':  /* --background */
			background = true;
			ভঙ্গ;

		পক্ষ 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		পক্ষ 'u':  /* --user <username>|<uid> */
		{
			ৱর্ণ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			যদি (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			অন্যথা
				pw = getpwuid((uid_t)tid);

			যদি (pw == রিক্ত)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			যদি (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			যদি (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			যদি (gid == 0)
				gid = pw->pw_gid;

			যদি (p) {
				tmp = strsep (&p, ":");
				যদি (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				অন্যথা
					gr = getgrgid((gid_t) tid);

				যদি (gr == রিক্ত)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ভঙ্গ;

		পক্ষ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ভঙ্গ;

		পক্ষ 'e': /* --env */
			putenv(optarg);
			ভঙ্গ;

		পক্ষ 'g':  /* --group <group>|<gid> */
			যদি (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			অন্যথা
				gr = getgrgid((gid_t)tid);
			যদি (gr == রিক্ত)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ভঙ্গ;

		পক্ষ 'i': /* --interpreted */
			interpreted = true;
			ভঙ্গ;

		পক্ষ 'k':
			যদি (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ভঙ্গ;

		পক্ষ 'm':  /* --make-pidfile */
			makepidfile = true;
			ভঙ্গ;

		পক্ষ 'n':  /* --name <process-name> */
			name = optarg;
			ভঙ্গ;

		পক্ষ 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			ভঙ্গ;

		পক্ষ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ভঙ্গ;

		পক্ষ 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			ভঙ্গ;

		পক্ষ 't':  /* --test */
			test = true;
			ভঙ্গ;

		পক্ষ 'r':  /* --chroot /new/root */
			ch_মূল = optarg;
			ভঙ্গ;

		পক্ষ 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			ভঙ্গ;
		পক্ষ 'w':
			যদি (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			ভঙ্গ;
		পক্ষ 'x':  /* --exec <executable> */
			exec = optarg;
			ভঙ্গ;

		পক্ষ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ভঙ্গ;

		পক্ষ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ভঙ্গ;

		পক্ষ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ভঙ্গ;

		পক্ষ '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ভঙ্গ;

		পক্ষ '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ভঙ্গ;

		পক্ষ LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			ভঙ্গ;

		পক্ষ LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			ভঙ্গ;

		পক্ষ LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			ভঙ্গ;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	যদি (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	যদি (!exec)
		exec = startas;
	অন্যথা যদি (!name)
		name = startas;

	যদি (!exec) {
		exec = *argv;
		যদি (!exec)
			exec = name;
		যদি (name && start)
			*argv = name;
	} অন্যথা যদি (name) {
		*--argv = name;
		++argc;
	} অন্যথা যদি (exec) {
		*--argv = exec;
		++argc;
	};

	যদি (stop || sig != -1) {
		যদি (sig == -1)
			sig = SIGTERM;
		যদি (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		যদি (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		যদি (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		যদি (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		যদি (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		যদি (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} অন্যথা {
		যদি (!exec)
			eerrorx("%s: nothing to start", applet);
		যদি (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		যদি ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		যদি ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		যদি (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		যদি (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	যদি (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	যদি (ch_মূল && *ch_মূল == '~')
		ch_মূল = expand_home(home, ch_মূল);
	যদি (exec) {
		যদি (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		যদি (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			যদি (ch_মূল)
				xasprintf(&exec_file, "%s/%s", ch_মূল, exec);
			অন্যথা
				xasprintf(&exec_file, "%s", exec);
		} অন্যথা {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = রিক্ত;
			যেতিয়ালৈ ((token = strsep(&p, ":"))) {
				যদি (ch_মূল)
					xasprintf(&exec_file, "%s/%s/%s", ch_মূল, token, exec);
				অন্যথা
					xasprintf(&exec_file, "%s/%s", token, exec);
				যদি (exec_file && access(exec_file, F_OK) == 0)
					ভঙ্গ;
				মুক্ত(exec_file);
				exec_file = রিক্ত;
			}
			মুক্ত(tmp);
		}
	}
	যদি (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		মুক্ত(exec_file);
		exit(EXIT_FAILURE);
	}
	যদি (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	যদি (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		যদি (fp) {
			সাৰি = রিক্ত;
			যদি (xgetline(&সাৰি, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = সাৰি;
			fclose(fp);
			যদি (p != রিক্ত && সাৰি[0] == '#' && সাৰি[1] == '!') {
				p = সাৰি + 2;
				/* Strip leading spaces */
				যেতিয়ালৈ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				মুক্ত(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				বাবে (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(পরিমাণ(ৱর্ণ *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				যদি (p)
					nav[len++] = p;
				বাবে (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = রিক্ত;
			}
		}
	}
	margv = nav ? nav : argv;

	যদি (stop || sig != -1) {
		যদি (sig == -1)
			sig = SIGTERM;
		যদি (!stop)
			oknodo = true;
		যদি (retry)
			parse_schedule(applet, retry, sig);
		অন্যথা যদি (test || oknodo)
			parse_schedule(applet, "0", sig);
		অন্যথা
			parse_schedule(applet, রিক্ত, sig);
		যদি (pidfile) {
			pid = get_pid(applet, pidfile);
			যদি (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} অন্যথা {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)margv,
		    pid, uid, stopgroup, test, progress, false);

		যদি (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		যদি (test || oknodo)
			ঘূৰাই i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		যদি (pidfile)
			unlink(pidfile);
		যদি (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	যদি (pidfile)
		pid = get_pid(applet, pidfile);
	অন্যথা
		pid = 0;

	যদি (pid)
		pids = rc_find_pids(রিক্ত, রিক্ত, 0, pid);
	অন্যথা
		pids = rc_find_pids(exec, (ধ্রুৱ ৱর্ণ * ধ্রুৱ *) argv, uid, 0);
	যদি (pids)
		eerrorx("%s: %s is already running", applet, exec);

	মুক্ত(pids);
	যদি (test) {
		যদি (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		যেতিয়ালৈ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		যদি (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		যদি (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		যদি (ch_মূল)
			einfo("in root `%s'", ch_মূল);
		যদি (ch_dir)
			einfo("in dir `%s'", ch_dir);
		যদি (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		যদি (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	যদি (pidfile)
		unlink(pidfile);

	যদি (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	যদি (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	যদি ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	যদি (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		পূর্ণাঙ্ক group_গুনতি = ARRAY_SIZE(group_buf);
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

		যদি (nicelevel != INT_MIN) {
			যদি (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			যদি (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} অন্যথা যদি (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		যদি (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		যদি (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			যদি (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		যদি (changeuser != রিক্ত) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			যদি (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			যদি (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			যদি (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		যদি (changeuser && getgrouplist(changeuser, gid, group_list, &group_গুনতি) < 0) {
			group_list = xmalloc(group_গুনতি * পরিমাণ(*group_list));
			যদি (getgrouplist(changeuser, gid, group_list, &group_গুনতি) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		যদি (ch_মূল && chroot(ch_মূল) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_মূল, strerror(errno));

		যদি (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		যদি (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			যদি (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		যদি (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		যদি (changeuser && setgroups(group_গুনতি, group_list))
			eerrorx("%s: setgroups() failed", applet);
		যদি (group_list != group_buf)
			মুক্ত(group_list);
#ifdef __linux__
		যদি (uid && cap_setuid(uid))
#অন্যথা
		যদি (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		যদি (cap_iab != রিক্ত) {
			i = cap_iab_set_proc(cap_iab);

			যদি (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			যদি (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		যদি (secbits != 0) {
			যদি (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		যদি (no_new_privs) {
			যদি (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		যেতিয়ালৈ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		যদি (changeuser != রিক্ত) {
			pamenv = (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)pam_getenvlist(pamh);
			যদি (pamenv) {
				যেতিয়ালৈ (*pamenv) {
					/* Don't add strings unless they set a var */
					যদি (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					অন্যথা
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			যদি (strncmp(env->মান, "RC_", 3) == 0
					|| strncmp(env->মান, "SSD_", 4) == 0
					|| strncmp(env->মান, "EINFO_", 6) == 0) {
				*strchr(env->মান, '=') = '\0';
				unsetenv(env->মান);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		যদি ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			যেতিয়ালৈ (token && *token) {
				p = strchr(token, ':');
				যদি (p) {
					*p++ = '\0';
					যেতিয়ালৈ (*p == ':')
						p++;
				}
				যদি (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					যদি (np != newpath)
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
		যদি (redirect_stdin) {
			যদি ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		যদি (redirect_stdout) {
			যদি ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}অন্যথা যদি (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			যদি (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		যদি (redirect_stderr) {
			যদি ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}অন্যথা যদি (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			যদি (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		যদি (background)
			dup2(stdin_fd, STDIN_FILENO);
		যদি (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		যদি (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		যদি (notify.type == NOTIFY_FD) {
			যদি (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			যদি (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			যদি (notify.pipe[1] == notify.fd) {
				পূর্ণাঙ্ক flags = fcntl(notify.fd, F_GETFD, 0);
				যদি (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				যদি (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		যদি (scheduler != রিক্ত) {
			পূর্ণাঙ্ক scheduler_index;
			সংরচনা sched_param sched =  {.sched_priority = sched_prio};
			যদি (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			অন্যথা যদি (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			অন্যথা যদি (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			অন্যথা যদি (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			অন্যথা যদি (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			অন্যথা যদি (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			যদি (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			যদি (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} অন্যথা যদি (sched_prio != -1) {
			ধ্রুৱ সংরচনা sched_param sched =  {.sched_priority = sched_prio};
			যদি (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		যদি (changeuser != রিক্ত && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	যেতিয়ালৈ ((ss = পঠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	যদি (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	যদি (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		কৰা {
			pid = waitpid(spid, &i, 0);
			যদি (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ঘূৰাই -1;
			}
		} যেতিয়ালৈ (!WIFEXITED(i) && !WIFSIGNALED(i));
		যদি (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	যদি (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_মান("rc_start_wait"))))
	{
		যদি (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	যদি (notify.type != NOTIFY_NONE) {
		যদি (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} অন্যথা যদি (start_wait > 0) {
		সংরচনা timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		যদি (nanosleep(&ts, রিক্ত) == -1) {
			যদি (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ঘূৰাই 0;
			}
		}
		যদি (background) {
			যদি (kill(pid, 0) == 0)
				alive = true;
		} অন্যথা {
			যদি (pidfile) {
				pid = get_pid(applet, pidfile);
				যদি (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} অন্যথা
				pid = 0;
			যদি (do_stop(applet, exec, (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		যদি (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	যদি (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ধ্রুৱ ৱর্ণ *ধ্রুৱ *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
