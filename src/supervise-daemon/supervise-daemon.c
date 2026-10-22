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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <grp.h>
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <strings.h>
#నిర్వచితమైతే __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#అయితే_ముగింపు
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/resource.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>

#నిర్వచితమైతే HAVE_PAM
#అన్తర్భావ <security/pam_appl.h>

/* We are not supporting authentication conversations */
స్థిర నిర్మాణం pam_conv conv = { ఖాళీ, ఖాళీ};
#అయితే_ముగింపు

#నిర్వచితమైతే __linux__
#అన్తర్భావ <sys/capability.h>
#అయితే_ముగింపు

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "plugin.h"
#అన్తర్భావ "schedules.h"
#అన్తర్భావ "timeutils.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
గణన {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_పునాది = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_అడుగు,
  LONGOPT_RESPAWN_DELAY_CAP,
};

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "healthcheck-timer",        1, ఖాళీ, 'a'},
	{ "healthcheck-delay",        1, ఖాళీ, 'A'},
	{ "capabilities", 1, ఖాళీ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ఖాళీ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ఖాళీ, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ఖాళీ, 'D'},
	{ "respawn-delay-step",   1, ఖాళీ, LONGOPT_RESPAWN_DELAY_అడుగు},
	{ "respawn-delay-cap",    1, ఖాళీ, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ఖాళీ, 'd'},
	{ "env",          1, ఖాళీ, 'e'},
	{ "group",        1, ఖాళీ, 'g'},
	{ "stop-group",   0, ఖాళీ, 'G'},
	{ "ionice",       1, ఖాళీ, 'I'},
	{ "stop",         0, ఖాళీ, 'K'},
	{ "umask",        1, ఖాళీ, 'k'},
	{ "respawn-max",    1, ఖాళీ, 'm'},
	{ "nicelevel",    1, ఖాళీ, 'N'},
	{ "oom-score-adj",1, ఖాళీ, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ఖాళీ, 'p'},
	{ "respawn-period",        1, ఖాళీ, 'P'},
	{ "retry",       1, ఖాళీ, 'R'},
	{ "chroot",       1, ఖాళీ, 'r'},
	{ "signal",       1, ఖాళీ, 's'},
	{ "start",        0, ఖాళీ, 'S'},
	{ "user",         1, ఖాళీ, 'u'},
	{ "stdin",        1, ఖాళీ, '0'},
	{ "stdout",       1, ఖాళీ, '1'},
	{ "stderr",       1, ఖాళీ, '2'},
	{ "stdout-logger",1, ఖాళీ, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ఖాళీ, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ఖాళీ, '3'},
	{ "notify",       1, ఖాళీ, LONGOPT_NOTIFY},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

స్థిర పూర్ణాంకం healthcheckdelay = 0;
స్థిర పూర్ణాంకం healthchecktimer = 0;
స్థిర అస్థిర sig_atomic_t do_healthcheck = 0;
స్థిర అస్థిర sig_atomic_t exiting = 0;
స్థిర పూర్ణాంకం nicelevel = INT_MIN;
స్థిర పూర్ణాంకం ionicec = -1;
స్థిర పూర్ణాంకం ioniced = 0;
స్థిర పూర్ణాంకం oom_score_adj = INT_MIN;
స్థిర అక్షరం *changeuser, *ch_మూలం, *ch_dir;
స్థిర uid_t uid = 0;
స్థిర gid_t gid = 0;
స్థిర bool stopgroup = false;
స్థిర పూర్ణాంకం devnull_fd = -1;
స్థిర పూర్ణాంకం stdin_fd;
స్థిర పూర్ణాంకం stdout_fd;
స్థిర పూర్ణాంకం stderr_fd;
స్థిర నిర్మాణం notify notify;
స్థిర అక్షరం *redirect_stdin = ఖాళీ;
స్థిర అక్షరం *redirect_stderr = ఖాళీ;
స్థిర అక్షరం *redirect_stdout = ఖాళీ;
స్థిర అక్షరం *stderr_process = ఖాళీ;
స్థిర అక్షరం *stdout_process = ఖాళీ;
#నిర్వచితమైతే TIOCNOTTY
స్థిర పూర్ణాంకం tty_fd = -1;
#అయితే_ముగింపు
స్థిర pid_t బిడ్డ_pid;
స్థిర పూర్ణాంకం respawn_లెక్క = 0;
స్థిర int64_t respawn_delay; /* default set inside main() */
స్థిర int64_t respawn_delay_అడుగు = TM_MS(128);
స్థిర int64_t respawn_delay_cap = TM_SEC(30);
స్థిర int64_t respawn_period; /* default set inside main() */
స్థిర పూర్ణాంకం respawn_max = 10;
స్థిర అక్షరం *fifopath = ఖాళీ;
స్థిర పూర్ణాంకం fifo_fd = 0;
స్థిర అక్షరం *pidfile = ఖాళీ;
స్థిర అక్షరం *svcname = ఖాళీ;
స్థిర bool verbose = false;
#నిర్వచితమైతే __linux__
స్థిర cap_iab_t cap_iab = ఖాళీ;
స్థిర గుర్తులేని secbits = 0;
#అయితే_ముగింపు
#నిర్వచితమైతే PR_సమితి_NO_NEW_PRIVS
స్థిర bool no_new_privs = false;
#అయితే_ముగింపు

బాహ్య అక్షరం **environ;

#ఒకవేళ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#అయితే_ముగింపు
#ఒకవేళ !defined(__DragonFly__)
స్థిర అంతఃవరుస పూర్ణాంకం ioprio_సమితి(పూర్ణాంకం which RC_వాడని, పూర్ణాంకం who RC_వాడని,
			     పూర్ణాంకం ioprio RC_వాడని)
{
#నిర్వచితమైతే SYS_ioprio_set
	తిరిగి syscall(SYS_ioprio_set, which, who, ioprio);
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు
}
#అయితే_ముగింపు

స్థిర శూన్య cleanup(శూన్య)
{
	విడుదల(changeuser);
}

RC_NORETURN స్థిర శూన్య re_exec_supervisor(శూన్య)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(అక్షరం *) ఖాళీ);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	నిష్క్రమించు(EXIT_FAILURE);
}

స్థిర శూన్య handle_signal(పూర్ణాంకం sig)
{
	పూర్ణాంకం serrno = errno;
	pid_t pid;

	ఎంపిక (sig) {
	స్థితి SIGALRM:
		do_healthcheck = 1;
		ఆపు;
	స్థితి SIGCHLD:
		ఒకవేళ (exiting)
			అయితే (waitpid((pid_t)(-1), ఖాళీ, WNOHANG) > 0) {}
		లేకపోతే {
			అయితే ((pid = waitpid((pid_t)(-1), ఖాళీ, WNOHANG|WNOWAIT)) > 0) {
				ఒకవేళ (pid == బిడ్డ_pid)
					ఆపు;
				pid = waitpid(pid, ఖాళీ, WNOHANG);
			}
		}
		ఆపు;
	స్థితి SIGTERM:
		exiting = 1;
		ఆపు;
	అప్రమేయ:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

స్థిర అక్షరం * expand_home(స్థిరరాశి అక్షరం *home, స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం *opath, *ppath, *p, *nh;
	నిర్మాణం passwd *pw;

	ఒకవేళ (!మార్గం || *మార్గం != '~')
		తిరిగి xstrdup(మార్గం);

	opath = ppath = xstrdup(మార్గం);
	ఒకవేళ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ఒకవేళ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ఒకవేళ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ఒకవేళ (ppath)
				*ppath = '/';
		} లేకపోతే
			home = ఖాళీ;
	} లేకపోతే
		ppath++;

	ఒకవేళ (!home) {
		విడుదల(opath);
		తిరిగి xstrdup(మార్గం);
	}
	ఒకవేళ (!ppath) {
		విడుదల(opath);
		తిరిగి xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	విడుదల(opath);
	తిరిగి nh;
}

స్థిర అక్షరం *make_cmdline(అక్షరం **argv)
{
	అక్షరం **c;
	అక్షరం *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	కొరకు (c = argv; c && *c; c++) {
		fputs(*c, mem);
		ఒకవేళ (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	తిరిగి cmdline;
}

స్థిర pid_t exec_command(స్థిరరాశి అక్షరం *cmd)
{
	అక్షరం *file;
	pid_t pid = -1;
	sigset_t నిండిన;
	sigset_t old;
	నిర్మాణం sigaction sa;

	file = rc_service_resolve(svcname);
	ఒకవేళ (access(file, F_OK) != 0) {
		విడుదల(file);
		తిరిగి 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, పరిమాణం (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&నిండిన);
	sigprocmask(SIG_SETMASK, &నిండిన, &old);

	pid = fork();
	ఒకవేళ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ఖాళీ);
		sigaction(SIGHUP, &sa, ఖాళీ);
		sigaction(SIGINT, &sa, ఖాళీ);
		sigaction(SIGQUIT, &sa, ఖాళీ);
		sigaction(SIGTERM, &sa, ఖాళీ);
		sigaction(SIGUSR1, &sa, ఖాళీ);
		sigaction(SIGWINCH, &sa, ఖాళీ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ఖాళీ);

		/* Safe to run now */
		execl(file, file, cmd, (అక్షరం *) ఖాళీ);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ఒకవేళ (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ఖాళీ);
	విడుదల(file);
	తిరిగి pid;
}

RC_NORETURN స్థిర శూన్య బిడ్డ_process(అక్షరం *exec, అక్షరం **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	పూర్ణాంకం i;
	అక్షరం *p;
	అక్షరం *token;
	size_t len;
	అక్షరం *newpath;
	అక్షరం *np;
	అక్షరం *cmdline = ఖాళీ;
	time_t ప్రారంభం_సమయం;
	అక్షరం ప్రారంభం_లెక్క_string[20];
	అక్షరం ప్రారంభం_సమయం_string[20];
	FILE *fp;
	gid_t సమూహం_buf[32], *సమూహం_list = సమూహం_buf;
	పూర్ణాంకం సమూహం_లెక్క = ARRAY_కొలత(సమూహం_buf);

#నిర్వచితమైతే HAVE_PAM
	pam_handle_t *pamh = ఖాళీ;
	పూర్ణాంకం pamr;
	స్థిరరాశి అక్షరం *స్థిరరాశి *pamenv = ఖాళీ;
#అయితే_ముగింపు

	setsid();

	ఒకవేళ (svcname) {
		ప్రారంభం_సమయం = time(ఖాళీ);
		from_సమయం_t(ప్రారంభం_సమయం_string, ప్రారంభం_సమయం);
		rc_service_విలువ_సమితి(svcname, "start_time", ప్రారంభం_సమయం_string);
		sprintf(ప్రారంభం_లెక్క_string, "%i", respawn_లెక్క);
		rc_service_విలువ_సమితి(svcname, "start_count", ప్రారంభం_లెక్క_string);
		sprintf(ప్రారంభం_లెక్క_string, "%d", getpid());
		rc_service_విలువ_సమితి(svcname, "child_pid", ప్రారంభం_లెక్క_string);
	}

	ఒకవేళ (nicelevel != INT_MIN) {
		ఒకవేళ (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		ఒకవేళ (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} లేకపోతే ఒకవేళ (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	ఒకవేళ (ionicec != -1 && ioprio_సమితి(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	ఒకవేళ (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		ఒకవేళ (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#నిర్వచితమైతే HAVE_PAM
	ఒకవేళ (changeuser != ఖాళీ) {
		pamr = pam_ప్రారంభం("supervise-daemon",
		    changeuser, &conv, &pamh);

		ఒకవేళ (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		ఒకవేళ (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		ఒకవేళ (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#అయితే_ముగింపు

	ఒకవేళ (changeuser && getgrouplist(changeuser, gid, సమూహం_list, &సమూహం_లెక్క) < 0) {
		సమూహం_list = xmalloc(సమూహం_లెక్క * పరిమాణం(*సమూహం_list));
		ఒకవేళ (getgrouplist(changeuser, gid, సమూహం_list, &సమూహం_లెక్క) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	ఒకవేళ (ch_మూలం && chroot(ch_మూలం) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_మూలం, strerror(errno));

	ఒకవేళ (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	ఒకవేళ (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	ఒకవేళ (changeuser && setgroups(సమూహం_లెక్క, సమూహం_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	ఒకవేళ (సమూహం_list != సమూహం_buf)
		విడుదల(సమూహం_list);
#నిర్వచితమైతే __linux__
	ఒకవేళ (uid && cap_setuid(uid))
#లేకపోతే
	ఒకవేళ (uid && setuid(uid))
#అయితే_ముగింపు
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#నిర్వచితమైతే __linux__
	ఒకవేళ (cap_iab != ఖాళీ) {
		i = cap_iab_సమితి_proc(cap_iab);

		ఒకవేళ (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		ఒకవేళ (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	ఒకవేళ (secbits != 0) {
		ఒకవేళ (cap_సమితి_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#అయితే_ముగింపు

#నిర్వచితమైతే PR_సమితి_NO_NEW_PRIVS
	ఒకవేళ (no_new_privs) {
		ఒకవేళ (prctl(PR_సమితి_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", strerror(errno));
	}
#అయితే_ముగింపు

	/* remove the controlling tty */
#నిర్వచితమైతే TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#అయితే_ముగింపు

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	అయితే (environ[i])
		rc_stringlist_కూడు(env_list, environ[i++]);

#నిర్వచితమైతే HAVE_PAM
	ఒకవేళ (changeuser != ఖాళీ) {
		pamenv = (స్థిరరాశి అక్షరం *స్థిరరాశి *)pam_getenvlist(pamh);
		ఒకవేళ (pamenv) {
			అయితే (*pamenv) {
				/* Don't add strings unless they set a var */
				ఒకవేళ (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				లేకపోతే
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#అయితే_ముగింపు

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(env->విలువ, "RC_", 3) == 0
				|| అక్షరశ్రేణి_పోలిక(env->విలువ, "SSD_", 4) == 0
				|| అక్షరశ్రేణి_పోలిక(env->విలువ, "EINFO_", 6) == 0) {
			*strchr(env->విలువ, '=') = '\0';
			unsetenv(env->విలువ);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	ఒకవేళ ((token = getenv("PATH"))) {
		len = అక్షరశ్రేణి_పొడవు(token);
		newpath = np = xmalloc(len + 1);
		అయితే (token && *token) {
			p = strchr(token, ':');
			ఒకవేళ (p) {
				*p++ = '\0';
				అయితే (*p == ':')
					p++;
			}
			ఒకవేళ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = అక్షరశ్రేణి_పొడవు(token);
				ఒకవేళ (np != newpath)
					*np++ = ':';
				జ్ఞాపకం_నకలు(np, token, len);
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
	ఒకవేళ (redirect_stdin) {
		ఒకవేళ ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	ఒకవేళ (redirect_stdout) {
		ఒకవేళ ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} లేకపోతే ఒకవేళ (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		ఒకవేళ (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	ఒకవేళ (redirect_stderr) {
		ఒకవేళ ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} లేకపోతే ఒకవేళ (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		ఒకవేళ (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	ఒకవేళ (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	ఒకవేళ (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	ఒకవేళ (notify.type == NOTIFY_FD) {
		పూర్ణాంకం error;

		ఒకవేళ (notify.fd == notify.pipe[1]) {
			పూర్ణాంకం flags = error = fcntl(notify.fd, F_GETFD, 0);
			ఒకవేళ (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} లేకపోతే {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		ఒకవేళ (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	విడుదల(cmdline);
	execvp(exec, argv);

#నిర్వచితమైతే HAVE_PAM
	ఒకవేళ (changeuser != ఖాళీ && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#అయితే_ముగింపు
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN స్థిర శూన్య supervisor(అక్షరం *exec, అక్షరం **argv)
{
	FILE *fp;
	అక్షరం buf[2048];
	అక్షరం cmd[2048];
	పూర్ణాంకం లెక్క;
	పూర్ణాంకం failing;
	పూర్ణాంకం health_స్థితి;
	పూర్ణాంకం healthcheck_respawn;
	పూర్ణాంకం i;
	పూర్ణాంకం nkilled;
	పూర్ణాంకం sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_సంకేతాలు;
	sigset_t సంకేతాలు;
	నిర్మాణం sigaction sa;
	int64_t respawn_now = 0;
	int64_t మొదటి_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&సంకేతాలు);
	sigdelset(&సంకేతాలు, SIGALRM);
	sigdelset(&సంకేతాలు, SIGCHLD);
	sigdelset(&సంకేతాలు, SIGTERM);
	sigprocmask(SIG_SETMASK, &సంకేతాలు, &old_సంకేతాలు);

	/* install signal  handler */
	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ఖాళీ);
	sigaction(SIGCHLD, &sa, ఖాళీ);
	sigaction(SIGTERM, &sa, ఖాళీ);

	fp = fopen(pidfile, "w");
	ఒకవేళ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	ఒకవేళ (svcname)
		rc_service_daemon_సమితి(svcname, exec, (స్థిరరాశి అక్షరం * స్థిరరాశి *) argv,
				pidfile, true);

	/* remove the controlling tty */
#నిర్వచితమైతే TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#అయితే_ముగింపు

	/*
	 * Supervisor main loop
	 */
	ఒకవేళ (healthcheckdelay)
		alarm(healthcheckdelay);
	లేకపోతే ఒకవేళ (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	అయితే (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		ఒకవేళ (fifo_fd > 0) {
			memset(buf, 0, పరిమాణం(buf));
			లెక్క = పఠ(fifo_fd, buf, పరిమాణం(buf) - 1);
			close(fifo_fd);
			ఒకవేళ (లెక్క != -1)
				buf[లెక్క] = 0;
			ఒకవేళ (లెక్క == 0)
				కొనసాగు;
			ఒకవేళ (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			ఒకవేళ (strncasecmp(buf, "sig", 3) == 0) {
				ఒకవేళ ((అక్షరశ్రేణి_చదువు(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							బిడ్డ_pid);
					ఒకవేళ (kill(బిడ్డ_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, బిడ్డ_pid);
				}
			}
			కొనసాగు;
		}
		ఒకవేళ (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			ఒకవేళ (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_స్థితి = rc_waitpid(health_pid);
			ఒకవేళ (WIFEXITED(health_స్థితి) && WEXITSTATUS(health_స్థితి) == 0)
				alarm(healthchecktimer);
			లేకపోతే {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, బిడ్డ_pid);
				nkilled = నిరంతరం_stop_కాలపట్టిక(applet, ఖాళీ, ఖాళీ, బిడ్డ_pid, 0,
						stopgroup, false, false, true);
				ఒకవేళ (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							బిడ్డ_pid, strerror(errno));
				లేకపోతే
					healthcheck_respawn = 1;
			}
		}
		ఒకవేళ (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, బిడ్డ_pid);
			nkilled = నిరంతరం_stop_కాలపట్టిక(applet, ఖాళీ, ఖాళీ, బిడ్డ_pid, 0,
					stopgroup, false, false, true);
			ఒకవేళ (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			కొనసాగు;
		}
		wait_pid = waitpid(బిడ్డ_pid, &i, WNOHANG);
		ఒకవేళ (wait_pid == బిడ్డ_pid) {
			ఒకవేళ (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, బిడ్డ_pid, WEXITSTATUS(i));
			లేకపోతే ఒకవేళ (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, బిడ్డ_pid, WTERMSIG(i));
		}
		ఒకవేళ (wait_pid == బిడ్డ_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			ఒకవేళ (మొదటి_spawn == 0)
				మొదటి_spawn = respawn_now;
			ఒకవేళ ((respawn_period > 0)
					&& (respawn_now - మొదటి_spawn > respawn_period)) {
				respawn_లెక్క = 0;
				మొదటి_spawn = 0;
			} లేకపోతే
				respawn_లెక్క++;
			ఒకవేళ (respawn_max > 0 && respawn_లెక్క > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				కొనసాగు;
			}
			sleep_for = respawn_delay + (respawn_delay_అడుగు * respawn_లెక్క);
			ఒకవేళ (respawn_delay_అడుగు > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			ఒకవేళ (exiting)
				కొనసాగు;
			బిడ్డ_pid = fork();
			ఒకవేళ (బిడ్డ_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				నిష్క్రమించు(EXIT_FAILURE);
			}
			ఒకవేళ (బిడ్డ_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_సంకేతాలు, ఖాళీ);
				memset(&sa, 0, పరిమాణం(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ఖాళీ);
				sigaction(SIGCHLD, &sa, ఖాళీ);
				sigaction(SIGTERM, &sa, ఖాళీ);
				బిడ్డ_process(exec, argv);
			}
			ఒకవేళ (healthcheckdelay)
				alarm(healthcheckdelay);
			లేకపోతే ఒకవేళ (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	ఒకవేళ (svcname) {
		rc_service_daemon_సమితి(svcname, exec, (స్థిరరాశి అక్షరం *స్థిరరాశి *)argv,
				pidfile, false);
		rc_service_విలువ_సమితి(svcname, "child_pid", ఖాళీ);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		ఒకవేళ (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	ఒకవేళ (pidfile)
		unlink(pidfile);
	ఒకవేళ (fifopath)
		unlink(fifopath);
	నిష్క్రమించు(EXIT_SUCCESS);
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం opt;
	అక్షరం **c;
	పూర్ణాంకం x;
	bool ప్రారంభం = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	అక్షరం *exec = ఖాళీ;
	అక్షరం *retry = ఖాళీ;
	పూర్ణాంకం sig = SIGTERM;
	అక్షరం *home = ఖాళీ;
	పూర్ణాంకం tid = 0;
	pid_t pid;
	అక్షరం *tmp;
	అక్షరం *p;
	అక్షరం *token;
	పూర్ణాంకం i;
	పూర్ణాంకం n;
	అక్షరం *exec_file = ఖాళీ;
	అక్షరం *varbuf = ఖాళీ;
	అక్షరం numbuf[64];
	నిర్మాణం timespec ts;
	నిర్మాణం passwd *pw;
	నిర్మాణం group *gr;
	FILE *fp;
	mode_t numask = 022;
	పూర్ణాంకం బిడ్డ_argc = 0;
	అక్షరం **బిడ్డ_argv = ఖాళీ;
	అక్షరం *str = ఖాళీ;
	అక్షరం *cmdline = ఖాళీ;
	స్థిరరాశి అక్షరం *respawn_delay_str  = "0";
	స్థిరరాశి అక్షరం *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	ఒకవేళ (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	ఒకవేళ (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	ఒకవేళ ((tmp = getenv("SSD_NICELEVEL")))
		ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ఒకవేళ ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = అక్షరశ్రేణి_చదువు(tmp, "%d:%d", &ionicec, &ioniced);
		ఒకవేళ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ఒకవేళ (ionicec == 0)
			ioniced = 0;
		లేకపోతే ఒకవేళ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ఒకవేళ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ఒకవేళ (home == ఖాళీ || p == ఖాళీ) {
		pw = getpwuid(getuid());
		ఒకవేళ (pw != ఖాళీ) {
			ఒకవేళ (p == ఖాళీ)
				setenv("USER", pw->pw_name, 1);
			ఒకవేళ (home == ఖాళీ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	ఒకవేళ (svcname) {
		argc--;
		argv++;
	}
	అయితే ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (పూర్ణాంకం *) 0)) != -1)
		ఎంపిక (opt) {
		స్థితి 'a':  /* --healthcheck-timer <time> */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			ఆపు;

		స్థితి 'A':  /* --healthcheck-delay <time> */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			ఆపు;

		స్థితి LONGOPT_CAPABILITIES:
#నిర్వచితమైతే __linux__
			cap_iab = cap_iab_from_పాఠ్యం(optarg);
			ఒకవేళ (cap_iab == ఖాళీ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#లేకపోతే
			eerrorx("Capabilities support not enabled");
#అయితే_ముగింపు
			ఆపు;

		స్థితి LONGOPT_SECBITS:
#నిర్వచితమైతే __linux__
			ఒకవేళ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ఖాళీ;
			secbits = strtoul(optarg, &tmp, 0);
			ఒకవేళ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#లేకపోతే
			eerrorx("Capabilities support not enabled");
#అయితే_ముగింపు
			ఆపు;

		స్థితి LONGOPT_NO_NEW_PRIVS:
#నిర్వచితమైతే PR_సమితి_NO_NEW_PRIVS
			no_new_privs = true;
#లేకపోతే
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#అయితే_ముగింపు
			ఆపు;

		స్థితి 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			ఆపు;

		స్థితి 'I': /* --ionice */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ఒకవేళ (ionicec == 0)
				ioniced = 0;
			లేకపోతే ఒకవేళ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ఆపు;

		స్థితి 'K':  /* --stop */
			stop = true;
			ఆపు;

		స్థితి 'N':  /* --nice */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ఆపు;

		స్థితి LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ఆపు;

		స్థితి 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			ఆపు;

		స్థితి 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			ఆపు;
		స్థితి 'S':  /* --start */
			ప్రారంభం = true;
			ఆపు;

		స్థితి 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ఆపు;

		స్థితి 'e': /* --env */
			putenv(optarg);
			ఆపు;

		స్థితి 'g':  /* --group <group>|<gid> */
			ఒకవేళ (అక్షరశ్రేణి_చదువు(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			లేకపోతే
				gr = getgrgid((gid_t)tid);
			ఒకవేళ (gr == ఖాళీ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ఆపు;

		స్థితి 'G': /* -G */
			stopgroup = true;
			ఆపు;

		స్థితి 'k':
			ఒకవేళ (parse_బహుళకం(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ఆపు;

		స్థితి 'm':  /* --respawn-max count */
			n = అక్షరశ్రేణి_చదువు(optarg, "%d", &respawn_max);
			ఒకవేళ (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			ఆపు;

		స్థితి 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ఆపు;

		స్థితి 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			ఆపు;
		స్థితి 'r':  /* --chroot /new/root */
			ch_మూలం = optarg;
			ఆపు;

		స్థితి 'u':  /* --user <username>|<uid> */
		{
			అక్షరం dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			లేకపోతే
				pw = getpwuid((uid_t)tid);

			ఒకవేళ (pw == ఖాళీ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ఒకవేళ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ఒకవేళ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ఒకవేళ (gid == 0)
				gid = pw->pw_gid;

			ఒకవేళ (p) {
				tmp = strsep (&p, ":");
				ఒకవేళ (అక్షరశ్రేణి_చదువు(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				లేకపోతే
					gr = getgrgid((gid_t) tid);

				ఒకవేళ (gr == ఖాళీ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ఆపు;

		స్థితి '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ఆపు;

		స్థితి '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ఆపు;

		స్థితి '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ఆపు;
		స్థితి '3':  /* --reexec */
			reexec = true;
			ఆపు;

		స్థితి LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ఆపు;

		స్థితి LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ఆపు;

		స్థితి LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			ఆపు;

		స్థితి LONGOPT_RESPAWN_DELAY_అడుగు:
			respawn_delay_అడుగు = parse_duration(optarg);
			ఒకవేళ (respawn_delay_అడుగు < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			ఆపు;

		స్థితి LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			ఒకవేళ (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			ఆపు;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	ఒకవేళ (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	ఒకవేళ (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	ఒకవేళ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ఒకవేళ (ch_మూలం && *ch_మూలం == '~')
		ch_మూలం = expand_home(home, ch_మూలం);

	umask(numask);
	ఒకవేళ (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	ఒకవేళ (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	ఒకవేళ (reexec) {
		str = rc_service_విలువ_get(svcname, "argc");
		అక్షరశ్రేణి_చదువు(str, "%d", &బిడ్డ_argc);
		బిడ్డ_argv = xmalloc((బిడ్డ_argc + 1) * పరిమాణం(అక్షరం *));
		memset(బిడ్డ_argv, 0, (బిడ్డ_argc + 1) * పరిమాణం(అక్షరం *));
		కొరకు (x = 0; x < బిడ్డ_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_విలువ_get(svcname, varbuf);
			బిడ్డ_argv[x] = str;
			విడుదల(varbuf);
			varbuf = ఖాళీ;
		}
		విడుదల(str);
		str = rc_service_విలువ_get(svcname, "child_pid");
		అక్షరశ్రేణి_చదువు(str, "%d", &బిడ్డ_pid);
		విడుదల(str);
		exec = rc_service_విలువ_get(svcname, "exec");
		pidfile = rc_service_విలువ_get(svcname, "pidfile");
		retry = rc_service_విలువ_get(svcname, "retry");
		ఒకవేళ (retry) {
			parse_కాలపట్టిక(applet, retry, sig);
			rc_service_విలువ_సమితి(svcname, "retry", retry);
		} లేకపోతే
			parse_కాలపట్టిక(applet, ఖాళీ, sig);

		str = rc_service_విలువ_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_విలువ_get(svcname, "respawn_delay_step");
		అక్షరశ్రేణి_చదువు(str, "%"SCNd64, &respawn_delay_అడుగు);
		str = rc_service_విలువ_get(svcname, "respawn_delay_cap");
		అక్షరశ్రేణి_చదువు(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_విలువ_get(svcname, "respawn_max");
		అక్షరశ్రేణి_చదువు(str, "%d", &respawn_max);
		supervisor(exec, బిడ్డ_argv);
	} లేకపోతే ఒకవేళ (ప్రారంభం) {
		ఒకవేళ (exec) {
			ఒకవేళ (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			ఒకవేళ (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				ఒకవేళ (ch_మూలం)
					xasprintf(&exec_file, "%s/%s", ch_మూలం, exec);
				లేకపోతే
					xasprintf(&exec_file, "%s", exec);
			} లేకపోతే {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = ఖాళీ;
				అయితే ((token = strsep(&p, ":"))) {
					ఒకవేళ (ch_మూలం)
						xasprintf(&exec_file, "%s/%s/%s", ch_మూలం, token, exec);
					లేకపోతే
						xasprintf(&exec_file, "%s/%s", token, exec);
					ఒకవేళ (exec_file && access(exec_file, F_OK) == 0)
						ఆపు;
					విడుదల(exec_file);
					exec_file = ఖాళీ;
				}
				విడుదల(tmp);
			}
			ఒకవేళ (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				విడుదల(exec_file);
				నిష్క్రమించు(EXIT_FAILURE);
			}
		} లేకపోతే
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		ఒకవేళ (pid != -1)
			ఒకవేళ (do_stop(applet, exec, (స్థిరరాశి అక్షరం * స్థిరరాశి *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		ఒకవేళ (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		ఒకవేళ (respawn_delay_అడుగు > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		ఒకవేళ (retry) {
			parse_కాలపట్టిక(applet, retry, sig);
			rc_service_విలువ_సమితి(svcname, "retry", retry);
		} లేకపోతే
			parse_కాలపట్టిక(applet, ఖాళీ, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		విడుదల(cmdline);
		cmdline = ఖాళీ;

		/* Remove existing pidfile */
		ఒకవేళ (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		ఒకవేళ (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_విలువ_సమితి(svcname, "pidfile", pidfile);
		rc_service_విలువ_సమితి(svcname, "respawn_delay", respawn_delay_str);
		rc_service_విలువ_సమితి(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, పరిమాణం(numbuf), "%"PRId64, respawn_delay_అడుగు);
		rc_service_విలువ_సమితి(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, పరిమాణం(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_విలువ_సమితి(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, పరిమాణం(numbuf), "%i", respawn_max);
		rc_service_విలువ_సమితి(svcname, "respawn_max", numbuf);
		బిడ్డ_pid = fork();
		ఒకవేళ (బిడ్డ_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ఒకవేళ (బిడ్డ_pid != 0)
			నిష్క్రమించు(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#నిర్వచితమైతే TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#అయితే_ముగింపు
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		ఒకవేళ (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		బిడ్డ_pid = fork();
		ఒకవేళ (బిడ్డ_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		లేకపోతే ఒకవేళ (బిడ్డ_pid != 0) {
			ఒకవేళ (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			అయితే (c && *c) {
				varbuf = ఖాళీ;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_విలువ_సమితి(svcname, varbuf, *c);
				విడుదల(varbuf);
				varbuf = ఖాళీ;
				x++;
				c++;
			}
			snprintf(numbuf, పరిమాణం(numbuf), "%d", x);
			rc_service_విలువ_సమితి(svcname, "argc", numbuf);
			rc_service_విలువ_సమితి(svcname, "exec", exec);
			supervisor(exec, argv);
		} లేకపోతే
			బిడ్డ_process(exec, argv);
	} లేకపోతే ఒకవేళ (stop) {
		pid = get_pid(applet, pidfile);
		ఒకవేళ (pid != -1) {
			i = kill(pid, SIGTERM);
			ఒకవేళ (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			లేకపోతే {
				/* wait for the supervisor to go down */
				అయితే (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ఖాళీ);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ఒకవేళ (pidfile)
			unlink(pidfile);
		ఒకవేళ (svcname) {
			rc_service_daemon_సమితి(svcname, exec,
			    (స్థిరరాశి అక్షరం *స్థిరరాశి *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		నిష్క్రమించు(EXIT_SUCCESS);
	} లేకపోతే ఒకవేళ (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		ఒకవేళ (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = లిఖ(fifo_fd, str, అక్షరశ్రేణి_పొడవు(str));
		ఒకవేళ (x == -1) {
			విడుదల(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		విడుదల(tmp);
		నిష్క్రమించు(EXIT_SUCCESS);
	}
}
