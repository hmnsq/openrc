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

#شامل <errno.h>
#شامل <inttypes.h>
#شامل <fcntl.h>
#شامل <getopt.h>
#شامل <limits.h>
#شامل <grp.h>
#شامل <pwd.h>
#شامل <signal.h>
#شامل <stddef.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <stdbool.h>
#شامل <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#شامل <syslog.h>
#شامل <sys/ioctl.h>
#شامل <sys/resource.h>
#شامل <sys/stat.h>
#شامل <sys/wait.h>
#شامل <time.h>
#شامل <unistd.h>

#ifdef HAVE_PAM
#شامل <security/pam_appl.h>

/* We are not supporting authentication conversations */
ایستا ساختار pam_conv conv = { پوچ, پوچ};
#endif

#ifdef __linux__
#شامل <sys/capability.h>
#endif

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "rc_exec.h"
#شامل "plugin.h"
#شامل "schedules.h"
#شامل "timeutils.h"
#شامل "_usage.h"
#شامل "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
شمارش {
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

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "healthcheck-timer",        1, پوچ, 'a'},
	{ "healthcheck-delay",        1, پوچ, 'A'},
	{ "capabilities", 1, پوچ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, پوچ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, پوچ, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, پوچ, 'D'},
	{ "respawn-delay-step",   1, پوچ, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, پوچ, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, پوچ, 'd'},
	{ "env",          1, پوچ, 'e'},
	{ "group",        1, پوچ, 'g'},
	{ "stop-group",   0, پوچ, 'G'},
	{ "ionice",       1, پوچ, 'I'},
	{ "stop",         0, پوچ, 'K'},
	{ "umask",        1, پوچ, 'k'},
	{ "respawn-max",    1, پوچ, 'm'},
	{ "nicelevel",    1, پوچ, 'N'},
	{ "oom-score-adj",1, پوچ, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, پوچ, 'p'},
	{ "respawn-period",        1, پوچ, 'P'},
	{ "retry",       1, پوچ, 'R'},
	{ "chroot",       1, پوچ, 'r'},
	{ "signal",       1, پوچ, 's'},
	{ "start",        0, پوچ, 'S'},
	{ "user",         1, پوچ, 'u'},
	{ "stdin",        1, پوچ, '0'},
	{ "stdout",       1, پوچ, '1'},
	{ "stderr",       1, پوچ, '2'},
	{ "stdout-logger",1, پوچ, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, پوچ, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, پوچ, '3'},
	{ "notify",       1, پوچ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = پوچ;

ایستا صحیح healthcheckdelay = 0;
ایستا صحیح healthchecktimer = 0;
ایستا فرار sig_atomic_t do_healthcheck = 0;
ایستا فرار sig_atomic_t exiting = 0;
ایستا صحیح nicelevel = INT_MIN;
ایستا صحیح ionicec = -1;
ایستا صحیح ioniced = 0;
ایستا صحیح oom_score_adj = INT_MIN;
ایستا نویسه *changeuser, *ch_ریشه, *ch_dir;
ایستا uid_t uid = 0;
ایستا gid_t gid = 0;
ایستا bool stopgroup = false;
ایستا صحیح devnull_fd = -1;
ایستا صحیح stdin_fd;
ایستا صحیح stdout_fd;
ایستا صحیح stderr_fd;
ایستا ساختار notify notify;
ایستا نویسه *redirect_stdin = پوچ;
ایستا نویسه *redirect_stderr = پوچ;
ایستا نویسه *redirect_stdout = پوچ;
ایستا نویسه *stderr_process = پوچ;
ایستا نویسه *stdout_process = پوچ;
#ifdef TIOCNOTTY
ایستا صحیح tty_fd = -1;
#endif
ایستا pid_t child_pid;
ایستا صحیح respawn_شمار = 0;
ایستا int64_t respawn_delay; /* default set inside main() */
ایستا int64_t respawn_delay_step = TM_MS(128);
ایستا int64_t respawn_delay_cap = TM_SEC(30);
ایستا int64_t respawn_period; /* default set inside main() */
ایستا صحیح respawn_max = 10;
ایستا نویسه *fifopath = پوچ;
ایستا صحیح fifo_fd = 0;
ایستا نویسه *pidfile = پوچ;
ایستا نویسه *svcname = پوچ;
ایستا bool verbose = false;
#ifdef __linux__
ایستا cap_iab_t cap_iab = پوچ;
ایستا unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
ایستا bool no_new_privs = false;
#endif

بیرونی نویسه **environ;

#اگر !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#اگر !defined(__DragonFly__)
ایستا درونخطی صحیح ioprio_set(صحیح which RC_UNUSED, صحیح who RC_UNUSED,
			     صحیح ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	بازگشت syscall(SYS_ioprio_set, which, who, ioprio);
#وگرنه
	بازگشت 0;
#endif
}
#endif

ایستا تهی cleanup(تهی)
{
	آزاد(changeuser);
}

RC_NORETURN ایستا تهی re_exec_supervisor(تهی)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(نویسه *) پوچ);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

ایستا تهی handle_signal(صحیح sig)
{
	صحیح serrno = errno;
	pid_t pid;

	انتخاب (sig) {
	حالت SIGALRM:
		do_healthcheck = 1;
		بشکن;
	حالت SIGCHLD:
		اگر (exiting)
			وقتی (waitpid((pid_t)(-1), پوچ, WNOHANG) > 0) {}
		وگرنه {
			وقتی ((pid = waitpid((pid_t)(-1), پوچ, WNOHANG|WNOWAIT)) > 0) {
				اگر (pid == child_pid)
					بشکن;
				pid = waitpid(pid, پوچ, WNOHANG);
			}
		}
		بشکن;
	حالت SIGTERM:
		exiting = 1;
		بشکن;
	پیشفرض:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ایستا نویسه * expand_home(ثابت نویسه *home, ثابت نویسه *path)
{
	نویسه *opath, *ppath, *p, *nh;
	ساختار passwd *pw;

	اگر (!path || *path != '~')
		بازگشت xstrdup(path);

	opath = ppath = xstrdup(path);
	اگر (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		اگر (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		اگر (pw) {
			home = pw->pw_dir;
			ppath = p;
			اگر (ppath)
				*ppath = '/';
		} وگرنه
			home = پوچ;
	} وگرنه
		ppath++;

	اگر (!home) {
		آزاد(opath);
		بازگشت xstrdup(path);
	}
	اگر (!ppath) {
		آزاد(opath);
		بازگشت xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	آزاد(opath);
	بازگشت nh;
}

ایستا نویسه *make_cmdline(نویسه **argv)
{
	نویسه **c;
	نویسه *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	برای (c = argv; c && *c; c++) {
		fputs(*c, mem);
		اگر (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	بازگشت cmdline;
}

ایستا pid_t exec_command(ثابت نویسه *cmd)
{
	نویسه *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	ساختار sigaction sa;

	file = rc_service_resolve(svcname);
	اگر (access(file, F_OK) != 0) {
		آزاد(file);
		بازگشت 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, اندازه (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	اگر (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, پوچ);
		sigaction(SIGHUP, &sa, پوچ);
		sigaction(SIGINT, &sa, پوچ);
		sigaction(SIGQUIT, &sa, پوچ);
		sigaction(SIGTERM, &sa, پوچ);
		sigaction(SIGUSR1, &sa, پوچ);
		sigaction(SIGWINCH, &sa, پوچ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, پوچ);

		/* Safe to run now */
		execl(file, file, cmd, (نویسه *) پوچ);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	اگر (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, پوچ);
	آزاد(file);
	بازگشت pid;
}

RC_NORETURN ایستا تهی child_process(نویسه *exec, نویسه **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	صحیح i;
	نویسه *p;
	نویسه *token;
	size_t len;
	نویسه *newpath;
	نویسه *np;
	نویسه *cmdline = پوچ;
	time_t start_time;
	نویسه start_شمار_string[20];
	نویسه start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	صحیح group_شمار = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = پوچ;
	صحیح pamr;
	ثابت نویسه *ثابت *pamenv = پوچ;
#endif

	setsid();

	اگر (svcname) {
		start_time = time(پوچ);
		from_time_t(start_time_string, start_time);
		rc_service_مقدار_set(svcname, "start_time", start_time_string);
		sprintf(start_شمار_string, "%i", respawn_شمار);
		rc_service_مقدار_set(svcname, "start_count", start_شمار_string);
		sprintf(start_شمار_string, "%d", getpid());
		rc_service_مقدار_set(svcname, "child_pid", start_شمار_string);
	}

	اگر (nicelevel != INT_MIN) {
		اگر (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		اگر (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} وگرنه اگر (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	اگر (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	اگر (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		اگر (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	اگر (changeuser != پوچ) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		اگر (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		اگر (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		اگر (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	اگر (changeuser && getgrouplist(changeuser, gid, group_list, &group_شمار) < 0) {
		group_list = xmalloc(group_شمار * اندازه(*group_list));
		اگر (getgrouplist(changeuser, gid, group_list, &group_شمار) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	اگر (ch_ریشه && chroot(ch_ریشه) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_ریشه, strerror(errno));

	اگر (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	اگر (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	اگر (changeuser && setgroups(group_شمار, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	اگر (group_list != group_buf)
		آزاد(group_list);
#ifdef __linux__
	اگر (uid && cap_setuid(uid))
#وگرنه
	اگر (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	اگر (cap_iab != پوچ) {
		i = cap_iab_set_proc(cap_iab);

		اگر (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		اگر (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	اگر (secbits != 0) {
		اگر (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	اگر (no_new_privs) {
		اگر (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	وقتی (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	اگر (changeuser != پوچ) {
		pamenv = (ثابت نویسه *ثابت *)pam_getenvlist(pamh);
		اگر (pamenv) {
			وقتی (*pamenv) {
				/* Don't add strings unless they set a var */
				اگر (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				وگرنه
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		اگر (strncmp(env->مقدار, "RC_", 3) == 0
				|| strncmp(env->مقدار, "SSD_", 4) == 0
				|| strncmp(env->مقدار, "EINFO_", 6) == 0) {
			*strchr(env->مقدار, '=') = '\0';
			unsetenv(env->مقدار);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	اگر ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		وقتی (token && *token) {
			p = strchr(token, ':');
			اگر (p) {
				*p++ = '\0';
				وقتی (*p == ':')
					p++;
			}
			اگر (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				اگر (np != newpath)
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
	اگر (redirect_stdin) {
		اگر ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	اگر (redirect_stdout) {
		اگر ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} وگرنه اگر (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		اگر (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	اگر (redirect_stderr) {
		اگر ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} وگرنه اگر (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		اگر (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	اگر (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	اگر (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	اگر (notify.type == NOTIFY_FD) {
		صحیح error;

		اگر (notify.fd == notify.pipe[1]) {
			صحیح flags = error = fcntl(notify.fd, F_GETFD, 0);
			اگر (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} وگرنه {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		اگر (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	آزاد(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	اگر (changeuser != پوچ && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ایستا تهی supervisor(نویسه *exec, نویسه **argv)
{
	FILE *fp;
	نویسه buf[2048];
	نویسه cmd[2048];
	صحیح شمار;
	صحیح failing;
	صحیح health_status;
	صحیح healthcheck_respawn;
	صحیح i;
	صحیح nkilled;
	صحیح sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	ساختار sigaction sa;
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
	memset(&sa, 0, اندازه(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, پوچ);
	sigaction(SIGCHLD, &sa, پوچ);
	sigaction(SIGTERM, &sa, پوچ);

	fp = fopen(pidfile, "w");
	اگر (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	اگر (svcname)
		rc_service_daemon_set(svcname, exec, (ثابت نویسه * ثابت *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	اگر (healthcheckdelay)
		alarm(healthcheckdelay);
	وگرنه اگر (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	وقتی (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		اگر (fifo_fd > 0) {
			memset(buf, 0, اندازه(buf));
			شمار = اقرأ(fifo_fd, buf, اندازه(buf) - 1);
			close(fifo_fd);
			اگر (شمار != -1)
				buf[شمار] = 0;
			اگر (شمار == 0)
				ادامه;
			اگر (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			اگر (strncasecmp(buf, "sig", 3) == 0) {
				اگر ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					اگر (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			ادامه;
		}
		اگر (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			اگر (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			اگر (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			وگرنه {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, پوچ, پوچ, child_pid, 0,
						stopgroup, false, false, true);
				اگر (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				وگرنه
					healthcheck_respawn = 1;
			}
		}
		اگر (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, پوچ, پوچ, child_pid, 0,
					stopgroup, false, false, true);
			اگر (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			ادامه;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		اگر (wait_pid == child_pid) {
			اگر (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			وگرنه اگر (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		اگر (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			اگر (first_spawn == 0)
				first_spawn = respawn_now;
			اگر ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_شمار = 0;
				first_spawn = 0;
			} وگرنه
				respawn_شمار++;
			اگر (respawn_max > 0 && respawn_شمار > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				ادامه;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_شمار);
			اگر (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			اگر (exiting)
				ادامه;
			child_pid = fork();
			اگر (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			اگر (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, پوچ);
				memset(&sa, 0, اندازه(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, پوچ);
				sigaction(SIGCHLD, &sa, پوچ);
				sigaction(SIGTERM, &sa, پوچ);
				child_process(exec, argv);
			}
			اگر (healthcheckdelay)
				alarm(healthcheckdelay);
			وگرنه اگر (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	اگر (svcname) {
		rc_service_daemon_set(svcname, exec, (ثابت نویسه *ثابت *)argv,
				pidfile, false);
		rc_service_مقدار_set(svcname, "child_pid", پوچ);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		اگر (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	اگر (pidfile)
		unlink(pidfile);
	اگر (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح opt;
	نویسه **c;
	صحیح x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	نویسه *exec = پوچ;
	نویسه *retry = پوچ;
	صحیح sig = SIGTERM;
	نویسه *home = پوچ;
	صحیح tid = 0;
	pid_t pid;
	نویسه *tmp;
	نویسه *p;
	نویسه *token;
	صحیح i;
	صحیح n;
	نویسه *exec_file = پوچ;
	نویسه *varbuf = پوچ;
	نویسه numbuf[64];
	ساختار timespec ts;
	ساختار passwd *pw;
	ساختار group *gr;
	FILE *fp;
	mode_t numask = 022;
	صحیح child_argc = 0;
	نویسه **child_argv = پوچ;
	نویسه *str = پوچ;
	نویسه *cmdline = پوچ;
	ثابت نویسه *respawn_delay_str  = "0";
	ثابت نویسه *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	اگر (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	اگر (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	اگر ((tmp = getenv("SSD_NICELEVEL")))
		اگر (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	اگر ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		اگر (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		اگر (ionicec == 0)
			ioniced = 0;
		وگرنه اگر (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	اگر ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		اگر (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	اگر (home == پوچ || p == پوچ) {
		pw = getpwuid(getuid());
		اگر (pw != پوچ) {
			اگر (p == پوچ)
				setenv("USER", pw->pw_name, 1);
			اگر (home == پوچ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	اگر (svcname) {
		argc--;
		argv++;
	}
	وقتی ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحیح *) 0)) != -1)
		انتخاب (opt) {
		حالت 'a':  /* --healthcheck-timer <time> */
			اگر (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			بشکن;

		حالت 'A':  /* --healthcheck-delay <time> */
			اگر (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			بشکن;

		حالت LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			اگر (cap_iab == پوچ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#وگرنه
			eerrorx("Capabilities support not enabled");
#endif
			بشکن;

		حالت LONGOPT_SECBITS:
#ifdef __linux__
			اگر (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = پوچ;
			secbits = strtoul(optarg, &tmp, 0);
			اگر (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#وگرنه
			eerrorx("Capabilities support not enabled");
#endif
			بشکن;

		حالت LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#وگرنه
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			بشکن;

		حالت 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			بشکن;

		حالت 'I': /* --ionice */
			اگر (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			اگر (ionicec == 0)
				ioniced = 0;
			وگرنه اگر (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			بشکن;

		حالت 'K':  /* --stop */
			stop = true;
			بشکن;

		حالت 'N':  /* --nice */
			اگر (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			بشکن;

		حالت LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			اگر (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			بشکن;

		حالت 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			بشکن;

		حالت 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			بشکن;
		حالت 'S':  /* --start */
			start = true;
			بشکن;

		حالت 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			بشکن;

		حالت 'e': /* --env */
			putenv(optarg);
			بشکن;

		حالت 'g':  /* --group <group>|<gid> */
			اگر (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			وگرنه
				gr = getgrgid((gid_t)tid);
			اگر (gr == پوچ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			بشکن;

		حالت 'G': /* -G */
			stopgroup = true;
			بشکن;

		حالت 'k':
			اگر (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			بشکن;

		حالت 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			اگر (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			بشکن;

		حالت 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			بشکن;

		حالت 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			بشکن;
		حالت 'r':  /* --chroot /new/root */
			ch_ریشه = optarg;
			بشکن;

		حالت 'u':  /* --user <username>|<uid> */
		{
			نویسه dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			اگر (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			وگرنه
				pw = getpwuid((uid_t)tid);

			اگر (pw == پوچ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			اگر (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			اگر (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			اگر (gid == 0)
				gid = pw->pw_gid;

			اگر (p) {
				tmp = strsep (&p, ":");
				اگر (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				وگرنه
					gr = getgrgid((gid_t) tid);

				اگر (gr == پوچ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		بشکن;

		حالت '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			بشکن;

		حالت '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			بشکن;

		حالت '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			بشکن;
		حالت '3':  /* --reexec */
			reexec = true;
			بشکن;

		حالت LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			بشکن;

		حالت LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			بشکن;

		حالت LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			بشکن;

		حالت LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			اگر (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			بشکن;

		حالت LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			اگر (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			بشکن;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	اگر (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	اگر (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	اگر (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	اگر (ch_ریشه && *ch_ریشه == '~')
		ch_ریشه = expand_home(home, ch_ریشه);

	umask(numask);
	اگر (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	اگر (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	اگر (reexec) {
		str = rc_service_مقدار_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * اندازه(نویسه *));
		memset(child_argv, 0, (child_argc + 1) * اندازه(نویسه *));
		برای (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_مقدار_get(svcname, varbuf);
			child_argv[x] = str;
			آزاد(varbuf);
			varbuf = پوچ;
		}
		آزاد(str);
		str = rc_service_مقدار_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		آزاد(str);
		exec = rc_service_مقدار_get(svcname, "exec");
		pidfile = rc_service_مقدار_get(svcname, "pidfile");
		retry = rc_service_مقدار_get(svcname, "retry");
		اگر (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_مقدار_set(svcname, "retry", retry);
		} وگرنه
			parse_schedule(applet, پوچ, sig);

		str = rc_service_مقدار_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_مقدار_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_مقدار_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_مقدار_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} وگرنه اگر (start) {
		اگر (exec) {
			اگر (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			اگر (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				اگر (ch_ریشه)
					xasprintf(&exec_file, "%s/%s", ch_ریشه, exec);
				وگرنه
					xasprintf(&exec_file, "%s", exec);
			} وگرنه {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = پوچ;
				وقتی ((token = strsep(&p, ":"))) {
					اگر (ch_ریشه)
						xasprintf(&exec_file, "%s/%s/%s", ch_ریشه, token, exec);
					وگرنه
						xasprintf(&exec_file, "%s/%s", token, exec);
					اگر (exec_file && access(exec_file, F_OK) == 0)
						بشکن;
					آزاد(exec_file);
					exec_file = پوچ;
				}
				آزاد(tmp);
			}
			اگر (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				آزاد(exec_file);
				exit(EXIT_FAILURE);
			}
		} وگرنه
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		اگر (pid != -1)
			اگر (do_stop(applet, exec, (ثابت نویسه * ثابت *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		اگر (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		اگر (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		اگر (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_مقدار_set(svcname, "retry", retry);
		} وگرنه
			parse_schedule(applet, پوچ, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		آزاد(cmdline);
		cmdline = پوچ;

		/* Remove existing pidfile */
		اگر (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		اگر (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_مقدار_set(svcname, "pidfile", pidfile);
		rc_service_مقدار_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_مقدار_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, اندازه(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_مقدار_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, اندازه(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_مقدار_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, اندازه(numbuf), "%i", respawn_max);
		rc_service_مقدار_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		اگر (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		اگر (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		اگر (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		اگر (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		وگرنه اگر (child_pid != 0) {
			اگر (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			وقتی (c && *c) {
				varbuf = پوچ;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_مقدار_set(svcname, varbuf, *c);
				آزاد(varbuf);
				varbuf = پوچ;
				x++;
				c++;
			}
			snprintf(numbuf, اندازه(numbuf), "%d", x);
			rc_service_مقدار_set(svcname, "argc", numbuf);
			rc_service_مقدار_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} وگرنه
			child_process(exec, argv);
	} وگرنه اگر (stop) {
		pid = get_pid(applet, pidfile);
		اگر (pid != -1) {
			i = kill(pid, SIGTERM);
			اگر (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			وگرنه {
				/* wait for the supervisor to go down */
				وقتی (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, پوچ);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		اگر (pidfile)
			unlink(pidfile);
		اگر (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (ثابت نویسه *ثابت *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} وگرنه اگر (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		اگر (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = اكتب(fifo_fd, str, strlen(str));
		اگر (x == -1) {
			آزاد(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		آزاد(tmp);
		exit(EXIT_SUCCESS);
	}
}
