/*
 * rc-depend
 * rc service dependency and ordering
 */

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

स्थिरांक अक्षर *applet = रिक्त;
स्थिरांक अक्षर *extraopts = रिक्त;
स्थिरांक अक्षर getoptstring[] = "aot:suTF:" getoptstring_COMMON;
स्थिरांक संरचना option longopts[] = {
	{ "starting", 0, रिक्त, 'a'},
	{ "stopping", 0, रिक्त, 'o'},
	{ "type",     1, रिक्त, 't'},
	{ "notrace",  0, रिक्त, 'T'},
	{ "strict",   0, रिक्त, 's'},
	{ "update",   0, रिक्त, 'u'},
	{ "deptree-file", 1, रिक्त, 'F'},
	longopts_COMMON
};
स्थिरांक अक्षर * स्थिरांक longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
स्थिरांक अक्षर *usagestring = रिक्त;

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = रिक्त;
	पूर्णांक options = RC_DEP_TRACE, update = 0;
	bool first = true;
	अक्षर *runlevel = xstrdup(दो_पर्या("RC_RUNLEVEL"));
	पूर्णांक opt;
	अक्षर *token;
	अक्षर *deptree_file = रिक्त;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	पर्यंत ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		निवड (opt) {
		स्थिती 'a':
			options |= RC_DEP_START;
			थांबा;
		स्थिती 'o':
			options |= RC_DEP_STOP;
			थांबा;
		स्थिती 's':
			options |= RC_DEP_STRICT;
			थांबा;
		स्थिती 't':
			पर्यंत ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			थांबा;
		स्थिती 'u':
			update = 1;
			थांबा;
		स्थिती 'T':
			options &= RC_DEP_TRACE;
			थांबा;
		स्थिती 'F':
			deptree_file = xstrdup(optarg);
			थांबा;

		case_RC_COMMON_GETOPT
		}
	}

	जर (deptree_file) {
		जर (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} अन्यथा {
		जर (!(deptree = _rc_deptree_load(update, रिक्त)))
			eerrorx("failed to load deptree");
	}

	जर (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	पर्यंत (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		त्रुटि_सं = 0;
		depends = rc_deptree_depends(deptree, रिक्त, list, runlevel, 0);
		जर (!depends && त्रुटि_सं == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		अन्यथा
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	जर (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		मुक्त(runlevel);
		जर (update)
			परत निकास_सफल;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	जर (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	जर (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			जर (first)
				first = false;
			अन्यथा
				म_लिखो (" ");
			म_लिखो ("%s", s->मूल्य);

		}
		म_लिखो ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	मुक्त(runlevel);
	परत निकास_सफल;
}
