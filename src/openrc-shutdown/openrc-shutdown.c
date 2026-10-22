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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <utmp.h>

#ಅನ್ತರ್ಭಾವ "broadcast.h"
#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "sysvinit.h"
#ಅನ್ತರ್ಭಾವ "wtmp.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "cancel",        no_argument, ರಿಕ್ತ, 'c'},
	{ "no-write",        no_argument, ರಿಕ್ತ, 'd'},
	{ "dry-run",        no_argument, ರಿಕ್ತ, 'D'},
	{ "halt",        no_argument, ರಿಕ್ತ, 'H'},
	{ "kexec",        no_argument, ರಿಕ್ತ, 'K'},
	{ "poweroff",        no_argument, ರಿಕ್ತ, 'p'},
	{ "reexec",        no_argument, ರಿಕ್ತ, 'R'},
	{ "reboot",        no_argument, ರಿಕ್ತ, 'r'},
	{ "single",        no_argument, ರಿಕ್ತ, 's'},
	{ "write-only",        no_argument, ರಿಕ್ತ, 'w'},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಧ್ರುವ ವರ್ಣ *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ಧ್ರುವ ವರ್ಣ *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ಧ್ರುವ ವರ್ಣ *nologin_file = RC_SYSCONFDIR"/nologin";
ಧ್ರುವ ವರ್ಣ *shutdown_pid = "/run/openrc-shutdown.pid";

ಸ್ಥಿರ bool do_cancel = false;
ಸ್ಥಿರ bool do_dryrun = false;
ಸ್ಥಿರ bool do_halt = false;
ಸ್ಥಿರ bool do_kexec = false;
ಸ್ಥಿರ bool do_poweroff = false;
ಸ್ಥಿರ bool do_reboot = false;
ಸ್ಥಿರ bool do_reexec = false;
ಸ್ಥಿರ bool do_single = false;
ಸ್ಥಿರ bool do_wtmp = true;
ಸ್ಥಿರ bool do_wtmp_only = false;

ಸ್ಥಿರ ಶೂನ್ಯ cancel_shutdown(ಶೂನ್ಯ)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	ಯದಿ (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	ಯದಿ (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	ಅನ್ಯಥಾ
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ಸ್ಥಿರ ಶೂನ್ಯ create_nologin(ಪೂರ್ಣಾಙ್ಕ mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	ಯದಿ ((fp = fopen(nologin_file, "w")) != ರಿಕ್ತ) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ಸ್ಥಿರ ಶೂನ್ಯ send_cmd(ಧ್ರುವ ವರ್ಣ *cmd)
{
	FILE *fifo;
	size_t ignored;

	ಯದಿ (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ಪ್ರತ್ಯಾಗಮ;
	}
	ಯದಿ (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	ಯದಿ (!fifo) {
		perror("fopen");
		ಪ್ರತ್ಯಾಗಮ;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	ಯದಿ (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ಸ್ಥಿರ ಶೂನ್ಯ sleep_no_interrupt(ಪೂರ್ಣಾಙ್ಕ seconds)
{
	ಸಂರಚನಾ timespec duration;
	ಸಂರಚನಾ timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	ಯಾವತ್ (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ stop_shutdown(ಪೂರ್ಣಾಙ್ಕ sig)
{
	(ಶೂನ್ಯ) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ವರ್ಣ *ch = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ opt;
	ಪೂರ್ಣಾಙ್ಕ cmd_ಎಣಿಕೆ = 0;
	ಪೂರ್ಣಾಙ್ಕ hour = 0;
	ಪೂರ್ಣಾಙ್ಕ min = 0;
	ಪೂರ್ಣಾಙ್ಕ shutdown_delay = 0;
	ಸಂರಚನಾ sigaction sa;
	ಸಂರಚನಾ tm *lt;
	time_t tv;
	bool need_warning = false;
	ವರ್ಣ *ಸಂದೇಶ = ರಿಕ್ತ;
	ವರ್ಣ *state = ರಿಕ್ತ;
	ವರ್ಣ *time_arg = ರಿಕ್ತ;
	FILE *fp;

	applet = basename_c(argv[0]);
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
			ಪಕ್ಷ 'c':
				do_cancel = true;
			cmd_ಎಣಿಕೆ++;
				ಭಙ್ಗ;
			ಪಕ್ಷ 'd':
				do_wtmp = false;
				ಭಙ್ಗ;
		ಪಕ್ಷ 'D':
			do_dryrun = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'R':
			do_reexec = true;
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		ಪಕ್ಷ 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'w':
			do_wtmp_only = true;
			cmd_ಎಣಿಕೆ++;
			ಭಙ್ಗ;
		case_RC_COMMON_GETOPT
		}
	}
	ಯದಿ (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	ಯದಿ (cmd_ಎಣಿಕೆ != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	ಯದಿ (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} ಅನ್ಯಥಾ ಯದಿ (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} ಅನ್ಯಥಾ ಯದಿ (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	ಯದಿ (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	ಯದಿ (*time_arg == '+')
		time_arg++;
	ಯದಿ (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	ಪ್ರತಿ (ch=time_arg; *ch; ch++)
		ಯದಿ ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	ಯದಿ (strchr(time_arg, ':')) {
		ಯದಿ ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		ಯದಿ (shutdown_delay < 0)
			shutdown_delay += 1440;
	} ಅನ್ಯಥಾ {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	ಯದಿ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ರಿಕ್ತ);
	sigaction(SIGTERM, &sa, ರಿಕ್ತ);
	ಯಾವತ್ (shutdown_delay > 0) {
		ಯದಿ (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		ಅನ್ಯಥಾ ಯದಿ (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		ಅನ್ಯಥಾ ಯದಿ	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		ಅನ್ಯಥಾ
			need_warning = true;

		ಯದಿ (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		ಯದಿ (need_warning) {
			xasprintf(&ಸಂದೇಶ, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(ಸಂದೇಶ);
			ಮುಕ್ತ(ಸಂದೇಶ);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&ಸಂದೇಶ, "\rThe system will %s now\r\n", state);
	broadcast(ಸಂದೇಶ);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	ಯದಿ (do_halt) {
		ಯದಿ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} ಅನ್ಯಥಾ
			send_cmd("halt");
	} ಅನ್ಯಥಾ ಯದಿ (do_kexec)
		send_cmd("kexec");
	ಅನ್ಯಥಾ ಯದಿ (do_poweroff) {
		ಯದಿ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} ಅನ್ಯಥಾ
			send_cmd("poweroff");
	} ಅನ್ಯಥಾ ಯದಿ (do_reboot) {
		ಯದಿ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		ಅನ್ಯಥಾ
			send_cmd("reboot");
	} ಅನ್ಯಥಾ ಯದಿ (do_single) {
		ಯದಿ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		ಅನ್ಯಥಾ
			send_cmd("single");
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}
