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


#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = "[signal number]";
ਅਚਲ ਅੱਖਰ getoptstring[] = "do:" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "dry-run",        0, ਨਿਰਾ, 'd' },
	{ "omit",        1, ਨਿਰਾ, 'o' },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਸਥਿਰ ਪੂਰਨ mount_proc(ਖਾਲੀ)
{
	pid_t rc;
	ਪੂਰਨ ਸਥਿਤੀ;
	ਅਚਲ ਅੱਖਰ *argv[] = { "mount", "-t", "proc", "proc", "/proc", ਨਿਰਾ };
	ਬਣਤਰ exec_ਨਤੀਜਾ res;
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ = exec_init(argv);

	ਜੇ (access("/proc/version", F_OK) == 0)
		ਵਾਪਸ 0;

	res = do_exec(&ਦਲੀਲਾਂ);
	ਜੇ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		ਵਾਪਸ -1;
	}
	ਜਦੋਂ ((rc = wait(&ਸਥਿਤੀ)) != res.pid)
		ਜੇ (rc < 0 && errno == ECHILD)
			ਰੋਕੋ;
	ਜੇ (rc != res.pid || !WIFEXITED(ਸਥਿਤੀ) || WEXITSTATUS(ਸਥਿਤੀ) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	ਜੇ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		ਵਾਪਸ -1;
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ bool is_user_process(pid_t pid)
{
	ਅੱਖਰ *buf = ਨਿਰਾ;
	FILE *fp;
	ਅੱਖਰ *ਰਸਤਾ = ਨਿਰਾ;
	pid_t ਆਰਜ਼ੀ_pid;
	size_t ਮਾਪ;
	bool user_process = true;

	ਜਦੋਂ (pid >0 && user_process) {
		ਜੇ (pid == 2) {
			user_process = false;
			ਜਾਰੀ;
		}
		xasprintf(&ਰਸਤਾ, "/proc/%d/status", pid);
		fp = fopen(ਰਸਤਾ, "r");
		ਮੁਕਤ(ਰਸਤਾ);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		ਜੇ (!fp) {
			user_process = false;
			ਜਾਰੀ;
		}
		ਆਰਜ਼ੀ_pid = -1;
		ਜਦੋਂ (!feof(fp)) {
			buf = ਨਿਰਾ;
			ਜੇ (xgetline(&buf, &ਮਾਪ, fp) != -1) {
				ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(buf, "PPid: %d", &ਆਰਜ਼ੀ_pid);
				ਮੁਕਤ(buf);
			} ਨਹੀਂਤਾਂ {
				ਮੁਕਤ(buf);
				ਰੋਕੋ;
			}
		}
		fclose(fp);
		ਜੇ (ਆਰਜ਼ੀ_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			ਜਾਰੀ;
		}
		pid = ਆਰਜ਼ੀ_pid;
	}
	ਵਾਪਸ user_process;
}

ਸਥਿਰ ਪੂਰਨ signal_processes(ਪੂਰਨ sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t ਸੰਕੇਤ_2;
	sigset_t oldsigs;
	DIR *dir;
	ਬਣਤਰ dirent	*d;
	ਅੱਖਰ *buf = ਨਿਰਾ;
	pid_t pid;
	ਪੂਰਨ sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&ਸੰਕੇਤ_2);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &ਸੰਕੇਤ_2, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	ਜੇ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ਨਿਰਾ);
		kill(-1, SIGCONT);
		ਵਾਪਸ -1;
	}
	dir = opendir(".");
	ਜੇ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ਨਿਰਾ);
		kill(-1, SIGCONT);
		ਵਾਪਸ -1;
	}

	/* Walk through the directory. */
	ਜਦੋਂ ((d = readdir(dir)) != ਨਿਰਾ) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		ਜੇ (pid == 0)
			ਜਾਰੀ;

		/* Is this a process we have been requested to omit? */
		ਜੇ (buf) {
			ਮੁਕਤ(buf);
			buf = ਨਿਰਾ;
		}
		xasprintf(&buf, "%d", pid);
		ਜੇ (rc_stringlist_ਲੱਭੋ(omits, buf))
			ਜਾਰੀ;

		/* Is this process in our session? */
		ਜੇ (getsid(getpid()) == getsid(pid))
			ਜਾਰੀ;

		/* Is this a kernel thread? */
		ਜੇ (!is_user_process(pid))
			ਜਾਰੀ;

		ਜੇ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		ਨਹੀਂਤਾਂ ਜੇ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ਨਿਰਾ);
	kill(-1, SIGCONT);
	ਵਾਪਸ sendcount;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *arg = ਨਿਰਾ;
	ਪੂਰਨ opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	ਪੂਰਨ sig = SIGKILL;
	ਅੱਖਰ *ਇੱਥੇ;
	ਅੱਖਰ *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
			ਹਾਲਤ 'd':
				dryrun = true;
				ਰੋਕੋ;
			ਹਾਲਤ 'o':
				ਇੱਥੇ = optarg;
				ਜਦੋਂ ((token = strsep(&ਇੱਥੇ, ",;:"))) {
					ਜੇ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					ਨਹੀਂਤਾਂ {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				ਰੋਕੋ;
			case_RC_COMMON_GETOPT
		}
	}

	ਜੇ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	ਜੇ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	ਜੇ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	ਵਾਪਸ 0;
}
