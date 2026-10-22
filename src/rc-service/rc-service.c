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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "debug",     0, வெற்று, 'd' },
	{ "nodeps",     0, வெற்று, 'D' },
	{ "exists",   1, வெற்று, 'e' },
	{ "ifcrashed", 0, வெற்று, 'c' },
	{ "ifexists", 0, வெற்று, 'i' },
	{ "ifinactive", 0, வெற்று, 'I' },
	{ "ifnotstarted", 0, வெற்று, 'N' },
	{ "ifstarted", 0, வெற்று, 's' },
	{ "ifstopped", 0, வெற்று, 'S' },
	{ "list",     0, வெற்று, 'l' },
	{ "resolve",  1, வெற்று, 'r' },
	{ "dry-run",     0, வெற்று, 'Z' },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	முழு opt;
	எழுத்து *service;
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

	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை 'd':
			setenv("RC_DEBUG", "yes", 1);
			நிறுத்து;
		நிலை 'D':
			setenv("RC_NODEPS", "yes", 1);
			நிறுத்து;
		நிலை 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			விடுவி(service);
			திரும்பு opt;
			/* NOTREACHED */
		நிலை 'c':
			if_crashed = true;
			நிறுத்து;
		நிலை 'i':
			if_exists = true;
			நிறுத்து;
		நிலை 'I':
			if_inactive = true;
			நிறுத்து;
		நிலை 'N':
			if_notstarted = true;
			நிறுத்து;
		நிலை 'l':
			list = rc_services_in_runlevel(வெற்று);
			எனில் (TAILQ_FIRST(list) == வெற்று)
				திரும்பு EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->மதிப்பு);
			rc_stringlist_free(list);
			திரும்பு EXIT_SUCCESS;
			/* NOTREACHED */
		நிலை 'r':
			service = rc_service_resolve(optarg);
			எனில் (service == வெற்று)
				திரும்பு EXIT_FAILURE;
			printf("%s\n", service);
			விடுவி(service);
			திரும்பு EXIT_SUCCESS;
			/* NOTREACHED */
		நிலை 's':
			if_started = true;
			நிறுத்து;
		நிலை 'S':
			if_stopped = true;
			நிறுத்து;
		நிலை 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	எனில் (*argv == வெற்று)
		eerrorx("%s: you need to specify a service", applet);
	எனில் ((service = rc_service_resolve(*argv)) == வெற்று) {
		எனில் (if_exists)
			திரும்பு 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	எனில் (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		திரும்பு 0;
	எனில் (if_inactive && !(state & RC_SERVICE_INACTIVE))
		திரும்பு 0;
	எனில் (if_notstarted && (state & RC_SERVICE_STARTED))
		திரும்பு 0;
	எனில் (if_started && !(state & RC_SERVICE_STARTED))
		திரும்பு 0;
	எனில் (if_stopped && !(state & RC_SERVICE_STOPPED))
		திரும்பு 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
