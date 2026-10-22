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

#જો defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <grp.h>
#અન્તર્ભાવ <pthread.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <sched.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <sys/resource.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#અન્તર્ભાવ <termios.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
સ્થિર સંરચના pam_conv conv = { ખાલી, ખાલી};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "schedules.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ગણના {
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

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "capabilities", 1, ખાલી, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ખાલી, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ખાલી, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ખાલી, 'I'},
	{ "stop",         0, ખાલી, 'K'},
	{ "nicelevel",    1, ખાલી, 'N'},
	{ "oom-score-adj",1, ખાલી, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ખાલી, 'R'},
	{ "start",        0, ખાલી, 'S'},
	{ "startas",      1, ખાલી, 'a'},
	{ "background",   0, ખાલી, 'b'},
	{ "chuid",        1, ખાલી, 'c'},
	{ "chdir",        1, ખાલી, 'd'},
	{ "env",          1, ખાલી, 'e'},
	{ "umask",        1, ખાલી, 'k'},
	{ "group",        1, ખાલી, 'g'},
	{ "stop-group",   0, ખાલી, 'G'},
	{ "interpreted",  0, ખાલી, 'i'},
	{ "make-pidfile", 0, ખાલી, 'm'},
	{ "name",         1, ખાલી, 'n'},
	{ "oknodo",       0, ખાલી, 'o'},
	{ "pidfile",      1, ખાલી, 'p'},
	{ "signal",       1, ખાલી, 's'},
	{ "test",         0, ખાલી, 't'},
	{ "user",         1, ખાલી, 'u'},
	{ "chroot",       1, ખાલી, 'r'},
	{ "wait",         1, ખાલી, 'w'},
	{ "exec",         1, ખાલી, 'x'},
	{ "stdin",        1, ખાલી, '0'},
	{ "stdout",       1, ખાલી, '1'},
	{ "stderr",       1, ખાલી, '2'},
	{ "stdout-logger",1, ખાલી, '3'},
	{ "stderr-logger",1, ખાલી, '4'},
	{ "progress",     0, ખાલી, 'P'},
	{ "scheduler",    1, ખાલી, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ખાલી, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ખાલી, LONGOPT_NOTIFY},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ખાલી;

સ્થિર અક્ષર **nav;

સ્થિર અક્ષર *changeuser, *ch_મૂળ, *ch_dir;

બાહ્ય અક્ષર **environ;

#જો !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#જો !defined(__DragonFly__)
સ્થિર અંતઃરેખ પૂર્ણાંક ioprio_set(પૂર્ણાંક which RC_UNUSED,
			     પૂર્ણાંક who RC_UNUSED,
			     પૂર્ણાંક ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	પાછા syscall(SYS_ioprio_set, which, who, ioprio);
#નહીંતર
	પાછા 0;
#endif
}
#endif

સ્થિર શૂન્ય
cleanup(શૂન્ય)
{
	મુક્ત(changeuser);
	મુક્ત(nav);
	free_schedulelist();
}

સ્થિર શૂન્ય
handle_signal(પૂર્ણાંક sig)
{
	પૂર્ણાંક status;
	પૂર્ણાંક serrno = errno;
	અચલ અક્ષર *signame = ખાલી;

	પસંદગી (sig) {
	સ્થિતિ SIGINT:
		જો (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	સ્થિતિ SIGTERM:
		જો (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	સ્થિતિ SIGQUIT:
		જો (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	સ્થિતિ SIGCHLD:
		માટે (;;) {
			જો (waitpid(-1, &status, WNOHANG) < 0) {
				જો (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				તોડો;
			}
		}
		તોડો;

	મૂળભૂત:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

સ્થિર અક્ષર *
expand_home(અચલ અક્ષર *home, અચલ અક્ષર *path)
{
	અક્ષર *opath, *ppath, *p, *nh;
	સંરચના passwd *pw;

	જો (!path || *path != '~')
		પાછા xstrdup(path);

	opath = ppath = xstrdup(path);
	જો (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		જો (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		જો (pw) {
			home = pw->pw_dir;
			ppath = p;
			જો (ppath)
				*ppath = '/';
		} નહીંતર
			home = ખાલી;
	} નહીંતર
		ppath++;

	જો (!home) {
	મુક્ત(opath);
		પાછા xstrdup(path);
	}
	જો (!ppath) {
		મુક્ત(opath);
		પાછા xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	મુક્ત(opath);
	પાછા nh;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક devnull_fd = -1;
#ifdef TIOCNOTTY
	પૂર્ણાંક tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = ખાલી;
	પૂર્ણાંક pamr;
	અચલ અક્ષર *અચલ *pamenv = ખાલી;
#endif

	પૂર્ણાંક opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	અક્ષર *exec = ખાલી;
	અક્ષર *startas = ખાલી;
	અક્ષર *name = ખાલી;
	અક્ષર *pidfile = ખાલી;
	અક્ષર *retry = ખાલી;
	પૂર્ણાંક sig = -1;
	પૂર્ણાંક nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	પૂર્ણાંક oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	અક્ષર *home = ખાલી;
	પૂર્ણાંક tid = 0;
	અક્ષર *redirect_stdin = ખાલી;
	અક્ષર *redirect_stderr = ખાલી;
	અક્ષર *redirect_stdout = ખાલી;
	અક્ષર *stderr_process = ખાલી;
	અક્ષર *stdout_process = ખાલી;
	પૂર્ણાંક stdin_fd;
	પૂર્ણાંક stdout_fd;
	પૂર્ણાંક stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	પૂર્ણાંક i;
	અક્ષર *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	અક્ષર *tmp, *newpath, *np;
	અક્ષર *p;
	અક્ષર *token;
	અક્ષર *exec_file = ખાલી;
	સંરચના passwd *pw;
	સંરચના group *gr;
	bool stopgroup = false;
	અક્ષર *લીટી = ખાલી;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	અક્ષર **margv;
	unsigned પૂર્ણાંક start_wait = 0;
	અચલ અક્ષર *scheduler = ખાલી;
	પૂર્ણાંક sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = ખાલી;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	પૂર્ણાંક pipefd[2];
	અક્ષર readbuf[1];
	ssize_t ss;
	સંરચના notify notify = {0};
	પૂર્ણાંક ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	જો ((tmp = getenv("SSD_NICELEVEL")))
		જો (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	જો ((tmp = getenv("SSD_IONICELEVEL"))) {
		પૂર્ણાંક n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		જો (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		જો (ionicec == 0)
			ioniced = 0;
		નહીંતર જો (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	જો ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		જો (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	જો (home == ખાલી || p == ખાલી) {
		pw = getpwuid(getuid());
		જો (pw != ખાલી) {
			જો (p == ખાલી)
				setenv("USER", pw->pw_name, 1);
			જો (home == ખાલી) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	જ્યારે ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (પૂર્ણાંક *) 0)) != -1)
		પસંદગી (opt) {
		સ્થિતિ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			જો (cap_iab == ખાલી)
				eerrorx("Could not parse iab: %s", strerror(errno));
#નહીંતર
			eerrorx("Capabilities support not enabled");
#endif
			તોડો;

		સ્થિતિ LONGOPT_SECBITS:
#ifdef __linux__
			જો (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ખાલી;
			secbits = strtoul(optarg, &tmp, 0);
			જો (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#નહીંતર
			eerrorx("Capabilities support not enabled");
#endif
			તોડો;

		સ્થિતિ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#નહીંતર
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			તોડો;

		સ્થિતિ 'I': /* --ionice */
			જો (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			જો (ionicec == 0)
				ioniced = 0;
			નહીંતર જો (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			તોડો;

		સ્થિતિ 'K':  /* --stop */
			stop = true;
			તોડો;

		સ્થિતિ 'G':  /* --stop-group */
			stopgroup = true;
			તોડો;

		સ્થિતિ 'N':  /* --nice */
			જો (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			તોડો;

		સ્થિતિ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			જો (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			તોડો;

		સ્થિતિ 'P':  /* --progress */
			progress = true;
			તોડો;

		સ્થિતિ 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			તોડો;

		સ્થિતિ 'S':  /* --start */
			start = true;
			તોડો;

		સ્થિતિ 'b':  /* --background */
			background = true;
			તોડો;

		સ્થિતિ 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		સ્થિતિ 'u':  /* --user <username>|<uid> */
		{
			અક્ષર dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			જો (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			નહીંતર
				pw = getpwuid((uid_t)tid);

			જો (pw == ખાલી)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			જો (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			જો (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			જો (gid == 0)
				gid = pw->pw_gid;

			જો (p) {
				tmp = strsep (&p, ":");
				જો (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				નહીંતર
					gr = getgrgid((gid_t) tid);

				જો (gr == ખાલી)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		તોડો;

		સ્થિતિ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			તોડો;

		સ્થિતિ 'e': /* --env */
			putenv(optarg);
			તોડો;

		સ્થિતિ 'g':  /* --group <group>|<gid> */
			જો (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			નહીંતર
				gr = getgrgid((gid_t)tid);
			જો (gr == ખાલી)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			તોડો;

		સ્થિતિ 'i': /* --interpreted */
			interpreted = true;
			તોડો;

		સ્થિતિ 'k':
			જો (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			તોડો;

		સ્થિતિ 'm':  /* --make-pidfile */
			makepidfile = true;
			તોડો;

		સ્થિતિ 'n':  /* --name <process-name> */
			name = optarg;
			તોડો;

		સ્થિતિ 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			તોડો;

		સ્થિતિ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			તોડો;

		સ્થિતિ 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			તોડો;

		સ્થિતિ 't':  /* --test */
			test = true;
			તોડો;

		સ્થિતિ 'r':  /* --chroot /new/root */
			ch_મૂળ = optarg;
			તોડો;

		સ્થિતિ 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			તોડો;
		સ્થિતિ 'w':
			જો (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			તોડો;
		સ્થિતિ 'x':  /* --exec <executable> */
			exec = optarg;
			તોડો;

		સ્થિતિ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			તોડો;

		સ્થિતિ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			તોડો;

		સ્થિતિ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			તોડો;

		સ્થિતિ '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			તોડો;

		સ્થિતિ '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			તોડો;

		સ્થિતિ LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			તોડો;

		સ્થિતિ LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			તોડો;

		સ્થિતિ LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			તોડો;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	જો (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	જો (!exec)
		exec = startas;
	નહીંતર જો (!name)
		name = startas;

	જો (!exec) {
		exec = *argv;
		જો (!exec)
			exec = name;
		જો (name && start)
			*argv = name;
	} નહીંતર જો (name) {
		*--argv = name;
		++argc;
	} નહીંતર જો (exec) {
		*--argv = exec;
		++argc;
	};

	જો (stop || sig != -1) {
		જો (sig == -1)
			sig = SIGTERM;
		જો (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		જો (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		જો (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		જો (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		જો (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		જો (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} નહીંતર {
		જો (!exec)
			eerrorx("%s: nothing to start", applet);
		જો (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		જો ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		જો ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		જો (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		જો (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	જો (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	જો (ch_મૂળ && *ch_મૂળ == '~')
		ch_મૂળ = expand_home(home, ch_મૂળ);
	જો (exec) {
		જો (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		જો (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			જો (ch_મૂળ)
				xasprintf(&exec_file, "%s/%s", ch_મૂળ, exec);
			નહીંતર
				xasprintf(&exec_file, "%s", exec);
		} નહીંતર {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ખાલી;
			જ્યારે ((token = strsep(&p, ":"))) {
				જો (ch_મૂળ)
					xasprintf(&exec_file, "%s/%s/%s", ch_મૂળ, token, exec);
				નહીંતર
					xasprintf(&exec_file, "%s/%s", token, exec);
				જો (exec_file && access(exec_file, F_OK) == 0)
					તોડો;
				મુક્ત(exec_file);
				exec_file = ખાલી;
			}
			મુક્ત(tmp);
		}
	}
	જો (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		મુક્ત(exec_file);
		exit(EXIT_FAILURE);
	}
	જો (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	જો (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		જો (fp) {
			લીટી = ખાલી;
			જો (xgetline(&લીટી, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = લીટી;
			fclose(fp);
			જો (p != ખાલી && લીટી[0] == '#' && લીટી[1] == '!') {
				p = લીટી + 2;
				/* Strip leading spaces */
				જ્યારે (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				મુક્ત(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				માટે (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(કદ(અક્ષર *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				જો (p)
					nav[len++] = p;
				માટે (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ખાલી;
			}
		}
	}
	margv = nav ? nav : argv;

	જો (stop || sig != -1) {
		જો (sig == -1)
			sig = SIGTERM;
		જો (!stop)
			oknodo = true;
		જો (retry)
			parse_schedule(applet, retry, sig);
		નહીંતર જો (test || oknodo)
			parse_schedule(applet, "0", sig);
		નહીંતર
			parse_schedule(applet, ખાલી, sig);
		જો (pidfile) {
			pid = get_pid(applet, pidfile);
			જો (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} નહીંતર {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (અચલ અક્ષર *અચલ *)margv,
		    pid, uid, stopgroup, test, progress, false);

		જો (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		જો (test || oknodo)
			પાછા i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		જો (pidfile)
			unlink(pidfile);
		જો (svcname)
			rc_service_daemon_set(svcname, exec,
			    (અચલ અક્ષર *અચલ *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	જો (pidfile)
		pid = get_pid(applet, pidfile);
	નહીંતર
		pid = 0;

	જો (pid)
		pids = rc_find_pids(ખાલી, ખાલી, 0, pid);
	નહીંતર
		pids = rc_find_pids(exec, (અચલ અક્ષર * અચલ *) argv, uid, 0);
	જો (pids)
		eerrorx("%s: %s is already running", applet, exec);

	મુક્ત(pids);
	જો (test) {
		જો (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		જ્યારે (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		જો (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		જો (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		જો (ch_મૂળ)
			einfo("in root `%s'", ch_મૂળ);
		જો (ch_dir)
			einfo("in dir `%s'", ch_dir);
		જો (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		જો (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	જો (pidfile)
		unlink(pidfile);

	જો (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	જો (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	જો ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	જો (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		પૂર્ણાંક group_ગણતરી = ARRAY_SIZE(group_buf);
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

		જો (nicelevel != INT_MIN) {
			જો (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			જો (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} નહીંતર જો (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		જો (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		જો (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			જો (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		જો (changeuser != ખાલી) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			જો (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			જો (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			જો (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		જો (changeuser && getgrouplist(changeuser, gid, group_list, &group_ગણતરી) < 0) {
			group_list = xmalloc(group_ગણતરી * કદ(*group_list));
			જો (getgrouplist(changeuser, gid, group_list, &group_ગણતરી) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		જો (ch_મૂળ && chroot(ch_મૂળ) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_મૂળ, strerror(errno));

		જો (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		જો (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			જો (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		જો (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		જો (changeuser && setgroups(group_ગણતરી, group_list))
			eerrorx("%s: setgroups() failed", applet);
		જો (group_list != group_buf)
			મુક્ત(group_list);
#ifdef __linux__
		જો (uid && cap_setuid(uid))
#નહીંતર
		જો (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		જો (cap_iab != ખાલી) {
			i = cap_iab_set_proc(cap_iab);

			જો (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			જો (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		જો (secbits != 0) {
			જો (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		જો (no_new_privs) {
			જો (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		જ્યારે (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		જો (changeuser != ખાલી) {
			pamenv = (અચલ અક્ષર *અચલ *)pam_getenvlist(pamh);
			જો (pamenv) {
				જ્યારે (*pamenv) {
					/* Don't add strings unless they set a var */
					જો (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					નહીંતર
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			જો (strncmp(env->કિંમત, "RC_", 3) == 0
					|| strncmp(env->કિંમત, "SSD_", 4) == 0
					|| strncmp(env->કિંમત, "EINFO_", 6) == 0) {
				*strchr(env->કિંમત, '=') = '\0';
				unsetenv(env->કિંમત);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		જો ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			જ્યારે (token && *token) {
				p = strchr(token, ':');
				જો (p) {
					*p++ = '\0';
					જ્યારે (*p == ':')
						p++;
				}
				જો (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					જો (np != newpath)
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
		જો (redirect_stdin) {
			જો ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		જો (redirect_stdout) {
			જો ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}નહીંતર જો (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			જો (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		જો (redirect_stderr) {
			જો ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}નહીંતર જો (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			જો (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		જો (background)
			dup2(stdin_fd, STDIN_FILENO);
		જો (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		જો (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		જો (notify.type == NOTIFY_FD) {
			જો (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			જો (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			જો (notify.pipe[1] == notify.fd) {
				પૂર્ણાંક flags = fcntl(notify.fd, F_GETFD, 0);
				જો (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				જો (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		જો (scheduler != ખાલી) {
			પૂર્ણાંક scheduler_index;
			સંરચના sched_param sched =  {.sched_priority = sched_prio};
			જો (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			નહીંતર જો (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			નહીંતર જો (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			નહીંતર જો (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			નહીંતર જો (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			નહીંતર જો (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			જો (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			જો (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} નહીંતર જો (sched_prio != -1) {
			અચલ સંરચના sched_param sched =  {.sched_priority = sched_prio};
			જો (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		જો (changeuser != ખાલી && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	જ્યારે ((ss = પઠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	જો (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	જો (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		કરો {
			pid = waitpid(spid, &i, 0);
			જો (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				પાછા -1;
			}
		} જ્યારે (!WIFEXITED(i) && !WIFSIGNALED(i));
		જો (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	જો (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_કિંમત("rc_start_wait"))))
	{
		જો (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	જો (notify.type != NOTIFY_NONE) {
		જો (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} નહીંતર જો (start_wait > 0) {
		સંરચના timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		જો (nanosleep(&ts, ખાલી) == -1) {
			જો (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				પાછા 0;
			}
		}
		જો (background) {
			જો (kill(pid, 0) == 0)
				alive = true;
		} નહીંતર {
			જો (pidfile) {
				pid = get_pid(applet, pidfile);
				જો (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} નહીંતર
				pid = 0;
			જો (do_stop(applet, exec, (અચલ અક્ષર *અચલ *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		જો (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	જો (svcname)
		rc_service_daemon_set(svcname, exec,
		    (અચલ અક્ષર *અચલ *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
