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

#నిర్వచించు ONE_MS           1000000

#ఒకవేళ defined(__linux__) && !defined(GNU_ఆధారం)
/* For extra SCHED_* defines. */
# define GNU_ఆధారం
#అయితే_ముగింపు

#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <grp.h>
#అన్తర్భావ <pthread.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <sched.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/resource.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <sys/wait.h>
#నిర్వచితమైతే __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#అయితే_ముగింపు
#అన్తర్భావ <termios.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>

#నిర్వచితమైతే HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
స్థిర నిర్మాణం pam_conv conv = { ఖాళీ, ఖాళీ};
#అయితే_ముగింపు

#నిర్వచితమైతే __linux__
# include <sys/capability.h>
#అయితే_ముగింపు

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "schedules.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
గణన {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_పునాది = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SCHEDULER,
  LONGOPT_SCHEDULER_PRIO,
  LONGOPT_SECBITS,
  LONGOPT_NOTIFY,
};

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "capabilities", 1, ఖాళీ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ఖాళీ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ఖాళీ, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ఖాళీ, 'I'},
	{ "stop",         0, ఖాళీ, 'K'},
	{ "nicelevel",    1, ఖాళీ, 'N'},
	{ "oom-score-adj",1, ఖాళీ, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ఖాళీ, 'R'},
	{ "start",        0, ఖాళీ, 'S'},
	{ "startas",      1, ఖాళీ, 'a'},
	{ "background",   0, ఖాళీ, 'b'},
	{ "chuid",        1, ఖాళీ, 'c'},
	{ "chdir",        1, ఖాళీ, 'd'},
	{ "env",          1, ఖాళీ, 'e'},
	{ "umask",        1, ఖాళీ, 'k'},
	{ "group",        1, ఖాళీ, 'g'},
	{ "stop-group",   0, ఖాళీ, 'G'},
	{ "interpreted",  0, ఖాళీ, 'i'},
	{ "make-pidfile", 0, ఖాళీ, 'm'},
	{ "name",         1, ఖాళీ, 'n'},
	{ "oknodo",       0, ఖాళీ, 'o'},
	{ "pidfile",      1, ఖాళీ, 'p'},
	{ "signal",       1, ఖాళీ, 's'},
	{ "test",         0, ఖాళీ, 't'},
	{ "user",         1, ఖాళీ, 'u'},
	{ "chroot",       1, ఖాళీ, 'r'},
	{ "wait",         1, ఖాళీ, 'w'},
	{ "exec",         1, ఖాళీ, 'x'},
	{ "stdin",        1, ఖాళీ, '0'},
	{ "stdout",       1, ఖాళీ, '1'},
	{ "stderr",       1, ఖాళీ, '2'},
	{ "stdout-logger",1, ఖాళీ, '3'},
	{ "stderr-logger",1, ఖాళీ, '4'},
	{ "progress",     0, ఖాళీ, 'P'},
	{ "scheduler",    1, ఖాళీ, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ఖాళీ, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ఖాళీ, LONGOPT_NOTIFY},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

స్థిర అక్షరం **nav;

స్థిర అక్షరం *changeuser, *ch_మూలం, *ch_dir;

బాహ్య అక్షరం **environ;

#ఒకవేళ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#అయితే_ముగింపు
#ఒకవేళ !defined(__DragonFly__)
స్థిర అంతఃవరుస పూర్ణాంకం ioprio_సమితి(పూర్ణాంకం which RC_వాడని,
			     పూర్ణాంకం who RC_వాడని,
			     పూర్ణాంకం ioprio RC_వాడని)
{
#నిర్వచితమైతే SYS_ioprio_set
	తిరిగి syscall(SYS_ioprio_set, which, who, ioprio);
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు
}
#అయితే_ముగింపు

స్థిర శూన్య
cleanup(శూన్య)
{
	విడుదల(changeuser);
	విడుదల(nav);
	free_schedulelist();
}

స్థిర శూన్య
handle_signal(పూర్ణాంకం sig)
{
	పూర్ణాంకం స్థితి_2;
	పూర్ణాంకం serrno = errno;
	స్థిరరాశి అక్షరం *signame = ఖాళీ;

	ఎంపిక (sig) {
	స్థితి SIGINT:
		ఒకవేళ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	స్థితి SIGTERM:
		ఒకవేళ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	స్థితి SIGQUIT:
		ఒకవేళ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	స్థితి SIGCHLD:
		కొరకు (;;) {
			ఒకవేళ (waitpid(-1, &స్థితి_2, WNOHANG) < 0) {
				ఒకవేళ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				ఆపు;
			}
		}
		ఆపు;

	అప్రమేయ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

స్థిర అక్షరం *
expand_home(స్థిరరాశి అక్షరం *home, స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం *opath, *ppath, *p, *nh;
	నిర్మాణం passwd *pw;

	ఒకవేళ (!మార్గం || *మార్గం != '~')
		తిరిగి xstrdup(మార్గం);

	opath = ppath = xstrdup(మార్గం);
	ఒకవేళ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ఒకవేళ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ఒకవేళ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ఒకవేళ (ppath)
				*ppath = '/';
		} లేకపోతే
			home = ఖాళీ;
	} లేకపోతే
		ppath++;

	ఒకవేళ (!home) {
	విడుదల(opath);
		తిరిగి xstrdup(మార్గం);
	}
	ఒకవేళ (!ppath) {
		విడుదల(opath);
		తిరిగి xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	విడుదల(opath);
	తిరిగి nh;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం devnull_fd = -1;
#నిర్వచితమైతే TIOCNOTTY
	పూర్ణాంకం tty_fd = -1;
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_PAM
	pam_handle_t *pamh = ఖాళీ;
	పూర్ణాంకం pamr;
	స్థిరరాశి అక్షరం *స్థిరరాశి *pamenv = ఖాళీ;
#అయితే_ముగింపు

	పూర్ణాంకం opt;
	size_t కొలత = 0;
	bool ప్రారంభం = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	అక్షరం *exec = ఖాళీ;
	అక్షరం *startas = ఖాళీ;
	అక్షరం *name = ఖాళీ;
	అక్షరం *pidfile = ఖాళీ;
	అక్షరం *retry = ఖాళీ;
	పూర్ణాంకం sig = -1;
	పూర్ణాంకం nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	పూర్ణాంకం oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool పురోగతి = false;
	uid_t uid = 0;
	gid_t gid = 0;
	అక్షరం *home = ఖాళీ;
	పూర్ణాంకం tid = 0;
	అక్షరం *redirect_stdin = ఖాళీ;
	అక్షరం *redirect_stderr = ఖాళీ;
	అక్షరం *redirect_stdout = ఖాళీ;
	అక్షరం *stderr_process = ఖాళీ;
	అక్షరం *stdout_process = ఖాళీ;
	పూర్ణాంకం stdin_fd;
	పూర్ణాంకం stdout_fd;
	పూర్ణాంకం stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	పూర్ణాంకం i;
	అక్షరం *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	అక్షరం *tmp, *newpath, *np;
	అక్షరం *p;
	అక్షరం *token;
	అక్షరం *exec_file = ఖాళీ;
	నిర్మాణం passwd *pw;
	నిర్మాణం group *gr;
	bool stopgroup = false;
	అక్షరం *పంక్తి = ఖాళీ;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	అక్షరం **margv;
	గుర్తులేని పూర్ణాంకం ప్రారంభం_wait = 0;
	స్థిరరాశి అక్షరం *scheduler = ఖాళీ;
	పూర్ణాంకం sched_prio = -1;
#నిర్వచితమైతే __linux__
	cap_iab_t cap_iab = ఖాళీ;
	గుర్తులేని secbits = 0;
#అయితే_ముగింపు
#నిర్వచితమైతే PR_సమితి_NO_NEW_PRIVS
	bool no_new_privs = false;
#అయితే_ముగింపు
	పూర్ణాంకం pipefd[2];
	అక్షరం readbuf[1];
	ssize_t ss;
	నిర్మాణం notify notify = {0};
	పూర్ణాంకం ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_అమరిక(SIGINT, handle_signal);
	signal_అమరిక(SIGQUIT, handle_signal);
	signal_అమరిక(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ఒకవేళ ((tmp = getenv("SSD_NICELEVEL")))
		ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ఒకవేళ ((tmp = getenv("SSD_IONICELEVEL"))) {
		పూర్ణాంకం n = అక్షరశ్రేణి_చదువు(tmp, "%d:%d", &ionicec, &ioniced);
		ఒకవేళ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ఒకవేళ (ionicec == 0)
			ioniced = 0;
		లేకపోతే ఒకవేళ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ఒకవేళ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ఒకవేళ (home == ఖాళీ || p == ఖాళీ) {
		pw = getpwuid(getuid());
		ఒకవేళ (pw != ఖాళీ) {
			ఒకవేళ (p == ఖాళీ)
				setenv("USER", pw->pw_name, 1);
			ఒకవేళ (home == ఖాళీ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	అయితే ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (పూర్ణాంకం *) 0)) != -1)
		ఎంపిక (opt) {
		స్థితి LONGOPT_CAPABILITIES:
#నిర్వచితమైతే __linux__
			cap_iab = cap_iab_from_పాఠ్యం(optarg);
			ఒకవేళ (cap_iab == ఖాళీ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#లేకపోతే
			eerrorx("Capabilities support not enabled");
#అయితే_ముగింపు
			ఆపు;

		స్థితి LONGOPT_SECBITS:
#నిర్వచితమైతే __linux__
			ఒకవేళ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ఖాళీ;
			secbits = strtoul(optarg, &tmp, 0);
			ఒకవేళ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#లేకపోతే
			eerrorx("Capabilities support not enabled");
#అయితే_ముగింపు
			ఆపు;

		స్థితి LONGOPT_NO_NEW_PRIVS:
#నిర్వచితమైతే PR_సమితి_NO_NEW_PRIVS
			no_new_privs = true;
#లేకపోతే
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#అయితే_ముగింపు
			ఆపు;

		స్థితి 'I': /* --ionice */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ఒకవేళ (ionicec == 0)
				ioniced = 0;
			లేకపోతే ఒకవేళ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ఆపు;

		స్థితి 'K':  /* --stop */
			stop = true;
			ఆపు;

		స్థితి 'G':  /* --stop-group */
			stopgroup = true;
			ఆపు;

		స్థితి 'N':  /* --nice */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ఆపు;

		స్థితి LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ఆపు;

		స్థితి 'P':  /* --progress */
			పురోగతి = true;
			ఆపు;

		స్థితి 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			ఆపు;

		స్థితి 'S':  /* --start */
			ప్రారంభం = true;
			ఆపు;

		స్థితి 'b':  /* --background */
			background = true;
			ఆపు;

		స్థితి 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		స్థితి 'u':  /* --user <username>|<uid> */
		{
			అక్షరం dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			లేకపోతే
				pw = getpwuid((uid_t)tid);

			ఒకవేళ (pw == ఖాళీ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ఒకవేళ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ఒకవేళ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ఒకవేళ (gid == 0)
				gid = pw->pw_gid;

			ఒకవేళ (p) {
				tmp = strsep (&p, ":");
				ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				లేకపోతే
					gr = getgrgid((gid_t) tid);

				ఒకవేళ (gr == ఖాళీ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ఆపు;

		స్థితి 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ఆపు;

		స్థితి 'e': /* --env */
			putenv(optarg);
			ఆపు;

		స్థితి 'g':  /* --group <group>|<gid> */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			లేకపోతే
				gr = getgrgid((gid_t)tid);
			ఒకవేళ (gr == ఖాళీ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ఆపు;

		స్థితి 'i': /* --interpreted */
			interpreted = true;
			ఆపు;

		స్థితి 'k':
			ఒకవేళ (parse_బహుళకం(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ఆపు;

		స్థితి 'm':  /* --make-pidfile */
			makepidfile = true;
			ఆపు;

		స్థితి 'n':  /* --name <process-name> */
			name = optarg;
			ఆపు;

		స్థితి 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			ఆపు;

		స్థితి 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ఆపు;

		స్థితి 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			ఆపు;

		స్థితి 't':  /* --test */
			test = true;
			ఆపు;

		స్థితి 'r':  /* --chroot /new/root */
			ch_మూలం = optarg;
			ఆపు;

		స్థితి 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			ఆపు;
		స్థితి 'w':
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%u", &ప్రారంభం_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			ఆపు;
		స్థితి 'x':  /* --exec <executable> */
			exec = optarg;
			ఆపు;

		స్థితి '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ఆపు;

		స్థితి '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ఆపు;

		స్థితి '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ఆపు;

		స్థితి '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ఆపు;

		స్థితి '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ఆపు;

		స్థితి LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			ఆపు;

		స్థితి LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			అక్షరశ్రేణి_చదువు(optarg, "%d", &sched_prio);
			ఆపు;

		స్థితి LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			ఆపు;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	ఒకవేళ (!ప్రారంభం &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ఒకవేళ (!exec)
		exec = startas;
	లేకపోతే ఒకవేళ (!name)
		name = startas;

	ఒకవేళ (!exec) {
		exec = *argv;
		ఒకవేళ (!exec)
			exec = name;
		ఒకవేళ (name && ప్రారంభం)
			*argv = name;
	} లేకపోతే ఒకవేళ (name) {
		*--argv = name;
		++argc;
	} లేకపోతే ఒకవేళ (exec) {
		*--argv = exec;
		++argc;
	};

	ఒకవేళ (stop || sig != -1) {
		ఒకవేళ (sig == -1)
			sig = SIGTERM;
		ఒకవేళ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ఒకవేళ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ఒకవేళ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ఒకవేళ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ఒకవేళ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ఒకవేళ (ప్రారంభం_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} లేకపోతే {
		ఒకవేళ (!exec)
			eerrorx("%s: nothing to start", applet);
		ఒకవేళ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ఒకవేళ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ఒకవేళ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ఒకవేళ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ఒకవేళ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ఒకవేళ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ఒకవేళ (ch_మూలం && *ch_మూలం == '~')
		ch_మూలం = expand_home(home, ch_మూలం);
	ఒకవేళ (exec) {
		ఒకవేళ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ఒకవేళ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ఒకవేళ (ch_మూలం)
				xasprintf(&exec_file, "%s/%s", ch_మూలం, exec);
			లేకపోతే
				xasprintf(&exec_file, "%s", exec);
		} లేకపోతే {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ఖాళీ;
			అయితే ((token = strsep(&p, ":"))) {
				ఒకవేళ (ch_మూలం)
					xasprintf(&exec_file, "%s/%s/%s", ch_మూలం, token, exec);
				లేకపోతే
					xasprintf(&exec_file, "%s/%s", token, exec);
				ఒకవేళ (exec_file && access(exec_file, F_OK) == 0)
					ఆపు;
				విడుదల(exec_file);
				exec_file = ఖాళీ;
			}
			విడుదల(tmp);
		}
	}
	ఒకవేళ (ప్రారంభం && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		విడుదల(exec_file);
		నిష్క్రమించు(EXIT_FAILURE);
	}
	ఒకవేళ (ప్రారంభం && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ఒకవేళ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ఒకవేళ (fp) {
			పంక్తి = ఖాళీ;
			ఒకవేళ (xgetline(&పంక్తి, &కొలత, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = పంక్తి;
			fclose(fp);
			ఒకవేళ (p != ఖాళీ && పంక్తి[0] == '#' && పంక్తి[1] == '!') {
				p = పంక్తి + 2;
				/* Strip leading spaces */
				అయితే (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				విడుదల(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				కొరకు (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(పరిమాణం(అక్షరం *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ఒకవేళ (p)
					nav[len++] = p;
				కొరకు (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ఖాళీ;
			}
		}
	}
	margv = nav ? nav : argv;

	ఒకవేళ (stop || sig != -1) {
		ఒకవేళ (sig == -1)
			sig = SIGTERM;
		ఒకవేళ (!stop)
			oknodo = true;
		ఒకవేళ (retry)
			parse_కాలపట్టిక(applet, retry, sig);
		లేకపోతే ఒకవేళ (test || oknodo)
			parse_కాలపట్టిక(applet, "0", sig);
		లేకపోతే
			parse_కాలపట్టిక(applet, ఖాళీ, sig);
		ఒకవేళ (pidfile) {
			pid = get_pid(applet, pidfile);
			ఒకవేళ (pid == -1 && errno != ENOENT)
				నిష్క్రమించు(EXIT_FAILURE);
		} లేకపోతే {
			pid = 0;
		}
		i = నిరంతరం_stop_కాలపట్టిక(applet, exec, (స్థిరరాశి అక్షరం *స్థిరరాశి *)margv,
		    pid, uid, stopgroup, test, పురోగతి, false);

		ఒకవేళ (i < 0)
			/* We failed to stop something */
			నిష్క్రమించు(EXIT_FAILURE);
		ఒకవేళ (test || oknodo)
			తిరిగి i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ఒకవేళ (pidfile)
			unlink(pidfile);
		ఒకవేళ (svcname)
			rc_service_daemon_సమితి(svcname, exec,
			    (స్థిరరాశి అక్షరం *స్థిరరాశి *)argv,
			    pidfile, false);
		నిష్క్రమించు(EXIT_SUCCESS);
	}

	ఒకవేళ (pidfile)
		pid = get_pid(applet, pidfile);
	లేకపోతే
		pid = 0;

	ఒకవేళ (pid)
		pids = rc_కనుగొను_pids(ఖాళీ, ఖాళీ, 0, pid);
	లేకపోతే
		pids = rc_కనుగొను_pids(exec, (స్థిరరాశి అక్షరం * స్థిరరాశి *) argv, uid, 0);
	ఒకవేళ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	విడుదల(pids);
	ఒకవేళ (test) {
		ఒకవేళ (rc_yesno(getenv("EINFO_QUIET")))
			నిష్క్రమించు (EXIT_SUCCESS);

		einfon("Would start");
		అయితే (argc-- > 0)
			రాయి(" %s", *argv++);
		రాయి("\n");
		eindent();
		ఒకవేళ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ఒకవేళ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ఒకవేళ (ch_మూలం)
			einfo("in root `%s'", ch_మూలం);
		ఒకవేళ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ఒకవేళ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ఒకవేళ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		నిష్క్రమించు(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ఒకవేళ (pidfile)
		unlink(pidfile);

	ఒకవేళ (background)
		signal_అమరిక(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ఒకవేళ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ఒకవేళ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ఒకవేళ (pid == 0) {
		gid_t సమూహం_buf[32], *సమూహం_list = సమూహం_buf;
		పూర్ణాంకం సమూహం_లెక్క = ARRAY_కొలత(సమూహం_buf);
		pid_t mypid = getpid();
		close(pipefd[0]); /* Close the read end of the pipe. */
		umask(numask);

#నిర్వచితమైతే TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#అయితే_ముగింపు

		devnull_fd = open("/dev/null", O_RDWR);

		/* Must call setsid() before setting autogroup nicelevel
		 * but after opening tty_fd. */
		setsid();

		ఒకవేళ (nicelevel != INT_MIN) {
			ఒకవేళ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ఒకవేళ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} లేకపోతే ఒకవేళ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ఒకవేళ (ionicec != -1 &&
		    ioprio_సమితి(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ఒకవేళ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ఒకవేళ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#నిర్వచితమైతే HAVE_PAM
		ఒకవేళ (changeuser != ఖాళీ) {
			pamr = pam_ప్రారంభం("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ఒకవేళ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ఒకవేళ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ఒకవేళ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#అయితే_ముగింపు
		ఒకవేళ (changeuser && getgrouplist(changeuser, gid, సమూహం_list, &సమూహం_లెక్క) < 0) {
			సమూహం_list = xmalloc(సమూహం_లెక్క * పరిమాణం(*సమూహం_list));
			ఒకవేళ (getgrouplist(changeuser, gid, సమూహం_list, &సమూహం_లెక్క) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ఒకవేళ (ch_మూలం && chroot(ch_మూలం) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_మూలం, strerror(errno));

		ఒకవేళ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ఒకవేళ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ఒకవేళ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ఒకవేళ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ఒకవేళ (changeuser && setgroups(సమూహం_లెక్క, సమూహం_list))
			eerrorx("%s: setgroups() failed", applet);
		ఒకవేళ (సమూహం_list != సమూహం_buf)
			విడుదల(సమూహం_list);
#నిర్వచితమైతే __linux__
		ఒకవేళ (uid && cap_setuid(uid))
#లేకపోతే
		ఒకవేళ (uid && setuid(uid))
#అయితే_ముగింపు
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#నిర్వచితమైతే __linux__
		ఒకవేళ (cap_iab != ఖాళీ) {
			i = cap_iab_సమితి_proc(cap_iab);

			ఒకవేళ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ఒకవేళ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ఒకవేళ (secbits != 0) {
			ఒకవేళ (cap_సమితి_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#అయితే_ముగింపు

#నిర్వచితమైతే PR_సమితి_NO_NEW_PRIVS
		ఒకవేళ (no_new_privs) {
			ఒకవేళ (prctl(PR_సమితి_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
				eerrorx("Could not set No New Privs flag: %s", strerror(errno));
		}
#అయితే_ముగింపు


#నిర్వచితమైతే TIOCNOTTY
		ioctl(tty_fd, TIOCNOTTY, 0);
		close(tty_fd);
#అయితే_ముగింపు

		/* Clean the environment of any RC_ variables */
		env_list = rc_stringlist_new();
		i = 0;
		అయితే (environ[i])
			rc_stringlist_కూడు(env_list, environ[i++]);

#నిర్వచితమైతే HAVE_PAM
		ఒకవేళ (changeuser != ఖాళీ) {
			pamenv = (స్థిరరాశి అక్షరం *స్థిరరాశి *)pam_getenvlist(pamh);
			ఒకవేళ (pamenv) {
				అయితే (*pamenv) {
					/* Don't add strings unless they set a var */
					ఒకవేళ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					లేకపోతే
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#అయితే_ముగింపు

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ఒకవేళ (అక్షరశ్రేణి_పోలిక(env->విలువ, "RC_", 3) == 0
					|| అక్షరశ్రేణి_పోలిక(env->విలువ, "SSD_", 4) == 0
					|| అక్షరశ్రేణి_పోలిక(env->విలువ, "EINFO_", 6) == 0) {
				*strchr(env->విలువ, '=') = '\0';
				unsetenv(env->విలువ);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ఒకవేళ ((token = getenv("PATH"))) {
			len = అక్షరశ్రేణి_పొడవు(token);
			newpath = np = xmalloc(len + 1);
			అయితే (token && *token) {
				p = strchr(token, ':');
				ఒకవేళ (p) {
					*p++ = '\0';
					అయితే (*p == ':')
						p++;
				}
				ఒకవేళ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = అక్షరశ్రేణి_పొడవు(token);
					ఒకవేళ (np != newpath)
						*np++ = ':';
					జ్ఞాపకం_నకలు(np, token, len);
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
		ఒకవేళ (redirect_stdin) {
			ఒకవేళ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ఒకవేళ (redirect_stdout) {
			ఒకవేళ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}లేకపోతే ఒకవేళ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ఒకవేళ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ఒకవేళ (redirect_stderr) {
			ఒకవేళ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}లేకపోతే ఒకవేళ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ఒకవేళ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ఒకవేళ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ఒకవేళ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ఒకవేళ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ఒకవేళ (notify.type == NOTIFY_FD) {
			ఒకవేళ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ఒకవేళ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ఒకవేళ (notify.pipe[1] == notify.fd) {
				పూర్ణాంకం flags = fcntl(notify.fd, F_GETFD, 0);
				ఒకవేళ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ఒకవేళ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ఒకవేళ (scheduler != ఖాళీ) {
			పూర్ణాంకం scheduler_సూచీ;
			నిర్మాణం sched_param sched =  {.sched_priority = sched_prio};
			ఒకవేళ (strcmp(scheduler, "fifo") == 0)
				scheduler_సూచీ = SCHED_FIFO;
			లేకపోతే ఒకవేళ (strcmp(scheduler, "rr") == 0)
				scheduler_సూచీ = SCHED_RR;
			లేకపోతే ఒకవేళ (strcmp(scheduler, "other") == 0)
				scheduler_సూచీ = SCHED_OTHER;
#నిర్వచితమైతే SCHED_BATCH
			లేకపోతే ఒకవేళ (strcmp(scheduler, "batch") == 0)
				scheduler_సూచీ = SCHED_BATCH;
#అయితే_ముగింపు
#నిర్వచితమైతే SCHED_IDLE
			లేకపోతే ఒకవేళ (strcmp(scheduler, "idle") == 0)
				scheduler_సూచీ = SCHED_IDLE;
#అయితే_ముగింపు
			లేకపోతే ఒకవేళ (అక్షరశ్రేణి_చదువు(scheduler, "%d", &scheduler_సూచీ) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ఒకవేళ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_సూచీ);

			ఒకవేళ (pthread_setschedparam(pthread_self(), scheduler_సూచీ, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} లేకపోతే ఒకవేళ (sched_prio != -1) {
			స్థిరరాశి నిర్మాణం sched_param sched =  {.sched_priority = sched_prio};
			ఒకవేళ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#నిర్వచితమైతే HAVE_PAM
		ఒకవేళ (changeuser != ఖాళీ && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#అయితే_ముగింపు
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	అయితే ((ss = పఠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ఒకవేళ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ఒకవేళ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		చేయి {
			pid = waitpid(spid, &i, 0);
			ఒకవేళ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				తిరిగి -1;
			}
		} అయితే (!WIFEXITED(i) && !WIFSIGNALED(i));
		ఒకవేళ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			నిష్క్రమించు(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ఒకవేళ (ప్రారంభం_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_విలువ("rc_start_wait"))))
	{
		ఒకవేళ (అక్షరశ్రేణి_చదువు(p, "%u", &ప్రారంభం_wait) != 1)
			ప్రారంభం_wait = 0;
	}

	ఒకవేళ (notify.type != NOTIFY_NONE) {
		ఒకవేళ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} లేకపోతే ఒకవేళ (ప్రారంభం_wait > 0) {
		నిర్మాణం timespec ts;
		bool alive = false;

		ts.tv_sec = ప్రారంభం_wait / 1000;
		ts.tv_nsec = (ప్రారంభం_wait % 1000) * ONE_MS;
		ఒకవేళ (nanosleep(&ts, ఖాళీ) == -1) {
			ఒకవేళ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				తిరిగి 0;
			}
		}
		ఒకవేళ (background) {
			ఒకవేళ (kill(pid, 0) == 0)
				alive = true;
		} లేకపోతే {
			ఒకవేళ (pidfile) {
				pid = get_pid(applet, pidfile);
				ఒకవేళ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} లేకపోతే
				pid = 0;
			ఒకవేళ (do_stop(applet, exec, (స్థిరరాశి అక్షరం *స్థిరరాశి *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ఒకవేళ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ఒకవేళ (svcname)
		rc_service_daemon_సమితి(svcname, exec,
		    (స్థిరరాశి అక్షరం *స్థిరరాశి *)margv, pidfile, true);

	నిష్క్రమించు(ret);
	/* NOTREACHED */
}
