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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
અચલ અક્ષર getoptstring[] = "asu" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{ "all",             0, ખાલી, 'a' },
	{ "stack",           0, ખાલી, 's' },
	{ "update",          0, ખાલી, 'u' },
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
સ્થિર પૂર્ણાંક
add(અચલ અક્ષર *runlevel, અચલ અક્ષર *service)
{
	પૂર્ણાંક retval = -1;

	જો (!rc_service_exists(service)) {
		જો (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		નહીંતર
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} નહીંતર જો (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} નહીંતર જો (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} નહીંતર
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	પાછા retval;
}

સ્થિર પૂર્ણાંક
delete(અચલ અક્ષર *runlevel, અચલ અક્ષર *service)
{
	પૂર્ણાંક retval = -1;

	errno = 0;
	જો (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		પાછા 1;
	}

	જો (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	નહીંતર
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	પાછા retval;
}

સ્થિર પૂર્ણાંક
addstack(અચલ અક્ષર *runlevel, અચલ અક્ષર *ઢગલો)
{
	જો (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		પાછા -1;
	}
	જો (!rc_runlevel_exists(ઢગલો)) {
		eerror("%s: runlevel `%s' does not exist", applet, ઢગલો);
		પાછા -1;
	}
	જો (strcmp(runlevel, ઢગલો) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ઢગલો);
		પાછા -1;
	}
	જો (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(ઢગલો, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(ઢગલો, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(ઢગલો, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(ઢગલો, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		પાછા -1;
	}
	જો (!rc_runlevel_ઢગલો(runlevel, ઢગલો)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ઢગલો, runlevel, strerror(errno));
		પાછા -1;
	}
	einfo("runlevel %s added to runlevel %s", ઢગલો, runlevel);
	પાછા 1;
}

સ્થિર પૂર્ણાંક
delstack(અચલ અક્ષર *runlevel, અચલ અક્ષર *ઢગલો)
{
	જો (rc_runlevel_unstack(runlevel, ઢગલો)) {
		einfo("runlevel %s deleted from runlevel %s", ઢગલો, runlevel);
		પાછા 1;
	}

	જો (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ઢગલો, runlevel);
	નહીંતર
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ઢગલો, runlevel, strerror (errno));

	પાછા -1;
}

સ્થિર શૂન્ય
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(ખાલી);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	અક્ષર *buffer = ખાલી;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			જો (rc_service_in_runlevel(service->કિંમત,
				runlevel->કિંમત))
			{
				rc_stringlist_add(in, runlevel->કિંમત);
				inone = true;
			} નહીંતર {
				l = strlen(runlevel->કિંમત);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				મુક્ત(buffer);
			}
		}

		જો (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->કિંમત);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->કિંમત);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	અક્ષર *service = ખાલી;
	અક્ષર *p;
	પૂર્ણાંક action = 0;
	bool verbose = false, ઢગલો = false, all_runlevels = false;
	પૂર્ણાંક opt;
	પૂર્ણાંક retval = EXIT_FAILURE;
	પૂર્ણાંક અંક_updated = 0;
	પૂર્ણાંક (*actfunc)(અચલ અક્ષર *, અચલ અક્ષર *);
	પૂર્ણાંક ret;

	applet = basename_c(argv[0]);
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (પૂર્ણાંક *)0)) != -1)
		પસંદગી (opt) {
		સ્થિતિ 'a':
		    all_runlevels = true;
		તોડો;
		સ્થિતિ 's':
		    ઢગલો = true;
		તોડો;
		સ્થિતિ 'u':
			deptree = _rc_deptree_load(-1, &ret);
			જો (deptree)
				rc_deptree_free(deptree);
			પાછા ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	જો ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	જો (optind < argc) {
		જો (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		નહીંતર જો (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		નહીંતર જો (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		જો (action)
			optind++;
		નહીંતર
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	જો (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	જો (optind >= argc) {
		જો (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} નહીંતર {
		service = argv[optind];
		optind++;

		જ્યારે (optind < argc)
			જો (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			નહીંતર {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	જો (action & DOSHOW) {
		જો (service)
			rc_stringlist_add(runlevels, service);
		જો (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} નહીંતર {
		જો (!service)
			eerror ("%s: no service specified", applet);
		નહીંતર {
			જો (action & DOADD) {
				જો (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ઢગલો ? addstack : add;
			} નહીંતર જો (action & DODELETE) {
				actfunc = ઢગલો ? delstack : delete;
			} નહીંતર {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			જો (!TAILQ_FIRST(runlevels)) {
				જો (all_runlevels) {
					મુક્ત(runlevels);
					runlevels = rc_runlevel_list();
				} નહીંતર {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					મુક્ત(p);
				}
			}

			જો (!TAILQ_FIRST(runlevels)) {
				મુક્ત(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				જો (!rc_runlevel_exists(runlevel->કિંમત)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->કિંમત);
					ચાલુ;
				}

				ret = actfunc(runlevel->કિંમત, service);
				જો (ret < 0)
					retval = EXIT_FAILURE;
				અંક_updated += ret;
			}

			જો (retval == EXIT_SUCCESS &&
			    અંક_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	પાછા retval;
}
