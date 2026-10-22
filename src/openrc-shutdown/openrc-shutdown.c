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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <syslog.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <utmp.h>

#అన్తర్భావ "broadcast.h"
#అన్తర్భావ "einfo.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "helpers.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "sysvinit.h"
#అన్తర్భావ "wtmp.h"
#అన్తర్భావ "_usage.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "cancel",        no_argument, ఖాళీ, 'c'},
	{ "no-write",        no_argument, ఖాళీ, 'd'},
	{ "dry-run",        no_argument, ఖాళీ, 'D'},
	{ "halt",        no_argument, ఖాళీ, 'H'},
	{ "kexec",        no_argument, ఖాళీ, 'K'},
	{ "poweroff",        no_argument, ఖాళీ, 'p'},
	{ "reexec",        no_argument, ఖాళీ, 'R'},
	{ "reboot",        no_argument, ఖాళీ, 'r'},
	{ "single",        no_argument, ఖాళీ, 's'},
	{ "write-only",        no_argument, ఖాళీ, 'w'},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిరరాశి అక్షరం *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
స్థిరరాశి అక్షరం *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
స్థిరరాశి అక్షరం *nologin_file = RC_SYSCONFDIR"/nologin";
స్థిరరాశి అక్షరం *shutdown_pid = "/run/openrc-shutdown.pid";

స్థిర bool do_cancel = false;
స్థిర bool do_dryrun = false;
స్థిర bool do_halt = false;
స్థిర bool do_kexec = false;
స్థిర bool do_poweroff = false;
స్థిర bool do_reboot = false;
స్థిర bool do_reexec = false;
స్థిర bool do_single = false;
స్థిర bool do_wtmp = true;
స్థిర bool do_wtmp_only = false;

స్థిర శూన్య cancel_shutdown(శూన్య)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	ఒకవేళ (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	ఒకవేళ (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	లేకపోతే
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
స్థిర శూన్య create_nologin(పూర్ణాంకం mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	ఒకవేళ ((fp = fopen(nologin_file, "w")) != ఖాళీ) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
స్థిర శూన్య send_cmd(స్థిరరాశి అక్షరం *cmd)
{
	FILE *fifo;
	size_t ignored;

	ఒకవేళ (do_dryrun) {
		einfo("Would send %s to init", cmd);
		తిరిగి;
	}
	ఒకవేళ (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	ఒకవేళ (!fifo) {
		perror("fopen");
		తిరిగి;
	}

	ignored = fwrite(cmd, 1, అక్షరశ్రేణి_పొడవు(cmd), fifo);
	ఒకవేళ (ignored != అక్షరశ్రేణి_పొడవు(cmd))
		రాయి("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
స్థిర శూన్య sleep_no_అంతరాయం(పూర్ణాంకం seconds)
{
	నిర్మాణం timespec duration;
	నిర్మాణం timespec మిగిలిన;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	అయితే (nanosleep(&duration, &మిగిలిన) < 0 && errno == EINTR)
		duration = మిగిలిన;
}

RC_NORETURN స్థిర శూన్య stop_shutdown(పూర్ణాంకం sig)
{
	(శూన్య) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	నిష్క్రమించు(0);
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	అక్షరం *ch = ఖాళీ;
	పూర్ణాంకం opt;
	పూర్ణాంకం cmd_లెక్క = 0;
	పూర్ణాంకం hour = 0;
	పూర్ణాంకం min = 0;
	పూర్ణాంకం shutdown_delay = 0;
	నిర్మాణం sigaction sa;
	నిర్మాణం tm *lt;
	time_t tv;
	bool need_warning = false;
	అక్షరం *సందేశం_2 = ఖాళీ;
	అక్షరం *state = ఖాళీ;
	అక్షరం *సమయం_arg = ఖాళీ;
	FILE *fp;

	applet = basename_c(argv[0]);
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
			స్థితి 'c':
				do_cancel = true;
			cmd_లెక్క++;
				ఆపు;
			స్థితి 'd':
				do_wtmp = false;
				ఆపు;
		స్థితి 'D':
			do_dryrun = true;
			ఆపు;
		స్థితి 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_లెక్క++;
			ఆపు;
		స్థితి 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_లెక్క++;
			ఆపు;
		స్థితి 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_లెక్క++;
			ఆపు;
		స్థితి 'R':
			do_reexec = true;
			cmd_లెక్క++;
			ఆపు;
		స్థితి 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_లెక్క++;
			ఆపు;
		స్థితి 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_లెక్క++;
			ఆపు;
		స్థితి 'w':
			do_wtmp_only = true;
			cmd_లెక్క++;
			ఆపు;
		case_RC_COMMON_GETOPT
		}
	}
	ఒకవేళ (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	ఒకవేళ (cmd_లెక్క != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	ఒకవేళ (do_cancel) {
		cancel_shutdown();
		నిష్క్రమించు(EXIT_SUCCESS);
	} లేకపోతే ఒకవేళ (do_reexec) {
		send_cmd("reexec");
		నిష్క్రమించు(EXIT_SUCCESS);
	} లేకపోతే ఒకవేళ (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		నిష్క్రమించు(EXIT_SUCCESS);
	}

	ఒకవేళ (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	సమయం_arg = argv[optind];
	ఒకవేళ (*సమయం_arg == '+')
		సమయం_arg++;
	ఒకవేళ (strcasecmp(సమయం_arg, "now") == 0)
		strcpy(సమయం_arg, "0");
	కొరకు (ch=సమయం_arg; *ch; ch++)
		ఒకవేళ ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, సమయం_arg);
			usage(EXIT_FAILURE);
		}
	ఒకవేళ (strchr(సమయం_arg, ':')) {
		ఒకవేళ ((అక్షరశ్రేణి_చదువు(సమయం_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, సమయం_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		ఒకవేళ (shutdown_delay < 0)
			shutdown_delay += 1440;
	} లేకపోతే {
		shutdown_delay = atoi(సమయం_arg);
	}

	fp = fopen(shutdown_pid, "w");
	ఒకవేళ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ఖాళీ);
	sigaction(SIGTERM, &sa, ఖాళీ);
	అయితే (shutdown_delay > 0) {
		ఒకవేళ (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		లేకపోతే ఒకవేళ (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		లేకపోతే ఒకవేళ	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		లేకపోతే
			need_warning = true;

		ఒకవేళ (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		ఒకవేళ (need_warning) {
			xasprintf(&సందేశం_2, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			ప్రసారం(సందేశం_2);
			విడుదల(సందేశం_2);
		}
		sleep_no_అంతరాయం(60);
		shutdown_delay--;
	}
	xasprintf(&సందేశం_2, "\rThe system will %s now\r\n", state);
	ప్రసారం(సందేశం_2);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	ఒకవేళ (do_halt) {
		ఒకవేళ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} లేకపోతే
			send_cmd("halt");
	} లేకపోతే ఒకవేళ (do_kexec)
		send_cmd("kexec");
	లేకపోతే ఒకవేళ (do_poweroff) {
		ఒకవేళ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} లేకపోతే
			send_cmd("poweroff");
	} లేకపోతే ఒకవేళ (do_reboot) {
		ఒకవేళ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		లేకపోతే
			send_cmd("reboot");
	} లేకపోతే ఒకవేళ (do_single) {
		ఒకవేళ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		లేకపోతే
			send_cmd("single");
	}
	తిరిగి 0;
}
