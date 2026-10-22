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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "debug",     0, ਨਿਰਾ, 'd' },
	{ "nodeps",     0, ਨਿਰਾ, 'D' },
	{ "exists",   1, ਨਿਰਾ, 'e' },
	{ "ifcrashed", 0, ਨਿਰਾ, 'c' },
	{ "ifexists", 0, ਨਿਰਾ, 'i' },
	{ "ifinactive", 0, ਨਿਰਾ, 'I' },
	{ "ifnotstarted", 0, ਨਿਰਾ, 'N' },
	{ "ifstarted", 0, ਨਿਰਾ, 's' },
	{ "ifstopped", 0, ਨਿਰਾ, 'S' },
	{ "list",     0, ਨਿਰਾ, 'l' },
	{ "resolve",  1, ਨਿਰਾ, 'r' },
	{ "dry-run",     0, ਨਿਰਾ, 'Z' },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਪੂਰਨ opt;
	ਅੱਖਰ *service;
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

	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ 'd':
			setenv("RC_DEBUG", "yes", 1);
			ਰੋਕੋ;
		ਹਾਲਤ 'D':
			setenv("RC_NODEPS", "yes", 1);
			ਰੋਕੋ;
		ਹਾਲਤ 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			ਮੁਕਤ(service);
			ਵਾਪਸ opt;
			/* NOTREACHED */
		ਹਾਲਤ 'c':
			if_crashed = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'i':
			if_exists = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'I':
			if_inactive = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'N':
			if_notstarted = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'l':
			list = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
			ਜੇ (TAILQ_ਪਹਿਲਾ(list) == ਨਿਰਾ)
				ਵਾਪਸ EXIT_FAILURE;
			rc_stringlist_ਛਾਂਟੋ(&list);
			TAILQ_FOREACH(s, list, entries)
			    ਲਿਖੋ("%s\n", s->ਮੁੱਲ);
			rc_stringlist_free(list);
			ਵਾਪਸ EXIT_SUCCESS;
			/* NOTREACHED */
		ਹਾਲਤ 'r':
			service = rc_service_resolve(optarg);
			ਜੇ (service == ਨਿਰਾ)
				ਵਾਪਸ EXIT_FAILURE;
			ਲਿਖੋ("%s\n", service);
			ਮੁਕਤ(service);
			ਵਾਪਸ EXIT_SUCCESS;
			/* NOTREACHED */
		ਹਾਲਤ 's':
			if_started = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'S':
			if_stopped = true;
			ਰੋਕੋ;
		ਹਾਲਤ 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	ਜੇ (*argv == ਨਿਰਾ)
		eerrorx("%s: you need to specify a service", applet);
	ਜੇ ((service = rc_service_resolve(*argv)) == ਨਿਰਾ) {
		ਜੇ (if_exists)
			ਵਾਪਸ 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	ਜੇ (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		ਵਾਪਸ 0;
	ਜੇ (if_inactive && !(state & RC_SERVICE_INACTIVE))
		ਵਾਪਸ 0;
	ਜੇ (if_notstarted && (state & RC_SERVICE_STARTED))
		ਵਾਪਸ 0;
	ਜੇ (if_started && !(state & RC_SERVICE_STARTED))
		ਵਾਪਸ 0;
	ਜੇ (if_stopped && !(state & RC_SERVICE_STOPPED))
		ਵਾਪਸ 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
