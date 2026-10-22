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
#ضمن "misc.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "aot:suTF:" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "starting", 0, عدم, 'a'},
	{ "stopping", 0, عدم, 'o'},
	{ "type",     1, عدم, 't'},
	{ "notrace",  0, عدم, 'T'},
	{ "strict",   0, عدم, 's'},
	{ "update",   0, عدم, 'u'},
	{ "deptree-file", 1, عدم, 'F'},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
ثابت حرف *usagestring = عدم;

صحيح main(صحيح argc, حرف **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = عدم;
	صحيح options = RC_DEP_TRACE, update = 0;
	bool first = true;
	حرف *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	صحيح opt;
	حرف *token;
	حرف *deptree_file = عدم;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *) 0)) != -1)
	{
		بدل (opt) {
		حالة 'a':
			options |= RC_DEP_START;
			اكسر;
		حالة 'o':
			options |= RC_DEP_STOP;
			اكسر;
		حالة 's':
			options |= RC_DEP_STRICT;
			اكسر;
		حالة 't':
			طالما ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			اكسر;
		حالة 'u':
			update = 1;
			اكسر;
		حالة 'T':
			options &= RC_DEP_TRACE;
			اكسر;
		حالة 'F':
			deptree_file = xstrdup(optarg);
			اكسر;

		case_RC_COMMON_GETOPT
		}
	}

	إذا (deptree_file) {
		إذا (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} وإلا {
		إذا (!(deptree = _rc_deptree_load(update, عدم)))
			eerrorx("failed to load deptree");
	}

	إذا (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	طالما (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, عدم, list, runlevel, 0);
		إذا (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		وإلا
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	إذا (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		حرر(runlevel);
		إذا (update)
			أرجع EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	إذا (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	إذا (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			إذا (first)
				first = false;
			وإلا
				printf (" ");
			printf ("%s", s->درج);

		}
		printf ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	حرر(runlevel);
	أرجع EXIT_SUCCESS;
}
