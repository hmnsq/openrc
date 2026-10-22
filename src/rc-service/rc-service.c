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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "debug",     0, ಖಾಲಿ, 'd' },
	{ "nodeps",     0, ಖಾಲಿ, 'D' },
	{ "exists",   1, ಖಾಲಿ, 'e' },
	{ "ifcrashed", 0, ಖಾಲಿ, 'c' },
	{ "ifexists", 0, ಖಾಲಿ, 'i' },
	{ "ifinactive", 0, ಖಾಲಿ, 'I' },
	{ "ifnotstarted", 0, ಖಾಲಿ, 'N' },
	{ "ifstarted", 0, ಖಾಲಿ, 's' },
	{ "ifstopped", 0, ಖಾಲಿ, 'S' },
	{ "list",     0, ಖಾಲಿ, 'l' },
	{ "resolve",  1, ಖಾಲಿ, 'r' },
	{ "dry-run",     0, ಖಾಲಿ, 'Z' },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ opt;
	ಅಕ್ಷರ *service;
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

	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
	{
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'd':
			setenv("RC_DEBUG", "yes", 1);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'D':
			setenv("RC_NODEPS", "yes", 1);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			ಬಿಡುಗಡೆ(service);
			ಹಿಂತಿರುಗಿ opt;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'c':
			if_crashed = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'i':
			if_exists = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'I':
			if_inactive = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'N':
			if_notstarted = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'l':
			list = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
			ಆದರೆ (TAILQ_ಮೊದಲ(list) == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			rc_stringlist_ವಿಂಗಡಿಸು(&list);
			TAILQ_FOREACH(s, list, entries)
			    ಬರೆ("%s\n", s->ಮೌಲ್ಯ);
			rc_stringlist_free(list);
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'r':
			service = rc_service_resolve(optarg);
			ಆದರೆ (service == ಖಾಲಿ)
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			ಬರೆ("%s\n", service);
			ಬಿಡುಗಡೆ(service);
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 's':
			if_started = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'S':
			if_stopped = true;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			ನಿಲ್ಲಿಸು;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	ಆದರೆ (*argv == ಖಾಲಿ)
		eerrorx("%s: you need to specify a service", applet);
	ಆದರೆ ((service = rc_service_resolve(*argv)) == ಖಾಲಿ) {
		ಆದರೆ (if_exists)
			ಹಿಂತಿರುಗಿ 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	ಆದರೆ (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (if_inactive && !(state & RC_SERVICE_INACTIVE))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (if_notstarted && (state & RC_SERVICE_STARTED))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (if_started && !(state & RC_SERVICE_STARTED))
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (if_stopped && !(state & RC_SERVICE_STOPPED))
		ಹಿಂತಿರುಗಿ 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
