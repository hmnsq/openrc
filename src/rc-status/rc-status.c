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

#شامل <getopt.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <strings.h>
#شامل <unistd.h>
#شامل <inttypes.h>
#شامل <errno.h>
#شامل <time.h>

#شامل "einfo.h"
#شامل "queue.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "_usage.h"
#شامل "helpers.h"

شمارش format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ثابت نویسه *applet = پوچ;
ثابت نویسه *extraopts = پوچ;
ثابت نویسه getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ثابت ساختار option longopts[] = {
	{"all",         0, پوچ, 'a'},
	{"crashed",     0, پوچ, 'c'},
	{"format",     1, پوچ, 'f'},
	{"in-state",   1, پوچ, 'i'},
	{"list",        0, پوچ, 'l'},
	{"manual",        0, پوچ, 'm'},
	{"runlevel",    0, پوچ, 'r'},
	{"servicelist", 0, پوچ, 's'},
	{"supervised", 0, پوچ, 'S'},
	{"unused",      0, پوچ, 'u'},
	longopts_COMMON
};
ثابت نویسه * ثابت longopts_help[] = {
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
ثابت نویسه *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

ایستا RC_DEPTREE *deptree;
ایستا RC_STRINGLIST *types;

ایستا RC_STRINGLIST *levels, *services, *tmp, *alist;
ایستا RC_STRINGLIST *sservices, *nservices, *needsme;

ایستا تهی print_level(ثابت نویسه *prefix, ثابت نویسه *level,
		شمارش format_t format)
{
	انتخاب (format) {
	حالت FORMAT_DEFAULT:
		اگر (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		اگر (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		وگرنه
			printf("%s\n", level);
		بشکن;
	حالت FORMAT_INI:
		printf("%s", "[");
		اگر (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		بشکن;
	}
}

ایستا نویسه *get_uptime(ثابت نویسه *service)
{
	RC_SERVICE state = rc_service_state(service);
	نویسه *start_شمار;
	نویسه *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	نویسه *uptime = پوچ;

	اگر (state & RC_SERVICE_STARTED) {
		start_شمار = rc_service_مقدار_get(service, "start_count");
		start_time_string = rc_service_مقدار_get(service, "start_time");
		اگر (start_شمار && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(پوچ), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			اگر (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_شمار);
			وگرنه
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_شمار);
		}
		آزاد(start_شمار);
		آزاد(start_time_string);
	}
	بازگشت uptime;
}

ایستا تهی print_service(ثابت نویسه *service, شمارش format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	نویسه *status = پوچ;
	نویسه *uptime = پوچ;
	نویسه *child_pid = پوچ;
	نویسه *start_time = پوچ;
	صحیح cols;
	ثابت نویسه *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	اگر (!(state & accept) || (state & reject))
		بازگشت;

	اگر (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	وگرنه اگر (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} وگرنه اگر (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} وگرنه اگر (state & RC_SERVICE_STARTED) {
		errno = 0;
		اگر (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_مقدار_get(service, "child_pid");
			start_time = rc_service_مقدار_get(service, "start_time");
			اگر (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			وگرنه
				xasprintf(&status, " crashed ");
			آزاد(child_pid);
			آزاد(start_time);
		} وگرنه {
			uptime = get_uptime(service);
			اگر (uptime) {
				xasprintf(&status, " started %s", uptime);
				آزاد(uptime);
			} وگرنه
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} وگرنه اگر (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} وگرنه اگر (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} وگرنه
		xasprintf(&status, " stopped ");

	errno = 0;
	انتخاب (format) {
	حالت FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		اگر (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		بشکن;
	حالت FORMAT_INI:
		printf("%s = %s\n", service, status);
		بشکن;
	}
	آزاد(status);
}

ایستا تهی print_services_in_state(ثابت نویسه *runlevel, RC_STRINGLIST *svcs,
		شمارش format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = پوچ;
	RC_STRING *s;
	نویسه *r = پوچ;

	اگر (!svcs)
		بازگشت;
	اگر (!deptree)
		deptree = _rc_deptree_load(0, پوچ);
	اگر (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			اگر (!runlevel ||
			    rc_service_in_runlevel(s->مقدار, runlevel))
				print_service(s->مقدار, format, accept, reject);
		بازگشت;
	}
	اگر (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	اگر (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	آزاد(r);
	اگر (!l)
		بازگشت;
	TAILQ_FOREACH(s, l, entries) {
		اگر (!rc_stringlist_find(svcs, s->مقدار))
			ادامه;
		اگر (!runlevel || rc_service_in_runlevel(s->مقدار, runlevel))
			print_service(s->مقدار, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ایستا تهی print_services(ثابت نویسه *runlevel, RC_STRINGLIST *svcs,
	شمارش format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

ایستا تهی print_stacked_services(ثابت نویسه *runlevel, شمارش format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		اگر (rc_stringlist_find(levels, stackedlevel->مقدار) != پوچ)
			ادامه;
		print_level("Stacked", stackedlevel->مقدار, format);
		servicelist = rc_services_in_runlevel(stackedlevel->مقدار);
		print_services_in_state(stackedlevel->مقدار, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = پوچ;
}

صحیح اصلی(صحیح argc, نویسه **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ثابت rc_service_state_name_t *it;
	شمارش format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	نویسه *p, *runlevel = پوچ;
	صحیح opt, retval = 0;

	applet = basename_c(argv[0]);
	وقتی ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (صحیح *) 0)) != -1)
		انتخاب (opt) {
		حالت 'a':
			show_all = true;
			levels = rc_runlevel_list();
			بشکن;
		حالت 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				اگر (rc_service_daemons_crashed(s->مقدار)) {
					printf("%s\n", s->مقدار);
					retval = 0;
				}
			برو exit;
			/* NOTREACHED */
		حالت 'f':
			اگر (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} وگرنه
				eerrorx("%s: invalid argument to --format switch", applet);
			بشکن;
		حالت 'i':
			اگر (accept == (RC_SERVICE)-1)
				accept = 0;
			برای (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				اگر (it->state == RC_SERVICE_WASINACTIVE)
					ادامه;
				اگر (strcmp(optarg, it->name) == 0) {
					state = it->state;
					بشکن;
				}
			}
			اگر (it->name == پوچ)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			اگر (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} وگرنه اگر (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			اگر (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} وگرنه اگر (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			بشکن;
		حالت 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->مقدار);
			برو exit;
			/* NOTREACHED */
		حالت 'm':
			services = rc_services_in_runlevel(پوچ);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					اگر (rc_service_in_runlevel(s->مقدار, l->مقدار)) {
						TAILQ_REMOVE(services, s, entries);
						آزاد(s->مقدار);
						آزاد(s);
						بشکن;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				اگر (rc_service_state(s->مقدار) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					آزاد(s->مقدار);
					آزاد(s);
				}
			print_services(پوچ, services, format);
			برو exit;
			/* NOTREACHED */
		حالت 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			برو exit;
			/* NOTREACHED */
		حالت 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				نویسه *ret = rc_service_مقدار_get(s->مقدار, "child_pid");
				اگر (!ret) {
					TAILQ_REMOVE(services, s, entries);
					آزاد(s->مقدار);
					آزاد(s);
				}
				آزاد(ret);
			}
			print_services(پوچ, services, format);
			برو exit;
			/* NOTREACHED */
		حالت 's':
			services = rc_services_in_runlevel(پوچ);
			print_services(پوچ, services, format);
			برو exit;
			/* NOTREACHED */
		حالت 'u':
			services = rc_services_in_runlevel(پوچ);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					اگر (rc_service_in_runlevel(s->مقدار, l->مقدار)) {
						TAILQ_REMOVE(services, s, entries);
						آزاد(s->مقدار);
						آزاد(s);
						بشکن;
					}
			}
			print_services(پوچ, services, format);
			برو exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	اگر (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	وقتی (optind < argc) {
		اگر (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} وگرنه
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	اگر (opt == 0)
		exit(EXIT_FAILURE);
	اگر (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, پوچ);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(پوچ, l->مقدار, format);
		services = rc_services_in_runlevel(l->مقدار);
		print_services_in_state(l->مقدار, services, format, accept, reject);
		print_stacked_services(l->مقدار, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = پوچ;
		rc_stringlist_free(services);
		services = پوچ;
	}

	اگر (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(پوچ, services, format, accept, reject);
		rc_stringlist_free(services);
		services = پوچ;

		/* Show manually started and unassigned depended services */
		اگر (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			اگر (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(پوچ);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->مقدار);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->مقدار);
			اگر ((rc_stringlist_find(sservices, s->مقدار) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				اگر (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					آزاد(s->مقدار);
					آزاد(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->مقدار;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->مقدار = s->مقدار;
				setenv("RC_SVCNAME", l->مقدار, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->مقدار, RC_DEP_TRACE);
				اگر (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_درج_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->مقدار = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(پوچ, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(پوچ, services, format, accept, reject);
	}

exit:
	آزاد(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	بازگشت retval;
}
