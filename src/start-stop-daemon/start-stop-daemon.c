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

#якщо defined(__linux__) && !defined(_GNU_SOURCE)
/* For extra SCHED_* defines. */
# define _GNU_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <inttypes.h>
#include <grp.h>
#include <pthread.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <inttypes.h>
#include <sys/wait.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#include <termios.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_PAM
# include <security/pam_appl.h>

/* We are not supporting authentication conversations */
static struct pam_conv conv = { NULL, NULL};
#endif

#ifdef __linux__
# include <sys/capability.h>
#endif

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "rc_exec.h"
#include "schedules.h"
#include "_usage.h"
#include "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
enum {
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

const char *applet = NULL;
const char *extraopts = NULL;
const char getoptstring[] = "I:KN:PR:Sa:bc:d:e:Gg:ik:mn:op:s:tu:r:w:x:0:1:2:3:4:" \
	getoptstring_COMMON;
const struct option longopts[] = {
	{ "capabilities", 1, NULL, LONGOPT_CAPABILITIES},
	{ "secbits",      1, NULL, LONGOPT_SECBITS},
	{ "no-new-privs", 0, NULL, LONGOPT_NO_NEW_PRIVS},
	{ "ionice",       1, NULL, 'I'},
	{ "stop",         0, NULL, 'K'},
	{ "nicelevel",    1, NULL, 'N'},
	{ "oom-score-adj",1, NULL, LONGOPT_OOM_SCORE_ADJ},
	{ "retry",        1, NULL, 'R'},
	{ "start",        0, NULL, 'S'},
	{ "startas",      1, NULL, 'a'},
	{ "background",   0, NULL, 'b'},
	{ "chuid",        1, NULL, 'c'},
	{ "chdir",        1, NULL, 'd'},
	{ "env",          1, NULL, 'e'},
	{ "umask",        1, NULL, 'k'},
	{ "group",        1, NULL, 'g'},
	{ "stop-group",   0, NULL, 'G'},
	{ "interpreted",  0, NULL, 'i'},
	{ "make-pidfile", 0, NULL, 'm'},
	{ "name",         1, NULL, 'n'},
	{ "oknodo",       0, NULL, 'o'},
	{ "pidfile",      1, NULL, 'p'},
	{ "signal",       1, NULL, 's'},
	{ "test",         0, NULL, 't'},
	{ "user",         1, NULL, 'u'},
	{ "chroot",       1, NULL, 'r'},
	{ "wait",         1, NULL, 'w'},
	{ "exec",         1, NULL, 'x'},
	{ "stdin",        1, NULL, '0'},
	{ "stdout",       1, NULL, '1'},
	{ "stderr",       1, NULL, '2'},
	{ "stdout-logger",1, NULL, '3'},
	{ "stderr-logger",1, NULL, '4'},
	{ "progress",     0, NULL, 'P'},
	{ "scheduler",    1, NULL, LONGOPT_SCHEDULER},
	{ "scheduler-priority",    1, NULL, LONGOPT_SCHEDULER_PRIO},
	{ "notify",        1, NULL, LONGOPT_NOTIFY},
	longopts_COMMON
};
const char * const longopts_help[] = {
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
const char *usagestring = NULL;

static char **nav;

static char *changeuser, *ch_root, *ch_dir;

extern char **environ;

#якщо !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#якщо !defined(__DragonFly__)
static inline ціле ioprio_set(ціле which RC_UNUSED,
			     ціле who RC_UNUSED,
			     ціле ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	повернути syscall(SYS_ioprio_set, which, who, ioprio);
#інакше
	повернути 0;
#endif
}
#endif

static void
cleanup(void)
{
	free(changeuser);
	free(nav);
	free_schedulelist();
}

static void
handle_signal(ціле sig)
{
	ціле status;
	ціле serrno = errno;
	const char *signame = NULL;

	switch (sig) {
	case SIGINT:
		якщо (!signame)
			signame = "SIGINT";
		/* FALLTHROUGH */
	case SIGTERM:
		якщо (!signame)
			signame = "SIGTERM";
		/* FALLTHROUGH */
	case SIGQUIT:
		якщо (!signame)
			signame = "SIGQUIT";
		eerrorx("%s: caught %s, aborting", applet, signame);
		/* NOTREACHED */

	case SIGCHLD:
		для (;;) {
			якщо (waitpid(-1, &status, WNOHANG) < 0) {
				якщо (errno != ECHILD)
					eerror("%s: waitpid: %s",
					    applet, strerror(errno));
				break;
			}
		}
		break;

	default:
		eerror("%s: caught unknown signal %d", applet, sig);
	}

	/* Restore errno */
	errno = serrno;
}

static char *
expand_home(const char *home, const char *path)
{
	char *opath, *ppath, *p, *nh;
	struct passwd *pw;

	якщо (!path || *path != '~')
		повернути xstrdup(path);

	opath = ppath = xstrdup(path);
	якщо (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		якщо (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		якщо (pw) {
			home = pw->pw_dir;
			ppath = p;
			якщо (ppath)
				*ppath = '/';
		} інакше
			home = NULL;
	} інакше
		ppath++;

	якщо (!home) {
	free(opath);
		повернути xstrdup(path);
	}
	якщо (!ppath) {
		free(opath);
		повернути xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	повернути nh;
}

ціле main(ціле argc, char **argv)
{
	ціле devnull_fd = -1;
#ifdef TIOCNOTTY
	ціле tty_fd = -1;
#endif

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	ціле pamr;
	const char *const *pamenv = NULL;
#endif

	ціле opt;
	size_t size = 0;
	bool start = false;
	bool stop = false;
	bool oknodo = false;
	bool test = false;
	char *exec = NULL;
	char *startas = NULL;
	char *name = NULL;
	char *pidfile = NULL;
	char *retry = NULL;
	ціле sig = -1;
	ціле nicelevel = INT_MIN, ionicec = -1, ioniced = 0;
	ціле oom_score_adj = INT_MIN;
	bool background = false;
	bool makepidfile = false;
	bool interpreted = false;
	bool progress = false;
	uid_t uid = 0;
	gid_t gid = 0;
	char *home = NULL;
	ціле tid = 0;
	char *redirect_stdin = NULL;
	char *redirect_stderr = NULL;
	char *redirect_stdout = NULL;
	char *stderr_process = NULL;
	char *stdout_process = NULL;
	ціле stdin_fd;
	ціле stdout_fd;
	ціле stderr_fd;
	pid_t pid, spid;
	RC_PIDLIST *pids;
	ціле i;
	char *svcname = getenv("RC_SVCNAME");
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	char *tmp, *newpath, *np;
	char *p;
	char *token;
	char *exec_file = NULL;
	struct passwd *pw;
	struct group *gr;
	bool stopgroup = false;
	char *line = NULL;
	FILE *fp;
	size_t len;
	mode_t numask = 022;
	char **margv;
	unsigned ціле start_wait = 0;
	const char *scheduler = NULL;
	ціле sched_prio = -1;
#ifdef __linux__
	cap_iab_t cap_iab = NULL;
	unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
	bool no_new_privs = false;
#endif
	ціле pipefd[2];
	char readbuf[1];
	ssize_t ss;
	struct notify notify = {0};
	ціле ret = EXIT_SUCCESS;

	applet = basename_c(argv[0]);
	atexit(cleanup);

	signal_setup(SIGINT, handle_signal);
	signal_setup(SIGQUIT, handle_signal);
	signal_setup(SIGTERM, handle_signal);

	openlog(applet, LOG_PID, LOG_DAEMON);

	якщо ((tmp = getenv("SSD_NICELEVEL")))
		якщо (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	якщо ((tmp = getenv("SSD_IONICELEVEL"))) {
		ціле n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		якщо (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		якщо (ionicec == 0)
			ioniced = 0;
		інакше якщо (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	якщо ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		якщо (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	якщо (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		якщо (pw != NULL) {
			якщо (p == NULL)
				setenv("USER", pw->pw_name, 1);
			якщо (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	поки ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ціле *) 0)) != -1)
		switch (opt) {
		case LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			якщо (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#інакше
			eerrorx("Capabilities support not enabled");
#endif
			break;

		case LONGOPT_SECBITS:
#ifdef __linux__
			якщо (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			якщо (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#інакше
			eerrorx("Capabilities support not enabled");
#endif
			break;

		case LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#інакше
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			break;

		case 'I': /* --ionice */
			якщо (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			якщо (ionicec == 0)
				ioniced = 0;
			інакше якщо (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			break;

		case 'K':  /* --stop */
			stop = true;
			break;

		case 'G':  /* --stop-group */
			stopgroup = true;
			break;

		case 'N':  /* --nice */
			якщо (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			break;

		case LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			якщо (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			break;

		case 'P':  /* --progress */
			progress = true;
			break;

		case 'R':  /* --retry <schedule>|<timeout> */
			retry = optarg;
			break;

		case 'S':  /* --start */
			start = true;
			break;

		case 'b':  /* --background */
			background = true;
			break;

		case 'c':  /* --chuid <username>|<uid> */
			/* DEPRECATED */
			ewarn("WARNING: -c/--chuid is deprecated and will be removed in the future, please use -u/--user instead");
			/* falls through */
		case 'u':  /* --user <username>|<uid> */
		{
			char dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			якщо (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			інакше
				pw = getpwuid((uid_t)tid);

			якщо (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			якщо (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			якщо (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			якщо (gid == 0)
				gid = pw->pw_gid;

			якщо (p) {
				tmp = strsep (&p, ":");
				якщо (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				інакше
					gr = getgrgid((gid_t) tid);

				якщо (gr == NULL)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		break;

		case 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			break;

		case 'e': /* --env */
			putenv(optarg);
			break;

		case 'g':  /* --group <group>|<gid> */
			якщо (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			інакше
				gr = getgrgid((gid_t)tid);
			якщо (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			break;

		case 'i': /* --interpreted */
			interpreted = true;
			break;

		case 'k':
			якщо (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			break;

		case 'm':  /* --make-pidfile */
			makepidfile = true;
			break;

		case 'n':  /* --name <process-name> */
			name = optarg;
			break;

		case 'o':  /* --oknodo */
			/* DEPRECATED */
			ewarn("WARNING: -o/--oknodo is deprecated and will be removed in the future");
			oknodo = true;
			break;

		case 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			break;

		case 's':  /* --signal <signal> */
			sig = parse_signal(applet, optarg);
			break;

		case 't':  /* --test */
			test = true;
			break;

		case 'r':  /* --chroot /new/root */
			ch_root = optarg;
			break;

		case 'a': /* --startas <name> */
			/* DEPRECATED */
			ewarn("WARNING: -a/--startas is deprecated and will be removed in the future, please use -x/--exec or -n/--name instead");
			startas = optarg;
			break;
		case 'w':
			якщо (sscanf(optarg, "%u", &start_wait) != 1)
				eerrorx("%s: `%s' not a number",
				    applet, optarg);
			break;
		case 'x':  /* --exec <executable> */
			exec = optarg;
			break;

		case '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			break;

		case '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			break;

		case '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			break;

		case '3':   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			break;

		case '4':  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			break;

		case LONGOPT_SCHEDULER: /* --scheduler "Process scheduler policy" */
			scheduler = optarg;
			break;

		case LONGOPT_SCHEDULER_PRIO: /* --scheduler-priority "Process scheduler priority" */
			sscanf(optarg, "%d", &sched_prio);
			break;

		case LONGOPT_NOTIFY:
			notify = notify_parse(svcname ? svcname : applet, optarg);
			break;

		case_RC_COMMON_GETOPT
		}

	endpwent();
	argc -= optind;
	argv += optind;

	/* Allow start-stop-daemon --signal HUP --exec /usr/sbin/dnsmasq
	 * instead of forcing --stop --oknodo as well */
	якщо (!start &&
	    !stop &&
	    sig != SIGINT &&
	    sig != SIGTERM &&
	    sig != SIGQUIT &&
	    sig != SIGKILL)
		oknodo = true;

	якщо (!exec)
		exec = startas;
	інакше якщо (!name)
		name = startas;

	якщо (!exec) {
		exec = *argv;
		якщо (!exec)
			exec = name;
		якщо (name && start)
			*argv = name;
	} інакше якщо (name) {
		*--argv = name;
		++argc;
	} інакше якщо (exec) {
		*--argv = exec;
		++argc;
	};

	якщо (stop || sig != -1) {
		якщо (sig == -1)
			sig = SIGTERM;
		якщо (!*argv && !pidfile && !name && !uid)
			eerrorx("%s: --stop needs --exec, --pidfile,"
			    " --name or --user", applet);
		якщо (background)
			eerrorx("%s: --background is only relevant with"
			    " --start", applet);
		якщо (makepidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --start", applet);
		якщо (redirect_stdout || redirect_stderr)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --start", applet);
		якщо (stdout_process || stderr_process)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --start", applet);
		якщо (start_wait)
			ewarn("using --wait with --stop has no effect,"
			    " use --retry instead");
	} інакше {
		якщо (!exec)
			eerrorx("%s: nothing to start", applet);
		якщо (makepidfile && !pidfile)
			eerrorx("%s: --make-pidfile is only relevant with"
			    " --pidfile", applet);
		якщо ((redirect_stdout || redirect_stderr) && !background)
			eerrorx("%s: --stdout and --stderr are only relevant"
			    " with --background", applet);
		якщо ((stdout_process || stderr_process) && !background)
			eerrorx("%s: --stdout-logger and --stderr-logger are only relevant"
			    " with --background", applet);
		якщо (redirect_stdout && stdout_process)
			eerrorx("%s: do not use --stdout and --stdout-logger together",
					applet);
		якщо (redirect_stderr && stderr_process)
			eerrorx("%s: do not use --stderr and --stderr-logger together",
					applet);
	}

	/* Expand ~ */
	якщо (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	якщо (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);
	якщо (exec) {
		якщо (*exec == '~')
			exec = expand_home(home, exec);

		/* Validate that the binary exists if we are starting */
		якщо (*exec == '/' || *exec == '.') {
			/* Full or relative path */
			якщо (ch_root)
				xasprintf(&exec_file, "%s/%s", ch_root, exec);
			інакше
				xasprintf(&exec_file, "%s", exec);
		} інакше {
			/* Something in $PATH */
			p = tmp = xstrdup(getenv("PATH"));
			exec_file = NULL;
			поки ((token = strsep(&p, ":"))) {
				якщо (ch_root)
					xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
				інакше
					xasprintf(&exec_file, "%s/%s", token, exec);
				якщо (exec_file && access(exec_file, F_OK) == 0)
					break;
				free(exec_file);
				exec_file = NULL;
			}
			free(tmp);
		}
	}
	якщо (start && access(exec_file, F_OK) != 0) {
		eerror("%s: %s does not exist", applet,
		    exec_file ? exec_file : exec);
		free(exec_file);
		exit(EXIT_FAILURE);
	}
	якщо (start && retry)
		ewarn("using --retry with --start has no effect,"
		    " use --wait instead");

	/* If we don't have a pidfile we should check if it's interpreted
	 * or not. If it we, we need to pass the interpreter through
	 * to our daemon calls to find it correctly. */
	якщо (interpreted && !pidfile) {
		fp = fopen(exec_file, "r");
		якщо (fp) {
			line = NULL;
			якщо (xgetline(&line, &size, fp) == -1)
				eerrorx("%s: %s", applet, strerror(errno));
			p = line;
			fclose(fp);
			якщо (p != NULL && line[0] == '#' && line[1] == '!') {
				p = line + 2;
				/* Strip leading spaces */
				поки (*p == ' ' || *p == '\t')
					p++;
				token = strsep(&p, " ");
				free(exec_file);
				xasprintf(&exec_file, "%s", token);
				opt = 0;
				для (nav = argv; *nav; nav++)
					opt++;
				nav = xmalloc(sizeof(char *) * (opt + 3));
				nav[0] = exec_file;
				len = 1;
				якщо (p)
					nav[len++] = p;
				для (i = 0; i < opt; i++)
					nav[i + len] = argv[i];
				nav[i + len] = NULL;
			}
		}
	}
	margv = nav ? nav : argv;

	якщо (stop || sig != -1) {
		якщо (sig == -1)
			sig = SIGTERM;
		якщо (!stop)
			oknodo = true;
		якщо (retry)
			parse_schedule(applet, retry, sig);
		інакше якщо (test || oknodo)
			parse_schedule(applet, "0", sig);
		інакше
			parse_schedule(applet, NULL, sig);
		якщо (pidfile) {
			pid = get_pid(applet, pidfile);
			якщо (pid == -1 && errno != ENOENT)
				exit(EXIT_FAILURE);
		} інакше {
			pid = 0;
		}
		i = run_stop_schedule(applet, exec, (const char *const *)margv,
		    pid, uid, stopgroup, test, progress, false);

		якщо (i < 0)
			/* We failed to stop something */
			exit(EXIT_FAILURE);
		якщо (test || oknodo)
			повернути i > 0 ? EXIT_SUCCESS : EXIT_FAILURE;

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		якщо (pidfile)
			unlink(pidfile);
		якщо (svcname)
			rc_service_daemon_set(svcname, exec,
			    (const char *const *)argv,
			    pidfile, false);
		exit(EXIT_SUCCESS);
	}

	якщо (pidfile)
		pid = get_pid(applet, pidfile);
	інакше
		pid = 0;

	якщо (pid)
		pids = rc_find_pids(NULL, NULL, 0, pid);
	інакше
		pids = rc_find_pids(exec, (const char * const *) argv, uid, 0);
	якщо (pids)
		eerrorx("%s: %s is already running", applet, exec);

	free(pids);
	якщо (test) {
		якщо (rc_yesno(getenv("EINFO_QUIET")))
			exit (EXIT_SUCCESS);

		einfon("Would start");
		поки (argc-- > 0)
			printf(" %s", *argv++);
		printf("\n");
		eindent();
		якщо (uid != 0)
			einfo("as user id %"PRIuMAX, (uintmax_t)uid);
		якщо (gid != 0)
			einfo("as group id %"PRIuMAX, (uintmax_t)gid);
		якщо (ch_root)
			einfo("in root `%s'", ch_root);
		якщо (ch_dir)
			einfo("in dir `%s'", ch_dir);
		якщо (nicelevel != 0)
			einfo("with a priority of %d", nicelevel);
		якщо (name)
			einfo ("with a process name of %s", name);
		eoutdent();
		exit(EXIT_SUCCESS);
	}

	ebeginv("Detaching to start `%s'", exec);
	eindentv();

	/* Remove existing pidfile */
	якщо (pidfile)
		unlink(pidfile);

	якщо (background)
		signal_setup(SIGCHLD, handle_signal);

	/* Use a pipe to sync the parent/child processes. */
	якщо (pipe2(pipefd, O_CLOEXEC) == -1)
		eerrorx("%s: pipe2: %s", applet, strerror(errno));

	якщо ((pid = fork()) == -1)
		eerrorx("%s: fork: %s", applet, strerror(errno));

	/* Child process - lets go! */
	якщо (pid == 0) {
		gid_t group_buf[32], *group_list = group_buf;
		ціле group_count = ARRAY_SIZE(group_buf);
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

		якщо (nicelevel != INT_MIN) {
			якщо (setpriority(PRIO_PROCESS, mypid, nicelevel) == -1)
				eerrorx("%s: setpriority %d: %s",
				    applet, nicelevel,
				    strerror(errno));
			/* Open in "r+" mode to avoid creating if non-existent. */
			fp = fopen("/proc/self/autogroup", "r+");
			якщо (fp) {
				fprintf(fp, "%d\n", nicelevel);
				fclose(fp);
			} інакше якщо (errno != ENOENT)
				eerrorx("%s: autogroup nice %d: %s", applet,
				    nicelevel, strerror(errno));
		}

		якщо (ionicec != -1 &&
		    ioprio_set(1, mypid, ionicec | ioniced) == -1)
			eerrorx("%s: ioprio_set %d %d: %s", applet,
			    ionicec, ioniced, strerror(errno));

		якщо (oom_score_adj != INT_MIN) {
			fp = fopen("/proc/self/oom_score_adj", "w");
			якщо (!fp)
				eerrorx("%s: oom_score_adj %d: %s", applet,
				    oom_score_adj, strerror(errno));
			fprintf(fp, "%d\n", oom_score_adj);
			fclose(fp);
		}

#ifdef HAVE_PAM
		якщо (changeuser != NULL) {
			pamr = pam_start("start-stop-daemon",
			    changeuser, &conv, &pamh);

			якщо (pamr == PAM_SUCCESS)
				pamr = pam_acct_mgmt(pamh, PAM_SILENT);
			якщо (pamr == PAM_SUCCESS)
				pamr = pam_open_session(pamh, PAM_SILENT);
			якщо (pamr != PAM_SUCCESS)
				eerrorx("%s: pam error: %s",
					applet, pam_strerror(pamh, pamr));
		}
#endif
		якщо (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
			group_list = xmalloc(group_count * sizeof(*group_list));
			якщо (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
				eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
		}

		/* Close any fd's to the passwd database */
		endpwent();

		якщо (ch_root && chroot(ch_root) < 0)
			eerrorx("%s: chroot `%s': %s",
			    applet, ch_root, strerror(errno));

		якщо (ch_dir && chdir(ch_dir) < 0)
			eerrorx("%s: chdir `%s': %s",
			    applet, ch_dir, strerror(errno));

		якщо (makepidfile && pidfile) {
			fp = fopen(pidfile, "w");
			якщо (!fp)
				eerrorx("%s: fopen `%s': %s", applet, pidfile,
				    strerror(errno));
			fprintf(fp, "%d\n", mypid);
			fclose(fp);
		}


		якщо (gid && setgid(gid))
			eerrorx("%s: unable to set groupid to %"PRIuMAX,
			    applet, (uintmax_t)gid);
		якщо (changeuser && setgroups(group_count, group_list))
			eerrorx("%s: setgroups() failed", applet);
		якщо (group_list != group_buf)
			free(group_list);
#ifdef __linux__
		якщо (uid && cap_setuid(uid))
#інакше
		якщо (uid && setuid(uid))
#endif
			eerrorx ("%s: unable to set userid to %"PRIuMAX,
			    applet, (uintmax_t)uid);

#ifdef __linux__
		якщо (cap_iab != NULL) {
			i = cap_iab_set_proc(cap_iab);

			якщо (cap_free(cap_iab) != 0)
				eerrorx("Could not releasable memory: %s", strerror(errno));

			якщо (i != 0)
				eerrorx("Could not set iab: %s", strerror(errno));
		}

		якщо (secbits != 0) {
			якщо (cap_set_secbits(secbits) < 0)
				eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
		}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
		якщо (no_new_privs) {
			якщо (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
		поки (environ[i])
			rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
		якщо (changeuser != NULL) {
			pamenv = (const char *const *)pam_getenvlist(pamh);
			якщо (pamenv) {
				поки (*pamenv) {
					/* Don't add strings unless they set a var */
					якщо (strchr(*pamenv, '='))
						putenv(xstrdup(*pamenv));
					інакше
						unsetenv(*pamenv);
					pamenv++;
				}
			}
		}
#endif

		unsetenv("SVCNAME");
		TAILQ_FOREACH(env, env_list, entries) {
			якщо (strncmp(env->value, "RC_", 3) == 0
					|| strncmp(env->value, "SSD_", 4) == 0
					|| strncmp(env->value, "EINFO_", 6) == 0) {
				*strchr(env->value, '=') = '\0';
				unsetenv(env->value);
			}
		}
		rc_stringlist_free(env_list);

		/* For the path, remove the rcscript bin dir from it */
		якщо ((token = getenv("PATH"))) {
			len = strlen(token);
			newpath = np = xmalloc(len + 1);
			поки (token && *token) {
				p = strchr(token, ':');
				якщо (p) {
					*p++ = '\0';
					поки (*p == ':')
						p++;
				}
				якщо (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
				    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
				{
					len = strlen(token);
					якщо (np != newpath)
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
		якщо (redirect_stdin) {
			якщо ((stdin_fd = xopen_nonblock(redirect_stdin,
				    O_RDONLY, 0)) == -1)
				eerrorx("%s: unable to redirect the input file"
				    " for stdin `%s': %s",
				    applet, redirect_stdin, strerror(errno));
		}
		якщо (redirect_stdout) {
			якщо ((stdout_fd = xopen_nonblock(redirect_stdout,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
		}інакше якщо (stdout_process) {
			stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
			якщо (stdout_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stdout `%s': %s",
				    applet, stdout_process, strerror(errno));
		}
		якщо (redirect_stderr) {
			якщо ((stderr_fd = xopen_nonblock(redirect_stderr,
				    O_WRONLY | O_CREAT | O_APPEND,
				    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
				eerrorx("%s: unable to redirect the logfile"
				    " for stderr `%s': %s",
				    applet, redirect_stderr, strerror(errno));
		}інакше якщо (stderr_process) {
			stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
			якщо (stderr_fd == -1)
				eerrorx("%s: unable to open the logging process"
				    " for stderr `%s': %s",
				    applet, stderr_process, strerror(errno));
		}

		якщо (background)
			dup2(stdin_fd, STDIN_FILENO);
		якщо (background || redirect_stdout || stdout_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stdout_fd, STDOUT_FILENO);
		якщо (background || redirect_stderr || stderr_process
				|| rc_yesno(getenv("EINFO_QUIET")))
			dup2(stderr_fd, STDERR_FILENO);

		cloexec_fds_from(3); /* FIXME: this is problematic, see right below */

		якщо (notify.type == NOTIFY_FD) {
			якщо (close(notify.pipe[0]) == -1)
				eerrorx("%s: failed to close notify pipe[0]: %s", applet, strerror(errno));
			якщо (dup2(notify.pipe[1], notify.fd) == -1)
				eerrorx("%s: failed to initialize notify fd: %s", applet, strerror(errno));

			/* if notify.pipe[1] == notify.fd then the FD_CLOEXEC flag is not cleared by dup2,
			   leading to failure. The workaround here is to clear it manually, but the
			   real fix is that we should never close/cloexec fds in bulk like this */
			якщо (notify.pipe[1] == notify.fd) {
				ціле flags = fcntl(notify.fd, F_GETFD, 0);
				якщо (flags == -1)
					eerrorx("%s: failed to get flags for notify fd: %s", applet, strerror(errno));
				якщо (fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
					eerrorx("%s: failed to set flags for notify fd: %s", applet, strerror(errno));
			}

		}

		якщо (scheduler != NULL) {
			ціле scheduler_index;
			struct sched_param sched =  {.sched_priority = sched_prio};
			якщо (strcmp(scheduler, "fifo") == 0)
				scheduler_index = SCHED_FIFO;
			інакше якщо (strcmp(scheduler, "rr") == 0)
				scheduler_index = SCHED_RR;
			інакше якщо (strcmp(scheduler, "other") == 0)
				scheduler_index = SCHED_OTHER;
#ifdef SCHED_BATCH
			інакше якщо (strcmp(scheduler, "batch") == 0)
				scheduler_index = SCHED_BATCH;
#endif
#ifdef SCHED_IDLE
			інакше якщо (strcmp(scheduler, "idle") == 0)
				scheduler_index = SCHED_IDLE;
#endif
			інакше якщо (sscanf(scheduler, "%d", &scheduler_index) != 1)
				eerrorx("Unknown scheduler: %s", scheduler);

			якщо (sched_prio == -1)
				sched.sched_priority = sched_get_priority_min(scheduler_index);

			якщо (pthread_setschedparam(pthread_self(), scheduler_index, &sched))
				eerrorx("Failed to set scheduler: %s", strerror(errno));
		} інакше якщо (sched_prio != -1) {
			const struct sched_param sched =  {.sched_priority = sched_prio};
			якщо (sched_setparam(mypid, &sched))
				eerrorx("Failed to set scheduler parameters: %s", strerror(errno));
		}

		execvp(exec, argv);
#ifdef HAVE_PAM
		якщо (changeuser != NULL && pamr == PAM_SUCCESS)
			pam_close_session(pamh, PAM_SILENT);
#endif
		eerrorx("%s: failed to exec `%s': %s",
		    applet, exec,strerror(errno));
	}

	/* Parent process */

	close(pipefd[1]); /* Close the write end of the pipe. */

	/* The child never writes to the pipe, so this read will block until
	 * the child calls exec or exits. */
	поки ((ss = read(pipefd[0], readbuf, 1)) == -1 && errno == EINTR);
	якщо (ss == -1)
		eerrorx("%s: failed to read from pipe: %s",
			applet, strerror(errno));

	close(pipefd[0]);

	якщо (!background) {
		/* As we're not backgrounding the process, wait for our pid
		 * to return */
		i = 0;
		spid = pid;

		do {
			pid = waitpid(spid, &i, 0);
			якщо (pid < 1) {
				eerror("waitpid %d: %s",
				    spid, strerror(errno));
				повернути -1;
			}
		} поки (!WIFEXITED(i) && !WIFSIGNALED(i));
		якщо (!WIFEXITED(i) || WEXITSTATUS(i) != 0) {
			eerror("%s: failed to start `%s'", applet, exec);
			exit(EXIT_FAILURE);
		}
		pid = spid;
	}

	/* Wait a little bit and check that process is still running
	   We do this as some badly written daemons fork and then barf */
	якщо (start_wait == 0 &&
	    ((p = getenv("SSD_STARTWAIT")) ||
		(p = rc_conf_value("rc_start_wait"))))
	{
		якщо (sscanf(p, "%u", &start_wait) != 1)
			start_wait = 0;
	}

	якщо (notify.type != NOTIFY_NONE) {
		якщо (!notify_wait(applet, notify))
			ret = EXIT_FAILURE;
	} інакше якщо (start_wait > 0) {
		struct timespec ts;
		bool alive = false;

		ts.tv_sec = start_wait / 1000;
		ts.tv_nsec = (start_wait % 1000) * ONE_MS;
		якщо (nanosleep(&ts, NULL) == -1) {
			якщо (errno != EINTR) {
				eerror("%s: nanosleep: %s",
				    applet, strerror(errno));
				повернути 0;
			}
		}
		якщо (background) {
			якщо (kill(pid, 0) == 0)
				alive = true;
		} інакше {
			якщо (pidfile) {
				pid = get_pid(applet, pidfile);
				якщо (pid == -1) {
					eerrorx("%s: did not "
					    "create a valid"
					    " pid in `%s'",
					    applet, pidfile);
				}
			} інакше
				pid = 0;
			якщо (do_stop(applet, exec, (const char *const *)margv,
				pid, uid, 0, false, test, false) > 0)
				alive = true;
		}

		якщо (!alive)
			eerrorx("%s: %s died", applet, exec);
	}

	якщо (svcname)
		rc_service_daemon_set(svcname, exec,
		    (const char *const *)margv, pidfile, true);

	exit(ret);
	/* NOTREACHED */
}
