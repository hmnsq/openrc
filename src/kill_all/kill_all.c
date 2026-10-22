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


#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = "[signal number]";
ಧ್ರುವ ವರ್ಣ getoptstring[] = "do:" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "dry-run",        0, ರಿಕ್ತ, 'd' },
	{ "omit",        1, ರಿಕ್ತ, 'o' },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ಧ್ರುವ ವರ್ಣ *usagestring = ರಿಕ್ತ;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mount_proc(ಶೂನ್ಯ)
{
	pid_t rc;
	ಪೂರ್ಣಾಙ್ಕ status;
	ಧ್ರುವ ವರ್ಣ *argv[] = { "mount", "-t", "proc", "proc", "/proc", ರಿಕ್ತ };
	ಸಂರಚನಾ exec_result res;
	ಸಂರಚನಾ exec_args args = exec_init(argv);

	ಯದಿ (access("/proc/version", F_OK) == 0)
		ಪ್ರತ್ಯಾಗಮ 0;

	res = do_exec(&args);
	ಯದಿ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯಾವತ್ ((rc = wait(&status)) != res.pid)
		ಯದಿ (rc < 0 && errno == ECHILD)
			ಭಙ್ಗ;
	ಯದಿ (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	ಯದಿ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ bool is_user_process(pid_t pid)
{
	ವರ್ಣ *buf = ರಿಕ್ತ;
	FILE *fp;
	ವರ್ಣ *path = ರಿಕ್ತ;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	ಯಾವತ್ (pid >0 && user_process) {
		ಯದಿ (pid == 2) {
			user_process = false;
			ಅನುವರ್ತ;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		ಮುಕ್ತ(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		ಯದಿ (!fp) {
			user_process = false;
			ಅನುವರ್ತ;
		}
		temp_pid = -1;
		ಯಾವತ್ (!feof(fp)) {
			buf = ರಿಕ್ತ;
			ಯದಿ (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				ಮುಕ್ತ(buf);
			} ಅನ್ಯಥಾ {
				ಮುಕ್ತ(buf);
				ಭಙ್ಗ;
			}
		}
		fclose(fp);
		ಯದಿ (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			ಅನುವರ್ತ;
		}
		pid = temp_pid;
	}
	ಪ್ರತ್ಯಾಗಮ user_process;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_processes(ಪೂರ್ಣಾಙ್ಕ sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	ಸಂರಚನಾ dirent	*d;
	ವರ್ಣ *buf = ರಿಕ್ತ;
	pid_t pid;
	ಪೂರ್ಣಾಙ್ಕ sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	ಯದಿ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ರಿಕ್ತ);
		kill(-1, SIGCONT);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	dir = opendir(".");
	ಯದಿ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ರಿಕ್ತ);
		kill(-1, SIGCONT);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	/* Walk through the directory. */
	ಯಾವತ್ ((d = readdir(dir)) != ರಿಕ್ತ) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		ಯದಿ (pid == 0)
			ಅನುವರ್ತ;

		/* Is this a process we have been requested to omit? */
		ಯದಿ (buf) {
			ಮುಕ್ತ(buf);
			buf = ರಿಕ್ತ;
		}
		xasprintf(&buf, "%d", pid);
		ಯದಿ (rc_stringlist_find(omits, buf))
			ಅನುವರ್ತ;

		/* Is this process in our session? */
		ಯದಿ (getsid(getpid()) == getsid(pid))
			ಅನುವರ್ತ;

		/* Is this a kernel thread? */
		ಯದಿ (!is_user_process(pid))
			ಅನುವರ್ತ;

		ಯದಿ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		ಅನ್ಯಥಾ ಯದಿ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ರಿಕ್ತ);
	kill(-1, SIGCONT);
	ಪ್ರತ್ಯಾಗಮ sendcount;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ವರ್ಣ *arg = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	ಪೂರ್ಣಾಙ್ಕ sig = SIGKILL;
	ವರ್ಣ *here;
	ವರ್ಣ *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
			ಪಕ್ಷ 'd':
				dryrun = true;
				ಭಙ್ಗ;
			ಪಕ್ಷ 'o':
				here = optarg;
				ಯಾವತ್ ((token = strsep(&here, ",;:"))) {
					ಯದಿ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					ಅನ್ಯಥಾ {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				ಭಙ್ಗ;
			case_RC_COMMON_GETOPT
		}
	}

	ಯದಿ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	ಯದಿ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	ಯದಿ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	ಪ್ರತ್ಯಾಗಮ 0;
}
