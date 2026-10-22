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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/resource.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
#ಅನ್ತರ್ಭಾವ <security/pam_appl.h>

/* We are not supporting authentication conversations */
ಸ್ಥಿರ ರಚನೆ pam_conv conv = { ಖಾಲಿ, ಖಾಲಿ};
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
#ಅನ್ತರ್ಭಾವ <sys/capability.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "plugin.h"
#ಅನ್ತರ್ಭಾವ "schedules.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ಎಣಿಕೆ {
  /* This has to come first so following values stay in the 0x100+ range. */
  LONGOPT_ತಳ = 0x100,

  LONGOPT_CAPABILITIES,
  LONGOPT_OOM_SCORE_ADJ,
  LONGOPT_NO_NEW_PRIVS,
  LONGOPT_SECBITS,
  LONGOPT_STDERR_LOGGER,
  LONGOPT_STDOUT_LOGGER,
  LONGOPT_NOTIFY,
  LONGOPT_RESPAWN_DELAY_ಹೆಜ್ಜೆ,
  LONGOPT_RESPAWN_DELAY_CAP,
};

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "healthcheck-timer",        1, ಖಾಲಿ, 'a'},
	{ "healthcheck-delay",        1, ಖಾಲಿ, 'A'},
	{ "capabilities", 1, ಖಾಲಿ, LONGOPT_CAPABILITIES},
	{ "secbits",      1, ಖಾಲಿ, LONGOPT_SECBITS},
	{ "no-new-privs", 0, ಖಾಲಿ, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, ಖಾಲಿ, 'D'},
	{ "respawn-delay-step",   1, ಖಾಲಿ, LONGOPT_RESPAWN_DELAY_ಹೆಜ್ಜೆ},
	{ "respawn-delay-cap",    1, ಖಾಲಿ, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, ಖಾಲಿ, 'd'},
	{ "env",          1, ಖಾಲಿ, 'e'},
	{ "group",        1, ಖಾಲಿ, 'g'},
	{ "stop-group",   0, ಖಾಲಿ, 'G'},
	{ "ionice",       1, ಖಾಲಿ, 'I'},
	{ "stop",         0, ಖಾಲಿ, 'K'},
	{ "umask",        1, ಖಾಲಿ, 'k'},
	{ "respawn-max",    1, ಖಾಲಿ, 'm'},
	{ "nicelevel",    1, ಖಾಲಿ, 'N'},
	{ "oom-score-adj",1, ಖಾಲಿ, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, ಖಾಲಿ, 'p'},
	{ "respawn-period",        1, ಖಾಲಿ, 'P'},
	{ "retry",       1, ಖಾಲಿ, 'R'},
	{ "chroot",       1, ಖಾಲಿ, 'r'},
	{ "signal",       1, ಖಾಲಿ, 's'},
	{ "start",        0, ಖಾಲಿ, 'S'},
	{ "user",         1, ಖಾಲಿ, 'u'},
	{ "stdin",        1, ಖಾಲಿ, '0'},
	{ "stdout",       1, ಖಾಲಿ, '1'},
	{ "stderr",       1, ಖಾಲಿ, '2'},
	{ "stdout-logger",1, ಖಾಲಿ, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, ಖಾಲಿ, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, ಖಾಲಿ, '3'},
	{ "notify",       1, ಖಾಲಿ, LONGOPT_NOTIFY},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ healthcheckdelay = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ healthchecktimer = 0;
ಸ್ಥಿರ ಅಸ್ಥಿರ sig_atomic_t do_healthcheck = 0;
ಸ್ಥಿರ ಅಸ್ಥಿರ sig_atomic_t exiting = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nicelevel = INT_MIN;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ionicec = -1;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ioniced = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ oom_score_adj = INT_MIN;
ಸ್ಥಿರ ಅಕ್ಷರ *changeuser, *ch_ಬೇರು, *ch_dir;
ಸ್ಥಿರ uid_t uid = 0;
ಸ್ಥಿರ gid_t gid = 0;
ಸ್ಥಿರ bool stopgroup = false;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devnull_fd = -1;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stdin_fd;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stdout_fd;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stderr_fd;
ಸ್ಥಿರ ರಚನೆ notify notify;
ಸ್ಥಿರ ಅಕ್ಷರ *redirect_stdin = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *redirect_stderr = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *redirect_stdout = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *stderr_process = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *stdout_process = ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tty_fd = -1;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ pid_t ಮಗು_pid;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ respawn_ಎಣಿಕೆ = 0;
ಸ್ಥಿರ int64_t respawn_delay; /* default set inside main() */
ಸ್ಥಿರ int64_t respawn_delay_ಹೆಜ್ಜೆ = TM_MS(128);
ಸ್ಥಿರ int64_t respawn_delay_cap = TM_SEC(30);
ಸ್ಥಿರ int64_t respawn_period; /* default set inside main() */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ respawn_max = 10;
ಸ್ಥಿರ ಅಕ್ಷರ *fifopath = ಖಾಲಿ;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fifo_fd = 0;
ಸ್ಥಿರ ಅಕ್ಷರ *pidfile = ಖಾಲಿ;
ಸ್ಥಿರ ಅಕ್ಷರ *svcname = ಖಾಲಿ;
ಸ್ಥಿರ bool verbose = false;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
ಸ್ಥಿರ cap_iab_t cap_iab = ಖಾಲಿ;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ secbits = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PR_ಗಣ_NO_NEW_PRIVS
ಸ್ಥಿರ bool no_new_privs = false;
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಅಕ್ಷರ **environ;

#ಆದರೆ !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(__DragonFly__)
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ ioprio_ಗಣ(ಪೂರ್ಣಾಂಕ which RC_ಬಳಸದ, ಪೂರ್ಣಾಂಕ who RC_ಬಳಸದ,
			     ಪೂರ್ಣಾಂಕ ioprio RC_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SYS_ioprio_set
	ಹಿಂತಿರುಗಿ syscall(SYS_ioprio_set, which, who, ioprio);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ cleanup(ಶೂನ್ಯ)
{
	ಬಿಡುಗಡೆ(changeuser);
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ re_exec_supervisor(ಶೂನ್ಯ)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(ಅಕ್ಷರ *) ಖಾಲಿ);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
}

ಸ್ಥಿರ ಶೂನ್ಯ handle_signal(ಪೂರ್ಣಾಂಕ sig)
{
	ಪೂರ್ಣಾಂಕ serrno = errno;
	pid_t pid;

	ಆಯ್ಕೆ (sig) {
	ಸ್ಥಿತಿ SIGALRM:
		do_healthcheck = 1;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SIGCHLD:
		ಆದರೆ (exiting)
			ಆಗ (waitpid((pid_t)(-1), ಖಾಲಿ, WNOHANG) > 0) {}
		ಇಲ್ಲದಿದ್ದರೆ {
			ಆಗ ((pid = waitpid((pid_t)(-1), ಖಾಲಿ, WNOHANG|WNOWAIT)) > 0) {
				ಆದರೆ (pid == ಮಗು_pid)
					ನಿಲ್ಲಿಸು;
				pid = waitpid(pid, ಖಾಲಿ, WNOHANG);
			}
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ SIGTERM:
		exiting = 1;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

ಸ್ಥಿರ ಅಕ್ಷರ * expand_home(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *home, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *opath, *ppath, *p, *nh;
	ರಚನೆ passwd *pw;

	ಆದರೆ (!ಮಾರ್ಗ || *ಮಾರ್ಗ != '~')
		ಹಿಂತಿರುಗಿ xstrdup(ಮಾರ್ಗ);

	opath = ppath = xstrdup(ಮಾರ್ಗ);
	ಆದರೆ (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ಆದರೆ (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ಆದರೆ (pw) {
			home = pw->pw_dir;
			ppath = p;
			ಆದರೆ (ppath)
				*ppath = '/';
		} ಇಲ್ಲದಿದ್ದರೆ
			home = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ
		ppath++;

	ಆದರೆ (!home) {
		ಬಿಡುಗಡೆ(opath);
		ಹಿಂತಿರುಗಿ xstrdup(ಮಾರ್ಗ);
	}
	ಆದರೆ (!ppath) {
		ಬಿಡುಗಡೆ(opath);
		ಹಿಂತಿರುಗಿ xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	ಬಿಡುಗಡೆ(opath);
	ಹಿಂತಿರುಗಿ nh;
}

ಸ್ಥಿರ ಅಕ್ಷರ *make_cmdline(ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ **c;
	ಅಕ್ಷರ *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	ಗಾಗಿ (c = argv; c && *c; c++) {
		fputs(*c, mem);
		ಆದರೆ (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	ಹಿಂತಿರುಗಿ cmdline;
}

ಸ್ಥಿರ pid_t exec_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd)
{
	ಅಕ್ಷರ *file;
	pid_t pid = -1;
	sigset_t ತುಂಬಿದ;
	sigset_t old;
	ರಚನೆ sigaction sa;

	file = rc_service_resolve(svcname);
	ಆದರೆ (access(file, F_OK) != 0) {
		ಬಿಡುಗಡೆ(file);
		ಹಿಂತಿರುಗಿ 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, ಗಾತ್ರ (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&ತುಂಬಿದ);
	sigprocmask(SIG_SETMASK, &ತುಂಬಿದ, &old);

	pid = fork();
	ಆದರೆ (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, ಖಾಲಿ);
		sigaction(SIGHUP, &sa, ಖಾಲಿ);
		sigaction(SIGINT, &sa, ಖಾಲಿ);
		sigaction(SIGQUIT, &sa, ಖಾಲಿ);
		sigaction(SIGTERM, &sa, ಖಾಲಿ);
		sigaction(SIGUSR1, &sa, ಖಾಲಿ);
		sigaction(SIGWINCH, &sa, ಖಾಲಿ);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);

		/* Safe to run now */
		execl(file, file, cmd, (ಅಕ್ಷರ *) ಖಾಲಿ);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ಆದರೆ (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
	ಬಿಡುಗಡೆ(file);
	ಹಿಂತಿರುಗಿ pid;
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ ಮಗು_process(ಅಕ್ಷರ *exec, ಅಕ್ಷರ **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
	size_t len;
	ಅಕ್ಷರ *newpath;
	ಅಕ್ಷರ *np;
	ಅಕ್ಷರ *cmdline = ಖಾಲಿ;
	time_t ಆರಂಭ_ಸಮಯ;
	ಅಕ್ಷರ ಆರಂಭ_ಎಣಿಕೆ_string[20];
	ಅಕ್ಷರ ಆರಂಭ_ಸಮಯ_string[20];
	FILE *fp;
	gid_t ಗುಂಪು_buf[32], *ಗುಂಪು_list = ಗುಂಪು_buf;
	ಪೂರ್ಣಾಂಕ ಗುಂಪು_ಎಣಿಕೆ = ARRAY_ಗಾತ್ರ(ಗುಂಪು_buf);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	pam_handle_t *pamh = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ pamr;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *pamenv = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	setsid();

	ಆದರೆ (svcname) {
		ಆರಂಭ_ಸಮಯ = time(ಖಾಲಿ);
		from_ಸಮಯ_t(ಆರಂಭ_ಸಮಯ_string, ಆರಂಭ_ಸಮಯ);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "start_time", ಆರಂಭ_ಸಮಯ_string);
		sprintf(ಆರಂಭ_ಎಣಿಕೆ_string, "%i", respawn_ಎಣಿಕೆ);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "start_count", ಆರಂಭ_ಎಣಿಕೆ_string);
		sprintf(ಆರಂಭ_ಎಣಿಕೆ_string, "%d", getpid());
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "child_pid", ಆರಂಭ_ಎಣಿಕೆ_string);
	}

	ಆದರೆ (nicelevel != INT_MIN) {
		ಆದರೆ (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		ಆದರೆ (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	ಆದರೆ (ionicec != -1 && ioprio_ಗಣ(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	ಆದರೆ (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		ಆದರೆ (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	ಆದರೆ (changeuser != ಖಾಲಿ) {
		pamr = pam_ಆರಂಭ("supervise-daemon",
		    changeuser, &conv, &pamh);

		ಆದರೆ (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		ಆದರೆ (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		ಆದರೆ (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (changeuser && getgrouplist(changeuser, gid, ಗುಂಪು_list, &ಗುಂಪು_ಎಣಿಕೆ) < 0) {
		ಗುಂಪು_list = xmalloc(ಗುಂಪು_ಎಣಿಕೆ * ಗಾತ್ರ(*ಗುಂಪು_list));
		ಆದರೆ (getgrouplist(changeuser, gid, ಗುಂಪು_list, &ಗುಂಪು_ಎಣಿಕೆ) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	ಆದರೆ (ch_ಬೇರು && chroot(ch_ಬೇರು) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_ಬೇರು, strerror(errno));

	ಆದರೆ (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	ಆದರೆ (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	ಆದರೆ (changeuser && setgroups(ಗುಂಪು_ಎಣಿಕೆ, ಗುಂಪು_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	ಆದರೆ (ಗುಂಪು_list != ಗುಂಪು_buf)
		ಬಿಡುಗಡೆ(ಗುಂಪು_list);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಆದರೆ (uid && cap_setuid(uid))
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (uid && setuid(uid))
#ಆದರೆ_ಅಂತ್ಯ
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
	ಆದರೆ (cap_iab != ಖಾಲಿ) {
		i = cap_iab_ಗಣ_proc(cap_iab);

		ಆದರೆ (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		ಆದರೆ (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	ಆದರೆ (secbits != 0) {
		ಆದರೆ (cap_ಗಣ_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PR_ಗಣ_NO_NEW_PRIVS
	ಆದರೆ (no_new_privs) {
		ಆದರೆ (prctl(PR_ಗಣ_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
			eerrorx("Could not set No New Privs flag: %s", strerror(errno));
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* remove the controlling tty */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#ಆದರೆ_ಅಂತ್ಯ

	/* Clean the environment of any RC_ variables */
	env_list = rc_stringlist_new();
	i = 0;
	ಆಗ (environ[i])
		rc_stringlist_ಕೂಡಿಸು(env_list, environ[i++]);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	ಆದರೆ (changeuser != ಖಾಲಿ) {
		pamenv = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)pam_getenvlist(pamh);
		ಆದರೆ (pamenv) {
			ಆಗ (*pamenv) {
				/* Don't add strings unless they set a var */
				ಆದರೆ (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				ಇಲ್ಲದಿದ್ದರೆ
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(env->ಮೌಲ್ಯ, "RC_", 3) == 0
				|| ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(env->ಮೌಲ್ಯ, "SSD_", 4) == 0
				|| ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(env->ಮೌಲ್ಯ, "EINFO_", 6) == 0) {
			*strchr(env->ಮೌಲ್ಯ, '=') = '\0';
			unsetenv(env->ಮೌಲ್ಯ);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	ಆದರೆ ((token = getenv("PATH"))) {
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token);
		newpath = np = xmalloc(len + 1);
		ಆಗ (token && *token) {
			p = strchr(token, ':');
			ಆದರೆ (p) {
				*p++ = '\0';
				ಆಗ (*p == ':')
					p++;
			}
			ಆದರೆ (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(token);
				ಆದರೆ (np != newpath)
					*np++ = ':';
				ಸ್ಮರಣೆ_ನಕಲು(np, token, len);
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
	ಆದರೆ (redirect_stdin) {
		ಆದರೆ ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	ಆದರೆ (redirect_stdout) {
		ಆದರೆ ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		ಆದರೆ (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	ಆದರೆ (redirect_stderr) {
		ಆದರೆ ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		ಆದರೆ (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	ಆದರೆ (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	ಆದರೆ (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	ಆದರೆ (notify.type == NOTIFY_FD) {
		ಪೂರ್ಣಾಂಕ error;

		ಆದರೆ (notify.fd == notify.pipe[1]) {
			ಪೂರ್ಣಾಂಕ flags = error = fcntl(notify.fd, F_GETFD, 0);
			ಆದರೆ (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} ಇಲ್ಲದಿದ್ದರೆ {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		ಆದರೆ (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	ಬಿಡುಗಡೆ(cmdline);
	execvp(exec, argv);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_PAM
	ಆದರೆ (changeuser != ಖಾಲಿ && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#ಆದರೆ_ಅಂತ್ಯ
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ supervisor(ಅಕ್ಷರ *exec, ಅಕ್ಷರ **argv)
{
	FILE *fp;
	ಅಕ್ಷರ buf[2048];
	ಅಕ್ಷರ cmd[2048];
	ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
	ಪೂರ್ಣಾಂಕ failing;
	ಪೂರ್ಣಾಂಕ health_ಸ್ಥಿತಿ;
	ಪೂರ್ಣಾಂಕ healthcheck_respawn;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ nkilled;
	ಪೂರ್ಣಾಂಕ sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_ಸಂಕೇತಗಳು;
	sigset_t ಸಂಕೇತಗಳು;
	ರಚನೆ sigaction sa;
	int64_t respawn_now = 0;
	int64_t ಮೊದಲ_spawn = 0;
	int64_t sleep_for;

	/* block all signals we do not handle */
	sigfillset(&ಸಂಕೇತಗಳು);
	sigdelset(&ಸಂಕೇತಗಳು, SIGALRM);
	sigdelset(&ಸಂಕೇತಗಳು, SIGCHLD);
	sigdelset(&ಸಂಕೇತಗಳು, SIGTERM);
	sigprocmask(SIG_SETMASK, &ಸಂಕೇತಗಳು, &old_ಸಂಕೇತಗಳು);

	/* install signal  handler */
	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, ಖಾಲಿ);
	sigaction(SIGCHLD, &sa, ಖಾಲಿ);
	sigaction(SIGTERM, &sa, ಖಾಲಿ);

	fp = fopen(pidfile, "w");
	ಆದರೆ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	ಆದರೆ (svcname)
		rc_service_daemon_ಗಣ(svcname, exec, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * Supervisor main loop
	 */
	ಆದರೆ (healthcheckdelay)
		alarm(healthcheckdelay);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	ಆಗ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		ಆದರೆ (fifo_fd > 0) {
			memset(buf, 0, ಗಾತ್ರ(buf));
			ಎಣಿಕೆ_2 = ಪಠ(fifo_fd, buf, ಗಾತ್ರ(buf) - 1);
			close(fifo_fd);
			ಆದರೆ (ಎಣಿಕೆ_2 != -1)
				buf[ಎಣಿಕೆ_2] = 0;
			ಆದರೆ (ಎಣಿಕೆ_2 == 0)
				ಮುಂದುವರಿ;
			ಆದರೆ (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			ಆದರೆ (strncasecmp(buf, "sig", 3) == 0) {
				ಆದರೆ ((ಅಕ್ಷರಸರಣಿ_ಓದು(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							ಮಗು_pid);
					ಆದರೆ (kill(ಮಗು_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, ಮಗು_pid);
				}
			}
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			ಆದರೆ (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_ಸ್ಥಿತಿ = rc_waitpid(health_pid);
			ಆದರೆ (WIFEXITED(health_ಸ್ಥಿತಿ) && WEXITSTATUS(health_ಸ್ಥಿತಿ) == 0)
				alarm(healthchecktimer);
			ಇಲ್ಲದಿದ್ದರೆ {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, ಮಗು_pid);
				nkilled = ಸತತ_stop_ವೇಳಾಪಟ್ಟಿ(applet, ಖಾಲಿ, ಖಾಲಿ, ಮಗು_pid, 0,
						stopgroup, false, false, true);
				ಆದರೆ (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							ಮಗು_pid, strerror(errno));
				ಇಲ್ಲದಿದ್ದರೆ
					healthcheck_respawn = 1;
			}
		}
		ಆದರೆ (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, ಮಗು_pid);
			nkilled = ಸತತ_stop_ವೇಳಾಪಟ್ಟಿ(applet, ಖಾಲಿ, ಖಾಲಿ, ಮಗು_pid, 0,
					stopgroup, false, false, true);
			ಆದರೆ (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			ಮುಂದುವರಿ;
		}
		wait_pid = waitpid(ಮಗು_pid, &i, WNOHANG);
		ಆದರೆ (wait_pid == ಮಗು_pid) {
			ಆದರೆ (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, ಮಗು_pid, WEXITSTATUS(i));
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, ಮಗು_pid, WTERMSIG(i));
		}
		ಆದರೆ (wait_pid == ಮಗು_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			ಆದರೆ (ಮೊದಲ_spawn == 0)
				ಮೊದಲ_spawn = respawn_now;
			ಆದರೆ ((respawn_period > 0)
					&& (respawn_now - ಮೊದಲ_spawn > respawn_period)) {
				respawn_ಎಣಿಕೆ = 0;
				ಮೊದಲ_spawn = 0;
			} ಇಲ್ಲದಿದ್ದರೆ
				respawn_ಎಣಿಕೆ++;
			ಆದರೆ (respawn_max > 0 && respawn_ಎಣಿಕೆ > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				ಮುಂದುವರಿ;
			}
			sleep_for = respawn_delay + (respawn_delay_ಹೆಜ್ಜೆ * respawn_ಎಣಿಕೆ);
			ಆದರೆ (respawn_delay_ಹೆಜ್ಜೆ > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			ಆದರೆ (exiting)
				ಮುಂದುವರಿ;
			ಮಗು_pid = fork();
			ಆದರೆ (ಮಗು_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			}
			ಆದರೆ (ಮಗು_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_ಸಂಕೇತಗಳು, ಖಾಲಿ);
				memset(&sa, 0, ಗಾತ್ರ(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, ಖಾಲಿ);
				sigaction(SIGCHLD, &sa, ಖಾಲಿ);
				sigaction(SIGTERM, &sa, ಖಾಲಿ);
				ಮಗು_process(exec, argv);
			}
			ಆದರೆ (healthcheckdelay)
				alarm(healthcheckdelay);
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	ಆದರೆ (svcname) {
		rc_service_daemon_ಗಣ(svcname, exec, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)argv,
				pidfile, false);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "child_pid", ಖಾಲಿ);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		ಆದರೆ (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	ಆದರೆ (pidfile)
		unlink(pidfile);
	ಆದರೆ (fifopath)
		unlink(fifopath);
	ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ opt;
	ಅಕ್ಷರ **c;
	ಪೂರ್ಣಾಂಕ x;
	bool ಆರಂಭ = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	ಅಕ್ಷರ *exec = ಖಾಲಿ;
	ಅಕ್ಷರ *retry = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ sig = SIGTERM;
	ಅಕ್ಷರ *home = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ tid = 0;
	pid_t pid;
	ಅಕ್ಷರ *tmp;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ n;
	ಅಕ್ಷರ *exec_file = ಖಾಲಿ;
	ಅಕ್ಷರ *varbuf = ಖಾಲಿ;
	ಅಕ್ಷರ numbuf[64];
	ರಚನೆ timespec ts;
	ರಚನೆ passwd *pw;
	ರಚನೆ group *gr;
	FILE *fp;
	mode_t numask = 022;
	ಪೂರ್ಣಾಂಕ ಮಗು_argc = 0;
	ಅಕ್ಷರ **ಮಗು_argv = ಖಾಲಿ;
	ಅಕ್ಷರ *str = ಖಾಲಿ;
	ಅಕ್ಷರ *cmdline = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *respawn_delay_str  = "0";
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	ಆದರೆ (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	ಆದರೆ (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	ಆದರೆ ((tmp = getenv("SSD_NICELEVEL")))
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ಆದರೆ ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d:%d", &ionicec, &ioniced);
		ಆದರೆ (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ಆದರೆ (ionicec == 0)
			ioniced = 0;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ಆದರೆ ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ಆದರೆ (home == ಖಾಲಿ || p == ಖಾಲಿ) {
		pw = getpwuid(getuid());
		ಆದರೆ (pw != ಖಾಲಿ) {
			ಆದರೆ (p == ಖಾಲಿ)
				setenv("USER", pw->pw_name, 1);
			ಆದರೆ (home == ಖಾಲಿ) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	ಆದರೆ (svcname) {
		argc--;
		argv++;
	}
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'a':  /* --healthcheck-timer <time> */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'A':  /* --healthcheck-delay <time> */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_CAPABILITIES:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
			cap_iab = cap_iab_from_ಪಠ್ಯ(optarg);
			ಆದರೆ (cap_iab == ಖಾಲಿ)
				eerrorx("Could not parse iab: %s", strerror(errno));
#ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("Capabilities support not enabled");
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_SECBITS:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __linux__
			ಆದರೆ (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = ಖಾಲಿ;
			secbits = strtoul(optarg, &tmp, 0);
			ಆದರೆ (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("Capabilities support not enabled");
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_NO_NEW_PRIVS:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PR_ಗಣ_NO_NEW_PRIVS
			no_new_privs = true;
#ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#ಆದರೆ_ಅಂತ್ಯ
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'I': /* --ionice */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ಆದರೆ (ionicec == 0)
				ioniced = 0;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'K':  /* --stop */
			stop = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'N':  /* --nice */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'S':  /* --start */
			ಆರಂಭ = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'e': /* --env */
			putenv(optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'g':  /* --group <group>|<gid> */
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			ಇಲ್ಲದಿದ್ದರೆ
				gr = getgrgid((gid_t)tid);
			ಆದರೆ (gr == ಖಾಲಿ)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'G': /* -G */
			stopgroup = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'k':
			ಆದರೆ (parse_ಬಹುಲಕ(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'm':  /* --respawn-max count */
			n = ಅಕ್ಷರಸರಣಿ_ಓದು(optarg, "%d", &respawn_max);
			ಆದರೆ (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'r':  /* --chroot /new/root */
			ch_ಬೇರು = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ 'u':  /* --user <username>|<uid> */
		{
			ಅಕ್ಷರ dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			ಇಲ್ಲದಿದ್ದರೆ
				pw = getpwuid((uid_t)tid);

			ಆದರೆ (pw == ಖಾಲಿ)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ಆದರೆ (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ಆದರೆ (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ಆದರೆ (gid == 0)
				gid = pw->pw_gid;

			ಆದರೆ (p) {
				tmp = strsep (&p, ":");
				ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				ಇಲ್ಲದಿದ್ದರೆ
					gr = getgrgid((gid_t) tid);

				ಆದರೆ (gr == ಖಾಲಿ)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '3':  /* --reexec */
			reexec = true;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_RESPAWN_DELAY_ಹೆಜ್ಜೆ:
			respawn_delay_ಹೆಜ್ಜೆ = parse_duration(optarg);
			ಆದರೆ (respawn_delay_ಹೆಜ್ಜೆ < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			ನಿಲ್ಲಿಸು;

		ಸ್ಥಿತಿ LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			ಆದರೆ (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			ನಿಲ್ಲಿಸು;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	ಆದರೆ (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	ಆದರೆ (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	ಆದರೆ (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ಆದರೆ (ch_ಬೇರು && *ch_ಬೇರು == '~')
		ch_ಬೇರು = expand_home(home, ch_ಬೇರು);

	umask(numask);
	ಆದರೆ (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	ಆದರೆ (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	ಆದರೆ (reexec) {
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "argc");
		ಅಕ್ಷರಸರಣಿ_ಓದು(str, "%d", &ಮಗು_argc);
		ಮಗು_argv = xmalloc((ಮಗು_argc + 1) * ಗಾತ್ರ(ಅಕ್ಷರ *));
		memset(ಮಗು_argv, 0, (ಮಗು_argc + 1) * ಗಾತ್ರ(ಅಕ್ಷರ *));
		ಗಾಗಿ (x = 0; x < ಮಗು_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_ಮೌಲ್ಯ_get(svcname, varbuf);
			ಮಗು_argv[x] = str;
			ಬಿಡುಗಡೆ(varbuf);
			varbuf = ಖಾಲಿ;
		}
		ಬಿಡುಗಡೆ(str);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "child_pid");
		ಅಕ್ಷರಸರಣಿ_ಓದು(str, "%d", &ಮಗು_pid);
		ಬಿಡುಗಡೆ(str);
		exec = rc_service_ಮೌಲ್ಯ_get(svcname, "exec");
		pidfile = rc_service_ಮೌಲ್ಯ_get(svcname, "pidfile");
		retry = rc_service_ಮೌಲ್ಯ_get(svcname, "retry");
		ಆದರೆ (retry) {
			parse_ವೇಳಾಪಟ್ಟಿ(applet, retry, sig);
			rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "retry", retry);
		} ಇಲ್ಲದಿದ್ದರೆ
			parse_ವೇಳಾಪಟ್ಟಿ(applet, ಖಾಲಿ, sig);

		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_delay_step");
		ಅಕ್ಷರಸರಣಿ_ಓದು(str, "%"SCNd64, &respawn_delay_ಹೆಜ್ಜೆ);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_delay_cap");
		ಅಕ್ಷರಸರಣಿ_ಓದು(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_ಮೌಲ್ಯ_get(svcname, "respawn_max");
		ಅಕ್ಷರಸರಣಿ_ಓದು(str, "%d", &respawn_max);
		supervisor(exec, ಮಗು_argv);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಆರಂಭ) {
		ಆದರೆ (exec) {
			ಆದರೆ (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			ಆದರೆ (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				ಆದರೆ (ch_ಬೇರು)
					xasprintf(&exec_file, "%s/%s", ch_ಬೇರು, exec);
				ಇಲ್ಲದಿದ್ದರೆ
					xasprintf(&exec_file, "%s", exec);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = ಖಾಲಿ;
				ಆಗ ((token = strsep(&p, ":"))) {
					ಆದರೆ (ch_ಬೇರು)
						xasprintf(&exec_file, "%s/%s/%s", ch_ಬೇರು, token, exec);
					ಇಲ್ಲದಿದ್ದರೆ
						xasprintf(&exec_file, "%s/%s", token, exec);
					ಆದರೆ (exec_file && access(exec_file, F_OK) == 0)
						ನಿಲ್ಲಿಸು;
					ಬಿಡುಗಡೆ(exec_file);
					exec_file = ಖಾಲಿ;
				}
				ಬಿಡುಗಡೆ(tmp);
			}
			ಆದರೆ (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				ಬಿಡುಗಡೆ(exec_file);
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
			}
		} ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		ಆದರೆ (pid != -1)
			ಆದರೆ (do_stop(applet, exec, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		ಆದರೆ (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		ಆದರೆ (respawn_delay_ಹೆಜ್ಜೆ > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		ಆದರೆ (retry) {
			parse_ವೇಳಾಪಟ್ಟಿ(applet, retry, sig);
			rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "retry", retry);
		} ಇಲ್ಲದಿದ್ದರೆ
			parse_ವೇಳಾಪಟ್ಟಿ(applet, ಖಾಲಿ, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		ಬಿಡುಗಡೆ(cmdline);
		cmdline = ಖಾಲಿ;

		/* Remove existing pidfile */
		ಆದರೆ (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		ಆದರೆ (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "pidfile", pidfile);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "respawn_delay", respawn_delay_str);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, ಗಾತ್ರ(numbuf), "%"PRId64, respawn_delay_ಹೆಜ್ಜೆ);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, ಗಾತ್ರ(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, ಗಾತ್ರ(numbuf), "%i", respawn_max);
		rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "respawn_max", numbuf);
		ಮಗು_pid = fork();
		ಆದರೆ (ಮಗು_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ಆದರೆ (ಮಗು_pid != 0)
			ನಿರ್ಗಮಿಸು(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#ಆದರೆ_ಅಂತ್ಯ
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		ಆದರೆ (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		ಮಗು_pid = fork();
		ಆದರೆ (ಮಗು_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮಗು_pid != 0) {
			ಆದರೆ (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			ಆಗ (c && *c) {
				varbuf = ಖಾಲಿ;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, varbuf, *c);
				ಬಿಡುಗಡೆ(varbuf);
				varbuf = ಖಾಲಿ;
				x++;
				c++;
			}
			snprintf(numbuf, ಗಾತ್ರ(numbuf), "%d", x);
			rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "argc", numbuf);
			rc_service_ಮೌಲ್ಯ_ಗಣ(svcname, "exec", exec);
			supervisor(exec, argv);
		} ಇಲ್ಲದಿದ್ದರೆ
			ಮಗು_process(exec, argv);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stop) {
		pid = get_pid(applet, pidfile);
		ಆದರೆ (pid != -1) {
			i = kill(pid, SIGTERM);
			ಆದರೆ (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			ಇಲ್ಲದಿದ್ದರೆ {
				/* wait for the supervisor to go down */
				ಆಗ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, ಖಾಲಿ);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ಆದರೆ (pidfile)
			unlink(pidfile);
		ಆದರೆ (svcname) {
			rc_service_daemon_ಗಣ(svcname, exec,
			    (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		ಆದರೆ (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = ಲಿಖ(fifo_fd, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));
		ಆದರೆ (x == -1) {
			ಬಿಡುಗಡೆ(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		ಬಿಡುಗಡೆ(tmp);
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	}
}
