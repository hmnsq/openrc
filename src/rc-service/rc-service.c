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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "debug",     0, रिक्त, 'd' },
	{ "nodeps",     0, रिक्त, 'D' },
	{ "exists",   1, रिक्त, 'e' },
	{ "ifcrashed", 0, रिक्त, 'c' },
	{ "ifexists", 0, रिक्त, 'i' },
	{ "ifinactive", 0, रिक्त, 'I' },
	{ "ifnotstarted", 0, रिक्त, 'N' },
	{ "ifstarted", 0, रिक्त, 's' },
	{ "ifstopped", 0, रिक्त, 'S' },
	{ "list",     0, रिक्त, 'l' },
	{ "resolve",  1, रिक्त, 'r' },
	{ "dry-run",     0, रिक्त, 'Z' },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
ध्रुव वर्ण *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	पूर्णाङ्क opt;
	वर्ण *service;
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

	याव ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *) 0)) != -1)
	{
		विकल्प (opt) {
		पक्ष 'd':
			setenv("RC_DEBUG", "yes", 1);
			भङ्ग;
		पक्ष 'D':
			setenv("RC_NODEPS", "yes", 1);
			भङ्ग;
		पक्ष 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			मुक्त(service);
			पटिनिवत्त opt;
			/* NOTREACHED */
		पक्ष 'c':
			if_crashed = true;
			भङ्ग;
		पक्ष 'i':
			if_exists = true;
			भङ्ग;
		पक्ष 'I':
			if_inactive = true;
			भङ्ग;
		पक्ष 'N':
			if_notstarted = true;
			भङ्ग;
		पक्ष 'l':
			list = rc_services_in_runlevel(रिक्त);
			सचे (TAILQ_FIRST(list) == रिक्त)
				पटिनिवत्त EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->मानम्);
			rc_stringlist_free(list);
			पटिनिवत्त EXIT_SUCCESS;
			/* NOTREACHED */
		पक्ष 'r':
			service = rc_service_resolve(optarg);
			सचे (service == रिक्त)
				पटिनिवत्त EXIT_FAILURE;
			printf("%s\n", service);
			मुक्त(service);
			पटिनिवत्त EXIT_SUCCESS;
			/* NOTREACHED */
		पक्ष 's':
			if_started = true;
			भङ्ग;
		पक्ष 'S':
			if_stopped = true;
			भङ्ग;
		पक्ष 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			भङ्ग;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	सचे (*argv == रिक्त)
		eerrorx("%s: you need to specify a service", applet);
	सचे ((service = rc_service_resolve(*argv)) == रिक्त) {
		सचे (if_exists)
			पटिनिवत्त 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	सचे (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		पटिनिवत्त 0;
	सचे (if_inactive && !(state & RC_SERVICE_INACTIVE))
		पटिनिवत्त 0;
	सचे (if_notstarted && (state & RC_SERVICE_STARTED))
		पटिनिवत्त 0;
	सचे (if_started && !(state & RC_SERVICE_STARTED))
		पटिनिवत्त 0;
	सचे (if_stopped && !(state & RC_SERVICE_STOPPED))
		पटिनिवत्त 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
