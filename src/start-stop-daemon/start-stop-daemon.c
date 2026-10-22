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

#ವ್ಯಾಖ್ಯಾನಿಸು ONE_MS           1000000

#ಆದರೆ defined(__linux__) && !defined(GNU_ಮೂಲ)
/* For extra SCHED_* defines. */
# define GNU_ಮೂಲ
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <termios.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ಸ್ಥಿರ ರಚನೆ pam_conv conv = { ಖಾಲಿ, ಖಾಲಿ};
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
# include <sys/capability.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "schedules.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ಎಣಿಕೆ {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_ತಳ = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SCHEDULER,
  LONGOPT_SCHEDULER_PRIO,
  LONGOPT_SECBITS,
  LONGOPT_NOTIFY,
};

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "capabilities", 1, ಖಾಲಿ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ಖಾಲಿ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ಖಾಲಿ, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ಖಾಲಿ, 'I'},
	{ "stop",         0, ಖಾಲಿ, 'K'},
	{ "nicelevel",    1, ಖಾಲಿ, 'N'},
	{ "oom-score-adj",1, ಖಾಲಿ, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ಖಾಲಿ, 'R'},
	{ "start",        0, ಖಾಲಿ, 'S'},
	{ "startas",      1, ಖಾಲಿ, 'a'},
	{ "background",   0, ಖಾಲಿ, 'b'},
	{ "chuid",        1, ಖಾಲಿ, 'c'},
	{ "chdir",        1, ಖಾಲಿ, 'd'},
	{ "env",          1, ಖಾಲಿ, 'e'},
	{ "umask",        1, ಖಾಲಿ, 'k'},
	{ "group",        1, ಖಾಲಿ, 'g'},
	{ "stop-group",   0, ಖಾಲಿ, 'G'},
	{ "interpreted",  0, ಖಾಲಿ, 'i'},
	{ "make-pidfile", 0, ಖಾಲಿ, 'm'},
	{ "name",         1, ಖಾಲಿ, 'n'},
	{ "oknodo",       0, ಖಾಲಿ, 'o'},
	{ "pidfile",      1, ಖಾಲಿ, 'p'},
	{ "signal",       1, ಖಾಲಿ, 's'},
	{ "test",         0, ಖಾಲಿ, 't'},
	{ "user",         1, ಖಾಲಿ, 'u'},
	{ "chroot",       1, ಖಾಲಿ, 'r'},
	{ "wait",         1, ಖಾಲಿ, 'w'},
	{ "exec",         1, ಖಾಲಿ, 'x'},
	{ "stdin",        1, ಖಾಲಿ, '0'},
	{ "stdout",       1, ಖಾಲಿ, '1'},
	{ "stderr",       1, ಖಾಲಿ, '2'},
	{ "stdout-logger",1, ಖಾಲಿ, '3'},
	{ "stderr-logger",1, ಖಾಲಿ, '4'},
	{ "progress",     0, ಖಾಲಿ, 'P'},
	{ "scheduler",    1, ಖಾಲಿ, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ಖಾಲಿ, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ಖಾಲಿ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಸ್ಥಿರ ಅಕ್ಷರ **nav;

ಸ್ಥಿರ ಅಕ್ಷರ *changeuser, *ch_ಬೇರು, *ch_dir;

ಬಾಹ್ಯ ಅಕ್ಷರ **environ;

#ಆದರೆ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(__DragonFly__)
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ioprio_ಗಣ(ಪೂರ್ಣಾಂಕ which RC_ಬಳಸದ,
			     ಪೂರ್ಣಾಂಕ who RC_ಬಳಸದ,
			     ಪೂರ್ಣಾಂಕ ioprio RC_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS_ioprio_set
	ಹಿಂತಿರುಗಿ syscall(SYS_ioprio_set, which, who, ioprio);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ
cleanup(ಶೂನ್ಯ)
{
	ಬಿಡುಗಡೆ(changeuser);
	ಬಿಡುಗಡೆ(nav);
	free_schedulelist();
}

ಸ್ಥಿರ ಶೂನ್ಯ
handle_signal(ಪೂರ್ಣಾಂಕ sig)
{
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ serrno = errno;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *signame = ಖಾಲಿ;

	ಆಯ್ಕೆ (sig) {
	ಸ್ಥಿತಿ SIGINT:
		ಆದರೆ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ಸ್ಥಿತಿ SIGTERM:
		ಆದರೆ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ಸ್ಥಿತಿ SIGQUIT:
		ಆದರೆ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	ಸ್ಥಿತಿ SIGCHLD:
		ಗಾಗಿ (;;) {
			ಆದರೆ (waitpid(-1, &ಸ್ಥಿತಿ_2, WNOHANG) < 0) {
				ಆದರೆ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				ನಿಲ್ಲಿಸು;
			}
		}
		ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ಅಕ್ಷರ *
expand_home(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *home, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *opath, *ppath, *p, *nh;
	ರಚನೆ passwd *pw;

	ಆದರೆ (!ಮಾರ್ಗ || *ಮಾರ್ಗ != '~')
		ಹಿಂತಿರುಗಿ xstrdup(ಮಾರ್ಗ);

	opath = ppath = xstrdup(ಮಾರ್ಗ);
	ಆದರೆ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ಆದರೆ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ಆದರೆ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ಆದರೆ (ppath)
				*ppath = '/';
		} ಇಲ್ಲದಿದ್ದರೆ
			home = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ
		ppath++;

	ಆದರೆ (!home) {
	ಬಿಡುಗಡೆ(opath);
		ಹಿಂತಿರುಗಿ xstrdup(ಮಾರ್ಗ);
	}
	ಆದರೆ (!ppath) {
		ಬಿಡುಗಡೆ(opath);
		ಹಿಂತಿರುಗಿ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ಬಿಡುಗಡೆ(opath);
	ಹಿಂತಿರುಗಿ nh;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ devnull_fd = -1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
	ಪೂರ್ಣಾಂಕ tty_fd = -1;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	pam_handle_t *pamh = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ pamr;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *pamenv = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	ಪೂರ್ಣಾಂಕ opt;
	size_t ಗಾತ್ರ_2 = 0;
	bool ಆರಂಭ = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	ಅಕ್ಷರ *exec = ಖಾಲಿ;
	ಅಕ್ಷರ *startas = ಖಾಲಿ;
	ಅಕ್ಷರ *name = ಖಾಲಿ;
	ಅಕ್ಷರ *pidfile = ಖಾಲಿ;
	ಅಕ್ಷರ *retry = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ sig = -1;
	ಪೂರ್ಣಾಂಕ nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ಪೂರ್ಣಾಂಕ oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool ಪ್ರಗತಿ = false;
	uid_t uid = 0;
	gid_t gid = 0;
	ಅಕ್ಷರ *home = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ tid = 0;
	ಅಕ್ಷರ *redirect_stdin = ಖಾಲಿ;
	ಅಕ್ಷರ *redirect_stderr = ಖಾಲಿ;
	ಅಕ್ಷರ *redirect_stdout = ಖಾಲಿ;
	ಅಕ್ಷರ *stderr_process = ಖಾಲಿ;
	ಅಕ್ಷರ *stdout_process = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ stdin_fd;
	ಪೂರ್ಣಾಂಕ stdout_fd;
	ಪೂರ್ಣಾಂಕ stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ಅಕ್ಷರ *tmp, *newpath, *np;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
	ಅಕ್ಷರ *exec_file = ಖಾಲಿ;
	ರಚನೆ passwd *pw;
	ರಚನೆ group *gr;
	bool stopgroup = false;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	ಅಕ್ಷರ **margv;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಆರಂಭ_wait = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *scheduler = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ sched_prio = -1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	cap_iab_t cap_iab = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ secbits = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PR_ಗಣ_NO_NEW_PRIVS
	bool no_new_privs = false;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ pipefd[2];
	ಅಕ್ಷರ readbuf[1];
	ssize_t ss;
	ರಚನೆ notify notify = {0};
	ಪೂರ್ಣಾಂಕ ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_ಸಿದ್ಧತೆ(SIGINT, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGQUIT, handle_signal);
	signal_ಸಿದ್ಧತೆ(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ಆದರೆ ((tmp = getenv("SSD_NICELEVEL")))
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ಆದರೆ ((tmp = getenv("SSD_IONICELEVEL"))) {
		ಪೂರ್ಣಾಂಕ n = ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d:%d", &ionicec, &ioniced);
		ಆದರೆ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ಆದರೆ (ionicec == 0)
			ioniced = 0;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ಆದರೆ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ಆದರೆ (home == ಖಾಲಿ || p == ಖಾಲಿ) {
		pw = getpwuid(getuid());
		ಆದರೆ (pw != ಖಾಲಿ) {
			ಆದರೆ (p == ಖಾಲಿ)
				setenv("USER", pw->pw_name, 1);
			ಆದರೆ (home == ಖಾಲಿ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ಆಗ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ LONGOPT_CAPABILITIES:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
			cap_iab = cap_iab_from_ಪಠ್ಯ(optarg);
			ಆದರೆ (cap_iab == ಖಾಲಿ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("Capabilities support not enabled");
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_SECBITS:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
			ಆದರೆ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ಖಾಲಿ;
			secbits = strtoul(optarg, &tmp, 0);
			ಆದರೆ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("Capabilities support not enabled");
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_NO_NEW_PRIVS:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PR_ಗಣ_NO_NEW_PRIVS
			no_new_privs = true;
#ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'I': /* --ionice */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ಆದರೆ (ionicec == 0)
				ioniced = 0;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'K':  /* --stop */
			stop = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'G':  /* --stop-group */
			stopgroup = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'N':  /* --nice */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'P':  /* --progress */
			ಪ್ರಗತಿ = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'S':  /* --start */
			ಆರಂಭ = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'b':  /* --background */
			background = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		ಸ್ಥಿತಿ 'u':  /* --user <username>|<uid> */
		{
			ಅಕ್ಷರ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ಇಲ್ಲದಿದ್ದರೆ
				pw = getpwuid((uid_t)tid);

			ಆದರೆ (pw == ಖಾಲಿ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ಆದರೆ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ಆದರೆ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ಆದರೆ (gid == 0)
				gid = pw->pw_gid;

			ಆದರೆ (p) {
				tmp = strsep (&p, ":");
				ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ಇಲ್ಲದಿದ್ದರೆ
					gr = getgrgid((gid_t) tid);

				ಆದರೆ (gr == ಖಾಲಿ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'e': /* --env */
			putenv(optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'g':  /* --group <group>|<gid> */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ಇಲ್ಲದಿದ್ದರೆ
				gr = getgrgid((gid_t)tid);
			ಆದರೆ (gr == ಖಾಲಿ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'i': /* --interpreted */
			interpreted = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'k':
			ಆದರೆ (parse_ಬಹುಲಕ(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'm':  /* --make-pidfile */
			makepidfile = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'n':  /* --name <process-name> */
			name = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 't':  /* --test */
			test = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'r':  /* --chroot /new/root */
			ch_ಬೇರು = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'w':
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%u", &ಆರಂಭ_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'x':  /* --exec <executable> */
			exec = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &sched_prio);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			ನಿಲ್ಲಿಸು;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	ಆದರೆ (!ಆರಂಭ &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ಆದರೆ (!exec)
		exec = startas;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!name)
		name = startas;

	ಆದರೆ (!exec) {
		exec = *argv;
		ಆದರೆ (!exec)
			exec = name;
		ಆದರೆ (name && ಆರಂಭ)
			*argv = name;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (name) {
		*--argv = name;
		++argc;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (exec) {
		*--argv = exec;
		++argc;
	};

	ಆದರೆ (stop || sig != -1) {
		ಆದರೆ (sig == -1)
			sig = SIGTERM;
		ಆದರೆ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ಆದರೆ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ಆದರೆ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ಆದರೆ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ಆದರೆ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ಆದರೆ (ಆರಂಭ_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (!exec)
			eerrorx("%s: nothing to start", applet);
		ಆದರೆ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ಆದರೆ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ಆದರೆ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ಆದರೆ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ಆದರೆ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ಆದರೆ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ಆದರೆ (ch_ಬೇರು && *ch_ಬೇರು == '~')
		ch_ಬೇರು = expand_home(home, ch_ಬೇರು);
	ಆದರೆ (exec) {
		ಆದರೆ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ಆದರೆ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ಆದರೆ (ch_ಬೇರು)
				xasprintf(&exec_file, "%s/%s", ch_ಬೇರು, exec);
			ಇಲ್ಲದಿದ್ದರೆ
				xasprintf(&exec_file, "%s", exec);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ಖಾಲಿ;
			ಆಗ ((token = strsep(&p, ":"))) {
				ಆದರೆ (ch_ಬೇರು)
					xasprintf(&exec_file, "%s/%s/%s", ch_ಬೇರು, token, exec);
				ಇಲ್ಲದಿದ್ದರೆ
					xasprintf(&exec_file, "%s/%s", token, exec);
				ಆದರೆ (exec_file && access(exec_file, F_OK) == 0)
					ನಿಲ್ಲಿಸು;
				ಬಿಡುಗಡೆ(exec_file);
				exec_file = ಖಾಲಿ;
			}
			ಬಿಡುಗಡೆ(tmp);
		}
	}
	ಆದರೆ (ಆರಂಭ && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		ಬಿಡುಗಡೆ(exec_file);
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	}
	ಆದರೆ (ಆರಂಭ && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ಆದರೆ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ಆದರೆ (fp) {
			ಸಾಲು = ಖಾಲಿ;
			ಆದರೆ (xgetline(&ಸಾಲು, &ಗಾತ್ರ_2, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = ಸಾಲು;
			fclose(fp);
			ಆದರೆ (p != ಖಾಲಿ && ಸಾಲು[0] == '#' && ಸಾಲು[1] == '!') {
				p = ಸಾಲು + 2;
				/* Strip leading spaces */
				ಆಗ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				ಬಿಡುಗಡೆ(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				ಗಾಗಿ (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(ಗಾತ್ರ(ಅಕ್ಷರ *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ಆದರೆ (p)
					nav[len++] = p;
				ಗಾಗಿ (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ಖಾಲಿ;
			}
		}
	}
	margv = nav ? nav : argv;

	ಆದರೆ (stop || sig != -1) {
		ಆದರೆ (sig == -1)
			sig = SIGTERM;
		ಆದರೆ (!stop)
			oknodo = true;
		ಆದರೆ (retry)
			parse_ವೇಳಾಪಟ್ಟಿ(applet, retry, sig);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (test || oknodo)
			parse_ವೇಳಾಪಟ್ಟಿ(applet, "0", sig);
		ಇಲ್ಲದಿದ್ದರೆ
			parse_ವೇಳಾಪಟ್ಟಿ(applet, ಖಾಲಿ, sig);
		ಆದರೆ (pidfile) {
			pid = get_pid(applet, pidfile);
			ಆದರೆ (pid == -1 && errno != ENOENT)
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
		} ಇಲ್ಲದಿದ್ದರೆ {
			pid = 0;
		}
		i = ಸತತ_stop_ವೇಳಾಪಟ್ಟಿ(applet, exec, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)margv,
		    pid, uid, stopgroup, test, ಪ್ರಗತಿ, false);

		ಆದರೆ (i < 0)
			/* We failed to stop something */
			ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
		ಆದರೆ (test || oknodo)
			ಹಿಂತಿರುಗಿ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ಆದರೆ (pidfile)
			unlink(pidfile);
		ಆದರೆ (svcname)
			rc_service_daemon_ಗಣ(svcname, exec,
			    (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)argv,
			    pidfile, false);
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	}

	ಆದರೆ (pidfile)
		pid = get_pid(applet, pidfile);
	ಇಲ್ಲದಿದ್ದರೆ
		pid = 0;

	ಆದರೆ (pid)
		pids = rc_ಕಂಡುಹಿಡಿ_pids(ಖಾಲಿ, ಖಾಲಿ, 0, pid);
	ಇಲ್ಲದಿದ್ದರೆ
		pids = rc_ಕಂಡುಹಿಡಿ_pids(exec, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *) argv, uid, 0);
	ಆದರೆ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	ಬಿಡುಗಡೆ(pids);
	ಆದರೆ (test) {
		ಆದರೆ (rc_yesno(getenv("EINFO_QUIET")))
			ನಿರ್ಗಮಿಸು (EXIT_SUCCESS);

		einfon("Would start");
		ಆಗ (argc-- > 0)
			ಬರೆ(" %s", *argv++);
		ಬರೆ("\n");
		eindent();
		ಆದರೆ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ಆದರೆ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ಆದರೆ (ch_ಬೇರು)
			einfo("in root `%s'", ch_ಬೇರು);
		ಆದರೆ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ಆದರೆ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ಆದರೆ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ಆದರೆ (pidfile)
		unlink(pidfile);

	ಆದರೆ (background)
		signal_ಸಿದ್ಧತೆ(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ಆದರೆ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ಆದರೆ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ಆದರೆ (pid == 0) {
		gid_t ಗುಂಪು_buf[32], *ಗುಂಪು_list = ಗುಂಪು_buf;
		ಪೂರ್ಣಾಂಕ ಗುಂಪು_ಎಣಿಕೆ = ARRAY_ಗಾತ್ರ(ಗುಂಪು_buf);
		pid_t mypid = getpid();
		close(pipefd[0]); /* Close the read end of the pipe. */
		umask(numask);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#ಆದರೆ_ಅಂತ್ಯ

		devnull_fd = open("/dev/null", O_RDWR);

		/* Must call setsid() before setting autogroup nicelevel
		 * but after opening tty_fd. */
		setsid();

		ಆದರೆ (nicelevel != INT_MIN) {
			ಆದರೆ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ಆದರೆ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ಆದರೆ (ionicec != -1 &&
		    ioprio_ಗಣ(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ಆದರೆ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ಆದರೆ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
		ಆದರೆ (changeuser != ಖಾಲಿ) {
			pamr = pam_ಆರಂಭ("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ಆದರೆ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ಆದರೆ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ಆದರೆ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (changeuser && getgrouplist(changeuser, gid, ಗುಂಪು_list, &ಗುಂಪು_ಎಣಿಕೆ) < 0) {
			ಗುಂಪು_list = xmalloc(ಗುಂಪು_ಎಣಿಕೆ * ಗಾತ್ರ(*ಗುಂಪು_list));
			ಆದರೆ (getgrouplist(changeuser, gid, ಗುಂಪು_list, &ಗುಂಪು_ಎಣಿಕೆ) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ಆದರೆ (ch_ಬೇರು && chroot(ch_ಬೇರು) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_ಬೇರು, strerror(errno));

		ಆದರೆ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ಆದರೆ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ಆದರೆ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ಆದರೆ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ಆದರೆ (changeuser && setgroups(ಗುಂಪು_ಎಣಿಕೆ, ಗುಂಪು_list))
			eerrorx("%s: setgroups() failed", applet);
		ಆದರೆ (ಗುಂಪು_list != ಗುಂಪು_buf)
			ಬಿಡುಗಡೆ(ಗುಂಪು_list);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
		ಆದರೆ (uid && cap_setuid(uid))
#ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (uid && setuid(uid))
#ಆದರೆ_ಅಂತ್ಯ
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
		ಆದರೆ (cap_iab != ಖಾಲಿ) {
			i = cap_iab_ಗಣ_proc(cap_iab);

			ಆದರೆ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ಆದರೆ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ಆದರೆ (secbits != 0) {
			ಆದರೆ (cap_ಗಣ_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PR_ಗಣ_NO_NEW_PRIVS
		ಆದರೆ (no_new_privs) {
			ಆದರೆ (prctl(PR_ಗಣ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
				eerrorx("Could not set No New Privs flag: %s", strerror(errno));
		}
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
		ioctl(tty_fd, TIOCNOTTY, 0);
		close(tty_fd);
#ಆದರೆ_ಅಂತ್ಯ

		/* Clean the environment of any RC_ variables */
		env_list = rc_stringlist_new();
		i = 0;
		ಆಗ (environ[i])
			rc_stringlist_ಕೂಡಿಸು(env_list, environ[i++]);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
		ಆದರೆ (changeuser != ಖಾಲಿ) {
			pamenv = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)pam_getenvlist(pamh);
			ಆದರೆ (pamenv) {
				ಆಗ (*pamenv) {
					/* Don't add strings unless they set a var */
					ಆದರೆ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ಇಲ್ಲದಿದ್ದರೆ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#ಆದರೆ_ಅಂತ್ಯ

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(env->ಮೌಲ್ಯ, "RC_", 3) == 0
					|| ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(env->ಮೌಲ್ಯ, "SSD_", 4) == 0
					|| ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(env->ಮೌಲ್ಯ, "EINFO_", 6) == 0) {
				*strchr(env->ಮೌಲ್ಯ, '=') = '\0';
				unsetenv(env->ಮೌಲ್ಯ);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ಆದರೆ ((token = getenv("PATH"))) {
			len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token);
			newpath = np = xmalloc(len + 1);
			ಆಗ (token && *token) {
				p = strchr(token, ':');
				ಆದರೆ (p) {
					*p++ = '\0';
					ಆಗ (*p == ':')
						p++;
				}
				ಆದರೆ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token);
					ಆದರೆ (np != newpath)
						*np++ = ':';
					ಸ್ಮರಣೆ_ನಕಲು(np, token, len);
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
		ಆದರೆ (redirect_stdin) {
			ಆದರೆ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ಆದರೆ (redirect_stdout) {
			ಆದರೆ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ಆದರೆ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ಆದರೆ (redirect_stderr) {
			ಆದರೆ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ಆದರೆ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ಆದರೆ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ಆದರೆ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ಆದರೆ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ಆದರೆ (notify.type == NOTIFY_FD) {
			ಆದರೆ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ಆದರೆ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ಆದರೆ (notify.pipe[1] == notify.fd) {
				ಪೂರ್ಣಾಂಕ flags = fcntl(notify.fd, F_GETFD, 0);
				ಆದರೆ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ಆದರೆ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ಆದರೆ (scheduler != ಖಾಲಿ) {
			ಪೂರ್ಣಾಂಕ scheduler_ಸೂಚ್ಯಂಕ;
			ರಚನೆ sched_param sched =  {.sched_priority = sched_prio};
			ಆದರೆ (strcmp(scheduler, "fifo") == 0)
				scheduler_ಸೂಚ್ಯಂಕ = SCHED_FIFO;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(scheduler, "rr") == 0)
				scheduler_ಸೂಚ್ಯಂಕ = SCHED_RR;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(scheduler, "other") == 0)
				scheduler_ಸೂಚ್ಯಂಕ = SCHED_OTHER;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SCHED_BATCH
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(scheduler, "batch") == 0)
				scheduler_ಸೂಚ್ಯಂಕ = SCHED_BATCH;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SCHED_IDLE
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(scheduler, "idle") == 0)
				scheduler_ಸೂಚ್ಯಂಕ = SCHED_IDLE;
#ಆದರೆ_ಅಂತ್ಯ
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(scheduler, "%d", &scheduler_ಸೂಚ್ಯಂಕ) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ಆದರೆ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_ಸೂಚ್ಯಂಕ);

			ಆದರೆ (pthread_setschedparam(pthread_self(), scheduler_ಸೂಚ್ಯಂಕ, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sched_prio != -1) {
			ಸ್ಥಿರಾಂಕ ರಚನೆ sched_param sched =  {.sched_priority = sched_prio};
			ಆದರೆ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
		ಆದರೆ (changeuser != ಖಾಲಿ && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#ಆದರೆ_ಅಂತ್ಯ
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ಆಗ ((ss = ಪಠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ಆದರೆ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ಆದರೆ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		ಮಾಡು {
			pid = waitpid(spid, &i, 0);
			ಆದರೆ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ಹಿಂತಿರುಗಿ -1;
			}
		} ಆಗ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ಆದರೆ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ಆದರೆ (ಆರಂಭ_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_ಮೌಲ್ಯ("rc_start_wait"))))
	{
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(p, "%u", &ಆರಂಭ_wait) != 1)
			ಆರಂಭ_wait = 0;
	}

	ಆದರೆ (notify.type != NOTIFY_NONE) {
		ಆದರೆ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಆರಂಭ_wait > 0) {
		ರಚನೆ timespec ts;
		bool alive = false;

		ts.tv_sec = ಆರಂಭ_wait / 1000;
		ts.tv_nsec = (ಆರಂಭ_wait % 1000) * ONE_MS;
		ಆದರೆ (nanosleep(&ts, ಖಾಲಿ) == -1) {
			ಆದರೆ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ಹಿಂತಿರುಗಿ 0;
			}
		}
		ಆದರೆ (background) {
			ಆದರೆ (kill(pid, 0) == 0)
				alive = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (pidfile) {
				pid = get_pid(applet, pidfile);
				ಆದರೆ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ಇಲ್ಲದಿದ್ದರೆ
				pid = 0;
			ಆದರೆ (do_stop(applet, exec, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ಆದರೆ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ಆದರೆ (svcname)
		rc_service_daemon_ಗಣ(svcname, exec,
		    (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)margv, pidfile, true);

	ನಿರ್ಗಮಿಸು(ret);
	/* NOTREACHED */
}
