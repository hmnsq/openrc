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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

ধ্রুব বর্ণ *applet = রিক্ত;
ধ্রুব বর্ণ *extraopts = রিক্ত;
ধ্রুব বর্ণ *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ধ্রুব বর্ণ getoptstring[] = "asu" getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{ "all",             0, রিক্ত, 'a' },
	{ "stack",           0, রিক্ত, 's' },
	{ "update",          0, রিক্ত, 'u' },
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
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
স্থির পূর্ণাঙ্ক
add(ধ্রুব বর্ণ *runlevel, ধ্রুব বর্ণ *service)
{
	পূর্ণাঙ্ক retval = -1;

	যদি (!rc_service_exists(service)) {
		যদি (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		অন্যথা
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} অন্যথা যদি (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} অন্যথা যদি (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} অন্যথা
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	প্রত্যাগম retval;
}

স্থির পূর্ণাঙ্ক
delete(ধ্রুব বর্ণ *runlevel, ধ্রুব বর্ণ *service)
{
	পূর্ণাঙ্ক retval = -1;

	errno = 0;
	যদি (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		প্রত্যাগম 1;
	}

	যদি (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	অন্যথা
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	প্রত্যাগম retval;
}

স্থির পূর্ণাঙ্ক
addstack(ধ্রুব বর্ণ *runlevel, ধ্রুব বর্ণ *স্তূপ)
{
	যদি (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		প্রত্যাগম -1;
	}
	যদি (!rc_runlevel_exists(স্তূপ)) {
		eerror("%s: runlevel `%s' does not exist", applet, স্তূপ);
		প্রত্যাগম -1;
	}
	যদি (strcmp(runlevel, স্তূপ) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, স্তূপ);
		প্রত্যাগম -1;
	}
	যদি (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(স্তূপ, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(স্তূপ, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(স্তূপ, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(স্তূপ, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		প্রত্যাগম -1;
	}
	যদি (!rc_runlevel_স্তূপ(runlevel, স্তূপ)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, স্তূপ, runlevel, strerror(errno));
		প্রত্যাগম -1;
	}
	einfo("runlevel %s added to runlevel %s", স্তূপ, runlevel);
	প্রত্যাগম 1;
}

স্থির পূর্ণাঙ্ক
delstack(ধ্রুব বর্ণ *runlevel, ধ্রুব বর্ণ *স্তূপ)
{
	যদি (rc_runlevel_unstack(runlevel, স্তূপ)) {
		einfo("runlevel %s deleted from runlevel %s", স্তূপ, runlevel);
		প্রত্যাগম 1;
	}

	যদি (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, স্তূপ, runlevel);
	অন্যথা
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, স্তূপ, runlevel, strerror (errno));

	প্রত্যাগম -1;
}

স্থির শূন্য
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(রিক্ত);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	বর্ণ *buffer = রিক্ত;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			যদি (rc_service_in_runlevel(service->মান,
				runlevel->মান))
			{
				rc_stringlist_add(in, runlevel->মান);
				inone = true;
			} অন্যথা {
				l = strlen(runlevel->মান);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				মুক্ত(buffer);
			}
		}

		যদি (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->মান);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->মান);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	বর্ণ *service = রিক্ত;
	বর্ণ *p;
	পূর্ণাঙ্ক action = 0;
	bool verbose = false, স্তূপ = false, all_runlevels = false;
	পূর্ণাঙ্ক opt;
	পূর্ণাঙ্ক retval = EXIT_FAILURE;
	পূর্ণাঙ্ক অঙ্ক_updated = 0;
	পূর্ণাঙ্ক (*actfunc)(ধ্রুব বর্ণ *, ধ্রুব বর্ণ *);
	পূর্ণাঙ্ক ret;

	applet = basename_c(argv[0]);
	যাবত্ ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (পূর্ণাঙ্ক *)0)) != -1)
		বিকল্প (opt) {
		পক্ষ 'a':
		    all_runlevels = true;
		ভঙ্গ;
		পক্ষ 's':
		    স্তূপ = true;
		ভঙ্গ;
		পক্ষ 'u':
			deptree = _rc_deptree_load(-1, &ret);
			যদি (deptree)
				rc_deptree_free(deptree);
			প্রত্যাগম ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	যদি ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	যদি (optind < argc) {
		যদি (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		অন্যথা যদি (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		অন্যথা যদি (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		যদি (action)
			optind++;
		অন্যথা
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	যদি (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	যদি (optind >= argc) {
		যদি (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} অন্যথা {
		service = argv[optind];
		optind++;

		যাবত্ (optind < argc)
			যদি (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			অন্যথা {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	যদি (action & DOSHOW) {
		যদি (service)
			rc_stringlist_add(runlevels, service);
		যদি (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} অন্যথা {
		যদি (!service)
			eerror ("%s: no service specified", applet);
		অন্যথা {
			যদি (action & DOADD) {
				যদি (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = স্তূপ ? addstack : add;
			} অন্যথা যদি (action & DODELETE) {
				actfunc = স্তূপ ? delstack : delete;
			} অন্যথা {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			যদি (!TAILQ_FIRST(runlevels)) {
				যদি (all_runlevels) {
					মুক্ত(runlevels);
					runlevels = rc_runlevel_list();
				} অন্যথা {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					মুক্ত(p);
				}
			}

			যদি (!TAILQ_FIRST(runlevels)) {
				মুক্ত(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				যদি (!rc_runlevel_exists(runlevel->মান)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->মান);
					অনুবর্ত;
				}

				ret = actfunc(runlevel->মান, service);
				যদি (ret < 0)
					retval = EXIT_FAILURE;
				অঙ্ক_updated += ret;
			}

			যদি (retval == EXIT_SUCCESS &&
			    অঙ্ক_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	প্রত্যাগম retval;
}
