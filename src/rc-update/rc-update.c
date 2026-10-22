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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <getopt.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
స్థిరరాశి అక్షరం getoptstring[] = "asu" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{ "all",             0, ఖాళీ, 'a' },
	{ "stack",           0, ఖాళీ, 's' },
	{ "update",          0, ఖాళీ, 'u' },
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిర పూర్ణాంకం
కూడు(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *service)
{
	పూర్ణాంకం retval = -1;

	ఒకవేళ (!rc_service_exists(service)) {
		ఒకవేళ (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		లేకపోతే
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} లేకపోతే ఒకవేళ (rc_service_లోపల_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} లేకపోతే ఒకవేళ (rc_service_కూడు(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} లేకపోతే
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	తిరిగి retval;
}

స్థిర పూర్ణాంకం
delete(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *service)
{
	పూర్ణాంకం retval = -1;

	errno = 0;
	ఒకవేళ (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		తిరిగి 1;
	}

	ఒకవేళ (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	లేకపోతే
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	తిరిగి retval;
}

స్థిర పూర్ణాంకం
addstack(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *దొంతర)
{
	ఒకవేళ (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		తిరిగి -1;
	}
	ఒకవేళ (!rc_runlevel_exists(దొంతర)) {
		eerror("%s: runlevel `%s' does not exist", applet, దొంతర);
		తిరిగి -1;
	}
	ఒకవేళ (strcmp(runlevel, దొంతర) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, దొంతర);
		తిరిగి -1;
	}
	ఒకవేళ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(దొంతర, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(దొంతర, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(దొంతర, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(దొంతర, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		తిరిగి -1;
	}
	ఒకవేళ (!rc_runlevel_దొంతర(runlevel, దొంతర)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, దొంతర, runlevel, strerror(errno));
		తిరిగి -1;
	}
	einfo("runlevel %s added to runlevel %s", దొంతర, runlevel);
	తిరిగి 1;
}

స్థిర పూర్ణాంకం
delstack(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *దొంతర)
{
	ఒకవేళ (rc_runlevel_unstack(runlevel, దొంతర)) {
		einfo("runlevel %s deleted from runlevel %s", దొంతర, runlevel);
		తిరిగి 1;
	}

	ఒకవేళ (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, దొంతర, runlevel);
	లేకపోతే
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, దొంతర, runlevel, strerror (errno));

	తిరిగి -1;
}

స్థిర శూన్య
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_లోపల_runlevel(ఖాళీ);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *లోపల;
	bool inone;
	అక్షరం *బఫరు = ఖాళీ;
	size_t l;

	rc_stringlist_క్రమీకరించు(&services);
	TAILQ_FOREACH(service, services, entries) {
		లోపల = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			ఒకవేళ (rc_service_లోపల_runlevel(service->విలువ,
				runlevel->విలువ))
			{
				rc_stringlist_కూడు(లోపల, runlevel->విలువ);
				inone = true;
			} లేకపోతే {
				l = అక్షరశ్రేణి_పొడవు(runlevel->విలువ);
				బఫరు = xmalloc(l+1);
				memset(బఫరు, ' ', l);
				బఫరు[l] = 0;
				rc_stringlist_కూడు(లోపల, బఫరు);
				విడుదల(బఫరు);
			}
		}

		ఒకవేళ (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			రాయి(" %26s |", service->విలువ);
			TAILQ_FOREACH(runlevel, లోపల, entries)
			    రాయి (" %s", runlevel->విలువ);
			రాయి("\n");
		}
		rc_stringlist_free(లోపల);
	}

	rc_stringlist_free(services);
}

#నిర్వచించు DOADD    (1 << 1)
#నిర్వచించు DODELETE (1 << 2)
#నిర్వచించు DOSHOW   (1 << 3)

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	అక్షరం *service = ఖాళీ;
	అక్షరం *p;
	పూర్ణాంకం action = 0;
	bool verbose = false, దొంతర = false, all_runlevels = false;
	పూర్ణాంకం opt;
	పూర్ణాంకం retval = EXIT_FAILURE;
	పూర్ణాంకం అంకె_updated = 0;
	పూర్ణాంకం (*actfunc)(స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);
	పూర్ణాంకం ret;

	applet = basename_c(argv[0]);
	అయితే ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (పూర్ణాంకం *)0)) != -1)
		ఎంపిక (opt) {
		స్థితి 'a':
		    all_runlevels = true;
		ఆపు;
		స్థితి 's':
		    దొంతర = true;
		ఆపు;
		స్థితి 'u':
			deptree = rc_deptree_ఎక్కించు_2(-1, &ret);
			ఒకవేళ (deptree)
				rc_deptree_free(deptree);
			తిరిగి ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ఒకవేళ ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ఒకవేళ (optind < argc) {
		ఒకవేళ (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		లేకపోతే ఒకవేళ (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		లేకపోతే ఒకవేళ (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ఒకవేళ (action)
			optind++;
		లేకపోతే
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ఒకవేళ (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ఒకవేళ (optind >= argc) {
		ఒకవేళ (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} లేకపోతే {
		service = argv[optind];
		optind++;

		అయితే (optind < argc)
			ఒకవేళ (rc_runlevel_exists(argv[optind]))
				rc_stringlist_కూడు(runlevels, argv[optind++]);
			లేకపోతే {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ఒకవేళ (action & DOSHOW) {
		ఒకవేళ (service)
			rc_stringlist_కూడు(runlevels, service);
		ఒకవేళ (!TAILQ_మొదటి(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_క్రమీకరించు(&runlevels);
		show (runlevels, verbose);
	} లేకపోతే {
		ఒకవేళ (!service)
			eerror ("%s: no service specified", applet);
		లేకపోతే {
			ఒకవేళ (action & DOADD) {
				ఒకవేళ (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = దొంతర ? addstack : కూడు;
			} లేకపోతే ఒకవేళ (action & DODELETE) {
				actfunc = దొంతర ? delstack : delete;
			} లేకపోతే {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ఒకవేళ (!TAILQ_మొదటి(runlevels)) {
				ఒకవేళ (all_runlevels) {
					విడుదల(runlevels);
					runlevels = rc_runlevel_list();
				} లేకపోతే {
					p = rc_runlevel_get();
					rc_stringlist_కూడు(runlevels,
					    p ? p : default_runlevel());
					విడుదల(p);
				}
			}

			ఒకవేళ (!TAILQ_మొదటి(runlevels)) {
				విడుదల(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ఒకవేళ (!rc_runlevel_exists(runlevel->విలువ)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->విలువ);
					కొనసాగు;
				}

				ret = actfunc(runlevel->విలువ, service);
				ఒకవేళ (ret < 0)
					retval = EXIT_FAILURE;
				అంకె_updated += ret;
			}

			ఒకవేళ (retval == EXIT_SUCCESS &&
			    అంకె_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	తిరిగి retval;
}
