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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <getopt.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
ध्रुव वर्ण getoptstring[] = "asu" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{ "all",             0, रिक्त, 'a' },
	{ "stack",           0, रिक्त, 's' },
	{ "update",          0, रिक्त, 'u' },
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
स्थिर पूर्णाङ्क
add(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *service)
{
	पूर्णाङ्क retval = -1;

	सचे (!rc_service_exists(service)) {
		सचे (errno == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		अञ्ञथा
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} अञ्ञथा सचे (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} अञ्ञथा सचे (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} अञ्ञथा
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	पटिनिवत्त retval;
}

स्थिर पूर्णाङ्क
delete(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *service)
{
	पूर्णाङ्क retval = -1;

	errno = 0;
	सचे (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		पटिनिवत्त 1;
	}

	सचे (errno == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	अञ्ञथा
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, strerror (errno));

	पटिनिवत्त retval;
}

स्थिर पूर्णाङ्क
addstack(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *राशिः)
{
	सचे (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		पटिनिवत्त -1;
	}
	सचे (!rc_runlevel_exists(राशिः)) {
		eerror("%s: runlevel `%s' does not exist", applet, राशिः);
		पटिनिवत्त -1;
	}
	सचे (strcmp(runlevel, राशिः) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, राशिः);
		पटिनिवत्त -1;
	}
	सचे (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(राशिः, RC_LEVEL_SYSINIT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_BOOT) == 0 ||
	    strcmp(राशिः, RC_LEVEL_BOOT) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(राशिः, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    strcmp(राशिः, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		पटिनिवत्त -1;
	}
	सचे (!rc_runlevel_राशिः(runlevel, राशिः)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, राशिः, runlevel, strerror(errno));
		पटिनिवत्त -1;
	}
	einfo("runlevel %s added to runlevel %s", राशिः, runlevel);
	पटिनिवत्त 1;
}

स्थिर पूर्णाङ्क
delstack(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *राशिः)
{
	सचे (rc_runlevel_unstack(runlevel, राशिः)) {
		einfo("runlevel %s deleted from runlevel %s", राशिः, runlevel);
		पटिनिवत्त 1;
	}

	सचे (errno == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, राशिः, runlevel);
	अञ्ञथा
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, राशिः, runlevel, strerror (errno));

	पटिनिवत्त -1;
}

स्थिर शून्य
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(रिक्त);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	वर्ण *buffer = रिक्त;
	size_t l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			सचे (rc_service_in_runlevel(service->मानम्,
				runlevel->मानम्))
			{
				rc_stringlist_add(in, runlevel->मानम्);
				inone = true;
			} अञ्ञथा {
				l = strlen(runlevel->मानम्);
				buffer = xmalloc(l+1);
				memset(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				मुक्त(buffer);
			}
		}

		सचे (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			printf(" %26s |", service->मानम्);
			TAILQ_FOREACH(runlevel, in, entries)
			    printf (" %s", runlevel->मानम्);
			printf("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#define DOADD    (1 << 1)
#define DODELETE (1 << 2)
#define DOSHOW   (1 << 3)

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	वर्ण *service = रिक्त;
	वर्ण *p;
	पूर्णाङ्क action = 0;
	bool verbose = false, राशिः = false, all_runlevels = false;
	पूर्णाङ्क opt;
	पूर्णाङ्क retval = EXIT_FAILURE;
	पूर्णाङ्क अङ्कः_updated = 0;
	पूर्णाङ्क (*actfunc)(ध्रुव वर्ण *, ध्रुव वर्ण *);
	पूर्णाङ्क ret;

	applet = basename_c(argv[0]);
	याव ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *)0)) != -1)
		विकल्प (opt) {
		पक्ष 'a':
		    all_runlevels = true;
		भङ्ग;
		पक्ष 's':
		    राशिः = true;
		भङ्ग;
		पक्ष 'u':
			deptree = _rc_deptree_load(-1, &ret);
			सचे (deptree)
				rc_deptree_free(deptree);
			पटिनिवत्त ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(getenv ("EINFO_VERBOSE"));

	सचे ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	सचे (optind < argc) {
		सचे (strcmp(argv[optind], "add") == 0)
			action = DOADD;
		अञ्ञथा सचे (strcmp(argv[optind], "delete") == 0 ||
		    strcmp(argv[optind], "del") == 0)
			action = DODELETE;
		अञ्ञथा सचे (strcmp(argv[optind], "show") == 0)
			action = DOSHOW;
		सचे (action)
			optind++;
		अञ्ञथा
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	सचे (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	सचे (optind >= argc) {
		सचे (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} अञ्ञथा {
		service = argv[optind];
		optind++;

		याव (optind < argc)
			सचे (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			अञ्ञथा {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = EXIT_SUCCESS;
	सचे (action & DOSHOW) {
		सचे (service)
			rc_stringlist_add(runlevels, service);
		सचे (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} अञ्ञथा {
		सचे (!service)
			eerror ("%s: no service specified", applet);
		अञ्ञथा {
			सचे (action & DOADD) {
				सचे (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = राशिः ? addstack : add;
			} अञ्ञथा सचे (action & DODELETE) {
				actfunc = राशिः ? delstack : delete;
			} अञ्ञथा {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			सचे (!TAILQ_FIRST(runlevels)) {
				सचे (all_runlevels) {
					मुक्त(runlevels);
					runlevels = rc_runlevel_list();
				} अञ्ञथा {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					मुक्त(p);
				}
			}

			सचे (!TAILQ_FIRST(runlevels)) {
				मुक्त(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				सचे (!rc_runlevel_exists(runlevel->मानम्)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->मानम्);
					अनुवर्त;
				}

				ret = actfunc(runlevel->मानम्, service);
				सचे (ret < 0)
					retval = EXIT_FAILURE;
				अङ्कः_updated += ret;
			}

			सचे (retval == EXIT_SUCCESS &&
			    अङ्कः_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	पटिनिवत्त retval;
}
