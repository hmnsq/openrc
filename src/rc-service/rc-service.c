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

#شامل <errno.h>
#شامل <getopt.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "debug",     0, پوچ, 'd' },
	{ "nodeps",     0, پوچ, 'D' },
	{ "exists",   1, پوچ, 'e' },
	{ "ifcrashed", 0, پوچ, 'c' },
	{ "ifexists", 0, پوچ, 'i' },
	{ "ifinactive", 0, پوچ, 'I' },
	{ "ifnotstarted", 0, پوچ, 'N' },
	{ "ifstarted", 0, پوچ, 's' },
	{ "ifstopped", 0, پوچ, 'S' },
	{ "list",     0, پوچ, 'l' },
	{ "resolve",  1, پوچ, 'r' },
	{ "dry-run",     0, پوچ, 'Z' },
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

صحیح اصلی(صحیح argc, نویسه **argv)
{
	صحیح opt;
	نویسه *service;
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

	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'd':
			setenv("RC_DEBUG", "yes", 1);
			بشکن;
		حالت 'D':
			setenv("RC_NODEPS", "yes", 1);
			بشکن;
		حالت 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			آزاد(service);
			بازگشت opt;
			/* NOTREACHED */
		حالت 'c':
			if_crashed = true;
			بشکن;
		حالت 'i':
			if_exists = true;
			بشکن;
		حالت 'I':
			if_inactive = true;
			بشکن;
		حالت 'N':
			if_notstarted = true;
			بشکن;
		حالت 'l':
			list = rc_services_in_runlevel(پوچ);
			اگر (TAILQ_FIRST(list) == پوچ)
				بازگشت EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->مقدار);
			rc_stringlist_free(list);
			بازگشت EXIT_SUCCESS;
			/* NOTREACHED */
		حالت 'r':
			service = rc_service_resolve(optarg);
			اگر (service == پوچ)
				بازگشت EXIT_FAILURE;
			printf("%s\n", service);
			آزاد(service);
			بازگشت EXIT_SUCCESS;
			/* NOTREACHED */
		حالت 's':
			if_started = true;
			بشکن;
		حالت 'S':
			if_stopped = true;
			بشکن;
		حالت 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			بشکن;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	اگر (*argv == پوچ)
		eerrorx("%s: you need to specify a service", applet);
	اگر ((service = rc_service_resolve(*argv)) == پوچ) {
		اگر (if_exists)
			بازگشت 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	اگر (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		بازگشت 0;
	اگر (if_inactive && !(state & RC_SERVICE_INACTIVE))
		بازگشت 0;
	اگر (if_notstarted && (state & RC_SERVICE_STARTED))
		بازگشت 0;
	اگر (if_started && !(state & RC_SERVICE_STARTED))
		بازگشت 0;
	اگر (if_stopped && !(state & RC_SERVICE_STOPPED))
		بازگشت 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
