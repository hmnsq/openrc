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
স্থির কাঠামো pam_conv conv = { ফাঁকা, ফাঁকা};
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

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = ফাঁকা;
ধ্রুবক অক্ষর getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "healthcheck-timer",        1, ফাঁকা, 'a'},
	{ "healthcheck-delay",        1, ফাঁকা, 'A'},
	{ "capabilities", 1, ফাঁকা, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ফাঁকা, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ফাঁকা, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ফাঁকা, 'D'},
	{ "respawn-delay-step",   1, ফাঁকা, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, ফাঁকা, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ফাঁকা, 'd'},
	{ "env",          1, ফাঁকা, 'e'},
	{ "group",        1, ফাঁকা, 'g'},
	{ "stop-group",   0, ফাঁকা, 'G'},
	{ "ionice",       1, ফাঁকা, 'I'},
	{ "stop",         0, ফাঁকা, 'K'},
	{ "umask",        1, ফাঁকা, 'k'},
	{ "respawn-max",    1, ফাঁকা, 'm'},
	{ "nicelevel",    1, ফাঁকা, 'N'},
	{ "oom-score-adj",1, ফাঁকা, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ফাঁকা, 'p'},
	{ "respawn-period",        1, ফাঁকা, 'P'},
	{ "retry",       1, ফাঁকা, 'R'},
	{ "chroot",       1, ফাঁকা, 'r'},
	{ "signal",       1, ফাঁকা, 's'},
	{ "start",        0, ফাঁকা, 'S'},
	{ "user",         1, ফাঁকা, 'u'},
	{ "stdin",        1, ফাঁকা, '0'},
	{ "stdout",       1, ফাঁকা, '1'},
	{ "stderr",       1, ফাঁকা, '2'},
	{ "stdout-logger",1, ফাঁকা, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ফাঁকা, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ফাঁকা, '3'},
	{ "notify",       1, ফাঁকা, LONGOPT_NOTIFY},
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
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
ধ্রুবক অক্ষর *usagestring = ফাঁকা;

স্থির পূর্ণসংখ্যা healthcheckdelay = 0;
স্থির পূর্ণসংখ্যা healthchecktimer = 0;
স্থির অস্থির sig_atomic_t do_healthcheck = 0;
স্থির অস্থির sig_atomic_t exiting = 0;
স্থির পূর্ণসংখ্যা nicelevel = INT_MIN;
স্থির পূর্ণসংখ্যা ionicec = -1;
স্থির পূর্ণসংখ্যা ioniced = 0;
স্থির পূর্ণসংখ্যা oom_score_adj = INT_MIN;
স্থির অক্ষর *changeuser, *ch_মূল, *ch_dir;
স্থির uid_t uid = 0;
স্থির gid_t gid = 0;
স্থির bool stopgroup = false;
স্থির পূর্ণসংখ্যা devnull_fd = -1;
স্থির পূর্ণসংখ্যা stdin_fd;
স্থির পূর্ণসংখ্যা stdout_fd;
স্থির পূর্ণসংখ্যা stderr_fd;
স্থির কাঠামো notify notify;
স্থির অক্ষর *redirect_stdin = ফাঁকা;
স্থির অক্ষর *redirect_stderr = ফাঁকা;
স্থির অক্ষর *redirect_stdout = ফাঁকা;
স্থির অক্ষর *stderr_process = ফাঁকা;
স্থির অক্ষর *stdout_process = ফাঁকা;
#ifdef TIOCNOTTY
স্থির পূর্ণসংখ্যা tty_fd = -1;
#endif
স্থির pid_t child_pid;
স্থির পূর্ণসংখ্যা respawn_গুনতি = 0;
স্থির int64_t respawn_delay; /* default set inside main() */
স্থির int64_t respawn_delay_step = TM_MS(128);
স্থির int64_t respawn_delay_cap = TM_SEC(30);
স্থির int64_t respawn_period; /* default set inside main() */
স্থির পূর্ণসংখ্যা respawn_max = 10;
স্থির অক্ষর *fifopath = ফাঁকা;
স্থির পূর্ণসংখ্যা fifo_fd = 0;
স্থির অক্ষর *pidfile = ফাঁকা;
স্থির অক্ষর *svcname = ফাঁকা;
স্থির bool verbose = false;
#ifdef __linux__
স্থির cap_iab_t cap_iab = ফাঁকা;
স্থির unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
স্থির bool no_new_privs = false;
#endif

বাহ্যিক অক্ষর **environ;

#যদি !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#যদি !defined(__DragonFly__)
স্থির অন্তঃরেখ পূর্ণসংখ্যা ioprio_set(পূর্ণসংখ্যা which RC_UNUSED, পূর্ণসংখ্যা who RC_UNUSED,
			     পূর্ণসংখ্যা ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ফেরত syscall(SYS_ioprio_set, which, who, ioprio);
#অন্যথা
	ফেরত 0;
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
			(অক্ষর *) ফাঁকা);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

স্থির শূন্য handle_signal(পূর্ণসংখ্যা sig)
{
	পূর্ণসংখ্যা serrno = errno;
	pid_t pid;

	নির্বাচন (sig) {
	ক্ষেত্র SIGALRM:
		do_healthcheck = 1;
		বিরতি;
	ক্ষেত্র SIGCHLD:
		যদি (exiting)
			যখন (waitpid((pid_t)(-1), ফাঁকা, WNOHANG) > 0) {}
		অন্যথা {
			যখন ((pid = waitpid((pid_t)(-1), ফাঁকা, WNOHANG|WNOWAIT)) > 0) {
				যদি (pid == child_pid)
					বিরতি;
				pid = waitpid(pid, ফাঁকা, WNOHANG);
			}
		}
		বিরতি;
	ক্ষেত্র SIGTERM:
		exiting = 1;
		বিরতি;
	পূর্বনির্ধারিত:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

স্থির অক্ষর * expand_home(ধ্রুবক অক্ষর *home, ধ্রুবক অক্ষর *path)
{
	অক্ষর *opath, *ppath, *p, *nh;
	কাঠামো passwd *pw;

	যদি (!path || *path != '~')
		ফেরত xstrdup(path);

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
			home = ফাঁকা;
	} অন্যথা
		ppath++;

	যদি (!home) {
		মুক্ত(opath);
		ফেরত xstrdup(path);
	}
	যদি (!ppath) {
		মুক্ত(opath);
		ফেরত xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	মুক্ত(opath);
	ফেরত nh;
}

স্থির অক্ষর *make_cmdline(অক্ষর **argv)
{
	অক্ষর **c;
	অক্ষর *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	জন্য (c = argv; c && *c; c++) {
		fputs(*c, mem);
		যদি (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	ফেরত cmdline;
}

স্থির pid_t exec_command(ধ্রুবক অক্ষর *cmd)
{
	অক্ষর *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	কাঠামো sigaction sa;

	file = rc_service_resolve(svcname);
	যদি (access(file, F_OK) != 0) {
		মুক্ত(file);
		ফেরত 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, আকার (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	যদি (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ফাঁকা);
		sigaction(SIGHUP, &sa, ফাঁকা);
		sigaction(SIGINT, &sa, ফাঁকা);
		sigaction(SIGQUIT, &sa, ফাঁকা);
		sigaction(SIGTERM, &sa, ফাঁকা);
		sigaction(SIGUSR1, &sa, ফাঁকা);
		sigaction(SIGWINCH, &sa, ফাঁকা);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ফাঁকা);

		/* Safe to run now */
		execl(file, file, cmd, (অক্ষর *) ফাঁকা);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	যদি (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ফাঁকা);
	মুক্ত(file);
	ফেরত pid;
}

RC_NORETURN স্থির শূন্য child_process(অক্ষর *exec, অক্ষর **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	পূর্ণসংখ্যা i;
	অক্ষর *p;
	অক্ষর *token;
	size_t len;
	অক্ষর *newpath;
	অক্ষর *np;
	অক্ষর *cmdline = ফাঁকা;
	time_t start_time;
	অক্ষর start_গুনতি_string[20];
	অক্ষর start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	পূর্ণসংখ্যা group_গুনতি = ARRAY_আকার(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = ফাঁকা;
	পূর্ণসংখ্যা pamr;
	ধ্রুবক অক্ষর *ধ্রুবক *pamenv = ফাঁকা;
#endif

	setsid();

	যদি (svcname) {
		start_time = time(ফাঁকা);
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
	যদি (changeuser != ফাঁকা) {
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
		group_list = xmalloc(group_গুনতি * আকার(*group_list));
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
	যদি (cap_iab != ফাঁকা) {
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
	যখন (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	যদি (changeuser != ফাঁকা) {
		pamenv = (ধ্রুবক অক্ষর *ধ্রুবক *)pam_getenvlist(pamh);
		যদি (pamenv) {
			যখন (*pamenv) {
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
		যখন (token && *token) {
			p = strchr(token, ':');
			যদি (p) {
				*p++ = '\0';
				যখন (*p == ':')
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
		পূর্ণসংখ্যা error;

		যদি (notify.fd == notify.pipe[1]) {
			পূর্ণসংখ্যা flags = error = fcntl(notify.fd, F_GETFD, 0);
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
	যদি (changeuser != ফাঁকা && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN স্থির শূন্য supervisor(অক্ষর *exec, অক্ষর **argv)
{
	FILE *fp;
	অক্ষর buf[2048];
	অক্ষর cmd[2048];
	পূর্ণসংখ্যা গুনতি;
	পূর্ণসংখ্যা failing;
	পূর্ণসংখ্যা health_status;
	পূর্ণসংখ্যা healthcheck_respawn;
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা nkilled;
	পূর্ণসংখ্যা sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	কাঠামো sigaction sa;
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
	memset(&sa, 0, আকার(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ফাঁকা);
	sigaction(SIGCHLD, &sa, ফাঁকা);
	sigaction(SIGTERM, &sa, ফাঁকা);

	fp = fopen(pidfile, "w");
	যদি (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	যদি (svcname)
		rc_service_daemon_set(svcname, exec, (ধ্রুবক অক্ষর * ধ্রুবক *) argv,
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
	যখন (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		যদি (fifo_fd > 0) {
			memset(buf, 0, আকার(buf));
			গুনতি = পঠ(fifo_fd, buf, আকার(buf) - 1);
			close(fifo_fd);
			যদি (গুনতি != -1)
				buf[গুনতি] = 0;
			যদি (গুনতি == 0)
				চালু;
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
			চালু;
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
				nkilled = run_stop_schedule(applet, ফাঁকা, ফাঁকা, child_pid, 0,
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
			nkilled = run_stop_schedule(applet, ফাঁকা, ফাঁকা, child_pid, 0,
					stopgroup, false, false, true);
			যদি (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			চালু;
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
				চালু;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_গুনতি);
			যদি (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			যদি (exiting)
				চালু;
			child_pid = fork();
			যদি (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			যদি (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, ফাঁকা);
				memset(&sa, 0, আকার(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ফাঁকা);
				sigaction(SIGCHLD, &sa, ফাঁকা);
				sigaction(SIGTERM, &sa, ফাঁকা);
				child_process(exec, argv);
			}
			যদি (healthcheckdelay)
				alarm(healthcheckdelay);
			অন্যথা যদি (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	যদি (svcname) {
		rc_service_daemon_set(svcname, exec, (ধ্রুবক অক্ষর *ধ্রুবক *)argv,
				pidfile, false);
		rc_service_মান_set(svcname, "child_pid", ফাঁকা);
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

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা opt;
	অক্ষর **c;
	পূর্ণসংখ্যা x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	অক্ষর *exec = ফাঁকা;
	অক্ষর *retry = ফাঁকা;
	পূর্ণসংখ্যা sig = SIGTERM;
	অক্ষর *home = ফাঁকা;
	পূর্ণসংখ্যা tid = 0;
	pid_t pid;
	অক্ষর *tmp;
	অক্ষর *p;
	অক্ষর *token;
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা n;
	অক্ষর *exec_file = ফাঁকা;
	অক্ষর *varbuf = ফাঁকা;
	অক্ষর numbuf[64];
	কাঠামো timespec ts;
	কাঠামো passwd *pw;
	কাঠামো group *gr;
	FILE *fp;
	mode_t numask = 022;
	পূর্ণসংখ্যা child_argc = 0;
	অক্ষর **child_argv = ফাঁকা;
	অক্ষর *str = ফাঁকা;
	অক্ষর *cmdline = ফাঁকা;
	ধ্রুবক অক্ষর *respawn_delay_str  = "0";
	ধ্রুবক অক্ষর *respawn_period_str = "12sec";

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
	যদি (home == ফাঁকা || p == ফাঁকা) {
		pw = getpwuid(getuid());
		যদি (pw != ফাঁকা) {
			যদি (p == ফাঁকা)
				setenv("USER", pw->pw_name, 1);
			যদি (home == ফাঁকা) {
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
	যখন ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (পূর্ণসংখ্যা *) 0)) != -1)
		নির্বাচন (opt) {
		ক্ষেত্র 'a':  /* --healthcheck-timer <time> */
			যদি (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			বিরতি;

		ক্ষেত্র 'A':  /* --healthcheck-delay <time> */
			যদি (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			বিরতি;

		ক্ষেত্র LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			যদি (cap_iab == ফাঁকা)
				eerrorx("Could not parse iab: %s", strerror(errno));
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			বিরতি;

		ক্ষেত্র LONGOPT_SECBITS:
#ifdef __linux__
			যদি (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ফাঁকা;
			secbits = strtoul(optarg, &tmp, 0);
			যদি (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#অন্যথা
			eerrorx("Capabilities support not enabled");
#endif
			বিরতি;

		ক্ষেত্র LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#অন্যথা
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			বিরতি;

		ক্ষেত্র 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			বিরতি;

		ক্ষেত্র 'I': /* --ionice */
			যদি (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			যদি (ionicec == 0)
				ioniced = 0;
			অন্যথা যদি (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			বিরতি;

		ক্ষেত্র 'K':  /* --stop */
			stop = true;
			বিরতি;

		ক্ষেত্র 'N':  /* --nice */
			যদি (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			বিরতি;

		ক্ষেত্র LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			যদি (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			বিরতি;

		ক্ষেত্র 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			বিরতি;

		ক্ষেত্র 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			বিরতি;
		ক্ষেত্র 'S':  /* --start */
			start = true;
			বিরতি;

		ক্ষেত্র 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			বিরতি;

		ক্ষেত্র 'e': /* --env */
			putenv(optarg);
			বিরতি;

		ক্ষেত্র 'g':  /* --group <group>|<gid> */
			যদি (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			অন্যথা
				gr = getgrgid((gid_t)tid);
			যদি (gr == ফাঁকা)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			বিরতি;

		ক্ষেত্র 'G': /* -G */
			stopgroup = true;
			বিরতি;

		ক্ষেত্র 'k':
			যদি (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			বিরতি;

		ক্ষেত্র 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			যদি (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			বিরতি;

		ক্ষেত্র 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			বিরতি;

		ক্ষেত্র 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			বিরতি;
		ক্ষেত্র 'r':  /* --chroot /new/root */
			ch_মূল = optarg;
			বিরতি;

		ক্ষেত্র 'u':  /* --user <username>|<uid> */
		{
			অক্ষর dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			যদি (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			অন্যথা
				pw = getpwuid((uid_t)tid);

			যদি (pw == ফাঁকা)
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

				যদি (gr == ফাঁকা)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		বিরতি;

		ক্ষেত্র '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			বিরতি;

		ক্ষেত্র '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			বিরতি;

		ক্ষেত্র '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			বিরতি;
		ক্ষেত্র '3':  /* --reexec */
			reexec = true;
			বিরতি;

		ক্ষেত্র LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			বিরতি;

		ক্ষেত্র LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			বিরতি;

		ক্ষেত্র LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			বিরতি;

		ক্ষেত্র LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			যদি (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			বিরতি;

		ক্ষেত্র LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			যদি (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			বিরতি;

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
		child_argv = xmalloc((child_argc + 1) * আকার(অক্ষর *));
		memset(child_argv, 0, (child_argc + 1) * আকার(অক্ষর *));
		জন্য (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_মান_get(svcname, varbuf);
			child_argv[x] = str;
			মুক্ত(varbuf);
			varbuf = ফাঁকা;
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
			parse_schedule(applet, ফাঁকা, sig);

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
				exec_file = ফাঁকা;
				যখন ((token = strsep(&p, ":"))) {
					যদি (ch_মূল)
						xasprintf(&exec_file, "%s/%s/%s", ch_মূল, token, exec);
					অন্যথা
						xasprintf(&exec_file, "%s/%s", token, exec);
					যদি (exec_file && access(exec_file, F_OK) == 0)
						বিরতি;
					মুক্ত(exec_file);
					exec_file = ফাঁকা;
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
			যদি (do_stop(applet, exec, (ধ্রুবক অক্ষর * ধ্রুবক *)argv, pid, uid,
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
			parse_schedule(applet, ফাঁকা, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		মুক্ত(cmdline);
		cmdline = ফাঁকা;

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
		snprintf(numbuf, আকার(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_মান_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, আকার(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_মান_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, আকার(numbuf), "%i", respawn_max);
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
			যখন (c && *c) {
				varbuf = ফাঁকা;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_মান_set(svcname, varbuf, *c);
				মুক্ত(varbuf);
				varbuf = ফাঁকা;
				x++;
				c++;
			}
			snprintf(numbuf, আকার(numbuf), "%d", x);
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
				যখন (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ফাঁকা);
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
			    (ধ্রুবক অক্ষর *ধ্রুবক *)argv,
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
