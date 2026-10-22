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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <syslog.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <utmp.h>

#അന്തര്ഭാവ "broadcast.h"
#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "sysvinit.h"
#അന്തര്ഭാവ "wtmp.h"
#അന്തര്ഭാവ "_usage.h"

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "cancel",        no_argument, ശൂന്യ, 'c'},
	{ "no-write",        no_argument, ശൂന്യ, 'd'},
	{ "dry-run",        no_argument, ശൂന്യ, 'D'},
	{ "halt",        no_argument, ശൂന്യ, 'H'},
	{ "kexec",        no_argument, ശൂന്യ, 'K'},
	{ "poweroff",        no_argument, ശൂന്യ, 'p'},
	{ "reexec",        no_argument, ശൂന്യ, 'R'},
	{ "reboot",        no_argument, ശൂന്യ, 'r'},
	{ "single",        no_argument, ശൂന്യ, 's'},
	{ "write-only",        no_argument, ശൂന്യ, 'w'},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
സ്ഥിരം അക്ഷരം *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
സ്ഥിരം അക്ഷരം *nologin_file = RC_SYSCONFDIR"/nologin";
സ്ഥിരം അക്ഷരം *shutdown_pid = "/run/openrc-shutdown.pid";

സ്ഥിതം bool do_cancel = false;
സ്ഥിതം bool do_dryrun = false;
സ്ഥിതം bool do_halt = false;
സ്ഥിതം bool do_kexec = false;
സ്ഥിതം bool do_poweroff = false;
സ്ഥിതം bool do_reboot = false;
സ്ഥിതം bool do_reexec = false;
സ്ഥിതം bool do_single = false;
സ്ഥിതം bool do_wtmp = true;
സ്ഥിതം bool do_wtmp_only = false;

സ്ഥിതം ശൂന്യം cancel_shutdown(ശൂന്യം)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	എങ്കിൽ (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	എങ്കിൽ (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	അല്ലെങ്കിൽ
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
സ്ഥിതം ശൂന്യം create_nologin(പൂർണ്ണസംഖ്യ mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	എങ്കിൽ ((fp = fopen(nologin_file, "w")) != ശൂന്യ) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
സ്ഥിതം ശൂന്യം send_cmd(സ്ഥിരം അക്ഷരം *cmd)
{
	FILE *fifo;
	size_t ignored;

	എങ്കിൽ (do_dryrun) {
		einfo("Would send %s to init", cmd);
		തിരികെ;
	}
	എങ്കിൽ (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	എങ്കിൽ (!fifo) {
		perror("fopen");
		തിരികെ;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	എങ്കിൽ (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
സ്ഥിതം ശൂന്യം sleep_no_interrupt(പൂർണ്ണസംഖ്യ seconds)
{
	ഘടന timespec duration;
	ഘടന timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	ആകുമ്പോൾ (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN സ്ഥിതം ശൂന്യം stop_shutdown(പൂർണ്ണസംഖ്യ sig)
{
	(ശൂന്യം) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	അക്ഷരം *ch = ശൂന്യ;
	പൂർണ്ണസംഖ്യ opt;
	പൂർണ്ണസംഖ്യ cmd_എണ്ണം = 0;
	പൂർണ്ണസംഖ്യ hour = 0;
	പൂർണ്ണസംഖ്യ min = 0;
	പൂർണ്ണസംഖ്യ shutdown_delay = 0;
	ഘടന sigaction sa;
	ഘടന tm *lt;
	time_t tv;
	bool need_warning = false;
	അക്ഷരം *സന്ദേശം = ശൂന്യ;
	അക്ഷരം *state = ശൂന്യ;
	അക്ഷരം *time_arg = ശൂന്യ;
	FILE *fp;

	applet = basename_c(argv[0]);
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
			അവസ്ഥ 'c':
				do_cancel = true;
			cmd_എണ്ണം++;
				നിർത്തുക;
			അവസ്ഥ 'd':
				do_wtmp = false;
				നിർത്തുക;
		അവസ്ഥ 'D':
			do_dryrun = true;
			നിർത്തുക;
		അവസ്ഥ 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_എണ്ണം++;
			നിർത്തുക;
		അവസ്ഥ 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_എണ്ണം++;
			നിർത്തുക;
		അവസ്ഥ 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_എണ്ണം++;
			നിർത്തുക;
		അവസ്ഥ 'R':
			do_reexec = true;
			cmd_എണ്ണം++;
			നിർത്തുക;
		അവസ്ഥ 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_എണ്ണം++;
			നിർത്തുക;
		അവസ്ഥ 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_എണ്ണം++;
			നിർത്തുക;
		അവസ്ഥ 'w':
			do_wtmp_only = true;
			cmd_എണ്ണം++;
			നിർത്തുക;
		case_RC_COMMON_GETOPT
		}
	}
	എങ്കിൽ (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	എങ്കിൽ (cmd_എണ്ണം != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	എങ്കിൽ (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} അല്ലെങ്കിൽ എങ്കിൽ (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} അല്ലെങ്കിൽ എങ്കിൽ (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	എങ്കിൽ (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	എങ്കിൽ (*time_arg == '+')
		time_arg++;
	എങ്കിൽ (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	വേണ്ടി (ch=time_arg; *ch; ch++)
		എങ്കിൽ ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	എങ്കിൽ (strchr(time_arg, ':')) {
		എങ്കിൽ ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		എങ്കിൽ (shutdown_delay < 0)
			shutdown_delay += 1440;
	} അല്ലെങ്കിൽ {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	എങ്കിൽ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, വലുപ്പം(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ശൂന്യ);
	sigaction(SIGTERM, &sa, ശൂന്യ);
	ആകുമ്പോൾ (shutdown_delay > 0) {
		എങ്കിൽ (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		അല്ലെങ്കിൽ എങ്കിൽ (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		അല്ലെങ്കിൽ എങ്കിൽ	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		അല്ലെങ്കിൽ
			need_warning = true;

		എങ്കിൽ (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		എങ്കിൽ (need_warning) {
			xasprintf(&സന്ദേശം, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(സന്ദേശം);
			സ്വതന്ത്രം(സന്ദേശം);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&സന്ദേശം, "\rThe system will %s now\r\n", state);
	broadcast(സന്ദേശം);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	എങ്കിൽ (do_halt) {
		എങ്കിൽ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} അല്ലെങ്കിൽ
			send_cmd("halt");
	} അല്ലെങ്കിൽ എങ്കിൽ (do_kexec)
		send_cmd("kexec");
	അല്ലെങ്കിൽ എങ്കിൽ (do_poweroff) {
		എങ്കിൽ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} അല്ലെങ്കിൽ
			send_cmd("poweroff");
	} അല്ലെങ്കിൽ എങ്കിൽ (do_reboot) {
		എങ്കിൽ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		അല്ലെങ്കിൽ
			send_cmd("reboot");
	} അല്ലെങ്കിൽ എങ്കിൽ (do_single) {
		എങ്കിൽ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		അല്ലെങ്കിൽ
			send_cmd("single");
	}
	തിരികെ 0;
}
