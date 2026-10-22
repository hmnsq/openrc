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

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <utmp.h>

#include "broadcast.h"
#include "einfo.h"
#include "rc.h"
#include "helpers.h"
#include "misc.h"
#include "sysvinit.h"
#include "wtmp.h"
#include "_usage.h"

const var_na *applet = NULL;
const var_na *extraopts = NULL;
const var_na getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "cancel",        no_argument, NULL, 'c'},
	{ "no-write",        no_argument, NULL, 'd'},
	{ "dry-run",        no_argument, NULL, 'D'},
	{ "halt",        no_argument, NULL, 'H'},
	{ "kexec",        no_argument, NULL, 'K'},
	{ "poweroff",        no_argument, NULL, 'p'},
	{ "reexec",        no_argument, NULL, 'R'},
	{ "reboot",        no_argument, NULL, 'r'},
	{ "single",        no_argument, NULL, 's'},
	{ "write-only",        no_argument, NULL, 'w'},
	longopts_COMMON
};
const var_na * const longopts_help[] = {
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
const var_na *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
const var_na *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
const var_na *nologin_file = RC_SYSCONFDIR"/nologin";
const var_na *shutdown_pid = "/run/openrc-shutdown.pid";

static bool do_cancel = false;
static bool do_dryrun = false;
static bool do_halt = false;
static bool do_kexec = false;
static bool do_poweroff = false;
static bool do_reboot = false;
static bool do_reexec = false;
static bool do_single = false;
static bool do_wtmp = true;
static bool do_wtmp_only = false;

static void cancel_shutdown(void)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	အကယ်၍ (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	အကယ်၍ (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	မဟုတ်ပါက
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
static void create_nologin(ကိန်းပြည့် mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	အကယ်၍ ((fp = fopen(nologin_file, "w")) != NULL) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
static void send_cmd(const var_na *cmd)
{
	FILE *fifo;
	size_t ignored;

	အကယ်၍ (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ပြန်ပေး;
	}
	အကယ်၍ (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	အကယ်၍ (!fifo) {
		perror("fopen");
		ပြန်ပေး;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	အကယ်၍ (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
static void sleep_no_interrupt(ကိန်းပြည့် seconds)
{
	sa_nracanaa timespec duration;
	sa_nracanaa timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	နေစဉ် (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN static void stop_shutdown(ကိန်းပြည့် sig)
{
	(void) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na **argv)
{
	var_na *ch = NULL;
	ကိန်းပြည့် opt;
	ကိန်းပြည့် cmd_count = 0;
	ကိန်းပြည့် hour = 0;
	ကိန်းပြည့် min = 0;
	ကိန်းပြည့် shutdown_delay = 0;
	sa_nracanaa sigaction sa;
	sa_nracanaa tm *lt;
	time_t tv;
	bool need_warning = false;
	var_na *msg = NULL;
	var_na *state = NULL;
	var_na *time_arg = NULL;
	FILE *fp;

	applet = basename_c(argv[0]);
	နေစဉ် ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ကိန်းပြည့် *) 0)) != -1)
	{
		switch (opt) {
			pak_sa 'c':
				do_cancel = true;
			cmd_count++;
				bha_nga;
			pak_sa 'd':
				do_wtmp = false;
				bha_nga;
		pak_sa 'D':
			do_dryrun = true;
			bha_nga;
		pak_sa 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_count++;
			bha_nga;
		pak_sa 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_count++;
			bha_nga;
		pak_sa 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_count++;
			bha_nga;
		pak_sa 'R':
			do_reexec = true;
			cmd_count++;
			bha_nga;
		pak_sa 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_count++;
			bha_nga;
		pak_sa 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_count++;
			bha_nga;
		pak_sa 'w':
			do_wtmp_only = true;
			cmd_count++;
			bha_nga;
		case_RC_COMMON_GETOPT
		}
	}
	အကယ်၍ (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	အကယ်၍ (cmd_count != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	အကယ်၍ (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} မဟုတ်ပါက အကယ်၍ (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} မဟုတ်ပါက အကယ်၍ (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	အကယ်၍ (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	အကယ်၍ (*time_arg == '+')
		time_arg++;
	အကယ်၍ (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	အတွက် (ch=time_arg; *ch; ch++)
		အကယ်၍ ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	အကယ်၍ (strchr(time_arg, ':')) {
		အကယ်၍ ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		အကယ်၍ (shutdown_delay < 0)
			shutdown_delay += 1440;
	} မဟုတ်ပါက {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	အကယ်၍ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, parimaa_na(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	နေစဉ် (shutdown_delay > 0) {
		အကယ်၍ (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		မဟုတ်ပါက အကယ်၍ (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		မဟုတ်ပါက အကယ်၍	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		မဟုတ်ပါက
			need_warning = true;

		အကယ်၍ (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		အကယ်၍ (need_warning) {
			xasprintf(&msg, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(msg);
			free(msg);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&msg, "\rThe system will %s now\r\n", state);
	broadcast(msg);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	အကယ်၍ (do_halt) {
		အကယ်၍ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} မဟုတ်ပါက
			send_cmd("halt");
	} မဟုတ်ပါက အကယ်၍ (do_kexec)
		send_cmd("kexec");
	မဟုတ်ပါက အကယ်၍ (do_poweroff) {
		အကယ်၍ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} မဟုတ်ပါက
			send_cmd("poweroff");
	} မဟုတ်ပါက အကယ်၍ (do_reboot) {
		အကယ်၍ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		မဟုတ်ပါက
			send_cmd("reboot");
	} မဟုတ်ပါက အကယ်၍ (do_single) {
		အကယ်၍ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		မဟုတ်ပါက
			send_cmd("single");
	}
	ပြန်ပေး 0;
}
