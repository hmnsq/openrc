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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <syslog.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <utmp.h>

#અન્તર્ભાવ "broadcast.h"
#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "sysvinit.h"
#અન્તર્ભાવ "wtmp.h"
#અન્તર્ભાવ "_usage.h"

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "cancel",        no_argument, ખાલી, 'c'},
	{ "no-write",        no_argument, ખાલી, 'd'},
	{ "dry-run",        no_argument, ખાલી, 'D'},
	{ "halt",        no_argument, ખાલી, 'H'},
	{ "kexec",        no_argument, ખાલી, 'K'},
	{ "poweroff",        no_argument, ખાલી, 'p'},
	{ "reexec",        no_argument, ખાલી, 'R'},
	{ "reboot",        no_argument, ખાલી, 'r'},
	{ "single",        no_argument, ખાલી, 's'},
	{ "write-only",        no_argument, ખાલી, 'w'},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
અચલ અક્ષર *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
અચલ અક્ષર *nologin_file = RC_SYSCONFDIR"/nologin";
અચલ અક્ષર *shutdown_pid = "/run/openrc-shutdown.pid";

સ્થિર bool do_cancel = false;
સ્થિર bool do_dryrun = false;
સ્થિર bool do_halt = false;
સ્થિર bool do_kexec = false;
સ્થિર bool do_poweroff = false;
સ્થિર bool do_reboot = false;
સ્થિર bool do_reexec = false;
સ્થિર bool do_single = false;
સ્થિર bool do_wtmp = true;
સ્થિર bool do_wtmp_only = false;

સ્થિર શૂન્ય cancel_shutdown(શૂન્ય)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	જો (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	જો (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	નહીંતર
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
સ્થિર શૂન્ય create_nologin(પૂર્ણાંક mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	જો ((fp = fopen(nologin_file, "w")) != ખાલી) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
સ્થિર શૂન્ય send_cmd(અચલ અક્ષર *cmd)
{
	FILE *fifo;
	size_t ignored;

	જો (do_dryrun) {
		einfo("Would send %s to init", cmd);
		પાછા;
	}
	જો (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	જો (!fifo) {
		perror("fopen");
		પાછા;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	જો (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
સ્થિર શૂન્ય sleep_no_interrupt(પૂર્ણાંક seconds)
{
	સંરચના timespec duration;
	સંરચના timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	જ્યારે (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN સ્થિર શૂન્ય stop_shutdown(પૂર્ણાંક sig)
{
	(શૂન્ય) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	અક્ષર *ch = ખાલી;
	પૂર્ણાંક opt;
	પૂર્ણાંક cmd_ગણતરી = 0;
	પૂર્ણાંક hour = 0;
	પૂર્ણાંક min = 0;
	પૂર્ણાંક shutdown_delay = 0;
	સંરચના sigaction sa;
	સંરચના tm *lt;
	time_t tv;
	bool need_warning = false;
	અક્ષર *સંદેશ = ખાલી;
	અક્ષર *state = ખાલી;
	અક્ષર *time_arg = ખાલી;
	FILE *fp;

	applet = basename_c(argv[0]);
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
			સ્થિતિ 'c':
				do_cancel = true;
			cmd_ગણતરી++;
				તોડો;
			સ્થિતિ 'd':
				do_wtmp = false;
				તોડો;
		સ્થિતિ 'D':
			do_dryrun = true;
			તોડો;
		સ્થિતિ 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_ગણતરી++;
			તોડો;
		સ્થિતિ 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ગણતરી++;
			તોડો;
		સ્થિતિ 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_ગણતરી++;
			તોડો;
		સ્થિતિ 'R':
			do_reexec = true;
			cmd_ગણતરી++;
			તોડો;
		સ્થિતિ 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ગણતરી++;
			તોડો;
		સ્થિતિ 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_ગણતરી++;
			તોડો;
		સ્થિતિ 'w':
			do_wtmp_only = true;
			cmd_ગણતરી++;
			તોડો;
		case_RC_COMMON_GETOPT
		}
	}
	જો (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	જો (cmd_ગણતરી != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	જો (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} નહીંતર જો (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} નહીંતર જો (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	જો (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	જો (*time_arg == '+')
		time_arg++;
	જો (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	માટે (ch=time_arg; *ch; ch++)
		જો ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	જો (strchr(time_arg, ':')) {
		જો ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		જો (shutdown_delay < 0)
			shutdown_delay += 1440;
	} નહીંતર {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	જો (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, કદ(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ખાલી);
	sigaction(SIGTERM, &sa, ખાલી);
	જ્યારે (shutdown_delay > 0) {
		જો (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		નહીંતર જો (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		નહીંતર જો	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		નહીંતર
			need_warning = true;

		જો (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		જો (need_warning) {
			xasprintf(&સંદેશ, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(સંદેશ);
			મુક્ત(સંદેશ);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&સંદેશ, "\rThe system will %s now\r\n", state);
	broadcast(સંદેશ);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	જો (do_halt) {
		જો (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} નહીંતર
			send_cmd("halt");
	} નહીંતર જો (do_kexec)
		send_cmd("kexec");
	નહીંતર જો (do_poweroff) {
		જો (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} નહીંતર
			send_cmd("poweroff");
	} નહીંતર જો (do_reboot) {
		જો (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		નહીંતર
			send_cmd("reboot");
	} નહીંતર જો (do_single) {
		જો (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		નહીંતર
			send_cmd("single");
	}
	પાછા 0;
}
