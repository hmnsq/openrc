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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <grp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/resource.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
#ਅਨ੍ਤਰ੍ਭਾਵ <security/pam_appl.h>

/* We are not supporting authentication conversations */
ਸਥਿਰ ਬਣਤਰ pam_conv conv = { ਨਿਰਾ, ਨਿਰਾ};
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/capability.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "plugin.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "schedules.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ਗਿਣਤੀ {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_ਅਧਾਰ = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_ਕਦਮ,
  LONGOPT_RESPAWN_DELAY_CAP,
};

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "healthcheck-timer",        1, ਨਿਰਾ, 'a'},
	{ "healthcheck-delay",        1, ਨਿਰਾ, 'A'},
	{ "capabilities", 1, ਨਿਰਾ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ਨਿਰਾ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ਨਿਰਾ, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ਨਿਰਾ, 'D'},
	{ "respawn-delay-step",   1, ਨਿਰਾ, LONGOPT_RESPAWN_DELAY_ਕਦਮ},
	{ "respawn-delay-cap",    1, ਨਿਰਾ, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ਨਿਰਾ, 'd'},
	{ "env",          1, ਨਿਰਾ, 'e'},
	{ "group",        1, ਨਿਰਾ, 'g'},
	{ "stop-group",   0, ਨਿਰਾ, 'G'},
	{ "ionice",       1, ਨਿਰਾ, 'I'},
	{ "stop",         0, ਨਿਰਾ, 'K'},
	{ "umask",        1, ਨਿਰਾ, 'k'},
	{ "respawn-max",    1, ਨਿਰਾ, 'm'},
	{ "nicelevel",    1, ਨਿਰਾ, 'N'},
	{ "oom-score-adj",1, ਨਿਰਾ, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ਨਿਰਾ, 'p'},
	{ "respawn-period",        1, ਨਿਰਾ, 'P'},
	{ "retry",       1, ਨਿਰਾ, 'R'},
	{ "chroot",       1, ਨਿਰਾ, 'r'},
	{ "signal",       1, ਨਿਰਾ, 's'},
	{ "start",        0, ਨਿਰਾ, 'S'},
	{ "user",         1, ਨਿਰਾ, 'u'},
	{ "stdin",        1, ਨਿਰਾ, '0'},
	{ "stdout",       1, ਨਿਰਾ, '1'},
	{ "stderr",       1, ਨਿਰਾ, '2'},
	{ "stdout-logger",1, ਨਿਰਾ, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ਨਿਰਾ, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ਨਿਰਾ, '3'},
	{ "notify",       1, ਨਿਰਾ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਸਥਿਰ ਪੂਰਨ healthcheckdelay = 0;
ਸਥਿਰ ਪੂਰਨ healthchecktimer = 0;
ਸਥਿਰ ਅਸਥਿਰ sig_atomic_t do_healthcheck = 0;
ਸਥਿਰ ਅਸਥਿਰ sig_atomic_t exiting = 0;
ਸਥਿਰ ਪੂਰਨ nicelevel = INT_MIN;
ਸਥਿਰ ਪੂਰਨ ionicec = -1;
ਸਥਿਰ ਪੂਰਨ ioniced = 0;
ਸਥਿਰ ਪੂਰਨ oom_score_adj = INT_MIN;
ਸਥਿਰ ਅੱਖਰ *changeuser, *ch_ਜੜ੍ਹ, *ch_dir;
ਸਥਿਰ uid_t uid = 0;
ਸਥਿਰ gid_t gid = 0;
ਸਥਿਰ bool stopgroup = false;
ਸਥਿਰ ਪੂਰਨ devnull_fd = -1;
ਸਥਿਰ ਪੂਰਨ stdin_fd;
ਸਥਿਰ ਪੂਰਨ stdout_fd;
ਸਥਿਰ ਪੂਰਨ stderr_fd;
ਸਥਿਰ ਬਣਤਰ notify notify;
ਸਥਿਰ ਅੱਖਰ *redirect_stdin = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *redirect_stderr = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *redirect_stdout = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *stderr_process = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *stdout_process = ਨਿਰਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
ਸਥਿਰ ਪੂਰਨ tty_fd = -1;
#ਜੇ_ਅੰਤ
ਸਥਿਰ pid_t ਬੱਚਾ_pid;
ਸਥਿਰ ਪੂਰਨ respawn_ਗਿਣਤੀ = 0;
ਸਥਿਰ int64_t respawn_delay; /* default set inside main() */
ਸਥਿਰ int64_t respawn_delay_ਕਦਮ = TM_MS(128);
ਸਥਿਰ int64_t respawn_delay_cap = TM_SEC(30);
ਸਥਿਰ int64_t respawn_period; /* default set inside main() */
ਸਥਿਰ ਪੂਰਨ respawn_max = 10;
ਸਥਿਰ ਅੱਖਰ *fifopath = ਨਿਰਾ;
ਸਥਿਰ ਪੂਰਨ fifo_fd = 0;
ਸਥਿਰ ਅੱਖਰ *pidfile = ਨਿਰਾ;
ਸਥਿਰ ਅੱਖਰ *svcname = ਨਿਰਾ;
ਸਥਿਰ bool verbose = false;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
ਸਥਿਰ cap_iab_t cap_iab = ਨਿਰਾ;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ secbits = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PR_ਸੈੱਟ_NO_NEW_PRIVS
ਸਥਿਰ bool no_new_privs = false;
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਅੱਖਰ **environ;

#ਜੇ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#ਜੇ_ਅੰਤ
#ਜੇ !defined(__DragonFly__)
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ ioprio_ਸੈੱਟ(ਪੂਰਨ which RC_ਅਣਵਰਤਿਆ, ਪੂਰਨ who RC_ਅਣਵਰਤਿਆ,
			     ਪੂਰਨ ioprio RC_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS_ioprio_set
	ਵਾਪਸ syscall(SYS_ioprio_set, which, who, ioprio);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ cleanup(ਖਾਲੀ)
{
	ਮੁਕਤ(changeuser);
}

RC_NORETURN ਸਥਿਰ ਖਾਲੀ re_exec_supervisor(ਖਾਲੀ)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(ਅੱਖਰ *) ਨਿਰਾ);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}

ਸਥਿਰ ਖਾਲੀ handle_signal(ਪੂਰਨ sig)
{
	ਪੂਰਨ serrno = errno;
	pid_t pid;

	ਚੋਣ (sig) {
	ਹਾਲਤ SIGALRM:
		do_healthcheck = 1;
		ਰੋਕੋ;
	ਹਾਲਤ SIGCHLD:
		ਜੇ (exiting)
			ਜਦੋਂ (waitpid((pid_t)(-1), ਨਿਰਾ, WNOHANG) > 0) {}
		ਨਹੀਂਤਾਂ {
			ਜਦੋਂ ((pid = waitpid((pid_t)(-1), ਨਿਰਾ, WNOHANG|WNOWAIT)) > 0) {
				ਜੇ (pid == ਬੱਚਾ_pid)
					ਰੋਕੋ;
				pid = waitpid(pid, ਨਿਰਾ, WNOHANG);
			}
		}
		ਰੋਕੋ;
	ਹਾਲਤ SIGTERM:
		exiting = 1;
		ਰੋਕੋ;
	ਮੂਲ:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ਸਥਿਰ ਅੱਖਰ * expand_home(ਅਚਲ ਅੱਖਰ *home, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *opath, *ppath, *p, *nh;
	ਬਣਤਰ passwd *pw;

	ਜੇ (!ਰਸਤਾ || *ਰਸਤਾ != '~')
		ਵਾਪਸ xstrdup(ਰਸਤਾ);

	opath = ppath = xstrdup(ਰਸਤਾ);
	ਜੇ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ਜੇ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ਜੇ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ਜੇ (ppath)
				*ppath = '/';
		} ਨਹੀਂਤਾਂ
			home = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ
		ppath++;

	ਜੇ (!home) {
		ਮੁਕਤ(opath);
		ਵਾਪਸ xstrdup(ਰਸਤਾ);
	}
	ਜੇ (!ppath) {
		ਮੁਕਤ(opath);
		ਵਾਪਸ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ਮੁਕਤ(opath);
	ਵਾਪਸ nh;
}

ਸਥਿਰ ਅੱਖਰ *make_cmdline(ਅੱਖਰ **argv)
{
	ਅੱਖਰ **c;
	ਅੱਖਰ *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	ਲਈ (c = argv; c && *c; c++) {
		fputs(*c, mem);
		ਜੇ (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	ਵਾਪਸ cmdline;
}

ਸਥਿਰ pid_t exec_command(ਅਚਲ ਅੱਖਰ *cmd)
{
	ਅੱਖਰ *file;
	pid_t pid = -1;
	sigset_t ਭਰਿਆ;
	sigset_t old;
	ਬਣਤਰ sigaction sa;

	file = rc_service_resolve(svcname);
	ਜੇ (access(file, F_OK) != 0) {
		ਮੁਕਤ(file);
		ਵਾਪਸ 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, ਆਕਾਰ (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&ਭਰਿਆ);
	sigprocmask(SIG_SETMASK, &ਭਰਿਆ, &old);

	pid = fork();
	ਜੇ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ਨਿਰਾ);
		sigaction(SIGHUP, &sa, ਨਿਰਾ);
		sigaction(SIGINT, &sa, ਨਿਰਾ);
		sigaction(SIGQUIT, &sa, ਨਿਰਾ);
		sigaction(SIGTERM, &sa, ਨਿਰਾ);
		sigaction(SIGUSR1, &sa, ਨਿਰਾ);
		sigaction(SIGWINCH, &sa, ਨਿਰਾ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);

		/* Safe to run now */
		execl(file, file, cmd, (ਅੱਖਰ *) ਨਿਰਾ);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ਜੇ (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
	ਮੁਕਤ(file);
	ਵਾਪਸ pid;
}

RC_NORETURN ਸਥਿਰ ਖਾਲੀ ਬੱਚਾ_process(ਅੱਖਰ *exec, ਅੱਖਰ **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ਪੂਰਨ i;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
	size_t len;
	ਅੱਖਰ *newpath;
	ਅੱਖਰ *np;
	ਅੱਖਰ *cmdline = ਨਿਰਾ;
	time_t ਸ਼ੁਰੂ_ਸਮਾਂ;
	ਅੱਖਰ ਸ਼ੁਰੂ_ਗਿਣਤੀ_string[20];
	ਅੱਖਰ ਸ਼ੁਰੂ_ਸਮਾਂ_string[20];
	FILE *fp;
	gid_t ਸਮੂਹ_buf[32], *ਸਮੂਹ_list = ਸਮੂਹ_buf;
	ਪੂਰਨ ਸਮੂਹ_ਗਿਣਤੀ = ARRAY_ਮਾਪ(ਸਮੂਹ_buf);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	pam_handle_t *pamh = ਨਿਰਾ;
	ਪੂਰਨ pamr;
	ਅਚਲ ਅੱਖਰ *ਅਚਲ *pamenv = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	setsid();

	ਜੇ (svcname) {
		ਸ਼ੁਰੂ_ਸਮਾਂ = time(ਨਿਰਾ);
		from_ਸਮਾਂ_t(ਸ਼ੁਰੂ_ਸਮਾਂ_string, ਸ਼ੁਰੂ_ਸਮਾਂ);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "start_time", ਸ਼ੁਰੂ_ਸਮਾਂ_string);
		sprintf(ਸ਼ੁਰੂ_ਗਿਣਤੀ_string, "%i", respawn_ਗਿਣਤੀ);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "start_count", ਸ਼ੁਰੂ_ਗਿਣਤੀ_string);
		sprintf(ਸ਼ੁਰੂ_ਗਿਣਤੀ_string, "%d", getpid());
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "child_pid", ਸ਼ੁਰੂ_ਗਿਣਤੀ_string);
	}

	ਜੇ (nicelevel != INT_MIN) {
		ਜੇ (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		ਜੇ (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} ਨਹੀਂਤਾਂ ਜੇ (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	ਜੇ (ionicec != -1 && ioprio_ਸੈੱਟ(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	ਜੇ (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		ਜੇ (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	ਜੇ (changeuser != ਨਿਰਾ) {
		pamr = pam_ਸ਼ੁਰੂ("supervise-daemon",
		    changeuser, &conv, &pamh);

		ਜੇ (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		ਜੇ (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		ਜੇ (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#ਜੇ_ਅੰਤ

	ਜੇ (changeuser && getgrouplist(changeuser, gid, ਸਮੂਹ_list, &ਸਮੂਹ_ਗਿਣਤੀ) < 0) {
		ਸਮੂਹ_list = xmalloc(ਸਮੂਹ_ਗਿਣਤੀ * ਆਕਾਰ(*ਸਮੂਹ_list));
		ਜੇ (getgrouplist(changeuser, gid, ਸਮੂਹ_list, &ਸਮੂਹ_ਗਿਣਤੀ) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	ਜੇ (ch_ਜੜ੍ਹ && chroot(ch_ਜੜ੍ਹ) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_ਜੜ੍ਹ, strerror(errno));

	ਜੇ (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	ਜੇ (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	ਜੇ (changeuser && setgroups(ਸਮੂਹ_ਗਿਣਤੀ, ਸਮੂਹ_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	ਜੇ (ਸਮੂਹ_list != ਸਮੂਹ_buf)
		ਮੁਕਤ(ਸਮੂਹ_list);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਜੇ (uid && cap_setuid(uid))
#ਨਹੀਂਤਾਂ
	ਜੇ (uid && setuid(uid))
#ਜੇ_ਅੰਤ
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
	ਜੇ (cap_iab != ਨਿਰਾ) {
		i = cap_iab_ਸੈੱਟ_proc(cap_iab);

		ਜੇ (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		ਜੇ (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	ਜੇ (secbits != 0) {
		ਜੇ (cap_ਸੈੱਟ_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PR_ਸੈੱਟ_NO_NEW_PRIVS
	ਜੇ (no_new_privs) {
		ਜੇ (prctl(PR_ਸੈੱਟ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", strerror(errno));
	}
#ਜੇ_ਅੰਤ

	/* remove the controlling tty */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#ਜੇ_ਅੰਤ

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	ਜਦੋਂ (environ[i])
		rc_stringlist_ਜੋੜੋ(env_list, environ[i++]);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	ਜੇ (changeuser != ਨਿਰਾ) {
		pamenv = (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)pam_getenvlist(pamh);
		ਜੇ (pamenv) {
			ਜਦੋਂ (*pamenv) {
				/* Don't add strings unless they set a var */
				ਜੇ (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				ਨਹੀਂਤਾਂ
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#ਜੇ_ਅੰਤ

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(env->ਮੁੱਲ, "RC_", 3) == 0
				|| ਲੜੀ_ਤੁਲਨਾ(env->ਮੁੱਲ, "SSD_", 4) == 0
				|| ਲੜੀ_ਤੁਲਨਾ(env->ਮੁੱਲ, "EINFO_", 6) == 0) {
			*strchr(env->ਮੁੱਲ, '=') = '\0';
			unsetenv(env->ਮੁੱਲ);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	ਜੇ ((token = getenv("PATH"))) {
		len = ਲੜੀ_ਲੰਬਾਈ(token);
		newpath = np = xmalloc(len + 1);
		ਜਦੋਂ (token && *token) {
			p = strchr(token, ':');
			ਜੇ (p) {
				*p++ = '\0';
				ਜਦੋਂ (*p == ':')
					p++;
			}
			ਜੇ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = ਲੜੀ_ਲੰਬਾਈ(token);
				ਜੇ (np != newpath)
					*np++ = ':';
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(np, token, len);
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
	ਜੇ (redirect_stdin) {
		ਜੇ ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	ਜੇ (redirect_stdout) {
		ਜੇ ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} ਨਹੀਂਤਾਂ ਜੇ (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		ਜੇ (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	ਜੇ (redirect_stderr) {
		ਜੇ ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} ਨਹੀਂਤਾਂ ਜੇ (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		ਜੇ (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	ਜੇ (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	ਜੇ (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	ਜੇ (notify.type == NOTIFY_FD) {
		ਪੂਰਨ error;

		ਜੇ (notify.fd == notify.pipe[1]) {
			ਪੂਰਨ flags = error = fcntl(notify.fd, F_GETFD, 0);
			ਜੇ (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} ਨਹੀਂਤਾਂ {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		ਜੇ (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	ਮੁਕਤ(cmdline);
	execvp(exec, argv);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_PAM
	ਜੇ (changeuser != ਨਿਰਾ && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#ਜੇ_ਅੰਤ
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ਸਥਿਰ ਖਾਲੀ supervisor(ਅੱਖਰ *exec, ਅੱਖਰ **argv)
{
	FILE *fp;
	ਅੱਖਰ buf[2048];
	ਅੱਖਰ cmd[2048];
	ਪੂਰਨ ਗਿਣਤੀ_2;
	ਪੂਰਨ failing;
	ਪੂਰਨ health_ਸਥਿਤੀ;
	ਪੂਰਨ healthcheck_respawn;
	ਪੂਰਨ i;
	ਪੂਰਨ nkilled;
	ਪੂਰਨ sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_ਸੰਕੇਤ;
	sigset_t ਸੰਕੇਤ_2;
	ਬਣਤਰ sigaction sa;
	int64_t respawn_now = 0;
	int64_t ਪਹਿਲਾ_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&ਸੰਕੇਤ_2);
	sigdelset(&ਸੰਕੇਤ_2, SIGALRM);
	sigdelset(&ਸੰਕੇਤ_2, SIGCHLD);
	sigdelset(&ਸੰਕੇਤ_2, SIGTERM);
	sigprocmask(SIG_SETMASK, &ਸੰਕੇਤ_2, &old_ਸੰਕੇਤ);

	/* install signal  handler */
	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ਨਿਰਾ);
	sigaction(SIGCHLD, &sa, ਨਿਰਾ);
	sigaction(SIGTERM, &sa, ਨਿਰਾ);

	fp = fopen(pidfile, "w");
	ਜੇ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	ਜੇ (svcname)
		rc_service_daemon_ਸੈੱਟ(svcname, exec, (ਅਚਲ ਅੱਖਰ * ਅਚਲ *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#ਜੇ_ਅੰਤ

	/*
	 * Supervisor main loop
	 */
	ਜੇ (healthcheckdelay)
		alarm(healthcheckdelay);
	ਨਹੀਂਤਾਂ ਜੇ (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	ਜਦੋਂ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		ਜੇ (fifo_fd > 0) {
			memset(buf, 0, ਆਕਾਰ(buf));
			ਗਿਣਤੀ_2 = ਪਠ(fifo_fd, buf, ਆਕਾਰ(buf) - 1);
			close(fifo_fd);
			ਜੇ (ਗਿਣਤੀ_2 != -1)
				buf[ਗਿਣਤੀ_2] = 0;
			ਜੇ (ਗਿਣਤੀ_2 == 0)
				ਜਾਰੀ;
			ਜੇ (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			ਜੇ (strncasecmp(buf, "sig", 3) == 0) {
				ਜੇ ((ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							ਬੱਚਾ_pid);
					ਜੇ (kill(ਬੱਚਾ_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, ਬੱਚਾ_pid);
				}
			}
			ਜਾਰੀ;
		}
		ਜੇ (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			ਜੇ (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_ਸਥਿਤੀ = rc_waitpid(health_pid);
			ਜੇ (WIFEXITED(health_ਸਥਿਤੀ) && WEXITSTATUS(health_ਸਥਿਤੀ) == 0)
				alarm(healthchecktimer);
			ਨਹੀਂਤਾਂ {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, ਬੱਚਾ_pid);
				nkilled = ਲੜੀਵਾਰ_stop_ਸਮਾਂ_ਸਾਰਣੀ(applet, ਨਿਰਾ, ਨਿਰਾ, ਬੱਚਾ_pid, 0,
						stopgroup, false, false, true);
				ਜੇ (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							ਬੱਚਾ_pid, strerror(errno));
				ਨਹੀਂਤਾਂ
					healthcheck_respawn = 1;
			}
		}
		ਜੇ (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, ਬੱਚਾ_pid);
			nkilled = ਲੜੀਵਾਰ_stop_ਸਮਾਂ_ਸਾਰਣੀ(applet, ਨਿਰਾ, ਨਿਰਾ, ਬੱਚਾ_pid, 0,
					stopgroup, false, false, true);
			ਜੇ (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			ਜਾਰੀ;
		}
		wait_pid = waitpid(ਬੱਚਾ_pid, &i, WNOHANG);
		ਜੇ (wait_pid == ਬੱਚਾ_pid) {
			ਜੇ (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, ਬੱਚਾ_pid, WEXITSTATUS(i));
			ਨਹੀਂਤਾਂ ਜੇ (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, ਬੱਚਾ_pid, WTERMSIG(i));
		}
		ਜੇ (wait_pid == ਬੱਚਾ_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			ਜੇ (ਪਹਿਲਾ_spawn == 0)
				ਪਹਿਲਾ_spawn = respawn_now;
			ਜੇ ((respawn_period > 0)
					&& (respawn_now - ਪਹਿਲਾ_spawn > respawn_period)) {
				respawn_ਗਿਣਤੀ = 0;
				ਪਹਿਲਾ_spawn = 0;
			} ਨਹੀਂਤਾਂ
				respawn_ਗਿਣਤੀ++;
			ਜੇ (respawn_max > 0 && respawn_ਗਿਣਤੀ > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				ਜਾਰੀ;
			}
			sleep_for = respawn_delay + (respawn_delay_ਕਦਮ * respawn_ਗਿਣਤੀ);
			ਜੇ (respawn_delay_ਕਦਮ > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			ਜੇ (exiting)
				ਜਾਰੀ;
			ਬੱਚਾ_pid = fork();
			ਜੇ (ਬੱਚਾ_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			}
			ਜੇ (ਬੱਚਾ_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_ਸੰਕੇਤ, ਨਿਰਾ);
				memset(&sa, 0, ਆਕਾਰ(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ਨਿਰਾ);
				sigaction(SIGCHLD, &sa, ਨਿਰਾ);
				sigaction(SIGTERM, &sa, ਨਿਰਾ);
				ਬੱਚਾ_process(exec, argv);
			}
			ਜੇ (healthcheckdelay)
				alarm(healthcheckdelay);
			ਨਹੀਂਤਾਂ ਜੇ (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	ਜੇ (svcname) {
		rc_service_daemon_ਸੈੱਟ(svcname, exec, (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)argv,
				pidfile, false);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "child_pid", ਨਿਰਾ);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		ਜੇ (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	ਜੇ (pidfile)
		unlink(pidfile);
	ਜੇ (fifopath)
		unlink(fifopath);
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ opt;
	ਅੱਖਰ **c;
	ਪੂਰਨ x;
	bool ਸ਼ੁਰੂ = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	ਅੱਖਰ *exec = ਨਿਰਾ;
	ਅੱਖਰ *retry = ਨਿਰਾ;
	ਪੂਰਨ sig = SIGTERM;
	ਅੱਖਰ *home = ਨਿਰਾ;
	ਪੂਰਨ tid = 0;
	pid_t pid;
	ਅੱਖਰ *tmp;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
	ਪੂਰਨ i;
	ਪੂਰਨ n;
	ਅੱਖਰ *exec_file = ਨਿਰਾ;
	ਅੱਖਰ *varbuf = ਨਿਰਾ;
	ਅੱਖਰ numbuf[64];
	ਬਣਤਰ timespec ts;
	ਬਣਤਰ passwd *pw;
	ਬਣਤਰ group *gr;
	FILE *fp;
	mode_t numask = 022;
	ਪੂਰਨ ਬੱਚਾ_argc = 0;
	ਅੱਖਰ **ਬੱਚਾ_argv = ਨਿਰਾ;
	ਅੱਖਰ *str = ਨਿਰਾ;
	ਅੱਖਰ *cmdline = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *respawn_delay_str  = "0";
	ਅਚਲ ਅੱਖਰ *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	ਜੇ (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	ਜੇ (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	ਜੇ ((tmp = getenv("SSD_NICELEVEL")))
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ਜੇ ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d:%d", &ionicec, &ioniced);
		ਜੇ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ਜੇ (ionicec == 0)
			ioniced = 0;
		ਨਹੀਂਤਾਂ ਜੇ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ਜੇ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ਜੇ (home == ਨਿਰਾ || p == ਨਿਰਾ) {
		pw = getpwuid(getuid());
		ਜੇ (pw != ਨਿਰਾ) {
			ਜੇ (p == ਨਿਰਾ)
				setenv("USER", pw->pw_name, 1);
			ਜੇ (home == ਨਿਰਾ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	ਜੇ (svcname) {
		argc--;
		argv++;
	}
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ਪੂਰਨ *) 0)) != -1)
		ਚੋਣ (opt) {
		ਹਾਲਤ 'a':  /* --healthcheck-timer <time> */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'A':  /* --healthcheck-delay <time> */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_CAPABILITIES:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
			cap_iab = cap_iab_from_ਪਾਠ(optarg);
			ਜੇ (cap_iab == ਨਿਰਾ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ਨਹੀਂਤਾਂ
			eerrorx("Capabilities support not enabled");
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_SECBITS:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
			ਜੇ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ਨਿਰਾ;
			secbits = strtoul(optarg, &tmp, 0);
			ਜੇ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ਨਹੀਂਤਾਂ
			eerrorx("Capabilities support not enabled");
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_NO_NEW_PRIVS:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PR_ਸੈੱਟ_NO_NEW_PRIVS
			no_new_privs = true;
#ਨਹੀਂਤਾਂ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#ਜੇ_ਅੰਤ
			ਰੋਕੋ;

		ਹਾਲਤ 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'I': /* --ionice */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ਜੇ (ionicec == 0)
				ioniced = 0;
			ਨਹੀਂਤਾਂ ਜੇ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ਰੋਕੋ;

		ਹਾਲਤ 'K':  /* --stop */
			stop = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'N':  /* --nice */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'S':  /* --start */
			ਸ਼ੁਰੂ = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'e': /* --env */
			putenv(optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'g':  /* --group <group>|<gid> */
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ਨਹੀਂਤਾਂ
				gr = getgrgid((gid_t)tid);
			ਜੇ (gr == ਨਿਰਾ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ਰੋਕੋ;

		ਹਾਲਤ 'G': /* -G */
			stopgroup = true;
			ਰੋਕੋ;

		ਹਾਲਤ 'k':
			ਜੇ (parse_ਬਹੁਲਕ(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'm':  /* --respawn-max count */
			n = ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(optarg, "%d", &respawn_max);
			ਜੇ (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			ਰੋਕੋ;

		ਹਾਲਤ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'r':  /* --chroot /new/root */
			ch_ਜੜ੍ਹ = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ 'u':  /* --user <username>|<uid> */
		{
			ਅੱਖਰ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ਨਹੀਂਤਾਂ
				pw = getpwuid((uid_t)tid);

			ਜੇ (pw == ਨਿਰਾ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ਜੇ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ਜੇ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ਜੇ (gid == 0)
				gid = pw->pw_gid;

			ਜੇ (p) {
				tmp = strsep (&p, ":");
				ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ਨਹੀਂਤਾਂ
					gr = getgrgid((gid_t) tid);

				ਜੇ (gr == ਨਿਰਾ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ਰੋਕੋ;

		ਹਾਲਤ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ '3':  /* --reexec */
			reexec = true;
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_RESPAWN_DELAY_ਕਦਮ:
			respawn_delay_ਕਦਮ = parse_duration(optarg);
			ਜੇ (respawn_delay_ਕਦਮ < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			ਰੋਕੋ;

		ਹਾਲਤ LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			ਜੇ (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	ਜੇ (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	ਜੇ (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	ਜੇ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ਜੇ (ch_ਜੜ੍ਹ && *ch_ਜੜ੍ਹ == '~')
		ch_ਜੜ੍ਹ = expand_home(home, ch_ਜੜ੍ਹ);

	umask(numask);
	ਜੇ (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	ਜੇ (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	ਜੇ (reexec) {
		str = rc_service_ਮੁੱਲ_get(svcname, "argc");
		ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(str, "%d", &ਬੱਚਾ_argc);
		ਬੱਚਾ_argv = xmalloc((ਬੱਚਾ_argc + 1) * ਆਕਾਰ(ਅੱਖਰ *));
		memset(ਬੱਚਾ_argv, 0, (ਬੱਚਾ_argc + 1) * ਆਕਾਰ(ਅੱਖਰ *));
		ਲਈ (x = 0; x < ਬੱਚਾ_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_ਮੁੱਲ_get(svcname, varbuf);
			ਬੱਚਾ_argv[x] = str;
			ਮੁਕਤ(varbuf);
			varbuf = ਨਿਰਾ;
		}
		ਮੁਕਤ(str);
		str = rc_service_ਮੁੱਲ_get(svcname, "child_pid");
		ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(str, "%d", &ਬੱਚਾ_pid);
		ਮੁਕਤ(str);
		exec = rc_service_ਮੁੱਲ_get(svcname, "exec");
		pidfile = rc_service_ਮੁੱਲ_get(svcname, "pidfile");
		retry = rc_service_ਮੁੱਲ_get(svcname, "retry");
		ਜੇ (retry) {
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, retry, sig);
			rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "retry", retry);
		} ਨਹੀਂਤਾਂ
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, ਨਿਰਾ, sig);

		str = rc_service_ਮੁੱਲ_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_ਮੁੱਲ_get(svcname, "respawn_delay_step");
		ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(str, "%"SCNd64, &respawn_delay_ਕਦਮ);
		str = rc_service_ਮੁੱਲ_get(svcname, "respawn_delay_cap");
		ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_ਮੁੱਲ_get(svcname, "respawn_max");
		ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(str, "%d", &respawn_max);
		supervisor(exec, ਬੱਚਾ_argv);
	} ਨਹੀਂਤਾਂ ਜੇ (ਸ਼ੁਰੂ) {
		ਜੇ (exec) {
			ਜੇ (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			ਜੇ (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				ਜੇ (ch_ਜੜ੍ਹ)
					xasprintf(&exec_file, "%s/%s", ch_ਜੜ੍ਹ, exec);
				ਨਹੀਂਤਾਂ
					xasprintf(&exec_file, "%s", exec);
			} ਨਹੀਂਤਾਂ {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = ਨਿਰਾ;
				ਜਦੋਂ ((token = strsep(&p, ":"))) {
					ਜੇ (ch_ਜੜ੍ਹ)
						xasprintf(&exec_file, "%s/%s/%s", ch_ਜੜ੍ਹ, token, exec);
					ਨਹੀਂਤਾਂ
						xasprintf(&exec_file, "%s/%s", token, exec);
					ਜੇ (exec_file && access(exec_file, F_OK) == 0)
						ਰੋਕੋ;
					ਮੁਕਤ(exec_file);
					exec_file = ਨਿਰਾ;
				}
				ਮੁਕਤ(tmp);
			}
			ਜੇ (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				ਮੁਕਤ(exec_file);
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
			}
		} ਨਹੀਂਤਾਂ
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		ਜੇ (pid != -1)
			ਜੇ (do_stop(applet, exec, (ਅਚਲ ਅੱਖਰ * ਅਚਲ *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		ਜੇ (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		ਜੇ (respawn_delay_ਕਦਮ > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		ਜੇ (retry) {
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, retry, sig);
			rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "retry", retry);
		} ਨਹੀਂਤਾਂ
			parse_ਸਮਾਂ_ਸਾਰਣੀ(applet, ਨਿਰਾ, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		ਮੁਕਤ(cmdline);
		cmdline = ਨਿਰਾ;

		/* Remove existing pidfile */
		ਜੇ (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		ਜੇ (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "pidfile", pidfile);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "respawn_delay", respawn_delay_str);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, ਆਕਾਰ(numbuf), "%"PRId64, respawn_delay_ਕਦਮ);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, ਆਕਾਰ(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, ਆਕਾਰ(numbuf), "%i", respawn_max);
		rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "respawn_max", numbuf);
		ਬੱਚਾ_pid = fork();
		ਜੇ (ਬੱਚਾ_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ਜੇ (ਬੱਚਾ_pid != 0)
			ਬਾਹਰ_ਨਿਕਲੋ(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#ਜੇ_ਅੰਤ
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		ਜੇ (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		ਬੱਚਾ_pid = fork();
		ਜੇ (ਬੱਚਾ_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ਨਹੀਂਤਾਂ ਜੇ (ਬੱਚਾ_pid != 0) {
			ਜੇ (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			ਜਦੋਂ (c && *c) {
				varbuf = ਨਿਰਾ;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, varbuf, *c);
				ਮੁਕਤ(varbuf);
				varbuf = ਨਿਰਾ;
				x++;
				c++;
			}
			snprintf(numbuf, ਆਕਾਰ(numbuf), "%d", x);
			rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "argc", numbuf);
			rc_service_ਮੁੱਲ_ਸੈੱਟ(svcname, "exec", exec);
			supervisor(exec, argv);
		} ਨਹੀਂਤਾਂ
			ਬੱਚਾ_process(exec, argv);
	} ਨਹੀਂਤਾਂ ਜੇ (stop) {
		pid = get_pid(applet, pidfile);
		ਜੇ (pid != -1) {
			i = kill(pid, SIGTERM);
			ਜੇ (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			ਨਹੀਂਤਾਂ {
				/* wait for the supervisor to go down */
				ਜਦੋਂ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ਨਿਰਾ);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ਜੇ (pidfile)
			unlink(pidfile);
		ਜੇ (svcname) {
			rc_service_daemon_ਸੈੱਟ(svcname, exec,
			    (ਅਚਲ ਅੱਖਰ *ਅਚਲ *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	} ਨਹੀਂਤਾਂ ਜੇ (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		ਜੇ (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = ਲਿਖ(fifo_fd, str, ਲੜੀ_ਲੰਬਾਈ(str));
		ਜੇ (x == -1) {
			ਮੁਕਤ(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		ਮੁਕਤ(tmp);
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	}
}
