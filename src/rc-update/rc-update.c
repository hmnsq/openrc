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
static ចំនួនគត់
add(const var_na *runlevel, const var_na *service)
{
	ចំនួនគត់ retval = -1;

	ប្រសិនបើ (!rc_service_exists(service)) {
		ប្រសិនបើ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		បើមិនដូច្នេះ
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} បើមិនដូច្នេះ ប្រសិនបើ (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} បើមិនដូច្នេះ ប្រសិនបើ (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} បើមិនដូច្នេះ
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ត្រឡប់ retval;
}

static ចំនួនគត់
delete(const var_na *runlevel, const var_na *service)
{
	ចំនួនគត់ retval = -1;

	errno = 0;
	ប្រសិនបើ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ត្រឡប់ 1;
	}

	ប្រសិនបើ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	បើមិនដូច្នេះ
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ត្រឡប់ retval;
}

static ចំនួនគត់
addstack(const var_na *runlevel, const var_na *stack)
{
	ប្រសិនបើ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ត្រឡប់ -1;
	}
	ប្រសិនបើ (!rc_runlevel_exists(stack)) {
		eerror("%s: runlevel `%s' does not exist", applet, stack);
		ត្រឡប់ -1;
	}
	ប្រសិនបើ (strcmp(runlevel, stack) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, stack);
		ត្រឡប់ -1;
	}
	ប្រសិនបើ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
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
		ត្រឡប់ -1;
	}
	ប្រសិនបើ (!rc_runlevel_stack(runlevel, stack)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, stack, runlevel, strerror(errno));
		ត្រឡប់ -1;
	}
	einfo("runlevel %s added to runlevel %s", stack, runlevel);
	ត្រឡប់ 1;
}

static ចំនួនគត់
delstack(const var_na *runlevel, const var_na *stack)
{
	ប្រសិនបើ (rc_runlevel_unstack(runlevel, stack)) {
		einfo("runlevel %s deleted from runlevel %s", stack, runlevel);
		ត្រឡប់ 1;
	}

	ប្រសិនបើ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, stack, runlevel);
	បើមិនដូច្នេះ
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, stack, runlevel, strerror (errno));

	ត្រឡប់ -1;
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
			ប្រសិនបើ (rc_service_in_runlevel(service->value,
				runlevel->value))
			{
				rc_stringlist_add(in, runlevel->value);
				inone = true;
			} បើមិនដូច្នេះ {
				l = strlen(runlevel->value);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				free(buffer);
			}
		}

		ប្រសិនបើ (inone || verbose) {
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

ចំនួនគត់ main(ចំនួនគត់ argc, var_na **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	var_na *service = NULL;
	var_na *p;
	ចំនួនគត់ action = 0;
	bool verbose = false, stack = false, all_runlevels = false;
	ចំនួនគត់ opt;
	ចំនួនគត់ retval = EXIT_FAILURE;
	ចំនួនគត់ num_updated = 0;
	ចំនួនគត់ (*actfunc)(const var_na *, const var_na *);
	ចំនួនគត់ ret;

	applet = basename_c(argv[0]);
	ខណៈពេល ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ចំនួនគត់ *)0)) != -1)
		switch (opt) {
		pak_sa 'a':
		    all_runlevels = true;
		bha_nga;
		pak_sa 's':
		    stack = true;
		bha_nga;
		pak_sa 'u':
			deptree = _rc_deptree_load(-1, &ret);
			ប្រសិនបើ (deptree)
				rc_deptree_free(deptree);
			ត្រឡប់ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ប្រសិនបើ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ប្រសិនបើ (optind < argc) {
		ប្រសិនបើ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		បើមិនដូច្នេះ ប្រសិនបើ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ប្រសិនបើ (action)
			optind++;
		បើមិនដូច្នេះ
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ប្រសិនបើ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ប្រសិនបើ (optind >= argc) {
		ប្រសិនបើ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} បើមិនដូច្នេះ {
		service = argv[optind];
		optind++;

		ខណៈពេល (optind < argc)
			ប្រសិនបើ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			បើមិនដូច្នេះ {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ប្រសិនបើ (action & DOSHOW) {
		ប្រសិនបើ (service)
			rc_stringlist_add(runlevels, service);
		ប្រសិនបើ (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (!service)
			eerror ("%s: no service specified", applet);
		បើមិនដូច្នេះ {
			ប្រសិនបើ (action & DOADD) {
				ប្រសិនបើ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = stack ? addstack : add;
			} បើមិនដូច្នេះ ប្រសិនបើ (action & DODELETE) {
				actfunc = stack ? delstack : delete;
			} បើមិនដូច្នេះ {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ប្រសិនបើ (!TAILQ_FIRST(runlevels)) {
				ប្រសិនបើ (all_runlevels) {
					free(runlevels);
					runlevels = rc_runlevel_list();
				} បើមិនដូច្នេះ {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					free(p);
				}
			}

			ប្រសិនបើ (!TAILQ_FIRST(runlevels)) {
				free(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ប្រសិនបើ (!rc_runlevel_exists(runlevel->value)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->value);
					continue;
				}

				ret = actfunc(runlevel->value, service);
				ប្រសិនបើ (ret < 0)
					retval = EXIT_FAILURE;
				num_updated += ret;
			}

			ប្រសិនបើ (retval == EXIT_SUCCESS &&
			    num_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ត្រឡប់ retval;
}
