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

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "_usage.h"
#include "helpers.h"

const char *applet = NULL;
const char *extraopts = NULL;
const char getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
const struct option longopts[] = {
	{ "debug",     0, NULL, 'd' },
	{ "nodeps",     0, NULL, 'D' },
	{ "exists",   1, NULL, 'e' },
	{ "ifcrashed", 0, NULL, 'c' },
	{ "ifexists", 0, NULL, 'i' },
	{ "ifinactive", 0, NULL, 'I' },
	{ "ifnotstarted", 0, NULL, 'N' },
	{ "ifstarted", 0, NULL, 's' },
	{ "ifstopped", 0, NULL, 'S' },
	{ "list",     0, NULL, 'l' },
	{ "resolve",  1, NULL, 'r' },
	{ "dry-run",     0, NULL, 'Z' },
	longopts_COMMON
};
const char * const longopts_help[] = {
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
const char *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

整数 主要(整数 argc, char **argv)
{
	整数 opt;
	char *service;
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

	当 ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (整数 *) 0)) != -1)
	{
		switch (opt) {
		case 'd':
			setenv("RC_DEBUG", "yes", 1);
			break;
		case 'D':
			setenv("RC_NODEPS", "yes", 1);
			break;
		case 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			free(service);
			返回 opt;
			/* NOTREACHED */
		case 'c':
			if_crashed = true;
			break;
		case 'i':
			if_exists = true;
			break;
		case 'I':
			if_inactive = true;
			break;
		case 'N':
			if_notstarted = true;
			break;
		case 'l':
			list = rc_services_in_runlevel(NULL);
			如果 (TAILQ_FIRST(list) == NULL)
				返回 EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->value);
			rc_stringlist_free(list);
			返回 EXIT_SUCCESS;
			/* NOTREACHED */
		case 'r':
			service = rc_service_resolve(optarg);
			如果 (service == NULL)
				返回 EXIT_FAILURE;
			printf("%s\n", service);
			free(service);
			返回 EXIT_SUCCESS;
			/* NOTREACHED */
		case 's':
			if_started = true;
			break;
		case 'S':
			if_stopped = true;
			break;
		case 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			break;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	如果 (*argv == NULL)
		eerrorx("%s: you need to specify a service", applet);
	如果 ((service = rc_service_resolve(*argv)) == NULL) {
		如果 (if_exists)
			返回 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	如果 (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		返回 0;
	如果 (if_inactive && !(state & RC_SERVICE_INACTIVE))
		返回 0;
	如果 (if_notstarted && (state & RC_SERVICE_STARTED))
		返回 0;
	如果 (if_started && !(state & RC_SERVICE_STARTED))
		返回 0;
	如果 (if_stopped && !(state & RC_SERVICE_STOPPED))
		返回 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
