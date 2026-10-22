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

#अन्तर्भाव <getopt.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <time.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "queue.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "helpers.h"

गणना format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ध्रुव वर्ण *applet = रिक्त;
ध्रुव वर्ण *extraopts = रिक्त;
ध्रुव वर्ण getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ध्रुव संरचना option longopts[] = {
	{"all",         0, रिक्त, 'a'},
	{"crashed",     0, रिक्त, 'c'},
	{"format",     1, रिक्त, 'f'},
	{"in-state",   1, रिक्त, 'i'},
	{"list",        0, रिक्त, 'l'},
	{"manual",        0, रिक्त, 'm'},
	{"runlevel",    0, रिक्त, 'r'},
	{"servicelist", 0, रिक्त, 's'},
	{"supervised", 0, रिक्त, 'S'},
	{"unused",      0, रिक्त, 'u'},
	longopts_COMMON
};
ध्रुव वर्ण * ध्रुव longopts_help[] = {
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
ध्रुव वर्ण *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

स्थिर RC_DEPTREE *deptree;
स्थिर RC_STRINGLIST *types;

स्थिर RC_STRINGLIST *levels, *services, *tmp, *alist;
स्थिर RC_STRINGLIST *sservices, *nservices, *needsme;

स्थिर शून्य print_level(ध्रुव वर्ण *prefix, ध्रुव वर्ण *level,
		गणना format_t format)
{
	विकल्प (format) {
	पक्ष FORMAT_DEFAULT:
		यदि (prefix)
			म_लिखो("%s ", prefix);
		म_लिखो ("Runlevel: ");
		यदि (isatty(fileno(मानक_निकास)))
			म_लिखो("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		अन्यथा
			म_लिखो("%s\n", level);
		भङ्ग;
	पक्ष FORMAT_INI:
		म_लिखो("%s", "[");
		यदि (prefix)
			म_लिखो("%s ", prefix);
		म_लिखो("%s]\n", level);
		भङ्ग;
	}
}

स्थिर वर्ण *get_uptime(ध्रुव वर्ण *service)
{
	RC_SERVICE state = rc_service_state(service);
	वर्ण *start_गिनती;
	वर्ण *start_time_string;
	समय_प्रकार start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	वर्ण *uptime = रिक्त;

	यदि (state & RC_SERVICE_STARTED) {
		start_गिनती = rc_service_मूल्य_get(service, "start_count");
		start_time_string = rc_service_मूल्य_get(service, "start_time");
		यदि (start_गिनती && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) स_अंतर(समय(रिक्त), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			यदि (diff_days > 0)
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
	प्रत्यागम uptime;
}

स्थिर शून्य print_service(ध्रुव वर्ण *service, गणना format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	वर्ण *status = रिक्त;
	वर्ण *uptime = रिक्त;
	वर्ण *child_pid = रिक्त;
	वर्ण *start_time = रिक्त;
	पूर्णाङ्क cols;
	ध्रुव वर्ण *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	यदि (!(state & accept) || (state & reject))
		प्रत्यागम;

	यदि (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	अन्यथा यदि (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} अन्यथा यदि (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} अन्यथा यदि (state & RC_SERVICE_STARTED) {
		त्रुटि_सं = 0;
		यदि (rc_service_daemons_crashed(service) && त्रुटि_सं != EACCES)
		{
			child_pid = rc_service_मूल्य_get(service, "child_pid");
			start_time = rc_service_मूल्य_get(service, "start_time");
			यदि (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			अन्यथा
				xasprintf(&status, " crashed ");
			मुक्त(child_pid);
			मुक्त(start_time);
		} अन्यथा {
			uptime = get_uptime(service);
			यदि (uptime) {
				xasprintf(&status, " started %s", uptime);
				मुक्त(uptime);
			} अन्यथा
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} अन्यथा यदि (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} अन्यथा यदि (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} अन्यथा
		xasprintf(&status, " stopped ");

	त्रुटि_सं = 0;
	विकल्प (format) {
	पक्ष FORMAT_DEFAULT:
		cols =  म_लिखो(" %s", service);
		यदि (c && *c && isatty(fileno(मानक_निकास)))
			म_लिखो("\n");
		ebracket(cols, color, status);
		भङ्ग;
	पक्ष FORMAT_INI:
		म_लिखो("%s = %s\n", service, status);
		भङ्ग;
	}
	मुक्त(status);
}

स्थिर शून्य print_services_in_state(ध्रुव वर्ण *runlevel, RC_STRINGLIST *svcs,
		गणना format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = रिक्त;
	RC_STRING *s;
	वर्ण *r = रिक्त;

	यदि (!svcs)
		प्रत्यागम;
	यदि (!deptree)
		deptree = _rc_deptree_load(0, रिक्त);
	यदि (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			यदि (!runlevel ||
			    rc_service_in_runlevel(s->मूल्य, runlevel))
				print_service(s->मूल्य, format, accept, reject);
		प्रत्यागम;
	}
	यदि (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	यदि (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	मुक्त(r);
	यदि (!l)
		प्रत्यागम;
	TAILQ_FOREACH(s, l, entries) {
		यदि (!rc_stringlist_find(svcs, s->मूल्य))
			अनुवर्त;
		यदि (!runlevel || rc_service_in_runlevel(s->मूल्य, runlevel))
			print_service(s->मूल्य, format, accept, reject);
	}
	rc_stringlist_free(l);
}

स्थिर शून्य print_services(ध्रुव वर्ण *runlevel, RC_STRINGLIST *svcs,
	गणना format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

स्थिर शून्य print_stacked_services(ध्रुव वर्ण *runlevel, गणना format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		यदि (rc_stringlist_find(levels, stackedlevel->मूल्य) != रिक्त)
			अनुवर्त;
		print_level("Stacked", stackedlevel->मूल्य, format);
		servicelist = rc_services_in_runlevel(stackedlevel->मूल्य);
		print_services_in_state(stackedlevel->मूल्य, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = रिक्त;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ध्रुव rc_service_state_name_t *it;
	गणना format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	वर्ण *p, *runlevel = रिक्त;
	पूर्णाङ्क opt, retval = 0;

	applet = basename_c(argv[0]);
	यावत् ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (पूर्णाङ्क *) 0)) != -1)
		विकल्प (opt) {
		पक्ष 'a':
			show_all = true;
			levels = rc_runlevel_list();
			भङ्ग;
		पक्ष 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				यदि (rc_service_daemons_crashed(s->मूल्य)) {
					म_लिखो("%s\n", s->मूल्य);
					retval = 0;
				}
			गच्छ निकास;
			/* NOTREACHED */
		पक्ष 'f':
			यदि (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} अन्यथा
				eerrorx("%s: invalid argument to --format switch", applet);
			भङ्ग;
		पक्ष 'i':
			यदि (accept == (RC_SERVICE)-1)
				accept = 0;
			प्रति (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				यदि (it->state == RC_SERVICE_WASINACTIVE)
					अनुवर्त;
				यदि (म_भेद(optarg, it->name) == 0) {
					state = it->state;
					भङ्ग;
				}
			}
			यदि (it->name == रिक्त)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			यदि (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} अन्यथा यदि (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			यदि (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} अन्यथा यदि (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			भङ्ग;
		पक्ष 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				म_लिखो("%s\n", l->मूल्य);
			गच्छ निकास;
			/* NOTREACHED */
		पक्ष 'm':
			services = rc_services_in_runlevel(रिक्त);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					यदि (rc_service_in_runlevel(s->मूल्य, l->मूल्य)) {
						TAILQ_REMOVE(services, s, entries);
						मुक्त(s->मूल्य);
						मुक्त(s);
						भङ्ग;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				यदि (rc_service_state(s->मूल्य) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					मुक्त(s->मूल्य);
					मुक्त(s);
				}
			print_services(रिक्त, services, format);
			गच्छ निकास;
			/* NOTREACHED */
		पक्ष 'r':
			runlevel = rc_runlevel_get();
			म_लिखो("%s\n", runlevel ? runlevel : "");
			गच्छ निकास;
			/* NOTREACHED */
		पक्ष 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				वर्ण *ret = rc_service_मूल्य_get(s->मूल्य, "child_pid");
				यदि (!ret) {
					TAILQ_REMOVE(services, s, entries);
					मुक्त(s->मूल्य);
					मुक्त(s);
				}
				मुक्त(ret);
			}
			print_services(रिक्त, services, format);
			गच्छ निकास;
			/* NOTREACHED */
		पक्ष 's':
			services = rc_services_in_runlevel(रिक्त);
			print_services(रिक्त, services, format);
			गच्छ निकास;
			/* NOTREACHED */
		पक्ष 'u':
			services = rc_services_in_runlevel(रिक्त);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					यदि (rc_service_in_runlevel(s->मूल्य, l->मूल्य)) {
						TAILQ_REMOVE(services, s, entries);
						मुक्त(s->मूल्य);
						मुक्त(s);
						भङ्ग;
					}
			}
			print_services(रिक्त, services, format);
			गच्छ निकास;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	यदि (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	यावत् (optind < argc) {
		यदि (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} अन्यथा
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	यदि (opt == 0)
		निकास(निकास_त्रुटि);
	यदि (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, रिक्त);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(रिक्त, l->मूल्य, format);
		services = rc_services_in_runlevel(l->मूल्य);
		print_services_in_state(l->मूल्य, services, format, accept, reject);
		print_stacked_services(l->मूल्य, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = रिक्त;
		rc_stringlist_free(services);
		services = रिक्त;
	}

	यदि (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(रिक्त, services, format, accept, reject);
		rc_stringlist_free(services);
		services = रिक्त;

		/* Show manually started and unassigned depended services */
		यदि (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			यदि (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(रिक्त);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->मूल्य);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->मूल्य);
			यदि ((rc_stringlist_find(sservices, s->मूल्य) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				यदि (!(state & RC_SERVICE_FAILED)) {
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
				यदि (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_जोड़ो_TAIL(nservices, s, entries);
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
		print_services_in_state(रिक्त, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(रिक्त, services, format, accept, reject);
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

	प्रत्यागम retval;
}
