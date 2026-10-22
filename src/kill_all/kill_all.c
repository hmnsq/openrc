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


#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <stdbool.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = "[signal number]";
സ്ഥിരം അക്ഷരം getoptstring[] = "do:" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "dry-run",        0, ശൂന്യ, 'd' },
	{ "omit",        1, ശൂന്യ, 'o' },
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
സ്ഥിരം അക്ഷരം *usagestring = ശൂന്യ;

സ്ഥിതം പൂർണ്ണസംഖ്യ mount_proc(ശൂന്യം)
{
	pid_t rc;
	പൂർണ്ണസംഖ്യ status;
	സ്ഥിരം അക്ഷരം *argv[] = { "mount", "-t", "proc", "proc", "/proc", ശൂന്യ };
	ഘടന exec_result res;
	ഘടന exec_args args = exec_init(argv);

	എങ്കിൽ (access("/proc/version", F_OK) == 0)
		തിരികെ 0;

	res = do_exec(&args);
	എങ്കിൽ (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		തിരികെ -1;
	}
	ആകുമ്പോൾ ((rc = wait(&status)) != res.pid)
		എങ്കിൽ (rc < 0 && errno == ECHILD)
			നിർത്തുക;
	എങ്കിൽ (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	എങ്കിൽ (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		തിരികെ -1;
	}
	തിരികെ 0;
}

സ്ഥിതം bool is_user_process(pid_t pid)
{
	അക്ഷരം *buf = ശൂന്യ;
	FILE *fp;
	അക്ഷരം *path = ശൂന്യ;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	ആകുമ്പോൾ (pid >0 && user_process) {
		എങ്കിൽ (pid == 2) {
			user_process = false;
			തുടരുക;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		സ്വതന്ത്രം(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		എങ്കിൽ (!fp) {
			user_process = false;
			തുടരുക;
		}
		temp_pid = -1;
		ആകുമ്പോൾ (!feof(fp)) {
			buf = ശൂന്യ;
			എങ്കിൽ (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				സ്വതന്ത്രം(buf);
			} അല്ലെങ്കിൽ {
				സ്വതന്ത്രം(buf);
				നിർത്തുക;
			}
		}
		fclose(fp);
		എങ്കിൽ (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			തുടരുക;
		}
		pid = temp_pid;
	}
	തിരികെ user_process;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ signal_processes(പൂർണ്ണസംഖ്യ sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	ഘടന dirent	*d;
	അക്ഷരം *buf = ശൂന്യ;
	pid_t pid;
	പൂർണ്ണസംഖ്യ sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	എങ്കിൽ (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ശൂന്യ);
		kill(-1, SIGCONT);
		തിരികെ -1;
	}
	dir = opendir(".");
	എങ്കിൽ (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ശൂന്യ);
		kill(-1, SIGCONT);
		തിരികെ -1;
	}

	/* Walk through the directory. */
	ആകുമ്പോൾ ((d = readdir(dir)) != ശൂന്യ) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		എങ്കിൽ (pid == 0)
			തുടരുക;

		/* Is this a process we have been requested to omit? */
		എങ്കിൽ (buf) {
			സ്വതന്ത്രം(buf);
			buf = ശൂന്യ;
		}
		xasprintf(&buf, "%d", pid);
		എങ്കിൽ (rc_stringlist_find(omits, buf))
			തുടരുക;

		/* Is this process in our session? */
		എങ്കിൽ (getsid(getpid()) == getsid(pid))
			തുടരുക;

		/* Is this a kernel thread? */
		എങ്കിൽ (!is_user_process(pid))
			തുടരുക;

		എങ്കിൽ (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		അല്ലെങ്കിൽ എങ്കിൽ (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ശൂന്യ);
	kill(-1, SIGCONT);
	തിരികെ sendcount;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	അക്ഷരം *arg = ശൂന്യ;
	പൂർണ്ണസംഖ്യ opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	പൂർണ്ണസംഖ്യ sig = SIGKILL;
	അക്ഷരം *here;
	അക്ഷരം *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
			അവസ്ഥ 'd':
				dryrun = true;
				നിർത്തുക;
			അവസ്ഥ 'o':
				here = optarg;
				ആകുമ്പോൾ ((token = strsep(&here, ",;:"))) {
					എങ്കിൽ ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					അല്ലെങ്കിൽ {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				നിർത്തുക;
			case_RC_COMMON_GETOPT
		}
	}

	എങ്കിൽ (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	എങ്കിൽ (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	എങ്കിൽ (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	തിരികെ 0;
}
