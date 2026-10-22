/*
 * openrc-shutdown.c
 * If you are using OpenRC's provided init, this will shut down or
 * reboot your system.
 *
 * This is based on code written by James Hammons <jlhamm@acm.org>, so
 * I would like to publically thank him for his work.
 */

/*
 * Copyright 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <signal.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <syslog.h>
#ضمن <sys/types.h>
#ضمن <strings.h>
#ضمن <unistd.h>
#ضمن <time.h>
#ضمن <utmp.h>

#ضمن "broadcast.h"
#ضمن "einfo.h"
#ضمن "rc.h"
#ضمن "helpers.h"
#ضمن "misc.h"
#ضمن "sysvinit.h"
#ضمن "wtmp.h"
#ضمن "_usage.h"

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "cancel",        no_argument, عدم, 'c'},
	{ "no-write",        no_argument, عدم, 'd'},
	{ "dry-run",        no_argument, عدم, 'D'},
	{ "halt",        no_argument, عدم, 'H'},
	{ "kexec",        no_argument, عدم, 'K'},
	{ "poweroff",        no_argument, عدم, 'p'},
	{ "reexec",        no_argument, عدم, 'R'},
	{ "reboot",        no_argument, عدم, 'r'},
	{ "single",        no_argument, عدم, 's'},
	{ "write-only",        no_argument, عدم, 'w'},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"cancel a pending shutdown",
	"do not write wtmp record",
	"print actions instead of executing them",
	"halt the system",
	"reboot the system using kexec",
	"power off the system",
	"re-execute init (use after upgrading)",
	"reboot the system",
	"single user mode",
	"write wtmp boot record and exit",
	longopts_help_COMMON
};
ثابت حرف *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ثابت حرف *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ثابت حرف *nologin_file = RC_SYSCONFDIR"/nologin";
ثابت حرف *shutdown_pid = "/run/openrc-shutdown.pid";

ساكن bool do_cancel = false;
ساكن bool do_dryrun = false;
ساكن bool do_halt = false;
ساكن bool do_kexec = false;
ساكن bool do_poweroff = false;
ساكن bool do_reboot = false;
ساكن bool do_reexec = false;
ساكن bool do_single = false;
ساكن bool do_wtmp = true;
ساكن bool do_wtmp_only = false;

ساكن فراغ cancel_shutdown(فراغ)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	אם (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	אם (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	אחרת
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ساكن فراغ create_nologin(صحيح mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	אם ((fp = fopen(nologin_file, "w")) != عدم) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ساكن فراغ send_cmd(ثابت حرف *cmd)
{
	FILE *fifo;
	size_t ignored;

	אם (do_dryrun) {
		einfo("Would send %s to init", cmd);
		החזר;
	}
	אם (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	אם (!fifo) {
		perror("fopen");
		החזר;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	אם (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ساكن فراغ sleep_no_interrupt(صحيح seconds)
{
	بنية timespec duration;
	بنية timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	כלעוד (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN ساكن فراغ stop_shutdown(صحيح sig)
{
	(فراغ) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

صحيح main(صحيح argc, حرف **argv)
{
	حرف *ch = عدم;
	صحيح opt;
	صحيح cmd_count = 0;
	صحيح hour = 0;
	صحيح min = 0;
	صحيح shutdown_delay = 0;
	بنية sigaction sa;
	بنية tm *lt;
	time_t tv;
	bool need_warning = false;
	حرف *msg = عدم;
	حرف *state = عدم;
	حرف *time_arg = عدم;
	FILE *fp;

	applet = basename_c(argv[0]);
	כלעוד ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
			حالة 'c':
				do_cancel = true;
			cmd_count++;
				اكسر;
			حالة 'd':
				do_wtmp = false;
				اكسر;
		حالة 'D':
			do_dryrun = true;
			اكسر;
		حالة 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_count++;
			اكسر;
		حالة 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_count++;
			اكسر;
		حالة 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_count++;
			اكسر;
		حالة 'R':
			do_reexec = true;
			cmd_count++;
			اكسر;
		حالة 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_count++;
			اكسر;
		حالة 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_count++;
			اكسر;
		حالة 'w':
			do_wtmp_only = true;
			cmd_count++;
			اكسر;
		case_RC_COMMON_GETOPT
		}
	}
	אם (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	אם (cmd_count != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	אם (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} אחרת אם (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} אחרת אם (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	אם (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	אם (*time_arg == '+')
		time_arg++;
	אם (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	עבור (ch=time_arg; *ch; ch++)
		אם ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	אם (strchr(time_arg, ':')) {
		אם ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		אם (shutdown_delay < 0)
			shutdown_delay += 1440;
	} אחרת {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	אם (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, عدم);
	sigaction(SIGTERM, &sa, عدم);
	כלעוד (shutdown_delay > 0) {
		אם (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		אחרת אם (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		אחרת אם	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		אחרת
			need_warning = true;

		אם (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		אם (need_warning) {
			xasprintf(&msg, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(msg);
			حرر(msg);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&msg, "\rThe system will %s now\r\n", state);
	broadcast(msg);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	אם (do_halt) {
		אם (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} אחרת
			send_cmd("halt");
	} אחרת אם (do_kexec)
		send_cmd("kexec");
	אחרת אם (do_poweroff) {
		אם (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} אחרת
			send_cmd("poweroff");
	} אחרת אם (do_reboot) {
		אם (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		אחרת
			send_cmd("reboot");
	} אחרת אם (do_single) {
		אם (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		אחרת
			send_cmd("single");
	}
	החזר 0;
}
