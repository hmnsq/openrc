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

अचर वर्ण *applet = रिक्त;
अचर वर्ण *extraopts = "[signal number]";
अचर वर्ण getoptstring[] = "do:" getoptstring_COMMON;
अचर संरचना option longopts[] = {
	{ "dry-run",        0, रिक्त, 'd' },
	{ "omit",        1, रिक्त, 'o' },
	longopts_COMMON
};
अचर वर्ण * अचर longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
अचर वर्ण *usagestring = रिक्त;

स्थिर पूर्णाङ्क mount_proc(शून्य)
{
	pid_t rc;
	पूर्णाङ्क status;
	अचर वर्ण *argv[] = { "mount", "-t", "proc", "proc", "/proc", रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);

	यदि (access("/proc/version", F_OK) == 0)
		फर्काऊ 0;

	res = do_exec(&args);
	यदि (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		फर्काऊ -1;
	}
	जबसम्म ((rc = रुको(&status)) != res.pid)
		यदि (rc < 0 && त्रुटि_सं == ECHILD)
			रोक;
	यदि (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	यदि (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		फर्काऊ -1;
	}
	फर्काऊ 0;
}

स्थिर bool is_user_process(pid_t pid)
{
	वर्ण *buf = रिक्त;
	खाता *fp;
	वर्ण *path = रिक्त;
	pid_t temp_pid;
	माप_प्रकार size;
	bool user_process = true;

	जबसम्म (pid >0 && user_process) {
		यदि (pid == 2) {
			user_process = false;
			जारी;
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
		यदि (!fp) {
			user_process = false;
			जारी;
		}
		temp_pid = -1;
		जबसम्म (!ख_पूर्ण(fp)) {
			buf = रिक्त;
			यदि (xgetline(&buf, &size, fp) != -1) {
				माला_पूछो(buf, "PPid: %d", &temp_pid);
				मुक्त(buf);
			} अन्यथा {
				मुक्त(buf);
				रोक;
			}
		}
		ख_बंद(fp);
		यदि (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			जारी;
		}
		pid = temp_pid;
	}
	फर्काऊ user_process;
}

स्थिर पूर्णाङ्क signal_processes(पूर्णाङ्क sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	सूची *dir;
	संरचना dirent	*d;
	वर्ण *buf = रिक्त;
	pid_t pid;
	पूर्णाङ्क sendcount = 0;

	समाप्त(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	यदि (स_बदलो("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
		समाप्त(-1, SIGCONT);
		फर्काऊ -1;
	}
	dir = सूची_खोलो(".");
	यदि (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
		समाप्त(-1, SIGCONT);
		फर्काऊ -1;
	}

	/* Walk through the directory. */
	जबसम्म ((d = सूची_पढ़ो(dir)) != रिक्त) {
		/* Is this a process? */
		pid = (pid_t) म_से_प(d->d_name);
		यदि (pid == 0)
			जारी;

		/* Is this a process we have been requested to omit? */
		यदि (buf) {
			मुक्त(buf);
			buf = रिक्त;
		}
		xasprintf(&buf, "%d", pid);
		यदि (rc_stringlist_find(omits, buf))
			जारी;

		/* Is this process in our session? */
		यदि (getsid(getpid()) == getsid(pid))
			जारी;

		/* Is this a kernel thread? */
		यदि (!is_user_process(pid))
			जारी;

		यदि (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		अन्यथा यदि (समाप्त(pid, sig) == 0)
			sendcount++;
	}
	बंद_सूची(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, रिक्त);
	समाप्त(-1, SIGCONT);
	फर्काऊ sendcount;
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
	जबसम्म ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
			पक्ष 'd':
				dryrun = true;
				रोक;
			पक्ष 'o':
				here = optarg;
				जबसम्म ((token = strsep(&here, ",;:"))) {
					यदि ((pid_t) म_से_प(token) > 0)
						rc_stringlist_addu(omits, token);
					अन्यथा {
						eerror("Invalid omit pid value %s", token);
						usage(निकास_त्रुटि);
					}
				}
				रोक;
			case_RC_COMMON_GETOPT
		}
	}

	यदि (argc > optind) {
	arg = argv[optind];
	sig = म_से_प(arg);
	यदि (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(निकास_त्रुटि);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	यदि (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	फर्काऊ 0;
}
