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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "aot:suTF:" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "starting", 0, ਨਿਰਾ, 'a'},
	{ "stopping", 0, ਨਿਰਾ, 'o'},
	{ "type",     1, ਨਿਰਾ, 't'},
	{ "notrace",  0, ਨਿਰਾ, 'T'},
	{ "strict",   0, ਨਿਰਾ, 's'},
	{ "update",   0, ਨਿਰਾ, 'u'},
	{ "deptree-file", 1, ਨਿਰਾ, 'F'},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
	"Order services as if runlevel is starting",
	"Order services as if runlevel is stopping",
	"Type(s) of dependency to list",
	"Don't trace service dependencies",
	"Only use what is in the runlevels",
	"Force an update of the dependency tree",
	"File to load cached deptree from",
	longopts_help_COMMON
};
ਅਚਲ ਅੱਖਰ *usagestring = ਨਿਰਾ;

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;
	RC_STRINGLIST *depends;
	RC_STRING *s;
	RC_DEPTREE *deptree = ਨਿਰਾ;
	ਪੂਰਨ options = RC_DEP_TRACE, update = 0;
	bool ਪਹਿਲਾ = true;
	ਅੱਖਰ *runlevel = xstrdup(getenv("RC_RUNLEVEL"));
	ਪੂਰਨ opt;
	ਅੱਖਰ *token;
	ਅੱਖਰ *deptree_file = ਨਿਰਾ;

	applet = basename_c(argv[0]);
	types = rc_stringlist_new();
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *) 0)) != -1)
	{
		ਚੋਣ (opt) {
		ਹਾਲਤ 'a':
			options |= RC_DEP_ਸ਼ੁਰੂ;
			ਰੋਕੋ;
		ਹਾਲਤ 'o':
			options |= RC_DEP_STOP;
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			options |= RC_DEP_STRICT;
			ਰੋਕੋ;
		ਹਾਲਤ 't':
			ਜਦੋਂ ((token = strsep(&optarg, ",")))
				rc_stringlist_ਜੋੜੋ(types, token);
			ਰੋਕੋ;
		ਹਾਲਤ 'u':
			update = 1;
			ਰੋਕੋ;
		ਹਾਲਤ 'T':
			options &= RC_DEP_TRACE;
			ਰੋਕੋ;
		ਹਾਲਤ 'F':
			deptree_file = xstrdup(optarg);
			ਰੋਕੋ;

		case_RC_COMMON_GETOPT
		}
	}

	ਜੇ (deptree_file) {
		ਜੇ (!(deptree = rc_deptree_ਲੱਦੋ_file(deptree_file)))
			eerrorx("failed to load deptree");
	} ਨਹੀਂਤਾਂ {
		ਜੇ (!(deptree = rc_deptree_ਲੱਦੋ_2(update, ਨਿਰਾ)))
			eerrorx("failed to load deptree");
	}

	ਜੇ (!runlevel)
		runlevel = default_runlevel();

	services = rc_stringlist_new();
	ਜਦੋਂ (optind < argc) {
		list = rc_stringlist_new();
		rc_stringlist_ਜੋੜੋ(list, argv[optind]);
		errno = 0;
		depends = rc_deptree_depends(deptree, ਨਿਰਾ, list, runlevel, 0);
		ਜੇ (!depends && errno == ENOENT)
			eerror("no dependency info for service `%s'",
			    argv[optind]);
		ਨਹੀਂਤਾਂ
			rc_stringlist_ਜੋੜੋ(services, argv[optind]);

		rc_stringlist_free(depends);
		rc_stringlist_free(list);
		optind++;
	}
	ਜੇ (!TAILQ_ਪਹਿਲਾ(services)) {
		rc_stringlist_free(services);
		rc_stringlist_free(types);
		rc_deptree_free(deptree);
		ਮੁਕਤ(runlevel);
		ਜੇ (update)
			ਵਾਪਸ EXIT_SUCCESS;
		eerrorx("no services specified");
	}

	/* If we don't have any types, then supply some defaults */
	ਜੇ (!TAILQ_ਪਹਿਲਾ(types)) {
		rc_stringlist_ਜੋੜੋ(types, "ineed");
		rc_stringlist_ਜੋੜੋ(types, "iuse");
	}

	depends = rc_deptree_depends(deptree, types, services,
	    runlevel, options);

	ਜੇ (TAILQ_ਪਹਿਲਾ(depends)) {
		TAILQ_FOREACH(s, depends, entries) {
			ਜੇ (ਪਹਿਲਾ)
				ਪਹਿਲਾ = false;
			ਨਹੀਂਤਾਂ
				ਲਿਖੋ (" ");
			ਲਿਖੋ ("%s", s->ਮੁੱਲ);

		}
		ਲਿਖੋ ("\n");
	}

	rc_stringlist_free(types);
	rc_stringlist_free(services);
	rc_stringlist_free(depends);
	rc_deptree_free(deptree);
	ਮੁਕਤ(runlevel);
	ਵਾਪਸ EXIT_SUCCESS;
}
