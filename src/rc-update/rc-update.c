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

#شامل <errno.h>
#شامل <getopt.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "_usage.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ثابت نویسه getoptstring[] = "asu" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{ "all",             0, پوچ, 'a' },
	{ "stack",           0, پوچ, 's' },
	{ "update",          0, پوچ, 'u' },
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ایستا صحیح
add(ثابت نویسه *runlevel, ثابت نویسه *service)
{
	صحیح retval = -1;

	اگر (!rc_service_exists(service)) {
		اگر (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		وگرنه
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} وگرنه اگر (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} وگرنه اگر (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} وگرنه
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	بازگشت retval;
}

ایستا صحیح
delete(ثابت نویسه *runlevel, ثابت نویسه *service)
{
	صحیح retval = -1;

	errno = 0;
	اگر (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		بازگشت 1;
	}

	اگر (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	وگرنه
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	بازگشت retval;
}

ایستا صحیح
addstack(ثابت نویسه *runlevel, ثابت نویسه *پشته)
{
	اگر (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		بازگشت -1;
	}
	اگر (!rc_runlevel_exists(پشته)) {
		eerror("%s: runlevel `%s' does not exist", applet, پشته);
		بازگشت -1;
	}
	اگر (strcmp(runlevel, پشته) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, پشته);
		بازگشت -1;
	}
	اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(پشته, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(پشته, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(پشته, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(پشته, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		بازگشت -1;
	}
	اگر (!rc_runlevel_پشته(runlevel, پشته)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, پشته, runlevel, strerror(errno));
		بازگشت -1;
	}
	einfo("runlevel %s added to runlevel %s", پشته, runlevel);
	بازگشت 1;
}

ایستا صحیح
delstack(ثابت نویسه *runlevel, ثابت نویسه *پشته)
{
	اگر (rc_runlevel_unstack(runlevel, پشته)) {
		einfo("runlevel %s deleted from runlevel %s", پشته, runlevel);
		بازگشت 1;
	}

	اگر (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, پشته, runlevel);
	وگرنه
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, پشته, runlevel, strerror (errno));

	بازگشت -1;
}

ایستا تهی
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(پوچ);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	نویسه *buffer = پوچ;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			اگر (rc_service_in_runlevel(service->مقدار,
				runlevel->مقدار))
			{
				rc_stringlist_add(in, runlevel->مقدار);
				inone = true;
			} وگرنه {
				l = strlen(runlevel->مقدار);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				آزاد(buffer);
			}
		}

		اگر (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->مقدار);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->مقدار);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

صحیح اصلی(صحیح argc, نویسه **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	نویسه *service = پوچ;
	نویسه *p;
	صحیح action = 0;
	bool verbose = false, پشته = false, all_runlevels = false;
	صحیح opt;
	صحیح retval = EXIT_FAILURE;
	صحیح رقم_updated = 0;
	صحیح (*actfunc)(ثابت نویسه *, ثابت نویسه *);
	صحیح ret;

	applet = basename_c(argv[0]);
	وقتی ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (صحیح *)0)) != -1)
		انتخاب (opt) {
		حالت 'a':
		    all_runlevels = true;
		بشکن;
		حالت 's':
		    پشته = true;
		بشکن;
		حالت 'u':
			deptree = _rc_deptree_load(-1, &ret);
			اگر (deptree)
				rc_deptree_free(deptree);
			بازگشت ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	اگر ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	اگر (optind < argc) {
		اگر (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		وگرنه اگر (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		وگرنه اگر (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		اگر (action)
			optind++;
		وگرنه
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	اگر (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	اگر (optind >= argc) {
		اگر (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} وگرنه {
		service = argv[optind];
		optind++;

		وقتی (optind < argc)
			اگر (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			وگرنه {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	اگر (action & DOSHOW) {
		اگر (service)
			rc_stringlist_add(runlevels, service);
		اگر (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} وگرنه {
		اگر (!service)
			eerror ("%s: no service specified", applet);
		وگرنه {
			اگر (action & DOADD) {
				اگر (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = پشته ? addstack : add;
			} وگرنه اگر (action & DODELETE) {
				actfunc = پشته ? delstack : delete;
			} وگرنه {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			اگر (!TAILQ_FIRST(runlevels)) {
				اگر (all_runlevels) {
					آزاد(runlevels);
					runlevels = rc_runlevel_list();
				} وگرنه {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					آزاد(p);
				}
			}

			اگر (!TAILQ_FIRST(runlevels)) {
				آزاد(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				اگر (!rc_runlevel_exists(runlevel->مقدار)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->مقدار);
					ادامه;
				}

				ret = actfunc(runlevel->مقدار, service);
				اگر (ret < 0)
					retval = EXIT_FAILURE;
				رقم_updated += ret;
			}

			اگر (retval == EXIT_SUCCESS &&
			    رقم_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	بازگشت retval;
}
