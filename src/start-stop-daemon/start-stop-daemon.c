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

#जइ defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <grp.h>
#अन्तर्भाव <pthread.h>
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <sched.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/resource.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#अन्तर्भाव <termios.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
स्थिर संरचना pam_conv conv = { रिक्त, रिक्त};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "schedules.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
गणना {
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

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "capabilities", 1, रिक्त, LONGOPT_CAPABILITIES},
	{ "secbits",      1, रिक्त, LONGOPT_SECBITS},
	{ "no-new-privs", 0, रिक्त, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, रिक्त, 'I'},
	{ "stop",         0, रिक्त, 'K'},
	{ "nicelevel",    1, रिक्त, 'N'},
	{ "oom-score-adj",1, रिक्त, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, रिक्त, 'R'},
	{ "start",        0, रिक्त, 'S'},
	{ "startas",      1, रिक्त, 'a'},
	{ "background",   0, रिक्त, 'b'},
	{ "chuid",        1, रिक्त, 'c'},
	{ "chdir",        1, रिक्त, 'd'},
	{ "env",          1, रिक्त, 'e'},
	{ "umask",        1, रिक्त, 'k'},
	{ "group",        1, रिक्त, 'g'},
	{ "stop-group",   0, रिक्त, 'G'},
	{ "interpreted",  0, रिक्त, 'i'},
	{ "make-pidfile", 0, रिक्त, 'm'},
	{ "name",         1, रिक्त, 'n'},
	{ "oknodo",       0, रिक्त, 'o'},
	{ "pidfile",      1, रिक्त, 'p'},
	{ "signal",       1, रिक्त, 's'},
	{ "test",         0, रिक्त, 't'},
	{ "user",         1, रिक्त, 'u'},
	{ "chroot",       1, रिक्त, 'r'},
	{ "wait",         1, रिक्त, 'w'},
	{ "exec",         1, रिक्त, 'x'},
	{ "stdin",        1, रिक्त, '0'},
	{ "stdout",       1, रिक्त, '1'},
	{ "stderr",       1, रिक्त, '2'},
	{ "stdout-logger",1, रिक्त, '3'},
	{ "stderr-logger",1, रिक्त, '4'},
	{ "progress",     0, रिक्त, 'P'},
	{ "scheduler",    1, रिक्त, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, रिक्त, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, रिक्त, LONGOPT_NOTIFY},
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
ध्रुव वर्ण *usagestring = रिक्त;

स्थिर वर्ण **nav;

स्थिर वर्ण *changeuser, *ch_मूलम्, *ch_dir;

बाह्य वर्ण **environ;

#जइ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#जइ !defined(__DragonFly__)
स्थिर अन्तःस्थ पूर्णाङ्क ioprio_set(पूर्णाङ्क which RC_UNUSED,
			     पूर्णाङ्क who RC_UNUSED,
			     पूर्णाङ्क ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	प्रत्यागम syscall(SYS_ioprio_set, which, who, ioprio);
#अण्णहा
	प्रत्यागम 0;
#endif
}
#endif

स्थिर शून्य
cleanup(शून्य)
{
	मुक्त(changeuser);
	मुक्त(nav);
	free_schedulelist();
}

स्थिर शून्य
handle_signal(पूर्णाङ्क sig)
{
	पूर्णाङ्क status;
	पूर्णाङ्क serrno = errno;
	ध्रुव वर्ण *signame = रिक्त;

	विकल्प (sig) {
	पक्ष SIGINT:
		जइ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	पक्ष SIGTERM:
		जइ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	पक्ष SIGQUIT:
		जइ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	पक्ष SIGCHLD:
		प्रति (;;) {
			जइ (waitpid(-1, &status, WNOHANG) < 0) {
				जइ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				भङ्ग;
			}
		}
		भङ्ग;

	स्वभाव:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

स्थिर वर्ण *
expand_home(ध्रुव वर्ण *home, ध्रुव वर्ण *path)
{
	वर्ण *opath, *ppath, *p, *nh;
	संरचना passwd *pw;

	जइ (!path || *path != '~')
		प्रत्यागम xstrdup(path);

	opath = ppath = xstrdup(path);
	जइ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		जइ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		जइ (pw) {
			home = pw->pw_dir;
			ppath = p;
			जइ (ppath)
				*ppath = '/';
		} अण्णहा
			home = रिक्त;
	} अण्णहा
		ppath++;

	जइ (!home) {
	मुक्त(opath);
		प्रत्यागम xstrdup(path);
	}
	जइ (!ppath) {
		मुक्त(opath);
		प्रत्यागम xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	मुक्त(opath);
	प्रत्यागम nh;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क devnull_fd = -1;
#ifdef TIOCNOTTY
	पूर्णाङ्क tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = रिक्त;
	पूर्णाङ्क pamr;
	ध्रुव वर्ण *ध्रुव *pamenv = रिक्त;
#endif

	पूर्णाङ्क opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	वर्ण *exec = रिक्त;
	वर्ण *startas = रिक्त;
	वर्ण *name = रिक्त;
	वर्ण *pidfile = रिक्त;
	वर्ण *retry = रिक्त;
	पूर्णाङ्क sig = -1;
	पूर्णाङ्क nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	पूर्णाङ्क oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	वर्ण *home = रिक्त;
	पूर्णाङ्क tid = 0;
	वर्ण *redirect_stdin = रिक्त;
	वर्ण *redirect_stderr = रिक्त;
	वर्ण *redirect_stdout = रिक्त;
	वर्ण *stderr_process = रिक्त;
	वर्ण *stdout_process = रिक्त;
	पूर्णाङ्क stdin_fd;
	पूर्णाङ्क stdout_fd;
	पूर्णाङ्क stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	पूर्णाङ्क i;
	वर्ण *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	वर्ण *tmp, *newpath, *np;
	वर्ण *p;
	वर्ण *token;
	वर्ण *exec_file = रिक्त;
	संरचना passwd *pw;
	संरचना group *gr;
	bool stopgroup = false;
	वर्ण *पङ्क्तिः = रिक्त;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	वर्ण **margv;
	unsigned पूर्णाङ्क start_wait = 0;
	ध्रुव वर्ण *scheduler = रिक्त;
	पूर्णाङ्क sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = रिक्त;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	पूर्णाङ्क pipefd[2];
	वर्ण readbuf[1];
	ssize_t ss;
	संरचना notify notify = {0};
	पूर्णाङ्क ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	जइ ((tmp = getenv("SSD_NICELEVEL")))
		जइ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	जइ ((tmp = getenv("SSD_IONICELEVEL"))) {
		पूर्णाङ्क n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		जइ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		जइ (ionicec == 0)
			ioniced = 0;
		अण्णहा जइ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	जइ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		जइ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	जइ (home == रिक्त || p == रिक्त) {
		pw = getpwuid(getuid());
		जइ (pw != रिक्त) {
			जइ (p == रिक्त)
				setenv("USER", pw->pw_name, 1);
			जइ (home == रिक्त) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	जाव ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (पूर्णाङ्क *) 0)) != -1)
		विकल्प (opt) {
		पक्ष LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			जइ (cap_iab == रिक्त)
				eerrorx("Could not parse iab: %s", strerror(errno));
#अण्णहा
			eerrorx("Capabilities support not enabled");
#endif
			भङ्ग;

		पक्ष LONGOPT_SECBITS:
#ifdef __linux__
			जइ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = रिक्त;
			secbits = strtoul(optarg, &tmp, 0);
			जइ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#अण्णहा
			eerrorx("Capabilities support not enabled");
#endif
			भङ्ग;

		पक्ष LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#अण्णहा
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			भङ्ग;

		पक्ष 'I': /* --ionice */
			जइ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			जइ (ionicec == 0)
				ioniced = 0;
			अण्णहा जइ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			भङ्ग;

		पक्ष 'K':  /* --stop */
			stop = true;
			भङ्ग;

		पक्ष 'G':  /* --stop-group */
			stopgroup = true;
			भङ्ग;

		पक्ष 'N':  /* --nice */
			जइ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			भङ्ग;

		पक्ष LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			जइ (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			भङ्ग;

		पक्ष 'P':  /* --progress */
			progress = true;
			भङ्ग;

		पक्ष 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			भङ्ग;

		पक्ष 'S':  /* --start */
			start = true;
			भङ्ग;

		पक्ष 'b':  /* --background */
			background = true;
			भङ्ग;

		पक्ष 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		पक्ष 'u':  /* --user <username>|<uid> */
		{
			वर्ण dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			जइ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			अण्णहा
				pw = getpwuid((uid_t)tid);

			जइ (pw == रिक्त)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			जइ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			जइ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			जइ (gid == 0)
				gid = pw->pw_gid;

			जइ (p) {
				tmp = strsep (&p, ":");
				जइ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				अण्णहा
					gr = getgrgid((gid_t) tid);

				जइ (gr == रिक्त)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		भङ्ग;

		पक्ष 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			भङ्ग;

		पक्ष 'e': /* --env */
			putenv(optarg);
			भङ्ग;

		पक्ष 'g':  /* --group <group>|<gid> */
			जइ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			अण्णहा
				gr = getgrgid((gid_t)tid);
			जइ (gr == रिक्त)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			भङ्ग;

		पक्ष 'i': /* --interpreted */
			interpreted = true;
			भङ्ग;

		पक्ष 'k':
			जइ (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			भङ्ग;

		पक्ष 'm':  /* --make-pidfile */
			makepidfile = true;
			भङ्ग;

		पक्ष 'n':  /* --name <process-name> */
			name = optarg;
			भङ्ग;

		पक्ष 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			भङ्ग;

		पक्ष 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			भङ्ग;

		पक्ष 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			भङ्ग;

		पक्ष 't':  /* --test */
			test = true;
			भङ्ग;

		पक्ष 'r':  /* --chroot /new/root */
			ch_मूलम् = optarg;
			भङ्ग;

		पक्ष 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			भङ्ग;
		पक्ष 'w':
			जइ (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			भङ्ग;
		पक्ष 'x':  /* --exec <executable> */
			exec = optarg;
			भङ्ग;

		पक्ष '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			भङ्ग;

		पक्ष '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			भङ्ग;

		पक्ष '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			भङ्ग;

		पक्ष '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			भङ्ग;

		पक्ष '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			भङ्ग;

		पक्ष LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			भङ्ग;

		पक्ष LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			भङ्ग;

		पक्ष LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			भङ्ग;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	जइ (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	जइ (!exec)
		exec = startas;
	अण्णहा जइ (!name)
		name = startas;

	जइ (!exec) {
		exec = *argv;
		जइ (!exec)
			exec = name;
		जइ (name && start)
			*argv = name;
	} अण्णहा जइ (name) {
		*--argv = name;
		++argc;
	} अण्णहा जइ (exec) {
		*--argv = exec;
		++argc;
	};

	जइ (stop || sig != -1) {
		जइ (sig == -1)
			sig = SIGTERM;
		जइ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		जइ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		जइ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		जइ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		जइ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		जइ (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} अण्णहा {
		जइ (!exec)
			eerrorx("%s: nothing to start", applet);
		जइ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		जइ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		जइ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		जइ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		जइ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	जइ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	जइ (ch_मूलम् && *ch_मूलम् == '~')
		ch_मूलम् = expand_home(home, ch_मूलम्);
	जइ (exec) {
		जइ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		जइ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			जइ (ch_मूलम्)
				xasprintf(&exec_file, "%s/%s", ch_मूलम्, exec);
			अण्णहा
				xasprintf(&exec_file, "%s", exec);
		} अण्णहा {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = रिक्त;
			जाव ((token = strsep(&p, ":"))) {
				जइ (ch_मूलम्)
					xasprintf(&exec_file, "%s/%s/%s", ch_मूलम्, token, exec);
				अण्णहा
					xasprintf(&exec_file, "%s/%s", token, exec);
				जइ (exec_file && access(exec_file, F_OK) == 0)
					भङ्ग;
				मुक्त(exec_file);
				exec_file = रिक्त;
			}
			मुक्त(tmp);
		}
	}
	जइ (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		मुक्त(exec_file);
		exit(EXIT_FAILURE);
	}
	जइ (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	जइ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		जइ (fp) {
			पङ्क्तिः = रिक्त;
			जइ (xgetline(&पङ्क्तिः, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = पङ्क्तिः;
			fclose(fp);
			जइ (p != रिक्त && पङ्क्तिः[0] == '#' && पङ्क्तिः[1] == '!') {
				p = पङ्क्तिः + 2;
				/* Strip leading spaces */
				जाव (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				मुक्त(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				प्रति (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(परिमाण(वर्ण *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				जइ (p)
					nav[len++] = p;
				प्रति (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = रिक्त;
			}
		}
	}
	margv = nav ? nav : argv;

	जइ (stop || sig != -1) {
		जइ (sig == -1)
			sig = SIGTERM;
		जइ (!stop)
			oknodo = true;
		जइ (retry)
			parse_schedule(applet, retry, sig);
		अण्णहा जइ (test || oknodo)
			parse_schedule(applet, "0", sig);
		अण्णहा
			parse_schedule(applet, रिक्त, sig);
		जइ (pidfile) {
			pid = get_pid(applet, pidfile);
			जइ (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} अण्णहा {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (ध्रुव वर्ण *ध्रुव *)margv,
		    pid, uid, stopgroup, test, progress, false);

		जइ (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		जइ (test || oknodo)
			प्रत्यागम i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		जइ (pidfile)
			unlink(pidfile);
		जइ (svcname)
			rc_service_daemon_set(svcname, exec,
			    (ध्रुव वर्ण *ध्रुव *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	जइ (pidfile)
		pid = get_pid(applet, pidfile);
	अण्णहा
		pid = 0;

	जइ (pid)
		pids = rc_find_pids(रिक्त, रिक्त, 0, pid);
	अण्णहा
		pids = rc_find_pids(exec, (ध्रुव वर्ण * ध्रुव *) argv, uid, 0);
	जइ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	मुक्त(pids);
	जइ (test) {
		जइ (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		जाव (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		जइ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		जइ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		जइ (ch_मूलम्)
			einfo("in root `%s'", ch_मूलम्);
		जइ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		जइ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		जइ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	जइ (pidfile)
		unlink(pidfile);

	जइ (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	जइ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	जइ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	जइ (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		पूर्णाङ्क group_गणनम् = ARRAY_SIZE(group_buf);
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

		जइ (nicelevel != INT_MIN) {
			जइ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			जइ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} अण्णहा जइ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		जइ (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		जइ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			जइ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		जइ (changeuser != रिक्त) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			जइ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			जइ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			जइ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		जइ (changeuser && getgrouplist(changeuser, gid, group_list, &group_गणनम्) < 0) {
			group_list = xmalloc(group_गणनम् * परिमाण(*group_list));
			जइ (getgrouplist(changeuser, gid, group_list, &group_गणनम्) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		जइ (ch_मूलम् && chroot(ch_मूलम्) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_मूलम्, strerror(errno));

		जइ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		जइ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			जइ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		जइ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		जइ (changeuser && setgroups(group_गणनम्, group_list))
			eerrorx("%s: setgroups() failed", applet);
		जइ (group_list != group_buf)
			मुक्त(group_list);
#ifdef __linux__
		जइ (uid && cap_setuid(uid))
#अण्णहा
		जइ (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		जइ (cap_iab != रिक्त) {
			i = cap_iab_set_proc(cap_iab);

			जइ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			जइ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		जइ (secbits != 0) {
			जइ (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		जइ (no_new_privs) {
			जइ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		जाव (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		जइ (changeuser != रिक्त) {
			pamenv = (ध्रुव वर्ण *ध्रुव *)pam_getenvlist(pamh);
			जइ (pamenv) {
				जाव (*pamenv) {
					/* Don't add strings unless they set a var */
					जइ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					अण्णहा
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			जइ (strncmp(env->मानम्, "RC_", 3) == 0
					|| strncmp(env->मानम्, "SSD_", 4) == 0
					|| strncmp(env->मानम्, "EINFO_", 6) == 0) {
				*strchr(env->मानम्, '=') = '\0';
				unsetenv(env->मानम्);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		जइ ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			जाव (token && *token) {
				p = strchr(token, ':');
				जइ (p) {
					*p++ = '\0';
					जाव (*p == ':')
						p++;
				}
				जइ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					जइ (np != newpath)
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
		जइ (redirect_stdin) {
			जइ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		जइ (redirect_stdout) {
			जइ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}अण्णहा जइ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			जइ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		जइ (redirect_stderr) {
			जइ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}अण्णहा जइ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			जइ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		जइ (background)
			dup2(stdin_fd, STDIN_FILENO);
		जइ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		जइ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		जइ (notify.type == NOTIFY_FD) {
			जइ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			जइ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			जइ (notify.pipe[1] == notify.fd) {
				पूर्णाङ्क flags = fcntl(notify.fd, F_GETFD, 0);
				जइ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				जइ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		जइ (scheduler != रिक्त) {
			पूर्णाङ्क scheduler_index;
			संरचना sched_param sched =  {.sched_priority = sched_prio};
			जइ (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			अण्णहा जइ (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			अण्णहा जइ (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			अण्णहा जइ (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			अण्णहा जइ (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			अण्णहा जइ (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			जइ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			जइ (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} अण्णहा जइ (sched_prio != -1) {
			ध्रुव संरचना sched_param sched =  {.sched_priority = sched_prio};
			जइ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		जइ (changeuser != रिक्त && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	जाव ((ss = पठ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	जइ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	जइ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		करेहि {
			pid = waitpid(spid, &i, 0);
			जइ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				प्रत्यागम -1;
			}
		} जाव (!WIFEXITED(i) && !WIFSIGNALED(i));
		जइ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	जइ (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_मानम्("rc_start_wait"))))
	{
		जइ (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	जइ (notify.type != NOTIFY_NONE) {
		जइ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} अण्णहा जइ (start_wait > 0) {
		संरचना timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		जइ (nanosleep(&ts, रिक्त) == -1) {
			जइ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				प्रत्यागम 0;
			}
		}
		जइ (background) {
			जइ (kill(pid, 0) == 0)
				alive = true;
		} अण्णहा {
			जइ (pidfile) {
				pid = get_pid(applet, pidfile);
				जइ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} अण्णहा
				pid = 0;
			जइ (do_stop(applet, exec, (ध्रुव वर्ण *ध्रुव *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		जइ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	जइ (svcname)
		rc_service_daemon_set(svcname, exec,
		    (ध्रुव वर्ण *ध्रुव *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
