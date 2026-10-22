/*
 * supervise-daemon
 * This is a supervisor for daemons.
 * It will start a daemon and make sure it restarts if it crashes.
 */

/*
 * Copyright (c) 2016-2025 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ifdef HAVE_PAM
#ಅನ್ತರ್ಭಾವ <security/pam_appl.h>

/* We are not supporting authentication conversations */
ಸ್ಥಿರ ಸಂರಚನಾ pam_conv conv = { ರಿಕ್ತ, ರಿಕ್ತ};
#endif

#ifdef __linux__
#ಅನ್ತರ್ಭಾವ <sys/capability.h>
#endif

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "plugin.h"
#ಅನ್ತರ್ಭಾವ "schedules.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ಗಣನಾ {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_BASE = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_STEP,
  LONGOPT_RESPAWN_DELAY_CAP,
};

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "healthcheck-timer",        1, ರಿಕ್ತ, 'a'},
	{ "healthcheck-delay",        1, ರಿಕ್ತ, 'A'},
	{ "capabilities", 1, ರಿಕ್ತ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ರಿಕ್ತ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ರಿಕ್ತ, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ರಿಕ್ತ, 'D'},
	{ "respawn-delay-step",   1, ರಿಕ್ತ, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, ರಿಕ್ತ, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ರಿಕ್ತ, 'd'},
	{ "env",          1, ರಿಕ್ತ, 'e'},
	{ "group",        1, ರಿಕ್ತ, 'g'},
	{ "stop-group",   0, ರಿಕ್ತ, 'G'},
	{ "ionice",       1, ರಿಕ್ತ, 'I'},
	{ "stop",         0, ರಿಕ್ತ, 'K'},
	{ "umask",        1, ರಿಕ್ತ, 'k'},
	{ "respawn-max",    1, ರಿಕ್ತ, 'm'},
	{ "nicelevel",    1, ರಿಕ್ತ, 'N'},
	{ "oom-score-adj",1, ರಿಕ್ತ, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ರಿಕ್ತ, 'p'},
	{ "respawn-period",        1, ರಿಕ್ತ, 'P'},
	{ "retry",       1, ರಿಕ್ತ, 'R'},
	{ "chroot",       1, ರಿಕ್ತ, 'r'},
	{ "signal",       1, ರಿಕ್ತ, 's'},
	{ "start",        0, ರಿಕ್ತ, 'S'},
	{ "user",         1, ರಿಕ್ತ, 'u'},
	{ "stdin",        1, ರಿಕ್ತ, '0'},
	{ "stdout",       1, ರಿಕ್ತ, '1'},
	{ "stderr",       1, ರಿಕ್ತ, '2'},
	{ "stdout-logger",1, ರಿಕ್ತ, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ರಿಕ್ತ, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ರಿಕ್ತ, '3'},
	{ "notify",       1, ರಿಕ್ತ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
	"set an initial health check delay",
	"set a health check timer",
	"Set the inheritable, ambient and bounding capabilities",
	"Set the security-bits for the program",
	"Set the No New Privs flag for the program",
	"Set a respawn delay",
	"Increase the respawn delay by this amount for every retry",
	"Set maximum respawn delay when respawn-delay-step is also active",
	"Change the PWD",
	"Set an environment string",
	"Change the process group",
	"Stop the whole process group",
	"Set an ionice class:data when starting",
	"Stop daemon",
	"Set the umask for the daemon",
	"set maximum number of respawn attempts",
	"Set a nicelevel when starting",
	"Set OOM score adjustment when starting",
	"Match pid found in this file",
	"Set respawn time period",
	"Retry schedule to use when stopping",
	"Chroot to this directory",
	"Send a signal to the daemon",
	"Start daemon",
	"Change the process user",
	"Redirect stdin to file",
	"Redirect stdout to file",
	"Redirect stderr to file",
	"Redirect stdout to process",
	"Redirect stderr to process",
	"reexec (used internally)",
	"Configures experimental notification behaviour",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ healthcheckdelay = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ healthchecktimer = 0;
ಸ್ಥಿರ ಚಞ್ಚಲ sig_atomic_t do_healthcheck = 0;
ಸ್ಥಿರ ಚಞ್ಚಲ sig_atomic_t exiting = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nicelevel = INT_MIN;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ionicec = -1;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ioniced = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ oom_score_adj = INT_MIN;
ಸ್ಥಿರ ವರ್ಣ *changeuser, *ch_ಬೇರು, *ch_dir;
ಸ್ಥಿರ uid_t uid = 0;
ಸ್ಥಿರ gid_t gid = 0;
ಸ್ಥಿರ bool stopgroup = false;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devnull_fd = -1;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ stdin_fd;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ stdout_fd;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ stderr_fd;
ಸ್ಥಿರ ಸಂರಚನಾ notify notify;
ಸ್ಥಿರ ವರ್ಣ *redirect_stdin = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *redirect_stderr = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *redirect_stdout = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *stderr_process = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *stdout_process = ರಿಕ್ತ;
#ifdef TIOCNOTTY
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tty_fd = -1;
#endif
ಸ್ಥಿರ pid_t child_pid;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ respawn_ಎಣಿಕೆ = 0;
ಸ್ಥಿರ int64_t respawn_delay; /* default set inside main() */
ಸ್ಥಿರ int64_t respawn_delay_step = TM_MS(128);
ಸ್ಥಿರ int64_t respawn_delay_cap = TM_SEC(30);
ಸ್ಥಿರ int64_t respawn_period; /* default set inside main() */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ respawn_max = 10;
ಸ್ಥಿರ ವರ್ಣ *fifopath = ರಿಕ್ತ;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fifo_fd = 0;
ಸ್ಥಿರ ವರ್ಣ *pidfile = ರಿಕ್ತ;
ಸ್ಥಿರ ವರ್ಣ *svcname = ರಿಕ್ತ;
ಸ್ಥಿರ bool verbose = false;
#ifdef __linux__
ಸ್ಥಿರ cap_iab_t cap_iab = ರಿಕ್ತ;
ಸ್ಥಿರ unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
ಸ್ಥಿರ bool no_new_privs = false;
#endif

ಬಾಹ್ಯ ವರ್ಣ **environ;

#ಯದಿ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ಯದಿ !defined(__DragonFly__)
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ ioprio_set(ಪೂರ್ಣಾಙ್ಕ which RC_UNUSED, ಪೂರ್ಣಾಙ್ಕ who RC_UNUSED,
			     ಪೂರ್ಣಾಙ್ಕ ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ಪ್ರತ್ಯಾಗಮ syscall(SYS_ioprio_set, which, who, ioprio);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ cleanup(ಶೂನ್ಯ)
{
	ಮುಕ್ತ(changeuser);
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ re_exec_supervisor(ಶೂನ್ಯ)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(ವರ್ಣ *) ರಿಕ್ತ);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_signal(ಪೂರ್ಣಾಙ್ಕ sig)
{
	ಪೂರ್ಣಾಙ್ಕ serrno = errno;
	pid_t pid;

	ವಿಕಲ್ಪ (sig) {
	ಪಕ್ಷ SIGALRM:
		do_healthcheck = 1;
		ಭಙ್ಗ;
	ಪಕ್ಷ SIGCHLD:
		ಯದಿ (exiting)
			ಯಾವತ್ (waitpid((pid_t)(-1), ರಿಕ್ತ, WNOHANG) > 0) {}
		ಅನ್ಯಥಾ {
			ಯಾವತ್ ((pid = waitpid((pid_t)(-1), ರಿಕ್ತ, WNOHANG|WNOWAIT)) > 0) {
				ಯದಿ (pid == child_pid)
					ಭಙ್ಗ;
				pid = waitpid(pid, ರಿಕ್ತ, WNOHANG);
			}
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ SIGTERM:
		exiting = 1;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ವರ್ಣ * expand_home(ಧ್ರುವ ವರ್ಣ *home, ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ *opath, *ppath, *p, *nh;
	ಸಂರಚನಾ passwd *pw;

	ಯದಿ (!path || *path != '~')
		ಪ್ರತ್ಯಾಗಮ xstrdup(path);

	opath = ppath = xstrdup(path);
	ಯದಿ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ಯದಿ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ಯದಿ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ಯದಿ (ppath)
				*ppath = '/';
		} ಅನ್ಯಥಾ
			home = ರಿಕ್ತ;
	} ಅನ್ಯಥಾ
		ppath++;

	ಯದಿ (!home) {
		ಮುಕ್ತ(opath);
		ಪ್ರತ್ಯಾಗಮ xstrdup(path);
	}
	ಯದಿ (!ppath) {
		ಮುಕ್ತ(opath);
		ಪ್ರತ್ಯಾಗಮ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ಮುಕ್ತ(opath);
	ಪ್ರತ್ಯಾಗಮ nh;
}

ಸ್ಥಿರ ವರ್ಣ *make_cmdline(ವರ್ಣ **argv)
{
	ವರ್ಣ **c;
	ವರ್ಣ *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	ಪ್ರತಿ (c = argv; c && *c; c++) {
		fputs(*c, mem);
		ಯದಿ (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	ಪ್ರತ್ಯಾಗಮ cmdline;
}

ಸ್ಥಿರ pid_t exec_command(ಧ್ರುವ ವರ್ಣ *cmd)
{
	ವರ್ಣ *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	ಸಂರಚನಾ sigaction sa;

	file = rc_service_resolve(svcname);
	ಯದಿ (access(file, F_OK) != 0) {
		ಮುಕ್ತ(file);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, ಪರಿಮಾಣ (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	ಯದಿ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ರಿಕ್ತ);
		sigaction(SIGHUP, &sa, ರಿಕ್ತ);
		sigaction(SIGINT, &sa, ರಿಕ್ತ);
		sigaction(SIGQUIT, &sa, ರಿಕ್ತ);
		sigaction(SIGTERM, &sa, ರಿಕ್ತ);
		sigaction(SIGUSR1, &sa, ರಿಕ್ತ);
		sigaction(SIGWINCH, &sa, ರಿಕ್ತ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);

		/* Safe to run now */
		execl(file, file, cmd, (ವರ್ಣ *) ರಿಕ್ತ);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ಯದಿ (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
	ಮುಕ್ತ(file);
	ಪ್ರತ್ಯಾಗಮ pid;
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ child_process(ವರ್ಣ *exec, ವರ್ಣ **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ಪೂರ್ಣಾಙ್ಕ i;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
	size_t len;
	ವರ್ಣ *newpath;
	ವರ್ಣ *np;
	ವರ್ಣ *cmdline = ರಿಕ್ತ;
	time_t start_time;
	ವರ್ಣ start_ಎಣಿಕೆ_string[20];
	ವರ್ಣ start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	ಪೂರ್ಣಾಙ್ಕ group_ಎಣಿಕೆ = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ pamr;
	ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *pamenv = ರಿಕ್ತ;
#endif

	setsid();

	ಯದಿ (svcname) {
		start_time = time(ರಿಕ್ತ);
		from_time_t(start_time_string, start_time);
		rc_service_ಮೌಲ್ಯ_set(svcname, "start_time", start_time_string);
		sprintf(start_ಎಣಿಕೆ_string, "%i", respawn_ಎಣಿಕೆ);
		rc_service_ಮೌಲ್ಯ_set(svcname, "start_count", start_ಎಣಿಕೆ_string);
		sprintf(start_ಎಣಿಕೆ_string, "%d", getpid());
		rc_service_ಮೌಲ್ಯ_set(svcname, "child_pid", start_ಎಣಿಕೆ_string);
	}

	ಯದಿ (nicelevel != INT_MIN) {
		ಯದಿ (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		ಯದಿ (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} ಅನ್ಯಥಾ ಯದಿ (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	ಯದಿ (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	ಯದಿ (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		ಯದಿ (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	ಯದಿ (changeuser != ರಿಕ್ತ) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		ಯದಿ (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		ಯದಿ (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		ಯದಿ (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	ಯದಿ (changeuser && getgrouplist(changeuser, gid, group_list, &group_ಎಣಿಕೆ) < 0) {
		group_list = xmalloc(group_ಎಣಿಕೆ * ಪರಿಮಾಣ(*group_list));
		ಯದಿ (getgrouplist(changeuser, gid, group_list, &group_ಎಣಿಕೆ) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	ಯದಿ (ch_ಬೇರು && chroot(ch_ಬೇರು) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_ಬೇರು, strerror(errno));

	ಯದಿ (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	ಯದಿ (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	ಯದಿ (changeuser && setgroups(group_ಎಣಿಕೆ, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	ಯದಿ (group_list != group_buf)
		ಮುಕ್ತ(group_list);
#ifdef __linux__
	ಯದಿ (uid && cap_setuid(uid))
#ಅನ್ಯಥಾ
	ಯದಿ (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	ಯದಿ (cap_iab != ರಿಕ್ತ) {
		i = cap_iab_set_proc(cap_iab);

		ಯದಿ (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		ಯದಿ (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	ಯದಿ (secbits != 0) {
		ಯದಿ (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	ಯದಿ (no_new_privs) {
		ಯದಿ (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", strerror(errno));
	}
#endif

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	ಯಾವತ್ (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	ಯದಿ (changeuser != ರಿಕ್ತ) {
		pamenv = (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)pam_getenvlist(pamh);
		ಯದಿ (pamenv) {
			ಯಾವತ್ (*pamenv) {
				/* Don't add strings unless they set a var */
				ಯದಿ (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				ಅನ್ಯಥಾ
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		ಯದಿ (strncmp(env->ಮೌಲ್ಯ, "RC_", 3) == 0
				|| strncmp(env->ಮೌಲ್ಯ, "SSD_", 4) == 0
				|| strncmp(env->ಮೌಲ್ಯ, "EINFO_", 6) == 0) {
			*strchr(env->ಮೌಲ್ಯ, '=') = '\0';
			unsetenv(env->ಮೌಲ್ಯ);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	ಯದಿ ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		ಯಾವತ್ (token && *token) {
			p = strchr(token, ':');
			ಯದಿ (p) {
				*p++ = '\0';
				ಯಾವತ್ (*p == ':')
					p++;
			}
			ಯದಿ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				ಯದಿ (np != newpath)
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
	ಯದಿ (redirect_stdin) {
		ಯದಿ ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	ಯದಿ (redirect_stdout) {
		ಯದಿ ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} ಅನ್ಯಥಾ ಯದಿ (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		ಯದಿ (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	ಯದಿ (redirect_stderr) {
		ಯದಿ ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} ಅನ್ಯಥಾ ಯದಿ (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		ಯದಿ (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	ಯದಿ (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	ಯದಿ (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	ಯದಿ (notify.type == NOTIFY_FD) {
		ಪೂರ್ಣಾಙ್ಕ error;

		ಯದಿ (notify.fd == notify.pipe[1]) {
			ಪೂರ್ಣಾಙ್ಕ flags = error = fcntl(notify.fd, F_GETFD, 0);
			ಯದಿ (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} ಅನ್ಯಥಾ {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		ಯದಿ (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	ಮುಕ್ತ(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	ಯದಿ (changeuser != ರಿಕ್ತ && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ supervisor(ವರ್ಣ *exec, ವರ್ಣ **argv)
{
	FILE *fp;
	ವರ್ಣ buf[2048];
	ವರ್ಣ cmd[2048];
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
	ಪೂರ್ಣಾಙ್ಕ failing;
	ಪೂರ್ಣಾಙ್ಕ health_status;
	ಪೂರ್ಣಾಙ್ಕ healthcheck_respawn;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ nkilled;
	ಪೂರ್ಣಾಙ್ಕ sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	ಸಂರಚನಾ sigaction sa;
	int64_t respawn_now = 0;
	int64_t first_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&signals);
	sigdelset(&signals, SIGALRM);
	sigdelset(&signals, SIGCHLD);
	sigdelset(&signals, SIGTERM);
	sigprocmask(SIG_SETMASK, &signals, &old_signals);

	/* install signal  handler */
	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ರಿಕ್ತ);
	sigaction(SIGCHLD, &sa, ರಿಕ್ತ);
	sigaction(SIGTERM, &sa, ರಿಕ್ತ);

	fp = fopen(pidfile, "w");
	ಯದಿ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	ಯದಿ (svcname)
		rc_service_daemon_set(svcname, exec, (ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	ಯದಿ (healthcheckdelay)
		alarm(healthcheckdelay);
	ಅನ್ಯಥಾ ಯದಿ (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	ಯಾವತ್ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		ಯದಿ (fifo_fd > 0) {
			memset(buf, 0, ಪರಿಮಾಣ(buf));
			ಎಣಿಕೆ = ಪಠ(fifo_fd, buf, ಪರಿಮಾಣ(buf) - 1);
			close(fifo_fd);
			ಯದಿ (ಎಣಿಕೆ != -1)
				buf[ಎಣಿಕೆ] = 0;
			ಯದಿ (ಎಣಿಕೆ == 0)
				ಅನುವರ್ತ;
			ಯದಿ (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			ಯದಿ (strncasecmp(buf, "sig", 3) == 0) {
				ಯದಿ ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					ಯದಿ (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			ಅನುವರ್ತ;
		}
		ಯದಿ (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			ಯದಿ (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			ಯದಿ (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			ಅನ್ಯಥಾ {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, ರಿಕ್ತ, ರಿಕ್ತ, child_pid, 0,
						stopgroup, false, false, true);
				ಯದಿ (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				ಅನ್ಯಥಾ
					healthcheck_respawn = 1;
			}
		}
		ಯದಿ (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, ರಿಕ್ತ, ರಿಕ್ತ, child_pid, 0,
					stopgroup, false, false, true);
			ಯದಿ (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			ಅನುವರ್ತ;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		ಯದಿ (wait_pid == child_pid) {
			ಯದಿ (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			ಅನ್ಯಥಾ ಯದಿ (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		ಯದಿ (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			ಯದಿ (first_spawn == 0)
				first_spawn = respawn_now;
			ಯದಿ ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_ಎಣಿಕೆ = 0;
				first_spawn = 0;
			} ಅನ್ಯಥಾ
				respawn_ಎಣಿಕೆ++;
			ಯದಿ (respawn_max > 0 && respawn_ಎಣಿಕೆ > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				ಅನುವರ್ತ;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_ಎಣಿಕೆ);
			ಯದಿ (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			ಯದಿ (exiting)
				ಅನುವರ್ತ;
			child_pid = fork();
			ಯದಿ (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			ಯದಿ (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, ರಿಕ್ತ);
				memset(&sa, 0, ಪರಿಮಾಣ(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ರಿಕ್ತ);
				sigaction(SIGCHLD, &sa, ರಿಕ್ತ);
				sigaction(SIGTERM, &sa, ರಿಕ್ತ);
				child_process(exec, argv);
			}
			ಯದಿ (healthcheckdelay)
				alarm(healthcheckdelay);
			ಅನ್ಯಥಾ ಯದಿ (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	ಯದಿ (svcname) {
		rc_service_daemon_set(svcname, exec, (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)argv,
				pidfile, false);
		rc_service_ಮೌಲ್ಯ_set(svcname, "child_pid", ರಿಕ್ತ);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		ಯದಿ (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	ಯದಿ (pidfile)
		unlink(pidfile);
	ಯದಿ (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ opt;
	ವರ್ಣ **c;
	ಪೂರ್ಣಾಙ್ಕ x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	ವರ್ಣ *exec = ರಿಕ್ತ;
	ವರ್ಣ *retry = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ sig = SIGTERM;
	ವರ್ಣ *home = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ tid = 0;
	pid_t pid;
	ವರ್ಣ *tmp;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ n;
	ವರ್ಣ *exec_file = ರಿಕ್ತ;
	ವರ್ಣ *varbuf = ರಿಕ್ತ;
	ವರ್ಣ numbuf[64];
	ಸಂರಚನಾ timespec ts;
	ಸಂರಚನಾ passwd *pw;
	ಸಂರಚನಾ group *gr;
	FILE *fp;
	mode_t numask = 022;
	ಪೂರ್ಣಾಙ್ಕ child_argc = 0;
	ವರ್ಣ **child_argv = ರಿಕ್ತ;
	ವರ್ಣ *str = ರಿಕ್ತ;
	ವರ್ಣ *cmdline = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *respawn_delay_str  = "0";
	ಧ್ರುವ ವರ್ಣ *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	ಯದಿ (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	ಯದಿ (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	ಯದಿ ((tmp = getenv("SSD_NICELEVEL")))
		ಯದಿ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ಯದಿ ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ಯದಿ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ಯದಿ (ionicec == 0)
			ioniced = 0;
		ಅನ್ಯಥಾ ಯದಿ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ಯದಿ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ಯದಿ (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ಯದಿ (home == ರಿಕ್ತ || p == ರಿಕ್ತ) {
		pw = getpwuid(getuid());
		ಯದಿ (pw != ರಿಕ್ತ) {
			ಯದಿ (p == ರಿಕ್ತ)
				setenv("USER", pw->pw_name, 1);
			ಯದಿ (home == ರಿಕ್ತ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	ಯದಿ (svcname) {
		argc--;
		argv++;
	}
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'a':  /* --healthcheck-timer <time> */
			ಯದಿ (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'A':  /* --healthcheck-delay <time> */
			ಯದಿ (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ಯದಿ (cap_iab == ರಿಕ್ತ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ಅನ್ಯಥಾ
			eerrorx("Capabilities support not enabled");
#endif
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_SECBITS:
#ifdef __linux__
			ಯದಿ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ರಿಕ್ತ;
			secbits = strtoul(optarg, &tmp, 0);
			ಯದಿ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ಅನ್ಯಥಾ
			eerrorx("Capabilities support not enabled");
#endif
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ಅನ್ಯಥಾ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			ಭಙ್ಗ;

		ಪಕ್ಷ 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'I': /* --ionice */
			ಯದಿ (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ಯದಿ (ionicec == 0)
				ioniced = 0;
			ಅನ್ಯಥಾ ಯದಿ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ಭಙ್ಗ;

		ಪಕ್ಷ 'K':  /* --stop */
			stop = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'N':  /* --nice */
			ಯದಿ (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ಯದಿ (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'S':  /* --start */
			start = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'e': /* --env */
			putenv(optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'g':  /* --group <group>|<gid> */
			ಯದಿ (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ಅನ್ಯಥಾ
				gr = getgrgid((gid_t)tid);
			ಯದಿ (gr == ರಿಕ್ತ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'G': /* -G */
			stopgroup = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'k':
			ಯದಿ (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			ಯದಿ (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'r':  /* --chroot /new/root */
			ch_ಬೇರು = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ 'u':  /* --user <username>|<uid> */
		{
			ವರ್ಣ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ಯದಿ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ಅನ್ಯಥಾ
				pw = getpwuid((uid_t)tid);

			ಯದಿ (pw == ರಿಕ್ತ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ಯದಿ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ಯದಿ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ಯದಿ (gid == 0)
				gid = pw->pw_gid;

			ಯದಿ (p) {
				tmp = strsep (&p, ":");
				ಯದಿ (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ಅನ್ಯಥಾ
					gr = getgrgid((gid_t) tid);

				ಯದಿ (gr == ರಿಕ್ತ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ಭಙ್ಗ;

		ಪಕ್ಷ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ '3':  /* --reexec */
			reexec = true;
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			ಯದಿ (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			ಭಙ್ಗ;

		ಪಕ್ಷ LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			ಯದಿ (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			ಭಙ್ಗ;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	ಯದಿ (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	ಯದಿ (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	ಯದಿ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ಯದಿ (ch_ಬೇರು && *ch_ಬೇರು == '~')
		ch_ಬೇರು = expand_home(home, ch_ಬೇರು);

	umask(numask);
	ಯದಿ (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	ಯದಿ (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	ಯದಿ (reexec) {
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * ಪರಿಮಾಣ(ವರ್ಣ *));
		memset(child_argv, 0, (child_argc + 1) * ಪರಿಮಾಣ(ವರ್ಣ *));
		ಪ್ರತಿ (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_ಮೌಲ್ಯ_get(svcname, varbuf);
			child_argv[x] = str;
			ಮುಕ್ತ(varbuf);
			varbuf = ರಿಕ್ತ;
		}
		ಮುಕ್ತ(str);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		ಮುಕ್ತ(str);
		exec = rc_service_ಮೌಲ್ಯ_get(svcname, "exec");
		pidfile = rc_service_ಮೌಲ್ಯ_get(svcname, "pidfile");
		retry = rc_service_ಮೌಲ್ಯ_get(svcname, "retry");
		ಯದಿ (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_ಮೌಲ್ಯ_set(svcname, "retry", retry);
		} ಅನ್ಯಥಾ
			parse_schedule(applet, ರಿಕ್ತ, sig);

		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} ಅನ್ಯಥಾ ಯದಿ (start) {
		ಯದಿ (exec) {
			ಯದಿ (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			ಯದಿ (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				ಯದಿ (ch_ಬೇರು)
					xasprintf(&exec_file, "%s/%s", ch_ಬೇರು, exec);
				ಅನ್ಯಥಾ
					xasprintf(&exec_file, "%s", exec);
			} ಅನ್ಯಥಾ {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = ರಿಕ್ತ;
				ಯಾವತ್ ((token = strsep(&p, ":"))) {
					ಯದಿ (ch_ಬೇರು)
						xasprintf(&exec_file, "%s/%s/%s", ch_ಬೇರು, token, exec);
					ಅನ್ಯಥಾ
						xasprintf(&exec_file, "%s/%s", token, exec);
					ಯದಿ (exec_file && access(exec_file, F_OK) == 0)
						ಭಙ್ಗ;
					ಮುಕ್ತ(exec_file);
					exec_file = ರಿಕ್ತ;
				}
				ಮುಕ್ತ(tmp);
			}
			ಯದಿ (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				ಮುಕ್ತ(exec_file);
				exit(EXIT_FAILURE);
			}
		} ಅನ್ಯಥಾ
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		ಯದಿ (pid != -1)
			ಯದಿ (do_stop(applet, exec, (ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		ಯದಿ (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		ಯದಿ (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		ಯದಿ (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_ಮೌಲ್ಯ_set(svcname, "retry", retry);
		} ಅನ್ಯಥಾ
			parse_schedule(applet, ರಿಕ್ತ, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		ಮುಕ್ತ(cmdline);
		cmdline = ರಿಕ್ತ;

		/* Remove existing pidfile */
		ಯದಿ (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		ಯದಿ (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_ಮೌಲ್ಯ_set(svcname, "pidfile", pidfile);
		rc_service_ಮೌಲ್ಯ_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_ಮೌಲ್ಯ_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, ಪರಿಮಾಣ(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_ಮೌಲ್ಯ_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, ಪರಿಮಾಣ(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_ಮೌಲ್ಯ_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, ಪರಿಮಾಣ(numbuf), "%i", respawn_max);
		rc_service_ಮೌಲ್ಯ_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		ಯದಿ (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ಯದಿ (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		ಯದಿ (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		ಯದಿ (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ಅನ್ಯಥಾ ಯದಿ (child_pid != 0) {
			ಯದಿ (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			ಯಾವತ್ (c && *c) {
				varbuf = ರಿಕ್ತ;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_ಮೌಲ್ಯ_set(svcname, varbuf, *c);
				ಮುಕ್ತ(varbuf);
				varbuf = ರಿಕ್ತ;
				x++;
				c++;
			}
			snprintf(numbuf, ಪರಿಮಾಣ(numbuf), "%d", x);
			rc_service_ಮೌಲ್ಯ_set(svcname, "argc", numbuf);
			rc_service_ಮೌಲ್ಯ_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} ಅನ್ಯಥಾ
			child_process(exec, argv);
	} ಅನ್ಯಥಾ ಯದಿ (stop) {
		pid = get_pid(applet, pidfile);
		ಯದಿ (pid != -1) {
			i = kill(pid, SIGTERM);
			ಯದಿ (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			ಅನ್ಯಥಾ {
				/* wait for the supervisor to go down */
				ಯಾವತ್ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ರಿಕ್ತ);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ಯದಿ (pidfile)
			unlink(pidfile);
		ಯದಿ (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} ಅನ್ಯಥಾ ಯದಿ (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		ಯದಿ (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = ಲಿಖ(fifo_fd, str, strlen(str));
		ಯದಿ (x == -1) {
			ಮುಕ್ತ(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		ಮುಕ್ತ(tmp);
		exit(EXIT_SUCCESS);
	}
}
