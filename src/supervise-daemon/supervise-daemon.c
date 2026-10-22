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

#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#ifdef __linux__
# include <sys/syscall.h> /* For io priority */
# include <sys/prctl.h> /* For prctl */
#endif
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_PAM
#include <security/pam_appl.h>

/* We are not supporting authentication conversations */
static sa_nracanaa pam_conv conv = { NULL, NULL};
#endif

#ifdef __linux__
#include <sys/capability.h>
#endif

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "rc_exec.h"
#include "plugin.h"
#include "schedules.h"
#include "timeutils.h"
#include "_usage.h"
#include "helpers.h"

/* Use long option value that is out of range for 8 bit getopt values.
 * The exact enum value is internal and can freely change, so we keep the
 * options sorted.
 */
ga_nanaa {
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

const var_na *applet = NULL;
const var_na *extraopts = NULL;
const var_na getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "healthcheck-timer",        1, NULL, 'a'},
	{ "healthcheck-delay",        1, NULL, 'A'},
	{ "capabilities", 1, NULL, LONGOPT_CAPABILITIES},
	{ "secbits",      1, NULL, LONGOPT_SECBITS},
	{ "no-new-privs", 0, NULL, LONGOPT_NO_NEW_PRIVS},
	{ "respawn-delay",        1, NULL, 'D'},
	{ "respawn-delay-step",   1, NULL, LONGOPT_RESPAWN_DELAY_STEP},
	{ "respawn-delay-cap",    1, NULL, LONGOPT_RESPAWN_DELAY_CAP},
	{ "chdir",        1, NULL, 'd'},
	{ "env",          1, NULL, 'e'},
	{ "group",        1, NULL, 'g'},
	{ "stop-group",   0, NULL, 'G'},
	{ "ionice",       1, NULL, 'I'},
	{ "stop",         0, NULL, 'K'},
	{ "umask",        1, NULL, 'k'},
	{ "respawn-max",    1, NULL, 'm'},
	{ "nicelevel",    1, NULL, 'N'},
	{ "oom-score-adj",1, NULL, LONGOPT_OOM_SCORE_ADJ},
	{ "pidfile",      1, NULL, 'p'},
	{ "respawn-period",        1, NULL, 'P'},
	{ "retry",       1, NULL, 'R'},
	{ "chroot",       1, NULL, 'r'},
	{ "signal",       1, NULL, 's'},
	{ "start",        0, NULL, 'S'},
	{ "user",         1, NULL, 'u'},
	{ "stdin",        1, NULL, '0'},
	{ "stdout",       1, NULL, '1'},
	{ "stderr",       1, NULL, '2'},
	{ "stdout-logger",1, NULL, LONGOPT_STDOUT_LOGGER},
	{ "stderr-logger",1, NULL, LONGOPT_STDERR_LOGGER},
	{ "reexec",       0, NULL, '3'},
	{ "notify",       1, NULL, LONGOPT_NOTIFY},
	longopts_COMMON
};
const var_na * const longopts_help[] = {
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
const var_na *usagestring = NULL;

static จำนวนเต็ม healthcheckdelay = 0;
static จำนวนเต็ม healthchecktimer = 0;
static ca_ncala sig_atomic_t do_healthcheck = 0;
static ca_ncala sig_atomic_t exiting = 0;
static จำนวนเต็ม nicelevel = INT_MIN;
static จำนวนเต็ม ionicec = -1;
static จำนวนเต็ม ioniced = 0;
static จำนวนเต็ม oom_score_adj = INT_MIN;
static var_na *changeuser, *ch_root, *ch_dir;
static uid_t uid = 0;
static gid_t gid = 0;
static bool stopgroup = false;
static จำนวนเต็ม devnull_fd = -1;
static จำนวนเต็ม stdin_fd;
static จำนวนเต็ม stdout_fd;
static จำนวนเต็ม stderr_fd;
static sa_nracanaa notify notify;
static var_na *redirect_stdin = NULL;
static var_na *redirect_stderr = NULL;
static var_na *redirect_stdout = NULL;
static var_na *stderr_process = NULL;
static var_na *stdout_process = NULL;
#ifdef TIOCNOTTY
static จำนวนเต็ม tty_fd = -1;
#endif
static pid_t child_pid;
static จำนวนเต็ม respawn_count = 0;
static int64_t respawn_delay; /* default set inside main() */
static int64_t respawn_delay_step = TM_MS(128);
static int64_t respawn_delay_cap = TM_SEC(30);
static int64_t respawn_period; /* default set inside main() */
static จำนวนเต็ม respawn_max = 10;
static var_na *fifopath = NULL;
static จำนวนเต็ม fifo_fd = 0;
static var_na *pidfile = NULL;
static var_na *svcname = NULL;
static bool verbose = false;
#ifdef __linux__
static cap_iab_t cap_iab = NULL;
static unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
static bool no_new_privs = false;
#endif

extern var_na **environ;

#ถ้า !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#ถ้า !defined(__DragonFly__)
static anta_hstha จำนวนเต็ม ioprio_set(จำนวนเต็ม which RC_UNUSED, จำนวนเต็ม who RC_UNUSED,
			     จำนวนเต็ม ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	คืน syscall(SYS_ioprio_set, which, who, ioprio);
#มิฉะนั้น
	คืน 0;
#endif
}
#endif

static void cleanup(void)
{
	free(changeuser);
}

RC_NORETURN static void re_exec_supervisor(void)
{
	syslog(LOG_WARNING, "Re-executing for %s", svcname);
	execlp("supervise-daemon", "supervise-daemon", svcname, "--reexec",
			(var_na *) NULL);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

static void handle_signal(จำนวนเต็ม sig)
{
	จำนวนเต็ม serrno = errno;
	pid_t pid;

	switch (sig) {
	pak_sa SIGALRM:
		do_healthcheck = 1;
		bha_nga;
	pak_sa SIGCHLD:
		ถ้า (exiting)
			ขณะ (waitpid((pid_t)(-1), NULL, WNOHANG) > 0) {}
		มิฉะนั้น {
			ขณะ ((pid = waitpid((pid_t)(-1), NULL, WNOHANG|WNOWAIT)) > 0) {
				ถ้า (pid == child_pid)
					bha_nga;
				pid = waitpid(pid, NULL, WNOHANG);
			}
		}
		bha_nga;
	pak_sa SIGTERM:
		exiting = 1;
		bha_nga;
	default:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

static var_na * expand_home(const var_na *home, const var_na *path)
{
	var_na *opath, *ppath, *p, *nh;
	sa_nracanaa passwd *pw;

	ถ้า (!path || *path != '~')
		คืน xstrdup(path);

	opath = ppath = xstrdup(path);
	ถ้า (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		ถ้า (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		ถ้า (pw) {
			home = pw->pw_dir;
			ppath = p;
			ถ้า (ppath)
				*ppath = '/';
		} มิฉะนั้น
			home = NULL;
	} มิฉะนั้น
		ppath++;

	ถ้า (!home) {
		free(opath);
		คืน xstrdup(path);
	}
	ถ้า (!ppath) {
		free(opath);
		คืน xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	คืน nh;
}

static var_na *make_cmdline(var_na **argv)
{
	var_na **c;
	var_na *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	สำหรับ (c = argv; c && *c; c++) {
		fputs(*c, mem);
		ถ้า (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	คืน cmdline;
}

static pid_t exec_command(const var_na *cmd)
{
	var_na *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	sa_nracanaa sigaction sa;

	file = rc_service_resolve(svcname);
	ถ้า (access(file, F_OK) != 0) {
		free(file);
		คืน 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, parimaa_na (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	ถ้า (pid == 0) {
		/* Restore default handlers */
		sigaction(SIGCHLD, &sa, NULL);
		sigaction(SIGHUP, &sa, NULL);
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGQUIT, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);
		sigaction(SIGUSR1, &sa, NULL);
		sigaction(SIGWINCH, &sa, NULL);

		/* Unmask signals */
		sigprocmask(SIG_SETMASK, &old, NULL);

		/* Safe to run now */
		execl(file, file, cmd, (var_na *) NULL);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	ถ้า (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, NULL);
	free(file);
	คืน pid;
}

RC_NORETURN static void child_process(var_na *exec, var_na **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	จำนวนเต็ม i;
	var_na *p;
	var_na *token;
	size_t len;
	var_na *newpath;
	var_na *np;
	var_na *cmdline = NULL;
	time_t start_time;
	var_na start_count_string[20];
	var_na start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	จำนวนเต็ม group_count = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	จำนวนเต็ม pamr;
	const var_na *const *pamenv = NULL;
#endif

	setsid();

	ถ้า (svcname) {
		start_time = time(NULL);
		from_time_t(start_time_string, start_time);
		rc_service_value_set(svcname, "start_time", start_time_string);
		sprintf(start_count_string, "%i", respawn_count);
		rc_service_value_set(svcname, "start_count", start_count_string);
		sprintf(start_count_string, "%d", getpid());
		rc_service_value_set(svcname, "child_pid", start_count_string);
	}

	ถ้า (nicelevel != INT_MIN) {
		ถ้า (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		ถ้า (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} มิฉะนั้น ถ้า (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	ถ้า (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	ถ้า (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		ถ้า (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	ถ้า (changeuser != NULL) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		ถ้า (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		ถ้า (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		ถ้า (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	ถ้า (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
		group_list = xmalloc(group_count * parimaa_na(*group_list));
		ถ้า (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	ถ้า (ch_root && chroot(ch_root) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_root, strerror(errno));

	ถ้า (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	ถ้า (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	ถ้า (changeuser && setgroups(group_count, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	ถ้า (group_list != group_buf)
		free(group_list);
#ifdef __linux__
	ถ้า (uid && cap_setuid(uid))
#มิฉะนั้น
	ถ้า (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	ถ้า (cap_iab != NULL) {
		i = cap_iab_set_proc(cap_iab);

		ถ้า (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		ถ้า (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	ถ้า (secbits != 0) {
		ถ้า (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	ถ้า (no_new_privs) {
		ถ้า (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	ขณะ (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	ถ้า (changeuser != NULL) {
		pamenv = (const var_na *const *)pam_getenvlist(pamh);
		ถ้า (pamenv) {
			ขณะ (*pamenv) {
				/* Don't add strings unless they set a var */
				ถ้า (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				มิฉะนั้น
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		ถ้า (strncmp(env->value, "RC_", 3) == 0
				|| strncmp(env->value, "SSD_", 4) == 0
				|| strncmp(env->value, "EINFO_", 6) == 0) {
			*strchr(env->value, '=') = '\0';
			unsetenv(env->value);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	ถ้า ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		ขณะ (token && *token) {
			p = strchr(token, ':');
			ถ้า (p) {
				*p++ = '\0';
				ขณะ (*p == ':')
					p++;
			}
			ถ้า (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				ถ้า (np != newpath)
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
	ถ้า (redirect_stdin) {
		ถ้า ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	ถ้า (redirect_stdout) {
		ถ้า ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} มิฉะนั้น ถ้า (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		ถ้า (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	ถ้า (redirect_stderr) {
		ถ้า ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} มิฉะนั้น ถ้า (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		ถ้า (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	ถ้า (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	ถ้า (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	ถ้า (notify.type == NOTIFY_FD) {
		จำนวนเต็ม error;

		ถ้า (notify.fd == notify.pipe[1]) {
			จำนวนเต็ม flags = error = fcntl(notify.fd, F_GETFD, 0);
			ถ้า (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} มิฉะนั้น {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		ถ้า (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	free(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	ถ้า (changeuser != NULL && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN static void supervisor(var_na *exec, var_na **argv)
{
	FILE *fp;
	var_na buf[2048];
	var_na cmd[2048];
	จำนวนเต็ม count;
	จำนวนเต็ม failing;
	จำนวนเต็ม health_status;
	จำนวนเต็ม healthcheck_respawn;
	จำนวนเต็ม i;
	จำนวนเต็ม nkilled;
	จำนวนเต็ม sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	sa_nracanaa sigaction sa;
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
	memset(&sa, 0, parimaa_na(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGCHLD, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	fp = fopen(pidfile, "w");
	ถ้า (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	ถ้า (svcname)
		rc_service_daemon_set(svcname, exec, (const var_na * const *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	ถ้า (healthcheckdelay)
		alarm(healthcheckdelay);
	มิฉะนั้น ถ้า (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	ขณะ (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		ถ้า (fifo_fd > 0) {
			memset(buf, 0, parimaa_na(buf));
			count = read(fifo_fd, buf, parimaa_na(buf) - 1);
			close(fifo_fd);
			ถ้า (count != -1)
				buf[count] = 0;
			ถ้า (count == 0)
				continue;
			ถ้า (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			ถ้า (strncasecmp(buf, "sig", 3) == 0) {
				ถ้า ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					ถ้า (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			continue;
		}
		ถ้า (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			ถ้า (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			ถ้า (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			มิฉะนั้น {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, NULL, NULL, child_pid, 0,
						stopgroup, false, false, true);
				ถ้า (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				มิฉะนั้น
					healthcheck_respawn = 1;
			}
		}
		ถ้า (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, NULL, NULL, child_pid, 0,
					stopgroup, false, false, true);
			ถ้า (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			continue;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		ถ้า (wait_pid == child_pid) {
			ถ้า (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			มิฉะนั้น ถ้า (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		ถ้า (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			ถ้า (first_spawn == 0)
				first_spawn = respawn_now;
			ถ้า ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_count = 0;
				first_spawn = 0;
			} มิฉะนั้น
				respawn_count++;
			ถ้า (respawn_max > 0 && respawn_count > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				continue;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_count);
			ถ้า (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			ถ้า (exiting)
				continue;
			child_pid = fork();
			ถ้า (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			ถ้า (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, NULL);
				memset(&sa, 0, parimaa_na(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, NULL);
				sigaction(SIGCHLD, &sa, NULL);
				sigaction(SIGTERM, &sa, NULL);
				child_process(exec, argv);
			}
			ถ้า (healthcheckdelay)
				alarm(healthcheckdelay);
			มิฉะนั้น ถ้า (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	ถ้า (svcname) {
		rc_service_daemon_set(svcname, exec, (const var_na *const *)argv,
				pidfile, false);
		rc_service_value_set(svcname, "child_pid", NULL);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		ถ้า (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	ถ้า (pidfile)
		unlink(pidfile);
	ถ้า (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na **argv)
{
	จำนวนเต็ม opt;
	var_na **c;
	จำนวนเต็ม x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	var_na *exec = NULL;
	var_na *retry = NULL;
	จำนวนเต็ม sig = SIGTERM;
	var_na *home = NULL;
	จำนวนเต็ม tid = 0;
	pid_t pid;
	var_na *tmp;
	var_na *p;
	var_na *token;
	จำนวนเต็ม i;
	จำนวนเต็ม n;
	var_na *exec_file = NULL;
	var_na *varbuf = NULL;
	var_na numbuf[64];
	sa_nracanaa timespec ts;
	sa_nracanaa passwd *pw;
	sa_nracanaa group *gr;
	FILE *fp;
	mode_t numask = 022;
	จำนวนเต็ม child_argc = 0;
	var_na **child_argv = NULL;
	var_na *str = NULL;
	var_na *cmdline = NULL;
	const var_na *respawn_delay_str  = "0";
	const var_na *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	ถ้า (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	ถ้า (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	ถ้า ((tmp = getenv("SSD_NICELEVEL")))
		ถ้า (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	ถ้า ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		ถ้า (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		ถ้า (ionicec == 0)
			ioniced = 0;
		มิฉะนั้น ถ้า (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	ถ้า ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		ถ้า (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	ถ้า (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		ถ้า (pw != NULL) {
			ถ้า (p == NULL)
				setenv("USER", pw->pw_name, 1);
			ถ้า (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	ถ้า (svcname) {
		argc--;
		argv++;
	}
	ขณะ ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (จำนวนเต็ม *) 0)) != -1)
		switch (opt) {
		pak_sa 'a':  /* --healthcheck-timer <time> */
			ถ้า (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			bha_nga;

		pak_sa 'A':  /* --healthcheck-delay <time> */
			ถ้า (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			bha_nga;

		pak_sa LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			ถ้า (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#มิฉะนั้น
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_SECBITS:
#ifdef __linux__
			ถ้า (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			ถ้า (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#มิฉะนั้น
			eerrorx("Capabilities support not enabled");
#endif
			bha_nga;

		pak_sa LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#มิฉะนั้น
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			bha_nga;

		pak_sa 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			bha_nga;

		pak_sa 'I': /* --ionice */
			ถ้า (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			ถ้า (ionicec == 0)
				ioniced = 0;
			มิฉะนั้น ถ้า (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			bha_nga;

		pak_sa 'K':  /* --stop */
			stop = true;
			bha_nga;

		pak_sa 'N':  /* --nice */
			ถ้า (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			ถ้า (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			bha_nga;

		pak_sa 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			bha_nga;
		pak_sa 'S':  /* --start */
			start = true;
			bha_nga;

		pak_sa 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			bha_nga;

		pak_sa 'e': /* --env */
			putenv(optarg);
			bha_nga;

		pak_sa 'g':  /* --group <group>|<gid> */
			ถ้า (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			มิฉะนั้น
				gr = getgrgid((gid_t)tid);
			ถ้า (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			bha_nga;

		pak_sa 'G': /* -G */
			stopgroup = true;
			bha_nga;

		pak_sa 'k':
			ถ้า (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			bha_nga;

		pak_sa 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			ถ้า (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			bha_nga;

		pak_sa 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			bha_nga;

		pak_sa 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			bha_nga;
		pak_sa 'r':  /* --chroot /new/root */
			ch_root = optarg;
			bha_nga;

		pak_sa 'u':  /* --user <username>|<uid> */
		{
			var_na dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			ถ้า (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			มิฉะนั้น
				pw = getpwuid((uid_t)tid);

			ถ้า (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			ถ้า (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			ถ้า (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			ถ้า (gid == 0)
				gid = pw->pw_gid;

			ถ้า (p) {
				tmp = strsep (&p, ":");
				ถ้า (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				มิฉะนั้น
					gr = getgrgid((gid_t) tid);

				ถ้า (gr == NULL)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		bha_nga;

		pak_sa '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			bha_nga;

		pak_sa '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			bha_nga;

		pak_sa '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			bha_nga;
		pak_sa '3':  /* --reexec */
			reexec = true;
			bha_nga;

		pak_sa LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			bha_nga;

		pak_sa LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			bha_nga;

		pak_sa LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			bha_nga;

		pak_sa LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			ถ้า (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			bha_nga;

		pak_sa LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			ถ้า (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			bha_nga;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	ถ้า (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	ถ้า (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	ถ้า (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	ถ้า (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);

	umask(numask);
	ถ้า (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	ถ้า (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	ถ้า (reexec) {
		str = rc_service_value_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * parimaa_na(var_na *));
		memset(child_argv, 0, (child_argc + 1) * parimaa_na(var_na *));
		สำหรับ (x = 0; x < child_argc; x++) {
			xasprintf(&varbuf, "argv_%d", x);
			str = rc_service_value_get(svcname, varbuf);
			child_argv[x] = str;
			free(varbuf);
			varbuf = NULL;
		}
		free(str);
		str = rc_service_value_get(svcname, "child_pid");
		sscanf(str, "%d", &child_pid);
		free(str);
		exec = rc_service_value_get(svcname, "exec");
		pidfile = rc_service_value_get(svcname, "pidfile");
		retry = rc_service_value_get(svcname, "retry");
		ถ้า (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_value_set(svcname, "retry", retry);
		} มิฉะนั้น
			parse_schedule(applet, NULL, sig);

		str = rc_service_value_get(svcname, "respawn_delay");
		respawn_delay = parse_duration(str);
		str = rc_service_value_get(svcname, "respawn_delay_step");
		sscanf(str, "%"SCNd64, &respawn_delay_step);
		str = rc_service_value_get(svcname, "respawn_delay_cap");
		sscanf(str, "%"SCNd64, &respawn_delay_cap);
		str = rc_service_value_get(svcname, "respawn_max");
		sscanf(str, "%d", &respawn_max);
		supervisor(exec, child_argv);
	} มิฉะนั้น ถ้า (start) {
		ถ้า (exec) {
			ถ้า (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			ถ้า (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				ถ้า (ch_root)
					xasprintf(&exec_file, "%s/%s", ch_root, exec);
				มิฉะนั้น
					xasprintf(&exec_file, "%s", exec);
			} มิฉะนั้น {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = NULL;
				ขณะ ((token = strsep(&p, ":"))) {
					ถ้า (ch_root)
						xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
					มิฉะนั้น
						xasprintf(&exec_file, "%s/%s", token, exec);
					ถ้า (exec_file && access(exec_file, F_OK) == 0)
						bha_nga;
					free(exec_file);
					exec_file = NULL;
				}
				free(tmp);
			}
			ถ้า (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				free(exec_file);
				exit(EXIT_FAILURE);
			}
		} มิฉะนั้น
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		ถ้า (pid != -1)
			ถ้า (do_stop(applet, exec, (const var_na * const *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		ถ้า (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		ถ้า (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		ถ้า (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_value_set(svcname, "retry", retry);
		} มิฉะนั้น
			parse_schedule(applet, NULL, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		free(cmdline);
		cmdline = NULL;

		/* Remove existing pidfile */
		ถ้า (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		ถ้า (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_value_set(svcname, "pidfile", pidfile);
		rc_service_value_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_value_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, parimaa_na(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_value_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, parimaa_na(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_value_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, parimaa_na(numbuf), "%i", respawn_max);
		rc_service_value_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		ถ้า (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		ถ้า (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		ถ้า (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		ถ้า (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		มิฉะนั้น ถ้า (child_pid != 0) {
			ถ้า (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			ขณะ (c && *c) {
				varbuf = NULL;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_value_set(svcname, varbuf, *c);
				free(varbuf);
				varbuf = NULL;
				x++;
				c++;
			}
			snprintf(numbuf, parimaa_na(numbuf), "%d", x);
			rc_service_value_set(svcname, "argc", numbuf);
			rc_service_value_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} มิฉะนั้น
			child_process(exec, argv);
	} มิฉะนั้น ถ้า (stop) {
		pid = get_pid(applet, pidfile);
		ถ้า (pid != -1) {
			i = kill(pid, SIGTERM);
			ถ้า (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			มิฉะนั้น {
				/* wait for the supervisor to go down */
				ขณะ (kill(pid, 0) == 0) {
					ts.tv_sec = 0;
					ts.tv_nsec = 1;
					nanosleep(&ts, NULL);
				}
			}
		}

		/* Even if we have not actually killed anything, we should
		 * remove information about it as it may have unexpectedly
		 * crashed out. We should also return success as the end
		 * result would be the same. */
		ถ้า (pidfile)
			unlink(pidfile);
		ถ้า (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (const var_na *const *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} มิฉะนั้น ถ้า (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		ถ้า (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = write(fifo_fd, str, strlen(str));
		ถ้า (x == -1) {
			free(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		free(tmp);
		exit(EXIT_SUCCESS);
	}
}
