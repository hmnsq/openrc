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


#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <stdbool.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "rc_exec.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = "[signal number]";
ধ্রুবক অক্ষর getoptstring[] = "do:" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "dry-run",        0, ফাঁকা, 'd' },
	{ "omit",        1, ফাঁকা, 'o' },
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ধ্রুবক অক্ষর *usagestring = ফাঁকা;

স্থির পূর্ণসংখ্যা mount_proc(শূন্য)
{
	pid_t rc;
	পূর্ণসংখ্যা status;
	ধ্রুবক অক্ষর *argv[] = { "mount", "-t", "proc", "proc", "/proc", ফাঁকা };
	কাঠামো exec_ফলাফল res;
	কাঠামো exec_args args = exec_init(argv);

	যদি (access("/proc/version", F_OK) == 0)
		ফেরত 0;

	res = do_exec(&args);
	যদি (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ফেরত -1;
	}
	যখন ((rc = wait(&status)) != res.pid)
		যদি (rc < 0 && errno == ECHILD)
			বিরতি;
	যদি (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	যদি (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ফেরত -1;
	}
	ফেরত 0;
}

স্থির bool is_user_process(pid_t pid)
{
	অক্ষর *buf = ফাঁকা;
	FILE *fp;
	অক্ষর *path = ফাঁকা;
	pid_t অস্থায়ী_pid;
	size_t আকার_2;
	bool user_process = true;

	যখন (pid >0 && user_process) {
		যদি (pid == 2) {
			user_process = false;
			চালু;
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
			চালু;
		}
		অস্থায়ী_pid = -1;
		যখন (!feof(fp)) {
			buf = ফাঁকা;
			যদি (xgetline(&buf, &আকার_2, fp) != -1) {
				sscanf(buf, "PPid: %d", &অস্থায়ী_pid);
				মুক্ত(buf);
			} অন্যথা {
				মুক্ত(buf);
				বিরতি;
			}
		}
		fclose(fp);
		যদি (অস্থায়ী_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			চালু;
		}
		pid = অস্থায়ী_pid;
	}
	ফেরত user_process;
}

স্থির পূর্ণসংখ্যা signal_processes(পূর্ণসংখ্যা sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	কাঠামো dirent	*d;
	অক্ষর *buf = ফাঁকা;
	pid_t pid;
	পূর্ণসংখ্যা sendcount = 0;

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
		sigprocmask(SIG_SETMASK, &oldsigs, ফাঁকা);
		kill(-1, SIGCONT);
		ফেরত -1;
	}
	dir = opendir(".");
	যদি (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ফাঁকা);
		kill(-1, SIGCONT);
		ফেরত -1;
	}

	/* Walk through the directory. */
	যখন ((d = readdir(dir)) != ফাঁকা) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		যদি (pid == 0)
			চালু;

		/* Is this a process we have been requested to omit? */
		যদি (buf) {
			মুক্ত(buf);
			buf = ফাঁকা;
		}
		xasprintf(&buf, "%d", pid);
		যদি (rc_stringlist_find(omits, buf))
			চালু;

		/* Is this process in our session? */
		যদি (getsid(getpid()) == getsid(pid))
			চালু;

		/* Is this a kernel thread? */
		যদি (!is_user_process(pid))
			চালু;

		যদি (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		অন্যথা যদি (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ফাঁকা);
	kill(-1, SIGCONT);
	ফেরত sendcount;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	অক্ষর *arg = ফাঁকা;
	পূর্ণসংখ্যা opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	পূর্ণসংখ্যা sig = SIGKILL;
	অক্ষর *here;
	অক্ষর *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	যখন ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণসংখ্যা *) 0)) != -1)
	{
		নির্বাচন (opt) {
			ক্ষেত্র 'd':
				dryrun = true;
				বিরতি;
			ক্ষেত্র 'o':
				here = optarg;
				যখন ((token = strsep(&here, ",;:"))) {
					যদি ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					অন্যথা {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				বিরতি;
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
	ফেরত 0;
}
