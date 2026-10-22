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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <utmp.h>

#ଅନ୍ତର୍ଭାଵ "broadcast.h"
#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "sysvinit.h"
#ଅନ୍ତର୍ଭାଵ "wtmp.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "cancel",        no_argument, ଖାଲି, 'c'},
	{ "no-write",        no_argument, ଖାଲି, 'd'},
	{ "dry-run",        no_argument, ଖାଲି, 'D'},
	{ "halt",        no_argument, ଖାଲି, 'H'},
	{ "kexec",        no_argument, ଖାଲି, 'K'},
	{ "poweroff",        no_argument, ଖାଲି, 'p'},
	{ "reexec",        no_argument, ଖାଲି, 'R'},
	{ "reboot",        no_argument, ଖାଲି, 'r'},
	{ "single",        no_argument, ଖାଲି, 's'},
	{ "write-only",        no_argument, ଖାଲି, 'w'},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ଅଚଳ ଅକ୍ଷର *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ଅଚଳ ଅକ୍ଷର *nologin_file = RC_SYSCONFDIR"/nologin";
ଅଚଳ ଅକ୍ଷର *shutdown_pid = "/run/openrc-shutdown.pid";

ସ୍ଥିର bool do_cancel = false;
ସ୍ଥିର bool do_dryrun = false;
ସ୍ଥିର bool do_halt = false;
ସ୍ଥିର bool do_kexec = false;
ସ୍ଥିର bool do_poweroff = false;
ସ୍ଥିର bool do_reboot = false;
ସ୍ଥିର bool do_reexec = false;
ସ୍ଥିର bool do_single = false;
ସ୍ଥିର bool do_wtmp = true;
ସ୍ଥିର bool do_wtmp_only = false;

ସ୍ଥିର ଶୂନ୍ୟ cancel_shutdown(ଶୂନ୍ୟ)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	ଯଦି (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	ଯଦି (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	ଅନ୍ୟଥା
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ସ୍ଥିର ଶୂନ୍ୟ create_nologin(ପୂର୍ଣ୍ଣସଂଖ୍ୟା mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	ଯଦି ((fp = fopen(nologin_file, "w")) != ଖାଲି) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ସ୍ଥିର ଶୂନ୍ୟ send_cmd(ଅଚଳ ଅକ୍ଷର *cmd)
{
	FILE *fifo;
	size_t ignored;

	ଯଦି (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ଫେରନ୍ତୁ;
	}
	ଯଦି (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	ଯଦି (!fifo) {
		perror("fopen");
		ଫେରନ୍ତୁ;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	ଯଦି (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ସ୍ଥିର ଶୂନ୍ୟ sleep_no_interrupt(ପୂର୍ଣ୍ଣସଂଖ୍ୟା seconds)
{
	ସଂରଚନା timespec duration;
	ସଂରଚନା timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	ଯେତେବେଳେ (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN ସ୍ଥିର ଶୂନ୍ୟ stop_shutdown(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	(ଶୂନ୍ୟ) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *ch = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_ଗଣନା = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hour = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା min = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା shutdown_delay = 0;
	ସଂରଚନା sigaction sa;
	ସଂରଚନା tm *lt;
	time_t tv;
	bool need_warning = false;
	ଅକ୍ଷର *ସନ୍ଦେଶ = ଖାଲି;
	ଅକ୍ଷର *state = ଖାଲି;
	ଅକ୍ଷର *time_arg = ଖାଲି;
	FILE *fp;

	applet = basename_c(argv[0]);
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
			ସ୍ଥିତି 'c':
				do_cancel = true;
			cmd_ଗଣନା++;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'd':
				do_wtmp = false;
				ଭାଙ୍ଗ;
		ସ୍ଥିତି 'D':
			do_dryrun = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'R':
			do_reexec = true;
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'w':
			do_wtmp_only = true;
			cmd_ଗଣନା++;
			ଭାଙ୍ଗ;
		case_RC_COMMON_GETOPT
		}
	}
	ଯଦି (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	ଯଦି (cmd_ଗଣନା != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	ଯଦି (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} ଅନ୍ୟଥା ଯଦି (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} ଅନ୍ୟଥା ଯଦି (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	ଯଦି (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	ଯଦି (*time_arg == '+')
		time_arg++;
	ଯଦି (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	ପାଇଁ (ch=time_arg; *ch; ch++)
		ଯଦି ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	ଯଦି (strchr(time_arg, ':')) {
		ଯଦି ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		ଯଦି (shutdown_delay < 0)
			shutdown_delay += 1440;
	} ଅନ୍ୟଥା {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	ଯଦି (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, ଆକାର(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ଖାଲି);
	sigaction(SIGTERM, &sa, ଖାଲି);
	ଯେତେବେଳେ (shutdown_delay > 0) {
		ଯଦି (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		ଅନ୍ୟଥା ଯଦି (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		ଅନ୍ୟଥା ଯଦି	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		ଅନ୍ୟଥା
			need_warning = true;

		ଯଦି (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		ଯଦି (need_warning) {
			xasprintf(&ସନ୍ଦେଶ, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(ସନ୍ଦେଶ);
			ମୁକ୍ତ(ସନ୍ଦେଶ);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&ସନ୍ଦେଶ, "\rThe system will %s now\r\n", state);
	broadcast(ସନ୍ଦେଶ);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	ଯଦି (do_halt) {
		ଯଦି (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} ଅନ୍ୟଥା
			send_cmd("halt");
	} ଅନ୍ୟଥା ଯଦି (do_kexec)
		send_cmd("kexec");
	ଅନ୍ୟଥା ଯଦି (do_poweroff) {
		ଯଦି (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} ଅନ୍ୟଥା
			send_cmd("poweroff");
	} ଅନ୍ୟଥା ଯଦି (do_reboot) {
		ଯଦି (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		ଅନ୍ୟଥା
			send_cmd("reboot");
	} ଅନ୍ୟଥା ଯଦି (do_single) {
		ଯଦି (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		ଅନ୍ୟଥା
			send_cmd("single");
	}
	ଫେରନ୍ତୁ 0;
}
