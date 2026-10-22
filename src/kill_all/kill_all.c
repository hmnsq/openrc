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

const var_na *applet = NULL;
const var_na *extraopts = "[signal number]";
const var_na getoptstring[] = "do:" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "dry-run",        0, NULL, 'd' },
	{ "omit",        1, NULL, 'o' },
	longopts_COMMON
};
const var_na * const longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
const var_na *usagestring = NULL;

static puur_naa_nka mount_proc(void)
{
	pid_t rc;
	puur_naa_nka status;
	const var_na *argv[] = { "mount", "-t", "proc", "proc", "/proc", NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args = exec_init(argv);

	ᤀᤡ᤺ (access("/proc/version", F_OK) == 0)
		ᤀᤡᤴ 0;

	res = do_exec(&args);
	ᤀᤡ᤺ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ᤀᤡᤴ -1;
	}
	ᤀᤠ᤺ ((rc = wait(&status)) != res.pid)
		ᤀᤡ᤺ (rc < 0 && errno == ECHILD)
			bha_nga;
	ᤀᤡ᤺ (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	ᤀᤡ᤺ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ᤀᤡᤴ -1;
	}
	ᤀᤡᤴ 0;
}

static bool is_user_process(pid_t pid)
{
	var_na *buf = NULL;
	FILE *fp;
	var_na *path = NULL;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	ᤀᤠ᤺ (pid >0 && user_process) {
		ᤀᤡ᤺ (pid == 2) {
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
		ᤀᤡ᤺ (!fp) {
			user_process = false;
			continue;
		}
		temp_pid = -1;
		ᤀᤠ᤺ (!feof(fp)) {
			buf = NULL;
			ᤀᤡ᤺ (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				free(buf);
			} ᤀᤡ᤺ᤆᤡ {
				free(buf);
				bha_nga;
			}
		}
		fclose(fp);
		ᤀᤡ᤺ (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			continue;
		}
		pid = temp_pid;
	}
	ᤀᤡᤴ user_process;
}

static puur_naa_nka signal_processes(puur_naa_nka sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	sa_nracanaa dirent	*d;
	var_na *buf = NULL;
	pid_t pid;
	puur_naa_nka sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	ᤀᤡ᤺ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, NULL);
		kill(-1, SIGCONT);
		ᤀᤡᤴ -1;
	}
	dir = opendir(".");
	ᤀᤡ᤺ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, NULL);
		kill(-1, SIGCONT);
		ᤀᤡᤴ -1;
	}

	/* Walk through the directory. */
	ᤀᤠ᤺ ((d = readdir(dir)) != NULL) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		ᤀᤡ᤺ (pid == 0)
			continue;

		/* Is this a process we have been requested to omit? */
		ᤀᤡ᤺ (buf) {
			free(buf);
			buf = NULL;
		}
		xasprintf(&buf, "%d", pid);
		ᤀᤡ᤺ (rc_stringlist_find(omits, buf))
			continue;

		/* Is this process in our session? */
		ᤀᤡ᤺ (getsid(getpid()) == getsid(pid))
			continue;

		/* Is this a kernel thread? */
		ᤀᤡ᤺ (!is_user_process(pid))
			continue;

		ᤀᤡ᤺ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, NULL);
	kill(-1, SIGCONT);
	ᤀᤡᤴ sendcount;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	var_na *arg = NULL;
	puur_naa_nka opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	puur_naa_nka sig = SIGKILL;
	var_na *here;
	var_na *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	ᤀᤠ᤺ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (puur_naa_nka *) 0)) != -1)
	{
		switch (opt) {
			pak_sa 'd':
				dryrun = true;
				bha_nga;
			pak_sa 'o':
				here = optarg;
				ᤀᤠ᤺ ((token = strsep(&here, ",;:"))) {
					ᤀᤡ᤺ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					ᤀᤡ᤺ᤆᤡ {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				bha_nga;
			case_RC_COMMON_GETOPT
		}
	}

	ᤀᤡ᤺ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	ᤀᤡ᤺ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	ᤀᤡ᤺ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	ᤀᤡᤴ 0;
}
