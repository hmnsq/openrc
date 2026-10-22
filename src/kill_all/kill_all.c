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


#شامل <dirent.h>
#شامل <errno.h>
#شامل <getopt.h>
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <unistd.h>
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <stdbool.h>

#شامل "einfo.h"
#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = "[signal number]";
ثابت نویسه getoptstring[] = "do:" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "dry-run",        0, پوچ, 'd' },
	{ "omit",        1, پوچ, 'o' },
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ثابت نویسه *usagestring = پوچ;

ایستا صحیح mount_proc(تهی)
{
	pid_t rc;
	صحیح status;
	ثابت نویسه *argv[] = { "mount", "-t", "proc", "proc", "/proc", پوچ };
	ساختار exec_result res;
	ساختار exec_args args = exec_init(argv);

	اگر (access("/proc/version", F_OK) == 0)
		بازگشت 0;

	res = do_exec(&args);
	اگر (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		بازگشت -1;
	}
	وقتی ((rc = wait(&status)) != res.pid)
		اگر (rc < 0 && errno == ECHILD)
			بشکن;
	اگر (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	اگر (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		بازگشت -1;
	}
	بازگشت 0;
}

ایستا bool is_user_process(pid_t pid)
{
	نویسه *buf = پوچ;
	FILE *fp;
	نویسه *path = پوچ;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	وقتی (pid >0 && user_process) {
		اگر (pid == 2) {
			user_process = false;
			ادامه;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		آزاد(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		اگر (!fp) {
			user_process = false;
			ادامه;
		}
		temp_pid = -1;
		وقتی (!feof(fp)) {
			buf = پوچ;
			اگر (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				آزاد(buf);
			} وگرنه {
				آزاد(buf);
				بشکن;
			}
		}
		fclose(fp);
		اگر (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			ادامه;
		}
		pid = temp_pid;
	}
	بازگشت user_process;
}

ایستا صحیح signal_processes(صحیح sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	ساختار dirent	*d;
	نویسه *buf = پوچ;
	pid_t pid;
	صحیح sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	اگر (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, پوچ);
		kill(-1, SIGCONT);
		بازگشت -1;
	}
	dir = opendir(".");
	اگر (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, پوچ);
		kill(-1, SIGCONT);
		بازگشت -1;
	}

	/* Walk through the directory. */
	وقتی ((d = readdir(dir)) != پوچ) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		اگر (pid == 0)
			ادامه;

		/* Is this a process we have been requested to omit? */
		اگر (buf) {
			آزاد(buf);
			buf = پوچ;
		}
		xasprintf(&buf, "%d", pid);
		اگر (rc_stringlist_find(omits, buf))
			ادامه;

		/* Is this process in our session? */
		اگر (getsid(getpid()) == getsid(pid))
			ادامه;

		/* Is this a kernel thread? */
		اگر (!is_user_process(pid))
			ادامه;

		اگر (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		وگرنه اگر (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, پوچ);
	kill(-1, SIGCONT);
	بازگشت sendcount;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	نویسه *arg = پوچ;
	صحیح opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	صحیح sig = SIGKILL;
	نویسه *here;
	نویسه *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
			حالت 'd':
				dryrun = true;
				بشکن;
			حالت 'o':
				here = optarg;
				وقتی ((token = strsep(&here, ",;:"))) {
					اگر ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					وگرنه {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				بشکن;
			case_RC_COMMON_GETOPT
		}
	}

	اگر (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	اگر (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	اگر (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	بازگشت 0;
}
