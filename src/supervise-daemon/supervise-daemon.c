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

#ضمن <errno.h>
#ضمن <inttypes.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>
#ضمن <limits.h>
#ضمن <grp.h>
#ضمن <pwd.h>
#ضمن <signal.h>
#ضمن <stddef.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <strings.h>
#که_تعریف_شوی __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#د_که_پای
#ضمن <syslog.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/resource.h>
#ضمن <sys/stat.h>
#ضمن <sys/wait.h>
#ضمن <time.h>
#ضمن <unistd.h>

#که_تعریف_شوی HAVE_PAM
#ضمن <security/pam_appl.h>

/* We are not supporting authentication conversations */
ساكن بنية pam_conv conv = { عدم, عدم};
#د_که_پای

#که_تعریف_شوی __linux__
#ضمن <sys/capability.h>
#د_که_پای

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "rc_exec.h"
#ضمن "plugin.h"
#ضمن "schedules.h"
#ضمن "timeutils.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
تعداد {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_بنسټ = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_ګام,
  LONGOPT_RESPAWN_DELAY_CAP,
};

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "healthcheck-timer",        1, عدم, 'a'},
	{ "healthcheck-delay",        1, عدم, 'A'},
	{ "capabilities", 1, عدم, LONGOPT_CAPABILITIES},
	{ "secbits",      1, عدم, LONGOPT_SECBITS},
	{ "no-new-privs", 0, عدم, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, عدم, 'D'},
	{ "respawn-delay-step",   1, عدم, LONGOPT_RESPAWN_DELAY_ګام},
	{ "respawn-delay-cap",    1, عدم, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, عدم, 'd'},
	{ "env",          1, عدم, 'e'},
	{ "group",        1, عدم, 'g'},
	{ "stop-group",   0, عدم, 'G'},
	{ "ionice",       1, عدم, 'I'},
	{ "stop",         0, عدم, 'K'},
	{ "umask",        1, عدم, 'k'},
	{ "respawn-max",    1, عدم, 'm'},
	{ "nicelevel",    1, عدم, 'N'},
	{ "oom-score-adj",1, عدم, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, عدم, 'p'},
	{ "respawn-period",        1, عدم, 'P'},
	{ "retry",       1, عدم, 'R'},
	{ "chroot",       1, عدم, 'r'},
	{ "signal",       1, عدم, 's'},
	{ "start",        0, عدم, 'S'},
	{ "user",         1, عدم, 'u'},
	{ "stdin",        1, عدم, '0'},
	{ "stdout",       1, عدم, '1'},
	{ "stderr",       1, عدم, '2'},
	{ "stdout-logger",1, عدم, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, عدم, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, عدم, '3'},
	{ "notify",       1, عدم, LONGOPT_NOTIFY},
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
ثابت حرف *usagestring = عدم;

ساكن صحيح healthcheckdelay = 0;
ساكن صحيح healthchecktimer = 0;
ساكن متطاير sig_atomic_t do_healthcheck = 0;
ساكن متطاير sig_atomic_t exiting = 0;
ساكن صحيح nicelevel = INT_MIN;
ساكن صحيح ionicec = -1;
ساكن صحيح ioniced = 0;
ساكن صحيح oom_score_adj = INT_MIN;
ساكن حرف *changeuser, *ch_ریښه, *ch_dir;
ساكن uid_t uid = 0;
ساكن gid_t gid = 0;
ساكن bool stopgroup = false;
ساكن صحيح devnull_fd = -1;
ساكن صحيح stdin_fd;
ساكن صحيح stdout_fd;
ساكن صحيح stderr_fd;
ساكن بنية notify notify;
ساكن حرف *redirect_stdin = عدم;
ساكن حرف *redirect_stderr = عدم;
ساكن حرف *redirect_stdout = عدم;
ساكن حرف *stderr_process = عدم;
ساكن حرف *stdout_process = عدم;
#که_تعریف_شوی TIOCNOTTY
ساكن صحيح tty_fd = -1;
#د_که_پای
ساكن pid_t ماشوم_pid;
ساكن صحيح respawn_شمېر = 0;
ساكن int64_t respawn_delay; /* default set inside main() */
ساكن int64_t respawn_delay_ګام = TM_MS(128);
ساكن int64_t respawn_delay_cap = TM_SEC(30);
ساكن int64_t respawn_period; /* default set inside main() */
ساكن صحيح respawn_max = 10;
ساكن حرف *fifopath = عدم;
ساكن صحيح fifo_fd = 0;
ساكن حرف *pidfile = عدم;
ساكن حرف *svcname = عدم;
ساكن bool verbose = false;
#که_تعریف_شوی __linux__
ساكن cap_iab_t cap_iab = عدم;
ساكن بینښه secbits = 0;
#د_که_پای
#که_تعریف_شوی PR_سېټ_NO_NEW_PRIVS
ساكن bool no_new_privs = false;
#د_که_پای

خارجي حرف **environ;

#إذا !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#د_که_پای
#إذا !defined(__DragonFly__)
ساكن مضمن صحيح ioprio_سېټ(صحيح which RC_ناکارول_شوی, صحيح who RC_ناکارول_شوی,
			     صحيح ioprio RC_ناکارول_شوی)
{
#که_تعریف_شوی SYS_ioprio_set
	أرجع syscall(SYS_ioprio_set, which, who, ioprio);
#وإلا
	أرجع 0;
#د_که_پای
}
#د_که_پای

ساكن فراغ cleanup(فراغ)
{
	حرر(changeuser);
}

RC_NORETURN ساكن فراغ re_exec_supervisor(فراغ)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(حرف *) عدم);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	وتل(EXIT_FAILURE);
}

ساكن فراغ handle_signal(صحيح sig)
{
	صحيح serrno = errno;
	pid_t pid;

	بدل (sig) {
	حالة SIGALRM:
		do_healthcheck = 1;
		اكسر;
	حالة SIGCHLD:
		إذا (exiting)
			طالما (waitpid((pid_t)(-1), عدم, WNOHANG) > 0) {}
		وإلا {
			طالما ((pid = waitpid((pid_t)(-1), عدم, WNOHANG|WNOWAIT)) > 0) {
				إذا (pid == ماشوم_pid)
					اكسر;
				pid = waitpid(pid, عدم, WNOHANG);
			}
		}
		اكسر;
	حالة SIGTERM:
		exiting = 1;
		اكسر;
	افتراضي:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ساكن حرف * expand_home(ثابت حرف *home, ثابت حرف *لار)
{
	حرف *opath, *ppath, *p, *nh;
	بنية passwd *pw;

	إذا (!لار || *لار != '~')
		أرجع xstrdup(لار);

	opath = ppath = xstrdup(لار);
	إذا (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		إذا (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		إذا (pw) {
			home = pw->pw_dir;
			ppath = p;
			إذا (ppath)
				*ppath = '/';
		} وإلا
			home = عدم;
	} وإلا
		ppath++;

	إذا (!home) {
		حرر(opath);
		أرجع xstrdup(لار);
	}
	إذا (!ppath) {
		حرر(opath);
		أرجع xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	حرر(opath);
	أرجع nh;
}

ساكن حرف *make_cmdline(حرف **argv)
{
	حرف **c;
	حرف *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	لكل (c = argv; c && *c; c++) {
		fputs(*c, mem);
		إذا (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	أرجع cmdline;
}

ساكن pid_t exec_command(ثابت حرف *cmd)
{
	حرف *file;
	pid_t pid = -1;
	sigset_t ډک;
	sigset_t old;
	بنية sigaction sa;

	file = rc_service_resolve(svcname);
	إذا (access(file, F_OK) != 0) {
		حرر(file);
		أرجع 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&ډک);
	sigprocmask(SIG_SETMASK, &ډک, &old);

	pid = fork();
	إذا (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, عدم);
		sigaction(SIGHUP, &sa, عدم);
		sigaction(SIGINT, &sa, عدم);
		sigaction(SIGQUIT, &sa, عدم);
		sigaction(SIGTERM, &sa, عدم);
		sigaction(SIGUSR1, &sa, عدم);
		sigaction(SIGWINCH, &sa, عدم);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, عدم);

		/* Safe to run now */
		execl(file, file, cmd, (حرف *) عدم);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	إذا (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, عدم);
	حرر(file);
	أرجع pid;
}

RC_NORETURN ساكن فراغ ماشوم_process(حرف *exec, حرف **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	صحيح i;
	حرف *p;
	حرف *token;
	size_t len;
	حرف *newpath;
	حرف *np;
	حرف *cmdline = عدم;
	time_t پیل_وخت;
	حرف پیل_شمېر_string[20];
	حرف پیل_وخت_string[20];
	FILE *fp;
	gid_t ډله_buf[32], *ډله_list = ډله_buf;
	صحيح ډله_شمېر = ARRAY_اندازه(ډله_buf);

#که_تعریف_شوی HAVE_PAM
	pam_handle_t *pamh = عدم;
	صحيح pamr;
	ثابت حرف *ثابت *pamenv = عدم;
#د_که_پای

	setsid();

	إذا (svcname) {
		پیل_وخت = time(عدم);
		from_وخت_t(پیل_وخت_string, پیل_وخت);
		rc_service_ارزښت_سېټ(svcname, "start_time", پیل_وخت_string);
		sprintf(پیل_شمېر_string, "%i", respawn_شمېر);
		rc_service_ارزښت_سېټ(svcname, "start_count", پیل_شمېر_string);
		sprintf(پیل_شمېر_string, "%d", getpid());
		rc_service_ارزښت_سېټ(svcname, "child_pid", پیل_شمېر_string);
	}

	إذا (nicelevel != INT_MIN) {
		إذا (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		إذا (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} وإلا إذا (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	إذا (ionicec != -1 && ioprio_سېټ(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	إذا (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		إذا (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#که_تعریف_شوی HAVE_PAM
	إذا (changeuser != عدم) {
		pamr = pam_پیل("supervise-daemon",
		    changeuser, &conv, &pamh);

		إذا (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		إذا (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		إذا (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#د_که_پای

	إذا (changeuser && getgrouplist(changeuser, gid, ډله_list, &ډله_شمېر) < 0) {
		ډله_list = xmalloc(ډله_شمېر * حجم(*ډله_list));
		إذا (getgrouplist(changeuser, gid, ډله_list, &ډله_شمېر) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	إذا (ch_ریښه && chroot(ch_ریښه) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_ریښه, strerror(errno));

	إذا (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	إذا (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	إذا (changeuser && setgroups(ډله_شمېر, ډله_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	إذا (ډله_list != ډله_buf)
		حرر(ډله_list);
#که_تعریف_شوی __linux__
	إذا (uid && cap_setuid(uid))
#وإلا
	إذا (uid && setuid(uid))
#د_که_پای
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#که_تعریف_شوی __linux__
	إذا (cap_iab != عدم) {
		i = cap_iab_سېټ_proc(cap_iab);

		إذا (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		إذا (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	إذا (secbits != 0) {
		إذا (cap_سېټ_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#د_که_پای

#که_تعریف_شوی PR_سېټ_NO_NEW_PRIVS
	إذا (no_new_privs) {
		إذا (prctl(PR_سېټ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", strerror(errno));
	}
#د_که_پای

	/* remove the controlling tty */
#که_تعریف_شوی TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#د_که_پای

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	طالما (environ[i])
		rc_stringlist_جمع_کړه(env_list, environ[i++]);

#که_تعریف_شوی HAVE_PAM
	إذا (changeuser != عدم) {
		pamenv = (ثابت حرف *ثابت *)pam_getenvlist(pamh);
		إذا (pamenv) {
			طالما (*pamenv) {
				/* Don't add strings unless they set a var */
				إذا (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				وإلا
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#د_که_پای

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		إذا (مزي_پرتله(env->ارزښت, "RC_", 3) == 0
				|| مزي_پرتله(env->ارزښت, "SSD_", 4) == 0
				|| مزي_پرتله(env->ارزښت, "EINFO_", 6) == 0) {
			*strchr(env->ارزښت, '=') = '\0';
			unsetenv(env->ارزښت);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	إذا ((token = getenv("PATH"))) {
		len = مزي_اوږدوالی(token);
		newpath = np = xmalloc(len + 1);
		طالما (token && *token) {
			p = strchr(token, ':');
			إذا (p) {
				*p++ = '\0';
				طالما (*p == ':')
					p++;
			}
			إذا (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = مزي_اوږدوالی(token);
				إذا (np != newpath)
					*np++ = ':';
				حافظه_نقل(np, token, len);
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
	إذا (redirect_stdin) {
		إذا ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	إذا (redirect_stdout) {
		إذا ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} وإلا إذا (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		إذا (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	إذا (redirect_stderr) {
		إذا ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} وإلا إذا (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		إذا (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	إذا (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	إذا (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	إذا (notify.type == NOTIFY_FD) {
		صحيح error;

		إذا (notify.fd == notify.pipe[1]) {
			صحيح flags = error = fcntl(notify.fd, F_GETFD, 0);
			إذا (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} وإلا {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		إذا (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	حرر(cmdline);
	execvp(exec, argv);

#که_تعریف_شوی HAVE_PAM
	إذا (changeuser != عدم && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#د_که_پای
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ساكن فراغ supervisor(حرف *exec, حرف **argv)
{
	FILE *fp;
	حرف buf[2048];
	حرف cmd[2048];
	صحيح شمېر;
	صحيح failing;
	صحيح health_حالت;
	صحيح healthcheck_respawn;
	صحيح i;
	صحيح nkilled;
	صحيح sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_سیګنالونه;
	sigset_t سیګنالونه;
	بنية sigaction sa;
	int64_t respawn_now = 0;
	int64_t لومړی_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&سیګنالونه);
	sigdelset(&سیګنالونه, SIGALRM);
	sigdelset(&سیګنالونه, SIGCHLD);
	sigdelset(&سیګنالونه, SIGTERM);
	sigprocmask(SIG_SETMASK, &سیګنالونه, &old_سیګنالونه);

	/* install signal  handler */
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, عدم);
	sigaction(SIGCHLD, &sa, عدم);
	sigaction(SIGTERM, &sa, عدم);

	fp = fopen(pidfile, "w");
	إذا (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	إذا (svcname)
		rc_service_daemon_سېټ(svcname, exec, (ثابت حرف * ثابت *) argv,
				pidfile, true);

	/* remove the controlling tty */
#که_تعریف_شوی TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#د_که_پای

	/*
	 * Supervisor main loop
	 */
	إذا (healthcheckdelay)
		alarm(healthcheckdelay);
	وإلا إذا (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	طالما (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		إذا (fifo_fd > 0) {
			memset(buf, 0, حجم(buf));
			شمېر = اقرأ(fifo_fd, buf, حجم(buf) - 1);
			close(fifo_fd);
			إذا (شمېر != -1)
				buf[شمېر] = 0;
			إذا (شمېر == 0)
				استمر;
			إذا (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			إذا (strncasecmp(buf, "sig", 3) == 0) {
				إذا ((له_مزي_ولوله(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							ماشوم_pid);
					إذا (kill(ماشوم_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, ماشوم_pid);
				}
			}
			استمر;
		}
		إذا (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			إذا (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_حالت = rc_waitpid(health_pid);
			إذا (WIFEXITED(health_حالت) && WEXITSTATUS(health_حالت) == 0)
				alarm(healthchecktimer);
			وإلا {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, ماشوم_pid);
				nkilled = پرلپسې_stop_مهالوېش(applet, عدم, عدم, ماشوم_pid, 0,
						stopgroup, false, false, true);
				إذا (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							ماشوم_pid, strerror(errno));
				وإلا
					healthcheck_respawn = 1;
			}
		}
		إذا (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, ماشوم_pid);
			nkilled = پرلپسې_stop_مهالوېش(applet, عدم, عدم, ماشوم_pid, 0,
					stopgroup, false, false, true);
			إذا (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			استمر;
		}
		wait_pid = waitpid(ماشوم_pid, &i, WNOHANG);
		إذا (wait_pid == ماشوم_pid) {
			إذا (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, ماشوم_pid, WEXITSTATUS(i));
			وإلا إذا (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, ماشوم_pid, WTERMSIG(i));
		}
		إذا (wait_pid == ماشوم_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			إذا (لومړی_spawn == 0)
				لومړی_spawn = respawn_now;
			إذا ((respawn_period > 0)
					&& (respawn_now - لومړی_spawn > respawn_period)) {
				respawn_شمېر = 0;
				لومړی_spawn = 0;
			} وإلا
				respawn_شمېر++;
			إذا (respawn_max > 0 && respawn_شمېر > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				استمر;
			}
			sleep_for = respawn_delay + (respawn_delay_ګام * respawn_شمېر);
			إذا (respawn_delay_ګام > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			إذا (exiting)
				استمر;
			ماشوم_pid = fork();
			إذا (ماشوم_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				وتل(EXIT_FAILURE);
			}
			إذا (ماشوم_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_سیګنالونه, عدم);
				memset(&sa, 0, حجم(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, عدم);
				sigaction(SIGCHLD, &sa, عدم);
				sigaction(SIGTERM, &sa, عدم);
				ماشوم_process(exec, argv);
			}
			إذا (healthcheckdelay)
				alarm(healthcheckdelay);
			وإلا إذا (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	إذا (svcname) {
		rc_service_daemon_سېټ(svcname, exec, (ثابت حرف *ثابت *)argv,
				pidfile, false);
		rc_service_ارزښت_سېټ(svcname, "child_pid", عدم);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		إذا (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	إذا (pidfile)
		unlink(pidfile);
	إذا (fifopath)
		unlink(fifopath);
	وتل(EXIT_SUCCESS);
}

صحيح اصلي(صحيح argc, حرف **argv)
{
	صحيح opt;
	حرف **c;
	صحيح x;
	bool پیل = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	حرف *exec = عدم;
	حرف *retry = عدم;
	صحيح sig = SIGTERM;
	حرف *home = عدم;
	صحيح tid = 0;
	pid_t pid;
	حرف *tmp;
	حرف *p;
	حرف *token;
	صحيح i;
	صحيح n;
	حرف *exec_file = عدم;
	حرف *varbuf = عدم;
	حرف numbuf[64];
	بنية timespec ts;
	بنية passwd *pw;
	بنية group *gr;
	FILE *fp;
	mode_t numask = 022;
	صحيح ماشوم_argc = 0;
	حرف **ماشوم_argv = عدم;
	حرف *str = عدم;
	حرف *cmdline = عدم;
	ثابت حرف *respawn_delay_str  = "0";
	ثابت حرف *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	إذا (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	إذا (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	إذا ((tmp = getenv("SSD_NICELEVEL")))
		إذا (له_مزي_ولوله(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	إذا ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = له_مزي_ولوله(tmp, "%d:%d", &ionicec, &ioniced);
		إذا (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		إذا (ionicec == 0)
			ioniced = 0;
		وإلا إذا (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	إذا ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		إذا (له_مزي_ولوله(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	إذا (home == عدم || p == عدم) {
		pw = getpwuid(getuid());
		إذا (pw != عدم) {
			إذا (p == عدم)
				setenv("USER", pw->pw_name, 1);
			إذا (home == عدم) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	إذا (svcname) {
		argc--;
		argv++;
	}
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة 'a':  /* --healthcheck-timer <time> */
			إذا (له_مزي_ولوله(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			اكسر;

		حالة 'A':  /* --healthcheck-delay <time> */
			إذا (له_مزي_ولوله(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			اكسر;

		حالة LONGOPT_CAPABILITIES:
#که_تعریف_شوی __linux__
			cap_iab = cap_iab_from_متن(optarg);
			إذا (cap_iab == عدم)
				eerrorx("Could not parse iab: %s", strerror(errno));
#وإلا
			eerrorx("Capabilities support not enabled");
#د_که_پای
			اكسر;

		حالة LONGOPT_SECBITS:
#که_تعریف_شوی __linux__
			إذا (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = عدم;
			secbits = strtoul(optarg, &tmp, 0);
			إذا (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#وإلا
			eerrorx("Capabilities support not enabled");
#د_که_پای
			اكسر;

		حالة LONGOPT_NO_NEW_PRIVS:
#که_تعریف_شوی PR_سېټ_NO_NEW_PRIVS
			no_new_privs = true;
#وإلا
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#د_که_پای
			اكسر;

		حالة 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			اكسر;

		حالة 'I': /* --ionice */
			إذا (له_مزي_ولوله(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			إذا (ionicec == 0)
				ioniced = 0;
			وإلا إذا (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			اكسر;

		حالة 'K':  /* --stop */
			stop = true;
			اكسر;

		حالة 'N':  /* --nice */
			إذا (له_مزي_ولوله(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			اكسر;

		حالة LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			إذا (له_مزي_ولوله(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			اكسر;

		حالة 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			اكسر;

		حالة 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			اكسر;
		حالة 'S':  /* --start */
			پیل = true;
			اكسر;

		حالة 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			اكسر;

		حالة 'e': /* --env */
			putenv(optarg);
			اكسر;

		حالة 'g':  /* --group <group>|<gid> */
			إذا (له_مزي_ولوله(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			وإلا
				gr = getgrgid((gid_t)tid);
			إذا (gr == عدم)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			اكسر;

		حالة 'G': /* -G */
			stopgroup = true;
			اكسر;

		حالة 'k':
			إذا (parse_ډېرپېښ(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			اكسر;

		حالة 'm':  /* --respawn-max count */
			n = له_مزي_ولوله(optarg, "%d", &respawn_max);
			إذا (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			اكسر;

		حالة 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			اكسر;

		حالة 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			اكسر;
		حالة 'r':  /* --chroot /new/root */
			ch_ریښه = optarg;
			اكسر;

		حالة 'u':  /* --user <username>|<uid> */
		{
			حرف dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			إذا (له_مزي_ولوله(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			وإلا
				pw = getpwuid((uid_t)tid);

			إذا (pw == عدم)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			إذا (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			إذا (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			إذا (gid == 0)
				gid = pw->pw_gid;

			إذا (p) {
				tmp = strsep (&p, ":");
				إذا (له_مزي_ولوله(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				وإلا
					gr = getgrgid((gid_t) tid);

				إذا (gr == عدم)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		اكسر;

		حالة '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			اكسر;

		حالة '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			اكسر;

		حالة '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			اكسر;
		حالة '3':  /* --reexec */
			reexec = true;
			اكسر;

		حالة LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			اكسر;

		حالة LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			اكسر;

		حالة LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			اكسر;

		حالة LONGOPT_RESPAWN_DELAY_ګام:
			respawn_delay_ګام = parse_duration(optarg);
			إذا (respawn_delay_ګام < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			اكسر;

		حالة LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			إذا (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			اكسر;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	إذا (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	إذا (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	إذا (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	إذا (ch_ریښه && *ch_ریښه == '~')
		ch_ریښه = expand_home(home, ch_ریښه);

	umask(numask);
	إذا (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	إذا (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	إذا (reexec) {
		str = rc_service_ارزښت_get(svcname, "argc");
		له_مزي_ولوله(str, "%d", &ماشوم_argc);
		ماشوم_argv = xmalloc((ماشوم_argc + 1) * حجم(حرف *));
		memset(ماشوم_argv, 0, (ماشوم_argc + 1) * حجم(حرف *));
		لكل (x = 0; x < ماشوم_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_ارزښت_get(svcname, varbuf);
			ماشوم_argv[x] = str;
			حرر(varbuf);
			varbuf = عدم;
		}
		حرر(str);
		str = rc_service_ارزښت_get(svcname, "child_pid");
		له_مزي_ولوله(str, "%d", &ماشوم_pid);
		حرر(str);
		exec = rc_service_ارزښت_get(svcname, "exec");
		pidfile = rc_service_ارزښت_get(svcname, "pidfile");
		retry = rc_service_ارزښت_get(svcname, "retry");
		إذا (retry) {
			parse_مهالوېش(applet, retry, sig);
			rc_service_ارزښت_سېټ(svcname, "retry", retry);
		} وإلا
			parse_مهالوېش(applet, عدم, sig);

		str = rc_service_ارزښت_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_ارزښت_get(svcname, "respawn_delay_step");
		له_مزي_ولوله(str, "%"SCNd64, &respawn_delay_ګام);
		str = rc_service_ارزښت_get(svcname, "respawn_delay_cap");
		له_مزي_ولوله(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_ارزښت_get(svcname, "respawn_max");
		له_مزي_ولوله(str, "%d", &respawn_max);
		supervisor(exec, ماشوم_argv);
	} وإلا إذا (پیل) {
		إذا (exec) {
			إذا (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			إذا (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				إذا (ch_ریښه)
					xasprintf(&exec_file, "%s/%s", ch_ریښه, exec);
				وإلا
					xasprintf(&exec_file, "%s", exec);
			} وإلا {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = عدم;
				طالما ((token = strsep(&p, ":"))) {
					إذا (ch_ریښه)
						xasprintf(&exec_file, "%s/%s/%s", ch_ریښه, token, exec);
					وإلا
						xasprintf(&exec_file, "%s/%s", token, exec);
					إذا (exec_file && access(exec_file, F_OK) == 0)
						اكسر;
					حرر(exec_file);
					exec_file = عدم;
				}
				حرر(tmp);
			}
			إذا (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				حرر(exec_file);
				وتل(EXIT_FAILURE);
			}
		} وإلا
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		إذا (pid != -1)
			إذا (do_stop(applet, exec, (ثابت حرف * ثابت *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		إذا (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		إذا (respawn_delay_ګام > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		إذا (retry) {
			parse_مهالوېش(applet, retry, sig);
			rc_service_ارزښت_سېټ(svcname, "retry", retry);
		} وإلا
			parse_مهالوېش(applet, عدم, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		حرر(cmdline);
		cmdline = عدم;

		/* Remove existing pidfile */
		إذا (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		إذا (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_ارزښت_سېټ(svcname, "pidfile", pidfile);
		rc_service_ارزښت_سېټ(svcname, "respawn_delay", respawn_delay_str);
		rc_service_ارزښت_سېټ(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, حجم(numbuf), "%"PRId64, respawn_delay_ګام);
		rc_service_ارزښت_سېټ(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, حجم(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_ارزښت_سېټ(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, حجم(numbuf), "%i", respawn_max);
		rc_service_ارزښت_سېټ(svcname, "respawn_max", numbuf);
		ماشوم_pid = fork();
		إذا (ماشوم_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		إذا (ماشوم_pid != 0)
			وتل(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#که_تعریف_شوی TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#د_که_پای
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		إذا (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		ماشوم_pid = fork();
		إذا (ماشوم_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		وإلا إذا (ماشوم_pid != 0) {
			إذا (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			طالما (c && *c) {
				varbuf = عدم;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_ارزښت_سېټ(svcname, varbuf, *c);
				حرر(varbuf);
				varbuf = عدم;
				x++;
				c++;
			}
			snprintf(numbuf, حجم(numbuf), "%d", x);
			rc_service_ارزښت_سېټ(svcname, "argc", numbuf);
			rc_service_ارزښت_سېټ(svcname, "exec", exec);
			supervisor(exec, argv);
		} وإلا
			ماشوم_process(exec, argv);
	} وإلا إذا (stop) {
		pid = get_pid(applet, pidfile);
		إذا (pid != -1) {
			i = kill(pid, SIGTERM);
			إذا (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			وإلا {
				/* wait for the supervisor to go down */
				طالما (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, عدم);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		إذا (pidfile)
			unlink(pidfile);
		إذا (svcname) {
			rc_service_daemon_سېټ(svcname, exec,
			    (ثابت حرف *ثابت *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		وتل(EXIT_SUCCESS);
	} وإلا إذا (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		إذا (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = اكتب(fifo_fd, str, مزي_اوږدوالی(str));
		إذا (x == -1) {
			حرر(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		حرر(tmp);
		وتل(EXIT_SUCCESS);
	}
}
