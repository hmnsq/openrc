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

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = "[signal number]";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "do:" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "dry-run",        0, ಖಾಲಿ, 'd' },
	{ "omit",        1, ಖಾಲಿ, 'o' },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ಖಾಲಿ;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mount_proc(ಶೂನ್ಯ)
{
	pid_t rc;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = { "mount", "-t", "proc", "proc", "/proc", ಖಾಲಿ };
	ರಚನೆ exec_ಫಲಿತಾಂಶ res;
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು = exec_init(argv);

	ಆದರೆ (access("/proc/version", F_OK) == 0)
		ಹಿಂತಿರುಗಿ 0;

	res = do_exec(&ವಾದಗಳು);
	ಆದರೆ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆಗ ((rc = wait(&ಸ್ಥಿತಿ_2)) != res.pid)
		ಆದರೆ (rc < 0 && errno == ECHILD)
			ನಿಲ್ಲಿಸು;
	ಆದರೆ (rc != res.pid || !WIFEXITED(ಸ್ಥಿತಿ_2) || WEXITSTATUS(ಸ್ಥಿತಿ_2) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	ಆದರೆ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ಹಿಂತಿರುಗಿ -1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ bool is_user_process(pid_t pid)
{
	ಅಕ್ಷರ *buf = ಖಾಲಿ;
	FILE *fp;
	ಅಕ್ಷರ *ಮಾರ್ಗ = ಖಾಲಿ;
	pid_t ತಾತ್ಕಾಲಿಕ_pid;
	size_t ಗಾತ್ರ_2;
	bool user_process = true;

	ಆಗ (pid >0 && user_process) {
		ಆದರೆ (pid == 2) {
			user_process = false;
			ಮುಂದುವರಿ;
		}
		xasprintf(&ಮಾರ್ಗ, "/proc/%d/status", pid);
		fp = fopen(ಮಾರ್ಗ, "r");
		ಬಿಡುಗಡೆ(ಮಾರ್ಗ);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		ಆದರೆ (!fp) {
			user_process = false;
			ಮುಂದುವರಿ;
		}
		ತಾತ್ಕಾಲಿಕ_pid = -1;
		ಆಗ (!feof(fp)) {
			buf = ಖಾಲಿ;
			ಆದರೆ (xgetline(&buf, &ಗಾತ್ರ_2, fp) != -1) {
				ಅಕ್ಷರಸರಣಿ_ಓದು(buf, "PPid: %d", &ತಾತ್ಕಾಲಿಕ_pid);
				ಬಿಡುಗಡೆ(buf);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಬಿಡುಗಡೆ(buf);
				ನಿಲ್ಲಿಸು;
			}
		}
		fclose(fp);
		ಆದರೆ (ತಾತ್ಕಾಲಿಕ_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			ಮುಂದುವರಿ;
		}
		pid = ತಾತ್ಕಾಲಿಕ_pid;
	}
	ಹಿಂತಿರುಗಿ user_process;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_processes(ಪೂರ್ಣಾಂಕ sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t ಸಂಕೇತಗಳು;
	sigset_t oldsigs;
	DIR *dir;
	ರಚನೆ dirent	*d;
	ಅಕ್ಷರ *buf = ಖಾಲಿ;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&ಸಂಕೇತಗಳು);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &ಸಂಕೇತಗಳು, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	ಆದರೆ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ಖಾಲಿ);
		kill(-1, SIGCONT);
		ಹಿಂತಿರುಗಿ -1;
	}
	dir = opendir(".");
	ಆದರೆ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ಖಾಲಿ);
		kill(-1, SIGCONT);
		ಹಿಂತಿರುಗಿ -1;
	}

	/* Walk through the directory. */
	ಆಗ ((d = readdir(dir)) != ಖಾಲಿ) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		ಆದರೆ (pid == 0)
			ಮುಂದುವರಿ;

		/* Is this a process we have been requested to omit? */
		ಆದರೆ (buf) {
			ಬಿಡುಗಡೆ(buf);
			buf = ಖಾಲಿ;
		}
		xasprintf(&buf, "%d", pid);
		ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(omits, buf))
			ಮುಂದುವರಿ;

		/* Is this process in our session? */
		ಆದರೆ (getsid(getpid()) == getsid(pid))
			ಮುಂದುವರಿ;

		/* Is this a kernel thread? */
		ಆದರೆ (!is_user_process(pid))
			ಮುಂದುವರಿ;

		ಆದರೆ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ಖಾಲಿ);
	kill(-1, SIGCONT);
	ಹಿಂತಿರುಗಿ sendcount;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *arg = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	ಪೂರ್ಣಾಂಕ sig = SIGKILL;
	ಅಕ್ಷರ *ಇಲ್ಲಿ;
	ಅಕ್ಷರ *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
			ಸ್ಥಿತಿ 'd':
				dryrun = true;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'o':
				ಇಲ್ಲಿ = optarg;
				ಆಗ ((token = strsep(&ಇಲ್ಲಿ, ",;:"))) {
					ಆದರೆ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					ಇಲ್ಲದಿದ್ದರೆ {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				ನಿಲ್ಲಿಸು;
			case_RC_COMMON_GETOPT
		}
	}

	ಆದರೆ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	ಆದರೆ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	ಆದರೆ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	ಹಿಂತಿರುಗಿ 0;
}
