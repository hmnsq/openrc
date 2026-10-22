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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "aot:suTF:" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "starting", 0, ખાલી, 'a'},
	{ "stopping", 0, ખાલી, 'o'},
	{ "type",     1, ખાલી, 't'},
	{ "notrace",  0, ખાલી, 'T'},
	{ "strict",   0, ખાલી, 's'},
	{ "update",   0, ખાલી, 'u'},
	{ "deptree-file", 1, ખાલી, 'F'},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
અચલ અક્ષર *usagestring = ખાલી;

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = ખાલી;
	પૂર્ણાંક options = RC_DEP_TRACE, update = 0;
	bool first = true;
	અક્ષર *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	પૂર્ણાંક opt;
	અક્ષર *token;
	અક્ષર *deptree_file = ખાલી;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *) 0)) != -1)
	{
		પસંદગી (opt) {
		સ્થિતિ 'a':
			options |= RC_DEP_START;
			તોડો;
		સ્થિતિ 'o':
			options |= RC_DEP_STOP;
			તોડો;
		સ્થિતિ 's':
			options |= RC_DEP_STRICT;
			તોડો;
		સ્થિતિ 't':
			જ્યારે ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			તોડો;
		સ્થિતિ 'u':
			update = 1;
			તોડો;
		સ્થિતિ 'T':
			options &= RC_DEP_TRACE;
			તોડો;
		સ્થિતિ 'F':
			deptree_file = xstrdup(optarg);
			તોડો;

		case_RC_COMMON_GETOPT
		}
	}

	જો (deptree_file) {
		જો (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} નહીંતર {
		જો (!(deptree = _rc_deptree_load(update, ખાલી)))
			eerrorx("failed to load deptree");
	}

	જો (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	જ્યારે (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, ખાલી, list, runlevel, 0);
		જો (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		નહીંતર
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	જો (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		મુક્ત(runlevel);
		જો (update)
			પાછા EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	જો (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	જો (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			જો (first)
				first = false;
			નહીંતર
				printf (" ");
			printf ("%s", s->કિંમત);

		}
		printf ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	મુક્ત(runlevel);
	પાછા EXIT_SUCCESS;
}
