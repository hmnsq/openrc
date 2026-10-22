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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <getopt.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <syslog.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <strings.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <utmp.h>

#অন্তর্ভাৱ "broadcast.h"
#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "helpers.h"
#অন্তর্ভাৱ "misc.h"
#অন্তর্ভাৱ "sysvinit.h"
#অন্তর্ভাৱ "wtmp.h"
#অন্তর্ভাৱ "_usage.h"

ধ্রুৱ ৱর্ণ *applet = রিক্ত;
ধ্রুৱ ৱর্ণ *extraopts = রিক্ত;
ধ্রুৱ ৱর্ণ getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ধ্রুৱ সংরচনা option longopts[] = {
	{ "cancel",        no_argument, রিক্ত, 'c'},
	{ "no-write",        no_argument, রিক্ত, 'd'},
	{ "dry-run",        no_argument, রিক্ত, 'D'},
	{ "halt",        no_argument, রিক্ত, 'H'},
	{ "kexec",        no_argument, রিক্ত, 'K'},
	{ "poweroff",        no_argument, রিক্ত, 'p'},
	{ "reexec",        no_argument, রিক্ত, 'R'},
	{ "reboot",        no_argument, রিক্ত, 'r'},
	{ "single",        no_argument, রিক্ত, 's'},
	{ "write-only",        no_argument, রিক্ত, 'w'},
	longopts_COMMON
};
ধ্রুৱ ৱর্ণ * ধ্রুৱ longopts_help[] = {
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
ধ্রুৱ ৱর্ণ *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ধ্রুৱ ৱর্ণ *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ধ্রুৱ ৱর্ণ *nologin_file = RC_SYSCONFDIR"/nologin";
ধ্রুৱ ৱর্ণ *shutdown_pid = "/run/openrc-shutdown.pid";

স্থির bool do_cancel = false;
স্থির bool do_dryrun = false;
স্থির bool do_halt = false;
স্থির bool do_kexec = false;
স্থির bool do_poweroff = false;
স্থির bool do_reboot = false;
স্থির bool do_reexec = false;
স্থির bool do_single = false;
স্থির bool do_wtmp = true;
স্থির bool do_wtmp_only = false;

স্থির শূন্য cancel_shutdown(শূন্য)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	যদি (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	যদি (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	অন্যথা
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
স্থির শূন্য create_nologin(পূর্ণাঙ্ক mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	যদি ((fp = fopen(nologin_file, "w")) != রিক্ত) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
স্থির শূন্য send_cmd(ধ্রুৱ ৱর্ণ *cmd)
{
	FILE *fifo;
	size_t ignored;

	যদি (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ঘূৰাই;
	}
	যদি (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	যদি (!fifo) {
		perror("fopen");
		ঘূৰাই;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	যদি (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
স্থির শূন্য sleep_no_interrupt(পূর্ণাঙ্ক seconds)
{
	সংরচনা timespec duration;
	সংরচনা timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	যেতিয়ালৈ (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN স্থির শূন্য stop_shutdown(পূর্ণাঙ্ক sig)
{
	(শূন্য) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ **argv)
{
	ৱর্ণ *ch = রিক্ত;
	পূর্ণাঙ্ক opt;
	পূর্ণাঙ্ক cmd_গুনতি = 0;
	পূর্ণাঙ্ক hour = 0;
	পূর্ণাঙ্ক min = 0;
	পূর্ণাঙ্ক shutdown_delay = 0;
	সংরচনা sigaction sa;
	সংরচনা tm *lt;
	time_t tv;
	bool need_warning = false;
	ৱর্ণ *বাৰ্তা = রিক্ত;
	ৱর্ণ *state = রিক্ত;
	ৱর্ণ *time_arg = রিক্ত;
	FILE *fp;

	applet = basename_c(argv[0]);
	যেতিয়ালৈ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		ৱিকল্প (opt) {
			পক্ষ 'c':
				do_cancel = true;
			cmd_গুনতি++;
				ভঙ্গ;
			পক্ষ 'd':
				do_wtmp = false;
				ভঙ্গ;
		পক্ষ 'D':
			do_dryrun = true;
			ভঙ্গ;
		পক্ষ 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_গুনতি++;
			ভঙ্গ;
		পক্ষ 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_গুনতি++;
			ভঙ্গ;
		পক্ষ 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_গুনতি++;
			ভঙ্গ;
		পক্ষ 'R':
			do_reexec = true;
			cmd_গুনতি++;
			ভঙ্গ;
		পক্ষ 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_গুনতি++;
			ভঙ্গ;
		পক্ষ 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_গুনতি++;
			ভঙ্গ;
		পক্ষ 'w':
			do_wtmp_only = true;
			cmd_গুনতি++;
			ভঙ্গ;
		case_RC_COMMON_GETOPT
		}
	}
	যদি (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	যদি (cmd_গুনতি != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	যদি (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} অন্যথা যদি (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} অন্যথা যদি (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	যদি (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	যদি (*time_arg == '+')
		time_arg++;
	যদি (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	বাবে (ch=time_arg; *ch; ch++)
		যদি ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	যদি (strchr(time_arg, ':')) {
		যদি ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		যদি (shutdown_delay < 0)
			shutdown_delay += 1440;
	} অন্যথা {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	যদি (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, পরিমাণ(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, রিক্ত);
	sigaction(SIGTERM, &sa, রিক্ত);
	যেতিয়ালৈ (shutdown_delay > 0) {
		যদি (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		অন্যথা যদি (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		অন্যথা যদি	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		অন্যথা
			need_warning = true;

		যদি (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		যদি (need_warning) {
			xasprintf(&বাৰ্তা, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(বাৰ্তা);
			মুক্ত(বাৰ্তা);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&বাৰ্তা, "\rThe system will %s now\r\n", state);
	broadcast(বাৰ্তা);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	যদি (do_halt) {
		যদি (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} অন্যথা
			send_cmd("halt");
	} অন্যথা যদি (do_kexec)
		send_cmd("kexec");
	অন্যথা যদি (do_poweroff) {
		যদি (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} অন্যথা
			send_cmd("poweroff");
	} অন্যথা যদি (do_reboot) {
		যদি (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		অন্যথা
			send_cmd("reboot");
	} অন্যথা যদি (do_single) {
		যদি (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		অন্যথা
			send_cmd("single");
	}
	ঘূৰাই 0;
}
