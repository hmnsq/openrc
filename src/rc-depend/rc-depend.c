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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "aot:suTF:" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "starting", 0, வெற்று, 'a'},
	{ "stopping", 0, வெற்று, 'o'},
	{ "type",     1, வெற்று, 't'},
	{ "notrace",  0, வெற்று, 'T'},
	{ "strict",   0, வெற்று, 's'},
	{ "update",   0, வெற்று, 'u'},
	{ "deptree-file", 1, வெற்று, 'F'},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
மாறிலி எழுத்து *usagestring = வெற்று;

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = வெற்று;
	முழு options = RC_DEP_TRACE, update = 0;
	bool first = true;
	எழுத்து *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	முழு opt;
	எழுத்து *token;
	எழுத்து *deptree_file = வெற்று;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *) 0)) != -1)
	{
		தேர்வு (opt) {
		நிலை 'a':
			options |= RC_DEP_START;
			நிறுத்து;
		நிலை 'o':
			options |= RC_DEP_STOP;
			நிறுத்து;
		நிலை 's':
			options |= RC_DEP_STRICT;
			நிறுத்து;
		நிலை 't':
			போது ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			நிறுத்து;
		நிலை 'u':
			update = 1;
			நிறுத்து;
		நிலை 'T':
			options &= RC_DEP_TRACE;
			நிறுத்து;
		நிலை 'F':
			deptree_file = xstrdup(optarg);
			நிறுத்து;

		case_RC_COMMON_GETOPT
		}
	}

	எனில் (deptree_file) {
		எனில் (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} இல்லையெனில் {
		எனில் (!(deptree = _rc_deptree_load(update, வெற்று)))
			eerrorx("failed to load deptree");
	}

	எனில் (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	போது (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, வெற்று, list, runlevel, 0);
		எனில் (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		இல்லையெனில்
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	எனில் (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		விடுவி(runlevel);
		எனில் (update)
			திரும்பு EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	எனில் (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	எனில் (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			எனில் (first)
				first = false;
			இல்லையெனில்
				printf (" ");
			printf ("%s", s->மதிப்பு);

		}
		printf ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	விடுவி(runlevel);
	திரும்பு EXIT_SUCCESS;
}
