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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ଅଚଳ ଅକ୍ଷର getoptstring[] = "asu" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{ "all",             0, ଖାଲି, 'a' },
	{ "stack",           0, ଖାଲି, 's' },
	{ "update",          0, ଖାଲି, 'u' },
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
add(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *service)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = -1;

	ଯଦି (!rc_service_exists(service)) {
		ଯଦି (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		ଅନ୍ୟଥା
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} ଅନ୍ୟଥା ଯଦି (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} ଅନ୍ୟଥା ଯଦି (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} ଅନ୍ୟଥା
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
delete(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *service)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = -1;

	errno = 0;
	ଯଦି (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		ଫେରନ୍ତୁ 1;
	}

	ଯଦି (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	ଅନ୍ୟଥା
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	ଫେରନ୍ତୁ retval;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
addstack(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *ଗଦା)
{
	ଯଦି (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		ଫେରନ୍ତୁ -1;
	}
	ଯଦି (!rc_runlevel_exists(ଗଦା)) {
		eerror("%s: runlevel `%s' does not exist", applet, ଗଦା);
		ଫେରନ୍ତୁ -1;
	}
	ଯଦି (strcmp(runlevel, ଗଦା) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ଗଦା);
		ଫେରନ୍ତୁ -1;
	}
	ଯଦି (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(ଗଦା, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(ଗଦା, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(ଗଦା, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(ଗଦା, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		ଫେରନ୍ତୁ -1;
	}
	ଯଦି (!rc_runlevel_ଗଦା(runlevel, ଗଦା)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ଗଦା, runlevel, strerror(errno));
		ଫେରନ୍ତୁ -1;
	}
	einfo("runlevel %s added to runlevel %s", ଗଦା, runlevel);
	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
delstack(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *ଗଦା)
{
	ଯଦି (rc_runlevel_unstack(runlevel, ଗଦା)) {
		einfo("runlevel %s deleted from runlevel %s", ଗଦା, runlevel);
		ଫେରନ୍ତୁ 1;
	}

	ଯଦି (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ଗଦା, runlevel);
	ଅନ୍ୟଥା
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ଗଦା, runlevel, strerror (errno));

	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ଶୂନ୍ୟ
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(ଖାଲି);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	ଅକ୍ଷର *buffer = ଖାଲି;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			ଯଦି (rc_service_in_runlevel(service->ମୂଲ୍ୟ,
				runlevel->ମୂଲ୍ୟ))
			{
				rc_stringlist_add(in, runlevel->ମୂଲ୍ୟ);
				inone = true;
			} ଅନ୍ୟଥା {
				l = strlen(runlevel->ମୂଲ୍ୟ);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				ମୁକ୍ତ(buffer);
			}
		}

		ଯଦି (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->ମୂଲ୍ୟ);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->ମୂଲ୍ୟ);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	ଅକ୍ଷର *service = ଖାଲି;
	ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା action = 0;
	bool verbose = false, ଗଦା = false, all_runlevels = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval = EXIT_FAILURE;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_updated = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*actfunc)(ଅଚଳ ଅକ୍ଷର *, ଅଚଳ ଅକ୍ଷର *);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	applet = basename_c(argv[0]);
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)0)) != -1)
		ବାଛ (opt) {
		ସ୍ଥିତି 'a':
		    all_runlevels = true;
		ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
		    ଗଦା = true;
		ଭାଙ୍ଗ;
		ସ୍ଥିତି 'u':
			deptree = _rc_deptree_load(-1, &ret);
			ଯଦି (deptree)
				rc_deptree_free(deptree);
			ଫେରନ୍ତୁ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	ଯଦି ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	ଯଦି (optind < argc) {
		ଯଦି (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		ଅନ୍ୟଥା ଯଦି (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		ଅନ୍ୟଥା ଯଦି (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		ଯଦି (action)
			optind++;
		ଅନ୍ୟଥା
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	ଯଦି (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	ଯଦି (optind >= argc) {
		ଯଦି (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} ଅନ୍ୟଥା {
		service = argv[optind];
		optind++;

		ଯେତେବେଳେ (optind < argc)
			ଯଦି (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			ଅନ୍ୟଥା {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	ଯଦି (action & DOSHOW) {
		ଯଦି (service)
			rc_stringlist_add(runlevels, service);
		ଯଦି (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} ଅନ୍ୟଥା {
		ଯଦି (!service)
			eerror ("%s: no service specified", applet);
		ଅନ୍ୟଥା {
			ଯଦି (action & DOADD) {
				ଯଦି (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ଗଦା ? addstack : add;
			} ଅନ୍ୟଥା ଯଦି (action & DODELETE) {
				actfunc = ଗଦା ? delstack : delete;
			} ଅନ୍ୟଥା {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			ଯଦି (!TAILQ_FIRST(runlevels)) {
				ଯଦି (all_runlevels) {
					ମୁକ୍ତ(runlevels);
					runlevels = rc_runlevel_list();
				} ଅନ୍ୟଥା {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					ମୁକ୍ତ(p);
				}
			}

			ଯଦି (!TAILQ_FIRST(runlevels)) {
				ମୁକ୍ତ(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				ଯଦି (!rc_runlevel_exists(runlevel->ମୂଲ୍ୟ)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->ମୂଲ୍ୟ);
					ଜାରି;
				}

				ret = actfunc(runlevel->ମୂଲ୍ୟ, service);
				ଯଦି (ret < 0)
					retval = EXIT_FAILURE;
				ଅଙ୍କ_updated += ret;
			}

			ଯଦି (retval == EXIT_SUCCESS &&
			    ଅଙ୍କ_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	ଫେରନ୍ତୁ retval;
}
