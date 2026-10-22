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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <utmp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "broadcast.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sysvinit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "wtmp.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "cancel",        no_argument, ਨਿਰਾ, 'c'},
	{ "no-write",        no_argument, ਨਿਰਾ, 'd'},
	{ "dry-run",        no_argument, ਨਿਰਾ, 'D'},
	{ "halt",        no_argument, ਨਿਰਾ, 'H'},
	{ "kexec",        no_argument, ਨਿਰਾ, 'K'},
	{ "poweroff",        no_argument, ਨਿਰਾ, 'p'},
	{ "reexec",        no_argument, ਨਿਰਾ, 'R'},
	{ "reboot",        no_argument, ਨਿਰਾ, 'r'},
	{ "single",        no_argument, ਨਿਰਾ, 's'},
	{ "write-only",        no_argument, ਨਿਰਾ, 'w'},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ਅਚਲ ਅੱਖਰ *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ਅਚਲ ਅੱਖਰ *nologin_file = RC_SYSCONFDIR"/nologin";
ਅਚਲ ਅੱਖਰ *shutdown_pid = "/run/openrc-shutdown.pid";

ਸਥਿਰ bool do_cancel = false;
ਸਥਿਰ bool do_dryrun = false;
ਸਥਿਰ bool do_halt = false;
ਸਥਿਰ bool do_kexec = false;
ਸਥਿਰ bool do_poweroff = false;
ਸਥਿਰ bool do_reboot = false;
ਸਥਿਰ bool do_reexec = false;
ਸਥਿਰ bool do_single = false;
ਸਥਿਰ bool do_wtmp = true;
ਸਥਿਰ bool do_wtmp_only = false;

ਸਥਿਰ ਖਾਲੀ cancel_shutdown(ਖਾਲੀ)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	ਜੇ (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	ਜੇ (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	ਨਹੀਂਤਾਂ
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
ਸਥਿਰ ਖਾਲੀ create_nologin(ਪੂਰਨ mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	ਜੇ ((fp = fopen(nologin_file, "w")) != ਨਿਰਾ) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
ਸਥਿਰ ਖਾਲੀ send_cmd(ਅਚਲ ਅੱਖਰ *cmd)
{
	FILE *fifo;
	size_t ignored;

	ਜੇ (do_dryrun) {
		einfo("Would send %s to init", cmd);
		ਵਾਪਸ;
	}
	ਜੇ (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	ਜੇ (!fifo) {
		perror("fopen");
		ਵਾਪਸ;
	}

	ignored = fwrite(cmd, 1, ਲੜੀ_ਲੰਬਾਈ(cmd), fifo);
	ਜੇ (ignored != ਲੜੀ_ਲੰਬਾਈ(cmd))
		ਲਿਖੋ("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
ਸਥਿਰ ਖਾਲੀ sleep_no_ਰੁਕਾਵਟ(ਪੂਰਨ seconds)
{
	ਬਣਤਰ timespec duration;
	ਬਣਤਰ timespec ਬਚਿਆ;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	ਜਦੋਂ (nanosleep(&duration, &ਬਚਿਆ) < 0 && errno == EINTR)
		duration = ਬਚਿਆ;
}

RC_NORETURN ਸਥਿਰ ਖਾਲੀ stop_shutdown(ਪੂਰਨ sig)
{
	(ਖਾਲੀ) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	ਬਾਹਰ_ਨਿਕਲੋ(0);
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *ch = ਨਿਰਾ;
	ਪੂਰਨ opt;
	ਪੂਰਨ cmd_ਗਿਣਤੀ = 0;
	ਪੂਰਨ hour = 0;
	ਪੂਰਨ min = 0;
	ਪੂਰਨ shutdown_delay = 0;
	ਬਣਤਰ sigaction sa;
	ਬਣਤਰ tm *lt;
	time_t tv;
	bool need_warning = false;
	ਅੱਖਰ *ਸੁਨੇਹਾ_2 = ਨਿਰਾ;
	ਅੱਖਰ *state = ਨਿਰਾ;
	ਅੱਖਰ *ਸਮਾਂ_arg = ਨਿਰਾ;
	FILE *fp;

	applet = basename_c(argv[0]);
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
			ਹਾਲਤ 'c':
				do_cancel = true;
			cmd_ਗਿਣਤੀ++;
				ਰੋਕੋ;
			ਹਾਲਤ 'd':
				do_wtmp = false;
				ਰੋਕੋ;
		ਹਾਲਤ 'D':
			do_dryrun = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		ਹਾਲਤ 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		ਹਾਲਤ 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		ਹਾਲਤ 'R':
			do_reexec = true;
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		ਹਾਲਤ 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		ਹਾਲਤ 'w':
			do_wtmp_only = true;
			cmd_ਗਿਣਤੀ++;
			ਰੋਕੋ;
		case_RC_COMMON_GETOPT
		}
	}
	ਜੇ (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	ਜੇ (cmd_ਗਿਣਤੀ != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	ਜੇ (do_cancel) {
		cancel_shutdown();
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	} ਨਹੀਂਤਾਂ ਜੇ (do_reexec) {
		send_cmd("reexec");
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	} ਨਹੀਂਤਾਂ ਜੇ (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
	}

	ਜੇ (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	ਸਮਾਂ_arg = argv[optind];
	ਜੇ (*ਸਮਾਂ_arg == '+')
		ਸਮਾਂ_arg++;
	ਜੇ (strcasecmp(ਸਮਾਂ_arg, "now") == 0)
		strcpy(ਸਮਾਂ_arg, "0");
	ਲਈ (ch=ਸਮਾਂ_arg; *ch; ch++)
		ਜੇ ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, ਸਮਾਂ_arg);
			usage(EXIT_FAILURE);
		}
	ਜੇ (strchr(ਸਮਾਂ_arg, ':')) {
		ਜੇ ((ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ਸਮਾਂ_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, ਸਮਾਂ_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		ਜੇ (shutdown_delay < 0)
			shutdown_delay += 1440;
	} ਨਹੀਂਤਾਂ {
		shutdown_delay = atoi(ਸਮਾਂ_arg);
	}

	fp = fopen(shutdown_pid, "w");
	ਜੇ (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, ਨਿਰਾ);
	sigaction(SIGTERM, &sa, ਨਿਰਾ);
	ਜਦੋਂ (shutdown_delay > 0) {
		ਜੇ (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		ਨਹੀਂਤਾਂ ਜੇ (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		ਨਹੀਂਤਾਂ ਜੇ	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		ਨਹੀਂਤਾਂ
			need_warning = true;

		ਜੇ (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		ਜੇ (need_warning) {
			xasprintf(&ਸੁਨੇਹਾ_2, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			ਪ੍ਰਸਾਰਣ(ਸੁਨੇਹਾ_2);
			ਮੁਕਤ(ਸੁਨੇਹਾ_2);
		}
		sleep_no_ਰੁਕਾਵਟ(60);
		shutdown_delay--;
	}
	xasprintf(&ਸੁਨੇਹਾ_2, "\rThe system will %s now\r\n", state);
	ਪ੍ਰਸਾਰਣ(ਸੁਨੇਹਾ_2);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	ਜੇ (do_halt) {
		ਜੇ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} ਨਹੀਂਤਾਂ
			send_cmd("halt");
	} ਨਹੀਂਤਾਂ ਜੇ (do_kexec)
		send_cmd("kexec");
	ਨਹੀਂਤਾਂ ਜੇ (do_poweroff) {
		ਜੇ (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} ਨਹੀਂਤਾਂ
			send_cmd("poweroff");
	} ਨਹੀਂਤਾਂ ਜੇ (do_reboot) {
		ਜੇ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		ਨਹੀਂਤਾਂ
			send_cmd("reboot");
	} ਨਹੀਂਤਾਂ ਜੇ (do_single) {
		ਜੇ (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		ਨਹੀਂਤਾਂ
			send_cmd("single");
	}
	ਵਾਪਸ 0;
}
