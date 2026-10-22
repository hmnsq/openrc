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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <grp.h>
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <sys/resource.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ifdef HAVE_PAM
#ଅନ୍ତର୍ଭାଵ <security/pam_appl.h>

/* We are not supporting authentication conversations */
ସ୍ଥିର ସଂରଚନା pam_conv conv = { ଖାଲି, ଖାଲି};
#endif

#ifdef __linux__
#ଅନ୍ତର୍ଭାଵ <sys/capability.h>
#endif

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "plugin.h"
#ଅନ୍ତର୍ଭାଵ "schedules.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ଗଣନା {
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

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "healthcheck-timer",        1, ଖାଲି, 'a'},
	{ "healthcheck-delay",        1, ଖାଲି, 'A'},
	{ "capabilities", 1, ଖାଲି, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ଖାଲି, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ଖାଲି, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ଖାଲି, 'D'},
	{ "respawn-delay-step",   1, ଖାଲି, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, ଖାଲି, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ଖାଲି, 'd'},
	{ "env",          1, ଖାଲି, 'e'},
	{ "group",        1, ଖାଲି, 'g'},
	{ "stop-group",   0, ଖାଲି, 'G'},
	{ "ionice",       1, ଖାଲି, 'I'},
	{ "stop",         0, ଖାଲି, 'K'},
	{ "umask",        1, ଖାଲି, 'k'},
	{ "respawn-max",    1, ଖାଲି, 'm'},
	{ "nicelevel",    1, ଖାଲି, 'N'},
	{ "oom-score-adj",1, ଖାଲି, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ଖାଲି, 'p'},
	{ "respawn-period",        1, ଖାଲି, 'P'},
	{ "retry",       1, ଖାଲି, 'R'},
	{ "chroot",       1, ଖାଲି, 'r'},
	{ "signal",       1, ଖାଲି, 's'},
	{ "start",        0, ଖାଲି, 'S'},
	{ "user",         1, ଖାଲି, 'u'},
	{ "stdin",        1, ଖାଲି, '0'},
	{ "stdout",       1, ଖାଲି, '1'},
	{ "stderr",       1, ଖାଲି, '2'},
	{ "stdout-logger",1, ଖାଲି, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ଖାଲି, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ଖାଲି, '3'},
	{ "notify",       1, ଖାଲି, LONGOPT_NOTIFY},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା healthcheckdelay = 0;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା healthchecktimer = 0;
ସ୍ଥିର ଅସ୍ଥିର sig_atomic_t do_healthcheck = 0;
ସ୍ଥିର ଅସ୍ଥିର sig_atomic_t exiting = 0;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nicelevel = INT_MIN;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ionicec = -1;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ioniced = 0;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା oom_score_adj = INT_MIN;
ସ୍ଥିର ଅକ୍ଷର *changeuser, *ch_ମୂଳ, *ch_dir;
ସ୍ଥିର uid_t uid = 0;
ସ୍ଥିର gid_t gid = 0;
ସ୍ଥିର bool stopgroup = false;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnull_fd = -1;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdin_fd;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdout_fd;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା stderr_fd;
ସ୍ଥିର ସଂରଚନା notify notify;
ସ୍ଥିର ଅକ୍ଷର *redirect_stdin = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *redirect_stderr = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *redirect_stdout = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *stderr_process = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *stdout_process = ଖାଲି;
#ifdef TIOCNOTTY
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tty_fd = -1;
#endif
ସ୍ଥିର pid_t child_pid;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା respawn_ଗଣନା = 0;
ସ୍ଥିର int64_t respawn_delay; /* default set inside main() */
ସ୍ଥିର int64_t respawn_delay_step = TM_MS(128);
ସ୍ଥିର int64_t respawn_delay_cap = TM_SEC(30);
ସ୍ଥିର int64_t respawn_period; /* default set inside main() */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା respawn_max = 10;
ସ୍ଥିର ଅକ୍ଷର *fifopath = ଖାଲି;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fifo_fd = 0;
ସ୍ଥିର ଅକ୍ଷର *pidfile = ଖାଲି;
ସ୍ଥିର ଅକ୍ଷର *svcname = ଖାଲି;
ସ୍ଥିର bool verbose = false;
#ifdef __linux__
ସ୍ଥିର cap_iab_t cap_iab = ଖାଲି;
ସ୍ଥିର unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
ସ୍ଥିର bool no_new_privs = false;
#endif

ବାହ୍ୟ ଅକ୍ଷର **environ;

#ଯଦି !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ଯଦି !defined(__DragonFly__)
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ioprio_set(ପୂର୍ଣ୍ଣସଂଖ୍ୟା which RC_UNUSED, ପୂର୍ଣ୍ଣସଂଖ୍ୟା who RC_UNUSED,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	ଫେରନ୍ତୁ syscall(SYS_ioprio_set, which, who, ioprio);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ cleanup(ଶୂନ୍ୟ)
{
	ମୁକ୍ତ(changeuser);
}

RC_NORETURN ସ୍ଥିର ଶୂନ୍ୟ re_exec_supervisor(ଶୂନ୍ୟ)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(ଅକ୍ଷର *) ଖାଲି);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

ସ୍ଥିର ଶୂନ୍ୟ handle_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno = errno;
	pid_t pid;

	ବାଛ (sig) {
	ସ୍ଥିତି SIGALRM:
		do_healthcheck = 1;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SIGCHLD:
		ଯଦି (exiting)
			ଯେତେବେଳେ (waitpid((pid_t)(-1), ଖାଲି, WNOHANG) > 0) {}
		ଅନ୍ୟଥା {
			ଯେତେବେଳେ ((pid = waitpid((pid_t)(-1), ଖାଲି, WNOHANG|WNOWAIT)) > 0) {
				ଯଦି (pid == child_pid)
					ଭାଙ୍ଗ;
				pid = waitpid(pid, ଖାଲି, WNOHANG);
			}
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SIGTERM:
		exiting = 1;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ସ୍ଥିର ଅକ୍ଷର * expand_home(ଅଚଳ ଅକ୍ଷର *home, ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *opath, *ppath, *p, *nh;
	ସଂରଚନା passwd *pw;

	ଯଦି (!path || *path != '~')
		ଫେରନ୍ତୁ xstrdup(path);

	opath = ppath = xstrdup(path);
	ଯଦି (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ଯଦି (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ଯଦି (pw) {
			home = pw->pw_dir;
			ppath = p;
			ଯଦି (ppath)
				*ppath = '/';
		} ଅନ୍ୟଥା
			home = ଖାଲି;
	} ଅନ୍ୟଥା
		ppath++;

	ଯଦି (!home) {
		ମୁକ୍ତ(opath);
		ଫେରନ୍ତୁ xstrdup(path);
	}
	ଯଦି (!ppath) {
		ମୁକ୍ତ(opath);
		ଫେରନ୍ତୁ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ମୁକ୍ତ(opath);
	ଫେରନ୍ତୁ nh;
}

ସ୍ଥିର ଅକ୍ଷର *make_cmdline(ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର **c;
	ଅକ୍ଷର *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	ପାଇଁ (c = argv; c && *c; c++) {
		fputs(*c, mem);
		ଯଦି (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	ଫେରନ୍ତୁ cmdline;
}

ସ୍ଥିର pid_t exec_command(ଅଚଳ ଅକ୍ଷର *cmd)
{
	ଅକ୍ଷର *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	ସଂରଚନା sigaction sa;

	file = rc_service_resolve(svcname);
	ଯଦି (access(file, F_OK) != 0) {
		ମୁକ୍ତ(file);
		ଫେରନ୍ତୁ 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, ଆକାର (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	ଯଦି (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ଖାଲି);
		sigaction(SIGHUP, &sa, ଖାଲି);
		sigaction(SIGINT, &sa, ଖାଲି);
		sigaction(SIGQUIT, &sa, ଖାଲି);
		sigaction(SIGTERM, &sa, ଖାଲି);
		sigaction(SIGUSR1, &sa, ଖାଲି);
		sigaction(SIGWINCH, &sa, ଖାଲି);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ଖାଲି);

		/* Safe to run now */
		execl(file, file, cmd, (ଅକ୍ଷର *) ଖାଲି);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ଯଦି (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ଖାଲି);
	ମୁକ୍ତ(file);
	ଫେରନ୍ତୁ pid;
}

RC_NORETURN ସ୍ଥିର ଶୂନ୍ୟ child_process(ଅକ୍ଷର *exec, ଅକ୍ଷର **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
	size_t len;
	ଅକ୍ଷର *newpath;
	ଅକ୍ଷର *np;
	ଅକ୍ଷର *cmdline = ଖାଲି;
	time_t start_time;
	ଅକ୍ଷର start_ଗଣନା_string[20];
	ଅକ୍ଷର start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା group_ଗଣନା = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pamr;
	ଅଚଳ ଅକ୍ଷର *ଅଚଳ *pamenv = ଖାଲି;
#endif

	setsid();

	ଯଦି (svcname) {
		start_time = time(ଖାଲି);
		from_time_t(start_time_string, start_time);
		rc_service_ମୂଲ୍ୟ_set(svcname, "start_time", start_time_string);
		sprintf(start_ଗଣନା_string, "%i", respawn_ଗଣନା);
		rc_service_ମୂଲ୍ୟ_set(svcname, "start_count", start_ଗଣନା_string);
		sprintf(start_ଗଣନା_string, "%d", getpid());
		rc_service_ମୂଲ୍ୟ_set(svcname, "child_pid", start_ଗଣନା_string);
	}

	ଯଦି (nicelevel != INT_MIN) {
		ଯଦି (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		ଯଦି (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} ଅନ୍ୟଥା ଯଦି (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	ଯଦି (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	ଯଦି (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		ଯଦି (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	ଯଦି (changeuser != ଖାଲି) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		ଯଦି (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		ଯଦି (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		ଯଦି (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	ଯଦି (changeuser && getgrouplist(changeuser, gid, group_list, &group_ଗଣନା) < 0) {
		group_list = xmalloc(group_ଗଣନା * ଆକାର(*group_list));
		ଯଦି (getgrouplist(changeuser, gid, group_list, &group_ଗଣନା) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	ଯଦି (ch_ମୂଳ && chroot(ch_ମୂଳ) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_ମୂଳ, strerror(errno));

	ଯଦି (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	ଯଦି (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	ଯଦି (changeuser && setgroups(group_ଗଣନା, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	ଯଦି (group_list != group_buf)
		ମୁକ୍ତ(group_list);
#ifdef __linux__
	ଯଦି (uid && cap_setuid(uid))
#ଅନ୍ୟଥା
	ଯଦି (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	ଯଦି (cap_iab != ଖାଲି) {
		i = cap_iab_set_proc(cap_iab);

		ଯଦି (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		ଯଦି (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	ଯଦି (secbits != 0) {
		ଯଦି (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	ଯଦି (no_new_privs) {
		ଯଦି (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	ଯେତେବେଳେ (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	ଯଦି (changeuser != ଖାଲି) {
		pamenv = (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)pam_getenvlist(pamh);
		ଯଦି (pamenv) {
			ଯେତେବେଳେ (*pamenv) {
				/* Don't add strings unless they set a var */
				ଯଦି (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				ଅନ୍ୟଥା
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		ଯଦି (strncmp(env->ମୂଲ୍ୟ, "RC_", 3) == 0
				|| strncmp(env->ମୂଲ୍ୟ, "SSD_", 4) == 0
				|| strncmp(env->ମୂଲ୍ୟ, "EINFO_", 6) == 0) {
			*strchr(env->ମୂଲ୍ୟ, '=') = '\0';
			unsetenv(env->ମୂଲ୍ୟ);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	ଯଦି ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		ଯେତେବେଳେ (token && *token) {
			p = strchr(token, ':');
			ଯଦି (p) {
				*p++ = '\0';
				ଯେତେବେଳେ (*p == ':')
					p++;
			}
			ଯଦି (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				ଯଦି (np != newpath)
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
	ଯଦି (redirect_stdin) {
		ଯଦି ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	ଯଦି (redirect_stdout) {
		ଯଦି ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} ଅନ୍ୟଥା ଯଦି (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		ଯଦି (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	ଯଦି (redirect_stderr) {
		ଯଦି ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} ଅନ୍ୟଥା ଯଦି (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		ଯଦି (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	ଯଦି (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	ଯଦି (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	ଯଦି (notify.type == NOTIFY_FD) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା error;

		ଯଦି (notify.fd == notify.pipe[1]) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = error = fcntl(notify.fd, F_GETFD, 0);
			ଯଦି (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} ଅନ୍ୟଥା {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		ଯଦି (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	ମୁକ୍ତ(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	ଯଦି (changeuser != ଖାଲି && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ସ୍ଥିର ଶୂନ୍ୟ supervisor(ଅକ୍ଷର *exec, ଅକ୍ଷର **argv)
{
	FILE *fp;
	ଅକ୍ଷର buf[2048];
	ଅକ୍ଷର cmd[2048];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା failing;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା health_status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା healthcheck_respawn;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nkilled;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	ସଂରଚନା sigaction sa;
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
	memset(&sa, 0, ଆକାର(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ଖାଲି);
	sigaction(SIGCHLD, &sa, ଖାଲି);
	sigaction(SIGTERM, &sa, ଖାଲି);

	fp = fopen(pidfile, "w");
	ଯଦି (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	ଯଦି (svcname)
		rc_service_daemon_set(svcname, exec, (ଅଚଳ ଅକ୍ଷର * ଅଚଳ *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	ଯଦି (healthcheckdelay)
		alarm(healthcheckdelay);
	ଅନ୍ୟଥା ଯଦି (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	ଯେତେବେଳେ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		ଯଦି (fifo_fd > 0) {
			memset(buf, 0, ଆକାର(buf));
			ଗଣନା_2 = ପଠ(fifo_fd, buf, ଆକାର(buf) - 1);
			close(fifo_fd);
			ଯଦି (ଗଣନା_2 != -1)
				buf[ଗଣନା_2] = 0;
			ଯଦି (ଗଣନା_2 == 0)
				ଜାରି;
			ଯଦି (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			ଯଦି (strncasecmp(buf, "sig", 3) == 0) {
				ଯଦି ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					ଯଦି (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			ଜାରି;
		}
		ଯଦି (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			ଯଦି (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			ଯଦି (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			ଅନ୍ୟଥା {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, ଖାଲି, ଖାଲି, child_pid, 0,
						stopgroup, false, false, true);
				ଯଦି (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				ଅନ୍ୟଥା
					healthcheck_respawn = 1;
			}
		}
		ଯଦି (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, ଖାଲି, ଖାଲି, child_pid, 0,
					stopgroup, false, false, true);
			ଯଦି (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			ଜାରି;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		ଯଦି (wait_pid == child_pid) {
			ଯଦି (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			ଅନ୍ୟଥା ଯଦି (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		ଯଦି (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			ଯଦି (first_spawn == 0)
				first_spawn = respawn_now;
			ଯଦି ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_ଗଣନା = 0;
				first_spawn = 0;
			} ଅନ୍ୟଥା
				respawn_ଗଣନା++;
			ଯଦି (respawn_max > 0 && respawn_ଗଣନା > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				ଜାରି;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_ଗଣନା);
			ଯଦି (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			ଯଦି (exiting)
				ଜାରି;
			child_pid = fork();
			ଯଦି (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			ଯଦି (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, ଖାଲି);
				memset(&sa, 0, ଆକାର(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ଖାଲି);
				sigaction(SIGCHLD, &sa, ଖାଲି);
				sigaction(SIGTERM, &sa, ଖାଲି);
				child_process(exec, argv);
			}
			ଯଦି (healthcheckdelay)
				alarm(healthcheckdelay);
			ଅନ୍ୟଥା ଯଦି (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	ଯଦି (svcname) {
		rc_service_daemon_set(svcname, exec, (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)argv,
				pidfile, false);
		rc_service_ମୂଲ୍ୟ_set(svcname, "child_pid", ଖାଲି);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		ଯଦି (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	ଯଦି (pidfile)
		unlink(pidfile);
	ଯଦି (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ଅକ୍ଷର **c;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	ଅକ୍ଷର *exec = ଖାଲି;
	ଅକ୍ଷର *retry = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = SIGTERM;
	ଅକ୍ଷର *home = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା tid = 0;
	pid_t pid;
	ଅକ୍ଷର *tmp;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
	ଅକ୍ଷର *exec_file = ଖାଲି;
	ଅକ୍ଷର *varbuf = ଖାଲି;
	ଅକ୍ଷର numbuf[64];
	ସଂରଚନା timespec ts;
	ସଂରଚନା passwd *pw;
	ସଂରଚନା group *gr;
	FILE *fp;
	mode_t numask = 022;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା child_argc = 0;
	ଅକ୍ଷର **child_argv = ଖାଲି;
	ଅକ୍ଷର *str = ଖାଲି;
	ଅକ୍ଷର *cmdline = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *respawn_delay_str  = "0";
	ଅଚଳ ଅକ୍ଷର *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	ଯଦି (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	ଯଦି (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	ଯଦି ((tmp = getenv("SSD_NICELEVEL")))
		ଯଦି (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ଯଦି ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ଯଦି (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ଯଦି (ionicec == 0)
			ioniced = 0;
		ଅନ୍ୟଥା ଯଦି (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ଯଦି ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ଯଦି (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ଯଦି (home == ଖାଲି || p == ଖାଲି) {
		pw = getpwuid(getuid());
		ଯଦି (pw != ଖାଲି) {
			ଯଦି (p == ଖାଲି)
				setenv("USER", pw->pw_name, 1);
			ଯଦି (home == ଖାଲି) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	ଯଦି (svcname) {
		argc--;
		argv++;
	}
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
		ବାଛ (opt) {
		ସ୍ଥିତି 'a':  /* --healthcheck-timer <time> */
			ଯଦି (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'A':  /* --healthcheck-delay <time> */
			ଯଦି (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ଯଦି (cap_iab == ଖାଲି)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ଅନ୍ୟଥା
			eerrorx("Capabilities support not enabled");
#endif
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_SECBITS:
#ifdef __linux__
			ଯଦି (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ଖାଲି;
			secbits = strtoul(optarg, &tmp, 0);
			ଯଦି (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ଅନ୍ୟଥା
			eerrorx("Capabilities support not enabled");
#endif
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#ଅନ୍ୟଥା
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'I': /* --ionice */
			ଯଦି (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ଯଦି (ionicec == 0)
				ioniced = 0;
			ଅନ୍ୟଥା ଯଦି (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'K':  /* --stop */
			stop = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'N':  /* --nice */
			ଯଦି (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ଯଦି (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'S':  /* --start */
			start = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'e': /* --env */
			putenv(optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'g':  /* --group <group>|<gid> */
			ଯଦି (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ଅନ୍ୟଥା
				gr = getgrgid((gid_t)tid);
			ଯଦି (gr == ଖାଲି)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'G': /* -G */
			stopgroup = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'k':
			ଯଦି (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			ଯଦି (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':  /* --chroot /new/root */
			ch_ମୂଳ = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'u':  /* --user <username>|<uid> */
		{
			ଅକ୍ଷର dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ଯଦି (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ଅନ୍ୟଥା
				pw = getpwuid((uid_t)tid);

			ଯଦି (pw == ଖାଲି)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ଯଦି (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ଯଦି (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ଯଦି (gid == 0)
				gid = pw->pw_gid;

			ଯଦି (p) {
				tmp = strsep (&p, ":");
				ଯଦି (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ଅନ୍ୟଥା
					gr = getgrgid((gid_t) tid);

				ଯଦି (gr == ଖାଲି)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ଭାଙ୍ଗ;

		ସ୍ଥିତି '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '3':  /* --reexec */
			reexec = true;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			ଯଦି (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			ଯଦି (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	ଯଦି (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	ଯଦି (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	ଯଦି (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ଯଦି (ch_ମୂଳ && *ch_ମୂଳ == '~')
		ch_ମୂଳ = expand_home(home, ch_ମୂଳ);

	umask(numask);
	ଯଦି (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	ଯଦି (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	ଯଦି (reexec) {
		str = rc_service_ମୂଲ୍ୟ_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * ଆକାର(ଅକ୍ଷର *));
		memset(child_argv, 0, (child_argc + 1) * ଆକାର(ଅକ୍ଷର *));
		ପାଇଁ (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_ମୂଲ୍ୟ_get(svcname, varbuf);
			child_argv[x] = str;
			ମୁକ୍ତ(varbuf);
			varbuf = ଖାଲି;
		}
		ମୁକ୍ତ(str);
		str = rc_service_ମୂଲ୍ୟ_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		ମୁକ୍ତ(str);
		exec = rc_service_ମୂଲ୍ୟ_get(svcname, "exec");
		pidfile = rc_service_ମୂଲ୍ୟ_get(svcname, "pidfile");
		retry = rc_service_ମୂଲ୍ୟ_get(svcname, "retry");
		ଯଦି (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_ମୂଲ୍ୟ_set(svcname, "retry", retry);
		} ଅନ୍ୟଥା
			parse_schedule(applet, ଖାଲି, sig);

		str = rc_service_ମୂଲ୍ୟ_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_ମୂଲ୍ୟ_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_ମୂଲ୍ୟ_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_ମୂଲ୍ୟ_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} ଅନ୍ୟଥା ଯଦି (start) {
		ଯଦି (exec) {
			ଯଦି (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			ଯଦି (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				ଯଦି (ch_ମୂଳ)
					xasprintf(&exec_file, "%s/%s", ch_ମୂଳ, exec);
				ଅନ୍ୟଥା
					xasprintf(&exec_file, "%s", exec);
			} ଅନ୍ୟଥା {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = ଖାଲି;
				ଯେତେବେଳେ ((token = strsep(&p, ":"))) {
					ଯଦି (ch_ମୂଳ)
						xasprintf(&exec_file, "%s/%s/%s", ch_ମୂଳ, token, exec);
					ଅନ୍ୟଥା
						xasprintf(&exec_file, "%s/%s", token, exec);
					ଯଦି (exec_file && access(exec_file, F_OK) == 0)
						ଭାଙ୍ଗ;
					ମୁକ୍ତ(exec_file);
					exec_file = ଖାଲି;
				}
				ମୁକ୍ତ(tmp);
			}
			ଯଦି (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				ମୁକ୍ତ(exec_file);
				exit(EXIT_FAILURE);
			}
		} ଅନ୍ୟଥା
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		ଯଦି (pid != -1)
			ଯଦି (do_stop(applet, exec, (ଅଚଳ ଅକ୍ଷର * ଅଚଳ *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		ଯଦି (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		ଯଦି (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		ଯଦି (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_ମୂଲ୍ୟ_set(svcname, "retry", retry);
		} ଅନ୍ୟଥା
			parse_schedule(applet, ଖାଲି, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		ମୁକ୍ତ(cmdline);
		cmdline = ଖାଲି;

		/* Remove existing pidfile */
		ଯଦି (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		ଯଦି (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_ମୂଲ୍ୟ_set(svcname, "pidfile", pidfile);
		rc_service_ମୂଲ୍ୟ_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_ମୂଲ୍ୟ_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, ଆକାର(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_ମୂଲ୍ୟ_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, ଆକାର(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_ମୂଲ୍ୟ_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, ଆକାର(numbuf), "%i", respawn_max);
		rc_service_ମୂଲ୍ୟ_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		ଯଦି (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ଯଦି (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		ଯଦି (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		ଯଦି (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ଅନ୍ୟଥା ଯଦି (child_pid != 0) {
			ଯଦି (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			ଯେତେବେଳେ (c && *c) {
				varbuf = ଖାଲି;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_ମୂଲ୍ୟ_set(svcname, varbuf, *c);
				ମୁକ୍ତ(varbuf);
				varbuf = ଖାଲି;
				x++;
				c++;
			}
			snprintf(numbuf, ଆକାର(numbuf), "%d", x);
			rc_service_ମୂଲ୍ୟ_set(svcname, "argc", numbuf);
			rc_service_ମୂଲ୍ୟ_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} ଅନ୍ୟଥା
			child_process(exec, argv);
	} ଅନ୍ୟଥା ଯଦି (stop) {
		pid = get_pid(applet, pidfile);
		ଯଦି (pid != -1) {
			i = kill(pid, SIGTERM);
			ଯଦି (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			ଅନ୍ୟଥା {
				/* wait for the supervisor to go down */
				ଯେତେବେଳେ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ଖାଲି);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ଯଦି (pidfile)
			unlink(pidfile);
		ଯଦି (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (ଅଚଳ ଅକ୍ଷର *ଅଚଳ *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} ଅନ୍ୟଥା ଯଦି (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		ଯଦି (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = ଲିଖ(fifo_fd, str, strlen(str));
		ଯଦି (x == -1) {
			ମୁକ୍ତ(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		ମୁକ୍ତ(tmp);
		exit(EXIT_SUCCESS);
	}
}
