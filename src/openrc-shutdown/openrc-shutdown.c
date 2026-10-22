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

#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <signal.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <syslog.h>
#समावेश <sys/types.h>
#समावेश <strings.h>
#समावेश <unistd.h>
#समावेश <time.h>
#समावेश <utmp.h>

#समावेश "broadcast.h"
#समावेश "einfo.h"
#समावेश "rc.h"
#समावेश "helpers.h"
#समावेश "misc.h"
#समावेश "sysvinit.h"
#समावेश "wtmp.h"
#समावेश "_usage.h"

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "cdDfFHKpRrsw" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "cancel",        no_argument, शून्य, 'c'},
	{ "no-write",        no_argument, शून्य, 'd'},
	{ "dry-run",        no_argument, शून्य, 'D'},
	{ "halt",        no_argument, शून्य, 'H'},
	{ "kexec",        no_argument, शून्य, 'K'},
	{ "poweroff",        no_argument, शून्य, 'p'},
	{ "reexec",        no_argument, शून्य, 'R'},
	{ "reboot",        no_argument, शून्य, 'r'},
	{ "single",        no_argument, शून्य, 's'},
	{ "write-only",        no_argument, शून्य, 'w'},
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
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
स्थिर अक्षर *usagestring = ""
	"Usage: openrc-shutdown -c | --cancel\n"
	"   or: openrc-shutdown -R | --reexec\n"
	"   or: openrc-shutdown -w | --write-only\n"
	"   or: openrc-shutdown -H | --halt time\n"
	"   or: openrc-shutdown -K | --kexec time\n"
	"   or: openrc-shutdown -p | --poweroff time\n"
	"   or: openrc-shutdown -r | --reboot time\n"
	"   or: openrc-shutdown -s | --single time";
स्थिर अक्षर *exclusive = "Select one of "
	"--cancel, --halt, --kexec, --poweroff, --reexec, --reboot, --single or \n"
	"--write-only";
स्थिर अक्षर *nologin_file = RC_SYSCONFDIR"/nologin";
स्थिर अक्षर *shutdown_pid = "/run/openrc-shutdown.pid";

अटल bool do_cancel = false;
अटल bool do_dryrun = false;
अटल bool do_halt = false;
अटल bool do_kexec = false;
अटल bool do_poweroff = false;
अटल bool do_reboot = false;
अटल bool do_reexec = false;
अटल bool do_single = false;
अटल bool do_wtmp = true;
अटल bool do_wtmp_only = false;

अटल व्योम cancel_shutdown(व्योम)
{
	pid_t pid;

	pid = get_pid(applet, shutdown_pid);
	अगर (pid <= 0)
		eerrorx("%s: Unable to cancel shutdown", applet);

	अगर (समाप्त(pid, संक_इति) != -1)
		einfo("%s: shutdown canceled", applet);
	अन्यथा
		eerrorx("%s: Unable to cancel shutdown", applet);
}

/*
 *	Create the nologin file.
 */
अटल व्योम create_nologin(पूर्णांक mins)
{
	खाता *fp;
	समय_प्रकार t;

	समय(&t);
	t += 60 * mins;

	अगर ((fp = ख_खोलो(nologin_file, "w")) != शून्य) {
		ख_लिखो(fp, "\rThe system is going down on %s\r\n", स_समय(&t));
		ख_बंद(fp);
	}
}

/*
 * Send a command to our init
 */
अटल व्योम send_cmd(स्थिर अक्षर *cmd)
{
	खाता *fifo;
	माप_प्रकार ignored;

	अगर (do_dryrun) {
		einfo("Would send %s to init", cmd);
		वापस;
	}
	अगर (do_wtmp && (do_halt || do_kexec || do_reboot || do_poweroff))
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
	fifo = ख_खोलो(RC_INIT_FIFO, "w");
	अगर (!fifo) {
		लिखो_त्रुटि("fopen");
		वापस;
	}

	ignored = ख_डालो(cmd, 1, म_माप(cmd), fifo);
	अगर (ignored != म_माप(cmd))
		म_लिखो("Error writing to init fifo\n");
	ख_बंद(fifo);
}

/*
 * sleep without being interrupted.
 * The idea for this code came from sysvinit.
 */
अटल व्योम sleep_no_interrupt(पूर्णांक seconds)
{
	काष्ठा timespec duration;
	काष्ठा timespec remaining;

	duration.tv_sec = seconds;
	duration.tv_nsec = 0;

	जबतक (nanosleep(&duration, &remaining) < 0 && त्रुटि_सं == EINTR)
		duration = remaining;
}

RC_NORETURN अटल व्योम stop_shutdown(पूर्णांक sig)
{
	(व्योम) sig;
	unlink(nologin_file);
	unlink(shutdown_pid);
	einfo("Shutdown cancelled");
	निकास(0);
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *ch = शून्य;
	पूर्णांक opt;
	पूर्णांक cmd_गिनती = 0;
	पूर्णांक hour = 0;
	पूर्णांक min = 0;
	पूर्णांक shutdown_delay = 0;
	काष्ठा sigaction sa;
	काष्ठा पंचांग *lt;
	समय_प्रकार tv;
	bool need_warning = false;
	अक्षर *संदेश = शून्य;
	अक्षर *state = शून्य;
	अक्षर *time_arg = शून्य;
	खाता *fp;

	applet = basename_c(argv[0]);
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
			हाल 'c':
				do_cancel = true;
			cmd_गिनती++;
				अवरोध;
			हाल 'd':
				do_wtmp = false;
				अवरोध;
		हाल 'D':
			do_dryrun = true;
			अवरोध;
		हाल 'H':
			do_halt = true;
			xasprintf(&state, "%s", "halt");
			cmd_गिनती++;
			अवरोध;
		हाल 'K':
			do_kexec = true;
			xasprintf(&state, "%s", "reboot");
			cmd_गिनती++;
			अवरोध;
		हाल 'p':
			do_poweroff = true;
			xasprintf(&state, "%s", "power off");
			cmd_गिनती++;
			अवरोध;
		हाल 'R':
			do_reexec = true;
			cmd_गिनती++;
			अवरोध;
		हाल 'r':
			do_reboot = true;
			xasprintf(&state, "%s", "reboot");
			cmd_गिनती++;
			अवरोध;
		हाल 's':
			do_single = true;
			xasprintf(&state, "%s", "go down for maintenance");
			cmd_गिनती++;
			अवरोध;
		हाल 'w':
			do_wtmp_only = true;
			cmd_गिनती++;
			अवरोध;
		case_RC_COMMON_GETOPT
		}
	}
	अगर (geteuid() != 0)
		eerrorx("%s: you must be root\n", applet);
	अगर (cmd_गिनती != 1) {
		eerror("%s: %s\n", applet, exclusive);
		usage(निकास_त्रुटि);
	}

	अगर (do_cancel) {
		cancel_shutdown();
		निकास(निकास_सफल);
	} अन्यथा अगर (do_reexec) {
		send_cmd("reexec");
		निकास(निकास_सफल);
	} अन्यथा अगर (do_wtmp_only) {
		log_wtmp("shutdown", "~~", 0, RUN_LVL, "~~");
		निकास(निकास_सफल);
	}

	अगर (optind >= argc) {
		eerror("%s: No shutdown time specified", applet);
		usage(निकास_त्रुटि);
	}
	time_arg = argv[optind];
	अगर (*time_arg == '+')
		time_arg++;
	अगर (strcasecmp(time_arg, "now") == 0)
		म_नकल(time_arg, "0");
	क्रम (ch=time_arg; *ch; ch++)
		अगर ((*ch < '0' || *ch > '9') && *ch != ':') {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(निकास_त्रुटि);
		}
	अगर (म_अक्षर(time_arg, ':')) {
		अगर ((माला_पूछो(time_arg, "%2d:%2d", &hour, &min) != 2) ||
				(hour > 23) || (min > 59)) {
			eerror("%s: invalid time %s", applet, time_arg);
			usage(निकास_त्रुटि);
		}
		समय(&tv);
		lt = स_स्थानीय(&tv);
		shutdown_delay = (hour * 60 + min) - (lt->tm_hour * 60 + lt->tm_min);
		अगर (shutdown_delay < 0)
			shutdown_delay += 1440;
	} अन्यथा {
		shutdown_delay = म_से_प(time_arg);
	}

	fp = ख_खोलो(shutdown_pid, "w");
	अगर (!fp)
		eerrorx("%s: fopen `%s': %s", applet, shutdown_pid, म_त्रुटि(त्रुटि_सं));
	ख_लिखो(fp, "%d\n", getpid());
	ख_बंद(fp);

	openlog(applet, LOG_PID, LOG_DAEMON);
	स_रखो(&sa, 0, माप(sa));
	sa.sa_handler = stop_shutdown;
	sigemptyset(&sa.sa_mask);
	sigaction(संक_विघ्न, &sa, शून्य);
	sigaction(संक_इति, &sa, शून्य);
	जबतक (shutdown_delay > 0) {
		अगर (shutdown_delay > 180)
			need_warning = (shutdown_delay % 60 == 0);
		अन्यथा अगर (shutdown_delay > 60)
			need_warning = (shutdown_delay % 30 == 0);
		अन्यथा अगर	(shutdown_delay > 10)
			need_warning = (shutdown_delay % 15 == 0);
		अन्यथा
			need_warning = true;

		अगर (shutdown_delay <= 5)
			create_nologin(shutdown_delay);
		अगर (need_warning) {
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
	अगर (do_halt) {
		अगर (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "HALT");
			sysvinit_runlevel('0');
		} अन्यथा
			send_cmd("halt");
	} अन्यथा अगर (do_kexec)
		send_cmd("kexec");
	अन्यथा अगर (do_poweroff) {
		अगर (access("/run/initctl", F_OK) == 0) {
			sysvinit_setenv("INIT_HALT", "POWEROFF");
			sysvinit_runlevel('0');
		} अन्यथा
			send_cmd("poweroff");
	} अन्यथा अगर (do_reboot) {
		अगर (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('6');
		अन्यथा
			send_cmd("reboot");
	} अन्यथा अगर (do_single) {
		अगर (access("/run/initctl", F_OK) == 0)
			sysvinit_runlevel('S');
		अन्यथा
			send_cmd("single");
	}
	वापस 0;
}
