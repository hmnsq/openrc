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

#जर defined(__linux__) && !defined(_GNU_SOURCE)
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

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = रिक्त;
स्थिरांक अक्षर getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
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
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
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
स्थिरांक अक्षर *usagestring = रिक्त;

स्थिर अक्षर **nav;

स्थिर अक्षर *changeuser, *ch_मूळ, *ch_dir;

बाह्य अक्षर **environ;

#जर !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#जर !defined(__DragonFly__)
स्थिर ओळीत पूर्णांक ioprio_set(पूर्णांक which RC_UNUSED,
			     पूर्णांक who RC_UNUSED,
			     पूर्णांक ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	परत syscall(SYS_ioprio_set, which, who, ioprio);
#अन्यथा
	परत 0;
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
handle_signal(पूर्णांक sig)
{
	पूर्णांक status;
	पूर्णांक serrno = त्रुटि_सं;
	स्थिरांक अक्षर *signame = रिक्त;

	निवड (sig) {
	स्थिती संक_विघ्न:
		जर (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	स्थिती संक_इति:
		जर (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	स्थिती SIGQUIT:
		जर (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	स्थिती SIGCHLD:
		साठी (;;) {
			जर (waitpid(-1, &status, WNOHANG) < 0) {
				जर (त्रुटि_सं != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, म_त्रुटि(त्रुटि_सं));
				थांबा;
			}
		}
		थांबा;

	पूर्वनिर्धारित:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	त्रुटि_सं = serrno;
}

स्थिर अक्षर *
expand_home(स्थिरांक अक्षर *home, स्थिरांक अक्षर *path)
{
	अक्षर *opath, *ppath, *p, *nh;
	संरचना passwd *pw;

	जर (!path || *path != '~')
		परत xstrdup(path);

	opath = ppath = xstrdup(path);
	जर (ppath[1] != '/' && ppath[1] != '\0') {
		p = म_अक्षर(ppath + 1, '/');
		जर (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		जर (pw) {
			home = pw->pw_dir;
			ppath = p;
			जर (ppath)
				*ppath = '/';
		} अन्यथा
			home = रिक्त;
	} अन्यथा
		ppath++;

	जर (!home) {
	मुक्त(opath);
		परत xstrdup(path);
	}
	जर (!ppath) {
		मुक्त(opath);
		परत xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	मुक्त(opath);
	परत nh;
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक devnull_fd = -1;
#ifdef TIOCNOTTY
	पूर्णांक tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = रिक्त;
	पूर्णांक pamr;
	स्थिरांक अक्षर *स्थिरांक *pamenv = रिक्त;
#endif

	पूर्णांक opt;
	माप_प्रकार size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	अक्षर *exec = रिक्त;
	अक्षर *startas = रिक्त;
	अक्षर *name = रिक्त;
	अक्षर *pidfile = रिक्त;
	अक्षर *retry = रिक्त;
	पूर्णांक sig = -1;
	पूर्णांक nicelevel = पूर्णांक_न्यून, ionicec = -1, ioniced = 0;
	पूर्णांक oom_score_adj = पूर्णांक_न्यून;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	अक्षर *home = रिक्त;
	पूर्णांक tid = 0;
	अक्षर *redirect_stdin = रिक्त;
	अक्षर *redirect_stderr = रिक्त;
	अक्षर *redirect_stdout = रिक्त;
	अक्षर *stderr_process = रिक्त;
	अक्षर *stdout_process = रिक्त;
	पूर्णांक stdin_fd;
	पूर्णांक stdout_fd;
	पूर्णांक stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	पूर्णांक i;
	अक्षर *svcname = दो_पर्या("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	अक्षर *tmp, *newpath, *np;
	अक्षर *p;
	अक्षर *token;
	अक्षर *exec_file = रिक्त;
	संरचना passwd *pw;
	संरचना group *gr;
	bool stopgroup = false;
	अक्षर *ओळ = रिक्त;
	खाता *fp;
	माप_प्रकार len;
	mode_t numask = 022;
	अक्षर **margv;
	अचिन्हित पूर्णांक start_wait = 0;
	स्थिरांक अक्षर *scheduler = रिक्त;
	पूर्णांक sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = रिक्त;
	अचिन्हित secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	पूर्णांक pipefd[2];
	अक्षर readbuf[1];
	ssize_t ss;
	संरचना notify notify = {0};
	पूर्णांक ret = निकास_सफल;

	applet = basename_c(argv[0]);
	निकास_पर(cleanup);

	signal_setup(संक_विघ्न, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(संक_इति, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	जर ((tmp = दो_पर्या("SSD_NICELEVEL")))
		जर (माला_पूछो(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	जर ((tmp = दो_पर्या("SSD_IONICELEVEL"))) {
		पूर्णांक n = माला_पूछो(tmp, "%d:%d", &ionicec, &ioniced);
		जर (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		जर (ionicec == 0)
			ioniced = 0;
		अन्यथा जर (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	जर ((tmp = दो_पर्या("SSD_OOM_SCORE_ADJ")))
		जर (माला_पूछो(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = दो_पर्या("USER");
	home = दो_पर्या("HOME");
	जर (home == रिक्त || p == रिक्त) {
		pw = getpwuid(getuid());
		जर (pw != रिक्त) {
			जर (p == रिक्त)
				setenv("USER", pw->pw_name, 1);
			जर (home == रिक्त) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	पर्यंत ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (पूर्णांक *) 0)) != -1)
		निवड (opt) {
		स्थिती LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			जर (cap_iab == रिक्त)
				eerrorx("Could not parse iab: %s", म_त्रुटि(त्रुटि_सं));
#अन्यथा
			eerrorx("Capabilities support not enabled");
#endif
			थांबा;

		स्थिती LONGOPT_SECBITS:
#ifdef __linux__
			जर (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = रिक्त;
			secbits = म_से_अदीर्घ(optarg, &tmp, 0);
			जर (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#अन्यथा
			eerrorx("Capabilities support not enabled");
#endif
			थांबा;

		स्थिती LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#अन्यथा
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			थांबा;

		स्थिती 'I': /* --ionice */
			जर (माला_पूछो(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			जर (ionicec == 0)
				ioniced = 0;
			अन्यथा जर (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			थांबा;

		स्थिती 'K':  /* --stop */
			stop = true;
			थांबा;

		स्थिती 'G':  /* --stop-group */
			stopgroup = true;
			थांबा;

		स्थिती 'N':  /* --nice */
			जर (माला_पूछो(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			थांबा;

		स्थिती LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			जर (माला_पूछो(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			थांबा;

		स्थिती 'P':  /* --progress */
			progress = true;
			थांबा;

		स्थिती 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			थांबा;

		स्थिती 'S':  /* --start */
			start = true;
			थांबा;

		स्थिती 'b':  /* --background */
			background = true;
			थांबा;

		स्थिती 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		स्थिती 'u':  /* --user <username>|<uid> */
		{
			अक्षर dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			जर (माला_पूछो(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			अन्यथा
				pw = getpwuid((uid_t)tid);

			जर (pw == रिक्त)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			जर (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			जर (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			जर (gid == 0)
				gid = pw->pw_gid;

			जर (p) {
				tmp = strsep (&p, ":");
				जर (माला_पूछो(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				अन्यथा
					gr = getgrgid((gid_t) tid);

				जर (gr == रिक्त)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		थांबा;

		स्थिती 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			थांबा;

		स्थिती 'e': /* --env */
			putenv(optarg);
			थांबा;

		स्थिती 'g':  /* --group <group>|<gid> */
			जर (माला_पूछो(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			अन्यथा
				gr = getgrgid((gid_t)tid);
			जर (gr == रिक्त)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			थांबा;

		स्थिती 'i': /* --interpreted */
			interpreted = true;
			थांबा;

		स्थिती 'k':
			जर (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			थांबा;

		स्थिती 'm':  /* --make-pidfile */
			makepidfile = true;
			थांबा;

		स्थिती 'n':  /* --name <process-name> */
			name = optarg;
			थांबा;

		स्थिती 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			थांबा;

		स्थिती 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			थांबा;

		स्थिती 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			थांबा;

		स्थिती 't':  /* --test */
			test = true;
			थांबा;

		स्थिती 'r':  /* --chroot /new/root */
			ch_मूळ = optarg;
			थांबा;

		स्थिती 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			थांबा;
		स्थिती 'w':
			जर (माला_पूछो(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			थांबा;
		स्थिती 'x':  /* --exec <executable> */
			exec = optarg;
			थांबा;

		स्थिती '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			थांबा;

		स्थिती '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			थांबा;

		स्थिती '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			थांबा;

		स्थिती '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			थांबा;

		स्थिती '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			थांबा;

		स्थिती LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			थांबा;

		स्थिती LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			माला_पूछो(optarg, "%d", &sched_prio);
			थांबा;

		स्थिती LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			थांबा;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	जर (!start &&
	    !stop &&
	    sig != संक_विघ्न &&
	    sig != संक_इति &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	जर (!exec)
		exec = startas;
	अन्यथा जर (!name)
		name = startas;

	जर (!exec) {
		exec = *argv;
		जर (!exec)
			exec = name;
		जर (name && start)
			*argv = name;
	} अन्यथा जर (name) {
		*--argv = name;
		++argc;
	} अन्यथा जर (exec) {
		*--argv = exec;
		++argc;
	};

	जर (stop || sig != -1) {
		जर (sig == -1)
			sig = संक_इति;
		जर (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		जर (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		जर (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		जर (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		जर (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		जर (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} अन्यथा {
		जर (!exec)
			eerrorx("%s: nothing to start", applet);
		जर (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		जर ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		जर ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		जर (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		जर (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	जर (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	जर (ch_मूळ && *ch_मूळ == '~')
		ch_मूळ = expand_home(home, ch_मूळ);
	जर (exec) {
		जर (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		जर (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			जर (ch_मूळ)
				xasprintf(&exec_file, "%s/%s", ch_मूळ, exec);
			अन्यथा
				xasprintf(&exec_file, "%s", exec);
		} अन्यथा {
			/* Something in $PATH */
			p = tmp = xstrdup(दो_पर्या("PATH"));
			exec_file = रिक्त;
			पर्यंत ((token = strsep(&p, ":"))) {
				जर (ch_मूळ)
					xasprintf(&exec_file, "%s/%s/%s", ch_मूळ, token, exec);
				अन्यथा
					xasprintf(&exec_file, "%s/%s", token, exec);
				जर (exec_file && access(exec_file, F_OK) == 0)
					थांबा;
				मुक्त(exec_file);
				exec_file = रिक्त;
			}
			मुक्त(tmp);
		}
	}
	जर (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		मुक्त(exec_file);
		निकास(निकास_त्रुटि);
	}
	जर (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	जर (interpreted && !pidfile) {
		fp = ख_खोलो(exec_file, "r");
		जर (fp) {
			ओळ = रिक्त;
			जर (xgetline(&ओळ, &size, fp) == -1)
				eerrorx("%s: %s", applet, म_त्रुटि(त्रुटि_सं));
			p = ओळ;
			ख_बंद(fp);
			जर (p != रिक्त && ओळ[0] == '#' && ओळ[1] == '!') {
				p = ओळ + 2;
				/* Strip leading spaces */
				पर्यंत (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				मुक्त(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				साठी (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(आकार(अक्षर *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				जर (p)
					nav[len++] = p;
				साठी (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = रिक्त;
			}
		}
	}
	margv = nav ? nav : argv;

	जर (stop || sig != -1) {
		जर (sig == -1)
			sig = संक_इति;
		जर (!stop)
			oknodo = true;
		जर (retry)
			parse_schedule(applet, retry, sig);
		अन्यथा जर (test || oknodo)
			parse_schedule(applet, "0", sig);
		अन्यथा
			parse_schedule(applet, रिक्त, sig);
		जर (pidfile) {
			pid = get_pid(applet, pidfile);
			जर (pid == -1 && त्रुटि_सं != ENOENT)
				निकास(निकास_त्रुटि);
		} अन्यथा {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (स्थिरांक अक्षर *स्थिरांक *)margv,
		    pid, uid, stopgroup, test, progress, false);

		जर (i < 0)
			/* We failed to stop something */
			निकास(निकास_त्रुटि);
		जर (test || oknodo)
			परत i > 0 ? निकास_सफल : निकास_त्रुटि;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		जर (pidfile)
			unlink(pidfile);
		जर (svcname)
			rc_service_daemon_set(svcname, exec,
			    (स्थिरांक अक्षर *स्थिरांक *)argv,
			    pidfile, false);
		निकास(निकास_सफल);
	}

	जर (pidfile)
		pid = get_pid(applet, pidfile);
	अन्यथा
		pid = 0;

	जर (pid)
		pids = rc_find_pids(रिक्त, रिक्त, 0, pid);
	अन्यथा
		pids = rc_find_pids(exec, (स्थिरांक अक्षर * स्थिरांक *) argv, uid, 0);
	जर (pids)
		eerrorx("%s: %s is already running", applet, exec);

	मुक्त(pids);
	जर (test) {
		जर (rc_yesno(दो_पर्या("EINFO_QUIET")))
			निकास (निकास_सफल);

		einfon("Would start");
		पर्यंत (argc-- > 0)
			म_लिखो(" %s", *argv++);
		म_लिखो("\n");
		eindent();
		जर (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		जर (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		जर (ch_मूळ)
			einfo("in root `%s'", ch_मूळ);
		जर (ch_dir)
			einfo("in dir `%s'", ch_dir);
		जर (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		जर (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		निकास(निकास_सफल);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	जर (pidfile)
		unlink(pidfile);

	जर (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	जर (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, म_त्रुटि(त्रुटि_सं));

	जर ((pid = विभाजन()) == -1)
		eerrorx("%s: fork: %s", applet, म_त्रुटि(त्रुटि_सं));

	/* Child process - lets go! */
	जर (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		पूर्णांक group_मोजणी = ARRAY_SIZE(group_buf);
		pid_t mypid = getpid();
		बंद(pipefd[0]); /* Close the read end of the pipe. */
		umask(numask);

#ifdef TIOCNOTTY
		tty_fd = खोलो("/dev/tty", O_RDWR);
#endif

		devnull_fd = खोलो("/dev/null", O_RDWR);

		/* Must call setsid() before setting autogroup nicelevel
		 * but after opening tty_fd. */
		setsid();

		जर (nicelevel != पूर्णांक_न्यून) {
			जर (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    म_त्रुटि(त्रुटि_सं));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = ख_खोलो("/proc/self/autogroup", "r+");
			जर (fp) {
				ख_लिखो(fp, "%d\n", nicelevel);
				ख_बंद(fp);
			} अन्यथा जर (त्रुटि_सं != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, म_त्रुटि(त्रुटि_सं));
		}

		जर (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, म_त्रुटि(त्रुटि_सं));

		जर (oom_score_adj != पूर्णांक_न्यून) {
			fp = ख_खोलो("/proc/self/oom_score_adj", "w");
			जर (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, म_त्रुटि(त्रुटि_सं));
			ख_लिखो(fp, "%d\n", oom_score_adj);
			ख_बंद(fp);
		}

#ifdef HAVE_PAM
		जर (changeuser != रिक्त) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			जर (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			जर (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			जर (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		जर (changeuser && getgrouplist(changeuser, gid, group_list, &group_मोजणी) < 0) {
			group_list = xmalloc(group_मोजणी * आकार(*group_list));
			जर (getgrouplist(changeuser, gid, group_list, &group_मोजणी) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		जर (ch_मूळ && chroot(ch_मूळ) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_मूळ, म_त्रुटि(त्रुटि_सं));

		जर (ch_dir && स_बदलो(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, म_त्रुटि(त्रुटि_सं));

		जर (makepidfile && pidfile) {
			fp = ख_खोलो(pidfile, "w");
			जर (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    म_त्रुटि(त्रुटि_सं));
			ख_लिखो(fp, "%d\n", mypid);
			ख_बंद(fp);
		}


		जर (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		जर (changeuser && setgroups(group_मोजणी, group_list))
			eerrorx("%s: setgroups() failed", applet);
		जर (group_list != group_buf)
			मुक्त(group_list);
#ifdef __linux__
		जर (uid && cap_setuid(uid))
#अन्यथा
		जर (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		जर (cap_iab != रिक्त) {
			i = cap_iab_set_proc(cap_iab);

			जर (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", म_त्रुटि(त्रुटि_सं));

			जर (i != 0)
				eerrorx("Could not set iab: %s", म_त्रुटि(त्रुटि_सं));
		}

		जर (secbits != 0) {
			जर (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, म_त्रुटि(त्रुटि_सं));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		जर (no_new_privs) {
			जर (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
				eerrorx("Could not set No New Privs flag: %s", म_त्रुटि(त्रुटि_सं));
		}
#endif


#ifdef TIOCNOTTY
		ioctl(tty_fd, TIOCNOTTY, 0);
		बंद(tty_fd);
#endif

		/* Clean the environment of any RC_ variables */
		env_list = rc_stringlist_new();
		i = 0;
		पर्यंत (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		जर (changeuser != रिक्त) {
			pamenv = (स्थिरांक अक्षर *स्थिरांक *)pam_getenvlist(pamh);
			जर (pamenv) {
				पर्यंत (*pamenv) {
					/* Don't add strings unless they set a var */
					जर (म_अक्षर(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					अन्यथा
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			जर (म_भेदन(env->मूल्य, "RC_", 3) == 0
					|| म_भेदन(env->मूल्य, "SSD_", 4) == 0
					|| म_भेदन(env->मूल्य, "EINFO_", 6) == 0) {
				*म_अक्षर(env->मूल्य, '=') = '\0';
				unsetenv(env->मूल्य);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		जर ((token = दो_पर्या("PATH"))) {
			len = म_माप(token);
			newpath = np = xmalloc(len + 1);
			पर्यंत (token && *token) {
				p = म_अक्षर(token, ':');
				जर (p) {
					*p++ = '\0';
					पर्यंत (*p == ':')
						p++;
				}
				जर (म_भेद(token, RC_LIBEXECDIR "/bin") != 0 &&
				    म_भेद(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = म_माप(token);
					जर (np != newpath)
						*np++ = ':';
					स_नकल(np, token, len);
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
		जर (redirect_stdin) {
			जर ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, म_त्रुटि(त्रुटि_सं));
		}
		जर (redirect_stdout) {
			जर ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, म_त्रुटि(त्रुटि_सं));
		}अन्यथा जर (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			जर (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, म_त्रुटि(त्रुटि_सं));
		}
		जर (redirect_stderr) {
			जर ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, म_त्रुटि(त्रुटि_सं));
		}अन्यथा जर (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			जर (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, म_त्रुटि(त्रुटि_सं));
		}

		जर (background)
			dup2(stdin_fd, STDIN_FILENO);
		जर (background || redirect_stdout || stdout_process
				|| rc_yesno(दो_पर्या("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		जर (background || redirect_stderr || stderr_process
				|| rc_yesno(दो_पर्या("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		जर (notify.type == NOTIFY_FD) {
			जर (बंद(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, म_त्रुटि(त्रुटि_सं));
			जर (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, म_त्रुटि(त्रुटि_सं));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			जर (notify.pipe[1] == notify.fd) {
				पूर्णांक flags = fcntl(notify.fd, F_GETFD, 0);
				जर (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, म_त्रुटि(त्रुटि_सं));
				जर (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, म_त्रुटि(त्रुटि_सं));
			}

		}

		जर (scheduler != रिक्त) {
			पूर्णांक scheduler_index;
			संरचना sched_param sched =  {.sched_priority = sched_prio};
			जर (म_भेद(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			अन्यथा जर (म_भेद(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			अन्यथा जर (म_भेद(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			अन्यथा जर (म_भेद(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			अन्यथा जर (म_भेद(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			अन्यथा जर (माला_पूछो(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			जर (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			जर (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", म_त्रुटि(त्रुटि_सं));
		} अन्यथा जर (sched_prio != -1) {
			स्थिरांक संरचना sched_param sched =  {.sched_priority = sched_prio};
			जर (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", म_त्रुटि(त्रुटि_सं));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		जर (changeuser != रिक्त && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,म_त्रुटि(त्रुटि_सं));
	}

	/* Parent process */

	बंद(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	पर्यंत ((ss = पठ(pipefd[0], readbuf, 1)) == -1 && त्रुटि_सं == EINTR);
	जर (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, म_त्रुटि(त्रुटि_सं));

	बंद(pipefd[0]);

	जर (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		करा {
			pid = waitpid(spid, &i, 0);
			जर (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, म_त्रुटि(त्रुटि_सं));
				परत -1;
			}
		} पर्यंत (!WIFEXITED(i) && !WIFSIGNALED(i));
		जर (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			निकास(निकास_त्रुटि);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	जर (start_wait == 0 &&
	    ((p = दो_पर्या("SSD_STARTWAIT")) ||
		(p = rc_conf_मूल्य("rc_start_wait"))))
	{
		जर (माला_पूछो(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	जर (notify.type != NOTIFY_NONE) {
		जर (!notify_wait(applet, notify))
			ret = निकास_त्रुटि;
	} अन्यथा जर (start_wait > 0) {
		संरचना timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		जर (nanosleep(&ts, रिक्त) == -1) {
			जर (त्रुटि_सं != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, म_त्रुटि(त्रुटि_सं));
				परत 0;
			}
		}
		जर (background) {
			जर (समाप्त(pid, 0) == 0)
				alive = true;
		} अन्यथा {
			जर (pidfile) {
				pid = get_pid(applet, pidfile);
				जर (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} अन्यथा
				pid = 0;
			जर (do_stop(applet, exec, (स्थिरांक अक्षर *स्थिरांक *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		जर (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	जर (svcname)
		rc_service_daemon_set(svcname, exec,
		    (स्थिरांक अक्षर *स्थिरांक *)margv, pidfile, true);

	निकास(ret);
	/* NOTREACHED */
}
