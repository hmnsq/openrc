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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <grp.h>
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <sys/resource.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>

#ifdef HAVE_PAM
#অন্তর্ভাব <security/pam_appl.h>

/* We are not supporting authentication conversations */
স্থির সংরচনা pam_conv conv = { রিক্ত, রিক্ত};
#endif

#ifdef __linux__
#অন্তর্ভাব <sys/capability.h>
#endif

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "rc_exec.h"
#অন্তর্ভাব "plugin.h"
#অন্তর্ভাব "schedules.h"
#অন্তর্ভাব "timeutils.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
গণনা {
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

ধ্রুব বর্ণ *applet = রিক্ত;
ধ্রুব বর্ণ *extraopts = রিক্ত;
ধ্রুব বর্ণ getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{ "healthcheck-timer",        1, রিক্ত, 'a'},
	{ "healthcheck-delay",        1, রিক্ত, 'A'},
	{ "capabilities", 1, রিক্ত, LONGOPT_CAPABILITIES},
	{ "secbits",      1, রিক্ত, LONGOPT_SECBITS},
	{ "no-new-privs", 0, রিক্ত, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, রিক্ত, 'D'},
	{ "respawn-delay-step",   1, রিক্ত, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, রিক্ত, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, রিক্ত, 'd'},
	{ "env",          1, রিক্ত, 'e'},
	{ "group",        1, রিক্ত, 'g'},
	{ "stop-group",   0, রিক্ত, 'G'},
	{ "ionice",       1, রিক্ত, 'I'},
	{ "stop",         0, রিক্ত, 'K'},
	{ "umask",        1, রিক্ত, 'k'},
	{ "respawn-max",    1, রিক্ত, 'm'},
	{ "nicelevel",    1, রিক্ত, 'N'},
	{ "oom-score-adj",1, রিক্ত, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, রিক্ত, 'p'},
	{ "respawn-period",        1, রিক্ত, 'P'},
	{ "retry",       1, রিক্ত, 'R'},
	{ "chroot",       1, রিক্ত, 'r'},
	{ "signal",       1, রিক্ত, 's'},
	{ "start",        0, রিক্ত, 'S'},
	{ "user",         1, রিক্ত, 'u'},
	{ "stdin",        1, রিক্ত, '0'},
	{ "stdout",       1, রিক্ত, '1'},
	{ "stderr",       1, রিক্ত, '2'},
	{ "stdout-logger",1, রিক্ত, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, রিক্ত, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, রিক্ত, '3'},
	{ "notify",       1, রিক্ত, LONGOPT_NOTIFY},
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
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
ধ্রুব বর্ণ *usagestring = রিক্ত;

স্থির পূর্ণাঙ্ক healthcheckdelay = 0;
স্থির পূর্ণাঙ্ক healthchecktimer = 0;
স্থির চঞ্চল sig_atomic_t do_healthcheck = 0;
স্থির চঞ্চল sig_atomic_t exiting = 0;
স্থির পূর্ণাঙ্ক nicelevel = INT_MIN;
স্থির পূর্ণাঙ্ক ionicec = -1;
স্থির পূর্ণাঙ্ক ioniced = 0;
স্থির পূর্ণাঙ্ক oom_score_adj = INT_MIN;
স্থির বর্ণ *changeuser, *ch_মূল, *ch_dir;
স্থির uid_t uid = 0;
স্থির gid_t gid = 0;
স্থির bool stopgroup = false;
স্থির পূর্ণাঙ্ক devnull_fd = -1;
স্থির পূর্ণাঙ্ক stdin_fd;
স্থির পূর্ণাঙ্ক stdout_fd;
স্থির পূর্ণাঙ্ক stderr_fd;
স্থির সংরচনা notify notify;
স্থির বর্ণ *redirect_stdin = রিক্ত;
স্থির বর্ণ *redirect_stderr = রিক্ত;
স্থির বর্ণ *redirect_stdout = রিক্ত;
স্থির বর্ণ *stderr_process = রিক্ত;
স্থির বর্ণ *stdout_process = রিক্ত;
#ifdef TIOCNOTTY
স্থির পূর্ণাঙ্ক tty_fd = -1;
#endif
স্থির pid_t child_pid;
স্থির পূর্ণাঙ্ক respawn_গুনতি = 0;
স্থির int64_t respawn_delay; /* default set inside main() */
স্থির int64_t respawn_delay_step = TM_MS(128);
স্থির int64_t respawn_delay_cap = TM_SEC(30);
স্থির int64_t respawn_period; /* default set inside main() */
স্থির পূর্ণাঙ্ক respawn_max = 10;
স্থির বর্ণ *fifopath = রিক্ত;
স্থির পূর্ণাঙ্ক fifo_fd = 0;
স্থির বর্ণ *pidfile = রিক্ত;
স্থির বর্ণ *svcname = রিক্ত;
স্থির bool verbose = false;
#ifdef __linux__
স্থির cap_iab_t cap_iab = রিক্ত;
স্থির unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
স্থির bool no_new_privs = false;
#endif

বাহ্য বর্ণ **environ;

#যদি !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#যদি !defined(__DragonFly__)
স্থির অন্তঃস্থ পূর্ণাঙ্ক ioprio_set(পূর্ণাঙ্ক which RC_UNUSED, পূর্ণাঙ্ক who RC_UNUSED,
			     পূর্ণাঙ্ক ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	প্রত্যাগম syscall(SYS_ioprio_set, which, who, ioprio);
#অন্যথা
	প্রত্যাগম 0;
#endif
}
#endif

স্থির শূন্য cleanup(শূন্য)
{
	মুক্ত(changeuser);
}

RC_NORETURN স্থির শূন্য re_exec_supervisor(শূন্য)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(বর্ণ *) রিক্ত);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

স্থির শূন্য handle_signal(পূর্ণাঙ্ক sig)
{
	পূর্ণাঙ্ক serrno = errno;
	pid_t pid;

	বিকল্প (sig) {
	পক্ষ SIGALRM:
		do_healthcheck = 1;
		ভঙ্গ;
	পক্ষ SIGCHLD:
		যদি (exiting)
			যাবত্ (waitpid((pid_t)(-1), রিক্ত, WNOHANG) > 0) {}
		অন্যথা {
			যাবত্ ((pid = waitpid((pid_t)(-1), রিক্ত, WNOHANG|WNOWAIT)) > 0) {
				যদি (pid == child_pid)
					ভঙ্গ;
				pid = waitpid(pid, রিক্ত, WNOHANG);
			}
		}
		ভঙ্গ;
	পক্ষ SIGTERM:
		exiting = 1;
		ভঙ্গ;
	স্বভাব:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

স্থির বর্ণ * expand_home(ধ্রুব বর্ণ *home, ধ্রুব বর্ণ *path)
{
	বর্ণ *opath, *ppath, *p, *nh;
	সংরচনা passwd *pw;

	যদি (!path || *path != '~')
		প্রত্যাগম xstrdup(path);

	opath = ppath = xstrdup(path);
	যদি (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		যদি (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		যদি (pw) {
			home = pw->pw_dir;
			ppath = p;
			যদি (ppath)
				*ppath = '/';
		} অন্যথা
			home = রিক্ত;
	} অন্যথা
		ppath++;

	যদি (!home) {
		মুক্ত(opath);
		প্রত্যাগম xstrdup(path);
	}
	যদি (!ppath) {
		মুক্ত(opath);
		প্রত্যাগম xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	মুক্ত(opath);
	প্রত্যাগম nh;
}

স্থির বর্ণ *make_cmdline(বর্ণ **argv)
{
	বর্ণ **c;
	বর্ণ *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	প্রতি (c = argv; c && *c; c++) {
		fputs(*c, mem);
		যদি (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	প্রত্যাগম cmdline;
}

স্থির pid_t exec_command(ধ্রুব বর্ণ *cmd)
{
	বর্ণ *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	সংরচনা sigaction sa;

	file = rc_service_resolve(svcname);
	যদি (access(file, F_OK) != 0) {
		মুক্ত(file);
		প্রত্যাগম 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, পরিমাণ (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	যদি (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, রিক্ত);
		sigaction(SIGHUP, &sa, রিক্ত);
		sigaction(SIGINT, &sa, রিক্ত);
		sigaction(SIGQUIT, &sa, রিক্ত);
		sigaction(SIGTERM, &sa, রিক্ত);
		sigaction(SIGUSR1, &sa, রিক্ত);
		sigaction(SIGWINCH, &sa, রিক্ত);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, রিক্ত);

		/* Safe to run now */
		execl(file, file, cmd, (বর্ণ *) রিক্ত);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	যদি (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, রিক্ত);
	মুক্ত(file);
	প্রত্যাগম pid;
}

RC_NORETURN স্থির শূন্য child_process(বর্ণ *exec, বর্ণ **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	পূর্ণাঙ্ক i;
	বর্ণ *p;
	বর্ণ *token;
	size_t len;
	বর্ণ *newpath;
	বর্ণ *np;
	বর্ণ *cmdline = রিক্ত;
	time_t start_time;
	বর্ণ start_গুনতি_string[20];
	বর্ণ start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	পূর্ণাঙ্ক group_গুনতি = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = রিক্ত;
	পূর্ণাঙ্ক pamr;
	ধ্রুব বর্ণ *ধ্রুব *pamenv = রিক্ত;
#endif

	setsid();

	যদি (svcname) {
		start_time = time(রিক্ত);
		from_time_t(start_time_string, start_time);
		rc_service_মান_set(svcname, "start_time", start_time_string);
		sprintf(start_গুনতি_string, "%i", respawn_গুনতি);
		rc_service_মান_set(svcname, "start_count", start_গুনতি_string);
		sprintf(start_গুনতি_string, "%d", getpid());
		rc_service_মান_set(svcname, "child_pid", start_গুনতি_string);
	}

	যদি (nicelevel != INT_MIN) {
		যদি (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		যদি (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} অন্যথা যদি (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	যদি (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	যদি (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		যদি (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	যদি (changeuser != রিক্ত) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		যদি (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		যদি (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		যদি (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	যদি (changeuser && getgrouplist(changeuser, gid, group_list, &group_গুনতি) < 0) {
		group_list = xmalloc(group_গুনতি * পরিমাণ(*group_list));
		যদি (getgrouplist(changeuser, gid, group_list, &group_গুনতি) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	যদি (ch_মূল && chroot(ch_মূল) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_মূল, strerror(errno));

	যদি (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	যদি (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	যদি (changeuser && setgroups(group_গুনতি, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	যদি (group_list != group_buf)
		মুক্ত(group_list);
#ifdef __linux__
	যদি (uid && cap_setuid(uid))
#অন্যথা
	যদি (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	যদি (cap_iab != রিক্ত) {
		i = cap_iab_set_proc(cap_iab);

		যদি (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		যদি (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	যদি (secbits != 0) {
		যদি (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	যদি (no_new_privs) {
		যদি (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	যাবত্ (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	যদি (changeuser != রিক্ত) {
		pamenv = (ধ্রুব বর্ণ *ধ্রুব *)pam_getenvlist(pamh);
		যদি (pamenv) {
			যাবত্ (*pamenv) {
				/* Don't add strings unless they set a var */
				যদি (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				অন্যথা
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		যদি (strncmp(env->মান, "RC_", 3) == 0
				|| strncmp(env->মান, "SSD_", 4) == 0
				|| strncmp(env->মান, "EINFO_", 6) == 0) {
			*strchr(env->মান, '=') = '\0';
			unsetenv(env->মান);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	যদি ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		যাবত্ (token && *token) {
			p = strchr(token, ':');
			যদি (p) {
				*p++ = '\0';
				যাবত্ (*p == ':')
					p++;
			}
			যদি (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				যদি (np != newpath)
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
	যদি (redirect_stdin) {
		যদি ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	যদি (redirect_stdout) {
		যদি ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} অন্যথা যদি (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		যদি (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	যদি (redirect_stderr) {
		যদি ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} অন্যথা যদি (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		যদি (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	যদি (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	যদি (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	যদি (notify.type == NOTIFY_FD) {
		পূর্ণাঙ্ক error;

		যদি (notify.fd == notify.pipe[1]) {
			পূর্ণাঙ্ক flags = error = fcntl(notify.fd, F_GETFD, 0);
			যদি (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} অন্যথা {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		যদি (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	মুক্ত(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	যদি (changeuser != রিক্ত && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN স্থির শূন্য supervisor(বর্ণ *exec, বর্ণ **argv)
{
	FILE *fp;
	বর্ণ buf[2048];
	বর্ণ cmd[2048];
	পূর্ণাঙ্ক গুনতি;
	পূর্ণাঙ্ক failing;
	পূর্ণাঙ্ক health_status;
	পূর্ণাঙ্ক healthcheck_respawn;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক nkilled;
	পূর্ণাঙ্ক sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	সংরচনা sigaction sa;
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
	memset(&sa, 0, পরিমাণ(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, রিক্ত);
	sigaction(SIGCHLD, &sa, রিক্ত);
	sigaction(SIGTERM, &sa, রিক্ত);

	fp = fopen(pidfile, "w");
	যদি (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	যদি (svcname)
		rc_service_daemon_set(svcname, exec, (ধ্রুব বর্ণ * ধ্রুব *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	যদি (healthcheckdelay)
		alarm(healthcheckdelay);
	অন্যথা যদি (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	যাবত্ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		যদি (fifo_fd > 0) {
			memset(buf, 0, পরিমাণ(buf));
			গুনতি = পঠ(fifo_fd, buf, পরিমাণ(buf) - 1);
			close(fifo_fd);
			যদি (গুনতি != -1)
				buf[গুনতি] = 0;
			যদি (গুনতি == 0)
				অনুবর্ত;
			যদি (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			যদি (strncasecmp(buf, "sig", 3) == 0) {
				যদি ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					যদি (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			অনুবর্ত;
		}
		যদি (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			যদি (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			যদি (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			অন্যথা {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, রিক্ত, রিক্ত, child_pid, 0,
						stopgroup, false, false, true);
				যদি (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				অন্যথা
					healthcheck_respawn = 1;
			}
		}
		যদি (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, রিক্ত, রিক্ত, child_pid, 0,
					stopgroup, false, false, true);
			যদি (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			অনুবর্ত;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		যদি (wait_pid == child_pid) {
			যদি (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			অন্যথা যদি (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		যদি (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			যদি (first_spawn == 0)
				first_spawn = respawn_now;
			যদি ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_গুনতি = 0;
				first_spawn = 0;
			} অন্যথা
				respawn_গুনতি++;
			যদি (respawn_max > 0 && respawn_গুনতি > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				অনুবর্ত;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_গুনতি);
			যদি (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			যদি (exiting)
				অনুবর্ত;
			child_pid = fork();
			যদি (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			যদি (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, রিক্ত);
				memset(&sa, 0, পরিমাণ(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, রিক্ত);
				sigaction(SIGCHLD, &sa, রিক্ত);
				sigaction(SIGTERM, &sa, রিক্ত);
				child_process(exec, argv);
			}
			যদি (healthcheckdelay)
				alarm(healthcheckdelay);
			অন্যথা যদি (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	যদি (svcname) {
		rc_service_daemon_set(svcname, exec, (ধ্রুব বর্ণ *ধ্রুব *)argv,
				pidfile, false);
		rc_service_মান_set(svcname, "child_pid", রিক্ত);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		যদি (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	যদি (pidfile)
		unlink(pidfile);
	যদি (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	পূর্ণাঙ্ক opt;
	বর্ণ **c;
	পূর্ণাঙ্ক x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	বর্ণ *exec = রিক্ত;
	বর্ণ *retry = রিক্ত;
	পূর্ণাঙ্ক sig = SIGTERM;
	বর্ণ *home = রিক্ত;
	পূর্ণাঙ্ক tid = 0;
	pid_t pid;
	বর্ণ *tmp;
	বর্ণ *p;
	বর্ণ *token;
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক n;
	বর্ণ *exec_file = রিক্ত;
	বর্ণ *varbuf = রিক্ত;
	বর্ণ numbuf[64];
	সংরচনা timespec ts;
	সংরচনা passwd *pw;
	সংরচনা group *gr;
	FILE *fp;
	mode_t numask = 022;
	পূর্ণাঙ্ক child_argc = 0;
	বর্ণ **child_argv = রিক্ত;
	বর্ণ *str = রিক্ত;
	বর্ণ *cmdline = রিক্ত;
	ধ্রুব বর্ণ *respawn_delay_str  = "0";
	ধ্রুব বর্ণ *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	যদি (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	যদি (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	যদি ((tmp = getenv("SSD_NICELEVEL")))
		যদি (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	যদি ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		যদি (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		যদি (ionicec == 0)
			ioniced = 0;
		অন্যথা যদি (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	যদি ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		যদি (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	যদি (home == রিক্ত || p == রিক্ত) {
		pw = getpwuid(getuid());
		যদি (pw != রিক্ত) {
			যদি (p == রিক্ত)
				setenv("USER", pw->pw_name, 1);
			যদি (home == রিক্ত) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	যদি (svcname) {
		argc--;
		argv++;
	}
	যাবত্ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (পূর্ণাঙ্ক *) 0)) != -1)
		বিকল্প (opt) {
		পক্ষ 'a':  /* --healthcheck-timer <time> */
			যদি (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			ভঙ্গ;

		পক্ষ 'A':  /* --healthcheck-delay <time> */
			যদি (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			ভঙ্গ;

		পক্ষ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			যদি (cap_iab == রিক্ত)
				eerrorx("Could not parse iab: %s", strerror(errno));
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			ভঙ্গ;

		পক্ষ LONGOPT_SECBITS:
#ifdef __linux__
			যদি (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = রিক্ত;
			secbits = strtoul(optarg, &tmp, 0);
			যদি (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			ভঙ্গ;

		পক্ষ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#অন্যথা
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			ভঙ্গ;

		পক্ষ 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			ভঙ্গ;

		পক্ষ 'I': /* --ionice */
			যদি (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			যদি (ionicec == 0)
				ioniced = 0;
			অন্যথা যদি (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ভঙ্গ;

		পক্ষ 'K':  /* --stop */
			stop = true;
			ভঙ্গ;

		পক্ষ 'N':  /* --nice */
			যদি (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ভঙ্গ;

		পক্ষ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			যদি (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ভঙ্গ;

		পক্ষ 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			ভঙ্গ;

		পক্ষ 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			ভঙ্গ;
		পক্ষ 'S':  /* --start */
			start = true;
			ভঙ্গ;

		পক্ষ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ভঙ্গ;

		পক্ষ 'e': /* --env */
			putenv(optarg);
			ভঙ্গ;

		পক্ষ 'g':  /* --group <group>|<gid> */
			যদি (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			অন্যথা
				gr = getgrgid((gid_t)tid);
			যদি (gr == রিক্ত)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ভঙ্গ;

		পক্ষ 'G': /* -G */
			stopgroup = true;
			ভঙ্গ;

		পক্ষ 'k':
			যদি (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ভঙ্গ;

		পক্ষ 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			যদি (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			ভঙ্গ;

		পক্ষ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ভঙ্গ;

		পক্ষ 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			ভঙ্গ;
		পক্ষ 'r':  /* --chroot /new/root */
			ch_মূল = optarg;
			ভঙ্গ;

		পক্ষ 'u':  /* --user <username>|<uid> */
		{
			বর্ণ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			যদি (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			অন্যথা
				pw = getpwuid((uid_t)tid);

			যদি (pw == রিক্ত)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			যদি (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			যদি (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			যদি (gid == 0)
				gid = pw->pw_gid;

			যদি (p) {
				tmp = strsep (&p, ":");
				যদি (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				অন্যথা
					gr = getgrgid((gid_t) tid);

				যদি (gr == রিক্ত)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ভঙ্গ;

		পক্ষ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ভঙ্গ;

		পক্ষ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ভঙ্গ;

		পক্ষ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ভঙ্গ;
		পক্ষ '3':  /* --reexec */
			reexec = true;
			ভঙ্গ;

		পক্ষ LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ভঙ্গ;

		পক্ষ LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ভঙ্গ;

		পক্ষ LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			ভঙ্গ;

		পক্ষ LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			যদি (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			ভঙ্গ;

		পক্ষ LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			যদি (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			ভঙ্গ;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	যদি (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	যদি (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	যদি (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	যদি (ch_মূল && *ch_মূল == '~')
		ch_মূল = expand_home(home, ch_মূল);

	umask(numask);
	যদি (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	যদি (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	যদি (reexec) {
		str = rc_service_মান_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * পরিমাণ(বর্ণ *));
		memset(child_argv, 0, (child_argc + 1) * পরিমাণ(বর্ণ *));
		প্রতি (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_মান_get(svcname, varbuf);
			child_argv[x] = str;
			মুক্ত(varbuf);
			varbuf = রিক্ত;
		}
		মুক্ত(str);
		str = rc_service_মান_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		মুক্ত(str);
		exec = rc_service_মান_get(svcname, "exec");
		pidfile = rc_service_মান_get(svcname, "pidfile");
		retry = rc_service_মান_get(svcname, "retry");
		যদি (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_মান_set(svcname, "retry", retry);
		} অন্যথা
			parse_schedule(applet, রিক্ত, sig);

		str = rc_service_মান_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_মান_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_মান_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_মান_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} অন্যথা যদি (start) {
		যদি (exec) {
			যদি (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			যদি (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				যদি (ch_মূল)
					xasprintf(&exec_file, "%s/%s", ch_মূল, exec);
				অন্যথা
					xasprintf(&exec_file, "%s", exec);
			} অন্যথা {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = রিক্ত;
				যাবত্ ((token = strsep(&p, ":"))) {
					যদি (ch_মূল)
						xasprintf(&exec_file, "%s/%s/%s", ch_মূল, token, exec);
					অন্যথা
						xasprintf(&exec_file, "%s/%s", token, exec);
					যদি (exec_file && access(exec_file, F_OK) == 0)
						ভঙ্গ;
					মুক্ত(exec_file);
					exec_file = রিক্ত;
				}
				মুক্ত(tmp);
			}
			যদি (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				মুক্ত(exec_file);
				exit(EXIT_FAILURE);
			}
		} অন্যথা
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		যদি (pid != -1)
			যদি (do_stop(applet, exec, (ধ্রুব বর্ণ * ধ্রুব *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		যদি (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		যদি (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		যদি (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_মান_set(svcname, "retry", retry);
		} অন্যথা
			parse_schedule(applet, রিক্ত, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		মুক্ত(cmdline);
		cmdline = রিক্ত;

		/* Remove existing pidfile */
		যদি (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		যদি (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_মান_set(svcname, "pidfile", pidfile);
		rc_service_মান_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_মান_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, পরিমাণ(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_মান_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, পরিমাণ(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_মান_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, পরিমাণ(numbuf), "%i", respawn_max);
		rc_service_মান_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		যদি (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		যদি (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		যদি (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		যদি (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		অন্যথা যদি (child_pid != 0) {
			যদি (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			যাবত্ (c && *c) {
				varbuf = রিক্ত;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_মান_set(svcname, varbuf, *c);
				মুক্ত(varbuf);
				varbuf = রিক্ত;
				x++;
				c++;
			}
			snprintf(numbuf, পরিমাণ(numbuf), "%d", x);
			rc_service_মান_set(svcname, "argc", numbuf);
			rc_service_মান_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} অন্যথা
			child_process(exec, argv);
	} অন্যথা যদি (stop) {
		pid = get_pid(applet, pidfile);
		যদি (pid != -1) {
			i = kill(pid, SIGTERM);
			যদি (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			অন্যথা {
				/* wait for the supervisor to go down */
				যাবত্ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, রিক্ত);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		যদি (pidfile)
			unlink(pidfile);
		যদি (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (ধ্রুব বর্ণ *ধ্রুব *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} অন্যথা যদি (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		যদি (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = লিখ(fifo_fd, str, strlen(str));
		যদি (x == -1) {
			মুক্ত(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		মুক্ত(tmp);
		exit(EXIT_SUCCESS);
	}
}
