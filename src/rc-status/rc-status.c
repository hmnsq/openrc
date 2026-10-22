/*
 * rc-status.c
 * Display the status of the services in runlevels
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

#समावेश <getopt.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <strings.h>
#समावेश <unistd.h>
#समावेश <inttypes.h>
#समावेश <errno.h>
#समावेश <time.h>

#समावेश "einfo.h"
#समावेश "queue.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "_usage.h"
#समावेश "helpers.h"

क्रमागत format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

स्थिर अक्षर *applet = शून्य;
स्थिर अक्षर *extraopts = शून्य;
स्थिर अक्षर getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
स्थिर काष्ठा option longopts[] = {
	{"all",         0, शून्य, 'a'},
	{"crashed",     0, शून्य, 'c'},
	{"format",     1, शून्य, 'f'},
	{"in-state",   1, शून्य, 'i'},
	{"list",        0, शून्य, 'l'},
	{"manual",        0, शून्य, 'm'},
	{"runlevel",    0, शून्य, 'r'},
	{"servicelist", 0, शून्य, 's'},
	{"supervised", 0, शून्य, 'S'},
	{"unused",      0, शून्य, 'u'},
	longopts_COMMON
};
स्थिर अक्षर * स्थिर longopts_help[] = {
	"Show services from all run levels",
	"Show crashed services",
	"format status to be parsable (currently arg must be ini)",
	"Show services which are in this state",
	"Show list of run levels",
	"Show manually started services",
	"Show the name of the current runlevel",
	"Show service list",
	"show supervised services",
	"Show services not assigned to any runlevel",
	longopts_help_COMMON
};
स्थिर अक्षर *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

अटल RC_DEPTREE *deptree;
अटल RC_STRINGLIST *types;

अटल RC_STRINGLIST *levels, *services, *tmp, *alist;
अटल RC_STRINGLIST *sservices, *nservices, *needsme;

अटल व्योम print_level(स्थिर अक्षर *prefix, स्थिर अक्षर *level,
		क्रमागत format_t format)
{
	चयन (format) {
	हाल FORMAT_DEFAULT:
		अगर (prefix)
			म_लिखो("%s ", prefix);
		म_लिखो ("Runlevel: ");
		अगर (isatty(fileno(मानक_निकास)))
			म_लिखो("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		अन्यथा
			म_लिखो("%s\n", level);
		अवरोध;
	हाल FORMAT_INI:
		म_लिखो("%s", "[");
		अगर (prefix)
			म_लिखो("%s ", prefix);
		म_लिखो("%s]\n", level);
		अवरोध;
	}
}

अटल अक्षर *get_uptime(स्थिर अक्षर *service)
{
	RC_SERVICE state = rc_service_state(service);
	अक्षर *start_गिनती;
	अक्षर *start_time_string;
	समय_प्रकार start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	अक्षर *uptime = शून्य;

	अगर (state & RC_SERVICE_STARTED) {
		start_गिनती = rc_service_मूल्य_get(service, "start_count");
		start_time_string = rc_service_मूल्य_get(service, "start_time");
		अगर (start_गिनती && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) स_अंतर(समय(शून्य), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			अगर (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_गिनती);
			अन्यथा
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_गिनती);
		}
		मुक्त(start_गिनती);
		मुक्त(start_time_string);
	}
	वापस uptime;
}

अटल व्योम print_service(स्थिर अक्षर *service, क्रमागत format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	अक्षर *status = शून्य;
	अक्षर *uptime = शून्य;
	अक्षर *child_pid = शून्य;
	अक्षर *start_time = शून्य;
	पूर्णांक cols;
	स्थिर अक्षर *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	अगर (!(state & accept) || (state & reject))
		वापस;

	अगर (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	अन्यथा अगर (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} अन्यथा अगर (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} अन्यथा अगर (state & RC_SERVICE_STARTED) {
		त्रुटि_सं = 0;
		अगर (rc_service_daemons_crashed(service) && त्रुटि_सं != EACCES)
		{
			child_pid = rc_service_मूल्य_get(service, "child_pid");
			start_time = rc_service_मूल्य_get(service, "start_time");
			अगर (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			अन्यथा
				xasprintf(&status, " crashed ");
			मुक्त(child_pid);
			मुक्त(start_time);
		} अन्यथा {
			uptime = get_uptime(service);
			अगर (uptime) {
				xasprintf(&status, " started %s", uptime);
				मुक्त(uptime);
			} अन्यथा
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} अन्यथा अगर (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} अन्यथा अगर (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} अन्यथा
		xasprintf(&status, " stopped ");

	त्रुटि_सं = 0;
	चयन (format) {
	हाल FORMAT_DEFAULT:
		cols =  म_लिखो(" %s", service);
		अगर (c && *c && isatty(fileno(मानक_निकास)))
			म_लिखो("\n");
		ebracket(cols, color, status);
		अवरोध;
	हाल FORMAT_INI:
		म_लिखो("%s = %s\n", service, status);
		अवरोध;
	}
	मुक्त(status);
}

अटल व्योम print_services_in_state(स्थिर अक्षर *runlevel, RC_STRINGLIST *svcs,
		क्रमागत format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = शून्य;
	RC_STRING *s;
	अक्षर *r = शून्य;

	अगर (!svcs)
		वापस;
	अगर (!deptree)
		deptree = _rc_deptree_load(0, शून्य);
	अगर (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			अगर (!runlevel ||
			    rc_service_in_runlevel(s->मूल्य, runlevel))
				print_service(s->मूल्य, format, accept, reject);
		वापस;
	}
	अगर (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	अगर (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	मुक्त(r);
	अगर (!l)
		वापस;
	TAILQ_FOREACH(s, l, entries) {
		अगर (!rc_stringlist_find(svcs, s->मूल्य))
			जारी;
		अगर (!runlevel || rc_service_in_runlevel(s->मूल्य, runlevel))
			print_service(s->मूल्य, format, accept, reject);
	}
	rc_stringlist_free(l);
}

अटल व्योम print_services(स्थिर अक्षर *runlevel, RC_STRINGLIST *svcs,
	क्रमागत format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

अटल व्योम print_stacked_services(स्थिर अक्षर *runlevel, क्रमागत format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		अगर (rc_stringlist_find(levels, stackedlevel->मूल्य) != शून्य)
			जारी;
		print_level("Stacked", stackedlevel->मूल्य, format);
		servicelist = rc_services_in_runlevel(stackedlevel->मूल्य);
		print_services_in_state(stackedlevel->मूल्य, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = शून्य;
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	स्थिर rc_service_state_name_t *it;
	क्रमागत format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	अक्षर *p, *runlevel = शून्य;
	पूर्णांक opt, retval = 0;

	applet = basename_c(argv[0]);
	जबतक ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (पूर्णांक *) 0)) != -1)
		चयन (opt) {
		हाल 'a':
			show_all = true;
			levels = rc_runlevel_list();
			अवरोध;
		हाल 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				अगर (rc_service_daemons_crashed(s->मूल्य)) {
					म_लिखो("%s\n", s->मूल्य);
					retval = 0;
				}
			जाओ निकास;
			/* NOTREACHED */
		हाल 'f':
			अगर (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} अन्यथा
				eerrorx("%s: invalid argument to --format switch", applet);
			अवरोध;
		हाल 'i':
			अगर (accept == (RC_SERVICE)-1)
				accept = 0;
			क्रम (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				अगर (it->state == RC_SERVICE_WASINACTIVE)
					जारी;
				अगर (म_भेद(optarg, it->name) == 0) {
					state = it->state;
					अवरोध;
				}
			}
			अगर (it->name == शून्य)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			अगर (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} अन्यथा अगर (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			अगर (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} अन्यथा अगर (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			अवरोध;
		हाल 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				म_लिखो("%s\n", l->मूल्य);
			जाओ निकास;
			/* NOTREACHED */
		हाल 'm':
			services = rc_services_in_runlevel(शून्य);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					अगर (rc_service_in_runlevel(s->मूल्य, l->मूल्य)) {
						TAILQ_REMOVE(services, s, entries);
						मुक्त(s->मूल्य);
						मुक्त(s);
						अवरोध;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				अगर (rc_service_state(s->मूल्य) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					मुक्त(s->मूल्य);
					मुक्त(s);
				}
			print_services(शून्य, services, format);
			जाओ निकास;
			/* NOTREACHED */
		हाल 'r':
			runlevel = rc_runlevel_get();
			म_लिखो("%s\n", runlevel ? runlevel : "");
			जाओ निकास;
			/* NOTREACHED */
		हाल 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				अक्षर *ret = rc_service_मूल्य_get(s->मूल्य, "child_pid");
				अगर (!ret) {
					TAILQ_REMOVE(services, s, entries);
					मुक्त(s->मूल्य);
					मुक्त(s);
				}
				मुक्त(ret);
			}
			print_services(शून्य, services, format);
			जाओ निकास;
			/* NOTREACHED */
		हाल 's':
			services = rc_services_in_runlevel(शून्य);
			print_services(शून्य, services, format);
			जाओ निकास;
			/* NOTREACHED */
		हाल 'u':
			services = rc_services_in_runlevel(शून्य);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					अगर (rc_service_in_runlevel(s->मूल्य, l->मूल्य)) {
						TAILQ_REMOVE(services, s, entries);
						मुक्त(s->मूल्य);
						मुक्त(s);
						अवरोध;
					}
			}
			print_services(शून्य, services, format);
			जाओ निकास;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	अगर (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	जबतक (optind < argc) {
		अगर (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} अन्यथा
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	अगर (opt == 0)
		निकास(निकास_त्रुटि);
	अगर (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, शून्य);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(शून्य, l->मूल्य, format);
		services = rc_services_in_runlevel(l->मूल्य);
		print_services_in_state(l->मूल्य, services, format, accept, reject);
		print_stacked_services(l->मूल्य, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = शून्य;
		rc_stringlist_free(services);
		services = शून्य;
	}

	अगर (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(शून्य, services, format, accept, reject);
		rc_stringlist_free(services);
		services = शून्य;

		/* Show manually started and unassigned depended services */
		अगर (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			अगर (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(शून्य);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->मूल्य);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->मूल्य);
			अगर ((rc_stringlist_find(sservices, s->मूल्य) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				अगर (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					मुक्त(s->मूल्य);
					मुक्त(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->मूल्य;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->मूल्य = s->मूल्य;
				setenv("RC_SVCNAME", l->मूल्य, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->मूल्य, RC_DEP_TRACE);
				अगर (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_जोड़ो_पुच्छ(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->मूल्य = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(शून्य, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(शून्य, services, format, accept, reject);
	}

निकास:
	मुक्त(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	वापस retval;
}
