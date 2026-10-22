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

#எனில் defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <grp.h>
#அந்தர்பாவ <pthread.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <sched.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <sys/resource.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#அந்தர்பாவ <termios.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
நிலையான கட்டமைப்பு pam_conv conv = { வெற்று, வெற்று};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "schedules.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
எண்ணிக்கை {
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

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "capabilities", 1, வெற்று, LONGOPT_CAPABILITIES},
	{ "secbits",      1, வெற்று, LONGOPT_SECBITS},
	{ "no-new-privs", 0, வெற்று, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, வெற்று, 'I'},
	{ "stop",         0, வெற்று, 'K'},
	{ "nicelevel",    1, வெற்று, 'N'},
	{ "oom-score-adj",1, வெற்று, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, வெற்று, 'R'},
	{ "start",        0, வெற்று, 'S'},
	{ "startas",      1, வெற்று, 'a'},
	{ "background",   0, வெற்று, 'b'},
	{ "chuid",        1, வெற்று, 'c'},
	{ "chdir",        1, வெற்று, 'd'},
	{ "env",          1, வெற்று, 'e'},
	{ "umask",        1, வெற்று, 'k'},
	{ "group",        1, வெற்று, 'g'},
	{ "stop-group",   0, வெற்று, 'G'},
	{ "interpreted",  0, வெற்று, 'i'},
	{ "make-pidfile", 0, வெற்று, 'm'},
	{ "name",         1, வெற்று, 'n'},
	{ "oknodo",       0, வெற்று, 'o'},
	{ "pidfile",      1, வெற்று, 'p'},
	{ "signal",       1, வெற்று, 's'},
	{ "test",         0, வெற்று, 't'},
	{ "user",         1, வெற்று, 'u'},
	{ "chroot",       1, வெற்று, 'r'},
	{ "wait",         1, வெற்று, 'w'},
	{ "exec",         1, வெற்று, 'x'},
	{ "stdin",        1, வெற்று, '0'},
	{ "stdout",       1, வெற்று, '1'},
	{ "stderr",       1, வெற்று, '2'},
	{ "stdout-logger",1, வெற்று, '3'},
	{ "stderr-logger",1, வெற்று, '4'},
	{ "progress",     0, வெற்று, 'P'},
	{ "scheduler",    1, வெற்று, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, வெற்று, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, வெற்று, LONGOPT_NOTIFY},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = வெற்று;

நிலையான எழுத்து **nav;

நிலையான எழுத்து *changeuser, *ch_வேர், *ch_dir;

வெளி எழுத்து **environ;

#எனில் !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#எனில் !defined(__DragonFly__)
நிலையான உள்வரி முழு ioprio_set(முழு which RC_UNUSED,
			     முழு who RC_UNUSED,
			     முழு ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	திரும்பு syscall(SYS_ioprio_set, which, who, ioprio);
#இல்லையெனில்
	திரும்பு 0;
#endif
}
#endif

நிலையான வெறுமை
cleanup(வெறுமை)
{
	விடுவி(changeuser);
	விடுவி(nav);
	free_schedulelist();
}

நிலையான வெறுமை
handle_signal(முழு sig)
{
	முழு status;
	முழு serrno = errno;
	மாறிலி எழுத்து *signame = வெற்று;

	தேர்வு (sig) {
	நிலை SIGINT:
		எனில் (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	நிலை SIGTERM:
		எனில் (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	நிலை SIGQUIT:
		எனில் (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	நிலை SIGCHLD:
		க்கு (;;) {
			எனில் (waitpid(-1, &status, WNOHANG) < 0) {
				எனில் (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				நிறுத்து;
			}
		}
		நிறுத்து;

	இயல்பு:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

நிலையான எழுத்து *
expand_home(மாறிலி எழுத்து *home, மாறிலி எழுத்து *path)
{
	எழுத்து *opath, *ppath, *p, *nh;
	கட்டமைப்பு passwd *pw;

	எனில் (!path || *path != '~')
		திரும்பு xstrdup(path);

	opath = ppath = xstrdup(path);
	எனில் (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		எனில் (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		எனில் (pw) {
			home = pw->pw_dir;
			ppath = p;
			எனில் (ppath)
				*ppath = '/';
		} இல்லையெனில்
			home = வெற்று;
	} இல்லையெனில்
		ppath++;

	எனில் (!home) {
	விடுவி(opath);
		திரும்பு xstrdup(path);
	}
	எனில் (!ppath) {
		விடுவி(opath);
		திரும்பு xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	விடுவி(opath);
	திரும்பு nh;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு devnull_fd = -1;
#ifdef TIOCNOTTY
	முழு tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = வெற்று;
	முழு pamr;
	மாறிலி எழுத்து *மாறிலி *pamenv = வெற்று;
#endif

	முழு opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	எழுத்து *exec = வெற்று;
	எழுத்து *startas = வெற்று;
	எழுத்து *name = வெற்று;
	எழுத்து *pidfile = வெற்று;
	எழுத்து *retry = வெற்று;
	முழு sig = -1;
	முழு nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	முழு oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	எழுத்து *home = வெற்று;
	முழு tid = 0;
	எழுத்து *redirect_stdin = வெற்று;
	எழுத்து *redirect_stderr = வெற்று;
	எழுத்து *redirect_stdout = வெற்று;
	எழுத்து *stderr_process = வெற்று;
	எழுத்து *stdout_process = வெற்று;
	முழு stdin_fd;
	முழு stdout_fd;
	முழு stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	முழு i;
	எழுத்து *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	எழுத்து *tmp, *newpath, *np;
	எழுத்து *p;
	எழுத்து *token;
	எழுத்து *exec_file = வெற்று;
	கட்டமைப்பு passwd *pw;
	கட்டமைப்பு group *gr;
	bool stopgroup = false;
	எழுத்து *வரி = வெற்று;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	எழுத்து **margv;
	unsigned முழு start_wait = 0;
	மாறிலி எழுத்து *scheduler = வெற்று;
	முழு sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = வெற்று;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	முழு pipefd[2];
	எழுத்து readbuf[1];
	ssize_t ss;
	கட்டமைப்பு notify notify = {0};
	முழு ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	எனில் ((tmp = getenv("SSD_NICELEVEL")))
		எனில் (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	எனில் ((tmp = getenv("SSD_IONICELEVEL"))) {
		முழு n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		எனில் (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		எனில் (ionicec == 0)
			ioniced = 0;
		இல்லையெனில் எனில் (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	எனில் ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		எனில் (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	எனில் (home == வெற்று || p == வெற்று) {
		pw = getpwuid(getuid());
		எனில் (pw != வெற்று) {
			எனில் (p == வெற்று)
				setenv("USER", pw->pw_name, 1);
			எனில் (home == வெற்று) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	போது ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (முழு *) 0)) != -1)
		தேர்வு (opt) {
		நிலை LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			எனில் (cap_iab == வெற்று)
				eerrorx("Could not parse iab: %s", strerror(errno));
#இல்லையெனில்
			eerrorx("Capabilities support not enabled");
#endif
			நிறுத்து;

		நிலை LONGOPT_SECBITS:
#ifdef __linux__
			எனில் (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = வெற்று;
			secbits = strtoul(optarg, &tmp, 0);
			எனில் (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#இல்லையெனில்
			eerrorx("Capabilities support not enabled");
#endif
			நிறுத்து;

		நிலை LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#இல்லையெனில்
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			நிறுத்து;

		நிலை 'I': /* --ionice */
			எனில் (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			எனில் (ionicec == 0)
				ioniced = 0;
			இல்லையெனில் எனில் (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			நிறுத்து;

		நிலை 'K':  /* --stop */
			stop = true;
			நிறுத்து;

		நிலை 'G':  /* --stop-group */
			stopgroup = true;
			நிறுத்து;

		நிலை 'N':  /* --nice */
			எனில் (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			நிறுத்து;

		நிலை LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			எனில் (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			நிறுத்து;

		நிலை 'P':  /* --progress */
			progress = true;
			நிறுத்து;

		நிலை 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			நிறுத்து;

		நிலை 'S':  /* --start */
			start = true;
			நிறுத்து;

		நிலை 'b':  /* --background */
			background = true;
			நிறுத்து;

		நிலை 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		நிலை 'u':  /* --user <username>|<uid> */
		{
			எழுத்து dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			எனில் (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			இல்லையெனில்
				pw = getpwuid((uid_t)tid);

			எனில் (pw == வெற்று)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			எனில் (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			எனில் (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			எனில் (gid == 0)
				gid = pw->pw_gid;

			எனில் (p) {
				tmp = strsep (&p, ":");
				எனில் (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				இல்லையெனில்
					gr = getgrgid((gid_t) tid);

				எனில் (gr == வெற்று)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		நிறுத்து;

		நிலை 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			நிறுத்து;

		நிலை 'e': /* --env */
			putenv(optarg);
			நிறுத்து;

		நிலை 'g':  /* --group <group>|<gid> */
			எனில் (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			இல்லையெனில்
				gr = getgrgid((gid_t)tid);
			எனில் (gr == வெற்று)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			நிறுத்து;

		நிலை 'i': /* --interpreted */
			interpreted = true;
			நிறுத்து;

		நிலை 'k':
			எனில் (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			நிறுத்து;

		நிலை 'm':  /* --make-pidfile */
			makepidfile = true;
			நிறுத்து;

		நிலை 'n':  /* --name <process-name> */
			name = optarg;
			நிறுத்து;

		நிலை 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			நிறுத்து;

		நிலை 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			நிறுத்து;

		நிலை 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			நிறுத்து;

		நிலை 't':  /* --test */
			test = true;
			நிறுத்து;

		நிலை 'r':  /* --chroot /new/root */
			ch_வேர் = optarg;
			நிறுத்து;

		நிலை 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			நிறுத்து;
		நிலை 'w':
			எனில் (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			நிறுத்து;
		நிலை 'x':  /* --exec <executable> */
			exec = optarg;
			நிறுத்து;

		நிலை '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			நிறுத்து;

		நிலை '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			நிறுத்து;

		நிலை '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			நிறுத்து;

		நிலை '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			நிறுத்து;

		நிலை '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			நிறுத்து;

		நிலை LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			நிறுத்து;

		நிலை LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			நிறுத்து;

		நிலை LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	எனில் (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	எனில் (!exec)
		exec = startas;
	இல்லையெனில் எனில் (!name)
		name = startas;

	எனில் (!exec) {
		exec = *argv;
		எனில் (!exec)
			exec = name;
		எனில் (name && start)
			*argv = name;
	} இல்லையெனில் எனில் (name) {
		*--argv = name;
		++argc;
	} இல்லையெனில் எனில் (exec) {
		*--argv = exec;
		++argc;
	};

	எனில் (stop || sig != -1) {
		எனில் (sig == -1)
			sig = SIGTERM;
		எனில் (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		எனில் (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		எனில் (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		எனில் (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		எனில் (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		எனில் (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} இல்லையெனில் {
		எனில் (!exec)
			eerrorx("%s: nothing to start", applet);
		எனில் (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		எனில் ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		எனில் ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		எனில் (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		எனில் (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	எனில் (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	எனில் (ch_வேர் && *ch_வேர் == '~')
		ch_வேர் = expand_home(home, ch_வேர்);
	எனில் (exec) {
		எனில் (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		எனில் (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			எனில் (ch_வேர்)
				xasprintf(&exec_file, "%s/%s", ch_வேர், exec);
			இல்லையெனில்
				xasprintf(&exec_file, "%s", exec);
		} இல்லையெனில் {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = வெற்று;
			போது ((token = strsep(&p, ":"))) {
				எனில் (ch_வேர்)
					xasprintf(&exec_file, "%s/%s/%s", ch_வேர், token, exec);
				இல்லையெனில்
					xasprintf(&exec_file, "%s/%s", token, exec);
				எனில் (exec_file && access(exec_file, F_OK) == 0)
					நிறுத்து;
				விடுவி(exec_file);
				exec_file = வெற்று;
			}
			விடுவி(tmp);
		}
	}
	எனில் (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		விடுவி(exec_file);
		exit(EXIT_FAILURE);
	}
	எனில் (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	எனில் (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		எனில் (fp) {
			வரி = வெற்று;
			எனில் (xgetline(&வரி, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = வரி;
			fclose(fp);
			எனில் (p != வெற்று && வரி[0] == '#' && வரி[1] == '!') {
				p = வரி + 2;
				/* Strip leading spaces */
				போது (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				விடுவி(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				க்கு (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(அளவு(எழுத்து *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				எனில் (p)
					nav[len++] = p;
				க்கு (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = வெற்று;
			}
		}
	}
	margv = nav ? nav : argv;

	எனில் (stop || sig != -1) {
		எனில் (sig == -1)
			sig = SIGTERM;
		எனில் (!stop)
			oknodo = true;
		எனில் (retry)
			parse_schedule(applet, retry, sig);
		இல்லையெனில் எனில் (test || oknodo)
			parse_schedule(applet, "0", sig);
		இல்லையெனில்
			parse_schedule(applet, வெற்று, sig);
		எனில் (pidfile) {
			pid = get_pid(applet, pidfile);
			எனில் (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} இல்லையெனில் {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (மாறிலி எழுத்து *மாறிலி *)margv,
		    pid, uid, stopgroup, test, progress, false);

		எனில் (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		எனில் (test || oknodo)
			திரும்பு i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		எனில் (pidfile)
			unlink(pidfile);
		எனில் (svcname)
			rc_service_daemon_set(svcname, exec,
			    (மாறிலி எழுத்து *மாறிலி *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	எனில் (pidfile)
		pid = get_pid(applet, pidfile);
	இல்லையெனில்
		pid = 0;

	எனில் (pid)
		pids = rc_find_pids(வெற்று, வெற்று, 0, pid);
	இல்லையெனில்
		pids = rc_find_pids(exec, (மாறிலி எழுத்து * மாறிலி *) argv, uid, 0);
	எனில் (pids)
		eerrorx("%s: %s is already running", applet, exec);

	விடுவி(pids);
	எனில் (test) {
		எனில் (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		போது (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		எனில் (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		எனில் (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		எனில் (ch_வேர்)
			einfo("in root `%s'", ch_வேர்);
		எனில் (ch_dir)
			einfo("in dir `%s'", ch_dir);
		எனில் (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		எனில் (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	எனில் (pidfile)
		unlink(pidfile);

	எனில் (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	எனில் (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	எனில் ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	எனில் (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		முழு group_மொத்தம் = ARRAY_SIZE(group_buf);
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

		எனில் (nicelevel != INT_MIN) {
			எனில் (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			எனில் (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} இல்லையெனில் எனில் (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		எனில் (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		எனில் (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			எனில் (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		எனில் (changeuser != வெற்று) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			எனில் (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			எனில் (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			எனில் (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		எனில் (changeuser && getgrouplist(changeuser, gid, group_list, &group_மொத்தம்) < 0) {
			group_list = xmalloc(group_மொத்தம் * அளவு(*group_list));
			எனில் (getgrouplist(changeuser, gid, group_list, &group_மொத்தம்) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		எனில் (ch_வேர் && chroot(ch_வேர்) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_வேர், strerror(errno));

		எனில் (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		எனில் (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			எனில் (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		எனில் (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		எனில் (changeuser && setgroups(group_மொத்தம், group_list))
			eerrorx("%s: setgroups() failed", applet);
		எனில் (group_list != group_buf)
			விடுவி(group_list);
#ifdef __linux__
		எனில் (uid && cap_setuid(uid))
#இல்லையெனில்
		எனில் (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		எனில் (cap_iab != வெற்று) {
			i = cap_iab_set_proc(cap_iab);

			எனில் (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			எனில் (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		எனில் (secbits != 0) {
			எனில் (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		எனில் (no_new_privs) {
			எனில் (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		போது (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		எனில் (changeuser != வெற்று) {
			pamenv = (மாறிலி எழுத்து *மாறிலி *)pam_getenvlist(pamh);
			எனில் (pamenv) {
				போது (*pamenv) {
					/* Don't add strings unless they set a var */
					எனில் (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					இல்லையெனில்
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			எனில் (strncmp(env->மதிப்பு, "RC_", 3) == 0
					|| strncmp(env->மதிப்பு, "SSD_", 4) == 0
					|| strncmp(env->மதிப்பு, "EINFO_", 6) == 0) {
				*strchr(env->மதிப்பு, '=') = '\0';
				unsetenv(env->மதிப்பு);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		எனில் ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			போது (token && *token) {
				p = strchr(token, ':');
				எனில் (p) {
					*p++ = '\0';
					போது (*p == ':')
						p++;
				}
				எனில் (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					எனில் (np != newpath)
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
		எனில் (redirect_stdin) {
			எனில் ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		எனில் (redirect_stdout) {
			எனில் ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}இல்லையெனில் எனில் (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			எனில் (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		எனில் (redirect_stderr) {
			எனில் ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}இல்லையெனில் எனில் (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			எனில் (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		எனில் (background)
			dup2(stdin_fd, STDIN_FILENO);
		எனில் (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		எனில் (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		எனில் (notify.type == NOTIFY_FD) {
			எனில் (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			எனில் (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			எனில் (notify.pipe[1] == notify.fd) {
				முழு flags = fcntl(notify.fd, F_GETFD, 0);
				எனில் (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				எனில் (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		எனில் (scheduler != வெற்று) {
			முழு scheduler_index;
			கட்டமைப்பு sched_param sched =  {.sched_priority = sched_prio};
			எனில் (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			இல்லையெனில் எனில் (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			இல்லையெனில் எனில் (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			இல்லையெனில் எனில் (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			இல்லையெனில் எனில் (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			இல்லையெனில் எனில் (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			எனில் (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			எனில் (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} இல்லையெனில் எனில் (sched_prio != -1) {
			மாறிலி கட்டமைப்பு sched_param sched =  {.sched_priority = sched_prio};
			எனில் (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		எனில் (changeuser != வெற்று && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	போது ((ss = பட(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	எனில் (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	எனில் (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		செய் {
			pid = waitpid(spid, &i, 0);
			எனில் (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				திரும்பு -1;
			}
		} போது (!WIFEXITED(i) && !WIFSIGNALED(i));
		எனில் (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	எனில் (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_மதிப்பு("rc_start_wait"))))
	{
		எனில் (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	எனில் (notify.type != NOTIFY_NONE) {
		எனில் (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} இல்லையெனில் எனில் (start_wait > 0) {
		கட்டமைப்பு timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		எனில் (nanosleep(&ts, வெற்று) == -1) {
			எனில் (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				திரும்பு 0;
			}
		}
		எனில் (background) {
			எனில் (kill(pid, 0) == 0)
				alive = true;
		} இல்லையெனில் {
			எனில் (pidfile) {
				pid = get_pid(applet, pidfile);
				எனில் (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} இல்லையெனில்
				pid = 0;
			எனில் (do_stop(applet, exec, (மாறிலி எழுத்து *மாறிலி *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		எனில் (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	எனில் (svcname)
		rc_service_daemon_set(svcname, exec,
		    (மாறிலி எழுத்து *மாறிலி *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
