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

#تعریف_کرو ONE_MS           1000000

#اگر defined(__linux__) && !defined(GNU_ماخذ)
/* For extra SCHED_* defines. */
# define GNU_ماخذ
#اگر_ختم

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
#اگر_تعریف_شدہ __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#اگر_ختم
#شامل <termios.h>
#شامل <time.h>
#شامل <unistd.h>

#اگر_تعریف_شدہ HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ساکن ساخت pam_conv conv = { تہی, تہی};
#اگر_ختم

#اگر_تعریف_شدہ __linux__
# include <sys/capability.h>
#اگر_ختم

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
شمار {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_بنیاد = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SCHEDULER,
  LONGOPT_SCHEDULER_PRIO,
  LONGOPT_SECBITS,
  LONGOPT_NOTIFY,
};

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "capabilities", 1, تہی, LONGOPT_CAPABILITIES},
	{ "secbits",      1, تہی, LONGOPT_SECBITS},
	{ "no-new-privs", 0, تہی, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, تہی, 'I'},
	{ "stop",         0, تہی, 'K'},
	{ "nicelevel",    1, تہی, 'N'},
	{ "oom-score-adj",1, تہی, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, تہی, 'R'},
	{ "start",        0, تہی, 'S'},
	{ "startas",      1, تہی, 'a'},
	{ "background",   0, تہی, 'b'},
	{ "chuid",        1, تہی, 'c'},
	{ "chdir",        1, تہی, 'd'},
	{ "env",          1, تہی, 'e'},
	{ "umask",        1, تہی, 'k'},
	{ "group",        1, تہی, 'g'},
	{ "stop-group",   0, تہی, 'G'},
	{ "interpreted",  0, تہی, 'i'},
	{ "make-pidfile", 0, تہی, 'm'},
	{ "name",         1, تہی, 'n'},
	{ "oknodo",       0, تہی, 'o'},
	{ "pidfile",      1, تہی, 'p'},
	{ "signal",       1, تہی, 's'},
	{ "test",         0, تہی, 't'},
	{ "user",         1, تہی, 'u'},
	{ "chroot",       1, تہی, 'r'},
	{ "wait",         1, تہی, 'w'},
	{ "exec",         1, تہی, 'x'},
	{ "stdin",        1, تہی, '0'},
	{ "stdout",       1, تہی, '1'},
	{ "stderr",       1, تہی, '2'},
	{ "stdout-logger",1, تہی, '3'},
	{ "stderr-logger",1, تہی, '4'},
	{ "progress",     0, تہی, 'P'},
	{ "scheduler",    1, تہی, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, تہی, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, تہی, LONGOPT_NOTIFY},
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
ثابت حرف *usagestring = تہی;

ساکن حرف **nav;

ساکن حرف *changeuser, *ch_جڑ, *ch_dir;

خارجی حرف **environ;

#اگر !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#اگر_ختم
#اگر !defined(__DragonFly__)
ساکن اندرونی صحیح ioprio_سیٹ(صحیح which RC_غیر_مستعمل,
			     صحیح who RC_غیر_مستعمل,
			     صحیح ioprio RC_غیر_مستعمل)
{
#اگر_تعریف_شدہ SYS_ioprio_set
	واپس syscall(SYS_ioprio_set, which, who, ioprio);
#ورنہ
	واپس 0;
#اگر_ختم
}
#اگر_ختم

ساکن خالی
cleanup(خالی)
{
	آزاد(changeuser);
	آزاد(nav);
	free_schedulelist();
}

ساکن خالی
handle_signal(صحیح sig)
{
	صحیح حالت_2;
	صحیح serrno = errno;
	ثابت حرف *signame = تہی;

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
		برائے (;;) {
			اگر (waitpid(-1, &حالت_2, WNOHANG) < 0) {
				اگر (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				توڑو;
			}
		}
		توڑو;

	طےشدہ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ساکن حرف *
expand_home(ثابت حرف *home, ثابت حرف *راستہ)
{
	حرف *opath, *ppath, *p, *nh;
	ساخت passwd *pw;

	اگر (!راستہ || *راستہ != '~')
		واپس xstrdup(راستہ);

	opath = ppath = xstrdup(راستہ);
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
		} ورنہ
			home = تہی;
	} ورنہ
		ppath++;

	اگر (!home) {
	آزاد(opath);
		واپس xstrdup(راستہ);
	}
	اگر (!ppath) {
		آزاد(opath);
		واپس xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	آزاد(opath);
	واپس nh;
}

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح devnull_fd = -1;
#اگر_تعریف_شدہ TIOCNOTTY
	صحیح tty_fd = -1;
#اگر_ختم

#اگر_تعریف_شدہ HAVE_PAM
	pam_handle_t *pamh = تہی;
	صحیح pamr;
	ثابت حرف *ثابت *pamenv = تہی;
#اگر_ختم

	صحیح opt;
	size_t سائز = 0;
	bool آغاز = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	حرف *exec = تہی;
	حرف *startas = تہی;
	حرف *name = تہی;
	حرف *pidfile = تہی;
	حرف *retry = تہی;
	صحیح sig = -1;
	صحیح nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	صحیح oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool پیش_رفت = false;
	uid_t uid = 0;
	gid_t gid = 0;
	حرف *home = تہی;
	صحیح tid = 0;
	حرف *redirect_stdin = تہی;
	حرف *redirect_stderr = تہی;
	حرف *redirect_stdout = تہی;
	حرف *stderr_process = تہی;
	حرف *stdout_process = تہی;
	صحیح stdin_fd;
	صحیح stdout_fd;
	صحیح stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	صحیح i;
	حرف *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	حرف *tmp, *newpath, *np;
	حرف *p;
	حرف *token;
	حرف *exec_file = تہی;
	ساخت passwd *pw;
	ساخت group *gr;
	bool stopgroup = false;
	حرف *سطر = تہی;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	حرف **margv;
	غیرعلامتی صحیح آغاز_wait = 0;
	ثابت حرف *scheduler = تہی;
	صحیح sched_prio = -1;
#اگر_تعریف_شدہ __linux__
	cap_iab_t cap_iab = تہی;
	غیرعلامتی secbits = 0;
#اگر_ختم
#اگر_تعریف_شدہ PR_سیٹ_NO_NEW_PRIVS
	bool no_new_privs = false;
#اگر_ختم
	صحیح pipefd[2];
	حرف readbuf[1];
	ssize_t ss;
	ساخت notify notify = {0};
	صحیح ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_تیاری(SIGINT, handle_signal);
	signal_تیاری(SIGQUIT, handle_signal);
	signal_تیاری(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	اگر ((tmp = getenv("SSD_NICELEVEL")))
		اگر (لڑی_سے_پڑھو(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	اگر ((tmp = getenv("SSD_IONICELEVEL"))) {
		صحیح n = لڑی_سے_پڑھو(tmp, "%d:%d", &ionicec, &ioniced);
		اگر (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		اگر (ionicec == 0)
			ioniced = 0;
		ورنہ اگر (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	اگر ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		اگر (لڑی_سے_پڑھو(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	اگر (home == تہی || p == تہی) {
		pw = getpwuid(getuid());
		اگر (pw != تہی) {
			اگر (p == تہی)
				setenv("USER", pw->pw_name, 1);
			اگر (home == تہی) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	جب ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحیح *) 0)) != -1)
		انتخاب (opt) {
		حالت LONGOPT_CAPABILITIES:
#اگر_تعریف_شدہ __linux__
			cap_iab = cap_iab_from_متن(optarg);
			اگر (cap_iab == تہی)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ورنہ
			eerrorx("Capabilities support not enabled");
#اگر_ختم
			توڑو;

		حالت LONGOPT_SECBITS:
#اگر_تعریف_شدہ __linux__
			اگر (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = تہی;
			secbits = strtoul(optarg, &tmp, 0);
			اگر (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ورنہ
			eerrorx("Capabilities support not enabled");
#اگر_ختم
			توڑو;

		حالت LONGOPT_NO_NEW_PRIVS:
#اگر_تعریف_شدہ PR_سیٹ_NO_NEW_PRIVS
			no_new_privs = true;
#ورنہ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#اگر_ختم
			توڑو;

		حالت 'I': /* --ionice */
			اگر (لڑی_سے_پڑھو(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			اگر (ionicec == 0)
				ioniced = 0;
			ورنہ اگر (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			توڑو;

		حالت 'K':  /* --stop */
			stop = true;
			توڑو;

		حالت 'G':  /* --stop-group */
			stopgroup = true;
			توڑو;

		حالت 'N':  /* --nice */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			توڑو;

		حالت LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			توڑو;

		حالت 'P':  /* --progress */
			پیش_رفت = true;
			توڑو;

		حالت 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			توڑو;

		حالت 'S':  /* --start */
			آغاز = true;
			توڑو;

		حالت 'b':  /* --background */
			background = true;
			توڑو;

		حالت 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		حالت 'u':  /* --user <username>|<uid> */
		{
			حرف dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			اگر (لڑی_سے_پڑھو(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ورنہ
				pw = getpwuid((uid_t)tid);

			اگر (pw == تہی)
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
				اگر (لڑی_سے_پڑھو(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ورنہ
					gr = getgrgid((gid_t) tid);

				اگر (gr == تہی)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		توڑو;

		حالت 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			توڑو;

		حالت 'e': /* --env */
			putenv(optarg);
			توڑو;

		حالت 'g':  /* --group <group>|<gid> */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ورنہ
				gr = getgrgid((gid_t)tid);
			اگر (gr == تہی)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			توڑو;

		حالت 'i': /* --interpreted */
			interpreted = true;
			توڑو;

		حالت 'k':
			اگر (parse_منوال(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			توڑو;

		حالت 'm':  /* --make-pidfile */
			makepidfile = true;
			توڑو;

		حالت 'n':  /* --name <process-name> */
			name = optarg;
			توڑو;

		حالت 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			توڑو;

		حالت 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			توڑو;

		حالت 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			توڑو;

		حالت 't':  /* --test */
			test = true;
			توڑو;

		حالت 'r':  /* --chroot /new/root */
			ch_جڑ = optarg;
			توڑو;

		حالت 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			توڑو;
		حالت 'w':
			اگر (لڑی_سے_پڑھو(optarg, "%u", &آغاز_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			توڑو;
		حالت 'x':  /* --exec <executable> */
			exec = optarg;
			توڑو;

		حالت '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			توڑو;

		حالت '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			توڑو;

		حالت '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			توڑو;

		حالت '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			توڑو;

		حالت '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			توڑو;

		حالت LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			توڑو;

		حالت LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			لڑی_سے_پڑھو(optarg, "%d", &sched_prio);
			توڑو;

		حالت LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			توڑو;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	اگر (!آغاز &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	اگر (!exec)
		exec = startas;
	ورنہ اگر (!name)
		name = startas;

	اگر (!exec) {
		exec = *argv;
		اگر (!exec)
			exec = name;
		اگر (name && آغاز)
			*argv = name;
	} ورنہ اگر (name) {
		*--argv = name;
		++argc;
	} ورنہ اگر (exec) {
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
		اگر (آغاز_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ورنہ {
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
	اگر (ch_جڑ && *ch_جڑ == '~')
		ch_جڑ = expand_home(home, ch_جڑ);
	اگر (exec) {
		اگر (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		اگر (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			اگر (ch_جڑ)
				xasprintf(&exec_file, "%s/%s", ch_جڑ, exec);
			ورنہ
				xasprintf(&exec_file, "%s", exec);
		} ورنہ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = تہی;
			جب ((token = strsep(&p, ":"))) {
				اگر (ch_جڑ)
					xasprintf(&exec_file, "%s/%s/%s", ch_جڑ, token, exec);
				ورنہ
					xasprintf(&exec_file, "%s/%s", token, exec);
				اگر (exec_file && access(exec_file, F_OK) == 0)
					توڑو;
				آزاد(exec_file);
				exec_file = تہی;
			}
			آزاد(tmp);
		}
	}
	اگر (آغاز && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		آزاد(exec_file);
		باہر_نکلو(EXIT_FAILURE);
	}
	اگر (آغاز && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	اگر (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		اگر (fp) {
			سطر = تہی;
			اگر (xgetline(&سطر, &سائز, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = سطر;
			fclose(fp);
			اگر (p != تہی && سطر[0] == '#' && سطر[1] == '!') {
				p = سطر + 2;
				/* Strip leading spaces */
				جب (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				آزاد(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				برائے (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(حجم(حرف *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				اگر (p)
					nav[len++] = p;
				برائے (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = تہی;
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
			parse_جدول_وقت(applet, retry, sig);
		ورنہ اگر (test || oknodo)
			parse_جدول_وقت(applet, "0", sig);
		ورنہ
			parse_جدول_وقت(applet, تہی, sig);
		اگر (pidfile) {
			pid = get_pid(applet, pidfile);
			اگر (pid == -1 && errno != ENOENT)
				باہر_نکلو(EXIT_FAILURE);
		} ورنہ {
			pid = 0;
		}
		i = تسلسل_stop_جدول_وقت(applet, exec, (ثابت حرف *ثابت *)margv,
		    pid, uid, stopgroup, test, پیش_رفت, false);

		اگر (i < 0)
			/* We failed to stop something */
			باہر_نکلو(EXIT_FAILURE);
		اگر (test || oknodo)
			واپس i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		اگر (pidfile)
			unlink(pidfile);
		اگر (svcname)
			rc_service_daemon_سیٹ(svcname, exec,
			    (ثابت حرف *ثابت *)argv,
			    pidfile, false);
		باہر_نکلو(EXIT_SUCCESS);
	}

	اگر (pidfile)
		pid = get_pid(applet, pidfile);
	ورنہ
		pid = 0;

	اگر (pid)
		pids = rc_ڈھونڈو_pids(تہی, تہی, 0, pid);
	ورنہ
		pids = rc_ڈھونڈو_pids(exec, (ثابت حرف * ثابت *) argv, uid, 0);
	اگر (pids)
		eerrorx("%s: %s is already running", applet, exec);

	آزاد(pids);
	اگر (test) {
		اگر (rc_yesno(getenv("EINFO_QUIET")))
			باہر_نکلو (EXIT_SUCCESS);

		einfon("Would start");
		جب (argc-- > 0)
			لکھو(" %s", *argv++);
		لکھو("\n");
		eindent();
		اگر (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		اگر (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		اگر (ch_جڑ)
			einfo("in root `%s'", ch_جڑ);
		اگر (ch_dir)
			einfo("in dir `%s'", ch_dir);
		اگر (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		اگر (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		باہر_نکلو(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	اگر (pidfile)
		unlink(pidfile);

	اگر (background)
		signal_تیاری(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	اگر (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	اگر ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	اگر (pid == 0) {
		gid_t گروہ_buf[32], *گروہ_list = گروہ_buf;
		صحیح گروہ_گنتی = ARRAY_سائز(گروہ_buf);
		pid_t mypid = getpid();
		close(pipefd[0]); /* Close the read end of the pipe. */
		umask(numask);

#اگر_تعریف_شدہ TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#اگر_ختم

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
			} ورنہ اگر (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		اگر (ionicec != -1 &&
		    ioprio_سیٹ(1, mypid, ionicec | ioniced) == -1)
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

#اگر_تعریف_شدہ HAVE_PAM
		اگر (changeuser != تہی) {
			pamr = pam_آغاز("start-stop-daemon",
			    changeuser, &conv, &pamh);

			اگر (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			اگر (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			اگر (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#اگر_ختم
		اگر (changeuser && getgrouplist(changeuser, gid, گروہ_list, &گروہ_گنتی) < 0) {
			گروہ_list = xmalloc(گروہ_گنتی * حجم(*گروہ_list));
			اگر (getgrouplist(changeuser, gid, گروہ_list, &گروہ_گنتی) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		اگر (ch_جڑ && chroot(ch_جڑ) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_جڑ, strerror(errno));

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
		اگر (changeuser && setgroups(گروہ_گنتی, گروہ_list))
			eerrorx("%s: setgroups() failed", applet);
		اگر (گروہ_list != گروہ_buf)
			آزاد(گروہ_list);
#اگر_تعریف_شدہ __linux__
		اگر (uid && cap_setuid(uid))
#ورنہ
		اگر (uid && setuid(uid))
#اگر_ختم
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#اگر_تعریف_شدہ __linux__
		اگر (cap_iab != تہی) {
			i = cap_iab_سیٹ_proc(cap_iab);

			اگر (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			اگر (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		اگر (secbits != 0) {
			اگر (cap_سیٹ_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#اگر_ختم

#اگر_تعریف_شدہ PR_سیٹ_NO_NEW_PRIVS
		اگر (no_new_privs) {
			اگر (prctl(PR_سیٹ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
				eerrorx("Could not set No New Privs flag: %s", strerror(errno));
		}
#اگر_ختم


#اگر_تعریف_شدہ TIOCNOTTY
		ioctl(tty_fd, TIOCNOTTY, 0);
		close(tty_fd);
#اگر_ختم

		/* Clean the environment of any RC_ variables */
		env_list = rc_stringlist_new();
		i = 0;
		جب (environ[i])
			rc_stringlist_جوڑو(env_list, environ[i++]);

#اگر_تعریف_شدہ HAVE_PAM
		اگر (changeuser != تہی) {
			pamenv = (ثابت حرف *ثابت *)pam_getenvlist(pamh);
			اگر (pamenv) {
				جب (*pamenv) {
					/* Don't add strings unless they set a var */
					اگر (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ورنہ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#اگر_ختم

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			اگر (لڑی_موازنہ(env->قدر, "RC_", 3) == 0
					|| لڑی_موازنہ(env->قدر, "SSD_", 4) == 0
					|| لڑی_موازنہ(env->قدر, "EINFO_", 6) == 0) {
				*strchr(env->قدر, '=') = '\0';
				unsetenv(env->قدر);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		اگر ((token = getenv("PATH"))) {
			len = لڑی_لمبائی(token);
			newpath = np = xmalloc(len + 1);
			جب (token && *token) {
				p = strchr(token, ':');
				اگر (p) {
					*p++ = '\0';
					جب (*p == ':')
						p++;
				}
				اگر (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = لڑی_لمبائی(token);
					اگر (np != newpath)
						*np++ = ':';
					حافظہ_نقل(np, token, len);
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
		}ورنہ اگر (stdout_process) {
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
		}ورنہ اگر (stderr_process) {
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

		اگر (scheduler != تہی) {
			صحیح scheduler_اشاریہ;
			ساخت sched_param sched =  {.sched_priority = sched_prio};
			اگر (strcmp(scheduler, "fifo") == 0)
				scheduler_اشاریہ = SCHED_FIFO;
			ورنہ اگر (strcmp(scheduler, "rr") == 0)
				scheduler_اشاریہ = SCHED_RR;
			ورنہ اگر (strcmp(scheduler, "other") == 0)
				scheduler_اشاریہ = SCHED_OTHER;
#اگر_تعریف_شدہ SCHED_BATCH
			ورنہ اگر (strcmp(scheduler, "batch") == 0)
				scheduler_اشاریہ = SCHED_BATCH;
#اگر_ختم
#اگر_تعریف_شدہ SCHED_IDLE
			ورنہ اگر (strcmp(scheduler, "idle") == 0)
				scheduler_اشاریہ = SCHED_IDLE;
#اگر_ختم
			ورنہ اگر (لڑی_سے_پڑھو(scheduler, "%d", &scheduler_اشاریہ) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			اگر (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_اشاریہ);

			اگر (pthread_setschedparam(pthread_self(), scheduler_اشاریہ, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ورنہ اگر (sched_prio != -1) {
			ثابت ساخت sched_param sched =  {.sched_priority = sched_prio};
			اگر (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#اگر_تعریف_شدہ HAVE_PAM
		اگر (changeuser != تہی && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#اگر_ختم
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	جب ((ss = اقرأ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	اگر (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	اگر (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		کرو {
			pid = waitpid(spid, &i, 0);
			اگر (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				واپس -1;
			}
		} جب (!WIFEXITED(i) && !WIFSIGNALED(i));
		اگر (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			باہر_نکلو(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	اگر (آغاز_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_قدر("rc_start_wait"))))
	{
		اگر (لڑی_سے_پڑھو(p, "%u", &آغاز_wait) != 1)
			آغاز_wait = 0;
	}

	اگر (notify.type != NOTIFY_NONE) {
		اگر (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ورنہ اگر (آغاز_wait > 0) {
		ساخت timespec ts;
		bool alive = false;

		ts.tv_sec = آغاز_wait / 1000;
		ts.tv_nsec = (آغاز_wait % 1000) * ONE_MS;
		اگر (nanosleep(&ts, تہی) == -1) {
			اگر (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				واپس 0;
			}
		}
		اگر (background) {
			اگر (kill(pid, 0) == 0)
				alive = true;
		} ورنہ {
			اگر (pidfile) {
				pid = get_pid(applet, pidfile);
				اگر (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ورنہ
				pid = 0;
			اگر (do_stop(applet, exec, (ثابت حرف *ثابت *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		اگر (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	اگر (svcname)
		rc_service_daemon_سیٹ(svcname, exec,
		    (ثابت حرف *ثابت *)margv, pidfile, true);

	باہر_نکلو(ret);
	/* NOTREACHED */
}
