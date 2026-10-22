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
static ကိန်းပြည့်
add(const var_na *runlevel, const var_na *service)
{
	ကိန်းပြည့် retval = -1;

	အကယ်၍ (!rc_service_exists(service)) {
		အကယ်၍ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		မဟုတ်ပါက
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} မဟုတ်ပါက အကယ်၍ (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} မဟုတ်ပါက အကယ်၍ (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} မဟုတ်ပါက
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ပြန်ပေး retval;
}

static ကိန်းပြည့်
delete(const var_na *runlevel, const var_na *service)
{
	ကိန်းပြည့် retval = -1;

	errno = 0;
	အကယ်၍ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ပြန်ပေး 1;
	}

	အကယ်၍ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	မဟုတ်ပါက
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ပြန်ပေး retval;
}

static ကိန်းပြည့်
addstack(const var_na *runlevel, const var_na *stack)
{
	အကယ်၍ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ပြန်ပေး -1;
	}
	အကယ်၍ (!rc_runlevel_exists(stack)) {
		eerror("%s: runlevel `%s' does not exist", applet, stack);
		ပြန်ပေး -1;
	}
	အကယ်၍ (strcmp(runlevel, stack) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, stack);
		ပြန်ပေး -1;
	}
	အကယ်၍ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
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
		ပြန်ပေး -1;
	}
	အကယ်၍ (!rc_runlevel_stack(runlevel, stack)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, stack, runlevel, strerror(errno));
		ပြန်ပေး -1;
	}
	einfo("runlevel %s added to runlevel %s", stack, runlevel);
	ပြန်ပေး 1;
}

static ကိန်းပြည့်
delstack(const var_na *runlevel, const var_na *stack)
{
	အကယ်၍ (rc_runlevel_unstack(runlevel, stack)) {
		einfo("runlevel %s deleted from runlevel %s", stack, runlevel);
		ပြန်ပေး 1;
	}

	အကယ်၍ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, stack, runlevel);
	မဟုတ်ပါက
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, stack, runlevel, strerror (errno));

	ပြန်ပေး -1;
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
			အကယ်၍ (rc_service_in_runlevel(service->value,
				runlevel->value))
			{
				rc_stringlist_add(in, runlevel->value);
				inone = true;
			} မဟုတ်ပါက {
				l = strlen(runlevel->value);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				free(buffer);
			}
		}

		အကယ်၍ (inone || verbose) {
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

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	var_na *service = NULL;
	var_na *p;
	ကိန်းပြည့် action = 0;
	bool verbose = false, stack = false, all_runlevels = false;
	ကိန်းပြည့် opt;
	ကိန်းပြည့် retval = EXIT_FAILURE;
	ကိန်းပြည့် num_updated = 0;
	ကိန်းပြည့် (*actfunc)(const var_na *, const var_na *);
	ကိန်းပြည့် ret;

	applet = basename_c(argv[0]);
	နေစဉ် ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ကိန်းပြည့် *)0)) != -1)
		switch (opt) {
		pak_sa 'a':
		    all_runlevels = true;
		bha_nga;
		pak_sa 's':
		    stack = true;
		bha_nga;
		pak_sa 'u':
			deptree = _rc_deptree_load(-1, &ret);
			အကယ်၍ (deptree)
				rc_deptree_free(deptree);
			ပြန်ပေး ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	အကယ်၍ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	အကယ်၍ (optind < argc) {
		အကယ်၍ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		မဟုတ်ပါက အကယ်၍ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		မဟုတ်ပါက အကယ်၍ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		အကယ်၍ (action)
			optind++;
		မဟုတ်ပါက
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	အကယ်၍ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	အကယ်၍ (optind >= argc) {
		အကယ်၍ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} မဟုတ်ပါက {
		service = argv[optind];
		optind++;

		နေစဉ် (optind < argc)
			အကယ်၍ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			မဟုတ်ပါက {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	အကယ်၍ (action & DOSHOW) {
		အကယ်၍ (service)
			rc_stringlist_add(runlevels, service);
		အကယ်၍ (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} မဟုတ်ပါက {
		အကယ်၍ (!service)
			eerror ("%s: no service specified", applet);
		မဟုတ်ပါက {
			အကယ်၍ (action & DOADD) {
				အကယ်၍ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = stack ? addstack : add;
			} မဟုတ်ပါက အကယ်၍ (action & DODELETE) {
				actfunc = stack ? delstack : delete;
			} မဟုတ်ပါက {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			အကယ်၍ (!TAILQ_FIRST(runlevels)) {
				အကယ်၍ (all_runlevels) {
					free(runlevels);
					runlevels = rc_runlevel_list();
				} မဟုတ်ပါက {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					free(p);
				}
			}

			အကယ်၍ (!TAILQ_FIRST(runlevels)) {
				free(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				အကယ်၍ (!rc_runlevel_exists(runlevel->value)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->value);
					continue;
				}

				ret = actfunc(runlevel->value, service);
				အကယ်၍ (ret < 0)
					retval = EXIT_FAILURE;
				num_updated += ret;
			}

			အကယ်၍ (retval == EXIT_SUCCESS &&
			    num_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ပြန်ပေး retval;
}
