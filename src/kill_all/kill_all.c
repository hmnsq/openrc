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


#अन्तर्भाव <dirent.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <stdbool.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = "[signal number]";
ध्रुव वर्ण getoptstring[] = "do:" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "dry-run",        0, रिक्त, 'd' },
	{ "omit",        1, रिक्त, 'o' },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ध्रुव वर्ण *usagestring = रिक्त;

स्थिर पूर्णाङ्क mount_proc(शून्य)
{
	pid_t rc;
	पूर्णाङ्क status;
	ध्रुव वर्ण *argv[] = { "mount", "-t", "proc", "proc", "/proc", रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);

	जइ (access("/proc/version", F_OK) == 0)
		प्रत्यागम 0;

	res = do_exec(&args);
	जइ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		प्रत्यागम -1;
	}
	जाव ((rc = wait(&status)) != res.pid)
		जइ (rc < 0 && errno == ECHILD)
			भङ्ग;
	जइ (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	जइ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		प्रत्यागम -1;
	}
	प्रत्यागम 0;
}

स्थिर bool is_user_process(pid_t pid)
{
	वर्ण *buf = रिक्त;
	FILE *fp;
	वर्ण *path = रिक्त;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	जाव (pid >0 && user_process) {
		जइ (pid == 2) {
			user_process = false;
			अनुवर्त;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		मुक्त(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		जइ (!fp) {
			user_process = false;
			अनुवर्त;
		}
		temp_pid = -1;
		जाव (!feof(fp)) {
			buf = रिक्त;
			जइ (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				मुक्त(buf);
			} अण्णहा {
				मुक्त(buf);
				भङ्ग;
			}
		}
		fclose(fp);
		जइ (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			अनुवर्त;
		}
		pid = temp_pid;
	}
	प्रत्यागम user_process;
}

स्थिर पूर्णाङ्क signal_processes(पूर्णाङ्क sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	संरचना dirent	*d;
	वर्ण *buf = रिक्त;
	pid_t pid;
	पूर्णाङ्क sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	जइ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
		kill(-1, SIGCONT);
		प्रत्यागम -1;
	}
	dir = opendir(".");
	जइ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
		kill(-1, SIGCONT);
		प्रत्यागम -1;
	}

	/* Walk through the directory. */
	जाव ((d = readdir(dir)) != रिक्त) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		जइ (pid == 0)
			अनुवर्त;

		/* Is this a process we have been requested to omit? */
		जइ (buf) {
			मुक्त(buf);
			buf = रिक्त;
		}
		xasprintf(&buf, "%d", pid);
		जइ (rc_stringlist_find(omits, buf))
			अनुवर्त;

		/* Is this process in our session? */
		जइ (getsid(getpid()) == getsid(pid))
			अनुवर्त;

		/* Is this a kernel thread? */
		जइ (!is_user_process(pid))
			अनुवर्त;

		जइ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		अण्णहा जइ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
	kill(-1, SIGCONT);
	प्रत्यागम sendcount;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *arg = रिक्त;
	पूर्णाङ्क opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	पूर्णाङ्क sig = SIGKILL;
	वर्ण *here;
	वर्ण *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	जाव ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
			पक्ष 'd':
				dryrun = true;
				भङ्ग;
			पक्ष 'o':
				here = optarg;
				जाव ((token = strsep(&here, ",;:"))) {
					जइ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					अण्णहा {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				भङ्ग;
			case_RC_COMMON_GETOPT
		}
	}

	जइ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	जइ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	जइ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	प्रत्यागम 0;
}
