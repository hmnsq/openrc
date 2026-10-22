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

const var_na *applet = NULL;
const var_na *extraopts = NULL;
const var_na getoptstring[] = "aot:suTF:" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "starting", 0, NULL, 'a'},
	{ "stopping", 0, NULL, 'o'},
	{ "type",     1, NULL, 't'},
	{ "notrace",  0, NULL, 'T'},
	{ "strict",   0, NULL, 's'},
	{ "update",   0, NULL, 'u'},
	{ "deptree-file", 1, NULL, 'F'},
	longopts_COMMON
};
const var_na * const longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
const var_na *usagestring = NULL;

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = NULL;
	ကိန်းပြည့် options = RC_DEP_TRACE, update = 0;
	bool first = true;
	var_na *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	ကိန်းပြည့် opt;
	var_na *token;
	var_na *deptree_file = NULL;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	နေစဉ် ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ကိန်းပြည့် *) 0)) != -1)
	{
		switch (opt) {
		pak_sa 'a':
			options |= RC_DEP_START;
			bha_nga;
		pak_sa 'o':
			options |= RC_DEP_STOP;
			bha_nga;
		pak_sa 's':
			options |= RC_DEP_STRICT;
			bha_nga;
		pak_sa 't':
			နေစဉ် ((token = strsep(&optarg, ",")))
				rc_stringlist_add(types, token);
			bha_nga;
		pak_sa 'u':
			update = 1;
			bha_nga;
		pak_sa 'T':
			options &= RC_DEP_TRACE;
			bha_nga;
		pak_sa 'F':
			deptree_file = xstrdup(optarg);
			bha_nga;

		case_RC_COMMON_GETOPT
		}
	}

	အကယ်၍ (deptree_file) {
		အကယ်၍ (!(deptree = rc_deptree_load_file(deptree_file)))
			eerrorx("failed to load deptree");
	} မဟုတ်ပါက {
		အကယ်၍ (!(deptree = _rc_deptree_load(update, NULL)))
			eerrorx("failed to load deptree");
	}

	အကယ်၍ (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	နေစဉ် (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_add(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, NULL, list, runlevel, 0);
		အကယ်၍ (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		မဟုတ်ပါက
			rc_stringlist_add(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	အကယ်၍ (!TAILQ_FIRST(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		free(runlevel);
		အကယ်၍ (update)
			ပြန်ပေး EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	အကယ်၍ (!TAILQ_FIRST(types)) {
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	အကယ်၍ (TAILQ_FIRST(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			အကယ်၍ (first)
				first = false;
			မဟုတ်ပါက
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
	ပြန်ပေး EXIT_SUCCESS;
}
