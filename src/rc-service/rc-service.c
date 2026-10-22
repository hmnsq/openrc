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

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "debug",     0, ರಿಕ್ತ, 'd' },
	{ "nodeps",     0, ರಿಕ್ತ, 'D' },
	{ "exists",   1, ರಿಕ್ತ, 'e' },
	{ "ifcrashed", 0, ರಿಕ್ತ, 'c' },
	{ "ifexists", 0, ರಿಕ್ತ, 'i' },
	{ "ifinactive", 0, ರಿಕ್ತ, 'I' },
	{ "ifnotstarted", 0, ರಿಕ್ತ, 'N' },
	{ "ifstarted", 0, ರಿಕ್ತ, 's' },
	{ "ifstopped", 0, ರಿಕ್ತ, 'S' },
	{ "list",     0, ರಿಕ್ತ, 'l' },
	{ "resolve",  1, ರಿಕ್ತ, 'r' },
	{ "dry-run",     0, ರಿಕ್ತ, 'Z' },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಧ್ರುವ ವರ್ಣ *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	ಪೂರ್ಣಾಙ್ಕ opt;
	ವರ್ಣ *service;
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

	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
	{
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'd':
			setenv("RC_DEBUG", "yes", 1);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'D':
			setenv("RC_NODEPS", "yes", 1);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			ಮುಕ್ತ(service);
			ಪ್ರತ್ಯಾಗಮ opt;
			/* NOTREACHED */
		ಪಕ್ಷ 'c':
			if_crashed = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'i':
			if_exists = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'I':
			if_inactive = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'N':
			if_notstarted = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'l':
			list = rc_services_in_runlevel(ರಿಕ್ತ);
			ಯದಿ (TAILQ_FIRST(list) == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->ಮೌಲ್ಯ);
			rc_stringlist_free(list);
			ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
			/* NOTREACHED */
		ಪಕ್ಷ 'r':
			service = rc_service_resolve(optarg);
			ಯದಿ (service == ರಿಕ್ತ)
				ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
			printf("%s\n", service);
			ಮುಕ್ತ(service);
			ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
			/* NOTREACHED */
		ಪಕ್ಷ 's':
			if_started = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'S':
			if_stopped = true;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			ಭಙ್ಗ;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	ಯದಿ (*argv == ರಿಕ್ತ)
		eerrorx("%s: you need to specify a service", applet);
	ಯದಿ ((service = rc_service_resolve(*argv)) == ರಿಕ್ತ) {
		ಯದಿ (if_exists)
			ಪ್ರತ್ಯಾಗಮ 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	ಯದಿ (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (if_inactive && !(state & RC_SERVICE_INACTIVE))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (if_notstarted && (state & RC_SERVICE_STARTED))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (if_started && !(state & RC_SERVICE_STARTED))
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (if_stopped && !(state & RC_SERVICE_STOPPED))
		ಪ್ರತ್ಯಾಗಮ 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
