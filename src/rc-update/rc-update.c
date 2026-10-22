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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
மாறிலி எழுத்து getoptstring[] = "asu" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{ "all",             0, வெற்று, 'a' },
	{ "stack",           0, வெற்று, 's' },
	{ "update",          0, வெற்று, 'u' },
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
நிலையான முழு
add(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *service)
{
	முழு retval = -1;

	எனில் (!rc_service_exists(service)) {
		எனில் (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		இல்லையெனில்
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} இல்லையெனில் எனில் (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} இல்லையெனில் எனில் (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} இல்லையெனில்
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	திரும்பு retval;
}

நிலையான முழு
delete(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *service)
{
	முழு retval = -1;

	errno = 0;
	எனில் (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		திரும்பு 1;
	}

	எனில் (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	இல்லையெனில்
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	திரும்பு retval;
}

நிலையான முழு
addstack(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *அடுக்கு)
{
	எனில் (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		திரும்பு -1;
	}
	எனில் (!rc_runlevel_exists(அடுக்கு)) {
		eerror("%s: runlevel `%s' does not exist", applet, அடுக்கு);
		திரும்பு -1;
	}
	எனில் (strcmp(runlevel, அடுக்கு) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, அடுக்கு);
		திரும்பு -1;
	}
	எனில் (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(அடுக்கு, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(அடுக்கு, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(அடுக்கு, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(அடுக்கு, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		திரும்பு -1;
	}
	எனில் (!rc_runlevel_அடுக்கு(runlevel, அடுக்கு)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, அடுக்கு, runlevel, strerror(errno));
		திரும்பு -1;
	}
	einfo("runlevel %s added to runlevel %s", அடுக்கு, runlevel);
	திரும்பு 1;
}

நிலையான முழு
delstack(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *அடுக்கு)
{
	எனில் (rc_runlevel_unstack(runlevel, அடுக்கு)) {
		einfo("runlevel %s deleted from runlevel %s", அடுக்கு, runlevel);
		திரும்பு 1;
	}

	எனில் (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, அடுக்கு, runlevel);
	இல்லையெனில்
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, அடுக்கு, runlevel, strerror (errno));

	திரும்பு -1;
}

நிலையான வெறுமை
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(வெற்று);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	எழுத்து *buffer = வெற்று;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			எனில் (rc_service_in_runlevel(service->மதிப்பு,
				runlevel->மதிப்பு))
			{
				rc_stringlist_add(in, runlevel->மதிப்பு);
				inone = true;
			} இல்லையெனில் {
				l = strlen(runlevel->மதிப்பு);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				விடுவி(buffer);
			}
		}

		எனில் (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->மதிப்பு);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->மதிப்பு);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	எழுத்து *service = வெற்று;
	எழுத்து *p;
	முழு action = 0;
	bool verbose = false, அடுக்கு = false, all_runlevels = false;
	முழு opt;
	முழு retval = EXIT_FAILURE;
	முழு இலக்கம்_updated = 0;
	முழு (*actfunc)(மாறிலி எழுத்து *, மாறிலி எழுத்து *);
	முழு ret;

	applet = basename_c(argv[0]);
	போது ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (முழு *)0)) != -1)
		தேர்வு (opt) {
		நிலை 'a':
		    all_runlevels = true;
		நிறுத்து;
		நிலை 's':
		    அடுக்கு = true;
		நிறுத்து;
		நிலை 'u':
			deptree = _rc_deptree_load(-1, &ret);
			எனில் (deptree)
				rc_deptree_free(deptree);
			திரும்பு ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	எனில் ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	எனில் (optind < argc) {
		எனில் (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		இல்லையெனில் எனில் (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		இல்லையெனில் எனில் (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		எனில் (action)
			optind++;
		இல்லையெனில்
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	எனில் (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	எனில் (optind >= argc) {
		எனில் (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} இல்லையெனில் {
		service = argv[optind];
		optind++;

		போது (optind < argc)
			எனில் (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			இல்லையெனில் {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	எனில் (action & DOSHOW) {
		எனில் (service)
			rc_stringlist_add(runlevels, service);
		எனில் (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} இல்லையெனில் {
		எனில் (!service)
			eerror ("%s: no service specified", applet);
		இல்லையெனில் {
			எனில் (action & DOADD) {
				எனில் (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = அடுக்கு ? addstack : add;
			} இல்லையெனில் எனில் (action & DODELETE) {
				actfunc = அடுக்கு ? delstack : delete;
			} இல்லையெனில் {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			எனில் (!TAILQ_FIRST(runlevels)) {
				எனில் (all_runlevels) {
					விடுவி(runlevels);
					runlevels = rc_runlevel_list();
				} இல்லையெனில் {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					விடுவி(p);
				}
			}

			எனில் (!TAILQ_FIRST(runlevels)) {
				விடுவி(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				எனில் (!rc_runlevel_exists(runlevel->மதிப்பு)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->மதிப்பு);
					தொடர்;
				}

				ret = actfunc(runlevel->மதிப்பு, service);
				எனில் (ret < 0)
					retval = EXIT_FAILURE;
				இலக்கம்_updated += ret;
			}

			எனில் (retval == EXIT_SUCCESS &&
			    இலக்கம்_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	திரும்பு retval;
}
