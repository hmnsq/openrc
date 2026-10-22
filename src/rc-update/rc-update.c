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

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ಧ್ರುವ ವರ್ಣ getoptstring[] = "asu" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{ "all",             0, ರಿಕ್ತ, 'a' },
	{ "stack",           0, ರಿಕ್ತ, 's' },
	{ "update",          0, ರಿಕ್ತ, 'u' },
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
add(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *service)
{
	ಪೂರ್ಣಾಙ್ಕ retval = -1;

	ಯದಿ (!rc_service_exists(service)) {
		ಯದಿ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		ಅನ್ಯಥಾ
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} ಅನ್ಯಥಾ ಯದಿ (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} ಅನ್ಯಥಾ ಯದಿ (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} ಅನ್ಯಥಾ
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
delete(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *service)
{
	ಪೂರ್ಣಾಙ್ಕ retval = -1;

	errno = 0;
	ಯದಿ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಯದಿ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	ಅನ್ಯಥಾ
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ಪ್ರತ್ಯಾಗಮ retval;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
addstack(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *ರಾಶಿ)
{
	ಯದಿ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯದಿ (!rc_runlevel_exists(ರಾಶಿ)) {
		eerror("%s: runlevel `%s' does not exist", applet, ರಾಶಿ);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯದಿ (strcmp(runlevel, ರಾಶಿ) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ರಾಶಿ);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯದಿ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
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
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	ಯದಿ (!rc_runlevel_ರಾಶಿ(runlevel, ರಾಶಿ)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ರಾಶಿ, runlevel, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	einfo("runlevel %s added to runlevel %s", ರಾಶಿ, runlevel);
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
delstack(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *ರಾಶಿ)
{
	ಯದಿ (rc_runlevel_unstack(runlevel, ರಾಶಿ)) {
		einfo("runlevel %s deleted from runlevel %s", ರಾಶಿ, runlevel);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಯದಿ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ರಾಶಿ, runlevel);
	ಅನ್ಯಥಾ
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ರಾಶಿ, runlevel, strerror (errno));

	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಶೂನ್ಯ
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(ರಿಕ್ತ);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	ವರ್ಣ *buffer = ರಿಕ್ತ;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			ಯದಿ (rc_service_in_runlevel(service->ಮೌಲ್ಯ,
				runlevel->ಮೌಲ್ಯ))
			{
				rc_stringlist_add(in, runlevel->ಮೌಲ್ಯ);
				inone = true;
			} ಅನ್ಯಥಾ {
				l = strlen(runlevel->ಮೌಲ್ಯ);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				ಮುಕ್ತ(buffer);
			}
		}

		ಯದಿ (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->ಮೌಲ್ಯ);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->ಮೌಲ್ಯ);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	ವರ್ಣ *service = ರಿಕ್ತ;
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ action = 0;
	bool verbose = false, ರಾಶಿ = false, all_runlevels = false;
	ಪೂರ್ಣಾಙ್ಕ opt;
	ಪೂರ್ಣಾಙ್ಕ retval = EXIT_FAILURE;
	ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_updated = 0;
	ಪೂರ್ಣಾಙ್ಕ (*actfunc)(ಧ್ರುವ ವರ್ಣ *, ಧ್ರುವ ವರ್ಣ *);
	ಪೂರ್ಣಾಙ್ಕ ret;

	applet = basename_c(argv[0]);
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ಪೂರ್ಣಾಙ್ಕ *)0)) != -1)
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'a':
		    all_runlevels = true;
		ಭಙ್ಗ;
		ಪಕ್ಷ 's':
		    ರಾಶಿ = true;
		ಭಙ್ಗ;
		ಪಕ್ಷ 'u':
			deptree = _rc_deptree_load(-1, &ret);
			ಯದಿ (deptree)
				rc_deptree_free(deptree);
			ಪ್ರತ್ಯಾಗಮ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ಯದಿ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ಯದಿ (optind < argc) {
		ಯದಿ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		ಅನ್ಯಥಾ ಯದಿ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ಯದಿ (action)
			optind++;
		ಅನ್ಯಥಾ
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ಯದಿ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ಯದಿ (optind >= argc) {
		ಯದಿ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} ಅನ್ಯಥಾ {
		service = argv[optind];
		optind++;

		ಯಾವತ್ (optind < argc)
			ಯದಿ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			ಅನ್ಯಥಾ {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ಯದಿ (action & DOSHOW) {
		ಯದಿ (service)
			rc_stringlist_add(runlevels, service);
		ಯದಿ (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} ಅನ್ಯಥಾ {
		ಯದಿ (!service)
			eerror ("%s: no service specified", applet);
		ಅನ್ಯಥಾ {
			ಯದಿ (action & DOADD) {
				ಯದಿ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ರಾಶಿ ? addstack : add;
			} ಅನ್ಯಥಾ ಯದಿ (action & DODELETE) {
				actfunc = ರಾಶಿ ? delstack : delete;
			} ಅನ್ಯಥಾ {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ಯದಿ (!TAILQ_FIRST(runlevels)) {
				ಯದಿ (all_runlevels) {
					ಮುಕ್ತ(runlevels);
					runlevels = rc_runlevel_list();
				} ಅನ್ಯಥಾ {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					ಮುಕ್ತ(p);
				}
			}

			ಯದಿ (!TAILQ_FIRST(runlevels)) {
				ಮುಕ್ತ(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ಯದಿ (!rc_runlevel_exists(runlevel->ಮೌಲ್ಯ)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->ಮೌಲ್ಯ);
					ಅನುವರ್ತ;
				}

				ret = actfunc(runlevel->ಮೌಲ್ಯ, service);
				ಯದಿ (ret < 0)
					retval = EXIT_FAILURE;
				ಅಂಕೆ_updated += ret;
			}

			ಯದಿ (retval == EXIT_SUCCESS &&
			    ಅಂಕೆ_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ಪ್ರತ್ಯಾಗಮ retval;
}
