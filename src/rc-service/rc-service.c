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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "debug",     0, ଖାଲି, 'd' },
	{ "nodeps",     0, ଖାଲି, 'D' },
	{ "exists",   1, ଖାଲି, 'e' },
	{ "ifcrashed", 0, ଖାଲି, 'c' },
	{ "ifexists", 0, ଖାଲି, 'i' },
	{ "ifinactive", 0, ଖାଲି, 'I' },
	{ "ifnotstarted", 0, ଖାଲି, 'N' },
	{ "ifstarted", 0, ଖାଲି, 's' },
	{ "ifstopped", 0, ଖାଲି, 'S' },
	{ "list",     0, ଖାଲି, 'l' },
	{ "resolve",  1, ଖାଲି, 'r' },
	{ "dry-run",     0, ଖାଲି, 'Z' },
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ଅକ୍ଷର *service;
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

	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
		ସ୍ଥିତି 'd':
			setenv("RC_DEBUG", "yes", 1);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'D':
			setenv("RC_NODEPS", "yes", 1);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			ମୁକ୍ତ(service);
			ଫେରନ୍ତୁ opt;
			/* NOTREACHED */
		ସ୍ଥିତି 'c':
			if_crashed = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'i':
			if_exists = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'I':
			if_inactive = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'N':
			if_notstarted = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'l':
			list = rc_services_in_runlevel(ଖାଲି);
			ଯଦି (TAILQ_FIRST(list) == ଖାଲି)
				ଫେରନ୍ତୁ EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->ମୂଲ୍ୟ);
			rc_stringlist_free(list);
			ଫେରନ୍ତୁ EXIT_SUCCESS;
			/* NOTREACHED */
		ସ୍ଥିତି 'r':
			service = rc_service_resolve(optarg);
			ଯଦି (service == ଖାଲି)
				ଫେରନ୍ତୁ EXIT_FAILURE;
			printf("%s\n", service);
			ମୁକ୍ତ(service);
			ଫେରନ୍ତୁ EXIT_SUCCESS;
			/* NOTREACHED */
		ସ୍ଥିତି 's':
			if_started = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'S':
			if_stopped = true;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	ଯଦି (*argv == ଖାଲି)
		eerrorx("%s: you need to specify a service", applet);
	ଯଦି ((service = rc_service_resolve(*argv)) == ଖାଲି) {
		ଯଦି (if_exists)
			ଫେରନ୍ତୁ 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	ଯଦି (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		ଫେରନ୍ତୁ 0;
	ଯଦି (if_inactive && !(state & RC_SERVICE_INACTIVE))
		ଫେରନ୍ତୁ 0;
	ଯଦି (if_notstarted && (state & RC_SERVICE_STARTED))
		ଫେରନ୍ତୁ 0;
	ଯଦି (if_started && !(state & RC_SERVICE_STARTED))
		ଫେରନ୍ତୁ 0;
	ଯଦି (if_stopped && !(state & RC_SERVICE_STOPPED))
		ଫେରନ୍ତୁ 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
