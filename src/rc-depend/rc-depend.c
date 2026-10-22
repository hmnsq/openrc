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

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "_usage.h"
#include "helpers.h"

const char *applet = NULL;
const char *extraopts = NULL;
const char getoptstring[] = "aot:suTF:" getoptstring_COMMON;
const struct option longopts[] = {
	{ "starting", 0, NULL, 'a'},
	{ "stopping", 0, NULL, 'o'},
	{ "type",     1, NULL, 't'},
	{ "notrace",  0, NULL, 'T'},
	{ "strict",   0, NULL, 's'},
	{ "update",   0, NULL, 'u'},
	{ "deptree-file", 1, NULL, 'F'},
	longopts_COMMON
};
const char * const longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
const char *usagestring = NULL;

整数 主要(整数 argc, char **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = NULL;
	整数 options = RC_DEP_TRACE, update = 0;
	bool first = true;
	char *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	整数 opt;
	char *token;
	char *deptree_file = NULL;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	当 ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (整数 *) 0)) != -1)
	{
		switch (opt) {
		case 'a':
			options |= RC_DEP_START;
			break;
		case 'o':
			options |= RC_DEP_STOP;
			break;
		case 's':
			options |= RC_DEP_STRICT;
			break;
		case 't':
			当 ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			break;
		case 'u':
			update = 1;
			break;
		case 'T':
			options &= RC_DEP_TRACE;
			break;
		case 'F':
			deptree_file = xstrdup(optarg);
			break;

		case_RC_COMMON_GETOPT
		}
	}

	如果 (deptree_file) {
		如果 (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} 否则 {
		如果 (!(deptree = _rc_deptree_load(update, NULL)))
			eerrorx("failed to load deptree");
	}

	如果 (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	当 (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, NULL, list, runlevel, 0);
		如果 (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		否则
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	如果 (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		free(runlevel);
		如果 (update)
			返回 EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	如果 (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	如果 (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			如果 (first)
				first = false;
			否则
				printf (" ");
			printf ("%s", s->value);

		}
		printf ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	free(runlevel);
	返回 EXIT_SUCCESS;
}
