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
ਅਚਲ ਅੱਖਰ *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ਅਚਲ ਅੱਖਰ getoptstring[] = "asu" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{ "all",             0, ਨਿਰਾ, 'a' },
	{ "stack",           0, ਨਿਰਾ, 's' },
	{ "update",          0, ਨਿਰਾ, 'u' },
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਸਥਿਰ ਪੂਰਨ
ਜੋੜੋ(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *service)
{
	ਪੂਰਨ retval = -1;

	ਜੇ (!rc_service_exists(service)) {
		ਜੇ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		ਨਹੀਂਤਾਂ
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} ਨਹੀਂਤਾਂ ਜੇ (rc_service_ਵਿੱਚ_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} ਨਹੀਂਤਾਂ ਜੇ (rc_service_ਜੋੜੋ(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} ਨਹੀਂਤਾਂ
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ਵਾਪਸ retval;
}

ਸਥਿਰ ਪੂਰਨ
delete(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *service)
{
	ਪੂਰਨ retval = -1;

	errno = 0;
	ਜੇ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ਵਾਪਸ 1;
	}

	ਜੇ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	ਨਹੀਂਤਾਂ
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ਵਾਪਸ retval;
}

ਸਥਿਰ ਪੂਰਨ
addstack(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *ਢੇਰ)
{
	ਜੇ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ਵਾਪਸ -1;
	}
	ਜੇ (!rc_runlevel_exists(ਢੇਰ)) {
		eerror("%s: runlevel `%s' does not exist", applet, ਢੇਰ);
		ਵਾਪਸ -1;
	}
	ਜੇ (strcmp(runlevel, ਢੇਰ) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ਢੇਰ);
		ਵਾਪਸ -1;
	}
	ਜੇ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(ਢੇਰ, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(ਢੇਰ, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(ਢੇਰ, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(ਢੇਰ, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		ਵਾਪਸ -1;
	}
	ਜੇ (!rc_runlevel_ਢੇਰ(runlevel, ਢੇਰ)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ਢੇਰ, runlevel, strerror(errno));
		ਵਾਪਸ -1;
	}
	einfo("runlevel %s added to runlevel %s", ਢੇਰ, runlevel);
	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ
delstack(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *ਢੇਰ)
{
	ਜੇ (rc_runlevel_unstack(runlevel, ਢੇਰ)) {
		einfo("runlevel %s deleted from runlevel %s", ਢੇਰ, runlevel);
		ਵਾਪਸ 1;
	}

	ਜੇ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ਢੇਰ, runlevel);
	ਨਹੀਂਤਾਂ
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ਢੇਰ, runlevel, strerror (errno));

	ਵਾਪਸ -1;
}

ਸਥਿਰ ਖਾਲੀ
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *ਵਿੱਚ;
	bool inone;
	ਅੱਖਰ *ਬਫਰ = ਨਿਰਾ;
	size_t l;

	rc_stringlist_ਛਾਂਟੋ(&services);
	TAILQ_FOREACH(service, services, entries) {
		ਵਿੱਚ = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			ਜੇ (rc_service_ਵਿੱਚ_runlevel(service->ਮੁੱਲ,
				runlevel->ਮੁੱਲ))
			{
				rc_stringlist_ਜੋੜੋ(ਵਿੱਚ, runlevel->ਮੁੱਲ);
				inone = true;
			} ਨਹੀਂਤਾਂ {
				l = ਲੜੀ_ਲੰਬਾਈ(runlevel->ਮੁੱਲ);
				ਬਫਰ = xmalloc(l+1);
				memset(ਬਫਰ, ' ', l);
				ਬਫਰ[l] = 0;
				rc_stringlist_ਜੋੜੋ(ਵਿੱਚ, ਬਫਰ);
				ਮੁਕਤ(ਬਫਰ);
			}
		}

		ਜੇ (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			ਲਿਖੋ(" %26s |", service->ਮੁੱਲ);
			TAILQ_FOREACH(runlevel, ਵਿੱਚ, entries)
			    ਲਿਖੋ (" %s", runlevel->ਮੁੱਲ);
			ਲਿਖੋ("\n");
		}
		rc_stringlist_free(ਵਿੱਚ);
	}

	rc_stringlist_free(services);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DOADD    (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DODELETE (1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DOSHOW   (1 << 3)

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	ਅੱਖਰ *service = ਨਿਰਾ;
	ਅੱਖਰ *p;
	ਪੂਰਨ action = 0;
	bool verbose = false, ਢੇਰ = false, all_runlevels = false;
	ਪੂਰਨ opt;
	ਪੂਰਨ retval = EXIT_FAILURE;
	ਪੂਰਨ ਅੰਕ_updated = 0;
	ਪੂਰਨ (*actfunc)(ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *);
	ਪੂਰਨ ret;

	applet = basename_c(argv[0]);
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ਪੂਰਨ *)0)) != -1)
		ਚੋਣ (opt) {
		ਹਾਲਤ 'a':
		    all_runlevels = true;
		ਰੋਕੋ;
		ਹਾਲਤ 's':
		    ਢੇਰ = true;
		ਰੋਕੋ;
		ਹਾਲਤ 'u':
			deptree = rc_deptree_ਲੱਦੋ_2(-1, &ret);
			ਜੇ (deptree)
				rc_deptree_free(deptree);
			ਵਾਪਸ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ਜੇ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ਜੇ (optind < argc) {
		ਜੇ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		ਨਹੀਂਤਾਂ ਜੇ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ਜੇ (action)
			optind++;
		ਨਹੀਂਤਾਂ
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ਜੇ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ਜੇ (optind >= argc) {
		ਜੇ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} ਨਹੀਂਤਾਂ {
		service = argv[optind];
		optind++;

		ਜਦੋਂ (optind < argc)
			ਜੇ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_ਜੋੜੋ(runlevels, argv[optind++]);
			ਨਹੀਂਤਾਂ {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ਜੇ (action & DOSHOW) {
		ਜੇ (service)
			rc_stringlist_ਜੋੜੋ(runlevels, service);
		ਜੇ (!TAILQ_ਪਹਿਲਾ(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_ਛਾਂਟੋ(&runlevels);
		show (runlevels, verbose);
	} ਨਹੀਂਤਾਂ {
		ਜੇ (!service)
			eerror ("%s: no service specified", applet);
		ਨਹੀਂਤਾਂ {
			ਜੇ (action & DOADD) {
				ਜੇ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ਢੇਰ ? addstack : ਜੋੜੋ;
			} ਨਹੀਂਤਾਂ ਜੇ (action & DODELETE) {
				actfunc = ਢੇਰ ? delstack : delete;
			} ਨਹੀਂਤਾਂ {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ਜੇ (!TAILQ_ਪਹਿਲਾ(runlevels)) {
				ਜੇ (all_runlevels) {
					ਮੁਕਤ(runlevels);
					runlevels = rc_runlevel_list();
				} ਨਹੀਂਤਾਂ {
					p = rc_runlevel_get();
					rc_stringlist_ਜੋੜੋ(runlevels,
					    p ? p : default_runlevel());
					ਮੁਕਤ(p);
				}
			}

			ਜੇ (!TAILQ_ਪਹਿਲਾ(runlevels)) {
				ਮੁਕਤ(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ਜੇ (!rc_runlevel_exists(runlevel->ਮੁੱਲ)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->ਮੁੱਲ);
					ਜਾਰੀ;
				}

				ret = actfunc(runlevel->ਮੁੱਲ, service);
				ਜੇ (ret < 0)
					retval = EXIT_FAILURE;
				ਅੰਕ_updated += ret;
			}

			ਜੇ (retval == EXIT_SUCCESS &&
			    ਅੰਕ_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ਵਾਪਸ retval;
}
