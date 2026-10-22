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
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#ضمن <syslog.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/resource.h>
#ضمن <sys/stat.h>
#ضمن <sys/wait.h>
#ضمن <time.h>
#ضمن <unistd.h>

#ifdef HAVE_PAM
#ضمن <security/pam_appl.h>

/* We are not supporting authentication conversations */
ساكن بنية pam_conv conv = { عدم, عدم};
#endif

#ifdef __linux__
#ضمن <sys/capability.h>
#endif

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
	{ "respawn-delay-step",   1, عدم, LONGOPT_RESPAWN_DELAY_STEP},
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
ساكن حرف *changeuser, *ch_root, *ch_dir;
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
#ifdef TIOCNOTTY
ساكن صحيح tty_fd = -1;
#endif
ساكن pid_t child_pid;
ساكن صحيح respawn_count = 0;
ساكن int64_t respawn_delay; /* default set inside main() */
ساكن int64_t respawn_delay_step = TM_MS(128);
ساكن int64_t respawn_delay_cap = TM_SEC(30);
ساكن int64_t respawn_period; /* default set inside main() */
ساكن صحيح respawn_max = 10;
ساكن حرف *fifopath = عدم;
ساكن صحيح fifo_fd = 0;
ساكن حرف *pidfile = عدم;
ساكن حرف *svcname = عدم;
ساكن bool verbose = false;
#ifdef __linux__
ساكن cap_iab_t cap_iab = عدم;
ساكن unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
ساكن bool no_new_privs = false;
#endif

خارجي حرف **environ;

#𐡄𐡍 !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#𐡄𐡍 !defined(__DragonFly__)
ساكن مضمن صحيح ioprio_set(صحيح which RC_UNUSED, صحيح who RC_UNUSED,
			     صحيح ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	أرجع syscall(SYS_ioprio_set, which, who, ioprio);
#وإلا
	أرجع 0;
#endif
}
#endif

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
	exit(EXIT_FAILURE);
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
		𐡄𐡍 (exiting)
			طالما (waitpid((pid_t)(-1), عدم, WNOHANG) > 0) {}
		وإلا {
			طالما ((pid = waitpid((pid_t)(-1), عدم, WNOHANG|WNOWAIT)) > 0) {
				𐡄𐡍 (pid == child_pid)
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

ساكن حرف * expand_home(ثابت حرف *home, ثابت حرف *path)
{
	حرف *opath, *ppath, *p, *nh;
	بنية passwd *pw;

	𐡄𐡍 (!path || *path != '~')
		أرجع xstrdup(path);

	opath = ppath = xstrdup(path);
	𐡄𐡍 (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		𐡄𐡍 (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		𐡄𐡍 (pw) {
			home = pw->pw_dir;
			ppath = p;
			𐡄𐡍 (ppath)
				*ppath = '/';
		} وإلا
			home = عدم;
	} وإلا
		ppath++;

	𐡄𐡍 (!home) {
		حرر(opath);
		أرجع xstrdup(path);
	}
	𐡄𐡍 (!ppath) {
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
		𐡄𐡍 (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	أرجع cmdline;
}

ساكن pid_t exec_command(ثابت حرف *cmd)
{
	حرف *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	بنية sigaction sa;

	file = rc_service_resolve(svcname);
	𐡄𐡍 (access(file, F_OK) != 0) {
		حرر(file);
		أرجع 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, حجم (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	𐡄𐡍 (pid == 0) {
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

	𐡄𐡍 (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, عدم);
	حرر(file);
	أرجع pid;
}

RC_NORETURN ساكن فراغ child_process(حرف *exec, حرف **argv)
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
	time_t start_time;
	حرف start_count_string[20];
	حرف start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	صحيح group_count = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = عدم;
	صحيح pamr;
	ثابت حرف *ثابت *pamenv = عدم;
#endif

	setsid();

	𐡄𐡍 (svcname) {
		start_time = time(عدم);
		from_time_t(start_time_string, start_time);
		rc_service_value_set(svcname, "start_time", start_time_string);
		sprintf(start_count_string, "%i", respawn_count);
		rc_service_value_set(svcname, "start_count", start_count_string);
		sprintf(start_count_string, "%d", getpid());
		rc_service_value_set(svcname, "child_pid", start_count_string);
	}

	𐡄𐡍 (nicelevel != INT_MIN) {
		𐡄𐡍 (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		𐡄𐡍 (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} وإلا 𐡄𐡍 (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	𐡄𐡍 (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	𐡄𐡍 (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		𐡄𐡍 (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	𐡄𐡍 (changeuser != عدم) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		𐡄𐡍 (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		𐡄𐡍 (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		𐡄𐡍 (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	𐡄𐡍 (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
		group_list = xmalloc(group_count * حجم(*group_list));
		𐡄𐡍 (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	𐡄𐡍 (ch_root && chroot(ch_root) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_root, strerror(errno));

	𐡄𐡍 (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	𐡄𐡍 (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	𐡄𐡍 (changeuser && setgroups(group_count, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	𐡄𐡍 (group_list != group_buf)
		حرر(group_list);
#ifdef __linux__
	𐡄𐡍 (uid && cap_setuid(uid))
#وإلا
	𐡄𐡍 (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	𐡄𐡍 (cap_iab != عدم) {
		i = cap_iab_set_proc(cap_iab);

		𐡄𐡍 (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		𐡄𐡍 (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	𐡄𐡍 (secbits != 0) {
		𐡄𐡍 (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	𐡄𐡍 (no_new_privs) {
		𐡄𐡍 (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	طالما (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	𐡄𐡍 (changeuser != عدم) {
		pamenv = (ثابت حرف *ثابت *)pam_getenvlist(pamh);
		𐡄𐡍 (pamenv) {
			طالما (*pamenv) {
				/* Don't add strings unless they set a var */
				𐡄𐡍 (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				وإلا
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		𐡄𐡍 (strncmp(env->value, "RC_", 3) == 0
				|| strncmp(env->value, "SSD_", 4) == 0
				|| strncmp(env->value, "EINFO_", 6) == 0) {
			*strchr(env->value, '=') = '\0';
			unsetenv(env->value);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	𐡄𐡍 ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		طالما (token && *token) {
			p = strchr(token, ':');
			𐡄𐡍 (p) {
				*p++ = '\0';
				طالما (*p == ':')
					p++;
			}
			𐡄𐡍 (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				𐡄𐡍 (np != newpath)
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
	𐡄𐡍 (redirect_stdin) {
		𐡄𐡍 ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	𐡄𐡍 (redirect_stdout) {
		𐡄𐡍 ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} وإلا 𐡄𐡍 (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		𐡄𐡍 (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	𐡄𐡍 (redirect_stderr) {
		𐡄𐡍 ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} وإلا 𐡄𐡍 (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		𐡄𐡍 (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	𐡄𐡍 (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	𐡄𐡍 (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	𐡄𐡍 (notify.type == NOTIFY_FD) {
		صحيح error;

		𐡄𐡍 (notify.fd == notify.pipe[1]) {
			صحيح flags = error = fcntl(notify.fd, F_GETFD, 0);
			𐡄𐡍 (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} وإلا {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		𐡄𐡍 (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	حرر(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	𐡄𐡍 (changeuser != عدم && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ساكن فراغ supervisor(حرف *exec, حرف **argv)
{
	FILE *fp;
	حرف buf[2048];
	حرف cmd[2048];
	صحيح count;
	صحيح failing;
	صحيح health_status;
	صحيح healthcheck_respawn;
	صحيح i;
	صحيح nkilled;
	صحيح sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	بنية sigaction sa;
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
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, عدم);
	sigaction(SIGCHLD, &sa, عدم);
	sigaction(SIGTERM, &sa, عدم);

	fp = fopen(pidfile, "w");
	𐡄𐡍 (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	𐡄𐡍 (svcname)
		rc_service_daemon_set(svcname, exec, (ثابت حرف * ثابت *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	𐡄𐡍 (healthcheckdelay)
		alarm(healthcheckdelay);
	وإلا 𐡄𐡍 (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	طالما (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		𐡄𐡍 (fifo_fd > 0) {
			memset(buf, 0, حجم(buf));
			count = اقرأ(fifo_fd, buf, حجم(buf) - 1);
			close(fifo_fd);
			𐡄𐡍 (count != -1)
				buf[count] = 0;
			𐡄𐡍 (count == 0)
				استمر;
			𐡄𐡍 (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			𐡄𐡍 (strncasecmp(buf, "sig", 3) == 0) {
				𐡄𐡍 ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					𐡄𐡍 (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			استمر;
		}
		𐡄𐡍 (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			𐡄𐡍 (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			𐡄𐡍 (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			وإلا {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, عدم, عدم, child_pid, 0,
						stopgroup, false, false, true);
				𐡄𐡍 (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				وإلا
					healthcheck_respawn = 1;
			}
		}
		𐡄𐡍 (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, عدم, عدم, child_pid, 0,
					stopgroup, false, false, true);
			𐡄𐡍 (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			استمر;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		𐡄𐡍 (wait_pid == child_pid) {
			𐡄𐡍 (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			وإلا 𐡄𐡍 (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		𐡄𐡍 (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			𐡄𐡍 (first_spawn == 0)
				first_spawn = respawn_now;
			𐡄𐡍 ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_count = 0;
				first_spawn = 0;
			} وإلا
				respawn_count++;
			𐡄𐡍 (respawn_max > 0 && respawn_count > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				استمر;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_count);
			𐡄𐡍 (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			𐡄𐡍 (exiting)
				استمر;
			child_pid = fork();
			𐡄𐡍 (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			𐡄𐡍 (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, عدم);
				memset(&sa, 0, حجم(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, عدم);
				sigaction(SIGCHLD, &sa, عدم);
				sigaction(SIGTERM, &sa, عدم);
				child_process(exec, argv);
			}
			𐡄𐡍 (healthcheckdelay)
				alarm(healthcheckdelay);
			وإلا 𐡄𐡍 (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	𐡄𐡍 (svcname) {
		rc_service_daemon_set(svcname, exec, (ثابت حرف *ثابت *)argv,
				pidfile, false);
		rc_service_value_set(svcname, "child_pid", عدم);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		𐡄𐡍 (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	𐡄𐡍 (pidfile)
		unlink(pidfile);
	𐡄𐡍 (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

صحيح main(صحيح argc, حرف **argv)
{
	صحيح opt;
	حرف **c;
	صحيح x;
	bool start = false;
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
	صحيح child_argc = 0;
	حرف **child_argv = عدم;
	حرف *str = عدم;
	حرف *cmdline = عدم;
	ثابت حرف *respawn_delay_str  = "0";
	ثابت حرف *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	𐡄𐡍 (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	𐡄𐡍 (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	𐡄𐡍 ((tmp = getenv("SSD_NICELEVEL")))
		𐡄𐡍 (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	𐡄𐡍 ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		𐡄𐡍 (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		𐡄𐡍 (ionicec == 0)
			ioniced = 0;
		وإلا 𐡄𐡍 (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	𐡄𐡍 ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		𐡄𐡍 (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	𐡄𐡍 (home == عدم || p == عدم) {
		pw = getpwuid(getuid());
		𐡄𐡍 (pw != عدم) {
			𐡄𐡍 (p == عدم)
				setenv("USER", pw->pw_name, 1);
			𐡄𐡍 (home == عدم) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	𐡄𐡍 (svcname) {
		argc--;
		argv++;
	}
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة 'a':  /* --healthcheck-timer <time> */
			𐡄𐡍 (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			اكسر;

		حالة 'A':  /* --healthcheck-delay <time> */
			𐡄𐡍 (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			اكسر;

		حالة LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			𐡄𐡍 (cap_iab == عدم)
				eerrorx("Could not parse iab: %s", strerror(errno));
#وإلا
			eerrorx("Capabilities support not enabled");
#endif
			اكسر;

		حالة LONGOPT_SECBITS:
#ifdef __linux__
			𐡄𐡍 (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = عدم;
			secbits = strtoul(optarg, &tmp, 0);
			𐡄𐡍 (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#وإلا
			eerrorx("Capabilities support not enabled");
#endif
			اكسر;

		حالة LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#وإلا
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			اكسر;

		حالة 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			اكسر;

		حالة 'I': /* --ionice */
			𐡄𐡍 (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			𐡄𐡍 (ionicec == 0)
				ioniced = 0;
			وإلا 𐡄𐡍 (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			اكسر;

		حالة 'K':  /* --stop */
			stop = true;
			اكسر;

		حالة 'N':  /* --nice */
			𐡄𐡍 (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			اكسر;

		حالة LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			𐡄𐡍 (sscanf(optarg, "%d", &oom_score_adj) != 1)
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
			start = true;
			اكسر;

		حالة 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			اكسر;

		حالة 'e': /* --env */
			putenv(optarg);
			اكسر;

		حالة 'g':  /* --group <group>|<gid> */
			𐡄𐡍 (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			وإلا
				gr = getgrgid((gid_t)tid);
			𐡄𐡍 (gr == عدم)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			اكسر;

		حالة 'G': /* -G */
			stopgroup = true;
			اكسر;

		حالة 'k':
			𐡄𐡍 (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			اكسر;

		حالة 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			𐡄𐡍 (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			اكسر;

		حالة 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			اكسر;

		حالة 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			اكسر;
		حالة 'r':  /* --chroot /new/root */
			ch_root = optarg;
			اكسر;

		حالة 'u':  /* --user <username>|<uid> */
		{
			حرف dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			𐡄𐡍 (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			وإلا
				pw = getpwuid((uid_t)tid);

			𐡄𐡍 (pw == عدم)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			𐡄𐡍 (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			𐡄𐡍 (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			𐡄𐡍 (gid == 0)
				gid = pw->pw_gid;

			𐡄𐡍 (p) {
				tmp = strsep (&p, ":");
				𐡄𐡍 (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				وإلا
					gr = getgrgid((gid_t) tid);

				𐡄𐡍 (gr == عدم)
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

		حالة LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			𐡄𐡍 (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			اكسر;

		حالة LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			𐡄𐡍 (respawn_delay_cap < 0)
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
	𐡄𐡍 (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	𐡄𐡍 (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	𐡄𐡍 (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	𐡄𐡍 (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);

	umask(numask);
	𐡄𐡍 (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	𐡄𐡍 (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	𐡄𐡍 (reexec) {
		str = rc_service_value_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * حجم(حرف *));
		memset(child_argv, 0, (child_argc + 1) * حجم(حرف *));
		لكل (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_value_get(svcname, varbuf);
			child_argv[x] = str;
			حرر(varbuf);
			varbuf = عدم;
		}
		حرر(str);
		str = rc_service_value_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		حرر(str);
		exec = rc_service_value_get(svcname, "exec");
		pidfile = rc_service_value_get(svcname, "pidfile");
		retry = rc_service_value_get(svcname, "retry");
		𐡄𐡍 (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_value_set(svcname, "retry", retry);
		} وإلا
			parse_schedule(applet, عدم, sig);

		str = rc_service_value_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_value_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_value_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_value_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} وإلا 𐡄𐡍 (start) {
		𐡄𐡍 (exec) {
			𐡄𐡍 (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			𐡄𐡍 (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				𐡄𐡍 (ch_root)
					xasprintf(&exec_file, "%s/%s", ch_root, exec);
				وإلا
					xasprintf(&exec_file, "%s", exec);
			} وإلا {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = عدم;
				طالما ((token = strsep(&p, ":"))) {
					𐡄𐡍 (ch_root)
						xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
					وإلا
						xasprintf(&exec_file, "%s/%s", token, exec);
					𐡄𐡍 (exec_file && access(exec_file, F_OK) == 0)
						اكسر;
					حرر(exec_file);
					exec_file = عدم;
				}
				حرر(tmp);
			}
			𐡄𐡍 (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				حرر(exec_file);
				exit(EXIT_FAILURE);
			}
		} وإلا
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		𐡄𐡍 (pid != -1)
			𐡄𐡍 (do_stop(applet, exec, (ثابت حرف * ثابت *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		𐡄𐡍 (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		𐡄𐡍 (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		𐡄𐡍 (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_value_set(svcname, "retry", retry);
		} وإلا
			parse_schedule(applet, عدم, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		حرر(cmdline);
		cmdline = عدم;

		/* Remove existing pidfile */
		𐡄𐡍 (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		𐡄𐡍 (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_value_set(svcname, "pidfile", pidfile);
		rc_service_value_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_value_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, حجم(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_value_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, حجم(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_value_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, حجم(numbuf), "%i", respawn_max);
		rc_service_value_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		𐡄𐡍 (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		𐡄𐡍 (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		𐡄𐡍 (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		𐡄𐡍 (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		وإلا 𐡄𐡍 (child_pid != 0) {
			𐡄𐡍 (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			طالما (c && *c) {
				varbuf = عدم;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_value_set(svcname, varbuf, *c);
				حرر(varbuf);
				varbuf = عدم;
				x++;
				c++;
			}
			snprintf(numbuf, حجم(numbuf), "%d", x);
			rc_service_value_set(svcname, "argc", numbuf);
			rc_service_value_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} وإلا
			child_process(exec, argv);
	} وإلا 𐡄𐡍 (stop) {
		pid = get_pid(applet, pidfile);
		𐡄𐡍 (pid != -1) {
			i = kill(pid, SIGTERM);
			𐡄𐡍 (i != 0)
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
		𐡄𐡍 (pidfile)
			unlink(pidfile);
		𐡄𐡍 (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (ثابت حرف *ثابت *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} وإلا 𐡄𐡍 (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		𐡄𐡍 (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = اكتب(fifo_fd, str, strlen(str));
		𐡄𐡍 (x == -1) {
			حرر(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		حرر(tmp);
		exit(EXIT_SUCCESS);
	}
}
