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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{ "debug",     0, ശൂന്യ, 'd' },
	{ "nodeps",     0, ശൂന്യ, 'D' },
	{ "exists",   1, ശൂന്യ, 'e' },
	{ "ifcrashed", 0, ശൂന്യ, 'c' },
	{ "ifexists", 0, ശൂന്യ, 'i' },
	{ "ifinactive", 0, ശൂന്യ, 'I' },
	{ "ifnotstarted", 0, ശൂന്യ, 'N' },
	{ "ifstarted", 0, ശൂന്യ, 's' },
	{ "ifstopped", 0, ശൂന്യ, 'S' },
	{ "list",     0, ശൂന്യ, 'l' },
	{ "resolve",  1, ശൂന്യ, 'r' },
	{ "dry-run",     0, ശൂന്യ, 'Z' },
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ opt;
	അക്ഷരം *service;
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

	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (പൂർണ്ണസംഖ്യ *) 0)) != -1)
	{
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'd':
			setenv("RC_DEBUG", "yes", 1);
			നിർത്തുക;
		അവസ്ഥ 'D':
			setenv("RC_NODEPS", "yes", 1);
			നിർത്തുക;
		അവസ്ഥ 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			സ്വതന്ത്രം(service);
			തിരികെ opt;
			/* NOTREACHED */
		അവസ്ഥ 'c':
			if_crashed = true;
			നിർത്തുക;
		അവസ്ഥ 'i':
			if_exists = true;
			നിർത്തുക;
		അവസ്ഥ 'I':
			if_inactive = true;
			നിർത്തുക;
		അവസ്ഥ 'N':
			if_notstarted = true;
			നിർത്തുക;
		അവസ്ഥ 'l':
			list = rc_services_in_runlevel(ശൂന്യ);
			എങ്കിൽ (TAILQ_FIRST(list) == ശൂന്യ)
				തിരികെ EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->മൂല്യം);
			rc_stringlist_free(list);
			തിരികെ EXIT_SUCCESS;
			/* NOTREACHED */
		അവസ്ഥ 'r':
			service = rc_service_resolve(optarg);
			എങ്കിൽ (service == ശൂന്യ)
				തിരികെ EXIT_FAILURE;
			printf("%s\n", service);
			സ്വതന്ത്രം(service);
			തിരികെ EXIT_SUCCESS;
			/* NOTREACHED */
		അവസ്ഥ 's':
			if_started = true;
			നിർത്തുക;
		അവസ്ഥ 'S':
			if_stopped = true;
			നിർത്തുക;
		അവസ്ഥ 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			നിർത്തുക;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	എങ്കിൽ (*argv == ശൂന്യ)
		eerrorx("%s: you need to specify a service", applet);
	എങ്കിൽ ((service = rc_service_resolve(*argv)) == ശൂന്യ) {
		എങ്കിൽ (if_exists)
			തിരികെ 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	എങ്കിൽ (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		തിരികെ 0;
	എങ്കിൽ (if_inactive && !(state & RC_SERVICE_INACTIVE))
		തിരികെ 0;
	എങ്കിൽ (if_notstarted && (state & RC_SERVICE_STARTED))
		തിരികെ 0;
	എങ്കിൽ (if_started && !(state & RC_SERVICE_STARTED))
		തിരികെ 0;
	എങ്കിൽ (if_stopped && !(state & RC_SERVICE_STOPPED))
		തിരികെ 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
