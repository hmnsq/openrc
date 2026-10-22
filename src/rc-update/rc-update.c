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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "asu" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{ "all",             0, ಖಾಲಿ, 'a' },
	{ "stack",           0, ಖಾಲಿ, 's' },
	{ "update",          0, ಖಾಲಿ, 'u' },
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ಕೂಡಿಸು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಪೂರ್ಣಾಂಕ retval = -1;

	ಆದರೆ (!rc_service_exists(service)) {
		ಆದರೆ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		ಇಲ್ಲದಿದ್ದರೆ
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rc_service_ಒಳ_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rc_service_ಕೂಡಿಸು(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} ಇಲ್ಲದಿದ್ದರೆ
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
delete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	ಪೂರ್ಣಾಂಕ retval = -1;

	errno = 0;
	ಆದರೆ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಆದರೆ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	ಇಲ್ಲದಿದ್ದರೆ
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ಹಿಂತಿರುಗಿ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
addstack(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ರಾಶಿ)
{
	ಆದರೆ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆದರೆ (!rc_runlevel_exists(ರಾಶಿ)) {
		eerror("%s: runlevel `%s' does not exist", applet, ರಾಶಿ);
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆದರೆ (strcmp(runlevel, ರಾಶಿ) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ರಾಶಿ);
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆದರೆ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(ರಾಶಿ, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(ರಾಶಿ, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(ರಾಶಿ, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(ರಾಶಿ, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		ಹಿಂತಿರುಗಿ -1;
	}
	ಆದರೆ (!rc_runlevel_ರಾಶಿ(runlevel, ರಾಶಿ)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ರಾಶಿ, runlevel, strerror(errno));
		ಹಿಂತಿರುಗಿ -1;
	}
	einfo("runlevel %s added to runlevel %s", ರಾಶಿ, runlevel);
	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
delstack(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ರಾಶಿ)
{
	ಆದರೆ (rc_runlevel_unstack(runlevel, ರಾಶಿ)) {
		einfo("runlevel %s deleted from runlevel %s", ರಾಶಿ, runlevel);
		ಹಿಂತಿರುಗಿ 1;
	}

	ಆದರೆ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ರಾಶಿ, runlevel);
	ಇಲ್ಲದಿದ್ದರೆ
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ರಾಶಿ, runlevel, strerror (errno));

	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *ಒಳ;
	bool inone;
	ಅಕ್ಷರ *ಬಫರ್ = ಖಾಲಿ;
	size_t l;

	rc_stringlist_ವಿಂಗಡಿಸು(&services);
	TAILQ_FOREACH(service, services, entries) {
		ಒಳ = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			ಆದರೆ (rc_service_ಒಳ_runlevel(service->ಮೌಲ್ಯ,
				runlevel->ಮೌಲ್ಯ))
			{
				rc_stringlist_ಕೂಡಿಸು(ಒಳ, runlevel->ಮೌಲ್ಯ);
				inone = true;
			} ಇಲ್ಲದಿದ್ದರೆ {
				l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(runlevel->ಮೌಲ್ಯ);
				ಬಫರ್ = xmalloc(l+1);
				memset(ಬಫರ್, ' ', l);
				ಬಫರ್[l] = 0;
				rc_stringlist_ಕೂಡಿಸು(ಒಳ, ಬಫರ್);
				ಬಿಡುಗಡೆ(ಬಫರ್);
			}
		}

		ಆದರೆ (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			ಬರೆ(" %26s |", service->ಮೌಲ್ಯ);
			TAILQ_FOREACH(runlevel, ಒಳ, entries)
			    ಬರೆ (" %s", runlevel->ಮೌಲ್ಯ);
			ಬರೆ("\n");
		}
		rc_stringlist_free(ಒಳ);
	}

	rc_stringlist_free(services);
}

#ವ್ಯಾಖ್ಯಾನಿಸು DOADD    (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು DODELETE (1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು DOSHOW   (1 << 3)

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	ಅಕ್ಷರ *service = ಖಾಲಿ;
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ action = 0;
	bool verbose = false, ರಾಶಿ = false, all_runlevels = false;
	ಪೂರ್ಣಾಂಕ opt;
	ಪೂರ್ಣಾಂಕ retval = EXIT_FAILURE;
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ_updated = 0;
	ಪೂರ್ಣಾಂಕ (*actfunc)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
	ಪೂರ್ಣಾಂಕ ret;

	applet = basename_c(argv[0]);
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಂಕ *)0)) != -1)
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'a':
		    all_runlevels = true;
		ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
		    ರಾಶಿ = true;
		ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'u':
			deptree = rc_deptree_ತುಂಬು_2(-1, &ret);
			ಆದರೆ (deptree)
				rc_deptree_free(deptree);
			ಹಿಂತಿರುಗಿ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ಆದರೆ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ಆದರೆ (optind < argc) {
		ಆದರೆ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ಆದರೆ (action)
			optind++;
		ಇಲ್ಲದಿದ್ದರೆ
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ಆದರೆ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ಆದರೆ (optind >= argc) {
		ಆದರೆ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} ಇಲ್ಲದಿದ್ದರೆ {
		service = argv[optind];
		optind++;

		ಆಗ (optind < argc)
			ಆದರೆ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_ಕೂಡಿಸು(runlevels, argv[optind++]);
			ಇಲ್ಲದಿದ್ದರೆ {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ಆದರೆ (action & DOSHOW) {
		ಆದರೆ (service)
			rc_stringlist_ಕೂಡಿಸು(runlevels, service);
		ಆದರೆ (!TAILQ_ಮೊದಲ(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_ವಿಂಗಡಿಸು(&runlevels);
		show (runlevels, verbose);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (!service)
			eerror ("%s: no service specified", applet);
		ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (action & DOADD) {
				ಆದರೆ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ರಾಶಿ ? addstack : ಕೂಡಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (action & DODELETE) {
				actfunc = ರಾಶಿ ? delstack : delete;
			} ಇಲ್ಲದಿದ್ದರೆ {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ಆದರೆ (!TAILQ_ಮೊದಲ(runlevels)) {
				ಆದರೆ (all_runlevels) {
					ಬಿಡುಗಡೆ(runlevels);
					runlevels = rc_runlevel_list();
				} ಇಲ್ಲದಿದ್ದರೆ {
					p = rc_runlevel_get();
					rc_stringlist_ಕೂಡಿಸು(runlevels,
					    p ? p : default_runlevel());
					ಬಿಡುಗಡೆ(p);
				}
			}

			ಆದರೆ (!TAILQ_ಮೊದಲ(runlevels)) {
				ಬಿಡುಗಡೆ(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ಆದರೆ (!rc_runlevel_exists(runlevel->ಮೌಲ್ಯ)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->ಮೌಲ್ಯ);
					ಮುಂದುವರಿ;
				}

				ret = actfunc(runlevel->ಮೌಲ್ಯ, service);
				ಆದರೆ (ret < 0)
					retval = EXIT_FAILURE;
				ಅಂಕೆ_updated += ret;
			}

			ಆದರೆ (retval == EXIT_SUCCESS &&
			    ಅಂಕೆ_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ಹಿಂತಿರುಗಿ retval;
}
