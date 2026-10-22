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


#অন্তর্ভাৱ <dirent.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <getopt.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <stdbool.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "rc_exec.h"
#অন্তর্ভাৱ "_usage.h"
#অন্তর্ভাৱ "helpers.h"

ধ্রুৱ ৱর্ণ *applet = রিক্ত;
ধ্রুৱ ৱর্ণ *extraopts = "[signal number]";
ধ্রুৱ ৱর্ণ getoptstring[] = "do:" getoptstring_COMMON;
ধ্রুৱ সংরচনা option longopts[] = {
	{ "dry-run",        0, রিক্ত, 'd' },
	{ "omit",        1, রিক্ত, 'o' },
	longopts_COMMON
};
ধ্রুৱ ৱর্ণ * ধ্রুৱ longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ধ্রুৱ ৱর্ণ *usagestring = রিক্ত;

স্থির পূর্ণাঙ্ক mount_proc(শূন্য)
{
	pid_t rc;
	পূর্ণাঙ্ক status;
	ধ্রুৱ ৱর্ণ *argv[] = { "mount", "-t", "proc", "proc", "/proc", রিক্ত };
	সংরচনা exec_result res;
	সংরচনা exec_args args = exec_init(argv);

	যদি (access("/proc/version", F_OK) == 0)
		ঘূৰাই 0;

	res = do_exec(&args);
	যদি (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ঘূৰাই -1;
	}
	যেতিয়ালৈ ((rc = wait(&status)) != res.pid)
		যদি (rc < 0 && errno == ECHILD)
			ভঙ্গ;
	যদি (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	যদি (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ঘূৰাই -1;
	}
	ঘূৰাই 0;
}

স্থির bool is_user_process(pid_t pid)
{
	ৱর্ণ *buf = রিক্ত;
	FILE *fp;
	ৱর্ণ *path = রিক্ত;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	যেতিয়ালৈ (pid >0 && user_process) {
		যদি (pid == 2) {
			user_process = false;
			অনুৱর্ত;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		মুক্ত(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		যদি (!fp) {
			user_process = false;
			অনুৱর্ত;
		}
		temp_pid = -1;
		যেতিয়ালৈ (!feof(fp)) {
			buf = রিক্ত;
			যদি (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				মুক্ত(buf);
			} অন্যথা {
				মুক্ত(buf);
				ভঙ্গ;
			}
		}
		fclose(fp);
		যদি (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			অনুৱর্ত;
		}
		pid = temp_pid;
	}
	ঘূৰাই user_process;
}

স্থির পূর্ণাঙ্ক signal_processes(পূর্ণাঙ্ক sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	সংরচনা dirent	*d;
	ৱর্ণ *buf = রিক্ত;
	pid_t pid;
	পূর্ণাঙ্ক sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	যদি (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, রিক্ত);
		kill(-1, SIGCONT);
		ঘূৰাই -1;
	}
	dir = opendir(".");
	যদি (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, রিক্ত);
		kill(-1, SIGCONT);
		ঘূৰাই -1;
	}

	/* Walk through the directory. */
	যেতিয়ালৈ ((d = readdir(dir)) != রিক্ত) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		যদি (pid == 0)
			অনুৱর্ত;

		/* Is this a process we have been requested to omit? */
		যদি (buf) {
			মুক্ত(buf);
			buf = রিক্ত;
		}
		xasprintf(&buf, "%d", pid);
		যদি (rc_stringlist_find(omits, buf))
			অনুৱর্ত;

		/* Is this process in our session? */
		যদি (getsid(getpid()) == getsid(pid))
			অনুৱর্ত;

		/* Is this a kernel thread? */
		যদি (!is_user_process(pid))
			অনুৱর্ত;

		যদি (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		অন্যথা যদি (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, রিক্ত);
	kill(-1, SIGCONT);
	ঘূৰাই sendcount;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	ৱর্ণ *arg = রিক্ত;
	পূর্ণাঙ্ক opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	পূর্ণাঙ্ক sig = SIGKILL;
	ৱর্ণ *here;
	ৱর্ণ *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	যেতিয়ালৈ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		ৱিকল্প (opt) {
			পক্ষ 'd':
				dryrun = true;
				ভঙ্গ;
			পক্ষ 'o':
				here = optarg;
				যেতিয়ালৈ ((token = strsep(&here, ",;:"))) {
					যদি ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					অন্যথা {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				ভঙ্গ;
			case_RC_COMMON_GETOPT
		}
	}

	যদি (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	যদি (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	যদি (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	ঘূৰাই 0;
}
