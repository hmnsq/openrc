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

#شامل <errno.h>
#شامل <getopt.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "aot:suTF:" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "starting", 0, پوچ, 'a'},
	{ "stopping", 0, پوچ, 'o'},
	{ "type",     1, پوچ, 't'},
	{ "notrace",  0, پوچ, 'T'},
	{ "strict",   0, پوچ, 's'},
	{ "update",   0, پوچ, 'u'},
	{ "deptree-file", 1, پوچ, 'F'},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
ثابت نویسه *usagestring = پوچ;

صحیح اصلی(صحیح argc, نویسه **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = پوچ;
	صحیح options = RC_DEP_TRACE, update = 0;
	bool first = true;
	نویسه *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	صحیح opt;
	نویسه *token;
	نویسه *deptree_file = پوچ;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'a':
			options |= RC_DEP_START;
			بشکن;
		حالت 'o':
			options |= RC_DEP_STOP;
			بشکن;
		حالت 's':
			options |= RC_DEP_STRICT;
			بشکن;
		حالت 't':
			وقتی ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			بشکن;
		حالت 'u':
			update = 1;
			بشکن;
		حالت 'T':
			options &= RC_DEP_TRACE;
			بشکن;
		حالت 'F':
			deptree_file = xstrdup(optarg);
			بشکن;

		case_RC_COMMON_GETOPT
		}
	}

	اگر (deptree_file) {
		اگر (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} وگرنه {
		اگر (!(deptree = _rc_deptree_load(update, پوچ)))
			eerrorx("failed to load deptree");
	}

	اگر (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	وقتی (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, پوچ, list, runlevel, 0);
		اگر (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		وگرنه
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	اگر (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		آزاد(runlevel);
		اگر (update)
			بازگشت EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	اگر (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	اگر (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			اگر (first)
				first = false;
			وگرنه
				printf (" ");
			printf ("%s", s->مقدار);

		}
		printf ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	آزاد(runlevel);
	بازگشت EXIT_SUCCESS;
}
