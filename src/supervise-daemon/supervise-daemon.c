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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <grp.h>
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <sys/resource.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>

#ifdef HAVE_PAM
#અન્તર્ભાવ <security/pam_appl.h>

/* We are not supporting authentication conversations */
સ્થિર સંરચના pam_conv conv = { ખાલી, ખાલી};
#endif

#ifdef __linux__
#અન્તર્ભાવ <sys/capability.h>
#endif

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "plugin.h"
#અન્તર્ભાવ "schedules.h"
#અન્તર્ભાવ "timeutils.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ગણના {
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

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "healthcheck-timer",        1, ખાલી, 'a'},
	{ "healthcheck-delay",        1, ખાલી, 'A'},
	{ "capabilities", 1, ખાલી, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ખાલી, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ખાલી, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ખાલી, 'D'},
	{ "respawn-delay-step",   1, ખાલી, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, ખાલી, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ખાલી, 'd'},
	{ "env",          1, ખાલી, 'e'},
	{ "group",        1, ખાલી, 'g'},
	{ "stop-group",   0, ખાલી, 'G'},
	{ "ionice",       1, ખાલી, 'I'},
	{ "stop",         0, ખાલી, 'K'},
	{ "umask",        1, ખાલી, 'k'},
	{ "respawn-max",    1, ખાલી, 'm'},
	{ "nicelevel",    1, ખાલી, 'N'},
	{ "oom-score-adj",1, ખાલી, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ખાલી, 'p'},
	{ "respawn-period",        1, ખાલી, 'P'},
	{ "retry",       1, ખાલી, 'R'},
	{ "chroot",       1, ખાલી, 'r'},
	{ "signal",       1, ખાલી, 's'},
	{ "start",        0, ખાલી, 'S'},
	{ "user",         1, ખાલી, 'u'},
	{ "stdin",        1, ખાલી, '0'},
	{ "stdout",       1, ખાલી, '1'},
	{ "stderr",       1, ખાલી, '2'},
	{ "stdout-logger",1, ખાલી, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ખાલી, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ખાલી, '3'},
	{ "notify",       1, ખાલી, LONGOPT_NOTIFY},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ખાલી;

સ્થિર પૂર્ણાંક healthcheckdelay = 0;
સ્થિર પૂર્ણાંક healthchecktimer = 0;
સ્થિર અસ્થિર sig_atomic_t do_healthcheck = 0;
સ્થિર અસ્થિર sig_atomic_t exiting = 0;
સ્થિર પૂર્ણાંક nicelevel = INT_MIN;
સ્થિર પૂર્ણાંક ionicec = -1;
સ્થિર પૂર્ણાંક ioniced = 0;
સ્થિર પૂર્ણાંક oom_score_adj = INT_MIN;
સ્થિર અક્ષર *changeuser, *ch_મૂળ, *ch_dir;
સ્થિર uid_t uid = 0;
સ્થિર gid_t gid = 0;
સ્થિર bool stopgroup = false;
સ્થિર પૂર્ણાંક devnull_fd = -1;
સ્થિર પૂર્ણાંક stdin_fd;
સ્થિર પૂર્ણાંક stdout_fd;
સ્થિર પૂર્ણાંક stderr_fd;
સ્થિર સંરચના notify notify;
સ્થિર અક્ષર *redirect_stdin = ખાલી;
સ્થિર અક્ષર *redirect_stderr = ખાલી;
સ્થિર અક્ષર *redirect_stdout = ખાલી;
સ્થિર અક્ષર *stderr_process = ખાલી;
સ્થિર અક્ષર *stdout_process = ખાલી;
#ifdef TIOCNOTTY
સ્થિર પૂર્ણાંક tty_fd = -1;
#endif
સ્થિર pid_t child_pid;
સ્થિર પૂર્ણાંક respawn_ગણતરી = 0;
સ્થિર int64_t respawn_delay; /* default set inside main() */
સ્થિર int64_t respawn_delay_step = TM_MS(128);
સ્થિર int64_t respawn_delay_cap = TM_SEC(30);
સ્થિર int64_t respawn_period; /* default set inside main() */
સ્થિર પૂર્ણાંક respawn_max = 10;
સ્થિર અક્ષર *fifopath = ખાલી;
સ્થિર પૂર્ણાંક fifo_fd = 0;
સ્થિર અક્ષર *pidfile = ખાલી;
સ્થિર અક્ષર *svcname = ખાલી;
સ્થિર bool verbose = false;
#ifdef __linux__
સ્થિર cap_iab_t cap_iab = ખાલી;
સ્થિર unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
સ્થિર bool no_new_privs = false;
#endif

બાહ્ય અક્ષર **environ;

#જો !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#જો !defined(__DragonFly__)
સ્થિર અંતઃરેખ પૂર્ણાંક ioprio_set(પૂર્ણાંક which RC_UNUSED, પૂર્ણાંક who RC_UNUSED,
			     પૂર્ણાંક ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	પાછા syscall(SYS_ioprio_set, which, who, ioprio);
#નહીંતર
	પાછા 0;
#endif
}
#endif

સ્થિર શૂન્ય cleanup(શૂન્ય)
{
	મુક્ત(changeuser);
}

RC_NORETURN સ્થિર શૂન્ય re_exec_supervisor(શૂન્ય)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(અક્ષર *) ખાલી);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

સ્થિર શૂન્ય handle_signal(પૂર્ણાંક sig)
{
	પૂર્ણાંક serrno = errno;
	pid_t pid;

	પસંદગી (sig) {
	સ્થિતિ SIGALRM:
		do_healthcheck = 1;
		તોડો;
	સ્થિતિ SIGCHLD:
		જો (exiting)
			જ્યારે (waitpid((pid_t)(-1), ખાલી, WNOHANG) > 0) {}
		નહીંતર {
			જ્યારે ((pid = waitpid((pid_t)(-1), ખાલી, WNOHANG|WNOWAIT)) > 0) {
				જો (pid == child_pid)
					તોડો;
				pid = waitpid(pid, ખાલી, WNOHANG);
			}
		}
		તોડો;
	સ્થિતિ SIGTERM:
		exiting = 1;
		તોડો;
	મૂળભૂત:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

સ્થિર અક્ષર * expand_home(અચલ અક્ષર *home, અચલ અક્ષર *path)
{
	અક્ષર *opath, *ppath, *p, *nh;
	સંરચના passwd *pw;

	જો (!path || *path != '~')
		પાછા xstrdup(path);

	opath = ppath = xstrdup(path);
	જો (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		જો (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		જો (pw) {
			home = pw->pw_dir;
			ppath = p;
			જો (ppath)
				*ppath = '/';
		} નહીંતર
			home = ખાલી;
	} નહીંતર
		ppath++;

	જો (!home) {
		મુક્ત(opath);
		પાછા xstrdup(path);
	}
	જો (!ppath) {
		મુક્ત(opath);
		પાછા xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	મુક્ત(opath);
	પાછા nh;
}

સ્થિર અક્ષર *make_cmdline(અક્ષર **argv)
{
	અક્ષર **c;
	અક્ષર *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	માટે (c = argv; c && *c; c++) {
		fputs(*c, mem);
		જો (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	પાછા cmdline;
}

સ્થિર pid_t exec_command(અચલ અક્ષર *cmd)
{
	અક્ષર *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	સંરચના sigaction sa;

	file = rc_service_resolve(svcname);
	જો (access(file, F_OK) != 0) {
		મુક્ત(file);
		પાછા 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, કદ (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	જો (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ખાલી);
		sigaction(SIGHUP, &sa, ખાલી);
		sigaction(SIGINT, &sa, ખાલી);
		sigaction(SIGQUIT, &sa, ખાલી);
		sigaction(SIGTERM, &sa, ખાલી);
		sigaction(SIGUSR1, &sa, ખાલી);
		sigaction(SIGWINCH, &sa, ખાલી);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ખાલી);

		/* Safe to run now */
		execl(file, file, cmd, (અક્ષર *) ખાલી);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	જો (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ખાલી);
	મુક્ત(file);
	પાછા pid;
}

RC_NORETURN સ્થિર શૂન્ય child_process(અક્ષર *exec, અક્ષર **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	પૂર્ણાંક i;
	અક્ષર *p;
	અક્ષર *token;
	size_t len;
	અક્ષર *newpath;
	અક્ષર *np;
	અક્ષર *cmdline = ખાલી;
	time_t start_time;
	અક્ષર start_ગણતરી_string[20];
	અક્ષર start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	પૂર્ણાંક group_ગણતરી = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = ખાલી;
	પૂર્ણાંક pamr;
	અચલ અક્ષર *અચલ *pamenv = ખાલી;
#endif

	setsid();

	જો (svcname) {
		start_time = time(ખાલી);
		from_time_t(start_time_string, start_time);
		rc_service_કિંમત_set(svcname, "start_time", start_time_string);
		sprintf(start_ગણતરી_string, "%i", respawn_ગણતરી);
		rc_service_કિંમત_set(svcname, "start_count", start_ગણતરી_string);
		sprintf(start_ગણતરી_string, "%d", getpid());
		rc_service_કિંમત_set(svcname, "child_pid", start_ગણતરી_string);
	}

	જો (nicelevel != INT_MIN) {
		જો (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		જો (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} નહીંતર જો (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	જો (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	જો (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		જો (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	જો (changeuser != ખાલી) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		જો (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		જો (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		જો (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	જો (changeuser && getgrouplist(changeuser, gid, group_list, &group_ગણતરી) < 0) {
		group_list = xmalloc(group_ગણતરી * કદ(*group_list));
		જો (getgrouplist(changeuser, gid, group_list, &group_ગણતરી) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	જો (ch_મૂળ && chroot(ch_મૂળ) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_મૂળ, strerror(errno));

	જો (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	જો (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	જો (changeuser && setgroups(group_ગણતરી, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	જો (group_list != group_buf)
		મુક્ત(group_list);
#ifdef __linux__
	જો (uid && cap_setuid(uid))
#નહીંતર
	જો (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	જો (cap_iab != ખાલી) {
		i = cap_iab_set_proc(cap_iab);

		જો (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		જો (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	જો (secbits != 0) {
		જો (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	જો (no_new_privs) {
		જો (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	જ્યારે (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	જો (changeuser != ખાલી) {
		pamenv = (અચલ અક્ષર *અચલ *)pam_getenvlist(pamh);
		જો (pamenv) {
			જ્યારે (*pamenv) {
				/* Don't add strings unless they set a var */
				જો (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				નહીંતર
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		જો (strncmp(env->કિંમત, "RC_", 3) == 0
				|| strncmp(env->કિંમત, "SSD_", 4) == 0
				|| strncmp(env->કિંમત, "EINFO_", 6) == 0) {
			*strchr(env->કિંમત, '=') = '\0';
			unsetenv(env->કિંમત);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	જો ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		જ્યારે (token && *token) {
			p = strchr(token, ':');
			જો (p) {
				*p++ = '\0';
				જ્યારે (*p == ':')
					p++;
			}
			જો (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				જો (np != newpath)
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
	જો (redirect_stdin) {
		જો ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	જો (redirect_stdout) {
		જો ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} નહીંતર જો (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		જો (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	જો (redirect_stderr) {
		જો ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} નહીંતર જો (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		જો (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	જો (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	જો (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	જો (notify.type == NOTIFY_FD) {
		પૂર્ણાંક error;

		જો (notify.fd == notify.pipe[1]) {
			પૂર્ણાંક flags = error = fcntl(notify.fd, F_GETFD, 0);
			જો (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} નહીંતર {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		જો (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	મુક્ત(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	જો (changeuser != ખાલી && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN સ્થિર શૂન્ય supervisor(અક્ષર *exec, અક્ષર **argv)
{
	FILE *fp;
	અક્ષર buf[2048];
	અક્ષર cmd[2048];
	પૂર્ણાંક ગણતરી;
	પૂર્ણાંક failing;
	પૂર્ણાંક health_status;
	પૂર્ણાંક healthcheck_respawn;
	પૂર્ણાંક i;
	પૂર્ણાંક nkilled;
	પૂર્ણાંક sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	સંરચના sigaction sa;
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
	memset(&sa, 0, કદ(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ખાલી);
	sigaction(SIGCHLD, &sa, ખાલી);
	sigaction(SIGTERM, &sa, ખાલી);

	fp = fopen(pidfile, "w");
	જો (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	જો (svcname)
		rc_service_daemon_set(svcname, exec, (અચલ અક્ષર * અચલ *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	જો (healthcheckdelay)
		alarm(healthcheckdelay);
	નહીંતર જો (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	જ્યારે (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		જો (fifo_fd > 0) {
			memset(buf, 0, કદ(buf));
			ગણતરી = પઠ(fifo_fd, buf, કદ(buf) - 1);
			close(fifo_fd);
			જો (ગણતરી != -1)
				buf[ગણતરી] = 0;
			જો (ગણતરી == 0)
				ચાલુ;
			જો (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			જો (strncasecmp(buf, "sig", 3) == 0) {
				જો ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					જો (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			ચાલુ;
		}
		જો (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			જો (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			જો (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			નહીંતર {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, ખાલી, ખાલી, child_pid, 0,
						stopgroup, false, false, true);
				જો (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				નહીંતર
					healthcheck_respawn = 1;
			}
		}
		જો (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, ખાલી, ખાલી, child_pid, 0,
					stopgroup, false, false, true);
			જો (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			ચાલુ;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		જો (wait_pid == child_pid) {
			જો (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			નહીંતર જો (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		જો (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			જો (first_spawn == 0)
				first_spawn = respawn_now;
			જો ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_ગણતરી = 0;
				first_spawn = 0;
			} નહીંતર
				respawn_ગણતરી++;
			જો (respawn_max > 0 && respawn_ગણતરી > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				ચાલુ;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_ગણતરી);
			જો (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			જો (exiting)
				ચાલુ;
			child_pid = fork();
			જો (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			જો (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, ખાલી);
				memset(&sa, 0, કદ(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ખાલી);
				sigaction(SIGCHLD, &sa, ખાલી);
				sigaction(SIGTERM, &sa, ખાલી);
				child_process(exec, argv);
			}
			જો (healthcheckdelay)
				alarm(healthcheckdelay);
			નહીંતર જો (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	જો (svcname) {
		rc_service_daemon_set(svcname, exec, (અચલ અક્ષર *અચલ *)argv,
				pidfile, false);
		rc_service_કિંમત_set(svcname, "child_pid", ખાલી);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		જો (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	જો (pidfile)
		unlink(pidfile);
	જો (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક opt;
	અક્ષર **c;
	પૂર્ણાંક x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	અક્ષર *exec = ખાલી;
	અક્ષર *retry = ખાલી;
	પૂર્ણાંક sig = SIGTERM;
	અક્ષર *home = ખાલી;
	પૂર્ણાંક tid = 0;
	pid_t pid;
	અક્ષર *tmp;
	અક્ષર *p;
	અક્ષર *token;
	પૂર્ણાંક i;
	પૂર્ણાંક n;
	અક્ષર *exec_file = ખાલી;
	અક્ષર *varbuf = ખાલી;
	અક્ષર numbuf[64];
	સંરચના timespec ts;
	સંરચના passwd *pw;
	સંરચના group *gr;
	FILE *fp;
	mode_t numask = 022;
	પૂર્ણાંક child_argc = 0;
	અક્ષર **child_argv = ખાલી;
	અક્ષર *str = ખાલી;
	અક્ષર *cmdline = ખાલી;
	અચલ અક્ષર *respawn_delay_str  = "0";
	અચલ અક્ષર *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	જો (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	જો (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	જો ((tmp = getenv("SSD_NICELEVEL")))
		જો (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	જો ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		જો (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		જો (ionicec == 0)
			ioniced = 0;
		નહીંતર જો (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	જો ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		જો (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	જો (home == ખાલી || p == ખાલી) {
		pw = getpwuid(getuid());
		જો (pw != ખાલી) {
			જો (p == ખાલી)
				setenv("USER", pw->pw_name, 1);
			જો (home == ખાલી) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	જો (svcname) {
		argc--;
		argv++;
	}
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (પૂર્ણાંક *) 0)) != -1)
		પસંદગી (opt) {
		સ્થિતિ 'a':  /* --healthcheck-timer <time> */
			જો (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			તોડો;

		સ્થિતિ 'A':  /* --healthcheck-delay <time> */
			જો (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			તોડો;

		સ્થિતિ LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			જો (cap_iab == ખાલી)
				eerrorx("Could not parse iab: %s", strerror(errno));
#નહીંતર
			eerrorx("Capabilities support not enabled");
#endif
			તોડો;

		સ્થિતિ LONGOPT_SECBITS:
#ifdef __linux__
			જો (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ખાલી;
			secbits = strtoul(optarg, &tmp, 0);
			જો (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#નહીંતર
			eerrorx("Capabilities support not enabled");
#endif
			તોડો;

		સ્થિતિ LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#નહીંતર
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			તોડો;

		સ્થિતિ 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			તોડો;

		સ્થિતિ 'I': /* --ionice */
			જો (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			જો (ionicec == 0)
				ioniced = 0;
			નહીંતર જો (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			તોડો;

		સ્થિતિ 'K':  /* --stop */
			stop = true;
			તોડો;

		સ્થિતિ 'N':  /* --nice */
			જો (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			તોડો;

		સ્થિતિ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			જો (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			તોડો;

		સ્થિતિ 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			તોડો;

		સ્થિતિ 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			તોડો;
		સ્થિતિ 'S':  /* --start */
			start = true;
			તોડો;

		સ્થિતિ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			તોડો;

		સ્થિતિ 'e': /* --env */
			putenv(optarg);
			તોડો;

		સ્થિતિ 'g':  /* --group <group>|<gid> */
			જો (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			નહીંતર
				gr = getgrgid((gid_t)tid);
			જો (gr == ખાલી)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			તોડો;

		સ્થિતિ 'G': /* -G */
			stopgroup = true;
			તોડો;

		સ્થિતિ 'k':
			જો (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			તોડો;

		સ્થિતિ 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			જો (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			તોડો;

		સ્થિતિ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			તોડો;

		સ્થિતિ 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			તોડો;
		સ્થિતિ 'r':  /* --chroot /new/root */
			ch_મૂળ = optarg;
			તોડો;

		સ્થિતિ 'u':  /* --user <username>|<uid> */
		{
			અક્ષર dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			જો (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			નહીંતર
				pw = getpwuid((uid_t)tid);

			જો (pw == ખાલી)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			જો (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			જો (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			જો (gid == 0)
				gid = pw->pw_gid;

			જો (p) {
				tmp = strsep (&p, ":");
				જો (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				નહીંતર
					gr = getgrgid((gid_t) tid);

				જો (gr == ખાલી)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		તોડો;

		સ્થિતિ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			તોડો;

		સ્થિતિ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			તોડો;

		સ્થિતિ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			તોડો;
		સ્થિતિ '3':  /* --reexec */
			reexec = true;
			તોડો;

		સ્થિતિ LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			તોડો;

		સ્થિતિ LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			તોડો;

		સ્થિતિ LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			તોડો;

		સ્થિતિ LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			જો (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			તોડો;

		સ્થિતિ LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			જો (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			તોડો;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	જો (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	જો (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	જો (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	જો (ch_મૂળ && *ch_મૂળ == '~')
		ch_મૂળ = expand_home(home, ch_મૂળ);

	umask(numask);
	જો (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	જો (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	જો (reexec) {
		str = rc_service_કિંમત_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * કદ(અક્ષર *));
		memset(child_argv, 0, (child_argc + 1) * કદ(અક્ષર *));
		માટે (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_કિંમત_get(svcname, varbuf);
			child_argv[x] = str;
			મુક્ત(varbuf);
			varbuf = ખાલી;
		}
		મુક્ત(str);
		str = rc_service_કિંમત_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		મુક્ત(str);
		exec = rc_service_કિંમત_get(svcname, "exec");
		pidfile = rc_service_કિંમત_get(svcname, "pidfile");
		retry = rc_service_કિંમત_get(svcname, "retry");
		જો (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_કિંમત_set(svcname, "retry", retry);
		} નહીંતર
			parse_schedule(applet, ખાલી, sig);

		str = rc_service_કિંમત_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_કિંમત_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_કિંમત_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_કિંમત_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} નહીંતર જો (start) {
		જો (exec) {
			જો (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			જો (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				જો (ch_મૂળ)
					xasprintf(&exec_file, "%s/%s", ch_મૂળ, exec);
				નહીંતર
					xasprintf(&exec_file, "%s", exec);
			} નહીંતર {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = ખાલી;
				જ્યારે ((token = strsep(&p, ":"))) {
					જો (ch_મૂળ)
						xasprintf(&exec_file, "%s/%s/%s", ch_મૂળ, token, exec);
					નહીંતર
						xasprintf(&exec_file, "%s/%s", token, exec);
					જો (exec_file && access(exec_file, F_OK) == 0)
						તોડો;
					મુક્ત(exec_file);
					exec_file = ખાલી;
				}
				મુક્ત(tmp);
			}
			જો (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				મુક્ત(exec_file);
				exit(EXIT_FAILURE);
			}
		} નહીંતર
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		જો (pid != -1)
			જો (do_stop(applet, exec, (અચલ અક્ષર * અચલ *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		જો (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		જો (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		જો (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_કિંમત_set(svcname, "retry", retry);
		} નહીંતર
			parse_schedule(applet, ખાલી, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		મુક્ત(cmdline);
		cmdline = ખાલી;

		/* Remove existing pidfile */
		જો (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		જો (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_કિંમત_set(svcname, "pidfile", pidfile);
		rc_service_કિંમત_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_કિંમત_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, કદ(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_કિંમત_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, કદ(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_કિંમત_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, કદ(numbuf), "%i", respawn_max);
		rc_service_કિંમત_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		જો (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		જો (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		જો (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		જો (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		નહીંતર જો (child_pid != 0) {
			જો (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			જ્યારે (c && *c) {
				varbuf = ખાલી;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_કિંમત_set(svcname, varbuf, *c);
				મુક્ત(varbuf);
				varbuf = ખાલી;
				x++;
				c++;
			}
			snprintf(numbuf, કદ(numbuf), "%d", x);
			rc_service_કિંમત_set(svcname, "argc", numbuf);
			rc_service_કિંમત_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} નહીંતર
			child_process(exec, argv);
	} નહીંતર જો (stop) {
		pid = get_pid(applet, pidfile);
		જો (pid != -1) {
			i = kill(pid, SIGTERM);
			જો (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			નહીંતર {
				/* wait for the supervisor to go down */
				જ્યારે (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ખાલી);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		જો (pidfile)
			unlink(pidfile);
		જો (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (અચલ અક્ષર *અચલ *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} નહીંતર જો (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		જો (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = લિખ(fifo_fd, str, strlen(str));
		જો (x == -1) {
			મુક્ત(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		મુક્ત(tmp);
		exit(EXIT_SUCCESS);
	}
}
