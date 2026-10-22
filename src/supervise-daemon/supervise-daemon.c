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
static struct pam_conv conv = { NULL, NULL};
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
enum {
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

const char *applet = NULL;
const char *extraopts = NULL;
const char getoptstring[] = "A:a:D:d:e:Gg:I:Kk:m:N:p:R:r:s:Su:0:1:2:3" \
	getoptstring_COMMON;
const struct option longopts[] = {
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
const char * const longopts_help[] = {
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
const char *usagestring = NULL;

static số_nguyên healthcheckdelay = 0;
static số_nguyên healthchecktimer = 0;
static volatile sig_atomic_t do_healthcheck = 0;
static volatile sig_atomic_t exiting = 0;
static số_nguyên nicelevel = INT_MIN;
static số_nguyên ionicec = -1;
static số_nguyên ioniced = 0;
static số_nguyên oom_score_adj = INT_MIN;
static char *changeuser, *ch_root, *ch_dir;
static uid_t uid = 0;
static gid_t gid = 0;
static bool stopgroup = false;
static số_nguyên devnull_fd = -1;
static số_nguyên stdin_fd;
static số_nguyên stdout_fd;
static số_nguyên stderr_fd;
static struct notify notify;
static char *redirect_stdin = NULL;
static char *redirect_stderr = NULL;
static char *redirect_stdout = NULL;
static char *stderr_process = NULL;
static char *stdout_process = NULL;
#ifdef TIOCNOTTY
static số_nguyên tty_fd = -1;
#endif
static pid_t child_pid;
static số_nguyên respawn_count = 0;
static int64_t respawn_delay; /* default set inside main() */
static int64_t respawn_delay_step = TM_MS(128);
static int64_t respawn_delay_cap = TM_SEC(30);
static int64_t respawn_period; /* default set inside main() */
static số_nguyên respawn_max = 10;
static char *fifopath = NULL;
static số_nguyên fifo_fd = 0;
static char *pidfile = NULL;
static char *svcname = NULL;
static bool verbose = false;
#ifdef __linux__
static cap_iab_t cap_iab = NULL;
static unsigned secbits = 0;
#endif
#ifdef PR_SET_NO_NEW_PRIVS
static bool no_new_privs = false;
#endif

extern char **environ;

#nếu !defined(SYS_ioprio_set) && defined(__NR_ioprio_set)
# define SYS_ioprio_set __NR_ioprio_set
#endif
#nếu !defined(__DragonFly__)
static inline số_nguyên ioprio_set(số_nguyên which RC_UNUSED, số_nguyên who RC_UNUSED,
			     số_nguyên ioprio RC_UNUSED)
{
#ifdef SYS_ioprio_set
	trả_về syscall(SYS_ioprio_set, which, who, ioprio);
#không_thì
	trả_về 0;
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
			(char *) NULL);
	syslog(LOG_ERR, "Unable to execute supervise-daemon: %s",
			strerror(errno));
	exit(EXIT_FAILURE);
}

static void handle_signal(số_nguyên sig)
{
	số_nguyên serrno = errno;
	pid_t pid;

	switch (sig) {
	case SIGALRM:
		do_healthcheck = 1;
		break;
	case SIGCHLD:
		nếu (exiting)
			trong_khi (waitpid((pid_t)(-1), NULL, WNOHANG) > 0) {}
		không_thì {
			trong_khi ((pid = waitpid((pid_t)(-1), NULL, WNOHANG|WNOWAIT)) > 0) {
				nếu (pid == child_pid)
					break;
				pid = waitpid(pid, NULL, WNOHANG);
			}
		}
		break;
	case SIGTERM:
		exiting = 1;
		break;
	default:
		syslog(LOG_WARNING, "caught signal %d", sig);
		re_exec_supervisor();
	}
	/* Restore errno */
	errno = serrno;
}

static char * expand_home(const char *home, const char *path)
{
	char *opath, *ppath, *p, *nh;
	struct passwd *pw;

	nếu (!path || *path != '~')
		trả_về xstrdup(path);

	opath = ppath = xstrdup(path);
	nếu (ppath[1] != '/' && ppath[1] != '\0') {
		p = strchr(ppath + 1, '/');
		nếu (p)
			*p = '\0';
		pw = getpwnam(ppath + 1);
		nếu (pw) {
			home = pw->pw_dir;
			ppath = p;
			nếu (ppath)
				*ppath = '/';
		} không_thì
			home = NULL;
	} không_thì
		ppath++;

	nếu (!home) {
		free(opath);
		trả_về xstrdup(path);
	}
	nếu (!ppath) {
		free(opath);
		trả_về xstrdup(home);
	}

	xasprintf(&nh, "%s%s", home, ppath);
	free(opath);
	trả_về nh;
}

static char *make_cmdline(char **argv)
{
	char **c;
	char *cmdline;
	size_t len;
	FILE *mem = xopen_memstream(&cmdline, &len);

	cho (c = argv; c && *c; c++) {
		fputs(*c, mem);
		nếu (c[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);
	trả_về cmdline;
}

static pid_t exec_command(const char *cmd)
{
	char *file;
	pid_t pid = -1;
	sigset_t full;
	sigset_t old;
	struct sigaction sa;

	file = rc_service_resolve(svcname);
	nếu (access(file, F_OK) != 0) {
		free(file);
		trả_về 0;
	}

	/* We need to block signals until we have forked */
	memset(&sa, 0, sizeof (sa));
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);

	pid = fork();
	nếu (pid == 0) {
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
		execl(file, file, cmd, (char *) NULL);
		syslog(LOG_ERR, "unable to exec `%s': %s\n",
		    file, strerror(errno));
		_exit(EXIT_FAILURE);
	}

	nếu (pid == -1)
		syslog(LOG_ERR, "fork: %s\n",strerror (errno));

	sigprocmask(SIG_SETMASK, &old, NULL);
	free(file);
	trả_về pid;
}

RC_NORETURN static void child_process(char *exec, char **argv)
{
	RC_STRINGLIST *env_list;
	RC_STRING *env;
	số_nguyên i;
	char *p;
	char *token;
	size_t len;
	char *newpath;
	char *np;
	char *cmdline = NULL;
	time_t start_time;
	char start_count_string[20];
	char start_time_string[20];
	FILE *fp;
	gid_t group_buf[32], *group_list = group_buf;
	số_nguyên group_count = ARRAY_SIZE(group_buf);

#ifdef HAVE_PAM
	pam_handle_t *pamh = NULL;
	số_nguyên pamr;
	const char *const *pamenv = NULL;
#endif

	setsid();

	nếu (svcname) {
		start_time = time(NULL);
		from_time_t(start_time_string, start_time);
		rc_service_value_set(svcname, "start_time", start_time_string);
		sprintf(start_count_string, "%i", respawn_count);
		rc_service_value_set(svcname, "start_count", start_count_string);
		sprintf(start_count_string, "%d", getpid());
		rc_service_value_set(svcname, "child_pid", start_count_string);
	}

	nếu (nicelevel != INT_MIN) {
		nếu (setpriority(PRIO_PROCESS, getpid(), nicelevel) == -1)
			eerrorx("%s: setpriority %d: %s", applet, nicelevel,
					strerror(errno));
		/* Open in "r+" mode to avoid creating if non-existent. */
		fp = fopen("/proc/self/autogroup", "r+");
		nếu (fp) {
			fprintf(fp, "%d\n", nicelevel);
			fclose(fp);
		} không_thì nếu (errno != ENOENT)
			eerrorx("%s: autogroup nice %d: %s", applet,
			    nicelevel, strerror(errno));
	}

	nếu (ionicec != -1 && ioprio_set(1, getpid(), ionicec | ioniced) == -1)
		eerrorx("%s: ioprio_set %d %d: %s", applet, ionicec, ioniced,
				strerror(errno));

	nếu (oom_score_adj != INT_MIN) {
		fp = fopen("/proc/self/oom_score_adj", "w");
		nếu (!fp)
			eerrorx("%s: oom_score_adj %d: %s", applet,
			    oom_score_adj, strerror(errno));
		fprintf(fp, "%d\n", oom_score_adj);
		fclose(fp);
	}

#ifdef HAVE_PAM
	nếu (changeuser != NULL) {
		pamr = pam_start("supervise-daemon",
		    changeuser, &conv, &pamh);

		nếu (pamr == PAM_SUCCESS)
			pamr = pam_acct_mgmt(pamh, PAM_SILENT);
		nếu (pamr == PAM_SUCCESS)
			pamr = pam_open_session(pamh, PAM_SILENT);
		nếu (pamr != PAM_SUCCESS)
			eerrorx("%s: pam error: %s", applet, pam_strerror(pamh, pamr));
	}
#endif

	nếu (changeuser && getgrouplist(changeuser, gid, group_list, &group_count) < 0) {
		group_list = xmalloc(group_count * sizeof(*group_list));
		nếu (getgrouplist(changeuser, gid, group_list, &group_count) < 0)
			eerrorx("%s: getgrouplist(%s, %"PRIuMAX")", applet, changeuser, (uintmax_t)gid);
	}

	/* Close any fd's to the passwd database */
	endpwent();

	nếu (ch_root && chroot(ch_root) < 0)
		eerrorx("%s: chroot `%s': %s", applet, ch_root, strerror(errno));

	nếu (ch_dir && chdir(ch_dir) < 0)
		eerrorx("%s: chdir `%s': %s", applet, ch_dir, strerror(errno));

	nếu (gid && setgid(gid))
		eerrorx("%s: unable to set groupid to %"PRIuMAX, applet, (uintmax_t)gid);
	nếu (changeuser && setgroups(group_count, group_list) < 0)
		eerrorx("%s: setgroups() failed", applet);
	nếu (group_list != group_buf)
		free(group_list);
#ifdef __linux__
	nếu (uid && cap_setuid(uid))
#không_thì
	nếu (uid && setuid(uid))
#endif
		eerrorx ("%s: unable to set userid to %"PRIuMAX, applet, (uintmax_t)uid);

#ifdef __linux__
	nếu (cap_iab != NULL) {
		i = cap_iab_set_proc(cap_iab);

		nếu (cap_free(cap_iab) != 0)
			eerrorx("Could not releasable memory: %s", strerror(errno));

		nếu (i != 0)
			eerrorx("Could not set iab: %s", strerror(errno));
	}

	nếu (secbits != 0) {
		nếu (cap_set_secbits(secbits) < 0)
			eerrorx("Could not set securebits to 0x%x: %s", secbits, strerror(errno));
	}
#endif

#ifdef PR_SET_NO_NEW_PRIVS
	nếu (no_new_privs) {
		nếu (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1)
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
	trong_khi (environ[i])
		rc_stringlist_add(env_list, environ[i++]);

#ifdef HAVE_PAM
	nếu (changeuser != NULL) {
		pamenv = (const char *const *)pam_getenvlist(pamh);
		nếu (pamenv) {
			trong_khi (*pamenv) {
				/* Don't add strings unless they set a var */
				nếu (strchr(*pamenv, '='))
					putenv(xstrdup(*pamenv));
				không_thì
					unsetenv(*pamenv);
				pamenv++;
			}
		}
	}
#endif

	unsetenv("SVCNAME");
	TAILQ_FOREACH(env, env_list, entries) {
		nếu (strncmp(env->value, "RC_", 3) == 0
				|| strncmp(env->value, "SSD_", 4) == 0
				|| strncmp(env->value, "EINFO_", 6) == 0) {
			*strchr(env->value, '=') = '\0';
			unsetenv(env->value);
		}
	}
	rc_stringlist_free(env_list);

	/* For the path, remove the rcscript bin dir from it */
	nếu ((token = getenv("PATH"))) {
		len = strlen(token);
		newpath = np = xmalloc(len + 1);
		trong_khi (token && *token) {
			p = strchr(token, ':');
			nếu (p) {
				*p++ = '\0';
				trong_khi (*p == ':')
					p++;
			}
			nếu (strcmp(token, RC_LIBEXECDIR "/bin") != 0 &&
			    strcmp(token, RC_LIBEXECDIR "/sbin") != 0)
			{
				len = strlen(token);
				nếu (np != newpath)
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
	nếu (redirect_stdin) {
		nếu ((stdin_fd = xopen_nonblock(redirect_stdin, O_RDONLY, 0)) == -1)
			eerrorx("%s: unable to redirect the input file"
			    " for stdin `%s': %s",
			    applet, redirect_stdin, strerror(errno));
	}
	nếu (redirect_stdout) {
		nếu ((stdout_fd = xopen_nonblock(redirect_stdout,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
				    " for stdout `%s': %s",
				    applet, redirect_stdout, strerror(errno));
	} không_thì nếu (stdout_process) {
		stdout_fd = rc_pipe_command(stdout_process, devnull_fd);
		nếu (stdout_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stdout `%s': %s",
			    applet, stdout_process, strerror(errno));
	}
	nếu (redirect_stderr) {
		nếu ((stderr_fd = xopen_nonblock(redirect_stderr,
			    O_WRONLY | O_CREAT | O_APPEND,
			    S_IRUSR | S_IWUSR)) == -1)
			eerrorx("%s: unable to redirect the logfile"
			    " for stderr `%s': %s",
			    applet, redirect_stderr, strerror(errno));
	} không_thì nếu (stderr_process) {
		stderr_fd = rc_pipe_command(stderr_process, devnull_fd);
		nếu (stderr_fd == -1)
			eerrorx("%s: unable to open the logging process"
			    " for stderr `%s': %s",
			    applet, stderr_process, strerror(errno));
	}

	dup2(stdin_fd, STDIN_FILENO);
	nếu (redirect_stdout || stdout_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stdout_fd, STDOUT_FILENO);
	nếu (redirect_stderr || stderr_process || rc_yesno(getenv("EINFO_QUIET")))
		dup2(stderr_fd, STDERR_FILENO);

	cloexec_fds_from(3);

	nếu (notify.type == NOTIFY_FD) {
		số_nguyên error;

		nếu (notify.fd == notify.pipe[1]) {
			số_nguyên flags = error = fcntl(notify.fd, F_GETFD, 0);
			nếu (flags != -1)
				error = fcntl(notify.fd, F_SETFD, flags & ~FD_CLOEXEC);
		} không_thì {
			error = dup2(notify.pipe[1], notify.fd) == -1;
		}

		nếu (error)
			eerrorx("%s: failed to dup ready fd: %s", applet, strerror(errno));
	}

	cmdline = make_cmdline(argv);
	syslog(LOG_INFO, "Child command line: %s", cmdline);
	free(cmdline);
	execvp(exec, argv);

#ifdef HAVE_PAM
	nếu (changeuser != NULL && pamr == PAM_SUCCESS)
		pam_close_session(pamh, PAM_SILENT);
#endif
	eerrorx("%s: failed to exec `%s': %s", applet, exec,strerror(errno));
}

RC_NORETURN static void supervisor(char *exec, char **argv)
{
	FILE *fp;
	char buf[2048];
	char cmd[2048];
	số_nguyên count;
	số_nguyên failing;
	số_nguyên health_status;
	số_nguyên healthcheck_respawn;
	số_nguyên i;
	số_nguyên nkilled;
	số_nguyên sig_send;
	pid_t health_pid;
	pid_t wait_pid;
	sigset_t old_signals;
	sigset_t signals;
	struct sigaction sa;
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
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGALRM, &sa, NULL);
	sigaction(SIGCHLD, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	fp = fopen(pidfile, "w");
	nếu (!fp)
		eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	nếu (svcname)
		rc_service_daemon_set(svcname, exec, (const char * const *) argv,
				pidfile, true);

	/* remove the controlling tty */
#ifdef TIOCNOTTY
	ioctl(tty_fd, TIOCNOTTY, 0);
	close(tty_fd);
#endif

	/*
	 * Supervisor main loop
	 */
	nếu (healthcheckdelay)
		alarm(healthcheckdelay);
	không_thì nếu (healthchecktimer)
		alarm(healthchecktimer);
	failing = 0;
	trong_khi (!exiting) {
		healthcheck_respawn = 0;
		fifo_fd = open(fifopath, O_RDONLY);
		nếu (fifo_fd > 0) {
			memset(buf, 0, sizeof(buf));
			count = read(fifo_fd, buf, sizeof(buf) - 1);
			close(fifo_fd);
			nếu (count != -1)
				buf[count] = 0;
			nếu (count == 0)
				continue;
			nếu (verbose)
				syslog(LOG_DEBUG, "Received %s from fifo", buf);
			nếu (strncasecmp(buf, "sig", 3) == 0) {
				nếu ((sscanf(buf, "%s %d", cmd, &sig_send) == 2)
						&& (sig_send >= 0 && sig_send < NSIG)) {
					syslog(LOG_INFO, "Sending signal %d to %d", sig_send,
							child_pid);
					nếu (kill(child_pid, sig_send) == -1)
						syslog(LOG_ERR, "Unable to send signal %d to %d",
								sig_send, child_pid);
				}
			}
			continue;
		}
		nếu (do_healthcheck) {
			do_healthcheck = 0;
			alarm(0);
			nếu (verbose)
				syslog(LOG_DEBUG, "running health check for %s", svcname);
			health_pid = exec_command("healthcheck");
			health_status = rc_waitpid(health_pid);
			nếu (WIFEXITED(health_status) && WEXITSTATUS(health_status) == 0)
				alarm(healthchecktimer);
			không_thì {
				syslog(LOG_WARNING, "health check for %s failed", svcname);
				health_pid = exec_command("unhealthy");
				rc_waitpid(health_pid);
				syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
				nkilled = run_stop_schedule(applet, NULL, NULL, child_pid, 0,
						stopgroup, false, false, true);
				nếu (nkilled < 0)
					syslog(LOG_INFO, "Unable to kill %d: %s",
							child_pid, strerror(errno));
				không_thì
					healthcheck_respawn = 1;
			}
		}
		nếu (exiting) {
			alarm(0);
			syslog(LOG_INFO, "stopping %s, pid %d", exec, child_pid);
			nkilled = run_stop_schedule(applet, NULL, NULL, child_pid, 0,
					stopgroup, false, false, true);
			nếu (nkilled > 0)
				syslog(LOG_INFO, "killed %d processes", nkilled);
			continue;
		}
		wait_pid = waitpid(child_pid, &i, WNOHANG);
		nếu (wait_pid == child_pid) {
			nếu (WIFEXITED(i))
				syslog(LOG_WARNING, "%s, pid %d, exited with return code %d",
						exec, child_pid, WEXITSTATUS(i));
			không_thì nếu (WIFSIGNALED(i))
				syslog(LOG_WARNING, "%s, pid %d, terminated by signal %d",
						exec, child_pid, WTERMSIG(i));
		}
		nếu (wait_pid == child_pid || healthcheck_respawn) {
			do_healthcheck = 0;
			healthcheck_respawn = 0;
			alarm(0);
			respawn_now = tm_now();
			nếu (first_spawn == 0)
				first_spawn = respawn_now;
			nếu ((respawn_period > 0)
					&& (respawn_now - first_spawn > respawn_period)) {
				respawn_count = 0;
				first_spawn = 0;
			} không_thì
				respawn_count++;
			nếu (respawn_max > 0 && respawn_count > respawn_max) {
				syslog(LOG_WARNING, "respawned \"%s\" too many times, exiting",
						exec);
				exiting = 1;
				failing = 1;
				continue;
			}
			sleep_for = respawn_delay + (respawn_delay_step * respawn_count);
			nếu (respawn_delay_step > 0 && sleep_for > respawn_delay_cap)
				sleep_for = respawn_delay_cap;
			tm_sleep(sleep_for, TM_NO_EINTR);
			nếu (exiting)
				continue;
			child_pid = fork();
			nếu (child_pid == -1) {
				syslog(LOG_ERR, "%s: fork: %s", applet, strerror(errno));
				exit(EXIT_FAILURE);
			}
			nếu (child_pid == 0) {
				sigprocmask(SIG_SETMASK, &old_signals, NULL);
				memset(&sa, 0, sizeof(sa));
				sa.sa_handler = SIG_DFL;
				sigaction(SIGALRM, &sa, NULL);
				sigaction(SIGCHLD, &sa, NULL);
				sigaction(SIGTERM, &sa, NULL);
				child_process(exec, argv);
			}
			nếu (healthcheckdelay)
				alarm(healthcheckdelay);
			không_thì nếu (healthchecktimer)
				alarm(healthchecktimer);
		}
	}

	nếu (svcname) {
		rc_service_daemon_set(svcname, exec, (const char *const *)argv,
				pidfile, false);
		rc_service_value_set(svcname, "child_pid", NULL);
		rc_service_mark(svcname, RC_SERVICE_STOPPED);
		nếu (failing)
			rc_service_mark(svcname, RC_SERVICE_FAILED);
	}
	nếu (pidfile)
		unlink(pidfile);
	nếu (fifopath)
		unlink(fifopath);
	exit(EXIT_SUCCESS);
}

số_nguyên main(số_nguyên argc, char **argv)
{
	số_nguyên opt;
	char **c;
	số_nguyên x;
	bool start = false;
	bool stop = false;
	bool reexec = false;
	bool sendsig = false;
	char *exec = NULL;
	char *retry = NULL;
	số_nguyên sig = SIGTERM;
	char *home = NULL;
	số_nguyên tid = 0;
	pid_t pid;
	char *tmp;
	char *p;
	char *token;
	số_nguyên i;
	số_nguyên n;
	char *exec_file = NULL;
	char *varbuf = NULL;
	char numbuf[64];
	struct timespec ts;
	struct passwd *pw;
	struct group *gr;
	FILE *fp;
	mode_t numask = 022;
	số_nguyên child_argc = 0;
	char **child_argv = NULL;
	char *str = NULL;
	char *cmdline = NULL;
	const char *respawn_delay_str  = "0";
	const char *respawn_period_str = "12sec";

	applet = basename_c(argv[0]);
	atexit(cleanup);
	svcname = getenv("RC_SVCNAME");
	nếu (!svcname)
		eerrorx("%s: The RC_SVCNAME environment variable is not set", applet);
	openlog(applet, LOG_PID, LOG_DAEMON);

	nếu (argc <= 1 || strcmp(argv[1], svcname))
		eerrorx("%s: the first argument is %s and must be %s",
				applet, argv[1], svcname);

	nếu ((tmp = getenv("SSD_NICELEVEL")))
		nếu (sscanf(tmp, "%d", &nicelevel) != 1)
			eerror("%s: invalid nice level `%s' (SSD_NICELEVEL)",
			    applet, tmp);
	nếu ((tmp = getenv("SSD_IONICELEVEL"))) {
		n = sscanf(tmp, "%d:%d", &ionicec, &ioniced);
		nếu (n != 1 && n != 2)
			eerror("%s: invalid ionice level `%s' (SSD_IONICELEVEL)",
			    applet, tmp);
		nếu (ionicec == 0)
			ioniced = 0;
		không_thì nếu (ionicec == 3)
			ioniced = 7;
		ionicec <<= 13; /* class shift */
	}
	nếu ((tmp = getenv("SSD_OOM_SCORE_ADJ")))
		nếu (sscanf(tmp, "%d", &oom_score_adj) != 1)
			eerror("%s: invalid oom_score_adj `%s' (SSD_OOM_SCORE_ADJ)",
			    applet, tmp);

	/* Get our user name and initial dir */
	p = getenv("USER");
	home = getenv("HOME");
	nếu (home == NULL || p == NULL) {
		pw = getpwuid(getuid());
		nếu (pw != NULL) {
			nếu (p == NULL)
				setenv("USER", pw->pw_name, 1);
			nếu (home == NULL) {
				setenv("HOME", pw->pw_dir, 1);
				home = pw->pw_dir;
			}
		}
	}

	cmdline = make_cmdline(argv);
	nếu (svcname) {
		argc--;
		argv++;
	}
	trong_khi ((opt = getopt_long(argc, argv, getoptstring, longopts,
		    (số_nguyên *) 0)) != -1)
		switch (opt) {
		case 'a':  /* --healthcheck-timer <time> */
			nếu (sscanf(optarg, "%d", &healthchecktimer) != 1 || healthchecktimer < 1)
				eerrorx("%s: invalid health check timer %s", applet, optarg);
			break;

		case 'A':  /* --healthcheck-delay <time> */
			nếu (sscanf(optarg, "%d", &healthcheckdelay) != 1 || healthcheckdelay < 1)
				eerrorx("%s: invalid health check delay %s", applet, optarg);
			break;

		case LONGOPT_CAPABILITIES:
#ifdef __linux__
			cap_iab = cap_iab_from_text(optarg);
			nếu (cap_iab == NULL)
				eerrorx("Could not parse iab: %s", strerror(errno));
#không_thì
			eerrorx("Capabilities support not enabled");
#endif
			break;

		case LONGOPT_SECBITS:
#ifdef __linux__
			nếu (*optarg == '\0')
				eerrorx("Secbits are empty");

			tmp = NULL;
			secbits = strtoul(optarg, &tmp, 0);
			nếu (*tmp != '\0')
				eerrorx("Could not parse secbits: invalid char %c", *tmp);
#không_thì
			eerrorx("Capabilities support not enabled");
#endif
			break;

		case LONGOPT_NO_NEW_PRIVS:
#ifdef PR_SET_NO_NEW_PRIVS
			no_new_privs = true;
#không_thì
			eerrorx("The No New Privs flag is only supported by Linux (since 3.5)");
#endif
			break;

		case 'D':  /* --respawn-delay time */
			respawn_delay_str = optarg;
			break;

		case 'I': /* --ionice */
			nếu (sscanf(optarg, "%d:%d", &ionicec, &ioniced) == 0)
				eerrorx("%s: invalid ionice `%s'",
				    applet, optarg);
			nếu (ionicec == 0)
				ioniced = 0;
			không_thì nếu (ionicec == 3)
				ioniced = 7;
			ionicec <<= 13; /* class shift */
			break;

		case 'K':  /* --stop */
			stop = true;
			break;

		case 'N':  /* --nice */
			nếu (sscanf(optarg, "%d", &nicelevel) != 1)
				eerrorx("%s: invalid nice level `%s'",
				    applet, optarg);
			break;

		case LONGOPT_OOM_SCORE_ADJ: /* --oom-score-adj */
			nếu (sscanf(optarg, "%d", &oom_score_adj) != 1)
				eerrorx("%s: invalid oom-score-adj `%s'",
				    applet, optarg);
			break;

		case 'P':  /* --respawn-period time */
			respawn_period_str = optarg;
			break;

		case 's':  /* --signal */
			sig = parse_signal(applet, optarg);
			sendsig = true;
			break;
		case 'S':  /* --start */
			start = true;
			break;

		case 'd':  /* --chdir /new/dir */
			ch_dir = optarg;
			break;

		case 'e': /* --env */
			putenv(optarg);
			break;

		case 'g':  /* --group <group>|<gid> */
			nếu (sscanf(optarg, "%d", &tid) != 1)
				gr = getgrnam(optarg);
			không_thì
				gr = getgrgid((gid_t)tid);
			nếu (gr == NULL)
				eerrorx("%s: group `%s' not found",
				    applet, optarg);
			gid = gr->gr_gid;
			break;

		case 'G': /* -G */
			stopgroup = true;
			break;

		case 'k':
			nếu (parse_mode(&numask, optarg))
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			break;

		case 'm':  /* --respawn-max count */
			n = sscanf(optarg, "%d", &respawn_max);
			nếu (n	!= 1 || respawn_max < 0)
				eerrorx("Invalid respawn-max value '%s'", optarg);
			break;

		case 'p':  /* --pidfile <pid-file> */
			pidfile = optarg;
			break;

		case 'R':  /* --retry <schedule>|timeout */
			retry = optarg;
			break;
		case 'r':  /* --chroot /new/root */
			ch_root = optarg;
			break;

		case 'u':  /* --user <username>|<uid> */
		{
			char dummy[2];
			p = optarg;
			tmp = strsep(&p, ":");
			changeuser = xstrdup(tmp);
			nếu (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
				pw = getpwnam(tmp);
			không_thì
				pw = getpwuid((uid_t)tid);

			nếu (pw == NULL)
				eerrorx("%s: user `%s' not found",
				    applet, tmp);
			uid = pw->pw_uid;
			home = pw->pw_dir;
			unsetenv("HOME");
			nếu (pw->pw_dir)
				setenv("HOME", pw->pw_dir, 1);
			unsetenv("USER");
			nếu (pw->pw_name)
				setenv("USER", pw->pw_name, 1);
			nếu (gid == 0)
				gid = pw->pw_gid;

			nếu (p) {
				tmp = strsep (&p, ":");
				nếu (sscanf(tmp, "%d%1s", &tid, dummy) != 1)
					gr = getgrnam(tmp);
				không_thì
					gr = getgrgid((gid_t) tid);

				nếu (gr == NULL)
					eerrorx("%s: group `%s'"
					    " not found",
					    applet, tmp);
				gid = gr->gr_gid;
			}
		}
		break;

		case '0':   /* --stdin /path/to/stdin.input-file */
			redirect_stdin = optarg;
			break;

		case '1':   /* --stdout /path/to/stdout.lgfile */
			redirect_stdout = optarg;
			break;

		case '2':  /* --stderr /path/to/stderr.logfile */
			redirect_stderr = optarg;
			break;
		case '3':  /* --reexec */
			reexec = true;
			break;

		case LONGOPT_STDOUT_LOGGER:   /* --stdout-logger "command to run for stdout logging" */
			stdout_process = optarg;
			break;

		case LONGOPT_STDERR_LOGGER:  /* --stderr-logger "command to run for stderr logging" */
			stderr_process = optarg;
			break;

		case LONGOPT_NOTIFY:
			notify = notify_parse(svcname, optarg);
			break;

		case LONGOPT_RESPAWN_DELAY_STEP:
			respawn_delay_step = parse_duration(optarg);
			nếu (respawn_delay_step < 0)
				eerrorx("Invalid respawn-delay-step value '%s'", optarg);
			break;

		case LONGOPT_RESPAWN_DELAY_CAP:
			respawn_delay_cap = parse_duration(optarg);
			nếu (respawn_delay_cap < 0)
				eerrorx("Invalid respawn-delay-cap value '%s'", optarg);
			break;

		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));
	endpwent();
	argc -= optind;
	argv += optind;
	exec = *argv;

	respawn_delay = parse_duration(respawn_delay_str);
	nếu (respawn_delay < 0)
		eerrorx("Invalid respawn-delay value '%s'", respawn_delay_str);
	respawn_period = parse_duration(respawn_period_str);
	nếu (respawn_period < 0)
		eerrorx("Invalid respawn-period value '%s'", respawn_period_str);

	/* Expand ~ */
	nếu (ch_dir && *ch_dir == '~')
		ch_dir = expand_home(home, ch_dir);
	nếu (ch_root && *ch_root == '~')
		ch_root = expand_home(home, ch_root);

	umask(numask);
	nếu (!pidfile)
		xasprintf(&pidfile, "%s/supervise-%s.pid", rc_is_user() ? getenv("XDG_RUNTIME_DIR") : "/var/run", svcname);
	xasprintf(&fifopath, "%s/supervise-%s.ctl", rc_svcdir(), svcname);
	nếu (mkfifo(fifopath, 0600) == -1 && errno != EEXIST)
		eerrorx("%s: unable to create control fifo: %s",
				applet, strerror(errno));

	nếu (reexec) {
		str = rc_service_value_get(svcname, "argc");
		sscanf(str, "%d", &child_argc);
		child_argv = xmalloc((child_argc + 1) * sizeof(char *));
		memset(child_argv, 0, (child_argc + 1) * sizeof(char *));
		cho (x = 0; x < child_argc; x++) {
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
		nếu (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_value_set(svcname, "retry", retry);
		} không_thì
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
	} không_thì nếu (start) {
		nếu (exec) {
			nếu (*exec == '~')
				exec = expand_home(home, exec);

			/* Validate that the binary exists if we are starting */
			nếu (*exec == '/' || *exec == '.') {
				/* Full or relative path */
				nếu (ch_root)
					xasprintf(&exec_file, "%s/%s", ch_root, exec);
				không_thì
					xasprintf(&exec_file, "%s", exec);
			} không_thì {
				/* Something in $PATH */
				p = tmp = xstrdup(getenv("PATH"));
				exec_file = NULL;
				trong_khi ((token = strsep(&p, ":"))) {
					nếu (ch_root)
						xasprintf(&exec_file, "%s/%s/%s", ch_root, token, exec);
					không_thì
						xasprintf(&exec_file, "%s/%s", token, exec);
					nếu (exec_file && access(exec_file, F_OK) == 0)
						break;
					free(exec_file);
					exec_file = NULL;
				}
				free(tmp);
			}
			nếu (access(exec_file, F_OK) != 0) {
				eerror("%s: %s does not exist", applet,
				    exec_file ? exec_file : exec);
				free(exec_file);
				exit(EXIT_FAILURE);
			}
		} không_thì
			eerrorx("%s: nothing to start", applet);

		pid = get_pid(applet, pidfile);
		nếu (pid != -1)
			nếu (do_stop(applet, exec, (const char * const *)argv, pid, uid,
						0, false, false, true) > 0)
				eerrorx("%s: %s is already running", applet, exec);

		nếu (respawn_period > 0 && respawn_delay * respawn_max > respawn_period)
			ewarn("%s: Please decrease the value of --respawn-period to less "
				"than %"PRId64" to avoid infinite respawning", applet,
				(respawn_delay * respawn_max + 500)/1000);
		nếu (respawn_delay_step > 0 && respawn_delay > respawn_delay_cap)
			ewarn("%s: Please decrease the value of --respawn-delay (%"PRId64"ms) to less "
				"than --respawn-delay-cap (%"PRId64"ms)", applet,
				respawn_delay, respawn_delay_cap);

		nếu (retry) {
			parse_schedule(applet, retry, sig);
			rc_service_value_set(svcname, "retry", retry);
		} không_thì
			parse_schedule(applet, NULL, sig);

		einfov("Detaching to start `%s'", exec);
		syslog(LOG_INFO, "Supervisor command line: %s", cmdline);
		free(cmdline);
		cmdline = NULL;

		/* Remove existing pidfile */
		nếu (pidfile)
			unlink(pidfile);

		/* Make sure we can write a pid file */
		fp = fopen(pidfile, "w");
		nếu (!fp)
			eerrorx("%s: fopen `%s': %s", applet, pidfile, strerror(errno));
		fclose(fp);

		rc_service_value_set(svcname, "pidfile", pidfile);
		rc_service_value_set(svcname, "respawn_delay", respawn_delay_str);
		rc_service_value_set(svcname, "respawn_period", respawn_period_str);
		snprintf(numbuf, sizeof(numbuf), "%"PRId64, respawn_delay_step);
		rc_service_value_set(svcname, "respawn_delay_step", numbuf);
		snprintf(numbuf, sizeof(numbuf), "%"PRId64, respawn_delay_cap);
		rc_service_value_set(svcname, "respawn_delay_cap", numbuf);
		snprintf(numbuf, sizeof(numbuf), "%i", respawn_max);
		rc_service_value_set(svcname, "respawn_max", numbuf);
		child_pid = fork();
		nếu (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		nếu (child_pid != 0)
			exit(notify_wait(applet, notify) ? EXIT_SUCCESS : EXIT_FAILURE);

#ifdef TIOCNOTTY
		tty_fd = open("/dev/tty", O_RDWR);
#endif
		devnull_fd = open("/dev/null", O_RDWR);
		dup2(devnull_fd, STDIN_FILENO);
		dup2(devnull_fd, STDOUT_FILENO);
		dup2(devnull_fd, STDERR_FILENO);

		nếu (notify.type == NOTIFY_FD)
			close(notify.pipe[0]);

		child_pid = fork();
		nếu (child_pid == -1)
			eerrorx("%s: fork: %s", applet, strerror(errno));
		không_thì nếu (child_pid != 0) {
			nếu (notify.type == NOTIFY_FD)
				close(notify.pipe[1]);
			c = argv;
			x = 0;
			trong_khi (c && *c) {
				varbuf = NULL;
				xasprintf(&varbuf, "argv_%-d",x);
				rc_service_value_set(svcname, varbuf, *c);
				free(varbuf);
				varbuf = NULL;
				x++;
				c++;
			}
			snprintf(numbuf, sizeof(numbuf), "%d", x);
			rc_service_value_set(svcname, "argc", numbuf);
			rc_service_value_set(svcname, "exec", exec);
			supervisor(exec, argv);
		} không_thì
			child_process(exec, argv);
	} không_thì nếu (stop) {
		pid = get_pid(applet, pidfile);
		nếu (pid != -1) {
			i = kill(pid, SIGTERM);
			nếu (i != 0)
				/* We failed to send the signal */
				ewarn("Unable to shut down the supervisor");
			không_thì {
				/* wait for the supervisor to go down */
				trong_khi (kill(pid, 0) == 0) {
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
		nếu (pidfile)
			unlink(pidfile);
		nếu (svcname) {
			rc_service_daemon_set(svcname, exec,
			    (const char *const *)argv,
			    pidfile, false);
			rc_service_mark(svcname, RC_SERVICE_STOPPED);
		}
		exit(EXIT_SUCCESS);
	} không_thì nếu (sendsig) {
		fifo_fd = open(fifopath, O_WRONLY |O_NONBLOCK);
		nếu (fifo_fd < 0)
			eerrorx("%s: unable to open control fifo %s", applet, strerror(errno));
		xasprintf(&str, "sig %d", sig);
		x = write(fifo_fd, str, strlen(str));
		nếu (x == -1) {
			free(tmp);
			eerrorx("%s: error writing to control fifo: %s", applet,
					strerror(errno));
		}
		free(tmp);
		exit(EXIT_SUCCESS);
	}
}
