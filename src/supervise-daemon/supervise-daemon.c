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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <grp.h>
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/resource.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>

#ifdef HAVE_PAM
#അന്തര്ഭാവ <security/pam_appl.h>

/* We are not supporting authentication conversations */
സ്ഥിതം ഘടന pam_conv conv = { ശൂന്യ, ശൂന്യ};
#endif

#ifdef __linux__
#അന്തര്ഭാവ <sys/capability.h>
#endif

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "plugin.h"
#അന്തര്ഭാവ "schedules.h"
#അന്തര്ഭാവ "timeutils.h"
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
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_STEP,
  LONGOPT_RESPAWN_DELAY_CAP,
};

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "healthcheck-timer",        1, ശൂന്യ, 'a'},
	{ "healthcheck-delay",        1, ശൂന്യ, 'A'},
	{ "capabilities", 1, ശൂന്യ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ശൂന്യ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ശൂന്യ, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ശൂന്യ, 'D'},
	{ "respawn-delay-step",   1, ശൂന്യ, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, ശൂന്യ, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ശൂന്യ, 'd'},
	{ "env",          1, ശൂന്യ, 'e'},
	{ "group",        1, ശൂന്യ, 'g'},
	{ "stop-group",   0, ശൂന്യ, 'G'},
	{ "ionice",       1, ശൂന്യ, 'I'},
	{ "stop",         0, ശൂന്യ, 'K'},
	{ "umask",        1, ശൂന്യ, 'k'},
	{ "respawn-max",    1, ശൂന്യ, 'm'},
	{ "nicelevel",    1, ശൂന്യ, 'N'},
	{ "oom-score-adj",1, ശൂന്യ, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ശൂന്യ, 'p'},
	{ "respawn-period",        1, ശൂന്യ, 'P'},
	{ "retry",       1, ശൂന്യ, 'R'},
	{ "chroot",       1, ശൂന്യ, 'r'},
	{ "signal",       1, ശൂന്യ, 's'},
	{ "start",        0, ശൂന്യ, 'S'},
	{ "user",         1, ശൂന്യ, 'u'},
	{ "stdin",        1, ശൂന്യ, '0'},
	{ "stdout",       1, ശൂന്യ, '1'},
	{ "stderr",       1, ശൂന്യ, '2'},
	{ "stdout-logger",1, ശൂന്യ, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ശൂന്യ, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ശൂന്യ, '3'},
	{ "notify",       1, ശൂന്യ, LONGOPT_NOTIFY},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

സ്ഥിതം പൂർണ്ണസംഖ്യ healthcheckdelay = 0;
സ്ഥിതം പൂർണ്ണസംഖ്യ healthchecktimer = 0;
സ്ഥിതം അസ്ഥിര sig_atomic_t do_healthcheck = 0;
സ്ഥിതം അസ്ഥിര sig_atomic_t exiting = 0;
സ്ഥിതം പൂർണ്ണസംഖ്യ nicelevel = INT_MIN;
സ്ഥിതം പൂർണ്ണസംഖ്യ ionicec = -1;
സ്ഥിതം പൂർണ്ണസംഖ്യ ioniced = 0;
സ്ഥിതം പൂർണ്ണസംഖ്യ oom_score_adj = INT_MIN;
സ്ഥിതം അക്ഷരം *changeuser, *ch_വേര്, *ch_dir;
സ്ഥിതം uid_t uid = 0;
സ്ഥിതം gid_t gid = 0;
സ്ഥിതം bool stopgroup = false;
സ്ഥിതം പൂർണ്ണസംഖ്യ devnull_fd = -1;
സ്ഥിതം പൂർണ്ണസംഖ്യ stdin_fd;
സ്ഥിതം പൂർണ്ണസംഖ്യ stdout_fd;
സ്ഥിതം പൂർണ്ണസംഖ്യ stderr_fd;
സ്ഥിതം ഘടന notify notify;
സ്ഥിതം അക്ഷരം *redirect_stdin = ശൂന്യ;
സ്ഥിതം അക്ഷരം *redirect_stderr = ശൂന്യ;
സ്ഥിതം അക്ഷരം *redirect_stdout = ശൂന്യ;
സ്ഥിതം അക്ഷരം *stderr_process = ശൂന്യ;
സ്ഥിതം അക്ഷരം *stdout_process = ശൂന്യ;
#ifdef TIOCNOTTY
സ്ഥിതം പൂർണ്ണസംഖ്യ tty_fd = -1;
#endif
സ്ഥിതം pid_t child_pid;
സ്ഥിതം പൂർണ്ണസംഖ്യ respawn_എണ്ണം = 0;
സ്ഥിതം int64_t respawn_delay; /* default set inside main() */
സ്ഥിതം int64_t respawn_delay_step = TM_MS(128);
സ്ഥിതം int64_t respawn_delay_cap = TM_SEC(30);
സ്ഥിതം int64_t respawn_period; /* default set inside main() */
സ്ഥിതം പൂർണ്ണസംഖ്യ respawn_max = 10;
സ്ഥിതം അക്ഷരം *fifopath = ശൂന്യ;
സ്ഥിതം പൂർണ്ണസംഖ്യ fifo_fd = 0;
സ്ഥിതം അക്ഷരം *pidfile = ശൂന്യ;
സ്ഥിതം അക്ഷരം *svcname = ശൂന്യ;
സ്ഥിതം bool verbose = false;
#ifdef __linux__
സ്ഥിതം cap_iab_t cap_iab = ശൂന്യ;
സ്ഥിതം unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
സ്ഥിതം bool no_new_privs = false;
#endif

ബാഹ്യ അക്ഷരം **environ;

#എങ്കിൽ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#എങ്കിൽ !defined(__DragonFly__)
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ ioprio_set(പൂർണ്ണസംഖ്യ which RC_UNUSED, പൂർണ്ണസംഖ്യ who RC_UNUSED,
			     പൂർണ്ണസംഖ്യ ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	തിരികെ syscall(SYS_ioprio_set, which, who, ioprio);
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif
}
#endif

സ്ഥിതം ശൂന്യം cleanup(ശൂന്യം)
{
	സ്വതന്ത്രം(changeuser);
}

RC_NORETURN സ്ഥിതം ശൂന്യം re_exec_supervisor(ശൂന്യം)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(അക്ഷരം *) ശൂന്യ);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

സ്ഥിതം ശൂന്യം handle_signal(പൂർണ്ണസംഖ്യ sig)
{
	പൂർണ്ണസംഖ്യ serrno = errno;
	pid_t pid;

	തിരഞ്ഞെടുപ്പ് (sig) {
	അവസ്ഥ SIGALRM:
		do_healthcheck = 1;
		നിർത്തുക;
	അവസ്ഥ SIGCHLD:
		എങ്കിൽ (exiting)
			ആകുമ്പോൾ (waitpid((pid_t)(-1), ശൂന്യ, WNOHANG) > 0) {}
		അല്ലെങ്കിൽ {
			ആകുമ്പോൾ ((pid = waitpid((pid_t)(-1), ശൂന്യ, WNOHANG|WNOWAIT)) > 0) {
				എങ്കിൽ (pid == child_pid)
					നിർത്തുക;
				pid = waitpid(pid, ശൂന്യ, WNOHANG);
			}
		}
		നിർത്തുക;
	അവസ്ഥ SIGTERM:
		exiting = 1;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

സ്ഥിതം അക്ഷരം * expand_home(സ്ഥിരം അക്ഷരം *home, സ്ഥിരം അക്ഷരം *path)
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

സ്ഥിതം അക്ഷരം *make_cmdline(അക്ഷരം **argv)
{
	അക്ഷരം **c;
	അക്ഷരം *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	വേണ്ടി (c = argv; c && *c; c++) {
		fputs(*c, mem);
		എങ്കിൽ (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	തിരികെ cmdline;
}

സ്ഥിതം pid_t exec_command(സ്ഥിരം അക്ഷരം *cmd)
{
	അക്ഷരം *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	ഘടന sigaction sa;

	file = rc_service_resolve(svcname);
	എങ്കിൽ (access(file, F_OK) != 0) {
		സ്വതന്ത്രം(file);
		തിരികെ 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, വലുപ്പം (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	എങ്കിൽ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ശൂന്യ);
		sigaction(SIGHUP, &sa, ശൂന്യ);
		sigaction(SIGINT, &sa, ശൂന്യ);
		sigaction(SIGQUIT, &sa, ശൂന്യ);
		sigaction(SIGTERM, &sa, ശൂന്യ);
		sigaction(SIGUSR1, &sa, ശൂന്യ);
		sigaction(SIGWINCH, &sa, ശൂന്യ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ശൂന്യ);

		/* Safe to run now */
		execl(file, file, cmd, (അക്ഷരം *) ശൂന്യ);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	എങ്കിൽ (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
	സ്വതന്ത്രം(file);
	തിരികെ pid;
}

RC_NORETURN സ്ഥിതം ശൂന്യം child_process(അക്ഷരം *exec, അക്ഷരം **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം *p;
	അക്ഷരം *token;
	size_t len;
	അക്ഷരം *newpath;
	അക്ഷരം *np;
	അക്ഷരം *cmdline = ശൂന്യ;
	time_t start_time;
	അക്ഷരം start_എണ്ണം_string[20];
	അക്ഷരം start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	പൂർണ്ണസംഖ്യ group_എണ്ണം = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = ശൂന്യ;
	പൂർണ്ണസംഖ്യ pamr;
	സ്ഥിരം അക്ഷരം *സ്ഥിരം *pamenv = ശൂന്യ;
#endif

	setsid();

	എങ്കിൽ (svcname) {
		start_time = time(ശൂന്യ);
		from_time_t(start_time_string, start_time);
		rc_service_മൂല്യം_set(svcname, "start_time", start_time_string);
		sprintf(start_എണ്ണം_string, "%i", respawn_എണ്ണം);
		rc_service_മൂല്യം_set(svcname, "start_count", start_എണ്ണം_string);
		sprintf(start_എണ്ണം_string, "%d", getpid());
		rc_service_മൂല്യം_set(svcname, "child_pid", start_എണ്ണം_string);
	}

	എങ്കിൽ (nicelevel != INT_MIN) {
		എങ്കിൽ (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
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

	എങ്കിൽ (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

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
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		എങ്കിൽ (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		എങ്കിൽ (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		എങ്കിൽ (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
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
		eerrorx("%s: chroot `%s': %s", applet, ch_വേര്, strerror(errno));

	എങ്കിൽ (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	എങ്കിൽ (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	എങ്കിൽ (changeuser && setgroups(group_എണ്ണം, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	എങ്കിൽ (group_list != group_buf)
		സ്വതന്ത്രം(group_list);
#ifdef __linux__
	എങ്കിൽ (uid && cap_setuid(uid))
#അല്ലെങ്കിൽ
	എങ്കിൽ (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

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

	/* remove the controlling tty */
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
		എങ്കിൽ ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	എങ്കിൽ (redirect_stdout) {
		എങ്കിൽ ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} അല്ലെങ്കിൽ എങ്കിൽ (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		എങ്കിൽ (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	എങ്കിൽ (redirect_stderr) {
		എങ്കിൽ ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} അല്ലെങ്കിൽ എങ്കിൽ (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		എങ്കിൽ (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	എങ്കിൽ (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	എങ്കിൽ (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	എങ്കിൽ (notify.type == NOTIFY_FD) {
		പൂർണ്ണസംഖ്യ error;

		എങ്കിൽ (notify.fd == notify.pipe[1]) {
			പൂർണ്ണസംഖ്യ flags = error = fcntl(notify.fd, F_GETFD, 0);
			എങ്കിൽ (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} അല്ലെങ്കിൽ {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		എങ്കിൽ (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	സ്വതന്ത്രം(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	എങ്കിൽ (changeuser != ശൂന്യ && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN സ്ഥിതം ശൂന്യം supervisor(അക്ഷരം *exec, അക്ഷരം **argv)
{
	FILE *fp;
	അക്ഷരം buf[2048];
	അക്ഷരം cmd[2048];
	പൂർണ്ണസംഖ്യ എണ്ണം;
	പൂർണ്ണസംഖ്യ failing;
	പൂർണ്ണസംഖ്യ health_status;
	പൂർണ്ണസംഖ്യ healthcheck_respawn;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ nkilled;
	പൂർണ്ണസംഖ്യ sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	ഘടന sigaction sa;
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
	memset(&sa, 0, വലുപ്പം(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ശൂന്യ);
	sigaction(SIGCHLD, &sa, ശൂന്യ);
	sigaction(SIGTERM, &sa, ശൂന്യ);

	fp = fopen(pidfile, "w");
	എങ്കിൽ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	എങ്കിൽ (svcname)
		rc_service_daemon_set(svcname, exec, (സ്ഥിരം അക്ഷരം * സ്ഥിരം *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	എങ്കിൽ (healthcheckdelay)
		alarm(healthcheckdelay);
	അല്ലെങ്കിൽ എങ്കിൽ (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	ആകുമ്പോൾ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		എങ്കിൽ (fifo_fd > 0) {
			memset(buf, 0, വലുപ്പം(buf));
			എണ്ണം = പഠ(fifo_fd, buf, വലുപ്പം(buf) - 1);
			close(fifo_fd);
			എങ്കിൽ (എണ്ണം != -1)
				buf[എണ്ണം] = 0;
			എങ്കിൽ (എണ്ണം == 0)
				തുടരുക;
			എങ്കിൽ (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			എങ്കിൽ (strncasecmp(buf, "sig", 3) == 0) {
				എങ്കിൽ ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					എങ്കിൽ (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			തുടരുക;
		}
		എങ്കിൽ (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			എങ്കിൽ (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			എങ്കിൽ (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			അല്ലെങ്കിൽ {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, ശൂന്യ, ശൂന്യ, child_pid, 0,
						stopgroup, false, false, true);
				എങ്കിൽ (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				അല്ലെങ്കിൽ
					healthcheck_respawn = 1;
			}
		}
		എങ്കിൽ (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, ശൂന്യ, ശൂന്യ, child_pid, 0,
					stopgroup, false, false, true);
			എങ്കിൽ (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			തുടരുക;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		എങ്കിൽ (wait_pid == child_pid) {
			എങ്കിൽ (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			അല്ലെങ്കിൽ എങ്കിൽ (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		എങ്കിൽ (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			എങ്കിൽ (first_spawn == 0)
				first_spawn = respawn_now;
			എങ്കിൽ ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_എണ്ണം = 0;
				first_spawn = 0;
			} അല്ലെങ്കിൽ
				respawn_എണ്ണം++;
			എങ്കിൽ (respawn_max > 0 && respawn_എണ്ണം > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				തുടരുക;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_എണ്ണം);
			എങ്കിൽ (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			എങ്കിൽ (exiting)
				തുടരുക;
			child_pid = fork();
			എങ്കിൽ (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			എങ്കിൽ (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, ശൂന്യ);
				memset(&sa, 0, വലുപ്പം(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ശൂന്യ);
				sigaction(SIGCHLD, &sa, ശൂന്യ);
				sigaction(SIGTERM, &sa, ശൂന്യ);
				child_process(exec, argv);
			}
			എങ്കിൽ (healthcheckdelay)
				alarm(healthcheckdelay);
			അല്ലെങ്കിൽ എങ്കിൽ (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	എങ്കിൽ (svcname) {
		rc_service_daemon_set(svcname, exec, (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)argv,
				pidfile, false);
		rc_service_മൂല്യം_set(svcname, "child_pid", ശൂന്യ);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		എങ്കിൽ (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	എങ്കിൽ (pidfile)
		unlink(pidfile);
	എങ്കിൽ (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ opt;
	അക്ഷരം **c;
	പൂർണ്ണസംഖ്യ x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	അക്ഷരം *exec = ശൂന്യ;
	അക്ഷരം *retry = ശൂന്യ;
	പൂർണ്ണസംഖ്യ sig = SIGTERM;
	അക്ഷരം *home = ശൂന്യ;
	പൂർണ്ണസംഖ്യ tid = 0;
	pid_t pid;
	അക്ഷരം *tmp;
	അക്ഷരം *p;
	അക്ഷരം *token;
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ n;
	അക്ഷരം *exec_file = ശൂന്യ;
	അക്ഷരം *varbuf = ശൂന്യ;
	അക്ഷരം numbuf[64];
	ഘടന timespec ts;
	ഘടന passwd *pw;
	ഘടന group *gr;
	FILE *fp;
	mode_t numask = 022;
	പൂർണ്ണസംഖ്യ child_argc = 0;
	അക്ഷരം **child_argv = ശൂന്യ;
	അക്ഷരം *str = ശൂന്യ;
	അക്ഷരം *cmdline = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *respawn_delay_str  = "0";
	സ്ഥിരം അക്ഷരം *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	എങ്കിൽ (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	എങ്കിൽ (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	എങ്കിൽ ((tmp = getenv("SSD_NICELEVEL")))
		എങ്കിൽ (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	എങ്കിൽ ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
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

	cmdline = make_cmdline(argv);
	എങ്കിൽ (svcname) {
		argc--;
		argv++;
	}
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (പൂർണ്ണസംഖ്യ *) 0)) != -1)
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'a':  /* --healthcheck-timer <time> */
			എങ്കിൽ (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			നിർത്തുക;

		അവസ്ഥ 'A':  /* --healthcheck-delay <time> */
			എങ്കിൽ (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			നിർത്തുക;

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

		അവസ്ഥ 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
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

		അവസ്ഥ 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			നിർത്തുക;

		അവസ്ഥ 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			നിർത്തുക;
		അവസ്ഥ 'S':  /* --start */
			start = true;
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

		അവസ്ഥ 'G': /* -G */
			stopgroup = true;
			നിർത്തുക;

		അവസ്ഥ 'k':
			എങ്കിൽ (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			നിർത്തുക;

		അവസ്ഥ 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			എങ്കിൽ (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			നിർത്തുക;

		അവസ്ഥ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			നിർത്തുക;

		അവസ്ഥ 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			നിർത്തുക;
		അവസ്ഥ 'r':  /* --chroot /new/root */
			ch_വേര് = optarg;
			നിർത്തുക;

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

		അവസ്ഥ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			നിർത്തുക;

		അവസ്ഥ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			നിർത്തുക;

		അവസ്ഥ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			നിർത്തുക;
		അവസ്ഥ '3':  /* --reexec */
			reexec = true;
			നിർത്തുക;

		അവസ്ഥ LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			നിർത്തുക;

		അവസ്ഥ LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			നിർത്തുക;

		അവസ്ഥ LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			നിർത്തുക;

		അവസ്ഥ LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			എങ്കിൽ (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			നിർത്തുക;

		അവസ്ഥ LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			എങ്കിൽ (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			നിർത്തുക;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	എങ്കിൽ (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	എങ്കിൽ (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	എങ്കിൽ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	എങ്കിൽ (ch_വേര് && *ch_വേര് == '~')
		ch_വേര് = expand_home(home, ch_വേര്);

	umask(numask);
	എങ്കിൽ (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	എങ്കിൽ (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	എങ്കിൽ (reexec) {
		str = rc_service_മൂല്യം_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * വലുപ്പം(അക്ഷരം *));
		memset(child_argv, 0, (child_argc + 1) * വലുപ്പം(അക്ഷരം *));
		വേണ്ടി (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_മൂല്യം_get(svcname, varbuf);
			child_argv[x] = str;
			സ്വതന്ത്രം(varbuf);
			varbuf = ശൂന്യ;
		}
		സ്വതന്ത്രം(str);
		str = rc_service_മൂല്യം_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		സ്വതന്ത്രം(str);
		exec = rc_service_മൂല്യം_get(svcname, "exec");
		pidfile = rc_service_മൂല്യം_get(svcname, "pidfile");
		retry = rc_service_മൂല്യം_get(svcname, "retry");
		എങ്കിൽ (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_മൂല്യം_set(svcname, "retry", retry);
		} അല്ലെങ്കിൽ
			parse_schedule(applet, ശൂന്യ, sig);

		str = rc_service_മൂല്യം_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_മൂല്യം_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_മൂല്യം_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_മൂല്യം_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} അല്ലെങ്കിൽ എങ്കിൽ (start) {
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
			എങ്കിൽ (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				സ്വതന്ത്രം(exec_file);
				exit(EXIT_FAILURE);
			}
		} അല്ലെങ്കിൽ
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		എങ്കിൽ (pid != -1)
			എങ്കിൽ (do_stop(applet, exec, (സ്ഥിരം അക്ഷരം * സ്ഥിരം *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		എങ്കിൽ (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		എങ്കിൽ (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		എങ്കിൽ (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_മൂല്യം_set(svcname, "retry", retry);
		} അല്ലെങ്കിൽ
			parse_schedule(applet, ശൂന്യ, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		സ്വതന്ത്രം(cmdline);
		cmdline = ശൂന്യ;

		/* Remove existing pidfile */
		എങ്കിൽ (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		എങ്കിൽ (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_മൂല്യം_set(svcname, "pidfile", pidfile);
		rc_service_മൂല്യം_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_മൂല്യം_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, വലുപ്പം(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_മൂല്യം_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, വലുപ്പം(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_മൂല്യം_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, വലുപ്പം(numbuf), "%i", respawn_max);
		rc_service_മൂല്യം_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		എങ്കിൽ (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		എങ്കിൽ (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		എങ്കിൽ (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		എങ്കിൽ (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		അല്ലെങ്കിൽ എങ്കിൽ (child_pid != 0) {
			എങ്കിൽ (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			ആകുമ്പോൾ (c && *c) {
				varbuf = ശൂന്യ;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_മൂല്യം_set(svcname, varbuf, *c);
				സ്വതന്ത്രം(varbuf);
				varbuf = ശൂന്യ;
				x++;
				c++;
			}
			snprintf(numbuf, വലുപ്പം(numbuf), "%d", x);
			rc_service_മൂല്യം_set(svcname, "argc", numbuf);
			rc_service_മൂല്യം_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} അല്ലെങ്കിൽ
			child_process(exec, argv);
	} അല്ലെങ്കിൽ എങ്കിൽ (stop) {
		pid = get_pid(applet, pidfile);
		എങ്കിൽ (pid != -1) {
			i = kill(pid, SIGTERM);
			എങ്കിൽ (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			അല്ലെങ്കിൽ {
				/* wait for the supervisor to go down */
				ആകുമ്പോൾ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ശൂന്യ);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		എങ്കിൽ (pidfile)
			unlink(pidfile);
		എങ്കിൽ (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (സ്ഥിരം അക്ഷരം *സ്ഥിരം *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} അല്ലെങ്കിൽ എങ്കിൽ (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		എങ്കിൽ (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = ലിഖ(fifo_fd, str, strlen(str));
		എങ്കിൽ (x == -1) {
			സ്വതന്ത്രം(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		സ്വതന്ത്രം(tmp);
		exit(EXIT_SUCCESS);
	}
}
