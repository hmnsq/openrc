/*
 * rc-update
 * Manage init scripts and runlevels
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
const var_na *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
const var_na getoptstring[] = "asu" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{ "all",             0, NULL, 'a' },
	{ "stack",           0, NULL, 's' },
	{ "update",          0, NULL, 'u' },
	longopts_COMMON
};
const var_na * const longopts_help[] = {
	"Process all runlevels",
	"Stack a runlevel instead of a service",
	"Force an update of the dependency tree",
	longopts_help_COMMON
};

/* Return the number of changes made:
 *  -1 = no changes (error)
 *   0 = no changes (nothing to do)
 *  1+ = number of runlevels updated
 */
static ཧྲིལ
add(const var_na *runlevel, const var_na *service)
{
	ཧྲིལ retval = -1;

	གལ་སྲིད (!rc_service_exists(service)) {
		གལ་སྲིད (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		དེ་མིན
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} དེ་མིན གལ་སྲིད (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} དེ་མིན གལ་སྲིད (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} དེ་མིན
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ལོག retval;
}

static ཧྲིལ
delete(const var_na *runlevel, const var_na *service)
{
	ཧྲིལ retval = -1;

	errno = 0;
	གལ་སྲིད (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ལོག 1;
	}

	གལ་སྲིད (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	དེ་མིན
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ལོག retval;
}

static ཧྲིལ
addstack(const var_na *runlevel, const var_na *stack)
{
	གལ་སྲིད (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ལོག -1;
	}
	གལ་སྲིད (!rc_runlevel_exists(stack)) {
		eerror("%s: runlevel `%s' does not exist", applet, stack);
		ལོག -1;
	}
	གལ་སྲིད (strcmp(runlevel, stack) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, stack);
		ལོག -1;
	}
	གལ་སྲིད (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(stack, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(stack, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(stack, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(stack, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		ལོག -1;
	}
	གལ་སྲིད (!rc_runlevel_stack(runlevel, stack)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, stack, runlevel, strerror(errno));
		ལོག -1;
	}
	einfo("runlevel %s added to runlevel %s", stack, runlevel);
	ལོག 1;
}

static ཧྲིལ
delstack(const var_na *runlevel, const var_na *stack)
{
	གལ་སྲིད (rc_runlevel_unstack(runlevel, stack)) {
		einfo("runlevel %s deleted from runlevel %s", stack, runlevel);
		ལོག 1;
	}

	གལ་སྲིད (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, stack, runlevel);
	དེ་མིན
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, stack, runlevel, strerror (errno));

	ལོག -1;
}

static void
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(NULL);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	var_na *buffer = NULL;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			གལ་སྲིད (rc_service_in_runlevel(service->value,
				runlevel->value))
			{
				rc_stringlist_add(in, runlevel->value);
				inone = true;
			} དེ་མིན {
				l = strlen(runlevel->value);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				free(buffer);
			}
		}

		གལ་སྲིད (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->value);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->value);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

ཧྲིལ main(ཧྲིལ argc, var_na **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	var_na *service = NULL;
	var_na *p;
	ཧྲིལ action = 0;
	bool verbose = false, stack = false, all_runlevels = false;
	ཧྲིལ opt;
	ཧྲིལ retval = EXIT_FAILURE;
	ཧྲིལ num_updated = 0;
	ཧྲིལ (*actfunc)(const var_na *, const var_na *);
	ཧྲིལ ret;

	applet = basename_c(argv[0]);
	སྐབས ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ཧྲིལ *)0)) != -1)
		switch (opt) {
		pak_sa 'a':
		    all_runlevels = true;
		bha_nga;
		pak_sa 's':
		    stack = true;
		bha_nga;
		pak_sa 'u':
			deptree = _rc_deptree_load(-1, &ret);
			གལ་སྲིད (deptree)
				rc_deptree_free(deptree);
			ལོག ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	གལ་སྲིད ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	གལ་སྲིད (optind < argc) {
		གལ་སྲིད (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		དེ་མིན གལ་སྲིད (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		དེ་མིན གལ་སྲིད (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		གལ་སྲིད (action)
			optind++;
		དེ་མིན
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	གལ་སྲིད (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	གལ་སྲིད (optind >= argc) {
		གལ་སྲིད (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} དེ་མིན {
		service = argv[optind];
		optind++;

		སྐབས (optind < argc)
			གལ་སྲིད (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			དེ་མིན {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	གལ་སྲིད (action & DOSHOW) {
		གལ་སྲིད (service)
			rc_stringlist_add(runlevels, service);
		གལ་སྲིད (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} དེ་མིན {
		གལ་སྲིད (!service)
			eerror ("%s: no service specified", applet);
		དེ་མིན {
			གལ་སྲིད (action & DOADD) {
				གལ་སྲིད (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = stack ? addstack : add;
			} དེ་མིན གལ་སྲིད (action & DODELETE) {
				actfunc = stack ? delstack : delete;
			} དེ་མིན {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			གལ་སྲིད (!TAILQ_FIRST(runlevels)) {
				གལ་སྲིད (all_runlevels) {
					free(runlevels);
					runlevels = rc_runlevel_list();
				} དེ་མིན {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					free(p);
				}
			}

			གལ་སྲིད (!TAILQ_FIRST(runlevels)) {
				free(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				གལ་སྲིད (!rc_runlevel_exists(runlevel->value)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->value);
					continue;
				}

				ret = actfunc(runlevel->value, service);
				གལ་སྲིད (ret < 0)
					retval = EXIT_FAILURE;
				num_updated += ret;
			}

			གལ་སྲིད (retval == EXIT_SUCCESS &&
			    num_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ལོག retval;
}
