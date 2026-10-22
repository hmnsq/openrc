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

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "cancel",        no_argument, ಖಾಲಿ, 'c'},
	{ "no-write",        no_argument, ಖಾಲಿ, 'd'},
	{ "dry-run",        no_argument, ಖಾಲಿ, 'D'},
	{ "halt",        no_argument, ಖಾಲಿ, 'H'},
	{ "kexec",        no_argument, ಖಾಲಿ, 'K'},
	{ "poweroff",        no_argument, ಖಾಲಿ, 'p'},
	{ "reexec",        no_argument, ಖಾಲಿ, 'R'},
	{ "reboot",        no_argument, ಖಾಲಿ, 'r'},
	{ "single",        no_argument, ಖಾಲಿ, 's'},
	{ "write-only",        no_argument, ಖಾಲಿ, 'w'},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *nologin_file = RC_SYSCONFDIR"/nologin";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *shutdown_pid = "/run/openrc-shutdown.pid";

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
	ಆದರೆ (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	ಆದರೆ (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	ಇಲ್ಲದಿದ್ದರೆ
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ಸ್ಥಿರ ಶೂನ್ಯ create_nologin(ಪೂರ್ಣಾಂಕ mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	ಆದರೆ ((fp = fopen(nologin_file, "w")) != ಖಾಲಿ) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ಸ್ಥಿರ ಶೂನ್ಯ send_cmd(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd)
{
	FILE *fifo;
	size_t ignored;

	ಆದರೆ (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ಹಿಂತಿರುಗಿ;
	}
	ಆದರೆ (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	ಆದರೆ (!fifo) {
		perror("fopen");
		ಹಿಂತಿರುಗಿ;
	}

	ignored = fwrite(cmd, 1, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmd), fifo);
	ಆದರೆ (ignored != ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmd))
		ಬರೆ("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ಸ್ಥಿರ ಶೂನ್ಯ sleep_no_ಅಡಚಣೆ(ಪೂರ್ಣಾಂಕ seconds)
{
	ರಚನೆ timespec duration;
	ರಚನೆ timespec ಉಳಿದ;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	ಆಗ (nanosleep(&duration, &ಉಳಿದ) < 0 && errno == EINTR)
		duration = ಉಳಿದ;
}

RC_NORETURN ಸ್ಥಿರ ಶೂನ್ಯ stop_shutdown(ಪೂರ್ಣಾಂಕ sig)
{
	(ಶೂನ್ಯ) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	ನಿರ್ಗಮಿಸು(0);
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *ch = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ opt;
	ಪೂರ್ಣಾಂಕ cmd_ಎಣಿಕೆ = 0;
	ಪೂರ್ಣಾಂಕ hour = 0;
	ಪೂರ್ಣಾಂಕ min = 0;
	ಪೂರ್ಣಾಂಕ shutdown_delay = 0;
	ರಚನೆ sigaction sa;
	ರಚನೆ tm *lt;
	time_t tv;
	bool need_warning = false;
	ಅಕ್ಷರ *ಸಂದೇಶ_2 = ಖಾಲಿ;
	ಅಕ್ಷರ *state = ಖಾಲಿ;
	ಅಕ್ಷರ *ಸಮಯ_arg = ಖಾಲಿ;
	FILE *fp;

	applet = basename_c(argv[0]);
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
			ಸ್ಥಿತಿ 'c':
				do_cancel = true;
			cmd_ಎಣಿಕೆ++;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'd':
				do_wtmp = false;
				ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'D':
			do_dryrun = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'R':
			do_reexec = true;
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'w':
			do_wtmp_only = true;
			cmd_ಎಣಿಕೆ++;
			ನಿಲ್ಲಿಸು;
		case_RC_COMMON_GETOPT
		}
	}
	ಆದರೆ (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	ಆದರೆ (cmd_ಎಣಿಕೆ != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	ಆದರೆ (do_cancel) {
		cancel_shutdown();
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (do_reexec) {
		send_cmd("reexec");
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
	}

	ಆದರೆ (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	ಸಮಯ_arg = argv[optind];
	ಆದರೆ (*ಸಮಯ_arg == '+')
		ಸಮಯ_arg++;
	ಆದರೆ (strcasecmp(ಸಮಯ_arg, "now") == 0)
		strcpy(ಸಮಯ_arg, "0");
	ಗಾಗಿ (ch=ಸಮಯ_arg; *ch; ch++)
		ಆದರೆ ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, ಸಮಯ_arg);
			usage(EXIT_FAILURE);
		}
	ಆದರೆ (strchr(ಸಮಯ_arg, ':')) {
		ಆದರೆ ((ಅಕ್ಷರಸರಣಿ_ಓದು(ಸಮಯ_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, ಸಮಯ_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		ಆದರೆ (shutdown_delay < 0)
			shutdown_delay += 1440;
	} ಇಲ್ಲದಿದ್ದರೆ {
		shutdown_delay = atoi(ಸಮಯ_arg);
	}

	fp = fopen(shutdown_pid, "w");
	ಆದರೆ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ಖಾಲಿ);
	sigaction(SIGTERM, &sa, ಖಾಲಿ);
	ಆಗ (shutdown_delay > 0) {
		ಆದರೆ (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		ಇಲ್ಲದಿದ್ದರೆ
			need_warning = true;

		ಆದರೆ (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		ಆದರೆ (need_warning) {
			xasprintf(&ಸಂದೇಶ_2, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			ಪ್ರಸಾರ(ಸಂದೇಶ_2);
			ಬಿಡುಗಡೆ(ಸಂದೇಶ_2);
		}
		sleep_no_ಅಡಚಣೆ(60);
		shutdown_delay--;
	}
	xasprintf(&ಸಂದೇಶ_2, "\rThe system will %s now\r\n", state);
	ಪ್ರಸಾರ(ಸಂದೇಶ_2);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	ಆದರೆ (do_halt) {
		ಆದರೆ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} ಇಲ್ಲದಿದ್ದರೆ
			send_cmd("halt");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (do_kexec)
		send_cmd("kexec");
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (do_poweroff) {
		ಆದರೆ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} ಇಲ್ಲದಿದ್ದರೆ
			send_cmd("poweroff");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (do_reboot) {
		ಆದರೆ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		ಇಲ್ಲದಿದ್ದರೆ
			send_cmd("reboot");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (do_single) {
		ಆದರೆ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		ಇಲ್ಲದಿದ್ದರೆ
			send_cmd("single");
	}
	ಹಿಂತಿರುಗಿ 0;
}
