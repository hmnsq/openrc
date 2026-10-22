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

#شامل <errno.h>
#شامل <getopt.h>
#شامل <signal.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <syslog.h>
#شامل <sys/types.h>
#شامل <strings.h>
#شامل <unistd.h>
#شامل <time.h>
#شامل <utmp.h>

#شامل "broadcast.h"
#شامل "einfo.h"
#شامل "rc.h"
#شامل "helpers.h"
#شامل "misc.h"
#شامل "sysvinit.h"
#شامل "wtmp.h"
#شامل "_usage.h"

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "cancel",        no_argument, تہی, 'c'},
	{ "no-write",        no_argument, تہی, 'd'},
	{ "dry-run",        no_argument, تہی, 'D'},
	{ "halt",        no_argument, تہی, 'H'},
	{ "kexec",        no_argument, تہی, 'K'},
	{ "poweroff",        no_argument, تہی, 'p'},
	{ "reexec",        no_argument, تہی, 'R'},
	{ "reboot",        no_argument, تہی, 'r'},
	{ "single",        no_argument, تہی, 's'},
	{ "write-only",        no_argument, تہی, 'w'},
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

ساکن bool do_cancel = false;
ساکن bool do_dryrun = false;
ساکن bool do_halt = false;
ساکن bool do_kexec = false;
ساکن bool do_poweroff = false;
ساکن bool do_reboot = false;
ساکن bool do_reexec = false;
ساکن bool do_single = false;
ساکن bool do_wtmp = true;
ساکن bool do_wtmp_only = false;

ساکن خالی cancel_shutdown(خالی)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	اگر (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	اگر (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	ورنہ
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ساکن خالی create_nologin(صحیح mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	اگر ((fp = fopen(nologin_file, "w")) != تہی) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ساکن خالی send_cmd(ثابت حرف *cmd)
{
	FILE *fifo;
	size_t ignored;

	اگر (do_dryrun) {
		einfo("Would send %s to init", cmd);
		واپس;
	}
	اگر (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	اگر (!fifo) {
		perror("fopen");
		واپس;
	}

	ignored = fwrite(cmd, 1, لڑی_لمبائی(cmd), fifo);
	اگر (ignored != لڑی_لمبائی(cmd))
		لکھو("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ساکن خالی sleep_no_مداخلت(صحیح seconds)
{
	ساخت timespec duration;
	ساخت timespec بقایا;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	جب (nanosleep(&duration, &بقایا) < 0 && errno == EINTR)
		duration = بقایا;
}

RC_NORETURN ساکن خالی stop_shutdown(صحیح sig)
{
	(خالی) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	باہر_نکلو(0);
}

صحیح مین(صحیح argc, حرف **argv)
{
	حرف *ch = تہی;
	صحیح opt;
	صحیح cmd_گنتی = 0;
	صحیح hour = 0;
	صحیح min = 0;
	صحیح shutdown_delay = 0;
	ساخت sigaction sa;
	ساخت tm *lt;
	time_t tv;
	bool need_warning = false;
	حرف *پیغام_2 = تہی;
	حرف *state = تہی;
	حرف *وقت_arg = تہی;
	FILE *fp;

	applet = basename_c(argv[0]);
	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
			حالت 'c':
				do_cancel = true;
			cmd_گنتی++;
				توڑو;
			حالت 'd':
				do_wtmp = false;
				توڑو;
		حالت 'D':
			do_dryrun = true;
			توڑو;
		حالت 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_گنتی++;
			توڑو;
		حالت 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_گنتی++;
			توڑو;
		حالت 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_گنتی++;
			توڑو;
		حالت 'R':
			do_reexec = true;
			cmd_گنتی++;
			توڑو;
		حالت 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_گنتی++;
			توڑو;
		حالت 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_گنتی++;
			توڑو;
		حالت 'w':
			do_wtmp_only = true;
			cmd_گنتی++;
			توڑو;
		case_RC_COMMON_GETOPT
		}
	}
	اگر (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	اگر (cmd_گنتی != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	اگر (do_cancel) {
		cancel_shutdown();
		باہر_نکلو(EXIT_SUCCESS);
	} ورنہ اگر (do_reexec) {
		send_cmd("reexec");
		باہر_نکلو(EXIT_SUCCESS);
	} ورنہ اگر (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		باہر_نکلو(EXIT_SUCCESS);
	}

	اگر (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	وقت_arg = argv[optind];
	اگر (*وقت_arg == '+')
		وقت_arg++;
	اگر (strcasecmp(وقت_arg, "now") == 0)
		strcpy(وقت_arg, "0");
	برائے (ch=وقت_arg; *ch; ch++)
		اگر ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, وقت_arg);
			usage(EXIT_FAILURE);
		}
	اگر (strchr(وقت_arg, ':')) {
		اگر ((لڑی_سے_پڑھو(وقت_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, وقت_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		اگر (shutdown_delay < 0)
			shutdown_delay += 1440;
	} ورنہ {
		shutdown_delay = atoi(وقت_arg);
	}

	fp = fopen(shutdown_pid, "w");
	اگر (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, حجم(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, تہی);
	sigaction(SIGTERM, &sa, تہی);
	جب (shutdown_delay > 0) {
		اگر (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		ورنہ اگر (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		ورنہ اگر	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		ورنہ
			need_warning = true;

		اگر (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		اگر (need_warning) {
			xasprintf(&پیغام_2, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			نشر(پیغام_2);
			آزاد(پیغام_2);
		}
		sleep_no_مداخلت(60);
		shutdown_delay--;
	}
	xasprintf(&پیغام_2, "\rThe system will %s now\r\n", state);
	نشر(پیغام_2);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	اگر (do_halt) {
		اگر (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} ورنہ
			send_cmd("halt");
	} ورنہ اگر (do_kexec)
		send_cmd("kexec");
	ورنہ اگر (do_poweroff) {
		اگر (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} ورنہ
			send_cmd("poweroff");
	} ورنہ اگر (do_reboot) {
		اگر (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		ورنہ
			send_cmd("reboot");
	} ورنہ اگر (do_single) {
		اگر (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		ورنہ
			send_cmd("single");
	}
	واپس 0;
}
