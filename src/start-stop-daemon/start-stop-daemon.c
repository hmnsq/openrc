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

#എങ്കിൽ defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <grp.h>
#അന്തര്ഭാവ <pthread.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <sched.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/resource.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#അന്തര്ഭാവ <termios.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
സ്ഥിതം ഘടന pam_conv conv = { ശൂന്യ, ശൂന്യ};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "schedules.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
എണ്ണൽ {
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

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "capabilities", 1, ശൂന്യ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ശൂന്യ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ശൂന്യ, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ശൂന്യ, 'I'},
	{ "stop",         0, ശൂന്യ, 'K'},
	{ "nicelevel",    1, ശൂന്യ, 'N'},
	{ "oom-score-adj",1, ശൂന്യ, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ശൂന്യ, 'R'},
	{ "start",        0, ശൂന്യ, 'S'},
	{ "startas",      1, ശൂന്യ, 'a'},
	{ "background",   0, ശൂന്യ, 'b'},
	{ "chuid",        1, ശൂന്യ, 'c'},
	{ "chdir",        1, ശൂന്യ, 'd'},
	{ "env",          1, ശൂന്യ, 'e'},
	{ "umask",        1, ശൂന്യ, 'k'},
	{ "group",        1, ശൂന്യ, 'g'},
	{ "stop-group",   0, ശൂന്യ, 'G'},
	{ "interpreted",  0, ശൂന്യ, 'i'},
	{ "make-pidfile", 0, ശൂന്യ, 'm'},
	{ "name",         1, ശൂന്യ, 'n'},
	{ "oknodo",       0, ശൂന്യ, 'o'},
	{ "pidfile",      1, ശൂന്യ, 'p'},
	{ "signal",       1, ശൂന്യ, 's'},
	{ "test",         0, ശൂന്യ, 't'},
	{ "user",         1, ശൂന്യ, 'u'},
	{ "chroot",       1, ശൂന്യ, 'r'},
	{ "wait",         1, ശൂന്യ, 'w'},
	{ "exec",         1, ശൂന്യ, 'x'},
	{ "stdin",        1, ശൂന്യ, '0'},
	{ "stdout",       1, ശൂന്യ, '1'},
	{ "stderr",       1, ശൂന്യ, '2'},
	{ "stdout-logger",1, ശൂന്യ, '3'},
	{ "stderr-logger",1, ശൂന്യ, '4'},
	{ "progress",     0, ശൂന്യ, 'P'},
	{ "scheduler",    1, ശൂന്യ, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ശൂന്യ, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ശൂന്യ, LONGOPT_NOTIFY},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

സ്ഥിതം അക്ഷരം **nav;

സ്ഥിതം അക്ഷരം *changeuser, *ch_വേര്, *ch_dir;

ബാഹ്യ അക്ഷരം **environ;

#എങ്കിൽ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#എങ്കിൽ !defined(__DragonFly__)
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ ioprio_set(പൂർണ്ണസംഖ്യ which RC_UNUSED,
			     പൂർണ്ണസംഖ്യ who RC_UNUSED,
			     പൂർണ്ണസംഖ്യ ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	തിരികെ syscall(SYS_ioprio_set, which, who, ioprio);
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif
}
#endif

സ്ഥിതം ശൂന്യം
cleanup(ശൂന്യം)
{
	സ്വതന്ത്രം(changeuser);
	സ്വതന്ത്രം(nav);
	free_schedulelist();
}

സ്ഥിതം ശൂന്യം
handle_signal(പൂർണ്ണസംഖ്യ sig)
{
	പൂർണ്ണസംഖ്യ status;
	പൂർണ്ണസംഖ്യ serrno = errno;
	സ്ഥിരം അക്ഷരം *signame = ശൂന്യ;

	തിരഞ്ഞെടുപ്പ് (sig) {
	അവസ്ഥ SIGINT:
		എങ്കിൽ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	അവസ്ഥ SIGTERM:
		എങ്കിൽ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	അവസ്ഥ SIGQUIT:
		എങ്കിൽ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	അവസ്ഥ SIGCHLD:
		വേണ്ടി (;;) {
			എങ്കിൽ (waitpid(-1, &status, WNOHANG) < 0) {
				എങ്കിൽ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				നിർത്തുക;
			}
		}
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

സ്ഥിതം അക്ഷരം *
expand_home(സ്ഥിരം അക്ഷരം *home, സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം *opath, *ppath, *p, *nh;
	ഘടന passwd *pw;

	എങ്കിൽ (!path || *path != '~')
		തിരികെ xstrdup(path);

	opath = ppath = xstrdup(path);
	എങ്കിൽ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		എങ്കിൽ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		എങ്കിൽ (pw) {
			home = pw->pw_dir;
			ppath = p;
			എങ്കിൽ (ppath)
				*ppath = '/';
		} അല്ലെങ്കിൽ
			home = ശൂന്യ;
	} അല്ലെങ്കിൽ
		ppath++;

	എങ്കിൽ (!home) {
	സ്വതന്ത്രം(opath);
		തിരികെ xstrdup(path);
	}
	എങ്കിൽ (!ppath) {
		സ്വതന്ത്രം(opath);
		തിരികെ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	സ്വതന്ത്രം(opath);
	തിരികെ nh;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ devnull_fd = -1;
#ifdef TIOCNOTTY
	പൂർണ്ണസംഖ്യ tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = ശൂന്യ;
	പൂർണ്ണസംഖ്യ pamr;
	സ്ഥിരം അക്ഷരം *സ്ഥിരം *pamenv = ശൂന്യ;
#endif

	പൂർണ്ണസംഖ്യ opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	അക്ഷരം *exec = ശൂന്യ;
	അക്ഷരം *startas = ശൂന്യ;
	അക്ഷരം *name = ശൂന്യ;
	അക്ഷരം *pidfile = ശൂന്യ;
	അക്ഷരം *retry = ശൂന്യ;
	പൂർണ്ണസംഖ്യ sig = -1;
	പൂർണ്ണസംഖ്യ nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	പൂർണ്ണസംഖ്യ oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	അക്ഷരം *home = ശൂന്യ;
	പൂർണ്ണസംഖ്യ tid = 0;
	അക്ഷരം *redirect_stdin = ശൂന്യ;
	അക്ഷരം *redirect_stderr = ശൂന്യ;
	അക്ഷരം *redirect_stdout = ശൂന്യ;
	അക്ഷരം *stderr_process = ശൂന്യ;
	അക്ഷരം *stdout_process = ശൂന്യ;
	പൂർണ്ണസംഖ്യ stdin_fd;
	പൂർണ്ണസംഖ്യ stdout_fd;
	പൂർണ്ണസംഖ്യ stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	അക്ഷരം *tmp, *newpath, *np;
	അക്ഷരം *p;
	അക്ഷരം *token;
	അക്ഷരം *exec_file = ശൂന്യ;
	ഘടന passwd *pw;
	ഘടന group *gr;
	bool stopgroup = false;
	അക്ഷരം *വരി = ശൂന്യ;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	അക്ഷരം **margv;
	unsigned പൂർണ്ണസംഖ്യ start_wait = 0;
	സ്ഥിരം അക്ഷരം *scheduler = ശൂന്യ;
	പൂർണ്ണസംഖ്യ sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = ശൂന്യ;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	പൂർണ്ണസംഖ്യ pipefd[2];
	അക്ഷരം readbuf[1];
	ssize_t ss;
	ഘടന notify notify = {0};
	പൂർണ്ണസംഖ്യ ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	എങ്കിൽ ((tmp = getenv("SSD_NICELEVEL")))
		എങ്കിൽ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	എങ്കിൽ ((tmp = getenv("SSD_IONICELEVEL"))) {
		പൂർണ്ണസംഖ്യ n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		എങ്കിൽ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		എങ്കിൽ (ionicec == 0)
			ioniced = 0;
		അല്ലെങ്കിൽ എങ്കിൽ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	എങ്കിൽ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		എങ്കിൽ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	എങ്കിൽ (home == ശൂന്യ || p == ശൂന്യ) {
		pw = getpwuid(getuid());
		എങ്കിൽ (pw != ശൂന്യ) {
			എങ്കിൽ (p == ശൂന്യ)
				setenv("USER", pw->pw_name, 1);
			എങ്കിൽ (home == ശൂന്യ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (പൂർണ്ണസംഖ്യ *) 0)) != -1)
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			എങ്കിൽ (cap_iab == ശൂന്യ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#അല്ലെങ്കിൽ
			eerrorx("Capabilities support not enabled");
#endif
			നിർത്തുക;

		അവസ്ഥ LONGOPT_SECBITS:
#ifdef __linux__
			എങ്കിൽ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ശൂന്യ;
			secbits = strtoul(optarg, &tmp, 0);
			എങ്കിൽ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#അല്ലെങ്കിൽ
			eerrorx("Capabilities support not enabled");
#endif
			നിർത്തുക;

		അവസ്ഥ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#അല്ലെങ്കിൽ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			നിർത്തുക;

		അവസ്ഥ 'I': /* --ionice */
			എങ്കിൽ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			എങ്കിൽ (ionicec == 0)
				ioniced = 0;
			അല്ലെങ്കിൽ എങ്കിൽ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			നിർത്തുക;

		അവസ്ഥ 'K':  /* --stop */
			stop = true;
			നിർത്തുക;

		അവസ്ഥ 'G':  /* --stop-group */
			stopgroup = true;
			നിർത്തുക;

		അവസ്ഥ 'N':  /* --nice */
			എങ്കിൽ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			നിർത്തുക;

		അവസ്ഥ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			എങ്കിൽ (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			നിർത്തുക;

		അവസ്ഥ 'P':  /* --progress */
			progress = true;
			നിർത്തുക;

		അവസ്ഥ 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			നിർത്തുക;

		അവസ്ഥ 'S':  /* --start */
			start = true;
			നിർത്തുക;

		അവസ്ഥ 'b':  /* --background */
			background = true;
			നിർത്തുക;

		അവസ്ഥ 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		അവസ്ഥ 'u':  /* --user <username>|<uid> */
		{
			അക്ഷരം dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			എങ്കിൽ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			അല്ലെങ്കിൽ
				pw = getpwuid((uid_t)tid);

			എങ്കിൽ (pw == ശൂന്യ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			എങ്കിൽ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			എങ്കിൽ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			എങ്കിൽ (gid == 0)
				gid = pw->pw_gid;

			എങ്കിൽ (p) {
				tmp = strsep (&p, ":");
				എങ്കിൽ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				അല്ലെങ്കിൽ
					gr = getgrgid((gid_t) tid);

				എങ്കിൽ (gr == ശൂന്യ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		നിർത്തുക;

		അവസ്ഥ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			നിർത്തുക;

		അവസ്ഥ 'e': /* --env */
			putenv(optarg);
			നിർത്തുക;

		അവസ്ഥ 'g':  /* --group <group>|<gid> */
			എങ്കിൽ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			അല്ലെങ്കിൽ
				gr = getgrgid((gid_t)tid);
			എങ്കിൽ (gr == ശൂന്യ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			നിർത്തുക;

		അവസ്ഥ 'i': /* --interpreted */
			interpreted = true;
			നിർത്തുക;

		അവസ്ഥ 'k':
			എങ്കിൽ (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			നിർത്തുക;

		അവസ്ഥ 'm':  /* --make-pidfile */
			makepidfile = true;
			നിർത്തുക;

		അവസ്ഥ 'n':  /* --name <process-name> */
			name = optarg;
			നിർത്തുക;

		അവസ്ഥ 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			നിർത്തുക;

		അവസ്ഥ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			നിർത്തുക;

		അവസ്ഥ 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			നിർത്തുക;

		അവസ്ഥ 't':  /* --test */
			test = true;
			നിർത്തുക;

		അവസ്ഥ 'r':  /* --chroot /new/root */
			ch_വേര് = optarg;
			നിർത്തുക;

		അവസ്ഥ 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			നിർത്തുക;
		അവസ്ഥ 'w':
			എങ്കിൽ (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			നിർത്തുക;
		അവസ്ഥ 'x':  /* --exec <executable> */
			exec = optarg;
			നിർത്തുക;

		അവസ്ഥ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			നിർത്തുക;

		അവസ്ഥ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			നിർത്തുക;

		അവസ്ഥ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			നിർത്തുക;

		അവസ്ഥ '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			നിർത്തുക;

		അവസ്ഥ '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			നിർത്തുക;

		അവസ്ഥ LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			നിർത്തുക;

		അവസ്ഥ LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			നിർത്തുക;

		അവസ്ഥ LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			നിർത്തുക;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	എങ്കിൽ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	എങ്കിൽ (!exec)
		exec = startas;
	അല്ലെങ്കിൽ എങ്കിൽ (!name)
		name = startas;

	എങ്കിൽ (!exec) {
		exec = *argv;
		എങ്കിൽ (!exec)
			exec = name;
		എങ്കിൽ (name && start)
			*argv = name;
	} അല്ലെങ്കിൽ എങ്കിൽ (name) {
		*--argv = name;
		++argc;
	} അല്ലെങ്കിൽ എങ്കിൽ (exec) {
		*--argv = exec;
		++argc;
	};

	എങ്കിൽ (stop || sig != -1) {
		എങ്കിൽ (sig == -1)
			sig = SIGTERM;
		എങ്കിൽ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		എങ്കിൽ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		എങ്കിൽ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		എങ്കിൽ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		എങ്കിൽ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		എങ്കിൽ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (!exec)
			eerrorx("%s: nothing to start", applet);
		എങ്കിൽ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		എങ്കിൽ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		എങ്കിൽ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		എങ്കിൽ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		എങ്കിൽ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	എങ്കിൽ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	എങ്കിൽ (ch_വേര് && *ch_വേര് == '~')
		ch_വേര് = expand_home(home, ch_വേര്);
	എങ്കിൽ (exec) {
		എങ്കിൽ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		എങ്കിൽ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			എങ്കിൽ (ch_വേര്)
				xasprintf(&exec_file, "%s/%s", ch_വേര്, exec);
			അല്ലെങ്കിൽ
				xasprintf(&exec_file, "%s", exec);
		} അല്ലെങ്കിൽ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ശൂന്യ;
			ആകുമ്പോൾ ((token = strsep(&p, ":"))) {
				എങ്കിൽ (ch_വേര്)
					xasprintf(&exec_file, "%s/%s/%s", ch_വേര്, token, exec);
				അല്ലെങ്കിൽ
					xasprintf(&exec_file, "%s/%s", token, exec);
				എങ്കിൽ (exec_file && access(exec_file, F_OK) == 0)
					നിർത്തുക;
				സ്വതന്ത്രം(exec_file);
				exec_file = ശൂന്യ;
			}
			സ്വതന്ത്രം(tmp);
		}
	}
	എങ്കിൽ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		സ്വതന്ത്രം(exec_file);
		exit(EXIT_FAILURE);
	}
	എങ്കിൽ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	എങ്കിൽ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		എങ്കിൽ (fp) {
			വരി = ശൂന്യ;
			എങ്കിൽ (xgetline(&വരി, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = വരി;
			fclose(fp);
			എങ്കിൽ (p != ശൂന്യ && വരി[0] == '#' && വരി[1] == '!') {
				p = വരി + 2;
				/* Strip leading spaces */
				ആകുമ്പോൾ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				സ്വതന്ത്രം(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				വേണ്ടി (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(വലുപ്പം(അക്ഷരം *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				എങ്കിൽ (p)
					nav[len++] = p;
				വേണ്ടി (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ശൂന്യ;
			}
		}
	}
	margv = nav ? nav : argv;

	എങ്കിൽ (stop || sig != -1) {
		എങ്കിൽ (sig == -1)
			sig = SIGTERM;
		എങ്കിൽ (!stop)
			oknodo = true;
		എങ്കിൽ (retry)
			parse_schedule(applet, retry, sig);
		അല്ലെങ്കിൽ എങ്കിൽ (test || oknodo)
			parse_schedule(applet, "0", sig);
		അല്ലെങ്കിൽ
			parse_schedule(applet, ശൂന്യ, sig);
		എങ്കിൽ (pidfile) {
			pid = get_pid(applet, pidfile);
			എങ്കിൽ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} അല്ലെങ്കിൽ {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)margv,
		    pid, uid, stopgroup, test, progress, false);

		എങ്കിൽ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		എങ്കിൽ (test || oknodo)
			തിരികെ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		എങ്കിൽ (pidfile)
			unlink(pidfile);
		എങ്കിൽ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	എങ്കിൽ (pidfile)
		pid = get_pid(applet, pidfile);
	അല്ലെങ്കിൽ
		pid = 0;

	എങ്കിൽ (pid)
		pids = rc_find_pids(ശൂന്യ, ശൂന്യ, 0, pid);
	അല്ലെങ്കിൽ
		pids = rc_find_pids(exec, (സ്ഥിരം അക്ഷരം * സ്ഥിരം *) argv, uid, 0);
	എങ്കിൽ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	സ്വതന്ത്രം(pids);
	എങ്കിൽ (test) {
		എങ്കിൽ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		ആകുമ്പോൾ (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		എങ്കിൽ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		എങ്കിൽ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		എങ്കിൽ (ch_വേര്)
			einfo("in root `%s'", ch_വേര്);
		എങ്കിൽ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		എങ്കിൽ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		എങ്കിൽ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	എങ്കിൽ (pidfile)
		unlink(pidfile);

	എങ്കിൽ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	എങ്കിൽ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	എങ്കിൽ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	എങ്കിൽ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		പൂർണ്ണസംഖ്യ group_എണ്ണം = ARRAY_SIZE(group_buf);
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

		എങ്കിൽ (nicelevel != INT_MIN) {
			എങ്കിൽ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			എങ്കിൽ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} അല്ലെങ്കിൽ എങ്കിൽ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		എങ്കിൽ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		എങ്കിൽ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			എങ്കിൽ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		എങ്കിൽ (changeuser != ശൂന്യ) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			എങ്കിൽ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			എങ്കിൽ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			എങ്കിൽ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		എങ്കിൽ (changeuser && getgrouplist(changeuser, gid, group_list, &group_എണ്ണം) < 0) {
			group_list = xmalloc(group_എണ്ണം * വലുപ്പം(*group_list));
			എങ്കിൽ (getgrouplist(changeuser, gid, group_list, &group_എണ്ണം) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		എങ്കിൽ (ch_വേര് && chroot(ch_വേര്) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_വേര്, strerror(errno));

		എങ്കിൽ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		എങ്കിൽ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			എങ്കിൽ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		എങ്കിൽ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		എങ്കിൽ (changeuser && setgroups(group_എണ്ണം, group_list))
			eerrorx("%s: setgroups() failed", applet);
		എങ്കിൽ (group_list != group_buf)
			സ്വതന്ത്രം(group_list);
#ifdef __linux__
		എങ്കിൽ (uid && cap_setuid(uid))
#അല്ലെങ്കിൽ
		എങ്കിൽ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		എങ്കിൽ (cap_iab != ശൂന്യ) {
			i = cap_iab_set_proc(cap_iab);

			എങ്കിൽ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			എങ്കിൽ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		എങ്കിൽ (secbits != 0) {
			എങ്കിൽ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		എങ്കിൽ (no_new_privs) {
			എങ്കിൽ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		ആകുമ്പോൾ (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		എങ്കിൽ (changeuser != ശൂന്യ) {
			pamenv = (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)pam_getenvlist(pamh);
			എങ്കിൽ (pamenv) {
				ആകുമ്പോൾ (*pamenv) {
					/* Don't add strings unless they set a var */
					എങ്കിൽ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					അല്ലെങ്കിൽ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			എങ്കിൽ (strncmp(env->മൂല്യം, "RC_", 3) == 0
					|| strncmp(env->മൂല്യം, "SSD_", 4) == 0
					|| strncmp(env->മൂല്യം, "EINFO_", 6) == 0) {
				*strchr(env->മൂല്യം, '=') = '\0';
				unsetenv(env->മൂല്യം);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		എങ്കിൽ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			ആകുമ്പോൾ (token && *token) {
				p = strchr(token, ':');
				എങ്കിൽ (p) {
					*p++ = '\0';
					ആകുമ്പോൾ (*p == ':')
						p++;
				}
				എങ്കിൽ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					എങ്കിൽ (np != newpath)
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
		എങ്കിൽ (redirect_stdin) {
			എങ്കിൽ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		എങ്കിൽ (redirect_stdout) {
			എങ്കിൽ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}അല്ലെങ്കിൽ എങ്കിൽ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			എങ്കിൽ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		എങ്കിൽ (redirect_stderr) {
			എങ്കിൽ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}അല്ലെങ്കിൽ എങ്കിൽ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			എങ്കിൽ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		എങ്കിൽ (background)
			dup2(stdin_fd, STDIN_FILENO);
		എങ്കിൽ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		എങ്കിൽ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		എങ്കിൽ (notify.type == NOTIFY_FD) {
			എങ്കിൽ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			എങ്കിൽ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			എങ്കിൽ (notify.pipe[1] == notify.fd) {
				പൂർണ്ണസംഖ്യ flags = fcntl(notify.fd, F_GETFD, 0);
				എങ്കിൽ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				എങ്കിൽ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		എങ്കിൽ (scheduler != ശൂന്യ) {
			പൂർണ്ണസംഖ്യ scheduler_index;
			ഘടന sched_param sched =  {.sched_priority = sched_prio};
			എങ്കിൽ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			അല്ലെങ്കിൽ എങ്കിൽ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			അല്ലെങ്കിൽ എങ്കിൽ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			അല്ലെങ്കിൽ എങ്കിൽ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			അല്ലെങ്കിൽ എങ്കിൽ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			അല്ലെങ്കിൽ എങ്കിൽ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			എങ്കിൽ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			എങ്കിൽ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} അല്ലെങ്കിൽ എങ്കിൽ (sched_prio != -1) {
			സ്ഥിരം ഘടന sched_param sched =  {.sched_priority = sched_prio};
			എങ്കിൽ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		എങ്കിൽ (changeuser != ശൂന്യ && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ആകുമ്പോൾ ((ss = പഠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	എങ്കിൽ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	എങ്കിൽ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		ചെയ്യുക {
			pid = waitpid(spid, &i, 0);
			എങ്കിൽ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				തിരികെ -1;
			}
		} ആകുമ്പോൾ (!WIFEXITED(i) && !WIFSIGNALED(i));
		എങ്കിൽ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	എങ്കിൽ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_മൂല്യം("rc_start_wait"))))
	{
		എങ്കിൽ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	എങ്കിൽ (notify.type != NOTIFY_NONE) {
		എങ്കിൽ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} അല്ലെങ്കിൽ എങ്കിൽ (start_wait > 0) {
		ഘടന timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		എങ്കിൽ (nanosleep(&ts, ശൂന്യ) == -1) {
			എങ്കിൽ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				തിരികെ 0;
			}
		}
		എങ്കിൽ (background) {
			എങ്കിൽ (kill(pid, 0) == 0)
				alive = true;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (pidfile) {
				pid = get_pid(applet, pidfile);
				എങ്കിൽ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} അല്ലെങ്കിൽ
				pid = 0;
			എങ്കിൽ (do_stop(applet, exec, (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		എങ്കിൽ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	എങ്കിൽ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
