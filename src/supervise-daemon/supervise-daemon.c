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
#اگر_تعریف_شدہ __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#اگر_ختم
#شامل <syslog.h>
#شامل <sys/ioctl.h>
#شامل <sys/resource.h>
#شامل <sys/stat.h>
#شامل <sys/wait.h>
#شامل <time.h>
#شامل <unistd.h>

#اگر_تعریف_شدہ HAVE_PAM
#شامل <security/pam_appl.h>

/* We are not supporting authentication conversations */
ساکن ساخت pam_conv conv = { تہی, تہی};
#اگر_ختم

#اگر_تعریف_شدہ __linux__
#شامل <sys/capability.h>
#اگر_ختم

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
شمار {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_بنیاد = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_قدم,
  LONGOPT_RESPAWN_DELAY_CAP,
};

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "healthcheck-timer",        1, تہی, 'a'},
	{ "healthcheck-delay",        1, تہی, 'A'},
	{ "capabilities", 1, تہی, LONGOPT_CAPABILITIES},
	{ "secbits",      1, تہی, LONGOPT_SECBITS},
	{ "no-new-privs", 0, تہی, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, تہی, 'D'},
	{ "respawn-delay-step",   1, تہی, LONGOPT_RESPAWN_DELAY_قدم},
	{ "respawn-delay-cap",    1, تہی, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, تہی, 'd'},
	{ "env",          1, تہی, 'e'},
	{ "group",        1, تہی, 'g'},
	{ "stop-group",   0, تہی, 'G'},
	{ "ionice",       1, تہی, 'I'},
	{ "stop",         0, تہی, 'K'},
	{ "umask",        1, تہی, 'k'},
	{ "respawn-max",    1, تہی, 'm'},
	{ "nicelevel",    1, تہی, 'N'},
	{ "oom-score-adj",1, تہی, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, تہی, 'p'},
	{ "respawn-period",        1, تہی, 'P'},
	{ "retry",       1, تہی, 'R'},
	{ "chroot",       1, تہی, 'r'},
	{ "signal",       1, تہی, 's'},
	{ "start",        0, تہی, 'S'},
	{ "user",         1, تہی, 'u'},
	{ "stdin",        1, تہی, '0'},
	{ "stdout",       1, تہی, '1'},
	{ "stderr",       1, تہی, '2'},
	{ "stdout-logger",1, تہی, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, تہی, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, تہی, '3'},
	{ "notify",       1, تہی, LONGOPT_NOTIFY},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = تہی;

ساکن صحیح healthcheckdelay = 0;
ساکن صحیح healthchecktimer = 0;
ساکن متلون sig_atomic_t do_healthcheck = 0;
ساکن متلون sig_atomic_t exiting = 0;
ساکن صحیح nicelevel = INT_MIN;
ساکن صحیح ionicec = -1;
ساکن صحیح ioniced = 0;
ساکن صحیح oom_score_adj = INT_MIN;
ساکن حرف *changeuser, *ch_جڑ, *ch_dir;
ساکن uid_t uid = 0;
ساکن gid_t gid = 0;
ساکن bool stopgroup = false;
ساکن صحیح devnull_fd = -1;
ساکن صحیح stdin_fd;
ساکن صحیح stdout_fd;
ساکن صحیح stderr_fd;
ساکن ساخت notify notify;
ساکن حرف *redirect_stdin = تہی;
ساکن حرف *redirect_stderr = تہی;
ساکن حرف *redirect_stdout = تہی;
ساکن حرف *stderr_process = تہی;
ساکن حرف *stdout_process = تہی;
#اگر_تعریف_شدہ TIOCNOTTY
ساکن صحیح tty_fd = -1;
#اگر_ختم
ساکن pid_t بچہ_pid;
ساکن صحیح respawn_گنتی = 0;
ساکن int64_t respawn_delay; /* default set inside main() */
ساکن int64_t respawn_delay_قدم = TM_MS(128);
ساکن int64_t respawn_delay_cap = TM_SEC(30);
ساکن int64_t respawn_period; /* default set inside main() */
ساکن صحیح respawn_max = 10;
ساکن حرف *fifopath = تہی;
ساکن صحیح fifo_fd = 0;
ساکن حرف *pidfile = تہی;
ساکن حرف *svcname = تہی;
ساکن bool verbose = false;
#اگر_تعریف_شدہ __linux__
ساکن cap_iab_t cap_iab = تہی;
ساکن غیرعلامتی secbits = 0;
#اگر_ختم
#اگر_تعریف_شدہ PR_سیٹ_NO_NEW_PRIVS
ساکن bool no_new_privs = false;
#اگر_ختم

خارجی حرف **environ;

#اگر !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#اگر_ختم
#اگر !defined(__DragonFly__)
ساکن اندرونی صحیح ioprio_سیٹ(صحیح which RC_غیر_مستعمل, صحیح who RC_غیر_مستعمل,
			     صحیح ioprio RC_غیر_مستعمل)
{
#اگر_تعریف_شدہ SYS_ioprio_set
	واپس syscall(SYS_ioprio_set, which, who, ioprio);
#ورنہ
	واپس 0;
#اگر_ختم
}
#اگر_ختم

ساکن خالی cleanup(خالی)
{
	آزاد(changeuser);
}

RC_NORETURN ساکن خالی re_exec_supervisor(خالی)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(حرف *) تہی);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	باہر_نکلو(EXIT_FAILURE);
}

ساکن خالی handle_signal(صحیح sig)
{
	صحیح serrno = errno;
	pid_t pid;

	انتخاب (sig) {
	حالت SIGALRM:
		do_healthcheck = 1;
		توڑو;
	حالت SIGCHLD:
		اگر (exiting)
			جب (waitpid((pid_t)(-1), تہی, WNOHANG) > 0) {}
		ورنہ {
			جب ((pid = waitpid((pid_t)(-1), تہی, WNOHANG|WNOWAIT)) > 0) {
				اگر (pid == بچہ_pid)
					توڑو;
				pid = waitpid(pid, تہی, WNOHANG);
			}
		}
		توڑو;
	حالت SIGTERM:
		exiting = 1;
		توڑو;
	طےشدہ:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ساکن حرف * expand_home(ثابت حرف *home, ثابت حرف *راستہ)
{
	حرف *opath, *ppath, *p, *nh;
	ساخت passwd *pw;

	اگر (!راستہ || *راستہ != '~')
		واپس xstrdup(راستہ);

	opath = ppath = xstrdup(راستہ);
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
		} ورنہ
			home = تہی;
	} ورنہ
		ppath++;

	اگر (!home) {
		آزاد(opath);
		واپس xstrdup(راستہ);
	}
	اگر (!ppath) {
		آزاد(opath);
		واپس xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	آزاد(opath);
	واپس nh;
}

ساکن حرف *make_cmdline(حرف **argv)
{
	حرف **c;
	حرف *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	برائے (c = argv; c && *c; c++) {
		fputs(*c, mem);
		اگر (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	واپس cmdline;
}

ساکن pid_t exec_command(ثابت حرف *cmd)
{
	حرف *file;
	pid_t pid = -1;
	sigset_t بھرا;
	sigset_t old;
	ساخت sigaction sa;

	file = rc_service_resolve(svcname);
	اگر (access(file, F_OK) != 0) {
		آزاد(file);
		واپس 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&بھرا);
	sigprocmask(SIG_SETMASK, &بھرا, &old);

	pid = fork();
	اگر (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, تہی);
		sigaction(SIGHUP, &sa, تہی);
		sigaction(SIGINT, &sa, تہی);
		sigaction(SIGQUIT, &sa, تہی);
		sigaction(SIGTERM, &sa, تہی);
		sigaction(SIGUSR1, &sa, تہی);
		sigaction(SIGWINCH, &sa, تہی);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, تہی);

		/* Safe to run now */
		execl(file, file, cmd, (حرف *) تہی);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	اگر (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, تہی);
	آزاد(file);
	واپس pid;
}

RC_NORETURN ساکن خالی بچہ_process(حرف *exec, حرف **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	صحیح i;
	حرف *p;
	حرف *token;
	size_t len;
	حرف *newpath;
	حرف *np;
	حرف *cmdline = تہی;
	time_t آغاز_وقت;
	حرف آغاز_گنتی_string[20];
	حرف آغاز_وقت_string[20];
	FILE *fp;
	gid_t گروہ_buf[32], *گروہ_list = گروہ_buf;
	صحیح گروہ_گنتی = ARRAY_سائز(گروہ_buf);

#اگر_تعریف_شدہ HAVE_PAM
	pam_handle_t *pamh = تہی;
	صحیح pamr;
	ثابت حرف *ثابت *pamenv = تہی;
#اگر_ختم

	setsid();

	اگر (svcname) {
		آغاز_وقت = time(تہی);
		from_وقت_t(آغاز_وقت_string, آغاز_وقت);
		rc_service_قدر_سیٹ(svcname, "start_time", آغاز_وقت_string);
		sprintf(آغاز_گنتی_string, "%i", respawn_گنتی);
		rc_service_قدر_سیٹ(svcname, "start_count", آغاز_گنتی_string);
		sprintf(آغاز_گنتی_string, "%d", getpid());
		rc_service_قدر_سیٹ(svcname, "child_pid", آغاز_گنتی_string);
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
		} ورنہ اگر (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	اگر (ionicec != -1 && ioprio_سیٹ(1, getpid(), ionicec | ioniced) == -1)
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

#اگر_تعریف_شدہ HAVE_PAM
	اگر (changeuser != تہی) {
		pamr = pam_آغاز("supervise-daemon",
		    changeuser, &conv, &pamh);

		اگر (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		اگر (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		اگر (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#اگر_ختم

	اگر (changeuser && getgrouplist(changeuser, gid, گروہ_list, &گروہ_گنتی) < 0) {
		گروہ_list = xmalloc(گروہ_گنتی * حجم(*گروہ_list));
		اگر (getgrouplist(changeuser, gid, گروہ_list, &گروہ_گنتی) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	اگر (ch_جڑ && chroot(ch_جڑ) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_جڑ, strerror(errno));

	اگر (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	اگر (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	اگر (changeuser && setgroups(گروہ_گنتی, گروہ_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	اگر (گروہ_list != گروہ_buf)
		آزاد(گروہ_list);
#اگر_تعریف_شدہ __linux__
	اگر (uid && cap_setuid(uid))
#ورنہ
	اگر (uid && setuid(uid))
#اگر_ختم
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#اگر_تعریف_شدہ __linux__
	اگر (cap_iab != تہی) {
		i = cap_iab_سیٹ_proc(cap_iab);

		اگر (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		اگر (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	اگر (secbits != 0) {
		اگر (cap_سیٹ_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#اگر_ختم

#اگر_تعریف_شدہ PR_سیٹ_NO_NEW_PRIVS
	اگر (no_new_privs) {
		اگر (prctl(PR_سیٹ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", strerror(errno));
	}
#اگر_ختم

	/* remove the controlling tty */
#اگر_تعریف_شدہ TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#اگر_ختم

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	جب (environ[i])
		rc_stringlist_جوڑو(env_list, environ[i++]);

#اگر_تعریف_شدہ HAVE_PAM
	اگر (changeuser != تہی) {
		pamenv = (ثابت حرف *ثابت *)pam_getenvlist(pamh);
		اگر (pamenv) {
			جب (*pamenv) {
				/* Don't add strings unless they set a var */
				اگر (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				ورنہ
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#اگر_ختم

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		اگر (لڑی_موازنہ(env->قدر, "RC_", 3) == 0
				|| لڑی_موازنہ(env->قدر, "SSD_", 4) == 0
				|| لڑی_موازنہ(env->قدر, "EINFO_", 6) == 0) {
			*strchr(env->قدر, '=') = '\0';
			unsetenv(env->قدر);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	اگر ((token = getenv("PATH"))) {
		len = لڑی_لمبائی(token);
		newpath = np = xmalloc(len + 1);
		جب (token && *token) {
			p = strchr(token, ':');
			اگر (p) {
				*p++ = '\0';
				جب (*p == ':')
					p++;
			}
			اگر (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = لڑی_لمبائی(token);
				اگر (np != newpath)
					*np++ = ':';
				حافظہ_نقل(np, token, len);
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
	} ورنہ اگر (stdout_process) {
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
	} ورنہ اگر (stderr_process) {
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
		} ورنہ {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		اگر (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	آزاد(cmdline);
	execvp(exec, argv);

#اگر_تعریف_شدہ HAVE_PAM
	اگر (changeuser != تہی && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#اگر_ختم
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ساکن خالی supervisor(حرف *exec, حرف **argv)
{
	FILE *fp;
	حرف buf[2048];
	حرف cmd[2048];
	صحیح گنتی;
	صحیح failing;
	صحیح health_حالت;
	صحیح healthcheck_respawn;
	صحیح i;
	صحیح nkilled;
	صحیح sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_اشارات;
	sigset_t اشارات;
	ساخت sigaction sa;
	int64_t respawn_now = 0;
	int64_t پہلا_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&اشارات);
	sigdelset(&اشارات, SIGALRM);
	sigdelset(&اشارات, SIGCHLD);
	sigdelset(&اشارات, SIGTERM);
	sigprocmask(SIG_SETMASK, &اشارات, &old_اشارات);

	/* install signal  handler */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, تہی);
	sigaction(SIGCHLD, &sa, تہی);
	sigaction(SIGTERM, &sa, تہی);

	fp = fopen(pidfile, "w");
	اگر (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	اگر (svcname)
		rc_service_daemon_سیٹ(svcname, exec, (ثابت حرف * ثابت *) argv,
				pidfile, true);

	/* remove the controlling tty */
#اگر_تعریف_شدہ TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#اگر_ختم

	/*
	 * Supervisor main loop
	 */
	اگر (healthcheckdelay)
		alarm(healthcheckdelay);
	ورنہ اگر (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	جب (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		اگر (fifo_fd > 0) {
			memset(buf, 0, حجم(buf));
			گنتی = اقرأ(fifo_fd, buf, حجم(buf) - 1);
			close(fifo_fd);
			اگر (گنتی != -1)
				buf[گنتی] = 0;
			اگر (گنتی == 0)
				جاری;
			اگر (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			اگر (strncasecmp(buf, "sig", 3) == 0) {
				اگر ((لڑی_سے_پڑھو(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							بچہ_pid);
					اگر (kill(بچہ_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, بچہ_pid);
				}
			}
			جاری;
		}
		اگر (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			اگر (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_حالت = rc_waitpid(health_pid);
			اگر (WIFEXITED(health_حالت) && WEXITSTATUS(health_حالت) == 0)
				alarm(healthchecktimer);
			ورنہ {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, بچہ_pid);
				nkilled = تسلسل_stop_جدول_وقت(applet, تہی, تہی, بچہ_pid, 0,
						stopgroup, false, false, true);
				اگر (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							بچہ_pid, strerror(errno));
				ورنہ
					healthcheck_respawn = 1;
			}
		}
		اگر (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, بچہ_pid);
			nkilled = تسلسل_stop_جدول_وقت(applet, تہی, تہی, بچہ_pid, 0,
					stopgroup, false, false, true);
			اگر (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			جاری;
		}
		wait_pid = waitpid(بچہ_pid, &i, WNOHANG);
		اگر (wait_pid == بچہ_pid) {
			اگر (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, بچہ_pid, WEXITSTATUS(i));
			ورنہ اگر (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, بچہ_pid, WTERMSIG(i));
		}
		اگر (wait_pid == بچہ_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			اگر (پہلا_spawn == 0)
				پہلا_spawn = respawn_now;
			اگر ((respawn_period > 0)
					&& (respawn_now - پہلا_spawn > respawn_period)) {
				respawn_گنتی = 0;
				پہلا_spawn = 0;
			} ورنہ
				respawn_گنتی++;
			اگر (respawn_max > 0 && respawn_گنتی > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				جاری;
			}
			sleep_for = respawn_delay + (respawn_delay_قدم * respawn_گنتی);
			اگر (respawn_delay_قدم > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			اگر (exiting)
				جاری;
			بچہ_pid = fork();
			اگر (بچہ_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				باہر_نکلو(EXIT_FAILURE);
			}
			اگر (بچہ_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_اشارات, تہی);
				memset(&sa, 0, حجم(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, تہی);
				sigaction(SIGCHLD, &sa, تہی);
				sigaction(SIGTERM, &sa, تہی);
				بچہ_process(exec, argv);
			}
			اگر (healthcheckdelay)
				alarm(healthcheckdelay);
			ورنہ اگر (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	اگر (svcname) {
		rc_service_daemon_سیٹ(svcname, exec, (ثابت حرف *ثابت *)argv,
				pidfile, false);
		rc_service_قدر_سیٹ(svcname, "child_pid", تہی);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		اگر (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	اگر (pidfile)
		unlink(pidfile);
	اگر (fifopath)
		unlink(fifopath);
	باہر_نکلو(EXIT_SUCCESS);
}

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح opt;
	حرف **c;
	صحیح x;
	bool آغاز = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	حرف *exec = تہی;
	حرف *retry = تہی;
	صحیح sig = SIGTERM;
	حرف *home = تہی;
	صحیح tid = 0;
	pid_t pid;
	حرف *tmp;
	حرف *p;
	حرف *token;
	صحیح i;
	صحیح n;
	حرف *exec_file = تہی;
	حرف *varbuf = تہی;
	حرف numbuf[64];
	ساخت timespec ts;
	ساخت passwd *pw;
	ساخت group *gr;
	FILE *fp;
	mode_t numask = 022;
	صحیح بچہ_argc = 0;
	حرف **بچہ_argv = تہی;
	حرف *str = تہی;
	حرف *cmdline = تہی;
	ثابت حرف *respawn_delay_str  = "0";
	ثابت حرف *respawn_period_str = "12sec";

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
		اگر (لڑی_سے_پڑھو(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	اگر ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = لڑی_سے_پڑھو(tmp, "%d:%d", &ionicec, &ioniced);
		اگر (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		اگر (ionicec == 0)
			ioniced = 0;
		ورنہ اگر (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	اگر ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		اگر (لڑی_سے_پڑھو(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	اگر (home == تہی || p == تہی) {
		pw = getpwuid(getuid());
		اگر (pw != تہی) {
			اگر (p == تہی)
				setenv("USER", pw->pw_name, 1);
			اگر (home == تہی) {
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
	جب ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحیح *) 0)) != -1)
		انتخاب (opt) {
		حالت 'a':  /* --healthcheck-timer <time> */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			توڑو;

		حالت 'A':  /* --healthcheck-delay <time> */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			توڑو;

		حالت LONGOPT_CAPABILITIES:
#اگر_تعریف_شدہ __linux__
			cap_iab = cap_iab_from_متن(optarg);
			اگر (cap_iab == تہی)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ورنہ
			eerrorx("Capabilities support not enabled");
#اگر_ختم
			توڑو;

		حالت LONGOPT_SECBITS:
#اگر_تعریف_شدہ __linux__
			اگر (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = تہی;
			secbits = strtoul(optarg, &tmp, 0);
			اگر (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ورنہ
			eerrorx("Capabilities support not enabled");
#اگر_ختم
			توڑو;

		حالت LONGOPT_NO_NEW_PRIVS:
#اگر_تعریف_شدہ PR_سیٹ_NO_NEW_PRIVS
			no_new_privs = true;
#ورنہ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#اگر_ختم
			توڑو;

		حالت 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			توڑو;

		حالت 'I': /* --ionice */
			اگر (لڑی_سے_پڑھو(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			اگر (ionicec == 0)
				ioniced = 0;
			ورنہ اگر (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			توڑو;

		حالت 'K':  /* --stop */
			stop = true;
			توڑو;

		حالت 'N':  /* --nice */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			توڑو;

		حالت LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			توڑو;

		حالت 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			توڑو;

		حالت 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			توڑو;
		حالت 'S':  /* --start */
			آغاز = true;
			توڑو;

		حالت 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			توڑو;

		حالت 'e': /* --env */
			putenv(optarg);
			توڑو;

		حالت 'g':  /* --group <group>|<gid> */
			اگر (لڑی_سے_پڑھو(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ورنہ
				gr = getgrgid((gid_t)tid);
			اگر (gr == تہی)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			توڑو;

		حالت 'G': /* -G */
			stopgroup = true;
			توڑو;

		حالت 'k':
			اگر (parse_منوال(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			توڑو;

		حالت 'm':  /* --respawn-max count */
			n = لڑی_سے_پڑھو(optarg, "%d", &respawn_max);
			اگر (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			توڑو;

		حالت 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			توڑو;

		حالت 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			توڑو;
		حالت 'r':  /* --chroot /new/root */
			ch_جڑ = optarg;
			توڑو;

		حالت 'u':  /* --user <username>|<uid> */
		{
			حرف dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			اگر (لڑی_سے_پڑھو(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ورنہ
				pw = getpwuid((uid_t)tid);

			اگر (pw == تہی)
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
				اگر (لڑی_سے_پڑھو(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ورنہ
					gr = getgrgid((gid_t) tid);

				اگر (gr == تہی)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		توڑو;

		حالت '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			توڑو;

		حالت '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			توڑو;

		حالت '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			توڑو;
		حالت '3':  /* --reexec */
			reexec = true;
			توڑو;

		حالت LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			توڑو;

		حالت LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			توڑو;

		حالت LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			توڑو;

		حالت LONGOPT_RESPAWN_DELAY_قدم:
			respawn_delay_قدم = parse_duration(optarg);
			اگر (respawn_delay_قدم < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			توڑو;

		حالت LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			اگر (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			توڑو;

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
	اگر (ch_جڑ && *ch_جڑ == '~')
		ch_جڑ = expand_home(home, ch_جڑ);

	umask(numask);
	اگر (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	اگر (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	اگر (reexec) {
		str = rc_service_قدر_get(svcname, "argc");
		لڑی_سے_پڑھو(str, "%d", &بچہ_argc);
		بچہ_argv = xmalloc((بچہ_argc + 1) * حجم(حرف *));
		memset(بچہ_argv, 0, (بچہ_argc + 1) * حجم(حرف *));
		برائے (x = 0; x < بچہ_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_قدر_get(svcname, varbuf);
			بچہ_argv[x] = str;
			آزاد(varbuf);
			varbuf = تہی;
		}
		آزاد(str);
		str = rc_service_قدر_get(svcname, "child_pid");
		لڑی_سے_پڑھو(str, "%d", &بچہ_pid);
		آزاد(str);
		exec = rc_service_قدر_get(svcname, "exec");
		pidfile = rc_service_قدر_get(svcname, "pidfile");
		retry = rc_service_قدر_get(svcname, "retry");
		اگر (retry) {
			parse_جدول_وقت(applet, retry, sig);
			rc_service_قدر_سیٹ(svcname, "retry", retry);
		} ورنہ
			parse_جدول_وقت(applet, تہی, sig);

		str = rc_service_قدر_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_قدر_get(svcname, "respawn_delay_step");
		لڑی_سے_پڑھو(str, "%"SCNd64, &respawn_delay_قدم);
		str = rc_service_قدر_get(svcname, "respawn_delay_cap");
		لڑی_سے_پڑھو(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_قدر_get(svcname, "respawn_max");
		لڑی_سے_پڑھو(str, "%d", &respawn_max);
		supervisor(exec, بچہ_argv);
	} ورنہ اگر (آغاز) {
		اگر (exec) {
			اگر (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			اگر (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				اگر (ch_جڑ)
					xasprintf(&exec_file, "%s/%s", ch_جڑ, exec);
				ورنہ
					xasprintf(&exec_file, "%s", exec);
			} ورنہ {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = تہی;
				جب ((token = strsep(&p, ":"))) {
					اگر (ch_جڑ)
						xasprintf(&exec_file, "%s/%s/%s", ch_جڑ, token, exec);
					ورنہ
						xasprintf(&exec_file, "%s/%s", token, exec);
					اگر (exec_file && access(exec_file, F_OK) == 0)
						توڑو;
					آزاد(exec_file);
					exec_file = تہی;
				}
				آزاد(tmp);
			}
			اگر (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				آزاد(exec_file);
				باہر_نکلو(EXIT_FAILURE);
			}
		} ورنہ
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		اگر (pid != -1)
			اگر (do_stop(applet, exec, (ثابت حرف * ثابت *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		اگر (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		اگر (respawn_delay_قدم > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		اگر (retry) {
			parse_جدول_وقت(applet, retry, sig);
			rc_service_قدر_سیٹ(svcname, "retry", retry);
		} ورنہ
			parse_جدول_وقت(applet, تہی, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		آزاد(cmdline);
		cmdline = تہی;

		/* Remove existing pidfile */
		اگر (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		اگر (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_قدر_سیٹ(svcname, "pidfile", pidfile);
		rc_service_قدر_سیٹ(svcname, "respawn_delay", respawn_delay_str);
		rc_service_قدر_سیٹ(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, حجم(numbuf), "%"PRId64, respawn_delay_قدم);
		rc_service_قدر_سیٹ(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, حجم(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_قدر_سیٹ(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, حجم(numbuf), "%i", respawn_max);
		rc_service_قدر_سیٹ(svcname, "respawn_max", numbuf);
		بچہ_pid = fork();
		اگر (بچہ_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		اگر (بچہ_pid != 0)
			باہر_نکلو(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#اگر_تعریف_شدہ TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#اگر_ختم
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		اگر (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		بچہ_pid = fork();
		اگر (بچہ_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ورنہ اگر (بچہ_pid != 0) {
			اگر (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			جب (c && *c) {
				varbuf = تہی;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_قدر_سیٹ(svcname, varbuf, *c);
				آزاد(varbuf);
				varbuf = تہی;
				x++;
				c++;
			}
			snprintf(numbuf, حجم(numbuf), "%d", x);
			rc_service_قدر_سیٹ(svcname, "argc", numbuf);
			rc_service_قدر_سیٹ(svcname, "exec", exec);
			supervisor(exec, argv);
		} ورنہ
			بچہ_process(exec, argv);
	} ورنہ اگر (stop) {
		pid = get_pid(applet, pidfile);
		اگر (pid != -1) {
			i = kill(pid, SIGTERM);
			اگر (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			ورنہ {
				/* wait for the supervisor to go down */
				جب (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, تہی);
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
			rc_service_daemon_سیٹ(svcname, exec,
			    (ثابت حرف *ثابت *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		باہر_نکلو(EXIT_SUCCESS);
	} ورنہ اگر (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		اگر (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = اكتب(fifo_fd, str, لڑی_لمبائی(str));
		اگر (x == -1) {
			آزاد(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		آزاد(tmp);
		باہر_نکلو(EXIT_SUCCESS);
	}
}
