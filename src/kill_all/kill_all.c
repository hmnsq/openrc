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


#అన్తర్భావ <dirent.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <stdbool.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = "[signal number]";
స్థిరరాశి అక్షరం getoptstring[] = "do:" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "dry-run",        0, ఖాళీ, 'd' },
	{ "omit",        1, ఖాళీ, 'o' },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

స్థిర పూర్ణాంకం mount_proc(శూన్య)
{
	pid_t rc;
	పూర్ణాంకం స్థితి_2;
	స్థిరరాశి అక్షరం *argv[] = { "mount", "-t", "proc", "proc", "/proc", ఖాళీ };
	నిర్మాణం exec_ఫలితం res;
	నిర్మాణం exec_వాదనలు వాదనలు = exec_init(argv);

	ఒకవేళ (access("/proc/version", F_OK) == 0)
		తిరిగి 0;

	res = do_exec(&వాదనలు);
	ఒకవేళ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		తిరిగి -1;
	}
	అయితే ((rc = wait(&స్థితి_2)) != res.pid)
		ఒకవేళ (rc < 0 && errno == ECHILD)
			ఆపు;
	ఒకవేళ (rc != res.pid || !WIFEXITED(స్థితి_2) || WEXITSTATUS(స్థితి_2) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	ఒకవేళ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		తిరిగి -1;
	}
	తిరిగి 0;
}

స్థిర bool is_user_process(pid_t pid)
{
	అక్షరం *buf = ఖాళీ;
	FILE *fp;
	అక్షరం *మార్గం = ఖాళీ;
	pid_t తాత్కాలిక_pid;
	size_t కొలత;
	bool user_process = true;

	అయితే (pid >0 && user_process) {
		ఒకవేళ (pid == 2) {
			user_process = false;
			కొనసాగు;
		}
		xasprintf(&మార్గం, "/proc/%d/status", pid);
		fp = fopen(మార్గం, "r");
		విడుదల(మార్గం);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		ఒకవేళ (!fp) {
			user_process = false;
			కొనసాగు;
		}
		తాత్కాలిక_pid = -1;
		అయితే (!feof(fp)) {
			buf = ఖాళీ;
			ఒకవేళ (xgetline(&buf, &కొలత, fp) != -1) {
				అక్షరశ్రేణి_చదువు(buf, "PPid: %d", &తాత్కాలిక_pid);
				విడుదల(buf);
			} లేకపోతే {
				విడుదల(buf);
				ఆపు;
			}
		}
		fclose(fp);
		ఒకవేళ (తాత్కాలిక_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			కొనసాగు;
		}
		pid = తాత్కాలిక_pid;
	}
	తిరిగి user_process;
}

స్థిర పూర్ణాంకం signal_processes(పూర్ణాంకం sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t సంకేతాలు;
	sigset_t oldsigs;
	DIR *dir;
	నిర్మాణం dirent	*d;
	అక్షరం *buf = ఖాళీ;
	pid_t pid;
	పూర్ణాంకం sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&సంకేతాలు);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &సంకేతాలు, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	ఒకవేళ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ఖాళీ);
		kill(-1, SIGCONT);
		తిరిగి -1;
	}
	dir = opendir(".");
	ఒకవేళ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ఖాళీ);
		kill(-1, SIGCONT);
		తిరిగి -1;
	}

	/* Walk through the directory. */
	అయితే ((d = readdir(dir)) != ఖాళీ) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		ఒకవేళ (pid == 0)
			కొనసాగు;

		/* Is this a process we have been requested to omit? */
		ఒకవేళ (buf) {
			విడుదల(buf);
			buf = ఖాళీ;
		}
		xasprintf(&buf, "%d", pid);
		ఒకవేళ (rc_stringlist_కనుగొను(omits, buf))
			కొనసాగు;

		/* Is this process in our session? */
		ఒకవేళ (getsid(getpid()) == getsid(pid))
			కొనసాగు;

		/* Is this a kernel thread? */
		ఒకవేళ (!is_user_process(pid))
			కొనసాగు;

		ఒకవేళ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		లేకపోతే ఒకవేళ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ఖాళీ);
	kill(-1, SIGCONT);
	తిరిగి sendcount;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	అక్షరం *arg = ఖాళీ;
	పూర్ణాంకం opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	పూర్ణాంకం sig = SIGKILL;
	అక్షరం *ఇక్కడ;
	అక్షరం *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
			స్థితి 'd':
				dryrun = true;
				ఆపు;
			స్థితి 'o':
				ఇక్కడ = optarg;
				అయితే ((token = strsep(&ఇక్కడ, ",;:"))) {
					ఒకవేళ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					లేకపోతే {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				ఆపు;
			case_RC_COMMON_GETOPT
		}
	}

	ఒకవేళ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	ఒకవేళ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	ఒకవేళ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	తిరిగి 0;
}
