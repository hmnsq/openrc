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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <syslog.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <utmp.h>

#अन्तर्भाव "broadcast.h"
#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "helpers.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "sysvinit.h"
#अन्तर्भाव "wtmp.h"
#अन्तर्भाव "_usage.h"

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "cancel",        no_argument, रिक्त, 'c'},
	{ "no-write",        no_argument, रिक्त, 'd'},
	{ "dry-run",        no_argument, रिक्त, 'D'},
	{ "halt",        no_argument, रिक्त, 'H'},
	{ "kexec",        no_argument, रिक्त, 'K'},
	{ "poweroff",        no_argument, रिक्त, 'p'},
	{ "reexec",        no_argument, रिक्त, 'R'},
	{ "reboot",        no_argument, रिक्त, 'r'},
	{ "single",        no_argument, रिक्त, 's'},
	{ "write-only",        no_argument, रिक्त, 'w'},
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
ध्रुव वर्ण *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
ध्रुव वर्ण *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
ध्रुव वर्ण *nologin_file = RC_SYSCONFDIR"/nologin";
ध्रुव वर्ण *shutdown_pid = "/run/openrc-shutdown.pid";

स्थिर bool do_cancel = false;
स्थिर bool do_dryrun = false;
स्थिर bool do_halt = false;
स्थिर bool do_kexec = false;
स्थिर bool do_poweroff = false;
स्थिर bool do_reboot = false;
स्थिर bool do_reexec = false;
स्थिर bool do_single = false;
स्थिर bool do_wtmp = true;
स्थिर bool do_wtmp_only = false;

स्थिर शून्य cancel_shutdown(शून्य)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	यदि (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	यदि (kill(pid, SIGTERM) != -1)
		einfo("%s: shutdown canceled", applet);
	अन्यथा
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
स्थिर शून्य create_nologin(पूर्णाङ्क mins)
{
	FILE *fp;
	time_t t;

	time(&t);
	t += 60 * mins;

	यदि ((fp = fopen(nologin_file, "w")) != रिक्त) {
		fprintf(fp, "\rThe system is going down on %s\r\n", ctime(&t));
		fclose(fp);
	}
}

/*
 * Send a command to our init
 */
स्थिर शून्य send_cmd(ध्रुव वर्ण *cmd)
{
	FILE *fifo;
	size_t ignored;

	यदि (do_dryrun) {
		einfo("Would send %s to init", cmd);
		प्रत्यागम;
	}
	यदि (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = fopen(RC_INIT_FIFO, "w");
	यदि (!fifo) {
		perror("fopen");
		प्रत्यागम;
	}

	ignored = fwrite(cmd, 1, strlen(cmd), fifo);
	यदि (ignored != strlen(cmd))
		printf("Error writing to init fifo\n");
	fclose(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
स्थिर शून्य sleep_no_interrupt(पूर्णाङ्क seconds)
{
	संरचना timespec duration;
	संरचना timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	यावत् (nanosleep(&duration, &remaining) < 0 && errno == EINTR)
		duration = remaining;
}

RC_NORETURN स्थिर शून्य stop_shutdown(पूर्णाङ्क sig)
{
	(शून्य) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	exit(0);
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *ch = रिक्त;
	पूर्णाङ्क opt;
	पूर्णाङ्क cmd_मोजणी = 0;
	पूर्णाङ्क hour = 0;
	पूर्णाङ्क min = 0;
	पूर्णाङ्क shutdown_delay = 0;
	संरचना sigaction sa;
	संरचना tm *lt;
	time_t tv;
	bool need_warning = false;
	वर्ण *संदेश = रिक्त;
	वर्ण *state = रिक्त;
	वर्ण *time_arg = रिक्त;
	FILE *fp;

	applet = basename_c(argv[0]);
	यावत् ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
			पक्ष 'c':
				do_cancel = true;
			cmd_मोजणी++;
				भङ्ग;
			पक्ष 'd':
				do_wtmp = false;
				भङ्ग;
		पक्ष 'D':
			do_dryrun = true;
			भङ्ग;
		पक्ष 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_मोजणी++;
			भङ्ग;
		पक्ष 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_मोजणी++;
			भङ्ग;
		पक्ष 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_मोजणी++;
			भङ्ग;
		पक्ष 'R':
			do_reexec = true;
			cmd_मोजणी++;
			भङ्ग;
		पक्ष 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_मोजणी++;
			भङ्ग;
		पक्ष 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_मोजणी++;
			भङ्ग;
		पक्ष 'w':
			do_wtmp_only = true;
			cmd_मोजणी++;
			भङ्ग;
		case_RC_COMMON_GETOPT
		}
	}
	यदि (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	यदि (cmd_मोजणी != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(EXIT_FAILURE);
	}

	यदि (do_cancel) {
		cancel_shutdown();
		exit(EXIT_SUCCESS);
	} अन्यथा यदि (do_reexec) {
		send_cmd("reexec");
		exit(EXIT_SUCCESS);
	} अन्यथा यदि (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		exit(EXIT_SUCCESS);
	}

	यदि (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(EXIT_FAILURE);
	}
	time_arg = argv[optind];
	यदि (*time_arg == '+')
		time_arg++;
	यदि (strcasecmp(time_arg, "now") == 0)
		strcpy(time_arg, "0");
	प्रति (ch=time_arg; *ch; ch++)
		यदि ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
	यदि (strchr(time_arg, ':')) {
		यदि ((sscanf(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(EXIT_FAILURE);
		}
		time(&tv);
		lt = localtime(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		यदि (shutdown_delay < 0)
			shutdown_delay += 1440;
	} अन्यथा {
		shutdown_delay = atoi(time_arg);
	}

	fp = fopen(shutdown_pid, "w");
	यदि (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, strerror(errno));
	fprintf(fp, "%d\n", getpid());
	fclose(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	memset(&sa, 0, परिमाण(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, रिक्त);
	sigaction(SIGTERM, &sa, रिक्त);
	यावत् (shutdown_delay > 0) {
		यदि (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		अन्यथा यदि (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		अन्यथा यदि	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		अन्यथा
			need_warning = true;

		यदि (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		यदि (need_warning) {
			xasprintf(&संदेश, "\rThe system will %s in %d minutes\r\n",
			          state, shutdown_delay);
			broadcast(संदेश);
			मुक्त(संदेश);
		}
		sleep_no_interrupt(60);
		shutdown_delay--;
	}
	xasprintf(&संदेश, "\rThe system will %s now\r\n", state);
	broadcast(संदेश);
	syslog(LOG_NOTICE, "The system will %s now", state);
	unlink(nologin_file);
	unlink(shutdown_pid);
	यदि (do_halt) {
		यदि (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} अन्यथा
			send_cmd("halt");
	} अन्यथा यदि (do_kexec)
		send_cmd("kexec");
	अन्यथा यदि (do_poweroff) {
		यदि (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} अन्यथा
			send_cmd("poweroff");
	} अन्यथा यदि (do_reboot) {
		यदि (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		अन्यथा
			send_cmd("reboot");
	} अन्यथा यदि (do_single) {
		यदि (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		अन्यथा
			send_cmd("single");
	}
	प्रत्यागम 0;
}
