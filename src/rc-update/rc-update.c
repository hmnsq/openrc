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

const char *applet = NULL;
const char *extraopts = NULL;
const char *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
const char getoptstring[] = "asu" getoptstring_COMMON;
const struct option longopts[] = {
	{ "all",             0, NULL, 'a' },
	{ "stack",           0, NULL, 's' },
	{ "update",          0, NULL, 'u' },
	longopts_COMMON
};
const char * const longopts_help[] = {
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
static số_nguyên
add(const char *runlevel, const char *service)
{
	số_nguyên retval = -1;

	nếu (!rc_service_exists(service)) {
		nếu (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		không_thì
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} không_thì nếu (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} không_thì nếu (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} không_thì
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	trả_về retval;
}

static số_nguyên
delete(const char *runlevel, const char *service)
{
	số_nguyên retval = -1;

	errno = 0;
	nếu (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		trả_về 1;
	}

	nếu (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	không_thì
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	trả_về retval;
}

static số_nguyên
addstack(const char *runlevel, const char *stack)
{
	nếu (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		trả_về -1;
	}
	nếu (!rc_runlevel_exists(stack)) {
		eerror("%s: runlevel `%s' does not exist", applet, stack);
		trả_về -1;
	}
	nếu (strcmp(runlevel, stack) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, stack);
		trả_về -1;
	}
	nếu (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
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
		trả_về -1;
	}
	nếu (!rc_runlevel_stack(runlevel, stack)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, stack, runlevel, strerror(errno));
		trả_về -1;
	}
	einfo("runlevel %s added to runlevel %s", stack, runlevel);
	trả_về 1;
}

static số_nguyên
delstack(const char *runlevel, const char *stack)
{
	nếu (rc_runlevel_unstack(runlevel, stack)) {
		einfo("runlevel %s deleted from runlevel %s", stack, runlevel);
		trả_về 1;
	}

	nếu (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, stack, runlevel);
	không_thì
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, stack, runlevel, strerror (errno));

	trả_về -1;
}

static void
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(NULL);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	char *buffer = NULL;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			nếu (rc_service_in_runlevel(service->value,
				runlevel->value))
			{
				rc_stringlist_add(in, runlevel->value);
				inone = true;
			} không_thì {
				l = strlen(runlevel->value);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				free(buffer);
			}
		}

		nếu (inone || verbose) {
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

số_nguyên main(số_nguyên argc, char **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	char *service = NULL;
	char *p;
	số_nguyên action = 0;
	bool verbose = false, stack = false, all_runlevels = false;
	số_nguyên opt;
	số_nguyên retval = EXIT_FAILURE;
	số_nguyên num_updated = 0;
	số_nguyên (*actfunc)(const char *, const char *);
	số_nguyên ret;

	applet = basename_c(argv[0]);
	trong_khi ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (số_nguyên *)0)) != -1)
		switch (opt) {
		case 'a':
		    all_runlevels = true;
		break;
		case 's':
		    stack = true;
		break;
		case 'u':
			deptree = _rc_deptree_load(-1, &ret);
			nếu (deptree)
				rc_deptree_free(deptree);
			trả_về ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	nếu ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	nếu (optind < argc) {
		nếu (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		không_thì nếu (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		không_thì nếu (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		nếu (action)
			optind++;
		không_thì
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	nếu (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	nếu (optind >= argc) {
		nếu (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} không_thì {
		service = argv[optind];
		optind++;

		trong_khi (optind < argc)
			nếu (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			không_thì {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	nếu (action & DOSHOW) {
		nếu (service)
			rc_stringlist_add(runlevels, service);
		nếu (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} không_thì {
		nếu (!service)
			eerror ("%s: no service specified", applet);
		không_thì {
			nếu (action & DOADD) {
				nếu (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = stack ? addstack : add;
			} không_thì nếu (action & DODELETE) {
				actfunc = stack ? delstack : delete;
			} không_thì {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			nếu (!TAILQ_FIRST(runlevels)) {
				nếu (all_runlevels) {
					free(runlevels);
					runlevels = rc_runlevel_list();
				} không_thì {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					free(p);
				}
			}

			nếu (!TAILQ_FIRST(runlevels)) {
				free(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				nếu (!rc_runlevel_exists(runlevel->value)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->value);
					continue;
				}

				ret = actfunc(runlevel->value, service);
				nếu (ret < 0)
					retval = EXIT_FAILURE;
				num_updated += ret;
			}

			nếu (retval == EXIT_SUCCESS &&
			    num_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	trả_về retval;
}
