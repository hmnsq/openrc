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

#समावेश <errno.h>
#समावेश <getopt.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर *usagestring = ""
	"Usage: rc-update [options] add <service> [<runlevel>...]\n"
	"   or: rc-update [options] del <service> [<runlevel>...]\n"
	"   or: rc-update [options] [show [<runlevel>...]]";
स्थिर अक्षर getoptstring[] = "asu" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{ "all",             0, शून्य, 'a' },
	{ "stack",           0, शून्य, 's' },
	{ "update",          0, शून्य, 'u' },
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
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
अटल पूर्णांक
add(स्थिर अक्षर *runlevel, स्थिर अक्षर *service)
{
	पूर्णांक retval = -1;

	अगर (!rc_service_exists(service)) {
		अगर (त्रुटि_सं == ENOEXEC)
			eerror("%s: service `%s' is not executable",
			    applet, service);
		अन्यथा
			eerror("%s: service `%s' does not exist",
			    applet, service);
	} अन्यथा अगर (rc_service_in_runlevel(service, runlevel)) {
		einfo("%s: %s already installed in runlevel `%s'; skipping",
		    applet, service, runlevel);
		retval = 0;
	} अन्यथा अगर (rc_service_add(runlevel, service)) {
		einfo("service %s added to runlevel %s", service, runlevel);
		retval = 1;
	} अन्यथा
		eerror("%s: failed to add service `%s' to runlevel `%s': %s",
		    applet, service, runlevel, म_त्रुटि (त्रुटि_सं));

	वापस retval;
}

अटल पूर्णांक
delete(स्थिर अक्षर *runlevel, स्थिर अक्षर *service)
{
	पूर्णांक retval = -1;

	त्रुटि_सं = 0;
	अगर (rc_service_delete(runlevel, service)) {
		einfo("service %s deleted from runlevel %s",
		    service, runlevel);
		वापस 1;
	}

	अगर (त्रुटि_सं == ENOENT)
		eerror("%s: service `%s' is not in the runlevel `%s'",
		    applet, service, runlevel);
	अन्यथा
		eerror("%s: failed to delete service `%s' from runlevel `%s': %s",
		    applet, service, runlevel, म_त्रुटि (त्रुटि_सं));

	वापस retval;
}

अटल पूर्णांक
addstack(स्थिर अक्षर *runlevel, स्थिर अक्षर *ढेर)
{
	अगर (!rc_runlevel_exists(runlevel)) {
		eerror("%s: runlevel `%s' does not exist", applet, runlevel);
		वापस -1;
	}
	अगर (!rc_runlevel_exists(ढेर)) {
		eerror("%s: runlevel `%s' does not exist", applet, ढेर);
		वापस -1;
	}
	अगर (म_भेद(runlevel, ढेर) == 0) {
		eerror("%s: cannot stack `%s' onto itself", applet, ढेर);
		वापस -1;
	}
	अगर (म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0 ||
	    म_भेद(ढेर, RC_LEVEL_SYSINIT) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_BOOT) == 0 ||
	    म_भेद(ढेर, RC_LEVEL_BOOT) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(ढेर, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0 ||
	    म_भेद(ढेर, RC_LEVEL_SHUTDOWN) == 0)
	{
		eerror("%s: cannot stack the %s runlevel",
		    applet, RC_LEVEL_SYSINIT);
		वापस -1;
	}
	अगर (!rc_runlevel_ढेर(runlevel, ढेर)) {
		eerror("%s: failed to stack `%s' to `%s': %s",
		    applet, ढेर, runlevel, म_त्रुटि(त्रुटि_सं));
		वापस -1;
	}
	einfo("runlevel %s added to runlevel %s", ढेर, runlevel);
	वापस 1;
}

अटल पूर्णांक
delstack(स्थिर अक्षर *runlevel, स्थिर अक्षर *ढेर)
{
	अगर (rc_runlevel_unstack(runlevel, ढेर)) {
		einfo("runlevel %s deleted from runlevel %s", ढेर, runlevel);
		वापस 1;
	}

	अगर (त्रुटि_सं == ENOENT)
		eerror("%s: runlevel `%s' is not in the runlevel `%s'",
		    applet, ढेर, runlevel);
	अन्यथा
		eerror("%s: failed to delete runlevel `%s' from runlevel `%s': %s",
		    applet, ढेर, runlevel, म_त्रुटि (त्रुटि_सं));

	वापस -1;
}

अटल व्योम
show(RC_STRINGLIST *runlevels, bool verbose)
{
	RC_STRINGLIST *services = rc_services_in_runlevel(शून्य);
	RC_STRING *service;
	RC_STRING *runlevel;
	RC_STRINGLIST *in;
	bool inone;
	अक्षर *बफर = शून्य;
	माप_प्रकार l;

	rc_stringlist_sort(&services);
	TAILQ_FOREACH(service, services, entries) {
		in = rc_stringlist_new();
		inone = false;

		TAILQ_FOREACH(runlevel, runlevels, entries) {
			अगर (rc_service_in_runlevel(service->मूल्य,
				runlevel->मूल्य))
			{
				rc_stringlist_add(in, runlevel->मूल्य);
				inone = true;
			} अन्यथा {
				l = म_माप(runlevel->मूल्य);
				बफर = xmalloc(l+1);
				स_रखो(बफर, ' ', l);
				बफर[l] = 0;
				rc_stringlist_add(in, बफर);
				मुक्त(बफर);
			}
		}

		अगर (inone || verbose) {
			/* For more information on why padding is 26, see issue 937: */
			/* https://github.com/OpenRC/openrc/issues/937 */
			म_लिखो(" %26s |", service->मूल्य);
			TAILQ_FOREACH(runlevel, in, entries)
			    म_लिखो (" %s", runlevel->मूल्य);
			म_लिखो("\n");
		}
		rc_stringlist_free(in);
	}

	rc_stringlist_free(services);
}

#घोषणा DOADD    (1 << 1)
#घोषणा DODELETE (1 << 2)
#घोषणा DOSHOW   (1 << 3)

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	RC_DEPTREE *deptree;
	RC_STRINGLIST *runlevels;
	RC_STRING *runlevel;
	अक्षर *service = शून्य;
	अक्षर *p;
	पूर्णांक action = 0;
	bool verbose = false, ढेर = false, all_runlevels = false;
	पूर्णांक opt;
	पूर्णांक retval = निकास_त्रुटि;
	पूर्णांक अंक_updated = 0;
	पूर्णांक (*actfunc)(स्थिर अक्षर *, स्थिर अक्षर *);
	पूर्णांक ret;

	applet = basename_c(argv[0]);
	जबतक ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (पूर्णांक *)0)) != -1)
		चयन (opt) {
		हाल 'a':
		    all_runlevels = true;
		अवरोध;
		हाल 's':
		    ढेर = true;
		अवरोध;
		हाल 'u':
			deptree = _rc_deptree_load(-1, &ret);
			अगर (deptree)
				rc_deptree_free(deptree);
			वापस ret;
		case_RC_COMMON_GETOPT
		}

	verbose = rc_yesno(दो_पर्या ("EINFO_VERBOSE"));

	अगर ((action & DOSHOW   && action != DOSHOW) ||
	    (action & DOADD    && action != DOADD) ||
	    (action & DODELETE && action != DODELETE))
		eerrorx("%s: cannot mix commands", applet);

	/* We need to be backwards compatible */
	अगर (optind < argc) {
		अगर (म_भेद(argv[optind], "add") == 0)
			action = DOADD;
		अन्यथा अगर (म_भेद(argv[optind], "delete") == 0 ||
		    म_भेद(argv[optind], "del") == 0)
			action = DODELETE;
		अन्यथा अगर (म_भेद(argv[optind], "show") == 0)
			action = DOSHOW;
		अगर (action)
			optind++;
		अन्यथा
			eerrorx("%s: invalid command `%s'",
			    applet, argv[optind]);
	}
	अगर (!action)
		action = DOSHOW;

	runlevels = rc_stringlist_new();

	अगर (optind >= argc) {
		अगर (!(action & DOSHOW))
			eerrorx("%s: no service specified", applet);
	} अन्यथा {
		service = argv[optind];
		optind++;

		जबतक (optind < argc)
			अगर (rc_runlevel_exists(argv[optind]))
				rc_stringlist_add(runlevels, argv[optind++]);
			अन्यथा {
				rc_stringlist_free(runlevels);
				eerrorx ("%s: `%s' is not a valid runlevel",
				    applet, argv[optind]);
			}
	}

	retval = निकास_सफल;
	अगर (action & DOSHOW) {
		अगर (service)
			rc_stringlist_add(runlevels, service);
		अगर (!TAILQ_FIRST(runlevels)) {
			rc_stringlist_free(runlevels);
			runlevels = rc_runlevel_list();
		}

		rc_stringlist_sort(&runlevels);
		show (runlevels, verbose);
	} अन्यथा {
		अगर (!service)
			eerror ("%s: no service specified", applet);
		अन्यथा {
			अगर (action & DOADD) {
				अगर (all_runlevels) {
					rc_stringlist_free(runlevels);
					eerrorx("%s: the -a option is invalid with add", applet);
				}
				actfunc = ढेर ? addstack : add;
			} अन्यथा अगर (action & DODELETE) {
				actfunc = ढेर ? delstack : delete;
			} अन्यथा {
				rc_stringlist_free(runlevels);
				eerrorx("%s: invalid action", applet);
			}

			अगर (!TAILQ_FIRST(runlevels)) {
				अगर (all_runlevels) {
					मुक्त(runlevels);
					runlevels = rc_runlevel_list();
				} अन्यथा {
					p = rc_runlevel_get();
					rc_stringlist_add(runlevels,
					    p ? p : default_runlevel());
					मुक्त(p);
				}
			}

			अगर (!TAILQ_FIRST(runlevels)) {
				मुक्त(runlevels);
				eerrorx("%s: no runlevels found", applet);
			}

			TAILQ_FOREACH(runlevel, runlevels, entries) {
				अगर (!rc_runlevel_exists(runlevel->मूल्य)) {
					eerror ("%s: runlevel `%s' does not exist",
					    applet, runlevel->मूल्य);
					जारी;
				}

				ret = actfunc(runlevel->मूल्य, service);
				अगर (ret < 0)
					retval = निकास_त्रुटि;
				अंक_updated += ret;
			}

			अगर (retval == निकास_सफल &&
			    अंक_updated == 0 && action & DODELETE)
				ewarnx("%s: service `%s' not found in any"
				    " of the specified runlevels",
				    applet, service);
		}
	}

	rc_stringlist_free(runlevels);
	वापस retval;
}
