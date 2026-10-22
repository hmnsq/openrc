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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "debug",     0, ખાલી, 'd' },
	{ "nodeps",     0, ખાલી, 'D' },
	{ "exists",   1, ખાલી, 'e' },
	{ "ifcrashed", 0, ખાલી, 'c' },
	{ "ifexists", 0, ખાલી, 'i' },
	{ "ifinactive", 0, ખાલી, 'I' },
	{ "ifnotstarted", 0, ખાલી, 'N' },
	{ "ifstarted", 0, ખાલી, 's' },
	{ "ifstopped", 0, ખાલી, 'S' },
	{ "list",     0, ખાલી, 'l' },
	{ "resolve",  1, ખાલી, 'r' },
	{ "dry-run",     0, ખાલી, 'Z' },
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	પૂર્ણાંક opt;
	અક્ષર *service;
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

	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
		સ્થિતિ 'd':
			setenv("RC_DEBUG", "yes", 1);
			તોડો;
		સ્થિતિ 'D':
			setenv("RC_NODEPS", "yes", 1);
			તોડો;
		સ્થિતિ 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			મુક્ત(service);
			પાછા opt;
			/* NOTREACHED */
		સ્થિતિ 'c':
			if_crashed = true;
			તોડો;
		સ્થિતિ 'i':
			if_exists = true;
			તોડો;
		સ્થિતિ 'I':
			if_inactive = true;
			તોડો;
		સ્થિતિ 'N':
			if_notstarted = true;
			તોડો;
		સ્થિતિ 'l':
			list = rc_services_in_runlevel(ખાલી);
			જો (TAILQ_FIRST(list) == ખાલી)
				પાછા EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->કિંમત);
			rc_stringlist_free(list);
			પાછા EXIT_SUCCESS;
			/* NOTREACHED */
		સ્થિતિ 'r':
			service = rc_service_resolve(optarg);
			જો (service == ખાલી)
				પાછા EXIT_FAILURE;
			printf("%s\n", service);
			મુક્ત(service);
			પાછા EXIT_SUCCESS;
			/* NOTREACHED */
		સ્થિતિ 's':
			if_started = true;
			તોડો;
		સ્થિતિ 'S':
			if_stopped = true;
			તોડો;
		સ્થિતિ 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			તોડો;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	જો (*argv == ખાલી)
		eerrorx("%s: you need to specify a service", applet);
	જો ((service = rc_service_resolve(*argv)) == ખાલી) {
		જો (if_exists)
			પાછા 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	જો (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		પાછા 0;
	જો (if_inactive && !(state & RC_SERVICE_INACTIVE))
		પાછા 0;
	જો (if_notstarted && (state & RC_SERVICE_STARTED))
		પાછા 0;
	જો (if_started && !(state & RC_SERVICE_STARTED))
		પાછા 0;
	જો (if_stopped && !(state & RC_SERVICE_STOPPED))
		પાછા 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
