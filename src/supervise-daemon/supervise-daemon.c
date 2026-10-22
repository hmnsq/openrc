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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <grp.h>
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <sys/resource.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>

#ifdef HAVE_PAM
#அந்தர்பாவ <security/pam_appl.h>

/* We are not supporting authentication conversations */
நிலையான கட்டமைப்பு pam_conv conv = { வெற்று, வெற்று};
#endif

#ifdef __linux__
#அந்தர்பாவ <sys/capability.h>
#endif

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "plugin.h"
#அந்தர்பாவ "schedules.h"
#அந்தர்பாவ "timeutils.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
எண்ணிக்கை {
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

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "healthcheck-timer",        1, வெற்று, 'a'},
	{ "healthcheck-delay",        1, வெற்று, 'A'},
	{ "capabilities", 1, வெற்று, LONGOPT_CAPABILITIES},
	{ "secbits",      1, வெற்று, LONGOPT_SECBITS},
	{ "no-new-privs", 0, வெற்று, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, வெற்று, 'D'},
	{ "respawn-delay-step",   1, வெற்று, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, வெற்று, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, வெற்று, 'd'},
	{ "env",          1, வெற்று, 'e'},
	{ "group",        1, வெற்று, 'g'},
	{ "stop-group",   0, வெற்று, 'G'},
	{ "ionice",       1, வெற்று, 'I'},
	{ "stop",         0, வெற்று, 'K'},
	{ "umask",        1, வெற்று, 'k'},
	{ "respawn-max",    1, வெற்று, 'm'},
	{ "nicelevel",    1, வெற்று, 'N'},
	{ "oom-score-adj",1, வெற்று, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, வெற்று, 'p'},
	{ "respawn-period",        1, வெற்று, 'P'},
	{ "retry",       1, வெற்று, 'R'},
	{ "chroot",       1, வெற்று, 'r'},
	{ "signal",       1, வெற்று, 's'},
	{ "start",        0, வெற்று, 'S'},
	{ "user",         1, வெற்று, 'u'},
	{ "stdin",        1, வெற்று, '0'},
	{ "stdout",       1, வெற்று, '1'},
	{ "stderr",       1, வெற்று, '2'},
	{ "stdout-logger",1, வெற்று, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, வெற்று, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, வெற்று, '3'},
	{ "notify",       1, வெற்று, LONGOPT_NOTIFY},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = வெற்று;

நிலையான முழு healthcheckdelay = 0;
நிலையான முழு healthchecktimer = 0;
நிலையான நிலையற்ற sig_atomic_t do_healthcheck = 0;
நிலையான நிலையற்ற sig_atomic_t exiting = 0;
நிலையான முழு nicelevel = INT_MIN;
நிலையான முழு ionicec = -1;
நிலையான முழு ioniced = 0;
நிலையான முழு oom_score_adj = INT_MIN;
நிலையான எழுத்து *changeuser, *ch_வேர், *ch_dir;
நிலையான uid_t uid = 0;
நிலையான gid_t gid = 0;
நிலையான bool stopgroup = false;
நிலையான முழு devnull_fd = -1;
நிலையான முழு stdin_fd;
நிலையான முழு stdout_fd;
நிலையான முழு stderr_fd;
நிலையான கட்டமைப்பு notify notify;
நிலையான எழுத்து *redirect_stdin = வெற்று;
நிலையான எழுத்து *redirect_stderr = வெற்று;
நிலையான எழுத்து *redirect_stdout = வெற்று;
நிலையான எழுத்து *stderr_process = வெற்று;
நிலையான எழுத்து *stdout_process = வெற்று;
#ifdef TIOCNOTTY
நிலையான முழு tty_fd = -1;
#endif
நிலையான pid_t child_pid;
நிலையான முழு respawn_மொத்தம் = 0;
நிலையான int64_t respawn_delay; /* default set inside main() */
நிலையான int64_t respawn_delay_step = TM_MS(128);
நிலையான int64_t respawn_delay_cap = TM_SEC(30);
நிலையான int64_t respawn_period; /* default set inside main() */
நிலையான முழு respawn_max = 10;
நிலையான எழுத்து *fifopath = வெற்று;
நிலையான முழு fifo_fd = 0;
நிலையான எழுத்து *pidfile = வெற்று;
நிலையான எழுத்து *svcname = வெற்று;
நிலையான bool verbose = false;
#ifdef __linux__
நிலையான cap_iab_t cap_iab = வெற்று;
நிலையான unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
நிலையான bool no_new_privs = false;
#endif

வெளி எழுத்து **environ;

#எனில் !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#எனில் !defined(__DragonFly__)
நிலையான உள்வரி முழு ioprio_set(முழு which RC_UNUSED, முழு who RC_UNUSED,
			     முழு ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	திரும்பு syscall(SYS_ioprio_set, which, who, ioprio);
#இல்லையெனில்
	திரும்பு 0;
#endif
}
#endif

நிலையான வெறுமை cleanup(வெறுமை)
{
	விடுவி(changeuser);
}

RC_NORETURN நிலையான வெறுமை re_exec_supervisor(வெறுமை)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(எழுத்து *) வெற்று);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

நிலையான வெறுமை handle_signal(முழு sig)
{
	முழு serrno = errno;
	pid_t pid;

	தேர்வு (sig) {
	நிலை SIGALRM:
		do_healthcheck = 1;
		நிறுத்து;
	நிலை SIGCHLD:
		எனில் (exiting)
			போது (waitpid((pid_t)(-1), வெற்று, WNOHANG) > 0) {}
		இல்லையெனில் {
			போது ((pid = waitpid((pid_t)(-1), வெற்று, WNOHANG|WNOWAIT)) > 0) {
				எனில் (pid == child_pid)
					நிறுத்து;
				pid = waitpid(pid, வெற்று, WNOHANG);
			}
		}
		நிறுத்து;
	நிலை SIGTERM:
		exiting = 1;
		நிறுத்து;
	இயல்பு:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

நிலையான எழுத்து * expand_home(மாறிலி எழுத்து *home, மாறிலி எழுத்து *path)
{
	எழுத்து *opath, *ppath, *p, *nh;
	கட்டமைப்பு passwd *pw;

	எனில் (!path || *path != '~')
		திரும்பு xstrdup(path);

	opath = ppath = xstrdup(path);
	எனில் (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		எனில் (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		எனில் (pw) {
			home = pw->pw_dir;
			ppath = p;
			எனில் (ppath)
				*ppath = '/';
		} இல்லையெனில்
			home = வெற்று;
	} இல்லையெனில்
		ppath++;

	எனில் (!home) {
		விடுவி(opath);
		திரும்பு xstrdup(path);
	}
	எனில் (!ppath) {
		விடுவி(opath);
		திரும்பு xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	விடுவி(opath);
	திரும்பு nh;
}

நிலையான எழுத்து *make_cmdline(எழுத்து **argv)
{
	எழுத்து **c;
	எழுத்து *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	க்கு (c = argv; c && *c; c++) {
		fputs(*c, mem);
		எனில் (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	திரும்பு cmdline;
}

நிலையான pid_t exec_command(மாறிலி எழுத்து *cmd)
{
	எழுத்து *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	கட்டமைப்பு sigaction sa;

	file = rc_service_resolve(svcname);
	எனில் (access(file, F_OK) != 0) {
		விடுவி(file);
		திரும்பு 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, அளவு (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	எனில் (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, வெற்று);
		sigaction(SIGHUP, &sa, வெற்று);
		sigaction(SIGINT, &sa, வெற்று);
		sigaction(SIGQUIT, &sa, வெற்று);
		sigaction(SIGTERM, &sa, வெற்று);
		sigaction(SIGUSR1, &sa, வெற்று);
		sigaction(SIGWINCH, &sa, வெற்று);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, வெற்று);

		/* Safe to run now */
		execl(file, file, cmd, (எழுத்து *) வெற்று);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	எனில் (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, வெற்று);
	விடுவி(file);
	திரும்பு pid;
}

RC_NORETURN நிலையான வெறுமை child_process(எழுத்து *exec, எழுத்து **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	முழு i;
	எழுத்து *p;
	எழுத்து *token;
	size_t len;
	எழுத்து *newpath;
	எழுத்து *np;
	எழுத்து *cmdline = வெற்று;
	time_t start_time;
	எழுத்து start_மொத்தம்_string[20];
	எழுத்து start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	முழு group_மொத்தம் = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = வெற்று;
	முழு pamr;
	மாறிலி எழுத்து *மாறிலி *pamenv = வெற்று;
#endif

	setsid();

	எனில் (svcname) {
		start_time = time(வெற்று);
		from_time_t(start_time_string, start_time);
		rc_service_மதிப்பு_set(svcname, "start_time", start_time_string);
		sprintf(start_மொத்தம்_string, "%i", respawn_மொத்தம்);
		rc_service_மதிப்பு_set(svcname, "start_count", start_மொத்தம்_string);
		sprintf(start_மொத்தம்_string, "%d", getpid());
		rc_service_மதிப்பு_set(svcname, "child_pid", start_மொத்தம்_string);
	}

	எனில் (nicelevel != INT_MIN) {
		எனில் (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		எனில் (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} இல்லையெனில் எனில் (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	எனில் (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	எனில் (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		எனில் (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	எனில் (changeuser != வெற்று) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		எனில் (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		எனில் (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		எனில் (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	எனில் (changeuser && getgrouplist(changeuser, gid, group_list, &group_மொத்தம்) < 0) {
		group_list = xmalloc(group_மொத்தம் * அளவு(*group_list));
		எனில் (getgrouplist(changeuser, gid, group_list, &group_மொத்தம்) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	எனில் (ch_வேர் && chroot(ch_வேர்) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_வேர், strerror(errno));

	எனில் (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	எனில் (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	எனில் (changeuser && setgroups(group_மொத்தம், group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	எனில் (group_list != group_buf)
		விடுவி(group_list);
#ifdef __linux__
	எனில் (uid && cap_setuid(uid))
#இல்லையெனில்
	எனில் (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	எனில் (cap_iab != வெற்று) {
		i = cap_iab_set_proc(cap_iab);

		எனில் (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		எனில் (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	எனில் (secbits != 0) {
		எனில் (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	எனில் (no_new_privs) {
		எனில் (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	போது (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	எனில் (changeuser != வெற்று) {
		pamenv = (மாறிலி எழுத்து *மாறிலி *)pam_getenvlist(pamh);
		எனில் (pamenv) {
			போது (*pamenv) {
				/* Don't add strings unless they set a var */
				எனில் (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				இல்லையெனில்
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		எனில் (strncmp(env->மதிப்பு, "RC_", 3) == 0
				|| strncmp(env->மதிப்பு, "SSD_", 4) == 0
				|| strncmp(env->மதிப்பு, "EINFO_", 6) == 0) {
			*strchr(env->மதிப்பு, '=') = '\0';
			unsetenv(env->மதிப்பு);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	எனில் ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		போது (token && *token) {
			p = strchr(token, ':');
			எனில் (p) {
				*p++ = '\0';
				போது (*p == ':')
					p++;
			}
			எனில் (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				எனில் (np != newpath)
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
	எனில் (redirect_stdin) {
		எனில் ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	எனில் (redirect_stdout) {
		எனில் ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} இல்லையெனில் எனில் (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		எனில் (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	எனில் (redirect_stderr) {
		எனில் ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} இல்லையெனில் எனில் (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		எனில் (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	எனில் (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	எனில் (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	எனில் (notify.type == NOTIFY_FD) {
		முழு error;

		எனில் (notify.fd == notify.pipe[1]) {
			முழு flags = error = fcntl(notify.fd, F_GETFD, 0);
			எனில் (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} இல்லையெனில் {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		எனில் (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	விடுவி(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	எனில் (changeuser != வெற்று && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN நிலையான வெறுமை supervisor(எழுத்து *exec, எழுத்து **argv)
{
	FILE *fp;
	எழுத்து buf[2048];
	எழுத்து cmd[2048];
	முழு மொத்தம்;
	முழு failing;
	முழு health_status;
	முழு healthcheck_respawn;
	முழு i;
	முழு nkilled;
	முழு sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	கட்டமைப்பு sigaction sa;
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
	memset(&sa, 0, அளவு(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, வெற்று);
	sigaction(SIGCHLD, &sa, வெற்று);
	sigaction(SIGTERM, &sa, வெற்று);

	fp = fopen(pidfile, "w");
	எனில் (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	எனில் (svcname)
		rc_service_daemon_set(svcname, exec, (மாறிலி எழுத்து * மாறிலி *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	எனில் (healthcheckdelay)
		alarm(healthcheckdelay);
	இல்லையெனில் எனில் (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	போது (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		எனில் (fifo_fd > 0) {
			memset(buf, 0, அளவு(buf));
			மொத்தம் = பட(fifo_fd, buf, அளவு(buf) - 1);
			close(fifo_fd);
			எனில் (மொத்தம் != -1)
				buf[மொத்தம்] = 0;
			எனில் (மொத்தம் == 0)
				தொடர்;
			எனில் (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			எனில் (strncasecmp(buf, "sig", 3) == 0) {
				எனில் ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					எனில் (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			தொடர்;
		}
		எனில் (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			எனில் (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			எனில் (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			இல்லையெனில் {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, வெற்று, வெற்று, child_pid, 0,
						stopgroup, false, false, true);
				எனில் (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				இல்லையெனில்
					healthcheck_respawn = 1;
			}
		}
		எனில் (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, வெற்று, வெற்று, child_pid, 0,
					stopgroup, false, false, true);
			எனில் (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			தொடர்;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		எனில் (wait_pid == child_pid) {
			எனில் (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			இல்லையெனில் எனில் (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		எனில் (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			எனில் (first_spawn == 0)
				first_spawn = respawn_now;
			எனில் ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_மொத்தம் = 0;
				first_spawn = 0;
			} இல்லையெனில்
				respawn_மொத்தம்++;
			எனில் (respawn_max > 0 && respawn_மொத்தம் > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				தொடர்;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_மொத்தம்);
			எனில் (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			எனில் (exiting)
				தொடர்;
			child_pid = fork();
			எனில் (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			எனில் (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, வெற்று);
				memset(&sa, 0, அளவு(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, வெற்று);
				sigaction(SIGCHLD, &sa, வெற்று);
				sigaction(SIGTERM, &sa, வெற்று);
				child_process(exec, argv);
			}
			எனில் (healthcheckdelay)
				alarm(healthcheckdelay);
			இல்லையெனில் எனில் (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	எனில் (svcname) {
		rc_service_daemon_set(svcname, exec, (மாறிலி எழுத்து *மாறிலி *)argv,
				pidfile, false);
		rc_service_மதிப்பு_set(svcname, "child_pid", வெற்று);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		எனில் (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	எனில் (pidfile)
		unlink(pidfile);
	எனில் (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு opt;
	எழுத்து **c;
	முழு x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	எழுத்து *exec = வெற்று;
	எழுத்து *retry = வெற்று;
	முழு sig = SIGTERM;
	எழுத்து *home = வெற்று;
	முழு tid = 0;
	pid_t pid;
	எழுத்து *tmp;
	எழுத்து *p;
	எழுத்து *token;
	முழு i;
	முழு n;
	எழுத்து *exec_file = வெற்று;
	எழுத்து *varbuf = வெற்று;
	எழுத்து numbuf[64];
	கட்டமைப்பு timespec ts;
	கட்டமைப்பு passwd *pw;
	கட்டமைப்பு group *gr;
	FILE *fp;
	mode_t numask = 022;
	முழு child_argc = 0;
	எழுத்து **child_argv = வெற்று;
	எழுத்து *str = வெற்று;
	எழுத்து *cmdline = வெற்று;
	மாறிலி எழுத்து *respawn_delay_str  = "0";
	மாறிலி எழுத்து *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	எனில் (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	எனில் (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	எனில் ((tmp = getenv("SSD_NICELEVEL")))
		எனில் (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	எனில் ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		எனில் (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		எனில் (ionicec == 0)
			ioniced = 0;
		இல்லையெனில் எனில் (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	எனில் ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		எனில் (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	எனில் (home == வெற்று || p == வெற்று) {
		pw = getpwuid(getuid());
		எனில் (pw != வெற்று) {
			எனில் (p == வெற்று)
				setenv("USER", pw->pw_name, 1);
			எனில் (home == வெற்று) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	எனில் (svcname) {
		argc--;
		argv++;
	}
	போது ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (முழு *) 0)) != -1)
		தேர்வு (opt) {
		நிலை 'a':  /* --healthcheck-timer <time> */
			எனில் (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			நிறுத்து;

		நிலை 'A':  /* --healthcheck-delay <time> */
			எனில் (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			நிறுத்து;

		நிலை LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			எனில் (cap_iab == வெற்று)
				eerrorx("Could not parse iab: %s", strerror(errno));
#இல்லையெனில்
			eerrorx("Capabilities support not enabled");
#endif
			நிறுத்து;

		நிலை LONGOPT_SECBITS:
#ifdef __linux__
			எனில் (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = வெற்று;
			secbits = strtoul(optarg, &tmp, 0);
			எனில் (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#இல்லையெனில்
			eerrorx("Capabilities support not enabled");
#endif
			நிறுத்து;

		நிலை LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#இல்லையெனில்
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			நிறுத்து;

		நிலை 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			நிறுத்து;

		நிலை 'I': /* --ionice */
			எனில் (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			எனில் (ionicec == 0)
				ioniced = 0;
			இல்லையெனில் எனில் (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			நிறுத்து;

		நிலை 'K':  /* --stop */
			stop = true;
			நிறுத்து;

		நிலை 'N':  /* --nice */
			எனில் (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			நிறுத்து;

		நிலை LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			எனில் (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			நிறுத்து;

		நிலை 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			நிறுத்து;

		நிலை 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			நிறுத்து;
		நிலை 'S':  /* --start */
			start = true;
			நிறுத்து;

		நிலை 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			நிறுத்து;

		நிலை 'e': /* --env */
			putenv(optarg);
			நிறுத்து;

		நிலை 'g':  /* --group <group>|<gid> */
			எனில் (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			இல்லையெனில்
				gr = getgrgid((gid_t)tid);
			எனில் (gr == வெற்று)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			நிறுத்து;

		நிலை 'G': /* -G */
			stopgroup = true;
			நிறுத்து;

		நிலை 'k':
			எனில் (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			நிறுத்து;

		நிலை 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			எனில் (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			நிறுத்து;

		நிலை 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			நிறுத்து;

		நிலை 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			நிறுத்து;
		நிலை 'r':  /* --chroot /new/root */
			ch_வேர் = optarg;
			நிறுத்து;

		நிலை 'u':  /* --user <username>|<uid> */
		{
			எழுத்து dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			எனில் (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			இல்லையெனில்
				pw = getpwuid((uid_t)tid);

			எனில் (pw == வெற்று)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			எனில் (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			எனில் (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			எனில் (gid == 0)
				gid = pw->pw_gid;

			எனில் (p) {
				tmp = strsep (&p, ":");
				எனில் (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				இல்லையெனில்
					gr = getgrgid((gid_t) tid);

				எனில் (gr == வெற்று)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		நிறுத்து;

		நிலை '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			நிறுத்து;

		நிலை '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			நிறுத்து;

		நிலை '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			நிறுத்து;
		நிலை '3':  /* --reexec */
			reexec = true;
			நிறுத்து;

		நிலை LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			நிறுத்து;

		நிலை LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			நிறுத்து;

		நிலை LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			நிறுத்து;

		நிலை LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			எனில் (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			நிறுத்து;

		நிலை LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			எனில் (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	எனில் (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	எனில் (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	எனில் (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	எனில் (ch_வேர் && *ch_வேர் == '~')
		ch_வேர் = expand_home(home, ch_வேர்);

	umask(numask);
	எனில் (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	எனில் (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	எனில் (reexec) {
		str = rc_service_மதிப்பு_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * அளவு(எழுத்து *));
		memset(child_argv, 0, (child_argc + 1) * அளவு(எழுத்து *));
		க்கு (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_மதிப்பு_get(svcname, varbuf);
			child_argv[x] = str;
			விடுவி(varbuf);
			varbuf = வெற்று;
		}
		விடுவி(str);
		str = rc_service_மதிப்பு_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		விடுவி(str);
		exec = rc_service_மதிப்பு_get(svcname, "exec");
		pidfile = rc_service_மதிப்பு_get(svcname, "pidfile");
		retry = rc_service_மதிப்பு_get(svcname, "retry");
		எனில் (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_மதிப்பு_set(svcname, "retry", retry);
		} இல்லையெனில்
			parse_schedule(applet, வெற்று, sig);

		str = rc_service_மதிப்பு_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_மதிப்பு_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_மதிப்பு_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_மதிப்பு_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} இல்லையெனில் எனில் (start) {
		எனில் (exec) {
			எனில் (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			எனில் (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				எனில் (ch_வேர்)
					xasprintf(&exec_file, "%s/%s", ch_வேர், exec);
				இல்லையெனில்
					xasprintf(&exec_file, "%s", exec);
			} இல்லையெனில் {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = வெற்று;
				போது ((token = strsep(&p, ":"))) {
					எனில் (ch_வேர்)
						xasprintf(&exec_file, "%s/%s/%s", ch_வேர், token, exec);
					இல்லையெனில்
						xasprintf(&exec_file, "%s/%s", token, exec);
					எனில் (exec_file && access(exec_file, F_OK) == 0)
						நிறுத்து;
					விடுவி(exec_file);
					exec_file = வெற்று;
				}
				விடுவி(tmp);
			}
			எனில் (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				விடுவி(exec_file);
				exit(EXIT_FAILURE);
			}
		} இல்லையெனில்
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		எனில் (pid != -1)
			எனில் (do_stop(applet, exec, (மாறிலி எழுத்து * மாறிலி *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		எனில் (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		எனில் (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		எனில் (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_மதிப்பு_set(svcname, "retry", retry);
		} இல்லையெனில்
			parse_schedule(applet, வெற்று, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		விடுவி(cmdline);
		cmdline = வெற்று;

		/* Remove existing pidfile */
		எனில் (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		எனில் (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_மதிப்பு_set(svcname, "pidfile", pidfile);
		rc_service_மதிப்பு_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_மதிப்பு_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, அளவு(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_மதிப்பு_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, அளவு(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_மதிப்பு_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, அளவு(numbuf), "%i", respawn_max);
		rc_service_மதிப்பு_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		எனில் (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		எனில் (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		எனில் (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		எனில் (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		இல்லையெனில் எனில் (child_pid != 0) {
			எனில் (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			போது (c && *c) {
				varbuf = வெற்று;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_மதிப்பு_set(svcname, varbuf, *c);
				விடுவி(varbuf);
				varbuf = வெற்று;
				x++;
				c++;
			}
			snprintf(numbuf, அளவு(numbuf), "%d", x);
			rc_service_மதிப்பு_set(svcname, "argc", numbuf);
			rc_service_மதிப்பு_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} இல்லையெனில்
			child_process(exec, argv);
	} இல்லையெனில் எனில் (stop) {
		pid = get_pid(applet, pidfile);
		எனில் (pid != -1) {
			i = kill(pid, SIGTERM);
			எனில் (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			இல்லையெனில் {
				/* wait for the supervisor to go down */
				போது (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, வெற்று);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		எனில் (pidfile)
			unlink(pidfile);
		எனில் (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (மாறிலி எழுத்து *மாறிலி *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} இல்லையெனில் எனில் (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		எனில் (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = லிக(fifo_fd, str, strlen(str));
		எனில் (x == -1) {
			விடுவி(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		விடுவி(tmp);
		exit(EXIT_SUCCESS);
	}
}
