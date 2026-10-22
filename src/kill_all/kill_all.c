/*
 * kill_all.c
 * Sends a signal to all processes on the system.
 */

/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */


#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "einfo.h"
#include "rc.h"
#include "rc_exec.h"
#include "_usage.h"
#include "helpers.h"

const char *applet = NULL;
const char *extraopts = "[signal number]";
const char getoptstring[] = "do:" getoptstring_COMMON;
const struct option longopts[] = {
	{ "dry-run",        0, NULL, 'd' },
	{ "omit",        1, NULL, 'o' },
	longopts_COMMON
};
const char * const longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
const char *usagestring = NULL;

static 整数 mount_proc(void)
{
	pid_t rc;
	整数 status;
	const char *argv[] = { "mount", "-t", "proc", "proc", "/proc", NULL };
	struct exec_result res;
	struct exec_args args = exec_init(argv);

	如果 (access("/proc/version", F_OK) == 0)
		返回 0;

	res = do_exec(&args);
	如果 (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		返回 -1;
	}
	当 ((rc = wait(&status)) != res.pid)
		如果 (rc < 0 && errno == ECHILD)
			break;
	如果 (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	如果 (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		返回 -1;
	}
	返回 0;
}

static bool is_user_process(pid_t pid)
{
	char *buf = NULL;
	FILE *fp;
	char *path = NULL;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	当 (pid >0 && user_process) {
		如果 (pid == 2) {
			user_process = false;
			continue;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		free(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		如果 (!fp) {
			user_process = false;
			continue;
		}
		temp_pid = -1;
		当 (!feof(fp)) {
			buf = NULL;
			如果 (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				free(buf);
			} 否则 {
				free(buf);
				break;
			}
		}
		fclose(fp);
		如果 (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			continue;
		}
		pid = temp_pid;
	}
	返回 user_process;
}

static 整数 signal_processes(整数 sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	struct dirent	*d;
	char *buf = NULL;
	pid_t pid;
	整数 sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	如果 (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, NULL);
		kill(-1, SIGCONT);
		返回 -1;
	}
	dir = opendir(".");
	如果 (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, NULL);
		kill(-1, SIGCONT);
		返回 -1;
	}

	/* Walk through the directory. */
	当 ((d = readdir(dir)) != NULL) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		如果 (pid == 0)
			continue;

		/* Is this a process we have been requested to omit? */
		如果 (buf) {
			free(buf);
			buf = NULL;
		}
		xasprintf(&buf, "%d", pid);
		如果 (rc_stringlist_find(omits, buf))
			continue;

		/* Is this process in our session? */
		如果 (getsid(getpid()) == getsid(pid))
			continue;

		/* Is this a kernel thread? */
		如果 (!is_user_process(pid))
			continue;

		如果 (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		否则 如果 (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, NULL);
	kill(-1, SIGCONT);
	返回 sendcount;
}

整数 主要(整数 argc, char **argv)
{
	char *arg = NULL;
	整数 opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	整数 sig = SIGKILL;
	char *here;
	char *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	当 ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (整数 *) 0)) != -1)
	{
		switch (opt) {
			case 'd':
				dryrun = true;
				break;
			case 'o':
				here = optarg;
				当 ((token = strsep(&here, ",;:"))) {
					如果 ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					否则 {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				break;
			case_RC_COMMON_GETOPT
		}
	}

	如果 (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	如果 (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	如果 (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	返回 0;
}
