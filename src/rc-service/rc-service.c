/*
 * rc-service.c
 * Finds all OpenRC services
 */

/*
 * Copyright (c) 2008-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = ফাঁকা;
ধ্রুবক অক্ষর getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{ "debug",     0, ফাঁকা, 'd' },
	{ "nodeps",     0, ফাঁকা, 'D' },
	{ "exists",   1, ফাঁকা, 'e' },
	{ "ifcrashed", 0, ফাঁকা, 'c' },
	{ "ifexists", 0, ফাঁকা, 'i' },
	{ "ifinactive", 0, ফাঁকা, 'I' },
	{ "ifnotstarted", 0, ফাঁকা, 'N' },
	{ "ifstarted", 0, ফাঁকা, 's' },
	{ "ifstopped", 0, ফাঁকা, 'S' },
	{ "list",     0, ফাঁকা, 'l' },
	{ "resolve",  1, ফাঁকা, 'r' },
	{ "dry-run",     0, ফাঁকা, 'Z' },
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
	"set xtrace when running the command",
	"ignore dependencies",
	"tests if the service exists or not",
	"if the service is crashed run the command",
	"if the service exists run the command",
	"if the service is inactive run the command",
	"if the service is not started run the command",
	"if the service is started run the command",
	"if the service is stopped run the command",
	"list all available services",
	"resolve the service name to an init script",
	"dry run (show what would happen)",
	longopts_help_COMMON
};
ধ্রুবক অক্ষর *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	পূর্ণসংখ্যা opt;
	অক্ষর *service;
	RC_STRINGLIST *list;
	RC_STRING *s;
	RC_SERVICE state;
	bool if_crashed = false;
	bool if_exists = false;
	bool if_inactive = false;
	bool if_notstarted = false;
	bool if_started = false;
	bool if_stopped = false;

	applet = basename_c(argv[0]);
	/* Ensure that we are only quiet when explicitly told to be */
	unsetenv("EINFO_QUIET");

	যখন ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণসংখ্যা *) 0)) != -1)
	{
		নির্বাচন (opt) {
		ক্ষেত্র 'd':
			setenv("RC_DEBUG", "yes", 1);
			বিরতি;
		ক্ষেত্র 'D':
			setenv("RC_NODEPS", "yes", 1);
			বিরতি;
		ক্ষেত্র 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			মুক্ত(service);
			ফেরত opt;
			/* NOTREACHED */
		ক্ষেত্র 'c':
			if_crashed = true;
			বিরতি;
		ক্ষেত্র 'i':
			if_exists = true;
			বিরতি;
		ক্ষেত্র 'I':
			if_inactive = true;
			বিরতি;
		ক্ষেত্র 'N':
			if_notstarted = true;
			বিরতি;
		ক্ষেত্র 'l':
			list = rc_services_in_runlevel(ফাঁকা);
			যদি (TAILQ_FIRST(list) == ফাঁকা)
				ফেরত EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->মান);
			rc_stringlist_free(list);
			ফেরত EXIT_SUCCESS;
			/* NOTREACHED */
		ক্ষেত্র 'r':
			service = rc_service_resolve(optarg);
			যদি (service == ফাঁকা)
				ফেরত EXIT_FAILURE;
			printf("%s\n", service);
			মুক্ত(service);
			ফেরত EXIT_SUCCESS;
			/* NOTREACHED */
		ক্ষেত্র 's':
			if_started = true;
			বিরতি;
		ক্ষেত্র 'S':
			if_stopped = true;
			বিরতি;
		ক্ষেত্র 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			বিরতি;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	যদি (*argv == ফাঁকা)
		eerrorx("%s: you need to specify a service", applet);
	যদি ((service = rc_service_resolve(*argv)) == ফাঁকা) {
		যদি (if_exists)
			ফেরত 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	যদি (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		ফেরত 0;
	যদি (if_inactive && !(state & RC_SERVICE_INACTIVE))
		ফেরত 0;
	যদি (if_notstarted && (state & RC_SERVICE_STARTED))
		ফেরত 0;
	যদি (if_started && !(state & RC_SERVICE_STARTED))
		ফেরত 0;
	যদি (if_stopped && !(state & RC_SERVICE_STOPPED))
		ফেরত 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
