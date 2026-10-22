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


#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <stdbool.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = "[signal number]";
மாறிலி எழுத்து getoptstring[] = "do:" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "dry-run",        0, வெற்று, 'd' },
	{ "omit",        1, வெற்று, 'o' },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = வெற்று;

நிலையான முழு mount_proc(வெறுமை)
{
	pid_t rc;
	முழு status;
	மாறிலி எழுத்து *argv[] = { "mount", "-t", "proc", "proc", "/proc", வெற்று };
	கட்டமைப்பு exec_result res;
	கட்டமைப்பு exec_args args = exec_init(argv);

	எனில் (access("/proc/version", F_OK) == 0)
		திரும்பு 0;

	res = do_exec(&args);
	எனில் (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		திரும்பு -1;
	}
	போது ((rc = wait(&status)) != res.pid)
		எனில் (rc < 0 && errno == ECHILD)
			நிறுத்து;
	எனில் (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	எனில் (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		திரும்பு -1;
	}
	திரும்பு 0;
}

நிலையான bool is_user_process(pid_t pid)
{
	எழுத்து *buf = வெற்று;
	FILE *fp;
	எழுத்து *path = வெற்று;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	போது (pid >0 && user_process) {
		எனில் (pid == 2) {
			user_process = false;
			தொடர்;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		விடுவி(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		எனில் (!fp) {
			user_process = false;
			தொடர்;
		}
		temp_pid = -1;
		போது (!feof(fp)) {
			buf = வெற்று;
			எனில் (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				விடுவி(buf);
			} இல்லையெனில் {
				விடுவி(buf);
				நிறுத்து;
			}
		}
		fclose(fp);
		எனில் (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			தொடர்;
		}
		pid = temp_pid;
	}
	திரும்பு user_process;
}

நிலையான முழு signal_processes(முழு sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	கட்டமைப்பு dirent	*d;
	எழுத்து *buf = வெற்று;
	pid_t pid;
	முழு sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	எனில் (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, வெற்று);
		kill(-1, SIGCONT);
		திரும்பு -1;
	}
	dir = opendir(".");
	எனில் (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, வெற்று);
		kill(-1, SIGCONT);
		திரும்பு -1;
	}

	/* Walk through the directory. */
	போது ((d = readdir(dir)) != வெற்று) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		எனில் (pid == 0)
			தொடர்;

		/* Is this a process we have been requested to omit? */
		எனில் (buf) {
			விடுவி(buf);
			buf = வெற்று;
		}
		xasprintf(&buf, "%d", pid);
		எனில் (rc_stringlist_find(omits, buf))
			தொடர்;

		/* Is this process in our session? */
		எனில் (getsid(getpid()) == getsid(pid))
			தொடர்;

		/* Is this a kernel thread? */
		எனில் (!is_user_process(pid))
			தொடர்;

		எனில் (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		இல்லையெனில் எனில் (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, வெற்று);
	kill(-1, SIGCONT);
	திரும்பு sendcount;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	எழுத்து *arg = வெற்று;
	முழு opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	முழு sig = SIGKILL;
	எழுத்து *here;
	எழுத்து *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
			நிலை 'd':
				dryrun = true;
				நிறுத்து;
			நிலை 'o':
				here = optarg;
				போது ((token = strsep(&here, ",;:"))) {
					எனில் ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					இல்லையெனில் {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				நிறுத்து;
			case_RC_COMMON_GETOPT
		}
	}

	எனில் (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	எனில் (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	எனில் (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	திரும்பு 0;
}
