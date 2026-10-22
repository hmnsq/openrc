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
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "aot:suTF:" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "starting", 0, ଖାଲି, 'a'},
	{ "stopping", 0, ଖାଲି, 'o'},
	{ "type",     1, ଖାଲି, 't'},
	{ "notrace",  0, ଖାଲି, 'T'},
	{ "strict",   0, ଖାଲି, 's'},
	{ "update",   0, ଖାଲି, 'u'},
	{ "deptree-file", 1, ଖାଲି, 'F'},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
ଅଚଳ ଅକ୍ଷର *usagestring = ଖାଲି;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା options = RC_DEP_TRACE, update = 0;
	bool first = true;
	ଅକ୍ଷର *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ଅକ୍ଷର *token;
	ଅକ୍ଷର *deptree_file = ଖାଲି;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
	{
		ବାଛ (opt) {
		ସ୍ଥିତି 'a':
			options |= RC_DEP_START;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'o':
			options |= RC_DEP_STOP;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			options |= RC_DEP_STRICT;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 't':
			ଯେତେବେଳେ ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'u':
			update = 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'T':
			options &= RC_DEP_TRACE;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'F':
			deptree_file = xstrdup(optarg);
			ଭାଙ୍ଗ;

		case_RC_COMMON_GETOPT
		}
	}

	ଯଦି (deptree_file) {
		ଯଦି (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} ଅନ୍ୟଥା {
		ଯଦି (!(deptree = _rc_deptree_load(update, ଖାଲି)))
			eerrorx("failed to load deptree");
	}

	ଯଦି (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	ଯେତେବେଳେ (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, ଖାଲି, list, runlevel, 0);
		ଯଦି (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		ଅନ୍ୟଥା
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	ଯଦି (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		ମୁକ୍ତ(runlevel);
		ଯଦି (update)
			ଫେରନ୍ତୁ EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	ଯଦି (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	ଯଦି (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			ଯଦି (first)
				first = false;
			ଅନ୍ୟଥା
				printf (" ");
			printf ("%s", s->ମୂଲ୍ୟ);

		}
		printf ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	ମୁକ୍ତ(runlevel);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
