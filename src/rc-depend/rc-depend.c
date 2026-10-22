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

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "aot:suTF:" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{ "starting", 0, تہی, 'a'},
	{ "stopping", 0, تہی, 'o'},
	{ "type",     1, تہی, 't'},
	{ "notrace",  0, تہی, 'T'},
	{ "strict",   0, تہی, 's'},
	{ "update",   0, تہی, 'u'},
	{ "deptree-file", 1, تہی, 'F'},
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
ثابت حرف *usagestring = تہی;

صحیح مین(صحیح argc, حرف **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = تہی;
	صحیح options = RC_DEP_TRACE, update = 0;
	bool پہلا = true;
	حرف *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	صحیح opt;
	حرف *token;
	حرف *deptree_file = تہی;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	جب ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *) 0)) != -1)
	{
		انتخاب (opt) {
		حالت 'a':
			options |= RC_DEP_آغاز;
			توڑو;
		حالت 'o':
			options |= RC_DEP_STOP;
			توڑو;
		حالت 's':
			options |= RC_DEP_STRICT;
			توڑو;
		حالت 't':
			جب ((token = strsep(&optarg, ",")))
				rc_stringlist_جوڑو(types, token);
			توڑو;
		حالت 'u':
			update = 1;
			توڑو;
		حالت 'T':
			options &= RC_DEP_TRACE;
			توڑو;
		حالت 'F':
			deptree_file = xstrdup(optarg);
			توڑو;

		case_RC_COMMON_GETOPT
		}
	}

	اگر (deptree_file) {
		اگر (!(deptree = rc_deptree_بھرو_file(deptree_file)))
			eerrorx("failed to load deptree");
	} ورنہ {
		اگر (!(deptree = rc_deptree_بھرو_2(update, تہی)))
			eerrorx("failed to load deptree");
	}

	اگر (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	جب (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_جوڑو(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, تہی, list, runlevel, 0);
		اگر (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		ورنہ
			rc_stringlist_جوڑو(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	اگر (!TAILQ_پہلا(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		آزاد(runlevel);
		اگر (update)
			واپس EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	اگر (!TAILQ_پہلا(types)) {
		rc_stringlist_جوڑو(types, "ineed");
		rc_stringlist_جوڑو(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	اگر (TAILQ_پہلا(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			اگر (پہلا)
				پہلا = false;
			ورنہ
				لکھو (" ");
			لکھو ("%s", s->قدر);

		}
		لکھو ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	آزاد(runlevel);
	واپس EXIT_SUCCESS;
}
