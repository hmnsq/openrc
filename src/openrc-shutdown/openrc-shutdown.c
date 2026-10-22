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

#شامل <errno.h>
#شامل <getopt.h>
#شامل <signal.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <sys/types.h>
#شامل <strings.h>
#شامل <unistd.h>
#شامل <time.h>
#شامل <utmp.h>

#شامل "broadcast.h"
#شامل "einfo.h"
#شامل "rc.h"
#شامل "helpers.h"
#شامل "misc.h"
#شامل "sysvinit.h"
#شامل "wtmp.h"
#شامل "_usage.h"

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "cancel",        no_argument, پوچ, 'c'},
	{ "no-write",        no_argument, پوچ, 'd'},
	{ "dry-run",        no_argument, پوچ, 'D'},
	{ "halt",        no_argument, پوچ, 'H'},
	{ "kexec",        no_argument, پوچ, 'K'},
	{ "poweroff",        no_argument, پوچ, 'p'},
	{ "reexec",        no_argument, پوچ, 'R'},
	{ "reboot",        no_argument, پوچ, 'r'},
	{ "single",        no_argument, پوچ, 's'},
	{ "write-only",        no_argument, پوچ, 'w'},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ثابت نویسه *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ثابت نویسه *nologin_file = RC_SYSCONFDIR"/nologin";
ثابت نویسه *shutdown_pid = "/run/openrc-shutdown.pid";

ایستا bool do_cancel = false;
ایستا bool do_dryrun = false;
ایستا bool do_halt = false;
ایستا bool do_kexec = false;
ایستا bool do_poweroff = false;
ایستا bool do_reboot = false;
ایستا bool do_reexec = false;
ایستا bool do_single = false;
ایستا bool do_wtmp = true;
ایستا bool do_wtmp_only = false;

ایستا تهی cancel_shutdown(تهی)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	اگر (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	اگر (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	وگرنه
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ایستا تهی create_nologin(صحیح mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	اگر ((fp = fopen(nologin_file, "w")) != پوچ) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ایستا تهی send_cmd(ثابت نویسه *cmd)
{
	FILE *fifo;
	size_t ignored;

	اگر (do_dryrun) {
		einfo("Would send %s to init", cmd);
		بازگشت;
	}
	اگر (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	اگر (!fifo) {
		perror("fopen");
		بازگشت;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	اگر (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ایستا تهی sleep_no_interrupt(صحیح seconds)
{
	ساختار timespec duration;
	ساختار timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	وقتی (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN ایستا تهی stop_shutdown(صحیح sig)
{
	(تهی) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	نویسه *ch = پوچ;
	صحیح opt;
	صحیح cmd_شمار = 0;
	صحیح hour = 0;
	صحیح min = 0;
	صحیح shutdown_delay = 0;
	ساختار sigaction sa;
	ساختار tm *lt;
	time_t tv;
	bool need_warning = false;
	نویسه *پیام = پوچ;
	نویسه *state = پوچ;
	نویسه *time_arg = پوچ;
	FILE *fp;

	applet = basename_c(argv[0]);
	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
			حالت 'c':
				do_cancel = true;
			cmd_شمار++;
				بشکن;
			حالت 'd':
				do_wtmp = false;
				بشکن;
		حالت 'D':
			do_dryrun = true;
			بشکن;
		حالت 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_شمار++;
			بشکن;
		حالت 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_شمار++;
			بشکن;
		حالت 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_شمار++;
			بشکن;
		حالت 'R':
			do_reexec = true;
			cmd_شمار++;
			بشکن;
		حالت 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_شمار++;
			بشکن;
		حالت 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_شمار++;
			بشکن;
		حالت 'w':
			do_wtmp_only = true;
			cmd_شمار++;
			بشکن;
		case_RC_COMMON_GETOPT
		}
	}
	اگر (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	اگر (cmd_شمار != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	اگر (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} وگرنه اگر (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} وگرنه اگر (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	اگر (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	اگر (*time_arg == '+')
		time_arg++;
	اگر (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	برای (ch=time_arg; *ch; ch++)
		اگر ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	اگر (strchr(time_arg, ':')) {
		اگر ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		اگر (shutdown_delay < 0)
			shutdown_delay += 1440;
	} وگرنه {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	اگر (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, اندازه(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, پوچ);
	sigaction(SIGTERM, &sa, پوچ);
	وقتی (shutdown_delay > 0) {
		اگر (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		وگرنه اگر (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		وگرنه اگر	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		وگرنه
			need_warning = true;

		اگر (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		اگر (need_warning) {
			xasprintf(&پیام, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(پیام);
			آزاد(پیام);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&پیام, "\rThe system will %s now\r\n", state);
	broadcast(پیام);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	اگر (do_halt) {
		اگر (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} وگرنه
			send_cmd("halt");
	} وگرنه اگر (do_kexec)
		send_cmd("kexec");
	وگرنه اگر (do_poweroff) {
		اگر (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} وگرنه
			send_cmd("poweroff");
	} وگرنه اگر (do_reboot) {
		اگر (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		وگرنه
			send_cmd("reboot");
	} وگرنه اگر (do_single) {
		اگر (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		وگرنه
			send_cmd("single");
	}
	بازگشت 0;
}
