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


#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = "[signal number]";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "do:" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "dry-run",        0, ଖାଲି, 'd' },
	{ "omit",        1, ଖାଲି, 'o' },
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mount_proc(ଶୂନ୍ୟ)
{
	pid_t rc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	ଅଚଳ ଅକ୍ଷର *argv[] = { "mount", "-t", "proc", "proc", "/proc", ଖାଲି };
	ସଂରଚନା exec_result res;
	ସଂରଚନା exec_args args = exec_init(argv);

	ଯଦି (access("/proc/version", F_OK) == 0)
		ଫେରନ୍ତୁ 0;

	res = do_exec(&args);
	ଯଦି (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ଫେରନ୍ତୁ -1;
	}
	ଯେତେବେଳେ ((rc = wait(&status)) != res.pid)
		ଯଦି (rc < 0 && errno == ECHILD)
			ଭାଙ୍ଗ;
	ଯଦି (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	ଯଦି (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ଫେରନ୍ତୁ -1;
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର bool is_user_process(pid_t pid)
{
	ଅକ୍ଷର *buf = ଖାଲି;
	FILE *fp;
	ଅକ୍ଷର *path = ଖାଲି;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	ଯେତେବେଳେ (pid >0 && user_process) {
		ଯଦି (pid == 2) {
			user_process = false;
			ଜାରି;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		ମୁକ୍ତ(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		ଯଦି (!fp) {
			user_process = false;
			ଜାରି;
		}
		temp_pid = -1;
		ଯେତେବେଳେ (!feof(fp)) {
			buf = ଖାଲି;
			ଯଦି (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				ମୁକ୍ତ(buf);
			} ଅନ୍ୟଥା {
				ମୁକ୍ତ(buf);
				ଭାଙ୍ଗ;
			}
		}
		fclose(fp);
		ଯଦି (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			ଜାରି;
		}
		pid = temp_pid;
	}
	ଫେରନ୍ତୁ user_process;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା signal_processes(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	ସଂରଚନା dirent	*d;
	ଅକ୍ଷର *buf = ଖାଲି;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	ଯଦି (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ଖାଲି);
		kill(-1, SIGCONT);
		ଫେରନ୍ତୁ -1;
	}
	dir = opendir(".");
	ଯଦି (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ଖାଲି);
		kill(-1, SIGCONT);
		ଫେରନ୍ତୁ -1;
	}

	/* Walk through the directory. */
	ଯେତେବେଳେ ((d = readdir(dir)) != ଖାଲି) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		ଯଦି (pid == 0)
			ଜାରି;

		/* Is this a process we have been requested to omit? */
		ଯଦି (buf) {
			ମୁକ୍ତ(buf);
			buf = ଖାଲି;
		}
		xasprintf(&buf, "%d", pid);
		ଯଦି (rc_stringlist_find(omits, buf))
			ଜାରି;

		/* Is this process in our session? */
		ଯଦି (getsid(getpid()) == getsid(pid))
			ଜାରି;

		/* Is this a kernel thread? */
		ଯଦି (!is_user_process(pid))
			ଜାରି;

		ଯଦି (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		ଅନ୍ୟଥା ଯଦି (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ଖାଲି);
	kill(-1, SIGCONT);
	ଫେରନ୍ତୁ sendcount;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *arg = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = SIGKILL;
	ଅକ୍ଷର *here;
	ଅକ୍ଷର *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
			ସ୍ଥିତି 'd':
				dryrun = true;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'o':
				here = optarg;
				ଯେତେବେଳେ ((token = strsep(&here, ",;:"))) {
					ଯଦି ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					ଅନ୍ୟଥା {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				ଭାଙ୍ଗ;
			case_RC_COMMON_GETOPT
		}
	}

	ଯଦି (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	ଯଦି (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	ଯଦି (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	ଫେରନ୍ତୁ 0;
}
