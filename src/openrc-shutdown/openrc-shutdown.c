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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <syslog.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <utmp.h>

#அந்தர்பாவ "broadcast.h"
#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "sysvinit.h"
#அந்தர்பாவ "wtmp.h"
#அந்தர்பாவ "_usage.h"

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "cancel",        no_argument, வெற்று, 'c'},
	{ "no-write",        no_argument, வெற்று, 'd'},
	{ "dry-run",        no_argument, வெற்று, 'D'},
	{ "halt",        no_argument, வெற்று, 'H'},
	{ "kexec",        no_argument, வெற்று, 'K'},
	{ "poweroff",        no_argument, வெற்று, 'p'},
	{ "reexec",        no_argument, வெற்று, 'R'},
	{ "reboot",        no_argument, வெற்று, 'r'},
	{ "single",        no_argument, வெற்று, 's'},
	{ "write-only",        no_argument, வெற்று, 'w'},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
மாறிலி எழுத்து *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
மாறிலி எழுத்து *nologin_file = RC_SYSCONFDIR"/nologin";
மாறிலி எழுத்து *shutdown_pid = "/run/openrc-shutdown.pid";

நிலையான bool do_cancel = false;
நிலையான bool do_dryrun = false;
நிலையான bool do_halt = false;
நிலையான bool do_kexec = false;
நிலையான bool do_poweroff = false;
நிலையான bool do_reboot = false;
நிலையான bool do_reexec = false;
நிலையான bool do_single = false;
நிலையான bool do_wtmp = true;
நிலையான bool do_wtmp_only = false;

நிலையான வெறுமை cancel_shutdown(வெறுமை)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	எனில் (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	எனில் (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	இல்லையெனில்
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
நிலையான வெறுமை create_nologin(முழு mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	எனில் ((fp = fopen(nologin_file, "w")) != வெற்று) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
நிலையான வெறுமை send_cmd(மாறிலி எழுத்து *cmd)
{
	FILE *fifo;
	size_t ignored;

	எனில் (do_dryrun) {
		einfo("Would send %s to init", cmd);
		திரும்பு;
	}
	எனில் (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	எனில் (!fifo) {
		perror("fopen");
		திரும்பு;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	எனில் (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
நிலையான வெறுமை sleep_no_interrupt(முழு seconds)
{
	கட்டமைப்பு timespec duration;
	கட்டமைப்பு timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	போது (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN நிலையான வெறுமை stop_shutdown(முழு sig)
{
	(வெறுமை) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	எழுத்து *ch = வெற்று;
	முழு opt;
	முழு cmd_மொத்தம் = 0;
	முழு hour = 0;
	முழு min = 0;
	முழு shutdown_delay = 0;
	கட்டமைப்பு sigaction sa;
	கட்டமைப்பு tm *lt;
	time_t tv;
	bool need_warning = false;
	எழுத்து *செய்தி = வெற்று;
	எழுத்து *state = வெற்று;
	எழுத்து *time_arg = வெற்று;
	FILE *fp;

	applet = basename_c(argv[0]);
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
			நிலை 'c':
				do_cancel = true;
			cmd_மொத்தம்++;
				நிறுத்து;
			நிலை 'd':
				do_wtmp = false;
				நிறுத்து;
		நிலை 'D':
			do_dryrun = true;
			நிறுத்து;
		நிலை 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_மொத்தம்++;
			நிறுத்து;
		நிலை 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_மொத்தம்++;
			நிறுத்து;
		நிலை 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_மொத்தம்++;
			நிறுத்து;
		நிலை 'R':
			do_reexec = true;
			cmd_மொத்தம்++;
			நிறுத்து;
		நிலை 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_மொத்தம்++;
			நிறுத்து;
		நிலை 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_மொத்தம்++;
			நிறுத்து;
		நிலை 'w':
			do_wtmp_only = true;
			cmd_மொத்தம்++;
			நிறுத்து;
		case_RC_COMMON_GETOPT
		}
	}
	எனில் (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	எனில் (cmd_மொத்தம் != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	எனில் (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} இல்லையெனில் எனில் (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} இல்லையெனில் எனில் (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	எனில் (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	எனில் (*time_arg == '+')
		time_arg++;
	எனில் (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	க்கு (ch=time_arg; *ch; ch++)
		எனில் ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	எனில் (strchr(time_arg, ':')) {
		எனில் ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		எனில் (shutdown_delay < 0)
			shutdown_delay += 1440;
	} இல்லையெனில் {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	எனில் (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, அளவு(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, வெற்று);
	sigaction(SIGTERM, &sa, வெற்று);
	போது (shutdown_delay > 0) {
		எனில் (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		இல்லையெனில் எனில் (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		இல்லையெனில் எனில்	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		இல்லையெனில்
			need_warning = true;

		எனில் (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		எனில் (need_warning) {
			xasprintf(&செய்தி, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(செய்தி);
			விடுவி(செய்தி);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&செய்தி, "\rThe system will %s now\r\n", state);
	broadcast(செய்தி);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	எனில் (do_halt) {
		எனில் (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} இல்லையெனில்
			send_cmd("halt");
	} இல்லையெனில் எனில் (do_kexec)
		send_cmd("kexec");
	இல்லையெனில் எனில் (do_poweroff) {
		எனில் (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} இல்லையெனில்
			send_cmd("poweroff");
	} இல்லையெனில் எனில் (do_reboot) {
		எனில் (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		இல்லையெனில்
			send_cmd("reboot");
	} இல்லையெனில் எனில் (do_single) {
		எனில் (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		இல்லையெனில்
			send_cmd("single");
	}
	திரும்பு 0;
}
