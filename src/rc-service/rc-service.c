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

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "debug",     0, تہی, 'd' },
	{ "nodeps",     0, تہی, 'D' },
	{ "exists",   1, تہی, 'e' },
	{ "ifcrashed", 0, تہی, 'c' },
	{ "ifexists", 0, تہی, 'i' },
	{ "ifinactive", 0, تہی, 'I' },
	{ "ifnotstarted", 0, تہی, 'N' },
	{ "ifstarted", 0, تہی, 's' },
	{ "ifstopped", 0, تہی, 'S' },
	{ "list",     0, تہی, 'l' },
	{ "resolve",  1, تہی, 'r' },
	{ "dry-run",     0, تہی, 'Z' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

صحیح مین(صحیح argc, حرف **argv)
{
	صحیح opt;
	حرف *service;
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

	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'd':
			setenv("RC_DEBUG", "yes", 1);
			توڑو;
		حالت 'D':
			setenv("RC_NODEPS", "yes", 1);
			توڑو;
		حالت 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			آزاد(service);
			واپس opt;
			/* NOTREACHED */
		حالت 'c':
			if_crashed = true;
			توڑو;
		حالت 'i':
			if_exists = true;
			توڑو;
		حالت 'I':
			if_inactive = true;
			توڑو;
		حالت 'N':
			if_notstarted = true;
			توڑو;
		حالت 'l':
			list = rc_services_اندر_runlevel(تہی);
			اگر (TAILQ_پہلا(list) == تہی)
				واپس EXIT_FAILURE;
			rc_stringlist_ترتیب_دو(&list);
			TAILQ_FOREACH(s, list, entries)
			    لکھو("%s\n", s->قدر);
			rc_stringlist_free(list);
			واپس EXIT_SUCCESS;
			/* NOTREACHED */
		حالت 'r':
			service = rc_service_resolve(optarg);
			اگر (service == تہی)
				واپس EXIT_FAILURE;
			لکھو("%s\n", service);
			آزاد(service);
			واپس EXIT_SUCCESS;
			/* NOTREACHED */
		حالت 's':
			if_started = true;
			توڑو;
		حالت 'S':
			if_stopped = true;
			توڑو;
		حالت 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			توڑو;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	اگر (*argv == تہی)
		eerrorx("%s: you need to specify a service", applet);
	اگر ((service = rc_service_resolve(*argv)) == تہی) {
		اگر (if_exists)
			واپس 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	اگر (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		واپس 0;
	اگر (if_inactive && !(state & RC_SERVICE_INACTIVE))
		واپس 0;
	اگر (if_notstarted && (state & RC_SERVICE_STARTED))
		واپس 0;
	اگر (if_started && !(state & RC_SERVICE_STARTED))
		واپس 0;
	اگر (if_stopped && !(state & RC_SERVICE_STOPPED))
		واپس 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
