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

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = रिक्त;
स्थिरांक अक्षर getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
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
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
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
स्थिरांक अक्षर *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
स्थिरांक अक्षर *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
स्थिरांक अक्षर *nologin_file = RC_SYSCONFDIR"/nologin";
स्थिरांक अक्षर *shutdown_pid = "/run/openrc-shutdown.pid";

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
	जर (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	जर (समाप्त(pid, संक_इति) != -1)
		einfo("%s: shutdown canceled", applet);
	अन्यथा
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
स्थिर शून्य create_nologin(पूर्णांक mins)
{
	खाता *fp;
	समय_प्रकार t;

	समय(&t);
	t += 60 * mins;

	जर ((fp = ख_खोलो(nologin_file, "w")) != रिक्त) {
		ख_लिखो(fp, "\rThe system is going down on %s\r\n", स_समय(&t));
		ख_बंद(fp);
	}
}

/*
 * Send a command to our init
 */
स्थिर शून्य send_cmd(स्थिरांक अक्षर *cmd)
{
	खाता *fifo;
	माप_प्रकार ignored;

	जर (do_dryrun) {
		einfo("Would send %s to init", cmd);
		परत;
	}
	जर (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = ख_खोलो(RC_INIT_FIFO, "w");
	जर (!fifo) {
		लिखो_त्रुटि("fopen");
		परत;
	}

	ignored = ख_डालो(cmd, 1, म_माप(cmd), fifo);
	जर (ignored != म_माप(cmd))
		म_लिखो("Error writing to init fifo\n");
	ख_बंद(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
स्थिर शून्य sleep_no_interrupt(पूर्णांक seconds)
{
	संरचना timespec duration;
	संरचना timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	पर्यंत (nanosleep(&duration, &remaining) < 0 && त्रुटि_सं == EINTR)
		duration = remaining;
}

RC_NORETURN स्थिर शून्य stop_shutdown(पूर्णांक sig)
{
	(शून्य) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	निकास(0);
}

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *ch = रिक्त;
	पूर्णांक opt;
	पूर्णांक cmd_मोजणी = 0;
	पूर्णांक hour = 0;
	पूर्णांक min = 0;
	पूर्णांक shutdown_delay = 0;
	संरचना sigaction sa;
	संरचना पंचांग *lt;
	समय_प्रकार tv;
	bool need_warning = false;
	अक्षर *संदेश = रिक्त;
	अक्षर *state = रिक्त;
	अक्षर *time_arg = रिक्त;
	खाता *fp;

	applet = basename_c(argv[0]);
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
			स्थिती 'c':
				do_cancel = true;
			cmd_मोजणी++;
				थांबा;
			स्थिती 'd':
				do_wtmp = false;
				थांबा;
		स्थिती 'D':
			do_dryrun = true;
			थांबा;
		स्थिती 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_मोजणी++;
			थांबा;
		स्थिती 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_मोजणी++;
			थांबा;
		स्थिती 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_मोजणी++;
			थांबा;
		स्थिती 'R':
			do_reexec = true;
			cmd_मोजणी++;
			थांबा;
		स्थिती 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_मोजणी++;
			थांबा;
		स्थिती 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_मोजणी++;
			थांबा;
		स्थिती 'w':
			do_wtmp_only = true;
			cmd_मोजणी++;
			थांबा;
		case_RC_COMMON_GETOPT
		}
	}
	जर (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	जर (cmd_मोजणी != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(निकास_त्रुटि);
	}

	जर (do_cancel) {
		cancel_shutdown();
		निकास(निकास_सफल);
	} अन्यथा जर (do_reexec) {
		send_cmd("reexec");
		निकास(निकास_सफल);
	} अन्यथा जर (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		निकास(निकास_सफल);
	}

	जर (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(निकास_त्रुटि);
	}
	time_arg = argv[optind];
	जर (*time_arg == '+')
		time_arg++;
	जर (strcasecmp(time_arg, "now") == 0)
		म_नकल(time_arg, "0");
	साठी (ch=time_arg; *ch; ch++)
		जर ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(निकास_त्रुटि);
		}
	जर (म_अक्षर(time_arg, ':')) {
		जर ((माला_पूछो(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(निकास_त्रुटि);
		}
		समय(&tv);
		lt = स_स्थानीय(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		जर (shutdown_delay < 0)
			shutdown_delay += 1440;
	} अन्यथा {
		shutdown_delay = म_से_प(time_arg);
	}

	fp = ख_खोलो(shutdown_pid, "w");
	जर (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, म_त्रुटि(त्रुटि_सं));
	ख_लिखो(fp, "%d\n", getpid());
	ख_बंद(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	स_रखो(&sa, 0, आकार(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(संक_विघ्न, &sa, रिक्त);
	sigaction(संक_इति, &sa, रिक्त);
	पर्यंत (shutdown_delay > 0) {
		जर (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		अन्यथा जर (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		अन्यथा जर	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		अन्यथा
			need_warning = true;

		जर (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		जर (need_warning) {
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
	जर (do_halt) {
		जर (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} अन्यथा
			send_cmd("halt");
	} अन्यथा जर (do_kexec)
		send_cmd("kexec");
	अन्यथा जर (do_poweroff) {
		जर (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} अन्यथा
			send_cmd("poweroff");
	} अन्यथा जर (do_reboot) {
		जर (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		अन्यथा
			send_cmd("reboot");
	} अन्यथा जर (do_single) {
		जर (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		अन्यथा
			send_cmd("single");
	}
	परत 0;
}
