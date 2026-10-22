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

#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "debug",     0, عدم, 'd' },
	{ "nodeps",     0, عدم, 'D' },
	{ "exists",   1, عدم, 'e' },
	{ "ifcrashed", 0, عدم, 'c' },
	{ "ifexists", 0, عدم, 'i' },
	{ "ifinactive", 0, عدم, 'I' },
	{ "ifnotstarted", 0, عدم, 'N' },
	{ "ifstarted", 0, عدم, 's' },
	{ "ifstopped", 0, عدم, 'S' },
	{ "list",     0, عدم, 'l' },
	{ "resolve",  1, عدم, 'r' },
	{ "dry-run",     0, عدم, 'Z' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

صحيح مکھ(صحيح argc, حرف **argv)
{
	صحيح opt;
	حرف *service;
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

	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'd':
			setenv("RC_DEBUG", "yes", 1);
			اكسر;
		حالة 'D':
			setenv("RC_NODEPS", "yes", 1);
			اكسر;
		حالة 'e':
			service = rc_service_resolve(optarg);
			opt = service ? EXIT_SUCCESS : EXIT_FAILURE;
			حرر(service);
			أرجع opt;
			/* NOTREACHED */
		حالة 'c':
			if_crashed = true;
			اكسر;
		حالة 'i':
			if_exists = true;
			اكسر;
		حالة 'I':
			if_inactive = true;
			اكسر;
		حالة 'N':
			if_notstarted = true;
			اكسر;
		حالة 'l':
			list = rc_services_وچ_runlevel(عدم);
			إذا (TAILQ_پہلا(list) == عدم)
				أرجع EXIT_FAILURE;
			rc_stringlist_ترتیب_دیو(&list);
			TAILQ_FOREACH(s, list, entries)
			    لکھو("%s\n", s->قدر);
			rc_stringlist_free(list);
			أرجع EXIT_SUCCESS;
			/* NOTREACHED */
		حالة 'r':
			service = rc_service_resolve(optarg);
			إذا (service == عدم)
				أرجع EXIT_FAILURE;
			لکھو("%s\n", service);
			حرر(service);
			أرجع EXIT_SUCCESS;
			/* NOTREACHED */
		حالة 's':
			if_started = true;
			اكسر;
		حالة 'S':
			if_stopped = true;
			اكسر;
		حالة 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	إذا (*argv == عدم)
		eerrorx("%s: you need to specify a service", applet);
	إذا ((service = rc_service_resolve(*argv)) == عدم) {
		إذا (if_exists)
			أرجع 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	إذا (if_crashed &&  !(rc_service_daemons_crashed(*argv) && errno != EACCES))
		أرجع 0;
	إذا (if_inactive && !(state & RC_SERVICE_INACTIVE))
		أرجع 0;
	إذا (if_notstarted && (state & RC_SERVICE_STARTED))
		أرجع 0;
	إذا (if_started && !(state & RC_SERVICE_STARTED))
		أرجع 0;
	إذا (if_stopped && !(state & RC_SERVICE_STOPPED))
		أرجع 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, strerror(errno));
	/* NOTREACHED */
}
