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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <grp.h>
#অন্তর্ভাব <pthread.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <sched.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <sys/resource.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/time.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#অন্তর্ভাব <termios.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
স্থির কাঠামো pam_conv conv = { ফাঁকা, ফাঁকা};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "rc_exec.h"
#অন্তর্ভাব "schedules.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

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

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = ফাঁকা;
ধ্রুবক অক্ষর getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "capabilities", 1, ফাঁকা, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ফাঁকা, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ফাঁকা, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ফাঁকা, 'I'},
	{ "stop",         0, ফাঁকা, 'K'},
	{ "nicelevel",    1, ফাঁকা, 'N'},
	{ "oom-score-adj",1, ফাঁকা, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ফাঁকা, 'R'},
	{ "start",        0, ফাঁকা, 'S'},
	{ "startas",      1, ফাঁকা, 'a'},
	{ "background",   0, ফাঁকা, 'b'},
	{ "chuid",        1, ফাঁকা, 'c'},
	{ "chdir",        1, ফাঁকা, 'd'},
	{ "env",          1, ফাঁকা, 'e'},
	{ "umask",        1, ফাঁকা, 'k'},
	{ "group",        1, ফাঁকা, 'g'},
	{ "stop-group",   0, ফাঁকা, 'G'},
	{ "interpreted",  0, ফাঁকা, 'i'},
	{ "make-pidfile", 0, ফাঁকা, 'm'},
	{ "name",         1, ফাঁকা, 'n'},
	{ "oknodo",       0, ফাঁকা, 'o'},
	{ "pidfile",      1, ফাঁকা, 'p'},
	{ "signal",       1, ফাঁকা, 's'},
	{ "test",         0, ফাঁকা, 't'},
	{ "user",         1, ফাঁকা, 'u'},
	{ "chroot",       1, ফাঁকা, 'r'},
	{ "wait",         1, ফাঁকা, 'w'},
	{ "exec",         1, ফাঁকা, 'x'},
	{ "stdin",        1, ফাঁকা, '0'},
	{ "stdout",       1, ফাঁকা, '1'},
	{ "stderr",       1, ফাঁকা, '2'},
	{ "stdout-logger",1, ফাঁকা, '3'},
	{ "stderr-logger",1, ফাঁকা, '4'},
	{ "progress",     0, ফাঁকা, 'P'},
	{ "scheduler",    1, ফাঁকা, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ফাঁকা, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ফাঁকা, LONGOPT_NOTIFY},
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
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
ধ্রুবক অক্ষর *usagestring = ফাঁকা;

স্থির অক্ষর **nav;

স্থির অক্ষর *changeuser, *ch_মূল, *ch_dir;

বাহ্যিক অক্ষর **environ;

#যদি !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#যদি !defined(__DragonFly__)
স্থির অন্তঃরেখ পূর্ণসংখ্যা ioprio_set(পূর্ণসংখ্যা which RC_UNUSED,
			     পূর্ণসংখ্যা who RC_UNUSED,
			     পূর্ণসংখ্যা ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ফেরত syscall(SYS_ioprio_set, which, who, ioprio);
#অন্যথা
	ফেরত 0;
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
handle_signal(পূর্ণসংখ্যা sig)
{
	পূর্ণসংখ্যা status;
	পূর্ণসংখ্যা serrno = errno;
	ধ্রুবক অক্ষর *signame = ফাঁকা;

	নির্বাচন (sig) {
	ক্ষেত্র SIGINT:
		যদি (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ক্ষেত্র SIGTERM:
		যদি (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ক্ষেত্র SIGQUIT:
		যদি (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	ক্ষেত্র SIGCHLD:
		জন্য (;;) {
			যদি (waitpid(-1, &status, WNOHANG) < 0) {
				যদি (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				বিরতি;
			}
		}
		বিরতি;

	পূর্বনির্ধারিত:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

স্থির অক্ষর *
expand_home(ধ্রুবক অক্ষর *home, ধ্রুবক অক্ষর *path)
{
	অক্ষর *opath, *ppath, *p, *nh;
	কাঠামো passwd *pw;

	যদি (!path || *path != '~')
		ফেরত xstrdup(path);

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
			home = ফাঁকা;
	} অন্যথা
		ppath++;

	যদি (!home) {
	মুক্ত(opath);
		ফেরত xstrdup(path);
	}
	যদি (!ppath) {
		মুক্ত(opath);
		ফেরত xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	মুক্ত(opath);
	ফেরত nh;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা devnull_fd = -1;
#ifdef TIOCNOTTY
	পূর্ণসংখ্যা tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = ফাঁকা;
	পূর্ণসংখ্যা pamr;
	ধ্রুবক অক্ষর *ধ্রুবক *pamenv = ফাঁকা;
#endif

	পূর্ণসংখ্যা opt;
	size_t আকার_2 = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	অক্ষর *exec = ফাঁকা;
	অক্ষর *startas = ফাঁকা;
	অক্ষর *name = ফাঁকা;
	অক্ষর *pidfile = ফাঁকা;
	অক্ষর *retry = ফাঁকা;
	পূর্ণসংখ্যা sig = -1;
	পূর্ণসংখ্যা nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	পূর্ণসংখ্যা oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	অক্ষর *home = ফাঁকা;
	পূর্ণসংখ্যা tid = 0;
	অক্ষর *redirect_stdin = ফাঁকা;
	অক্ষর *redirect_stderr = ফাঁকা;
	অক্ষর *redirect_stdout = ফাঁকা;
	অক্ষর *stderr_process = ফাঁকা;
	অক্ষর *stdout_process = ফাঁকা;
	পূর্ণসংখ্যা stdin_fd;
	পূর্ণসংখ্যা stdout_fd;
	পূর্ণসংখ্যা stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	পূর্ণসংখ্যা i;
	অক্ষর *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	অক্ষর *tmp, *newpath, *np;
	অক্ষর *p;
	অক্ষর *token;
	অক্ষর *exec_file = ফাঁকা;
	কাঠামো passwd *pw;
	কাঠামো group *gr;
	bool stopgroup = false;
	অক্ষর *সারি = ফাঁকা;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	অক্ষর **margv;
	unsigned পূর্ণসংখ্যা start_wait = 0;
	ধ্রুবক অক্ষর *scheduler = ফাঁকা;
	পূর্ণসংখ্যা sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = ফাঁকা;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	পূর্ণসংখ্যা pipefd[2];
	অক্ষর readbuf[1];
	ssize_t ss;
	কাঠামো notify notify = {0};
	পূর্ণসংখ্যা ret = EXIT_SUCCESS;

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
		পূর্ণসংখ্যা n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
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
	যদি (home == ফাঁকা || p == ফাঁকা) {
		pw = getpwuid(getuid());
		যদি (pw != ফাঁকা) {
			যদি (p == ফাঁকা)
				setenv("USER", pw->pw_name, 1);
			যদি (home == ফাঁকা) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	যখন ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (পূর্ণসংখ্যা *) 0)) != -1)
		নির্বাচন (opt) {
		ক্ষেত্র LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			যদি (cap_iab == ফাঁকা)
				eerrorx("Could not parse iab: %s", strerror(errno));
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			বিরতি;

		ক্ষেত্র LONGOPT_SECBITS:
#ifdef __linux__
			যদি (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ফাঁকা;
			secbits = strtoul(optarg, &tmp, 0);
			যদি (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			বিরতি;

		ক্ষেত্র LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#অন্যথা
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			বিরতি;

		ক্ষেত্র 'I': /* --ionice */
			যদি (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			যদি (ionicec == 0)
				ioniced = 0;
			অন্যথা যদি (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			বিরতি;

		ক্ষেত্র 'K':  /* --stop */
			stop = true;
			বিরতি;

		ক্ষেত্র 'G':  /* --stop-group */
			stopgroup = true;
			বিরতি;

		ক্ষেত্র 'N':  /* --nice */
			যদি (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			বিরতি;

		ক্ষেত্র LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			যদি (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			বিরতি;

		ক্ষেত্র 'P':  /* --progress */
			progress = true;
			বিরতি;

		ক্ষেত্র 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			বিরতি;

		ক্ষেত্র 'S':  /* --start */
			start = true;
			বিরতি;

		ক্ষেত্র 'b':  /* --background */
			background = true;
			বিরতি;

		ক্ষেত্র 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		ক্ষেত্র 'u':  /* --user <username>|<uid> */
		{
			অক্ষর dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			যদি (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			অন্যথা
				pw = getpwuid((uid_t)tid);

			যদি (pw == ফাঁকা)
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

				যদি (gr == ফাঁকা)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		বিরতি;

		ক্ষেত্র 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			বিরতি;

		ক্ষেত্র 'e': /* --env */
			putenv(optarg);
			বিরতি;

		ক্ষেত্র 'g':  /* --group <group>|<gid> */
			যদি (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			অন্যথা
				gr = getgrgid((gid_t)tid);
			যদি (gr == ফাঁকা)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			বিরতি;

		ক্ষেত্র 'i': /* --interpreted */
			interpreted = true;
			বিরতি;

		ক্ষেত্র 'k':
			যদি (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			বিরতি;

		ক্ষেত্র 'm':  /* --make-pidfile */
			makepidfile = true;
			বিরতি;

		ক্ষেত্র 'n':  /* --name <process-name> */
			name = optarg;
			বিরতি;

		ক্ষেত্র 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			বিরতি;

		ক্ষেত্র 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			বিরতি;

		ক্ষেত্র 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			বিরতি;

		ক্ষেত্র 't':  /* --test */
			test = true;
			বিরতি;

		ক্ষেত্র 'r':  /* --chroot /new/root */
			ch_মূল = optarg;
			বিরতি;

		ক্ষেত্র 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			বিরতি;
		ক্ষেত্র 'w':
			যদি (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			বিরতি;
		ক্ষেত্র 'x':  /* --exec <executable> */
			exec = optarg;
			বিরতি;

		ক্ষেত্র '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			বিরতি;

		ক্ষেত্র '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			বিরতি;

		ক্ষেত্র '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			বিরতি;

		ক্ষেত্র '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			বিরতি;

		ক্ষেত্র '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			বিরতি;

		ক্ষেত্র LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			বিরতি;

		ক্ষেত্র LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			বিরতি;

		ক্ষেত্র LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			বিরতি;

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
			exec_file = ফাঁকা;
			যখন ((token = strsep(&p, ":"))) {
				যদি (ch_মূল)
					xasprintf(&exec_file, "%s/%s/%s", ch_মূল, token, exec);
				অন্যথা
					xasprintf(&exec_file, "%s/%s", token, exec);
				যদি (exec_file && access(exec_file, F_OK) == 0)
					বিরতি;
				মুক্ত(exec_file);
				exec_file = ফাঁকা;
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
			সারি = ফাঁকা;
			যদি (xgetline(&সারি, &আকার_2, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = সারি;
			fclose(fp);
			যদি (p != ফাঁকা && সারি[0] == '#' && সারি[1] == '!') {
				p = সারি + 2;
				/* Strip leading spaces */
				যখন (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				মুক্ত(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				জন্য (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(আকার(অক্ষর *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				যদি (p)
					nav[len++] = p;
				জন্য (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ফাঁকা;
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
			parse_schedule(applet, ফাঁকা, sig);
		যদি (pidfile) {
			pid = get_pid(applet, pidfile);
			যদি (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} অন্যথা {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ধ্রুবক অক্ষর *ধ্রুবক *)margv,
		    pid, uid, stopgroup, test, progress, false);

		যদি (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		যদি (test || oknodo)
			ফেরত i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		যদি (pidfile)
			unlink(pidfile);
		যদি (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ধ্রুবক অক্ষর *ধ্রুবক *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	যদি (pidfile)
		pid = get_pid(applet, pidfile);
	অন্যথা
		pid = 0;

	যদি (pid)
		pids = rc_find_pids(ফাঁকা, ফাঁকা, 0, pid);
	অন্যথা
		pids = rc_find_pids(exec, (ধ্রুবক অক্ষর * ধ্রুবক *) argv, uid, 0);
	যদি (pids)
		eerrorx("%s: %s is already running", applet, exec);

	মুক্ত(pids);
	যদি (test) {
		যদি (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		যখন (argc-- > 0)
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
		পূর্ণসংখ্যা group_গুনতি = ARRAY_আকার(group_buf);
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
		যদি (changeuser != ফাঁকা) {
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
			group_list = xmalloc(group_গুনতি * আকার(*group_list));
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
		যদি (cap_iab != ফাঁকা) {
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
		যখন (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		যদি (changeuser != ফাঁকা) {
			pamenv = (ধ্রুবক অক্ষর *ধ্রুবক *)pam_getenvlist(pamh);
			যদি (pamenv) {
				যখন (*pamenv) {
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
			যখন (token && *token) {
				p = strchr(token, ':');
				যদি (p) {
					*p++ = '\0';
					যখন (*p == ':')
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
				পূর্ণসংখ্যা flags = fcntl(notify.fd, F_GETFD, 0);
				যদি (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				যদি (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		যদি (scheduler != ফাঁকা) {
			পূর্ণসংখ্যা scheduler_সূচক;
			কাঠামো sched_param sched =  {.sched_priority = sched_prio};
			যদি (strcmp(scheduler, "fifo") == 0)
				scheduler_সূচক = SCHED_FIFO;
			অন্যথা যদি (strcmp(scheduler, "rr") == 0)
				scheduler_সূচক = SCHED_RR;
			অন্যথা যদি (strcmp(scheduler, "other") == 0)
				scheduler_সূচক = SCHED_OTHER;
#ifdef SCHED_BATCH
			অন্যথা যদি (strcmp(scheduler, "batch") == 0)
				scheduler_সূচক = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			অন্যথা যদি (strcmp(scheduler, "idle") == 0)
				scheduler_সূচক = SCHED_IDLE;
#endif
			অন্যথা যদি (sscanf(scheduler, "%d", &scheduler_সূচক) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			যদি (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_সূচক);

			যদি (pthread_setschedparam(pthread_self(), scheduler_সূচক, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} অন্যথা যদি (sched_prio != -1) {
			ধ্রুবক কাঠামো sched_param sched =  {.sched_priority = sched_prio};
			যদি (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		যদি (changeuser != ফাঁকা && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	যখন ((ss = পঠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	যদি (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	যদি (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		করো {
			pid = waitpid(spid, &i, 0);
			যদি (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ফেরত -1;
			}
		} যখন (!WIFEXITED(i) && !WIFSIGNALED(i));
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
		কাঠামো timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		যদি (nanosleep(&ts, ফাঁকা) == -1) {
			যদি (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ফেরত 0;
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
			যদি (do_stop(applet, exec, (ধ্রুবক অক্ষর *ধ্রুবক *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		যদি (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	যদি (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ধ্রুবক অক্ষর *ধ্রুবক *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
