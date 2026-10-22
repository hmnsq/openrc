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

#ܐܢ defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>
#ضمن <limits.h>
#ضمن <inttypes.h>
#ضمن <grp.h>
#ضمن <pthread.h>
#ضمن <pwd.h>
#ضمن <sched.h>
#ضمن <signal.h>
#ضمن <stdbool.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <syslog.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/resource.h>
#ضمن <sys/stat.h>
#ضمن <sys/time.h>
#ضمن <sys/types.h>
#ضمن <inttypes.h>
#ضمن <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#ضمن <termios.h>
#ضمن <time.h>
#ضمن <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ساكن بنية pam_conv conv = { عدم, عدم};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "rc_exec.h"
#ضمن "schedules.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
تعداد {
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

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "capabilities", 1, عدم, LONGOPT_CAPABILITIES},
	{ "secbits",      1, عدم, LONGOPT_SECBITS},
	{ "no-new-privs", 0, عدم, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, عدم, 'I'},
	{ "stop",         0, عدم, 'K'},
	{ "nicelevel",    1, عدم, 'N'},
	{ "oom-score-adj",1, عدم, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, عدم, 'R'},
	{ "start",        0, عدم, 'S'},
	{ "startas",      1, عدم, 'a'},
	{ "background",   0, عدم, 'b'},
	{ "chuid",        1, عدم, 'c'},
	{ "chdir",        1, عدم, 'd'},
	{ "env",          1, عدم, 'e'},
	{ "umask",        1, عدم, 'k'},
	{ "group",        1, عدم, 'g'},
	{ "stop-group",   0, عدم, 'G'},
	{ "interpreted",  0, عدم, 'i'},
	{ "make-pidfile", 0, عدم, 'm'},
	{ "name",         1, عدم, 'n'},
	{ "oknodo",       0, عدم, 'o'},
	{ "pidfile",      1, عدم, 'p'},
	{ "signal",       1, عدم, 's'},
	{ "test",         0, عدم, 't'},
	{ "user",         1, عدم, 'u'},
	{ "chroot",       1, عدم, 'r'},
	{ "wait",         1, عدم, 'w'},
	{ "exec",         1, عدم, 'x'},
	{ "stdin",        1, عدم, '0'},
	{ "stdout",       1, عدم, '1'},
	{ "stderr",       1, عدم, '2'},
	{ "stdout-logger",1, عدم, '3'},
	{ "stderr-logger",1, عدم, '4'},
	{ "progress",     0, عدم, 'P'},
	{ "scheduler",    1, عدم, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, عدم, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, عدم, LONGOPT_NOTIFY},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = عدم;

ساكن حرف **nav;

ساكن حرف *changeuser, *ch_root, *ch_dir;

خارجي حرف **environ;

#ܐܢ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ܐܢ !defined(__DragonFly__)
ساكن مضمن صحيح ioprio_set(صحيح which RC_UNUSED,
			     صحيح who RC_UNUSED,
			     صحيح ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ܦܢܐ syscall(SYS_ioprio_set, which, who, ioprio);
#ܐܚܪܢܐ
	ܦܢܐ 0;
#endif
}
#endif

ساكن فراغ
cleanup(فراغ)
{
	حرر(changeuser);
	حرر(nav);
	free_schedulelist();
}

ساكن فراغ
handle_signal(صحيح sig)
{
	صحيح status;
	صحيح serrno = errno;
	ثابت حرف *signame = عدم;

	بدل (sig) {
	حالة SIGINT:
		ܐܢ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالة SIGTERM:
		ܐܢ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالة SIGQUIT:
		ܐܢ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	حالة SIGCHLD:
		لكل (;;) {
			ܐܢ (waitpid(-1, &status, WNOHANG) < 0) {
				ܐܢ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				اكسر;
			}
		}
		اكسر;

	افتراضي:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ساكن حرف *
expand_home(ثابت حرف *home, ثابت حرف *path)
{
	حرف *opath, *ppath, *p, *nh;
	بنية passwd *pw;

	ܐܢ (!path || *path != '~')
		ܦܢܐ xstrdup(path);

	opath = ppath = xstrdup(path);
	ܐܢ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ܐܢ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ܐܢ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ܐܢ (ppath)
				*ppath = '/';
		} ܐܚܪܢܐ
			home = عدم;
	} ܐܚܪܢܐ
		ppath++;

	ܐܢ (!home) {
	حرر(opath);
		ܦܢܐ xstrdup(path);
	}
	ܐܢ (!ppath) {
		حرر(opath);
		ܦܢܐ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	حرر(opath);
	ܦܢܐ nh;
}

صحيح main(صحيح argc, حرف **argv)
{
	صحيح devnull_fd = -1;
#ifdef TIOCNOTTY
	صحيح tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = عدم;
	صحيح pamr;
	ثابت حرف *ثابت *pamenv = عدم;
#endif

	صحيح opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	حرف *exec = عدم;
	حرف *startas = عدم;
	حرف *name = عدم;
	حرف *pidfile = عدم;
	حرف *retry = عدم;
	صحيح sig = -1;
	صحيح nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	صحيح oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	حرف *home = عدم;
	صحيح tid = 0;
	حرف *redirect_stdin = عدم;
	حرف *redirect_stderr = عدم;
	حرف *redirect_stdout = عدم;
	حرف *stderr_process = عدم;
	حرف *stdout_process = عدم;
	صحيح stdin_fd;
	صحيح stdout_fd;
	صحيح stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	صحيح i;
	حرف *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	حرف *tmp, *newpath, *np;
	حرف *p;
	حرف *token;
	حرف *exec_file = عدم;
	بنية passwd *pw;
	بنية group *gr;
	bool stopgroup = false;
	حرف *line = عدم;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	حرف **margv;
	unsigned صحيح start_wait = 0;
	ثابت حرف *scheduler = عدم;
	صحيح sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = عدم;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	صحيح pipefd[2];
	حرف readbuf[1];
	ssize_t ss;
	بنية notify notify = {0};
	صحيح ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ܐܢ ((tmp = getenv("SSD_NICELEVEL")))
		ܐܢ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ܐܢ ((tmp = getenv("SSD_IONICELEVEL"))) {
		صحيح n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ܐܢ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ܐܢ (ionicec == 0)
			ioniced = 0;
		ܐܚܪܢܐ ܐܢ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ܐܢ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ܐܢ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ܐܢ (home == عدم || p == عدم) {
		pw = getpwuid(getuid());
		ܐܢ (pw != عدم) {
			ܐܢ (p == عدم)
				setenv("USER", pw->pw_name, 1);
			ܐܢ (home == عدم) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ܟܡܐ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ܐܢ (cap_iab == عدم)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ܐܚܪܢܐ
			eerrorx("Capabilities support not enabled");
#endif
			اكسر;

		حالة LONGOPT_SECBITS:
#ifdef __linux__
			ܐܢ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = عدم;
			secbits = strtoul(optarg, &tmp, 0);
			ܐܢ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ܐܚܪܢܐ
			eerrorx("Capabilities support not enabled");
#endif
			اكسر;

		حالة LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ܐܚܪܢܐ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			اكسر;

		حالة 'I': /* --ionice */
			ܐܢ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ܐܢ (ionicec == 0)
				ioniced = 0;
			ܐܚܪܢܐ ܐܢ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			اكسر;

		حالة 'K':  /* --stop */
			stop = true;
			اكسر;

		حالة 'G':  /* --stop-group */
			stopgroup = true;
			اكسر;

		حالة 'N':  /* --nice */
			ܐܢ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			اكسر;

		حالة LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ܐܢ (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			اكسر;

		حالة 'P':  /* --progress */
			progress = true;
			اكسر;

		حالة 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			اكسر;

		حالة 'S':  /* --start */
			start = true;
			اكسر;

		حالة 'b':  /* --background */
			background = true;
			اكسر;

		حالة 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		حالة 'u':  /* --user <username>|<uid> */
		{
			حرف dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ܐܢ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ܐܚܪܢܐ
				pw = getpwuid((uid_t)tid);

			ܐܢ (pw == عدم)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ܐܢ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ܐܢ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ܐܢ (gid == 0)
				gid = pw->pw_gid;

			ܐܢ (p) {
				tmp = strsep (&p, ":");
				ܐܢ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ܐܚܪܢܐ
					gr = getgrgid((gid_t) tid);

				ܐܢ (gr == عدم)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		اكسر;

		حالة 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			اكسر;

		حالة 'e': /* --env */
			putenv(optarg);
			اكسر;

		حالة 'g':  /* --group <group>|<gid> */
			ܐܢ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ܐܚܪܢܐ
				gr = getgrgid((gid_t)tid);
			ܐܢ (gr == عدم)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			اكسر;

		حالة 'i': /* --interpreted */
			interpreted = true;
			اكسر;

		حالة 'k':
			ܐܢ (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			اكسر;

		حالة 'm':  /* --make-pidfile */
			makepidfile = true;
			اكسر;

		حالة 'n':  /* --name <process-name> */
			name = optarg;
			اكسر;

		حالة 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			اكسر;

		حالة 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			اكسر;

		حالة 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			اكسر;

		حالة 't':  /* --test */
			test = true;
			اكسر;

		حالة 'r':  /* --chroot /new/root */
			ch_root = optarg;
			اكسر;

		حالة 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			اكسر;
		حالة 'w':
			ܐܢ (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			اكسر;
		حالة 'x':  /* --exec <executable> */
			exec = optarg;
			اكسر;

		حالة '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			اكسر;

		حالة '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			اكسر;

		حالة '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			اكسر;

		حالة '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			اكسر;

		حالة '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			اكسر;

		حالة LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			اكسر;

		حالة LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			اكسر;

		حالة LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			اكسر;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	ܐܢ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ܐܢ (!exec)
		exec = startas;
	ܐܚܪܢܐ ܐܢ (!name)
		name = startas;

	ܐܢ (!exec) {
		exec = *argv;
		ܐܢ (!exec)
			exec = name;
		ܐܢ (name && start)
			*argv = name;
	} ܐܚܪܢܐ ܐܢ (name) {
		*--argv = name;
		++argc;
	} ܐܚܪܢܐ ܐܢ (exec) {
		*--argv = exec;
		++argc;
	};

	ܐܢ (stop || sig != -1) {
		ܐܢ (sig == -1)
			sig = SIGTERM;
		ܐܢ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ܐܢ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ܐܢ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ܐܢ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ܐܢ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ܐܢ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ܐܚܪܢܐ {
		ܐܢ (!exec)
			eerrorx("%s: nothing to start", applet);
		ܐܢ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ܐܢ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ܐܢ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ܐܢ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ܐܢ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ܐܢ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ܐܢ (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	ܐܢ (exec) {
		ܐܢ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ܐܢ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ܐܢ (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			ܐܚܪܢܐ
				xasprintf(&exec_file, "%s", exec);
		} ܐܚܪܢܐ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = عدم;
			ܟܡܐ ((token = strsep(&p, ":"))) {
				ܐܢ (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				ܐܚܪܢܐ
					xasprintf(&exec_file, "%s/%s", token, exec);
				ܐܢ (exec_file && access(exec_file, F_OK) == 0)
					اكسر;
				حرر(exec_file);
				exec_file = عدم;
			}
			حرر(tmp);
		}
	}
	ܐܢ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		حرر(exec_file);
		exit(EXIT_FAILURE);
	}
	ܐܢ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ܐܢ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ܐܢ (fp) {
			line = عدم;
			ܐܢ (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			ܐܢ (p != عدم && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				ܟܡܐ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				حرر(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				لكل (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(حجم(حرف *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ܐܢ (p)
					nav[len++] = p;
				لكل (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = عدم;
			}
		}
	}
	margv = nav ? nav : argv;

	ܐܢ (stop || sig != -1) {
		ܐܢ (sig == -1)
			sig = SIGTERM;
		ܐܢ (!stop)
			oknodo = true;
		ܐܢ (retry)
			parse_schedule(applet, retry, sig);
		ܐܚܪܢܐ ܐܢ (test || oknodo)
			parse_schedule(applet, "0", sig);
		ܐܚܪܢܐ
			parse_schedule(applet, عدم, sig);
		ܐܢ (pidfile) {
			pid = get_pid(applet, pidfile);
			ܐܢ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} ܐܚܪܢܐ {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ثابت حرف *ثابت *)margv,
		    pid, uid, stopgroup, test, progress, false);

		ܐܢ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		ܐܢ (test || oknodo)
			ܦܢܐ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ܐܢ (pidfile)
			unlink(pidfile);
		ܐܢ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ثابت حرف *ثابت *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	ܐܢ (pidfile)
		pid = get_pid(applet, pidfile);
	ܐܚܪܢܐ
		pid = 0;

	ܐܢ (pid)
		pids = rc_find_pids(عدم, عدم, 0, pid);
	ܐܚܪܢܐ
		pids = rc_find_pids(exec, (ثابت حرف * ثابت *) argv, uid, 0);
	ܐܢ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	حرر(pids);
	ܐܢ (test) {
		ܐܢ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		ܟܡܐ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		ܐܢ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ܐܢ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ܐܢ (ch_root)
			einfo("in root `%s'", ch_root);
		ܐܢ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ܐܢ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ܐܢ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ܐܢ (pidfile)
		unlink(pidfile);

	ܐܢ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ܐܢ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ܐܢ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ܐܢ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		صحيح group_count = ARRAY_SIZE(group_buf);
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

		ܐܢ (nicelevel != INT_MIN) {
			ܐܢ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ܐܢ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ܐܚܪܢܐ ܐܢ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ܐܢ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ܐܢ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ܐܢ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		ܐܢ (changeuser != عدم) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ܐܢ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ܐܢ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ܐܢ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		ܐܢ (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * حجم(*group_list));
			ܐܢ (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ܐܢ (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		ܐܢ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ܐܢ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ܐܢ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ܐܢ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ܐܢ (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		ܐܢ (group_list != group_buf)
			حرر(group_list);
#ifdef __linux__
		ܐܢ (uid && cap_setuid(uid))
#ܐܚܪܢܐ
		ܐܢ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		ܐܢ (cap_iab != عدم) {
			i = cap_iab_set_proc(cap_iab);

			ܐܢ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ܐܢ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ܐܢ (secbits != 0) {
			ܐܢ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		ܐܢ (no_new_privs) {
			ܐܢ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		ܟܡܐ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		ܐܢ (changeuser != عدم) {
			pamenv = (ثابت حرف *ثابت *)pam_getenvlist(pamh);
			ܐܢ (pamenv) {
				ܟܡܐ (*pamenv) {
					/* Don't add strings unless they set a var */
					ܐܢ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ܐܚܪܢܐ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ܐܢ (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ܐܢ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			ܟܡܐ (token && *token) {
				p = strchr(token, ':');
				ܐܢ (p) {
					*p++ = '\0';
					ܟܡܐ (*p == ':')
						p++;
				}
				ܐܢ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					ܐܢ (np != newpath)
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
		ܐܢ (redirect_stdin) {
			ܐܢ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ܐܢ (redirect_stdout) {
			ܐܢ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ܐܚܪܢܐ ܐܢ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ܐܢ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ܐܢ (redirect_stderr) {
			ܐܢ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ܐܚܪܢܐ ܐܢ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ܐܢ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ܐܢ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ܐܢ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ܐܢ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ܐܢ (notify.type == NOTIFY_FD) {
			ܐܢ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ܐܢ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ܐܢ (notify.pipe[1] == notify.fd) {
				صحيح flags = fcntl(notify.fd, F_GETFD, 0);
				ܐܢ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ܐܢ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ܐܢ (scheduler != عدم) {
			صحيح scheduler_index;
			بنية sched_param sched =  {.sched_priority = sched_prio};
			ܐܢ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			ܐܚܪܢܐ ܐܢ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			ܐܚܪܢܐ ܐܢ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			ܐܚܪܢܐ ܐܢ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			ܐܚܪܢܐ ܐܢ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			ܐܚܪܢܐ ܐܢ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ܐܢ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			ܐܢ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ܐܚܪܢܐ ܐܢ (sched_prio != -1) {
			ثابت بنية sched_param sched =  {.sched_priority = sched_prio};
			ܐܢ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		ܐܢ (changeuser != عدم && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ܟܡܐ ((ss = اقرأ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ܐܢ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ܐܢ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		ܥܒܕ {
			pid = waitpid(spid, &i, 0);
			ܐܢ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ܦܢܐ -1;
			}
		} ܟܡܐ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ܐܢ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ܐܢ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		ܐܢ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	ܐܢ (notify.type != NOTIFY_NONE) {
		ܐܢ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ܐܚܪܢܐ ܐܢ (start_wait > 0) {
		بنية timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		ܐܢ (nanosleep(&ts, عدم) == -1) {
			ܐܢ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ܦܢܐ 0;
			}
		}
		ܐܢ (background) {
			ܐܢ (kill(pid, 0) == 0)
				alive = true;
		} ܐܚܪܢܐ {
			ܐܢ (pidfile) {
				pid = get_pid(applet, pidfile);
				ܐܢ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ܐܚܪܢܐ
				pid = 0;
			ܐܢ (do_stop(applet, exec, (ثابت حرف *ثابت *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ܐܢ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ܐܢ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ثابت حرف *ثابت *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
