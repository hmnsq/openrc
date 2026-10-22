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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONE_MS           1000000

#ਜੇ defined(__linux__) && !defined(GNU_ਸਰੋਤ)
/* For extra SCHED_* defines. */
# define GNU_ਸਰੋਤ
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <grp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/resource.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <termios.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
ਸਥਿਰ ਬਣਤਰ pam_conv conv = { ਨਿਰਾ, ਨਿਰਾ};
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
# include <sys/capability.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "schedules.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ਗਿਣਤੀ {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_ਅਧਾਰ = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SCHEDULER,
  LONGOPT_SCHEDULER_PRIO,
  LONGOPT_SECBITS,
  LONGOPT_NOTIFY,
};

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "capabilities", 1, ਨਿਰਾ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ਨਿਰਾ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ਨਿਰਾ, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, ਨਿਰਾ, 'I'},
	{ "stop",         0, ਨਿਰਾ, 'K'},
	{ "nicelevel",    1, ਨਿਰਾ, 'N'},
	{ "oom-score-adj",1, ਨਿਰਾ, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, ਨਿਰਾ, 'R'},
	{ "start",        0, ਨਿਰਾ, 'S'},
	{ "startas",      1, ਨਿਰਾ, 'a'},
	{ "background",   0, ਨਿਰਾ, 'b'},
	{ "chuid",        1, ਨਿਰਾ, 'c'},
	{ "chdir",        1, ਨਿਰਾ, 'd'},
	{ "env",          1, ਨਿਰਾ, 'e'},
	{ "umask",        1, ਨਿਰਾ, 'k'},
	{ "group",        1, ਨਿਰਾ, 'g'},
	{ "stop-group",   0, ਨਿਰਾ, 'G'},
	{ "interpreted",  0, ਨਿਰਾ, 'i'},
	{ "make-pidfile", 0, ਨਿਰਾ, 'm'},
	{ "name",         1, ਨਿਰਾ, 'n'},
	{ "oknodo",       0, ਨਿਰਾ, 'o'},
	{ "pidfile",      1, ਨਿਰਾ, 'p'},
	{ "signal",       1, ਨਿਰਾ, 's'},
	{ "test",         0, ਨਿਰਾ, 't'},
	{ "user",         1, ਨਿਰਾ, 'u'},
	{ "chroot",       1, ਨਿਰਾ, 'r'},
	{ "wait",         1, ਨਿਰਾ, 'w'},
	{ "exec",         1, ਨਿਰਾ, 'x'},
	{ "stdin",        1, ਨਿਰਾ, '0'},
	{ "stdout",       1, ਨਿਰਾ, '1'},
	{ "stderr",       1, ਨਿਰਾ, '2'},
	{ "stdout-logger",1, ਨਿਰਾ, '3'},
	{ "stderr-logger",1, ਨਿਰਾ, '4'},
	{ "progress",     0, ਨਿਰਾ, 'P'},
	{ "scheduler",    1, ਨਿਰਾ, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, ਨਿਰਾ, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, ਨਿਰਾ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਸਥਿਰ ਅੱਖਰ **nav;

ਸਥਿਰ ਅੱਖਰ *changeuser, *ch_ਜੜ੍ਹ, *ch_dir;

ਬਾਹਰੀ ਅੱਖਰ **environ;

#ਜੇ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#ਜੇ_ਅੰਤ
#ਜੇ !defined(__DragonFly__)
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ioprio_ਸੈੱਟ(ਪੂਰਨ which RC_ਅਣਵਰਤਿਆ,
			     ਪੂਰਨ who RC_ਅਣਵਰਤਿਆ,
			     ਪੂਰਨ ioprio RC_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS_ioprio_set
	ਵਾਪਸ syscall(SYS_ioprio_set, which, who, ioprio);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ
cleanup(ਖਾਲੀ)
{
	ਮੁਕਤ(changeuser);
	ਮੁਕਤ(nav);
	free_schedulelist();
}

ਸਥਿਰ ਖਾਲੀ
handle_signal(ਪੂਰਨ sig)
{
	ਪੂਰਨ ਸਥਿਤੀ;
	ਪੂਰਨ serrno = errno;
	ਅਚਲ ਅੱਖਰ *signame = ਨਿਰਾ;

	ਚੋਣ (sig) {
	ਹਾਲਤ SIGINT:
		ਜੇ (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	ਹਾਲਤ SIGTERM:
		ਜੇ (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	ਹਾਲਤ SIGQUIT:
		ਜੇ (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	ਹਾਲਤ SIGCHLD:
		ਲਈ (;;) {
			ਜੇ (waitpid(-1, &ਸਥਿਤੀ, WNOHANG) < 0) {
				ਜੇ (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				ਰੋਕੋ;
			}
		}
		ਰੋਕੋ;

	ਮੂਲ:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

ਸਥਿਰ ਅੱਖਰ *
expand_home(ਅਚਲ ਅੱਖਰ *home, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *opath, *ppath, *p, *nh;
	ਬਣਤਰ passwd *pw;

	ਜੇ (!ਰਸਤਾ || *ਰਸਤਾ != '~')
		ਵਾਪਸ xstrdup(ਰਸਤਾ);

	opath = ppath = xstrdup(ਰਸਤਾ);
	ਜੇ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ਜੇ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ਜੇ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ਜੇ (ppath)
				*ppath = '/';
		} ਨਹੀਂਤਾਂ
			home = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ
		ppath++;

	ਜੇ (!home) {
	ਮੁਕਤ(opath);
		ਵਾਪਸ xstrdup(ਰਸਤਾ);
	}
	ਜੇ (!ppath) {
		ਮੁਕਤ(opath);
		ਵਾਪਸ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ਮੁਕਤ(opath);
	ਵਾਪਸ nh;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ devnull_fd = -1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
	ਪੂਰਨ tty_fd = -1;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	pam_handle_t *pamh = ਨਿਰਾ;
	ਪੂਰਨ pamr;
	ਅਚਲ ਅੱਖਰ *ਅਚਲ *pamenv = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	ਪੂਰਨ opt;
	size_t ਮਾਪ = 0;
	bool ਸ਼ੁਰੂ = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	ਅੱਖਰ *exec = ਨਿਰਾ;
	ਅੱਖਰ *startas = ਨਿਰਾ;
	ਅੱਖਰ *name = ਨਿਰਾ;
	ਅੱਖਰ *pidfile = ਨਿਰਾ;
	ਅੱਖਰ *retry = ਨਿਰਾ;
	ਪੂਰਨ sig = -1;
	ਪੂਰਨ nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ਪੂਰਨ oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool ਤਰੱਕੀ = false;
	uid_t uid = 0;
	gid_t gid = 0;
	ਅੱਖਰ *home = ਨਿਰਾ;
	ਪੂਰਨ tid = 0;
	ਅੱਖਰ *redirect_stdin = ਨਿਰਾ;
	ਅੱਖਰ *redirect_stderr = ਨਿਰਾ;
	ਅੱਖਰ *redirect_stdout = ਨਿਰਾ;
	ਅੱਖਰ *stderr_process = ਨਿਰਾ;
	ਅੱਖਰ *stdout_process = ਨਿਰਾ;
	ਪੂਰਨ stdin_fd;
	ਪੂਰਨ stdout_fd;
	ਪੂਰਨ stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ਪੂਰਨ i;
	ਅੱਖਰ *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ਅੱਖਰ *tmp, *newpath, *np;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
	ਅੱਖਰ *exec_file = ਨਿਰਾ;
	ਬਣਤਰ passwd *pw;
	ਬਣਤਰ group *gr;
	bool stopgroup = false;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	ਅੱਖਰ **margv;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਸ਼ੁਰੂ_wait = 0;
	ਅਚਲ ਅੱਖਰ *scheduler = ਨਿਰਾ;
	ਪੂਰਨ sched_prio = -1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	cap_iab_t cap_iab = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ secbits = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PR_ਸੈੱਟ_NO_NEW_PRIVS
	bool no_new_privs = false;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ pipefd[2];
	ਅੱਖਰ readbuf[1];
	ssize_t ss;
	ਬਣਤਰ notify notify = {0};
	ਪੂਰਨ ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_ਤਿਆਰੀ(SIGINT, handle_signal);
	signal_ਤਿਆਰੀ(SIGQUIT, handle_signal);
	signal_ਤਿਆਰੀ(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	ਜੇ ((tmp = getenv("SSD_NICELEVEL")))
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ਜੇ ((tmp = getenv("SSD_IONICELEVEL"))) {
		ਪੂਰਨ n = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d:%d", &ionicec, &ioniced);
		ਜੇ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ਜੇ (ionicec == 0)
			ioniced = 0;
		ਨਹੀਂਤਾਂ ਜੇ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ਜੇ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ਜੇ (home == ਨਿਰਾ || p == ਨਿਰਾ) {
		pw = getpwuid(getuid());
		ਜੇ (pw != ਨਿਰਾ) {
			ਜੇ (p == ਨਿਰਾ)
				setenv("USER", pw->pw_name, 1);
			ਜੇ (home == ਨਿਰਾ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ਪੂਰਨ *) 0)) != -1)
		ਚੋਣ (opt) {
		ਹਾਲਤ LONGOPT_CAPABILITIES:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
			cap_iab = cap_iab_from_ਪਾਠ(optarg);
			ਜੇ (cap_iab == ਨਿਰਾ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ਨਹੀਂਤਾਂ
			eerrorx("Capabilities support not enabled");
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_SECBITS:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
			ਜੇ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ਨਿਰਾ;
			secbits = strtoul(optarg, &tmp, 0);
			ਜੇ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ਨਹੀਂਤਾਂ
			eerrorx("Capabilities support not enabled");
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_NO_NEW_PRIVS:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PR_ਸੈੱਟ_NO_NEW_PRIVS
			no_new_privs = true;
#ਨਹੀਂਤਾਂ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;

		ਹਾਲਤ 'I': /* --ionice */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ਜੇ (ionicec == 0)
				ioniced = 0;
			ਨਹੀਂਤਾਂ ਜੇ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ਰੋਕੋ;

		ਹਾਲਤ 'K':  /* --stop */
			stop = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'G':  /* --stop-group */
			stopgroup = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'N':  /* --nice */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'P':  /* --progress */
			ਤਰੱਕੀ = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'S':  /* --start */
			ਸ਼ੁਰੂ = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'b':  /* --background */
			background = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		ਹਾਲਤ 'u':  /* --user <username>|<uid> */
		{
			ਅੱਖਰ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ਨਹੀਂਤਾਂ
				pw = getpwuid((uid_t)tid);

			ਜੇ (pw == ਨਿਰਾ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ਜੇ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ਜੇ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ਜੇ (gid == 0)
				gid = pw->pw_gid;

			ਜੇ (p) {
				tmp = strsep (&p, ":");
				ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ਨਹੀਂਤਾਂ
					gr = getgrgid((gid_t) tid);

				ਜੇ (gr == ਨਿਰਾ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ਰੋਕੋ;

		ਹਾਲਤ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'e': /* --env */
			putenv(optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'g':  /* --group <group>|<gid> */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ਨਹੀਂਤਾਂ
				gr = getgrgid((gid_t)tid);
			ਜੇ (gr == ਨਿਰਾ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ਰੋਕੋ;

		ਹਾਲਤ 'i': /* --interpreted */
			interpreted = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'k':
			ਜੇ (parse_ਬਹੁਲਕ(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'm':  /* --make-pidfile */
			makepidfile = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'n':  /* --name <process-name> */
			name = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 't':  /* --test */
			test = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'r':  /* --chroot /new/root */
			ch_ਜੜ੍ਹ = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'w':
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%u", &ਸ਼ੁਰੂ_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			ਰੋਕੋ;
		ਹਾਲਤ 'x':  /* --exec <executable> */
			exec = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &sched_prio);
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	ਜੇ (!ਸ਼ੁਰੂ &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	ਜੇ (!exec)
		exec = startas;
	ਨਹੀਂਤਾਂ ਜੇ (!name)
		name = startas;

	ਜੇ (!exec) {
		exec = *argv;
		ਜੇ (!exec)
			exec = name;
		ਜੇ (name && ਸ਼ੁਰੂ)
			*argv = name;
	} ਨਹੀਂਤਾਂ ਜੇ (name) {
		*--argv = name;
		++argc;
	} ਨਹੀਂਤਾਂ ਜੇ (exec) {
		*--argv = exec;
		++argc;
	};

	ਜੇ (stop || sig != -1) {
		ਜੇ (sig == -1)
			sig = SIGTERM;
		ਜੇ (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		ਜੇ (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		ਜੇ (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		ਜੇ (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		ਜੇ (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		ਜੇ (ਸ਼ੁਰੂ_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} ਨਹੀਂਤਾਂ {
		ਜੇ (!exec)
			eerrorx("%s: nothing to start", applet);
		ਜੇ (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		ਜੇ ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		ਜੇ ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		ਜੇ (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		ਜੇ (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	ਜੇ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ਜੇ (ch_ਜੜ੍ਹ && *ch_ਜੜ੍ਹ == '~')
		ch_ਜੜ੍ਹ = expand_home(home, ch_ਜੜ੍ਹ);
	ਜੇ (exec) {
		ਜੇ (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		ਜੇ (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			ਜੇ (ch_ਜੜ੍ਹ)
				xasprintf(&exec_file, "%s/%s", ch_ਜੜ੍ਹ, exec);
			ਨਹੀਂਤਾਂ
				xasprintf(&exec_file, "%s", exec);
		} ਨਹੀਂਤਾਂ {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = ਨਿਰਾ;
			ਜਦੋਂ ((token = strsep(&p, ":"))) {
				ਜੇ (ch_ਜੜ੍ਹ)
					xasprintf(&exec_file, "%s/%s/%s", ch_ਜੜ੍ਹ, token, exec);
				ਨਹੀਂਤਾਂ
					xasprintf(&exec_file, "%s/%s", token, exec);
				ਜੇ (exec_file && access(exec_file, F_OK) == 0)
					ਰੋਕੋ;
				ਮੁਕਤ(exec_file);
				exec_file = ਨਿਰਾ;
			}
			ਮੁਕਤ(tmp);
		}
	}
	ਜੇ (ਸ਼ੁਰੂ && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		ਮੁਕਤ(exec_file);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	}
	ਜੇ (ਸ਼ੁਰੂ && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	ਜੇ (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		ਜੇ (fp) {
			ਸਤਰ = ਨਿਰਾ;
			ਜੇ (xgetline(&ਸਤਰ, &ਮਾਪ, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = ਸਤਰ;
			fclose(fp);
			ਜੇ (p != ਨਿਰਾ && ਸਤਰ[0] == '#' && ਸਤਰ[1] == '!') {
				p = ਸਤਰ + 2;
				/* Strip leading spaces */
				ਜਦੋਂ (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				ਮੁਕਤ(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				ਲਈ (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(ਆਕਾਰ(ਅੱਖਰ *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				ਜੇ (p)
					nav[len++] = p;
				ਲਈ (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = ਨਿਰਾ;
			}
		}
	}
	margv = nav ? nav : argv;

	ਜੇ (stop || sig != -1) {
		ਜੇ (sig == -1)
			sig = SIGTERM;
		ਜੇ (!stop)
			oknodo = true;
		ਜੇ (retry)
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, retry, sig);
		ਨਹੀਂਤਾਂ ਜੇ (test || oknodo)
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, "0", sig);
		ਨਹੀਂਤਾਂ
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, ਨਿਰਾ, sig);
		ਜੇ (pidfile) {
			pid = get_pid(applet, pidfile);
			ਜੇ (pid == -1 && errno != ENOENT)
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
		} ਨਹੀਂਤਾਂ {
			pid = 0;
		}
		i = ਲੜੀਵਾਰ_stop_ਸਮਾਂ_ਸਾਰਣੀ(applet, exec, (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)margv,
		    pid, uid, stopgroup, test, ਤਰੱਕੀ, false);

		ਜੇ (i < 0)
			/* We failed to stop something */
			ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
		ਜੇ (test || oknodo)
			ਵਾਪਸ i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ਜੇ (pidfile)
			unlink(pidfile);
		ਜੇ (svcname)
			rc_service_daemon_ਸੈੱਟ(svcname, exec,
			    (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)argv,
			    pidfile, false);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	}

	ਜੇ (pidfile)
		pid = get_pid(applet, pidfile);
	ਨਹੀਂਤਾਂ
		pid = 0;

	ਜੇ (pid)
		pids = rc_ਲੱਭੋ_pids(ਨਿਰਾ, ਨਿਰਾ, 0, pid);
	ਨਹੀਂਤਾਂ
		pids = rc_ਲੱਭੋ_pids(exec, (ਅਚਲ ਅੱਖਰ * ਅਚਲ *) argv, uid, 0);
	ਜੇ (pids)
		eerrorx("%s: %s is already running", applet, exec);

	ਮੁਕਤ(pids);
	ਜੇ (test) {
		ਜੇ (rc_yesno(getenv("EINFO_QUIET")))
			ਬਾਹਰ_ਨਿਕਲੋ (EXIT_SUCCESS);

		einfon("Would start");
		ਜਦੋਂ (argc-- > 0)
			ਲਿਖੋ(" %s", *argv++);
		ਲਿਖੋ("\n");
		eindent();
		ਜੇ (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		ਜੇ (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		ਜੇ (ch_ਜੜ੍ਹ)
			einfo("in root `%s'", ch_ਜੜ੍ਹ);
		ਜੇ (ch_dir)
			einfo("in dir `%s'", ch_dir);
		ਜੇ (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		ਜੇ (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	ਜੇ (pidfile)
		unlink(pidfile);

	ਜੇ (background)
		signal_ਤਿਆਰੀ(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	ਜੇ (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	ਜੇ ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	ਜੇ (pid == 0) {
		gid_t ਸਮੂਹ_buf[32], *ਸਮੂਹ_list = ਸਮੂਹ_buf;
		ਪੂਰਨ ਸਮੂਹ_ਗਿਣਤੀ = ARRAY_ਮਾਪ(ਸਮੂਹ_buf);
		pid_t mypid = getpid();
		close(pipefd[0]); /* Close the read end of the pipe. */
		umask(numask);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#ਜੇ_ਅੰਤ

		devnull_fd = open("/dev/null", O_RDWR);

		/* Must call setsid() before setting autogroup nicelevel
		 * but after opening tty_fd. */
		setsid();

		ਜੇ (nicelevel != INT_MIN) {
			ਜੇ (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			ਜੇ (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} ਨਹੀਂਤਾਂ ਜੇ (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		ਜੇ (ionicec != -1 &&
		    ioprio_ਸੈੱਟ(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		ਜੇ (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			ਜੇ (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
		ਜੇ (changeuser != ਨਿਰਾ) {
			pamr = pam_ਸ਼ੁਰੂ("start-stop-daemon",
			    changeuser, &conv, &pamh);

			ਜੇ (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			ਜੇ (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			ਜੇ (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#ਜੇ_ਅੰਤ
		ਜੇ (changeuser && getgrouplist(changeuser, gid, ਸਮੂਹ_list, &ਸਮੂਹ_ਗਿਣਤੀ) < 0) {
			ਸਮੂਹ_list = xmalloc(ਸਮੂਹ_ਗਿਣਤੀ * ਆਕਾਰ(*ਸਮੂਹ_list));
			ਜੇ (getgrouplist(changeuser, gid, ਸਮੂਹ_list, &ਸਮੂਹ_ਗਿਣਤੀ) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		ਜੇ (ch_ਜੜ੍ਹ && chroot(ch_ਜੜ੍ਹ) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_ਜੜ੍ਹ, strerror(errno));

		ਜੇ (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		ਜੇ (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			ਜੇ (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		ਜੇ (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		ਜੇ (changeuser && setgroups(ਸਮੂਹ_ਗਿਣਤੀ, ਸਮੂਹ_list))
			eerrorx("%s: setgroups() failed", applet);
		ਜੇ (ਸਮੂਹ_list != ਸਮੂਹ_buf)
			ਮੁਕਤ(ਸਮੂਹ_list);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
		ਜੇ (uid && cap_setuid(uid))
#ਨਹੀਂਤਾਂ
		ਜੇ (uid && setuid(uid))
#ਜੇ_ਅੰਤ
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
		ਜੇ (cap_iab != ਨਿਰਾ) {
			i = cap_iab_ਸੈੱਟ_proc(cap_iab);

			ਜੇ (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			ਜੇ (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		ਜੇ (secbits != 0) {
			ਜੇ (cap_ਸੈੱਟ_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PR_ਸੈੱਟ_NO_NEW_PRIVS
		ਜੇ (no_new_privs) {
			ਜੇ (prctl(PR_ਸੈੱਟ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
				eerrorx("Could not set No New Privs flag: %s", strerror(errno));
		}
#ਜੇ_ਅੰਤ


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
		ioctl(tty_fd, TIOCNOTTY, 0);
		close(tty_fd);
#ਜੇ_ਅੰਤ

		/* Clean the environment of any RC_ variables */
		env_list = rc_stringlist_new();
		i = 0;
		ਜਦੋਂ (environ[i])
			rc_stringlist_ਜੋੜੋ(env_list, environ[i++]);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
		ਜੇ (changeuser != ਨਿਰਾ) {
			pamenv = (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)pam_getenvlist(pamh);
			ਜੇ (pamenv) {
				ਜਦੋਂ (*pamenv) {
					/* Don't add strings unless they set a var */
					ਜੇ (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					ਨਹੀਂਤਾਂ
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#ਜੇ_ਅੰਤ

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			ਜੇ (ਲੜੀ_ਤੁਲਨਾ(env->ਮੁੱਲ, "RC_", 3) == 0
					|| ਲੜੀ_ਤੁਲਨਾ(env->ਮੁੱਲ, "SSD_", 4) == 0
					|| ਲੜੀ_ਤੁਲਨਾ(env->ਮੁੱਲ, "EINFO_", 6) == 0) {
				*strchr(env->ਮੁੱਲ, '=') = '\0';
				unsetenv(env->ਮੁੱਲ);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		ਜੇ ((token = getenv("PATH"))) {
			len = ਲੜੀ_ਲੰਬਾਈ(token);
			newpath = np = xmalloc(len + 1);
			ਜਦੋਂ (token && *token) {
				p = strchr(token, ':');
				ਜੇ (p) {
					*p++ = '\0';
					ਜਦੋਂ (*p == ':')
						p++;
				}
				ਜੇ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = ਲੜੀ_ਲੰਬਾਈ(token);
					ਜੇ (np != newpath)
						*np++ = ':';
					ਸਮ੍ਰਿਤੀ_ਨਕਲ(np, token, len);
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
		ਜੇ (redirect_stdin) {
			ਜੇ ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		ਜੇ (redirect_stdout) {
			ਜੇ ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}ਨਹੀਂਤਾਂ ਜੇ (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			ਜੇ (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		ਜੇ (redirect_stderr) {
			ਜੇ ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}ਨਹੀਂਤਾਂ ਜੇ (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			ਜੇ (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		ਜੇ (background)
			dup2(stdin_fd, STDIN_FILENO);
		ਜੇ (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		ਜੇ (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		ਜੇ (notify.type == NOTIFY_FD) {
			ਜੇ (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			ਜੇ (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			ਜੇ (notify.pipe[1] == notify.fd) {
				ਪੂਰਨ flags = fcntl(notify.fd, F_GETFD, 0);
				ਜੇ (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				ਜੇ (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		ਜੇ (scheduler != ਨਿਰਾ) {
			ਪੂਰਨ scheduler_ਸੂਚਕਾਂਕ;
			ਬਣਤਰ sched_param sched =  {.sched_priority = sched_prio};
			ਜੇ (strcmp(scheduler, "fifo") == 0)
				scheduler_ਸੂਚਕਾਂਕ = SCHED_FIFO;
			ਨਹੀਂਤਾਂ ਜੇ (strcmp(scheduler, "rr") == 0)
				scheduler_ਸੂਚਕਾਂਕ = SCHED_RR;
			ਨਹੀਂਤਾਂ ਜੇ (strcmp(scheduler, "other") == 0)
				scheduler_ਸੂਚਕਾਂਕ = SCHED_OTHER;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SCHED_BATCH
			ਨਹੀਂਤਾਂ ਜੇ (strcmp(scheduler, "batch") == 0)
				scheduler_ਸੂਚਕਾਂਕ = SCHED_BATCH;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SCHED_IDLE
			ਨਹੀਂਤਾਂ ਜੇ (strcmp(scheduler, "idle") == 0)
				scheduler_ਸੂਚਕਾਂਕ = SCHED_IDLE;
#ਜੇ_ਅੰਤ
			ਨਹੀਂਤਾਂ ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(scheduler, "%d", &scheduler_ਸੂਚਕਾਂਕ) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			ਜੇ (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_ਸੂਚਕਾਂਕ);

			ਜੇ (pthread_setschedparam(pthread_self(), scheduler_ਸੂਚਕਾਂਕ, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} ਨਹੀਂਤਾਂ ਜੇ (sched_prio != -1) {
			ਅਚਲ ਬਣਤਰ sched_param sched =  {.sched_priority = sched_prio};
			ਜੇ (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
		ਜੇ (changeuser != ਨਿਰਾ && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#ਜੇ_ਅੰਤ
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	ਜਦੋਂ ((ss = ਪਠ(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	ਜੇ (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	ਜੇ (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		ਕਰੋ {
			pid = waitpid(spid, &i, 0);
			ਜੇ (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				ਵਾਪਸ -1;
			}
		} ਜਦੋਂ (!WIFEXITED(i) && !WIFSIGNALED(i));
		ਜੇ (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	ਜੇ (ਸ਼ੁਰੂ_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_ਮੁੱਲ("rc_start_wait"))))
	{
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(p, "%u", &ਸ਼ੁਰੂ_wait) != 1)
			ਸ਼ੁਰੂ_wait = 0;
	}

	ਜੇ (notify.type != NOTIFY_NONE) {
		ਜੇ (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} ਨਹੀਂਤਾਂ ਜੇ (ਸ਼ੁਰੂ_wait > 0) {
		ਬਣਤਰ timespec ts;
		bool alive = false;

		ts.tv_sec = ਸ਼ੁਰੂ_wait / 1000;
		ts.tv_nsec = (ਸ਼ੁਰੂ_wait % 1000) * ONE_MS;
		ਜੇ (nanosleep(&ts, ਨਿਰਾ) == -1) {
			ਜੇ (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				ਵਾਪਸ 0;
			}
		}
		ਜੇ (background) {
			ਜੇ (kill(pid, 0) == 0)
				alive = true;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (pidfile) {
				pid = get_pid(applet, pidfile);
				ਜੇ (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} ਨਹੀਂਤਾਂ
				pid = 0;
			ਜੇ (do_stop(applet, exec, (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		ਜੇ (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	ਜੇ (svcname)
		rc_service_daemon_ਸੈੱਟ(svcname, exec,
		    (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)margv, pidfile, true);

	ਬਾਹਰ_ਨਿਕਲੋ(ret);
	/* NOTREACHED */
}
