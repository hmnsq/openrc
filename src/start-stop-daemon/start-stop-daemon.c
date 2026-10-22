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

#اگر defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#شامل <errno.h>
#شامل <fcntl.h>
#شامل <getopt.h>
#شامل <limits.h>
#شامل <inttypes.h>
#شامل <grp.h>
#شامل <pthread.h>
#شامل <pwd.h>
#شامل <sched.h>
#شامل <signal.h>
#شامل <stdbool.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <sys/ioctl.h>
#شامل <sys/resource.h>
#شامل <sys/stat.h>
#شامل <sys/time.h>
#شامل <sys/types.h>
#شامل <inttypes.h>
#شامل <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#شامل <termios.h>
#شامل <time.h>
#شامل <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ایستا ساختار pam_conv conv = { پوچ, پوچ};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "rc_exec.h"
#شامل "schedules.h"
#شامل "_usage.h"
#شامل "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
شمارش {
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

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "capabilities", 1, پوچ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, پوچ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, پوچ, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, پوچ, 'I'},
	{ "stop",         0, پوچ, 'K'},
	{ "nicelevel",    1, پوچ, 'N'},
	{ "oom-score-adj",1, پوچ, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, پوچ, 'R'},
	{ "start",        0, پوچ, 'S'},
	{ "startas",      1, پوچ, 'a'},
	{ "background",   0, پوچ, 'b'},
	{ "chuid",        1, پوچ, 'c'},
	{ "chdir",        1, پوچ, 'd'},
	{ "env",          1, پوچ, 'e'},
	{ "umask",        1, پوچ, 'k'},
	{ "group",        1, پوچ, 'g'},
	{ "stop-group",   0, پوچ, 'G'},
	{ "interpreted",  0, پوچ, 'i'},
	{ "make-pidfile", 0, پوچ, 'm'},
	{ "name",         1, پوچ, 'n'},
	{ "oknodo",       0, پوچ, 'o'},
	{ "pidfile",      1, پوچ, 'p'},
	{ "signal",       1, پوچ, 's'},
	{ "test",         0, پوچ, 't'},
	{ "user",         1, پوچ, 'u'},
	{ "chroot",       1, پوچ, 'r'},
	{ "wait",         1, پوچ, 'w'},
	{ "exec",         1, پوچ, 'x'},
	{ "stdin",        1, پوچ, '0'},
	{ "stdout",       1, پوچ, '1'},
	{ "stderr",       1, پوچ, '2'},
	{ "stdout-logger",1, پوچ, '3'},
	{ "stderr-logger",1, پوچ, '4'},
	{ "progress",     0, پوچ, 'P'},
	{ "scheduler",    1, پوچ, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, پوچ, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, پوچ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = پوچ;

ایستا نویسه **nav;

ایستا نویسه *changeuser, *ch_ریشه, *ch_dir;

بیرونی نویسه **environ;

#اگر !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#اگر !defined(__DragonFly__)
ایستا درونخطی صحیح ioprio_set(صحیح which RC_UNUSED,
			     صحیح who RC_UNUSED,
			     صحیح ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	بازگشت syscall(SYS_ioprio_set, which, who, ioprio);
#وگرنه
	بازگشت 0;
#endif
}
#endif

ایستا تهی
cleanup(تهی)
{
	آزاد(changeuser);
	آزاد(nav);
	free_schedulelist();
}

ایستا تهی
handle_signal(صحیح sig)
{
	صحیح status;
	صحیح serrno = errno;
	ثابت نویسه *signame = پوچ;

	انتخاب (sig) {
	حالت SIGINT:
		اگر (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالت SIGTERM:
		اگر (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالت SIGQUIT:
		اگر (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	حالت SIGCHLD:
		برای (;;) {
			اگر (waitpid(-1, &status, WNOHANG) < 0) {
				اگر (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				بشکن;
			}
		}
		بشکن;

	پیشفرض:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ایستا نویسه *
expand_home(ثابت نویسه *home, ثابت نویسه *path)
{
	نویسه *opath, *ppath, *p, *nh;
	ساختار passwd *pw;

	اگر (!path || *path != '~')
		بازگشت xstrdup(path);

	opath = ppath = xstrdup(path);
	اگر (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		اگر (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		اگر (pw) {
			home = pw->pw_dir;
			ppath = p;
			اگر (ppath)
				*ppath = '/';
		} وگرنه
			home = پوچ;
	} وگرنه
		ppath++;

	اگر (!home) {
	آزاد(opath);
		بازگشت xstrdup(path);
	}
	اگر (!ppath) {
		آزاد(opath);
		بازگشت xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	آزاد(opath);
	بازگشت nh;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح devnull_fd = -1;
#ifdef TIOCNOTTY
	صحیح tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = پوچ;
	صحیح pamr;
	ثابت نویسه *ثابت *pamenv = پوچ;
#endif

	صحیح opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	نویسه *exec = پوچ;
	نویسه *startas = پوچ;
	نویسه *name = پوچ;
	نویسه *pidfile = پوچ;
	نویسه *retry = پوچ;
	صحیح sig = -1;
	صحیح nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	صحیح oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	نویسه *home = پوچ;
	صحیح tid = 0;
	نویسه *redirect_stdin = پوچ;
	نویسه *redirect_stderr = پوچ;
	نویسه *redirect_stdout = پوچ;
	نویسه *stderr_process = پوچ;
	نویسه *stdout_process = پوچ;
	صحیح stdin_fd;
	صحیح stdout_fd;
	صحیح stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	صحیح i;
	نویسه *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	نویسه *tmp, *newpath, *np;
	نویسه *p;
	نویسه *token;
	نویسه *exec_file = پوچ;
	ساختار passwd *pw;
	ساختار group *gr;
	bool stopgroup = false;
	نویسه *سطر = پوچ;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	نویسه **margv;
	unsigned صحیح start_wait = 0;
	ثابت نویسه *scheduler = پوچ;
	صحیح sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = پوچ;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	صحیح pipefd[2];
	نویسه readbuf[1];
	ssize_t ss;
	ساختار notify notify = {0};
	صحیح ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	اگر ((tmp = getenv("SSD_NICELEVEL")))
		اگر (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	اگر ((tmp = getenv("SSD_IONICELEVEL"))) {
		صحیح n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		اگر (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		اگر (ionicec == 0)
			ioniced = 0;
		وگرنه اگر (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	اگر ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		اگر (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	اگر (home == پوچ || p == پوچ) {
		pw = getpwuid(getuid());
		اگر (pw != پوچ) {
			اگر (p == پوچ)
				setenv("USER", pw->pw_name, 1);
			اگر (home == پوچ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	وقتی ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحیح *) 0)) != -1)
		انتخاب (opt) {
		حالت LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			اگر (cap_iab == پوچ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#وگرنه
			eerrorx("Capabilities support not enabled");
#endif
			بشکن;

		حالت LONGOPT_SECBITS:
#ifdef __linux__
			اگر (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = پوچ;
			secbits = strtoul(optarg, &tmp, 0);
			اگر (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#وگرنه
			eerrorx("Capabilities support not enabled");
#endif
			بشکن;

		حالت LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#وگرنه
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			بشکن;

		حالت 'I': /* --ionice */
			اگر (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			اگر (ionicec == 0)
				ioniced = 0;
			وگرنه اگر (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			بشکن;

		حالت 'K':  /* --stop */
			stop = true;
			بشکن;

		حالت 'G':  /* --stop-group */
			stopgroup = true;
			بشکن;

		حالت 'N':  /* --nice */
			اگر (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			بشکن;

		حالت LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			اگر (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			بشکن;

		حالت 'P':  /* --progress */
			progress = true;
			بشکن;

		حالت 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			بشکن;

		حالت 'S':  /* --start */
			start = true;
			بشکن;

		حالت 'b':  /* --background */
			background = true;
			بشکن;

		حالت 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		حالت 'u':  /* --user <username>|<uid> */
		{
			نویسه dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			اگر (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			وگرنه
				pw = getpwuid((uid_t)tid);

			اگر (pw == پوچ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			اگر (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			اگر (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			اگر (gid == 0)
				gid = pw->pw_gid;

			اگر (p) {
				tmp = strsep (&p, ":");
				اگر (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				وگرنه
					gr = getgrgid((gid_t) tid);

				اگر (gr == پوچ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		بشکن;

		حالت 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			بشکن;

		حالت 'e': /* --env */
			putenv(optarg);
			بشکن;

		حالت 'g':  /* --group <group>|<gid> */
			اگر (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			وگرنه
				gr = getgrgid((gid_t)tid);
			اگر (gr == پوچ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			بشکن;

		حالت 'i': /* --interpreted */
			interpreted = true;
			بشکن;

		حالت 'k':
			اگر (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			بشکن;

		حالت 'm':  /* --make-pidfile */
			makepidfile = true;
			بشکن;

		حالت 'n':  /* --name <process-name> */
			name = optarg;
			بشکن;

		حالت 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			بشکن;

		حالت 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			بشکن;

		حالت 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			بشکن;

		حالت 't':  /* --test */
			test = true;
			بشکن;

		حالت 'r':  /* --chroot /new/root */
			ch_ریشه = optarg;
			بشکن;

		حالت 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			بشکن;
		حالت 'w':
			اگر (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			بشکن;
		حالت 'x':  /* --exec <executable> */
			exec = optarg;
			بشکن;

		حالت '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			بشکن;

		حالت '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			بشکن;

		حالت '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			بشکن;

		حالت '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			بشکن;

		حالت '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			بشکن;

		حالت LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			بشکن;

		حالت LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			بشکن;

		حالت LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			بشکن;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	اگر (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	اگر (!exec)
		exec = startas;
	وگرنه اگر (!name)
		name = startas;

	اگر (!exec) {
		exec = *argv;
		اگر (!exec)
			exec = name;
		اگر (name && start)
			*argv = name;
	} وگرنه اگر (name) {
		*--argv = name;
		++argc;
	} وگرنه اگر (exec) {
		*--argv = exec;
		++argc;
	};

	اگر (stop || sig != -1) {
		اگر (sig == -1)
			sig = SIGTERM;
		اگر (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		اگر (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		اگر (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		اگر (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		اگر (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		اگر (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} وگرنه {
		اگر (!exec)
			eerrorx("%s: nothing to start", applet);
		اگر (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		اگر ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		اگر ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		اگر (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		اگر (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	اگر (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	اگر (ch_ریشه && *ch_ریشه == '~')
		ch_ریشه = expand_home(home, ch_ریشه);
	اگر (exec) {
		اگر (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		اگر (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			اگر (ch_ریشه)
				xasprintf(&exec_file, "%s/%s", ch_ریشه, exec);
			وگرنه
				xasprintf(&exec_file, "%s", exec);
		} وگرنه {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = پوچ;
			وقتی ((token = strsep(&p, ":"))) {
				اگر (ch_ریشه)
					xasprintf(&exec_file, "%s/%s/%s", ch_ریشه, token, exec);
				وگرنه
					xasprintf(&exec_file, "%s/%s", token, exec);
				اگر (exec_file && access(exec_file, F_OK) == 0)
					بشکن;
				آزاد(exec_file);
				exec_file = پوچ;
			}
			آزاد(tmp);
		}
	}
	اگر (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		آزاد(exec_file);
		exit(EXIT_FAILURE);
	}
	اگر (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	اگر (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		اگر (fp) {
			سطر = پوچ;
			اگر (xgetline(&سطر, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = سطر;
			fclose(fp);
			اگر (p != پوچ && سطر[0] == '#' && سطر[1] == '!') {
				p = سطر + 2;
				/* Strip leading spaces */
				وقتی (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				آزاد(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				برای (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(اندازه(نویسه *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				اگر (p)
					nav[len++] = p;
				برای (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = پوچ;
			}
		}
	}
	margv = nav ? nav : argv;

	اگر (stop || sig != -1) {
		اگر (sig == -1)
			sig = SIGTERM;
		اگر (!stop)
			oknodo = true;
		اگر (retry)
			parse_schedule(applet, retry, sig);
		وگرنه اگر (test || oknodo)
			parse_schedule(applet, "0", sig);
		وگرنه
			parse_schedule(applet, پوچ, sig);
		اگر (pidfile) {
			pid = get_pid(applet, pidfile);
			اگر (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} وگرنه {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ثابت نویسه *ثابت *)margv,
		    pid, uid, stopgroup, test, progress, false);

		اگر (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		اگر (test || oknodo)
			بازگشت i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		اگر (pidfile)
			unlink(pidfile);
		اگر (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ثابت نویسه *ثابت *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	اگر (pidfile)
		pid = get_pid(applet, pidfile);
	وگرنه
		pid = 0;

	اگر (pid)
		pids = rc_find_pids(پوچ, پوچ, 0, pid);
	وگرنه
		pids = rc_find_pids(exec, (ثابت نویسه * ثابت *) argv, uid, 0);
	اگر (pids)
		eerrorx("%s: %s is already running", applet, exec);

	آزاد(pids);
	اگر (test) {
		اگر (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		وقتی (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		اگر (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		اگر (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		اگر (ch_ریشه)
			einfo("in root `%s'", ch_ریشه);
		اگر (ch_dir)
			einfo("in dir `%s'", ch_dir);
		اگر (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		اگر (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	اگر (pidfile)
		unlink(pidfile);

	اگر (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	اگر (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	اگر ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	اگر (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		صحیح group_شمار = ARRAY_SIZE(group_buf);
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

		اگر (nicelevel != INT_MIN) {
			اگر (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			اگر (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} وگرنه اگر (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		اگر (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		اگر (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			اگر (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		اگر (changeuser != پوچ) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			اگر (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			اگر (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			اگر (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		اگر (changeuser && getgrouplist(changeuser, gid, group_list, &group_شمار) < 0) {
			group_list = xmalloc(group_شمار * اندازه(*group_list));
			اگر (getgrouplist(changeuser, gid, group_list, &group_شمار) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		اگر (ch_ریشه && chroot(ch_ریشه) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_ریشه, strerror(errno));

		اگر (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		اگر (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			اگر (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		اگر (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		اگر (changeuser && setgroups(group_شمار, group_list))
			eerrorx("%s: setgroups() failed", applet);
		اگر (group_list != group_buf)
			آزاد(group_list);
#ifdef __linux__
		اگر (uid && cap_setuid(uid))
#وگرنه
		اگر (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		اگر (cap_iab != پوچ) {
			i = cap_iab_set_proc(cap_iab);

			اگر (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			اگر (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		اگر (secbits != 0) {
			اگر (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		اگر (no_new_privs) {
			اگر (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		وقتی (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		اگر (changeuser != پوچ) {
			pamenv = (ثابت نویسه *ثابت *)pam_getenvlist(pamh);
			اگر (pamenv) {
				وقتی (*pamenv) {
					/* Don't add strings unless they set a var */
					اگر (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					وگرنه
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			اگر (strncmp(env->مقدار, "RC_", 3) == 0
					|| strncmp(env->مقدار, "SSD_", 4) == 0
					|| strncmp(env->مقدار, "EINFO_", 6) == 0) {
				*strchr(env->مقدار, '=') = '\0';
				unsetenv(env->مقدار);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		اگر ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			وقتی (token && *token) {
				p = strchr(token, ':');
				اگر (p) {
					*p++ = '\0';
					وقتی (*p == ':')
						p++;
				}
				اگر (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					اگر (np != newpath)
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
		اگر (redirect_stdin) {
			اگر ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		اگر (redirect_stdout) {
			اگر ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}وگرنه اگر (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			اگر (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		اگر (redirect_stderr) {
			اگر ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}وگرنه اگر (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			اگر (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		اگر (background)
			dup2(stdin_fd, STDIN_FILENO);
		اگر (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		اگر (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		اگر (notify.type == NOTIFY_FD) {
			اگر (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			اگر (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			اگر (notify.pipe[1] == notify.fd) {
				صحیح flags = fcntl(notify.fd, F_GETFD, 0);
				اگر (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				اگر (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		اگر (scheduler != پوچ) {
			صحیح scheduler_index;
			ساختار sched_param sched =  {.sched_priority = sched_prio};
			اگر (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			وگرنه اگر (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			وگرنه اگر (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			وگرنه اگر (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			وگرنه اگر (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			وگرنه اگر (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			اگر (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			اگر (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} وگرنه اگر (sched_prio != -1) {
			ثابت ساختار sched_param sched =  {.sched_priority = sched_prio};
			اگر (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		اگر (changeuser != پوچ && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	وقتی ((ss = اقرأ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	اگر (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	اگر (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		انجامبده {
			pid = waitpid(spid, &i, 0);
			اگر (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				بازگشت -1;
			}
		} وقتی (!WIFEXITED(i) && !WIFSIGNALED(i));
		اگر (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	اگر (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_مقدار("rc_start_wait"))))
	{
		اگر (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	اگر (notify.type != NOTIFY_NONE) {
		اگر (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} وگرنه اگر (start_wait > 0) {
		ساختار timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		اگر (nanosleep(&ts, پوچ) == -1) {
			اگر (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				بازگشت 0;
			}
		}
		اگر (background) {
			اگر (kill(pid, 0) == 0)
				alive = true;
		} وگرنه {
			اگر (pidfile) {
				pid = get_pid(applet, pidfile);
				اگر (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} وگرنه
				pid = 0;
			اگر (do_stop(applet, exec, (ثابت نویسه *ثابت *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		اگر (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	اگر (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ثابت نویسه *ثابت *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
