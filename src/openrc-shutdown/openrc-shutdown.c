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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <syslog.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <utmp.h>

#অন্তর্ভাব "broadcast.h"
#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "helpers.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "sysvinit.h"
#অন্তর্ভাব "wtmp.h"
#অন্তর্ভাব "_usage.h"

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = ফাঁকা;
ধ্রুবক অক্ষর getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "cancel",        no_argument, ফাঁকা, 'c'},
	{ "no-write",        no_argument, ফাঁকা, 'd'},
	{ "dry-run",        no_argument, ফাঁকা, 'D'},
	{ "halt",        no_argument, ফাঁকা, 'H'},
	{ "kexec",        no_argument, ফাঁকা, 'K'},
	{ "poweroff",        no_argument, ফাঁকা, 'p'},
	{ "reexec",        no_argument, ফাঁকা, 'R'},
	{ "reboot",        no_argument, ফাঁকা, 'r'},
	{ "single",        no_argument, ফাঁকা, 's'},
	{ "write-only",        no_argument, ফাঁকা, 'w'},
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
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
ধ্রুবক অক্ষর *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ধ্রুবক অক্ষর *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ধ্রুবক অক্ষর *nologin_file = RC_SYSCONFDIR"/nologin";
ধ্রুবক অক্ষর *shutdown_pid = "/run/openrc-shutdown.pid";

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
স্থির শূন্য create_nologin(পূর্ণসংখ্যা mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	যদি ((fp = fopen(nologin_file, "w")) != ফাঁকা) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
স্থির শূন্য send_cmd(ধ্রুবক অক্ষর *cmd)
{
	FILE *fifo;
	size_t ignored;

	যদি (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ফেরত;
	}
	যদি (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	যদি (!fifo) {
		perror("fopen");
		ফেরত;
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
স্থির শূন্য sleep_no_interrupt(পূর্ণসংখ্যা seconds)
{
	কাঠামো timespec duration;
	কাঠামো timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	যখন (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN স্থির শূন্য stop_shutdown(পূর্ণসংখ্যা sig)
{
	(শূন্য) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	অক্ষর *ch = ফাঁকা;
	পূর্ণসংখ্যা opt;
	পূর্ণসংখ্যা cmd_গুনতি = 0;
	পূর্ণসংখ্যা hour = 0;
	পূর্ণসংখ্যা min = 0;
	পূর্ণসংখ্যা shutdown_delay = 0;
	কাঠামো sigaction sa;
	কাঠামো tm *lt;
	time_t tv;
	bool need_warning = false;
	অক্ষর *বার্তা = ফাঁকা;
	অক্ষর *state = ফাঁকা;
	অক্ষর *time_arg = ফাঁকা;
	FILE *fp;

	applet = basename_c(argv[0]);
	যখন ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণসংখ্যা *) 0)) != -1)
	{
		নির্বাচন (opt) {
			ক্ষেত্র 'c':
				do_cancel = true;
			cmd_গুনতি++;
				বিরতি;
			ক্ষেত্র 'd':
				do_wtmp = false;
				বিরতি;
		ক্ষেত্র 'D':
			do_dryrun = true;
			বিরতি;
		ক্ষেত্র 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_গুনতি++;
			বিরতি;
		ক্ষেত্র 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_গুনতি++;
			বিরতি;
		ক্ষেত্র 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_গুনতি++;
			বিরতি;
		ক্ষেত্র 'R':
			do_reexec = true;
			cmd_গুনতি++;
			বিরতি;
		ক্ষেত্র 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_গুনতি++;
			বিরতি;
		ক্ষেত্র 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_গুনতি++;
			বিরতি;
		ক্ষেত্র 'w':
			do_wtmp_only = true;
			cmd_গুনতি++;
			বিরতি;
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
	জন্য (ch=time_arg; *ch; ch++)
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
	memset(&sa, 0, আকার(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ফাঁকা);
	sigaction(SIGTERM, &sa, ফাঁকা);
	যখন (shutdown_delay > 0) {
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
			xasprintf(&বার্তা, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(বার্তা);
			মুক্ত(বার্তা);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&বার্তা, "\rThe system will %s now\r\n", state);
	broadcast(বার্তা);
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
	ফেরত 0;
}
