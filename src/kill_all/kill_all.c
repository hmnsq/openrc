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


#ضمن <dirent.h>
#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <signal.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <syslog.h>
#ضمن <unistd.h>
#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <stdbool.h>

#ضمن "einfo.h"
#ضمن "rc.h"
#ضمن "rc_exec.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = "[signal number]";
ثابت حرف getoptstring[] = "do:" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "dry-run",        0, عدم, 'd' },
	{ "omit",        1, عدم, 'o' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ثابت حرف *usagestring = عدم;

ساكن صحيح mount_proc(فراغ)
{
	pid_t rc;
	صحيح status;
	ثابت حرف *argv[] = { "mount", "-t", "proc", "proc", "/proc", عدم };
	بنية exec_result res;
	بنية exec_args args = exec_init(argv);

	אם (access("/proc/version", F_OK) == 0)
		החזר 0;

	res = do_exec(&args);
	אם (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		החזר -1;
	}
	כלעוד ((rc = wait(&status)) != res.pid)
		אם (rc < 0 && errno == ECHILD)
			اكسر;
	אם (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	אם (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		החזר -1;
	}
	החזר 0;
}

ساكن bool is_user_process(pid_t pid)
{
	حرف *buf = عدم;
	FILE *fp;
	حرف *path = عدم;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	כלעוד (pid >0 && user_process) {
		אם (pid == 2) {
			user_process = false;
			استمر;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		حرر(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		אם (!fp) {
			user_process = false;
			استمر;
		}
		temp_pid = -1;
		כלעוד (!feof(fp)) {
			buf = عدم;
			אם (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				حرر(buf);
			} אחרת {
				حرر(buf);
				اكسر;
			}
		}
		fclose(fp);
		אם (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			استمر;
		}
		pid = temp_pid;
	}
	החזר user_process;
}

ساكن صحيح signal_processes(صحيح sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	بنية dirent	*d;
	حرف *buf = عدم;
	pid_t pid;
	صحيح sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	אם (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, عدم);
		kill(-1, SIGCONT);
		החזר -1;
	}
	dir = opendir(".");
	אם (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, عدم);
		kill(-1, SIGCONT);
		החזר -1;
	}

	/* Walk through the directory. */
	כלעוד ((d = readdir(dir)) != عدم) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		אם (pid == 0)
			استمر;

		/* Is this a process we have been requested to omit? */
		אם (buf) {
			حرر(buf);
			buf = عدم;
		}
		xasprintf(&buf, "%d", pid);
		אם (rc_stringlist_find(omits, buf))
			استمر;

		/* Is this process in our session? */
		אם (getsid(getpid()) == getsid(pid))
			استمر;

		/* Is this a kernel thread? */
		אם (!is_user_process(pid))
			استمر;

		אם (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		אחרת אם (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, عدم);
	kill(-1, SIGCONT);
	החזר sendcount;
}

صحيح main(صحيح argc, حرف **argv)
{
	حرف *arg = عدم;
	صحيح opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	صحيح sig = SIGKILL;
	حرف *here;
	حرف *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	כלעוד ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
			حالة 'd':
				dryrun = true;
				اكسر;
			حالة 'o':
				here = optarg;
				כלעוד ((token = strsep(&here, ",;:"))) {
					אם ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					אחרת {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				اكسر;
			case_RC_COMMON_GETOPT
		}
	}

	אם (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	אם (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	אם (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	החזר 0;
}
