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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

ধ্রুব বর্ণ *applet = রিক্ত;
ধ্রুব বর্ণ *extraopts = রিক্ত;
ধ্রুব বর্ণ getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{ "debug",     0, রিক্ত, 'd' },
	{ "nodeps",     0, রিক্ত, 'D' },
	{ "exists",   1, রিক্ত, 'e' },
	{ "ifcrashed", 0, রিক্ত, 'c' },
	{ "ifexists", 0, রিক্ত, 'i' },
	{ "ifinactive", 0, রিক্ত, 'I' },
	{ "ifnotstarted", 0, রিক্ত, 'N' },
	{ "ifstarted", 0, রিক্ত, 's' },
	{ "ifstopped", 0, রিক্ত, 'S' },
	{ "list",     0, রিক্ত, 'l' },
	{ "resolve",  1, রিক্ত, 'r' },
	{ "dry-run",     0, রিক্ত, 'Z' },
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
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
ধ্রুব বর্ণ *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	পূর্ণাঙ্ক opt;
	বর্ণ *service;
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

	যাবত্ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *) 0)) != -1)
	{
		বিকল্প (opt) {
		পক্ষ 'd':
			setenv("RC_DEBUG", "yes", 1);
			ভঙ্গ;
		পক্ষ 'D':
			setenv("RC_NODEPS", "yes", 1);
			ভঙ্গ;
		পক্ষ 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			মুক্ত(service);
			প্রত্যাগম opt;
			/* NOTREACHED */
		পক্ষ 'c':
			if_crashed = true;
			ভঙ্গ;
		পক্ষ 'i':
			if_exists = true;
			ভঙ্গ;
		পক্ষ 'I':
			if_inactive = true;
			ভঙ্গ;
		পক্ষ 'N':
			if_notstarted = true;
			ভঙ্গ;
		পক্ষ 'l':
			list = rc_services_in_runlevel(রিক্ত);
			যদি (TAILQ_FIRST(list) == রিক্ত)
				প্রত্যাগম EXIT_FAILURE;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    printf("%s\n", s->মান);
			rc_stringlist_free(list);
			প্রত্যাগম EXIT_SUCCESS;
			/* NOTREACHED */
		পক্ষ 'r':
			service = rc_service_resolve(optarg);
			যদি (service == রিক্ত)
				প্রত্যাগম EXIT_FAILURE;
			printf("%s\n", service);
			মুক্ত(service);
			প্রত্যাগম EXIT_SUCCESS;
			/* NOTREACHED */
		পক্ষ 's':
			if_started = true;
			ভঙ্গ;
		পক্ষ 'S':
			if_stopped = true;
			ভঙ্গ;
		পক্ষ 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			ভঙ্গ;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	যদি (*argv == রিক্ত)
		eerrorx("%s: you need to specify a service", applet);
	যদি ((service = rc_service_resolve(*argv)) == রিক্ত) {
		যদি (if_exists)
			প্রত্যাগম 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	যদি (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		প্রত্যাগম 0;
	যদি (if_inactive && !(state & RC_SERVICE_INACTIVE))
		প্রত্যাগম 0;
	যদি (if_notstarted && (state & RC_SERVICE_STARTED))
		প্রত্যাগম 0;
	যদি (if_started && !(state & RC_SERVICE_STARTED))
		প্রত্যাগম 0;
	যদি (if_stopped && !(state & RC_SERVICE_STOPPED))
		প্রত্যাগম 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
