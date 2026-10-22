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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "aot:suTF:" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "starting", 0, ఖాళీ, 'a'},
	{ "stopping", 0, ఖాళీ, 'o'},
	{ "type",     1, ఖాళీ, 't'},
	{ "notrace",  0, ఖాళీ, 'T'},
	{ "strict",   0, ఖాళీ, 's'},
	{ "update",   0, ఖాళీ, 'u'},
	{ "deptree-file", 1, ఖాళీ, 'F'},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
స్థిరరాశి అక్షరం *usagestring = ఖాళీ;

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = ఖాళీ;
	పూర్ణాంకం options = RC_DEP_TRACE, update = 0;
	bool మొదటి = true;
	అక్షరం *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	పూర్ణాంకం opt;
	అక్షరం *token;
	అక్షరం *deptree_file = ఖాళీ;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *) 0)) != -1)
	{
		ఎంపిక (opt) {
		స్థితి 'a':
			options |= RC_DEP_ప్రారంభం;
			ఆపు;
		స్థితి 'o':
			options |= RC_DEP_STOP;
			ఆపు;
		స్థితి 's':
			options |= RC_DEP_STRICT;
			ఆపు;
		స్థితి 't':
			అయితే ((token = strsep(&optarg, ",")))
				rc_stringlist_కూడు(types, token);
			ఆపు;
		స్థితి 'u':
			update = 1;
			ఆపు;
		స్థితి 'T':
			options &= RC_DEP_TRACE;
			ఆపు;
		స్థితి 'F':
			deptree_file = xstrdup(optarg);
			ఆపు;

		case_RC_COMMON_GETOPT
		}
	}

	ఒకవేళ (deptree_file) {
		ఒకవేళ (!(deptree = rc_deptree_ఎక్కించు_file(deptree_file)))
			eerrorx("failed to load deptree");
	} లేకపోతే {
		ఒకవేళ (!(deptree = rc_deptree_ఎక్కించు_2(update, ఖాళీ)))
			eerrorx("failed to load deptree");
	}

	ఒకవేళ (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	అయితే (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_కూడు(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, ఖాళీ, list, runlevel, 0);
		ఒకవేళ (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		లేకపోతే
			rc_stringlist_కూడు(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	ఒకవేళ (!TAILQ_మొదటి(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		విడుదల(runlevel);
		ఒకవేళ (update)
			తిరిగి EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	ఒకవేళ (!TAILQ_మొదటి(types)) {
		rc_stringlist_కూడు(types, "ineed");
		rc_stringlist_కూడు(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	ఒకవేళ (TAILQ_మొదటి(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			ఒకవేళ (మొదటి)
				మొదటి = false;
			లేకపోతే
				రాయి (" ");
			రాయి ("%s", s->విలువ);

		}
		రాయి ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	విడుదల(runlevel);
	తిరిగి EXIT_SUCCESS;
}
