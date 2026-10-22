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

#ضمن <errno.h>
#ضمن <getopt.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ثابت حرف getoptstring[] = "asu" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{ "all",             0, عدم, 'a' },
	{ "stack",           0, عدم, 's' },
	{ "update",          0, عدم, 'u' },
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ساكن صحيح
add(ثابت حرف *runlevel, ثابت حرف *service)
{
	صحيح retval = -1;

	ܐܢ (!rc_service_exists(service)) {
		ܐܢ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		ܐܚܪܢܐ
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} ܐܚܪܢܐ ܐܢ (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} ܐܚܪܢܐ ܐܢ (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} ܐܚܪܢܐ
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ܦܢܐ retval;
}

ساكن صحيح
delete(ثابت حرف *runlevel, ثابت حرف *service)
{
	صحيح retval = -1;

	errno = 0;
	ܐܢ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ܦܢܐ 1;
	}

	ܐܢ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	ܐܚܪܢܐ
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ܦܢܐ retval;
}

ساكن صحيح
addstack(ثابت حرف *runlevel, ثابت حرف *stack)
{
	ܐܢ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ܦܢܐ -1;
	}
	ܐܢ (!rc_runlevel_exists(stack)) {
		eerror("%s: runlevel `%s' does not exist", applet, stack);
		ܦܢܐ -1;
	}
	ܐܢ (strcmp(runlevel, stack) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, stack);
		ܦܢܐ -1;
	}
	ܐܢ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
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
		ܦܢܐ -1;
	}
	ܐܢ (!rc_runlevel_stack(runlevel, stack)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, stack, runlevel, strerror(errno));
		ܦܢܐ -1;
	}
	einfo("runlevel %s added to runlevel %s", stack, runlevel);
	ܦܢܐ 1;
}

ساكن صحيح
delstack(ثابت حرف *runlevel, ثابت حرف *stack)
{
	ܐܢ (rc_runlevel_unstack(runlevel, stack)) {
		einfo("runlevel %s deleted from runlevel %s", stack, runlevel);
		ܦܢܐ 1;
	}

	ܐܢ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, stack, runlevel);
	ܐܚܪܢܐ
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, stack, runlevel, strerror (errno));

	ܦܢܐ -1;
}

ساكن فراغ
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(عدم);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	حرف *buffer = عدم;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			ܐܢ (rc_service_in_runlevel(service->value,
				runlevel->value))
			{
				rc_stringlist_add(in, runlevel->value);
				inone = true;
			} ܐܚܪܢܐ {
				l = strlen(runlevel->value);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				حرر(buffer);
			}
		}

		ܐܢ (inone || verbose) {
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

صحيح main(صحيح argc, حرف **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	حرف *service = عدم;
	حرف *p;
	صحيح action = 0;
	bool verbose = false, stack = false, all_runlevels = false;
	صحيح opt;
	صحيح retval = EXIT_FAILURE;
	صحيح num_updated = 0;
	صحيح (*actfunc)(ثابت حرف *, ثابت حرف *);
	صحيح ret;

	applet = basename_c(argv[0]);
	ܟܡܐ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *)0)) != -1)
		بدل (opt) {
		حالة 'a':
		    all_runlevels = true;
		اكسر;
		حالة 's':
		    stack = true;
		اكسر;
		حالة 'u':
			deptree = _rc_deptree_load(-1, &ret);
			ܐܢ (deptree)
				rc_deptree_free(deptree);
			ܦܢܐ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ܐܢ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ܐܢ (optind < argc) {
		ܐܢ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		ܐܚܪܢܐ ܐܢ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		ܐܚܪܢܐ ܐܢ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ܐܢ (action)
			optind++;
		ܐܚܪܢܐ
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ܐܢ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ܐܢ (optind >= argc) {
		ܐܢ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} ܐܚܪܢܐ {
		service = argv[optind];
		optind++;

		ܟܡܐ (optind < argc)
			ܐܢ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			ܐܚܪܢܐ {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ܐܢ (action & DOSHOW) {
		ܐܢ (service)
			rc_stringlist_add(runlevels, service);
		ܐܢ (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} ܐܚܪܢܐ {
		ܐܢ (!service)
			eerror ("%s: no service specified", applet);
		ܐܚܪܢܐ {
			ܐܢ (action & DOADD) {
				ܐܢ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = stack ? addstack : add;
			} ܐܚܪܢܐ ܐܢ (action & DODELETE) {
				actfunc = stack ? delstack : delete;
			} ܐܚܪܢܐ {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ܐܢ (!TAILQ_FIRST(runlevels)) {
				ܐܢ (all_runlevels) {
					حرر(runlevels);
					runlevels = rc_runlevel_list();
				} ܐܚܪܢܐ {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					حرر(p);
				}
			}

			ܐܢ (!TAILQ_FIRST(runlevels)) {
				حرر(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ܐܢ (!rc_runlevel_exists(runlevel->value)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->value);
					استمر;
				}

				ret = actfunc(runlevel->value, service);
				ܐܢ (ret < 0)
					retval = EXIT_FAILURE;
				num_updated += ret;
			}

			ܐܢ (retval == EXIT_SUCCESS &&
			    num_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ܦܢܐ retval;
}
