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
جمع_کړه(ثابت حرف *runlevel, ثابت حرف *service)
{
	صحيح retval = -1;

	إذا (!rc_service_exists(service)) {
		إذا (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		وإلا
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} وإلا إذا (rc_service_دننه_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} وإلا إذا (rc_service_جمع_کړه(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} وإلا
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	أرجع retval;
}

ساكن صحيح
delete(ثابت حرف *runlevel, ثابت حرف *service)
{
	صحيح retval = -1;

	errno = 0;
	إذا (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		أرجع 1;
	}

	إذا (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	وإلا
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	أرجع retval;
}

ساكن صحيح
addstack(ثابت حرف *runlevel, ثابت حرف *ډېری)
{
	إذا (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		أرجع -1;
	}
	إذا (!rc_runlevel_exists(ډېری)) {
		eerror("%s: runlevel `%s' does not exist", applet, ډېری);
		أرجع -1;
	}
	إذا (strcmp(runlevel, ډېری) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ډېری);
		أرجع -1;
	}
	إذا (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(ډېری, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(ډېری, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(ډېری, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(ډېری, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		أرجع -1;
	}
	إذا (!rc_runlevel_ډېری(runlevel, ډېری)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ډېری, runlevel, strerror(errno));
		أرجع -1;
	}
	einfo("runlevel %s added to runlevel %s", ډېری, runlevel);
	أرجع 1;
}

ساكن صحيح
delstack(ثابت حرف *runlevel, ثابت حرف *ډېری)
{
	إذا (rc_runlevel_unstack(runlevel, ډېری)) {
		einfo("runlevel %s deleted from runlevel %s", ډېری, runlevel);
		أرجع 1;
	}

	إذا (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ډېری, runlevel);
	وإلا
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ډېری, runlevel, strerror (errno));

	أرجع -1;
}

ساكن فراغ
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_دننه_runlevel(عدم);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *دننه;
	bool inone;
	حرف *بفر = عدم;
	size_t l;

	rc_stringlist_ترتیبول(&services);
	TAILQ_FOREACH(service, services, entries) {
		دننه = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			إذا (rc_service_دننه_runlevel(service->ارزښت,
				runlevel->ارزښت))
			{
				rc_stringlist_جمع_کړه(دننه, runlevel->ارزښت);
				inone = true;
			} وإلا {
				l = مزي_اوږدوالی(runlevel->ارزښت);
				بفر = xmalloc(l+1);
				memset(بفر, ' ', l);
				بفر[l] = 0;
				rc_stringlist_جمع_کړه(دننه, بفر);
				حرر(بفر);
			}
		}

		إذا (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			چاپ_کړه(" %26s |", service->ارزښت);
			TAILQ_FOREACH(runlevel, دننه, entries)
			    چاپ_کړه (" %s", runlevel->ارزښت);
			چاپ_کړه("\n");
		}
		rc_stringlist_free(دننه);
	}

	rc_stringlist_free(services);
}

#تعریف_کړه DOADD    (1 << 1)
#تعریف_کړه DODELETE (1 << 2)
#تعریف_کړه DOSHOW   (1 << 3)

صحيح اصلي(صحيح argc, حرف **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	حرف *service = عدم;
	حرف *p;
	صحيح action = 0;
	bool verbose = false, ډېری = false, all_runlevels = false;
	صحيح opt;
	صحيح retval = EXIT_FAILURE;
	صحيح عدد_updated = 0;
	صحيح (*actfunc)(ثابت حرف *, ثابت حرف *);
	صحيح ret;

	applet = basename_c(argv[0]);
	طالما ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحيح *)0)) != -1)
		بدل (opt) {
		حالة 'a':
		    all_runlevels = true;
		اكسر;
		حالة 's':
		    ډېری = true;
		اكسر;
		حالة 'u':
			deptree = rc_deptree_بارول_2(-1, &ret);
			إذا (deptree)
				rc_deptree_free(deptree);
			أرجع ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	إذا ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	إذا (optind < argc) {
		إذا (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		وإلا إذا (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		وإلا إذا (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		إذا (action)
			optind++;
		وإلا
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	إذا (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	إذا (optind >= argc) {
		إذا (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} وإلا {
		service = argv[optind];
		optind++;

		طالما (optind < argc)
			إذا (rc_runlevel_exists(argv[optind]))
				rc_stringlist_جمع_کړه(runlevels, argv[optind++]);
			وإلا {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	إذا (action & DOSHOW) {
		إذا (service)
			rc_stringlist_جمع_کړه(runlevels, service);
		إذا (!TAILQ_لومړی(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_ترتیبول(&runlevels);
		show (runlevels, verbose);
	} وإلا {
		إذا (!service)
			eerror ("%s: no service specified", applet);
		وإلا {
			إذا (action & DOADD) {
				إذا (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ډېری ? addstack : جمع_کړه;
			} وإلا إذا (action & DODELETE) {
				actfunc = ډېری ? delstack : delete;
			} وإلا {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			إذا (!TAILQ_لومړی(runlevels)) {
				إذا (all_runlevels) {
					حرر(runlevels);
					runlevels = rc_runlevel_list();
				} وإلا {
					p = rc_runlevel_get();
					rc_stringlist_جمع_کړه(runlevels,
					    p ? p : default_runlevel());
					حرر(p);
				}
			}

			إذا (!TAILQ_لومړی(runlevels)) {
				حرر(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				إذا (!rc_runlevel_exists(runlevel->ارزښت)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->ارزښت);
					استمر;
				}

				ret = actfunc(runlevel->ارزښت, service);
				إذا (ret < 0)
					retval = EXIT_FAILURE;
				عدد_updated += ret;
			}

			إذا (retval == EXIT_SUCCESS &&
			    عدد_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	أرجع retval;
}
