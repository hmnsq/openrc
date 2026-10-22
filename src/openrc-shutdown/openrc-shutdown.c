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
	إذا (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	إذا (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	وإلا
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

	إذا ((fp = fopen(nologin_file, "w")) != عدم) {
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

	إذا (do_dryrun) {
		einfo("Would send %s to init", cmd);
		أرجع;
	}
	إذا (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	إذا (!fifo) {
		perror("fopen");
		أرجع;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	إذا (ignored != strlen(cmd))
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

	طالما (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
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
	صحيح cmd_كدي = 0;
	صحيح hour = 0;
	صحيح min = 0;
	صحيح shutdown_delay = 0;
	بنية sigaction sa;
	بنية tm *lt;
	time_t tv;
	bool need_warning = false;
	حرف *سكو = عدم;
	حرف *state = عدم;
	حرف *time_arg = عدم;
	FILE *fp;

	applet = basename_c(argv[0]);
	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
			حالة 'c':
				do_cancel = true;
			cmd_كدي++;
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
			cmd_كدي++;
			اكسر;
		حالة 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_كدي++;
			اكسر;
		حالة 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_كدي++;
			اكسر;
		حالة 'R':
			do_reexec = true;
			cmd_كدي++;
			اكسر;
		حالة 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_كدي++;
			اكسر;
		حالة 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_كدي++;
			اكسر;
		حالة 'w':
			do_wtmp_only = true;
			cmd_كدي++;
			اكسر;
		case_RC_COMMON_GETOPT
		}
	}
	إذا (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	إذا (cmd_كدي != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	إذا (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} وإلا إذا (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} وإلا إذا (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	إذا (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	إذا (*time_arg == '+')
		time_arg++;
	إذا (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	لكل (ch=time_arg; *ch; ch++)
		إذا ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	إذا (strchr(time_arg, ':')) {
		إذا ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		إذا (shutdown_delay < 0)
			shutdown_delay += 1440;
	} وإلا {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	إذا (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, عدم);
	sigaction(SIGTERM, &sa, عدم);
	طالما (shutdown_delay > 0) {
		إذا (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		وإلا إذا (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		وإلا إذا	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		وإلا
			need_warning = true;

		إذا (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		إذا (need_warning) {
			xasprintf(&سكو, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(سكو);
			حرر(سكو);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&سكو, "\rThe system will %s now\r\n", state);
	broadcast(سكو);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	إذا (do_halt) {
		إذا (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} وإلا
			send_cmd("halt");
	} وإلا إذا (do_kexec)
		send_cmd("kexec");
	وإلا إذا (do_poweroff) {
		إذا (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} وإلا
			send_cmd("poweroff");
	} وإلا إذا (do_reboot) {
		إذا (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		وإلا
			send_cmd("reboot");
	} وإلا إذا (do_single) {
		إذا (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		وإلا
			send_cmd("single");
	}
	أرجع 0;
}
