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

#𐤀𐤌 defined(__linux__) && !defined(_GNU_SOURCE)
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

#𐤀𐤌 !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#𐤀𐤌 !defined(__DragonFly__)
ساكن مضمن صحيح ioprio_set(صحيح which RC_UNUSED,
			     صحيح who RC_UNUSED,
			     صحيح ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	أرجع syscall(SYS_ioprio_set, which, who, ioprio);
#وإلا
	أرجع 0;
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
		𐤀𐤌 (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	حالة SIGTERM:
		𐤀𐤌 (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	حالة SIGQUIT:
		𐤀𐤌 (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	حالة SIGCHLD:
		لكل (;;) {
			𐤀𐤌 (waitpid(-1, &status, WNOHANG) < 0) {
				𐤀𐤌 (errno != ECHILD)
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

	𐤀𐤌 (!path || *path != '~')
		أرجع xstrdup(path);

	opath = ppath = xstrdup(path);
	𐤀𐤌 (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		𐤀𐤌 (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		𐤀𐤌 (pw) {
			home = pw->pw_dir;
			ppath = p;
			𐤀𐤌 (ppath)
				*ppath = '/';
		} وإلا
			home = عدم;
	} وإلا
		ppath++;

	𐤀𐤌 (!home) {
	حرر(opath);
		أرجع xstrdup(path);
	}
	𐤀𐤌 (!ppath) {
		حرر(opath);
		أرجع xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	حرر(opath);
	أرجع nh;
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

	𐤀𐤌 ((tmp = getenv("SSD_NICELEVEL")))
		𐤀𐤌 (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	𐤀𐤌 ((tmp = getenv("SSD_IONICELEVEL"))) {
		صحيح n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		𐤀𐤌 (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		𐤀𐤌 (ionicec == 0)
			ioniced = 0;
		وإلا 𐤀𐤌 (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	𐤀𐤌 ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		𐤀𐤌 (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	𐤀𐤌 (home == عدم || p == عدم) {
		pw = getpwuid(getuid());
		𐤀𐤌 (pw != عدم) {
			𐤀𐤌 (p == عدم)
				setenv("USER", pw->pw_name, 1);
			𐤀𐤌 (home == عدم) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			𐤀𐤌 (cap_iab == عدم)
				eerrorx("Could not parse iab: %s", strerror(errno));
#وإلا
			eerrorx("Capabilities support not enabled");
#endif
			اكسر;

		حالة LONGOPT_SECBITS:
#ifdef __linux__
			𐤀𐤌 (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = عدم;
			secbits = strtoul(optarg, &tmp, 0);
			𐤀𐤌 (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#وإلا
			eerrorx("Capabilities support not enabled");
#endif
			اكسر;

		حالة LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#وإلا
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			اكسر;

		حالة 'I': /* --ionice */
			𐤀𐤌 (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			𐤀𐤌 (ionicec == 0)
				ioniced = 0;
			وإلا 𐤀𐤌 (ionicec == 3)
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
			𐤀𐤌 (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			اكسر;

		حالة LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			𐤀𐤌 (sscanf(optarg, "%d", &oom_score_adj) != 1)
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
			𐤀𐤌 (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			وإلا
				pw = getpwuid((uid_t)tid);

			𐤀𐤌 (pw == عدم)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			𐤀𐤌 (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			𐤀𐤌 (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			𐤀𐤌 (gid == 0)
				gid = pw->pw_gid;

			𐤀𐤌 (p) {
				tmp = strsep (&p, ":");
				𐤀𐤌 (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				وإلا
					gr = getgrgid((gid_t) tid);

				𐤀𐤌 (gr == عدم)
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
			𐤀𐤌 (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			وإلا
				gr = getgrgid((gid_t)tid);
			𐤀𐤌 (gr == عدم)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			اكسر;

		حالة 'i': /* --interpreted */
			interpreted = true;
			اكسر;

		حالة 'k':
			𐤀𐤌 (parse_mode(&numask, optarg))
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
			𐤀𐤌 (sscanf(optarg, "%u", &start_wait) != 1)
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
	𐤀𐤌 (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	𐤀𐤌 (!exec)
		exec = startas;
	وإلا 𐤀𐤌 (!name)
		name = startas;

	𐤀𐤌 (!exec) {
		exec = *argv;
		𐤀𐤌 (!exec)
			exec = name;
		𐤀𐤌 (name && start)
			*argv = name;
	} وإلا 𐤀𐤌 (name) {
		*--argv = name;
		++argc;
	} وإلا 𐤀𐤌 (exec) {
		*--argv = exec;
		++argc;
	};

	𐤀𐤌 (stop || sig != -1) {
		𐤀𐤌 (sig == -1)
			sig = SIGTERM;
		𐤀𐤌 (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		𐤀𐤌 (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		𐤀𐤌 (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		𐤀𐤌 (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		𐤀𐤌 (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		𐤀𐤌 (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} وإلا {
		𐤀𐤌 (!exec)
			eerrorx("%s: nothing to start", applet);
		𐤀𐤌 (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		𐤀𐤌 ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		𐤀𐤌 ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		𐤀𐤌 (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		𐤀𐤌 (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	𐤀𐤌 (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	𐤀𐤌 (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	𐤀𐤌 (exec) {
		𐤀𐤌 (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		𐤀𐤌 (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			𐤀𐤌 (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			وإلا
				xasprintf(&exec_file, "%s", exec);
		} وإلا {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = عدم;
			طالما ((token = strsep(&p, ":"))) {
				𐤀𐤌 (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				وإلا
					xasprintf(&exec_file, "%s/%s", token, exec);
				𐤀𐤌 (exec_file && access(exec_file, F_OK) == 0)
					اكسر;
				حرر(exec_file);
				exec_file = عدم;
			}
			حرر(tmp);
		}
	}
	𐤀𐤌 (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		حرر(exec_file);
		exit(EXIT_FAILURE);
	}
	𐤀𐤌 (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	𐤀𐤌 (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		𐤀𐤌 (fp) {
			line = عدم;
			𐤀𐤌 (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			𐤀𐤌 (p != عدم && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				طالما (*p == ' ' || *p == '\t')
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
				𐤀𐤌 (p)
					nav[len++] = p;
				لكل (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = عدم;
			}
		}
	}
	margv = nav ? nav : argv;

	𐤀𐤌 (stop || sig != -1) {
		𐤀𐤌 (sig == -1)
			sig = SIGTERM;
		𐤀𐤌 (!stop)
			oknodo = true;
		𐤀𐤌 (retry)
			parse_schedule(applet, retry, sig);
		وإلا 𐤀𐤌 (test || oknodo)
			parse_schedule(applet, "0", sig);
		وإلا
			parse_schedule(applet, عدم, sig);
		𐤀𐤌 (pidfile) {
			pid = get_pid(applet, pidfile);
			𐤀𐤌 (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} وإلا {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ثابت حرف *ثابت *)margv,
		    pid, uid, stopgroup, test, progress, false);

		𐤀𐤌 (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		𐤀𐤌 (test || oknodo)
			أرجع i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		𐤀𐤌 (pidfile)
			unlink(pidfile);
		𐤀𐤌 (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ثابت حرف *ثابت *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	𐤀𐤌 (pidfile)
		pid = get_pid(applet, pidfile);
	وإلا
		pid = 0;

	𐤀𐤌 (pid)
		pids = rc_find_pids(عدم, عدم, 0, pid);
	وإلا
		pids = rc_find_pids(exec, (ثابت حرف * ثابت *) argv, uid, 0);
	𐤀𐤌 (pids)
		eerrorx("%s: %s is already running", applet, exec);

	حرر(pids);
	𐤀𐤌 (test) {
		𐤀𐤌 (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		طالما (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		𐤀𐤌 (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		𐤀𐤌 (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		𐤀𐤌 (ch_root)
			einfo("in root `%s'", ch_root);
		𐤀𐤌 (ch_dir)
			einfo("in dir `%s'", ch_dir);
		𐤀𐤌 (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		𐤀𐤌 (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	𐤀𐤌 (pidfile)
		unlink(pidfile);

	𐤀𐤌 (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	𐤀𐤌 (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	𐤀𐤌 ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	𐤀𐤌 (pid == 0) {
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

		𐤀𐤌 (nicelevel != INT_MIN) {
			𐤀𐤌 (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			𐤀𐤌 (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} وإلا 𐤀𐤌 (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		𐤀𐤌 (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		𐤀𐤌 (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			𐤀𐤌 (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		𐤀𐤌 (changeuser != عدم) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			𐤀𐤌 (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			𐤀𐤌 (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			𐤀𐤌 (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		𐤀𐤌 (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * حجم(*group_list));
			𐤀𐤌 (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		𐤀𐤌 (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		𐤀𐤌 (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		𐤀𐤌 (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			𐤀𐤌 (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		𐤀𐤌 (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		𐤀𐤌 (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		𐤀𐤌 (group_list != group_buf)
			حرر(group_list);
#ifdef __linux__
		𐤀𐤌 (uid && cap_setuid(uid))
#وإلا
		𐤀𐤌 (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		𐤀𐤌 (cap_iab != عدم) {
			i = cap_iab_set_proc(cap_iab);

			𐤀𐤌 (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			𐤀𐤌 (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		𐤀𐤌 (secbits != 0) {
			𐤀𐤌 (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		𐤀𐤌 (no_new_privs) {
			𐤀𐤌 (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		طالما (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		𐤀𐤌 (changeuser != عدم) {
			pamenv = (ثابت حرف *ثابت *)pam_getenvlist(pamh);
			𐤀𐤌 (pamenv) {
				طالما (*pamenv) {
					/* Don't add strings unless they set a var */
					𐤀𐤌 (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					وإلا
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			𐤀𐤌 (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		𐤀𐤌 ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			طالما (token && *token) {
				p = strchr(token, ':');
				𐤀𐤌 (p) {
					*p++ = '\0';
					طالما (*p == ':')
						p++;
				}
				𐤀𐤌 (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					𐤀𐤌 (np != newpath)
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
		𐤀𐤌 (redirect_stdin) {
			𐤀𐤌 ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		𐤀𐤌 (redirect_stdout) {
			𐤀𐤌 ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}وإلا 𐤀𐤌 (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			𐤀𐤌 (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		𐤀𐤌 (redirect_stderr) {
			𐤀𐤌 ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}وإلا 𐤀𐤌 (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			𐤀𐤌 (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		𐤀𐤌 (background)
			dup2(stdin_fd, STDIN_FILENO);
		𐤀𐤌 (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		𐤀𐤌 (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		𐤀𐤌 (notify.type == NOTIFY_FD) {
			𐤀𐤌 (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			𐤀𐤌 (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			𐤀𐤌 (notify.pipe[1] == notify.fd) {
				صحيح flags = fcntl(notify.fd, F_GETFD, 0);
				𐤀𐤌 (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				𐤀𐤌 (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		𐤀𐤌 (scheduler != عدم) {
			صحيح scheduler_index;
			بنية sched_param sched =  {.sched_priority = sched_prio};
			𐤀𐤌 (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			وإلا 𐤀𐤌 (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			وإلا 𐤀𐤌 (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			وإلا 𐤀𐤌 (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			وإلا 𐤀𐤌 (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			وإلا 𐤀𐤌 (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			𐤀𐤌 (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			𐤀𐤌 (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} وإلا 𐤀𐤌 (sched_prio != -1) {
			ثابت بنية sched_param sched =  {.sched_priority = sched_prio};
			𐤀𐤌 (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		𐤀𐤌 (changeuser != عدم && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	طالما ((ss = اقرأ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	𐤀𐤌 (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	𐤀𐤌 (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		𐤐𐤏𐤋 {
			pid = waitpid(spid, &i, 0);
			𐤀𐤌 (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				أرجع -1;
			}
		} طالما (!WIFEXITED(i) && !WIFSIGNALED(i));
		𐤀𐤌 (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	𐤀𐤌 (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		𐤀𐤌 (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	𐤀𐤌 (notify.type != NOTIFY_NONE) {
		𐤀𐤌 (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} وإلا 𐤀𐤌 (start_wait > 0) {
		بنية timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		𐤀𐤌 (nanosleep(&ts, عدم) == -1) {
			𐤀𐤌 (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				أرجع 0;
			}
		}
		𐤀𐤌 (background) {
			𐤀𐤌 (kill(pid, 0) == 0)
				alive = true;
		} وإلا {
			𐤀𐤌 (pidfile) {
				pid = get_pid(applet, pidfile);
				𐤀𐤌 (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} وإلا
				pid = 0;
			𐤀𐤌 (do_stop(applet, exec, (ثابت حرف *ثابت *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		𐤀𐤌 (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	𐤀𐤌 (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ثابت حرف *ثابت *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
