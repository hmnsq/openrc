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

अचर वर्ण *applet = रिक्त;
अचर वर्ण *extraopts = रिक्त;
अचर वर्ण *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
अचर वर्ण getoptstring[] = "asu" getoptstring_COMMON;
अचर संरचना option longopts[] = {
	{ "all",             0, रिक्त, 'a' },
	{ "stack",           0, रिक्त, 's' },
	{ "update",          0, रिक्त, 'u' },
	longopts_COMMON
};
अचर वर्ण * अचर longopts_help[] = {
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
add(अचर वर्ण *runlevel, अचर वर्ण *service)
{
	पूर्णाङ्क retval = -1;

	यदि (!rc_service_exists(service)) {
		यदि (त्रुटि_सं == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		अन्यथा
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} अन्यथा यदि (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} अन्यथा यदि (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} अन्यथा
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, म_त्रुटि (त्रुटि_सं));

	फर्काऊ retval;
}

स्थिर पूर्णाङ्क
delete(अचर वर्ण *runlevel, अचर वर्ण *service)
{
	पूर्णाङ्क retval = -1;

	त्रुटि_सं = 0;
	यदि (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		फर्काऊ 1;
	}

	यदि (त्रुटि_सं == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	अन्यथा
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, म_त्रुटि (त्रुटि_सं));

	फर्काऊ retval;
}

स्थिर पूर्णाङ्क
addstack(अचर वर्ण *runlevel, अचर वर्ण *थाक)
{
	यदि (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		फर्काऊ -1;
	}
	यदि (!rc_runlevel_exists(थाक)) {
		eerror("%s: runlevel `%s' does not exist", applet, थाक);
		फर्काऊ -1;
	}
	यदि (म_भेद(runlevel, थाक) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, थाक);
		फर्काऊ -1;
	}
	यदि (म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    म_भेद(थाक, RC_LEVEL_SYSINIT) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_BOOT) == 0 ||
	    म_भेद(थाक, RC_LEVEL_BOOT) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(थाक, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    म_भेद(थाक, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		फर्काऊ -1;
	}
	यदि (!rc_runlevel_थाक(runlevel, थाक)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, थाक, runlevel, म_त्रुटि(त्रुटि_सं));
		फर्काऊ -1;
	}
	einfo("runlevel %s added to runlevel %s", थाक, runlevel);
	फर्काऊ 1;
}

स्थिर पूर्णाङ्क
delstack(अचर वर्ण *runlevel, अचर वर्ण *थाक)
{
	यदि (rc_runlevel_unstack(runlevel, थाक)) {
		einfo("runlevel %s deleted from runlevel %s", थाक, runlevel);
		फर्काऊ 1;
	}

	यदि (त्रुटि_सं == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, थाक, runlevel);
	अन्यथा
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, थाक, runlevel, म_त्रुटि (त्रुटि_सं));

	फर्काऊ -1;
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
	माप_प्रकार l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			यदि (rc_service_in_runlevel(service->मान,
				runlevel->मान))
			{
				rc_stringlist_add(in, runlevel->मान);
				inone = true;
			} अन्यथा {
				l = म_माप(runlevel->मान);
				buffer = xmalloc(l+1);
				स_रखो(buffer, ' ', l);
				buffer[l] = 0;
				rc_stringlist_add(in, buffer);
				मुक्त(buffer);
			}
		}

		यदि (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			म_लिखो(" %26s |", service->मान);
			TAILQ_FOREACH(runlevel, in, entries)
			    म_लिखो (" %s", runlevel->मान);
			म_लिखो("\n");
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
	bool verbose = false, थाक = false, all_runlevels = false;
	पूर्णाङ्क opt;
	पूर्णाङ्क retval = निकास_त्रुटि;
	पूर्णाङ्क अङ्क_updated = 0;
	पूर्णाङ्क (*actfunc)(अचर वर्ण *, अचर वर्ण *);
	पूर्णाङ्क ret;

	applet = basename_c(argv[0]);
	जबसम्म ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णाङ्क *)0)) != -1)
		विकल्प (opt) {
		पक्ष 'a':
		    all_runlevels = true;
		रोक;
		पक्ष 's':
		    थाक = true;
		रोक;
		पक्ष 'u':
			deptree = _rc_deptree_load(-1, &ret);
			यदि (deptree)
				rc_deptree_free(deptree);
			फर्काऊ ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(दो_पर्या ("EINFO_VERBOSE"));

	यदि ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	यदि (optind < argc) {
		यदि (म_भेद(argv[optind], "add") == 0)
			action = DOADD;
		अन्यथा यदि (म_भेद(argv[optind], "delete") == 0 ||
		    म_भेद(argv[optind], "del") == 0)
			action = DODELETE;
		अन्यथा यदि (म_भेद(argv[optind], "show") == 0)
			action = DOSHOW;
		यदि (action)
			optind++;
		अन्यथा
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	यदि (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	यदि (optind >= argc) {
		यदि (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} अन्यथा {
		service = argv[optind];
		optind++;

		जबसम्म (optind < argc)
			यदि (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			अन्यथा {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = निकास_सफल;
	यदि (action & DOSHOW) {
		यदि (service)
			rc_stringlist_add(runlevels, service);
		यदि (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} अन्यथा {
		यदि (!service)
			eerror ("%s: no service specified", applet);
		अन्यथा {
			यदि (action & DOADD) {
				यदि (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = थाक ? addstack : add;
			} अन्यथा यदि (action & DODELETE) {
				actfunc = थाक ? delstack : delete;
			} अन्यथा {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			यदि (!TAILQ_FIRST(runlevels)) {
				यदि (all_runlevels) {
					मुक्त(runlevels);
					runlevels = rc_runlevel_list();
				} अन्यथा {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					मुक्त(p);
				}
			}

			यदि (!TAILQ_FIRST(runlevels)) {
				मुक्त(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				यदि (!rc_runlevel_exists(runlevel->मान)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->मान);
					जारी;
				}

				ret = actfunc(runlevel->मान, service);
				यदि (ret < 0)
					retval = निकास_त्रुटि;
				अङ्क_updated += ret;
			}

			यदि (retval == निकास_सफल &&
			    अङ्क_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	फर्काऊ retval;
}
