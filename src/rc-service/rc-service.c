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

#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "cdDe:ilr:INsSZ" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "debug",     0, शून्य, 'd' },
	{ "nodeps",     0, शून्य, 'D' },
	{ "exists",   1, शून्य, 'e' },
	{ "ifcrashed", 0, शून्य, 'c' },
	{ "ifexists", 0, शून्य, 'i' },
	{ "ifinactive", 0, शून्य, 'I' },
	{ "ifnotstarted", 0, शून्य, 'N' },
	{ "ifstarted", 0, शून्य, 's' },
	{ "ifstopped", 0, शून्य, 'S' },
	{ "list",     0, शून्य, 'l' },
	{ "resolve",  1, शून्य, 'r' },
	{ "dry-run",     0, शून्य, 'Z' },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
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
स्थिर अक्षर *usagestring = ""
	"Usage: rc-service [options] [-i] <service> <cmd>...\n"
	"   or: rc-service [options] -e <service>\n"
	"   or: rc-service [options] -l\n"
	"   or: rc-service [options] -r <service>";

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	पूर्णांक opt;
	अक्षर *service;
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

	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल 'd':
			setenv("RC_DEBUG", "yes", 1);
			अवरोध;
		हाल 'D':
			setenv("RC_NODEPS", "yes", 1);
			अवरोध;
		हाल 'e':
			service = rc_service_resolve(optarg);
			opt = service ? निकास_सफल : निकास_त्रुटि;
			मुक्त(service);
			वापस opt;
			/* NOTREACHED */
		हाल 'c':
			if_crashed = true;
			अवरोध;
		हाल 'i':
			if_exists = true;
			अवरोध;
		हाल 'I':
			if_inactive = true;
			अवरोध;
		हाल 'N':
			if_notstarted = true;
			अवरोध;
		हाल 'l':
			list = rc_services_in_runlevel(शून्य);
			अगर (TAILQ_FIRST(list) == शून्य)
				वापस निकास_त्रुटि;
			rc_stringlist_sort(&list);
			TAILQ_FOREACH(s, list, entries)
			    म_लिखो("%s\n", s->मूल्य);
			rc_stringlist_free(list);
			वापस निकास_सफल;
			/* NOTREACHED */
		हाल 'r':
			service = rc_service_resolve(optarg);
			अगर (service == शून्य)
				वापस निकास_त्रुटि;
			म_लिखो("%s\n", service);
			मुक्त(service);
			वापस निकास_सफल;
			/* NOTREACHED */
		हाल 's':
			if_started = true;
			अवरोध;
		हाल 'S':
			if_stopped = true;
			अवरोध;
		हाल 'Z':
			setenv("IN_DRYRUN", "yes", 1);
			अवरोध;

		case_RC_COMMON_GETOPT
		}
	}

	argc -= optind;
	argv += optind;
	अगर (*argv == शून्य)
		eerrorx("%s: you need to specify a service", applet);
	अगर ((service = rc_service_resolve(*argv)) == शून्य) {
		अगर (if_exists)
			वापस 0;
		eerrorx("%s: service `%s' does not exist", applet, *argv);
	}
	state = rc_service_state(*argv);
	अगर (if_crashed &&  !(rc_service_daemons_crashed(*argv) && त्रुटि_सं != EACCES))
		वापस 0;
	अगर (if_inactive && !(state & RC_SERVICE_INACTIVE))
		वापस 0;
	अगर (if_notstarted && (state & RC_SERVICE_STARTED))
		वापस 0;
	अगर (if_started && !(state & RC_SERVICE_STARTED))
		वापस 0;
	अगर (if_stopped && !(state & RC_SERVICE_STOPPED))
		वापस 0;
	env_filter();
	env_config();
	*argv = service;
	execv(*argv, argv);
	eerrorx("%s: Failed executing '%s': %s", applet, *argv, म_त्रुटि(त्रुटि_सं));
	/* NOTREACHED */
}
