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
#समावेश "misc.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "aot:suTF:" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "starting", 0, शून्य, 'a'},
	{ "stopping", 0, शून्य, 'o'},
	{ "type",     1, शून्य, 't'},
	{ "notrace",  0, शून्य, 'T'},
	{ "strict",   0, शून्य, 's'},
	{ "update",   0, शून्य, 'u'},
	{ "deptree-file", 1, शून्य, 'F'},
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = शून्य;

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = शून्य;
	पूर्णांक options = RC_DEP_TRACE, update = 0;
	bool first = true;
	अक्षर *runlevel = xstrdup(दो_पर्या("RC_RUNLEVEL"));
	पूर्णांक opt;
	अक्षर *token;
	अक्षर *deptree_file = शून्य;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *) 0)) != -1)
	{
		चयन (opt) {
		हाल 'a':
			options |= RC_DEP_START;
			अवरोध;
		हाल 'o':
			options |= RC_DEP_STOP;
			अवरोध;
		हाल 's':
			options |= RC_DEP_STRICT;
			अवरोध;
		हाल 't':
			जबतक ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			अवरोध;
		हाल 'u':
			update = 1;
			अवरोध;
		हाल 'T':
			options &= RC_DEP_TRACE;
			अवरोध;
		हाल 'F':
			deptree_file = xstrdup(optarg);
			अवरोध;

		case_RC_COMMON_GETOPT
		}
	}

	अगर (deptree_file) {
		अगर (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} अन्यथा {
		अगर (!(deptree = _rc_deptree_load(update, शून्य)))
			eerrorx("failed to load deptree");
	}

	अगर (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	जबतक (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		त्रुटि_सं = 0;
		depends = rc_deptree_depends(deptree, शून्य, list, runlevel, 0);
		अगर (!depends && त्रुटि_सं == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		अन्यथा
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	अगर (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		मुक्त(runlevel);
		अगर (update)
			वापस निकास_सफल;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	अगर (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	अगर (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			अगर (first)
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
	वापस निकास_सफल;
}
