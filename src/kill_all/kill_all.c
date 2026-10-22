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


#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <stdbool.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = "[signal number]";
અચલ અક્ષર getoptstring[] = "do:" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "dry-run",        0, ખાલી, 'd' },
	{ "omit",        1, ખાલી, 'o' },
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
	"print what would be done",
	"omit this pid (can be repeated)",
	longopts_help_COMMON
};
અચલ અક્ષર *usagestring = ખાલી;

સ્થિર પૂર્ણાંક mount_proc(શૂન્ય)
{
	pid_t rc;
	પૂર્ણાંક status;
	અચલ અક્ષર *argv[] = { "mount", "-t", "proc", "proc", "/proc", ખાલી };
	સંરચના exec_result res;
	સંરચના exec_args args = exec_init(argv);

	જો (access("/proc/version", F_OK) == 0)
		પાછા 0;

	res = do_exec(&args);
	જો (res.pid < 0) {
		syslog(LOG_ERR, "Failed to execute mount");
		પાછા -1;
	}
	જ્યારે ((rc = wait(&status)) != res.pid)
		જો (rc < 0 && errno == ECHILD)
			તોડો;
	જો (rc != res.pid || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
		syslog(LOG_ERR, "mount returned non-zero exit status");

	જો (access("/proc/version", F_OK) != 0) {
		syslog(LOG_ERR, "Could not mount /proc");
		પાછા -1;
	}
	પાછા 0;
}

સ્થિર bool is_user_process(pid_t pid)
{
	અક્ષર *buf = ખાલી;
	FILE *fp;
	અક્ષર *path = ખાલી;
	pid_t temp_pid;
	size_t size;
	bool user_process = true;

	જ્યારે (pid >0 && user_process) {
		જો (pid == 2) {
			user_process = false;
			ચાલુ;
		}
		xasprintf(&path, "/proc/%d/status", pid);
		fp = fopen(path, "r");
		મુક્ત(path);
		/*
		 * if we could not open the file, the process disappeared, which
		 * leaves us no way to determine for sure whether it was a user
		 * process or kernel thread, so we say it is a kernel thread to
		 * avoid accidentally killing it.
		 */
		જો (!fp) {
			user_process = false;
			ચાલુ;
		}
		temp_pid = -1;
		જ્યારે (!feof(fp)) {
			buf = ખાલી;
			જો (xgetline(&buf, &size, fp) != -1) {
				sscanf(buf, "PPid: %d", &temp_pid);
				મુક્ત(buf);
			} નહીંતર {
				મુક્ત(buf);
				તોડો;
			}
		}
		fclose(fp);
		જો (temp_pid == -1) {
			syslog(LOG_ERR, "Unable to read pid from /proc/%d/status", pid);
			user_process = false;
			ચાલુ;
		}
		pid = temp_pid;
	}
	પાછા user_process;
}

સ્થિર પૂર્ણાંક signal_processes(પૂર્ણાંક sig, RC_STRINGLIST *omits, bool dryrun)
{
	sigset_t signals;
	sigset_t oldsigs;
	DIR *dir;
	સંરચના dirent	*d;
	અક્ષર *buf = ખાલી;
	pid_t pid;
	પૂર્ણાંક sendcount = 0;

	kill(-1, SIGSTOP);
	sigfillset(&signals);
	sigemptyset(&oldsigs);
	sigprocmask(SIG_SETMASK, &signals, &oldsigs);
	/*
	 * Open the /proc directory.
	 * CWD must be /proc to avoid problems if / is affected by the killing
	 * (i.e. depends on fuse).
	 */
	જો (chdir("/proc") == -1) {
		syslog(LOG_ERR, "chdir /proc failed");
		sigprocmask(SIG_SETMASK, &oldsigs, ખાલી);
		kill(-1, SIGCONT);
		પાછા -1;
	}
	dir = opendir(".");
	જો (!dir) {
		syslog(LOG_ERR, "cannot opendir(/proc)");
		sigprocmask(SIG_SETMASK, &oldsigs, ખાલી);
		kill(-1, SIGCONT);
		પાછા -1;
	}

	/* Walk through the directory. */
	જ્યારે ((d = readdir(dir)) != ખાલી) {
		/* Is this a process? */
		pid = (pid_t) atoi(d->d_name);
		જો (pid == 0)
			ચાલુ;

		/* Is this a process we have been requested to omit? */
		જો (buf) {
			મુક્ત(buf);
			buf = ખાલી;
		}
		xasprintf(&buf, "%d", pid);
		જો (rc_stringlist_find(omits, buf))
			ચાલુ;

		/* Is this process in our session? */
		જો (getsid(getpid()) == getsid(pid))
			ચાલુ;

		/* Is this a kernel thread? */
		જો (!is_user_process(pid))
			ચાલુ;

		જો (dryrun)
			einfo("Would send signal %d to process %d", sig, pid);
		નહીંતર જો (kill(pid, sig) == 0)
			sendcount++;
	}
	closedir(dir);
	sigprocmask(SIG_SETMASK, &oldsigs, ખાલી);
	kill(-1, SIGCONT);
	પાછા sendcount;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	અક્ષર *arg = ખાલી;
	પૂર્ણાંક opt;
	bool dryrun = false;
	RC_STRINGLIST *omits = rc_stringlist_new();
	પૂર્ણાંક sig = SIGKILL;
	અક્ષર *here;
	અક્ષર *token;

	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	applet = basename_c(argv[0]);
	rc_stringlist_addu(omits, "1");
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
			સ્થિતિ 'd':
				dryrun = true;
				તોડો;
			સ્થિતિ 'o':
				here = optarg;
				જ્યારે ((token = strsep(&here, ",;:"))) {
					જો ((pid_t) atoi(token) > 0)
						rc_stringlist_addu(omits, token);
					નહીંતર {
						eerror("Invalid omit pid value %s", token);
						usage(EXIT_FAILURE);
					}
				}
				તોડો;
			case_RC_COMMON_GETOPT
		}
	}

	જો (argc > optind) {
	arg = argv[optind];
	sig = atoi(arg);
	જો (sig <= 0 || sig > 31) {
		rc_stringlist_free(omits);
		eerror("Invalid signal %s", arg);
		usage(EXIT_FAILURE);
	}
	}

	openlog(applet, LOG_CONS|LOG_PID, LOG_DAEMON);
	જો (mount_proc() != 0) {
		rc_stringlist_free(omits);
		eerrorx("Unable to mount /proc file system");
	}
	signal_processes(sig, omits, dryrun);
	rc_stringlist_free(omits);
	પાછા 0;
}
