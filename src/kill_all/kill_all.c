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


#شامل <dirent.h>
#شامل <errno.h>
#شامل <getopt.h>
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <unistd.h>
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <stdbool.h>

#شامل "einfo.h"
#شامل "rc.h"
#شامل "rc_exec.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = "[signal number]";
ثابت حرف getoptstring[] = "do:" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "dry-run",        0, تہی, 'd' },
	{ "omit",        1, تہی, 'o' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ثابت حرف *usagestring = تہی;

ساکن صحیح mount_proc(خالی)
{
	pid_t rc;
	صحیح حالت_2;
	ثابت حرف *argv[] = { "mount", "-t", "proc", "proc", "/proc", تہی };
	ساخت exec_نتیجہ res;
	ساخت exec_دلائل دلائل = exec_init(argv);

	اگر (access("/proc/version", F_OK) == 0)
		واپس 0;

	res = do_exec(&دلائل);
	اگر (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		واپس -1;
	}
	جب ((rc = wait(&حالت_2)) != res.pid)
		اگر (rc < 0 && errno == ECHILD)
			توڑو;
	اگر (rc != res.pid || !WIFEXITED(حالت_2) || WEXITSTATUS(حالت_2) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	اگر (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		واپس -1;
	}
	واپس 0;
}

ساکن bool is_user_process(pid_t pid)
{
	حرف *buf = تہی;
	FILE *fp;
	حرف *راستہ = تہی;
	pid_t عارضی_pid;
	size_t سائز;
	bool user_process = true;

	جب (pid >0 && user_process) {
		اگر (pid == 2) {
			user_process = false;
			جاری;
		}
		xasprintf(&راستہ, "/proc/%d/status", pid);
		fp = fopen(راستہ, "r");
		آزاد(راستہ);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		اگر (!fp) {
			user_process = false;
			جاری;
		}
		عارضی_pid = -1;
		جب (!feof(fp)) {
			buf = تہی;
			اگر (xgetline(&buf, &سائز, fp) != -1) {
				لڑی_سے_پڑھو(buf, "PPid: %d", &عارضی_pid);
				آزاد(buf);
			} ورنہ {
				آزاد(buf);
				توڑو;
			}
		}
		fclose(fp);
		اگر (عارضی_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			جاری;
		}
		pid = عارضی_pid;
	}
	واپس user_process;
}

ساکن صحیح signal_processes(صحیح sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t اشارات;
	sigset_t oldsigs;
	DIR *dir;
	ساخت dirent	*d;
	حرف *buf = تہی;
	pid_t pid;
	صحیح sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&اشارات);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &اشارات, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	اگر (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, تہی);
		kill(-1, SIGCONT);
		واپس -1;
	}
	dir = opendir(".");
	اگر (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, تہی);
		kill(-1, SIGCONT);
		واپس -1;
	}

	/* Walk through the directory. */
	جب ((d = readdir(dir)) != تہی) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		اگر (pid == 0)
			جاری;

		/* Is this a process we have been requested to omit? */
		اگر (buf) {
			آزاد(buf);
			buf = تہی;
		}
		xasprintf(&buf, "%d", pid);
		اگر (rc_stringlist_ڈھونڈو(omits, buf))
			جاری;

		/* Is this process in our session? */
		اگر (getsid(getpid()) == getsid(pid))
			جاری;

		/* Is this a kernel thread? */
		اگر (!is_user_process(pid))
			جاری;

		اگر (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		ورنہ اگر (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, تہی);
	kill(-1, SIGCONT);
	واپس sendcount;
}

صحیح مین(صحیح argc, حرف **argv)
{
	حرف *arg = تہی;
	صحیح opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	صحیح sig = SIGKILL;
	حرف *یہاں;
	حرف *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
			حالت 'd':
				dryrun = true;
				توڑو;
			حالت 'o':
				یہاں = optarg;
				جب ((token = strsep(&یہاں, ",;:"))) {
					اگر ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					ورنہ {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				توڑو;
			case_RC_COMMON_GETOPT
		}
	}

	اگر (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	اگر (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	اگر (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	واپس 0;
}
