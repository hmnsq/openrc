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

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = "[signal number]";
स्थिरांक अक्षर getoptstring[] = "do:" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "dry-run",        0, रिक्त, 'd' },
	{ "omit",        1, रिक्त, 'o' },
	longopts_COMMON
};
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
स्थिरांक अक्षर *usagestring = रिक्त;

स्थिर पूर्णांक mount_proc(शून्य)
{
	pid_t rc;
	पूर्णांक status;
	स्थिरांक अक्षर *argv[] = { "mount", "-t", "proc", "proc", "/proc", रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);

	जर (access("/proc/version", F_OK) == 0)
		परत 0;

	res = do_exec(&args);
	जर (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		परत -1;
	}
	पर्यंत ((rc = रुको(&status)) != res.pid)
		जर (rc < 0 && त्रुटि_सं == ECHILD)
			थांबा;
	जर (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	जर (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		परत -1;
	}
	परत 0;
}

स्थिर bool is_user_process(pid_t pid)
{
	अक्षर *buf = रिक्त;
	खाता *fp;
	अक्षर *path = रिक्त;
	pid_t temp_pid;
	माप_प्रकार size;
	bool user_process = true;

	पर्यंत (pid >0 && user_process) {
		जर (pid == 2) {
			user_process = false;
			सुरूठेवा;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = ख_खोलो(path, "r");
		मुक्त(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		जर (!fp) {
			user_process = false;
			सुरूठेवा;
		}
		temp_pid = -1;
		पर्यंत (!ख_पूर्ण(fp)) {
			buf = रिक्त;
			जर (xgetline(&buf, &size, fp) != -1) {
				माला_पूछो(buf, "PPid: %d", &temp_pid);
				मुक्त(buf);
			} अन्यथा {
				मुक्त(buf);
				थांबा;
			}
		}
		ख_बंद(fp);
		जर (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			सुरूठेवा;
		}
		pid = temp_pid;
	}
	परत user_process;
}

स्थिर पूर्णांक signal_processes(पूर्णांक sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	सूची *dir;
	संरचना dirent	*d;
	अक्षर *buf = रिक्त;
	pid_t pid;
	पूर्णांक sendcount = 0;

	समाप्त(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	जर (स_बदलो("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
		समाप्त(-1, SIGCONT);
		परत -1;
	}
	dir = सूची_खोलो(".");
	जर (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
		समाप्त(-1, SIGCONT);
		परत -1;
	}

	/* Walk through the directory. */
	पर्यंत ((d = सूची_पढ़ो(dir)) != रिक्त) {
		/* Is this a process? */
		pid = (pid_t) म_से_प(d->d_name);
		जर (pid == 0)
			सुरूठेवा;

		/* Is this a process we have been requested to omit? */
		जर (buf) {
			मुक्त(buf);
			buf = रिक्त;
		}
		xasprintf(&buf, "%d", pid);
		जर (rc_stringlist_find(omits, buf))
			सुरूठेवा;

		/* Is this process in our session? */
		जर (getsid(getpid()) == getsid(pid))
			सुरूठेवा;

		/* Is this a kernel thread? */
		जर (!is_user_process(pid))
			सुरूठेवा;

		जर (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		अन्यथा जर (समाप्त(pid, sig) == 0)
			sendcount++;
	}
	बंद_सूची(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
	समाप्त(-1, SIGCONT);
	परत sendcount;
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *arg = रिक्त;
	पूर्णांक opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	पूर्णांक sig = SIGKILL;
	अक्षर *here;
	अक्षर *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
			स्थिती 'd':
				dryrun = true;
				थांबा;
			स्थिती 'o':
				here = optarg;
				पर्यंत ((token = strsep(&here, ",;:"))) {
					जर ((pid_t) म_से_प(token) > 0)
						rc_stringlist_addu(omits, token);
					अन्यथा {
						eerror("Invalid omit pid value %s", token);
						usage(निकास_त्रुटि);
					}
				}
				थांबा;
			case_RC_COMMON_GETOPT
		}
	}

	जर (argc > optind) {
	arg = argv[optind];
	sig = म_से_प(arg);
	जर (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(निकास_त्रुटि);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	जर (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	परत 0;
}
