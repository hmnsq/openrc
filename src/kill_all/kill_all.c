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


#समावेश <dirent.h>
#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <signal.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <syslog.h>
#समावेश <unistd.h>
#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <stdbool.h>

#समावेश "einfo.h"
#समावेश "rc.h"
#समावेश "rc_exec.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = "[signal number]";
स्थिर अक्षर getoptstring[] = "do:" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "dry-run",        0, शून्य, 'd' },
	{ "omit",        1, शून्य, 'o' },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = शून्य;

अटल पूर्णांक mount_proc(व्योम)
{
	pid_t rc;
	पूर्णांक status;
	स्थिर अक्षर *argv[] = { "mount", "-t", "proc", "proc", "/proc", शून्य };
	काष्ठा exec_परिणाम res;
	काष्ठा exec_args args = exec_init(argv);

	अगर (access("/proc/version", F_OK) == 0)
		वापस 0;

	res = do_exec(&args);
	अगर (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		वापस -1;
	}
	जबतक ((rc = रुको(&status)) != res.pid)
		अगर (rc < 0 && त्रुटि_सं == ECHILD)
			अवरोध;
	अगर (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	अगर (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		वापस -1;
	}
	वापस 0;
}

अटल bool is_user_process(pid_t pid)
{
	अक्षर *buf = शून्य;
	खाता *fp;
	अक्षर *path = शून्य;
	pid_t अस्थायी_pid;
	माप_प्रकार आकार;
	bool user_process = true;

	जबतक (pid >0 && user_process) {
		अगर (pid == 2) {
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
		अगर (!fp) {
			user_process = false;
			जारी;
		}
		अस्थायी_pid = -1;
		जबतक (!ख_पूर्ण(fp)) {
			buf = शून्य;
			अगर (xgetline(&buf, &आकार, fp) != -1) {
				माला_पूछो(buf, "PPid: %d", &अस्थायी_pid);
				मुक्त(buf);
			} अन्यथा {
				मुक्त(buf);
				अवरोध;
			}
		}
		ख_बंद(fp);
		अगर (अस्थायी_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			जारी;
		}
		pid = अस्थायी_pid;
	}
	वापस user_process;
}

अटल पूर्णांक signal_processes(पूर्णांक sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	सूची *dir;
	काष्ठा dirent	*d;
	अक्षर *buf = शून्य;
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
	अगर (स_बदलो("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, शून्य);
		समाप्त(-1, SIGCONT);
		वापस -1;
	}
	dir = सूची_खोलो(".");
	अगर (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, शून्य);
		समाप्त(-1, SIGCONT);
		वापस -1;
	}

	/* Walk through the directory. */
	जबतक ((d = सूची_पढ़ो(dir)) != शून्य) {
		/* Is this a process? */
		pid = (pid_t) म_से_प(d->d_name);
		अगर (pid == 0)
			जारी;

		/* Is this a process we have been requested to omit? */
		अगर (buf) {
			मुक्त(buf);
			buf = शून्य;
		}
		xasprintf(&buf, "%d", pid);
		अगर (rc_stringlist_find(omits, buf))
			जारी;

		/* Is this process in our session? */
		अगर (getsid(getpid()) == getsid(pid))
			जारी;

		/* Is this a kernel thread? */
		अगर (!is_user_process(pid))
			जारी;

		अगर (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		अन्यथा अगर (समाप्त(pid, sig) == 0)
			sendcount++;
	}
	बंद_सूची(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, शून्य);
	समाप्त(-1, SIGCONT);
	वापस sendcount;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *arg = शून्य;
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
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
			हाल 'd':
				dryrun = true;
				अवरोध;
			हाल 'o':
				here = optarg;
				जबतक ((token = strsep(&here, ",;:"))) {
					अगर ((pid_t) म_से_प(token) > 0)
						rc_stringlist_addu(omits, token);
					अन्यथा {
						eerror("Invalid omit pid value %s", token);
						usage(निकास_त्रुटि);
					}
				}
				अवरोध;
			case_RC_COMMON_GETOPT
		}
	}

	अगर (argc > optind) {
	arg = argv[optind];
	sig = म_से_प(arg);
	अगर (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(निकास_त्रुटि);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	अगर (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	वापस 0;
}
