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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "debug",     0, ఖాళీ, 'd' },
	{ "nodeps",     0, ఖాళీ, 'D' },
	{ "exists",   1, ఖాళీ, 'e' },
	{ "ifcrashed", 0, ఖాళీ, 'c' },
	{ "ifexists", 0, ఖాళీ, 'i' },
	{ "ifinactive", 0, ఖాళీ, 'I' },
	{ "ifnotstarted", 0, ఖాళీ, 'N' },
	{ "ifstarted", 0, ఖాళీ, 's' },
	{ "ifstopped", 0, ఖాళీ, 'S' },
	{ "list",     0, ఖాళీ, 'l' },
	{ "resolve",  1, ఖాళీ, 'r' },
	{ "dry-run",     0, ఖాళీ, 'Z' },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిరరాశి అక్షరం *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	పూర్ణాంకం opt;
	అక్షరం *service;
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

	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి 'd':
			setenv("RC_DEBUG", "yes", 1);
			ఆపు;
		స్థితి 'D':
			setenv("RC_NODEPS", "yes", 1);
			ఆపు;
		స్థితి 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			విడుదల(service);
			తిరిగి opt;
			/* NOTREACHED */
		స్థితి 'c':
			if_crashed = true;
			ఆపు;
		స్థితి 'i':
			if_exists = true;
			ఆపు;
		స్థితి 'I':
			if_inactive = true;
			ఆపు;
		స్థితి 'N':
			if_notstarted = true;
			ఆపు;
		స్థితి 'l':
			list = rc_services_లోపల_runlevel(ఖాళీ);
			ఒకవేళ (TAILQ_మొదటి(list) == ఖాళీ)
				తిరిగి EXIT_FAILURE;
			rc_stringlist_క్రమీకరించు(&list);
			TAILQ_FOREACH(s, list, entries)
			    రాయి("%s\n", s->విలువ);
			rc_stringlist_free(list);
			తిరిగి EXIT_SUCCESS;
			/* NOTREACHED */
		స్థితి 'r':
			service = rc_service_resolve(optarg);
			ఒకవేళ (service == ఖాళీ)
				తిరిగి EXIT_FAILURE;
			రాయి("%s\n", service);
			విడుదల(service);
			తిరిగి EXIT_SUCCESS;
			/* NOTREACHED */
		స్థితి 's':
			if_started = true;
			ఆపు;
		స్థితి 'S':
			if_stopped = true;
			ఆపు;
		స్థితి 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			ఆపు;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	ఒకవేళ (*argv == ఖాళీ)
		eerrorx("%s: you need to specify a service", applet);
	ఒకవేళ ((service = rc_service_resolve(*argv)) == ఖాళీ) {
		ఒకవేళ (if_exists)
			తిరిగి 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	ఒకవేళ (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		తిరిగి 0;
	ఒకవేళ (if_inactive && !(state & RC_SERVICE_INACTIVE))
		తిరిగి 0;
	ఒకవేళ (if_notstarted && (state & RC_SERVICE_STARTED))
		తిరిగి 0;
	ఒకవేళ (if_started && !(state & RC_SERVICE_STARTED))
		తిరిగి 0;
	ఒకవేళ (if_stopped && !(state & RC_SERVICE_STOPPED))
		తిరిగి 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
