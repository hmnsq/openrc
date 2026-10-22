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

#ضمن <getopt.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <strings.h>
#ضمن <unistd.h>
#ضمن <inttypes.h>
#ضمن <errno.h>
#ضمن <time.h>

#ضمن "einfo.h"
#ضمن "queue.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "_usage.h"
#ضمن "helpers.h"

تعداد format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ثابت حرف *applet = عدم;
ثابت حرف *extraopts = عدم;
ثابت حرف getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ثابت بنية option longopts[] = {
	{"all",         0, عدم, 'a'},
	{"crashed",     0, عدم, 'c'},
	{"format",     1, عدم, 'f'},
	{"in-state",   1, عدم, 'i'},
	{"list",        0, عدم, 'l'},
	{"manual",        0, عدم, 'm'},
	{"runlevel",    0, عدم, 'r'},
	{"servicelist", 0, عدم, 's'},
	{"supervised", 0, عدم, 'S'},
	{"unused",      0, عدم, 'u'},
	longopts_COMMON
};
ثابت حرف * ثابت longopts_help[] = {
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
ثابت حرف *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

ساكن RC_DEPTREE *deptree;
ساكن RC_STRINGLIST *types;

ساكن RC_STRINGLIST *levels, *services, *tmp, *alist;
ساكن RC_STRINGLIST *sservices, *nservices, *needsme;

ساكن فراغ print_level(ثابت حرف *prefix, ثابت حرف *level,
		تعداد format_t format)
{
	بدل (format) {
	حالة FORMAT_DEFAULT:
		ܐܢ (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		ܐܢ (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		ܐܚܪܢܐ
			printf("%s\n", level);
		اكسر;
	حالة FORMAT_INI:
		printf("%s", "[");
		ܐܢ (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		اكسر;
	}
}

ساكن حرف *get_uptime(ثابت حرف *service)
{
	RC_SERVICE state = rc_service_state(service);
	حرف *start_count;
	حرف *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	حرف *uptime = عدم;

	ܐܢ (state & RC_SERVICE_STARTED) {
		start_count = rc_service_value_get(service, "start_count");
		start_time_string = rc_service_value_get(service, "start_time");
		ܐܢ (start_count && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(عدم), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ܐܢ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_count);
			ܐܚܪܢܐ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_count);
		}
		حرر(start_count);
		حرر(start_time_string);
	}
	ܦܢܐ uptime;
}

ساكن فراغ print_service(ثابت حرف *service, تعداد format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	حرف *status = عدم;
	حرف *uptime = عدم;
	حرف *child_pid = عدم;
	حرف *start_time = عدم;
	صحيح cols;
	ثابت حرف *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ܐܢ (!(state & accept) || (state & reject))
		ܦܢܐ;

	ܐܢ (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	ܐܚܪܢܐ ܐܢ (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} ܐܚܪܢܐ ܐܢ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} ܐܚܪܢܐ ܐܢ (state & RC_SERVICE_STARTED) {
		errno = 0;
		ܐܢ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_value_get(service, "child_pid");
			start_time = rc_service_value_get(service, "start_time");
			ܐܢ (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			ܐܚܪܢܐ
				xasprintf(&status, " crashed ");
			حرر(child_pid);
			حرر(start_time);
		} ܐܚܪܢܐ {
			uptime = get_uptime(service);
			ܐܢ (uptime) {
				xasprintf(&status, " started %s", uptime);
				حرر(uptime);
			} ܐܚܪܢܐ
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} ܐܚܪܢܐ ܐܢ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} ܐܚܪܢܐ ܐܢ (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} ܐܚܪܢܐ
		xasprintf(&status, " stopped ");

	errno = 0;
	بدل (format) {
	حالة FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		ܐܢ (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		اكسر;
	حالة FORMAT_INI:
		printf("%s = %s\n", service, status);
		اكسر;
	}
	حرر(status);
}

ساكن فراغ print_services_in_state(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
		تعداد format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = عدم;
	RC_STRING *s;
	حرف *r = عدم;

	ܐܢ (!svcs)
		ܦܢܐ;
	ܐܢ (!deptree)
		deptree = _rc_deptree_load(0, عدم);
	ܐܢ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ܐܢ (!runlevel ||
			    rc_service_in_runlevel(s->value, runlevel))
				print_service(s->value, format, accept, reject);
		ܦܢܐ;
	}
	ܐܢ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	ܐܢ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	حرر(r);
	ܐܢ (!l)
		ܦܢܐ;
	TAILQ_FOREACH(s, l, entries) {
		ܐܢ (!rc_stringlist_find(svcs, s->value))
			استمر;
		ܐܢ (!runlevel || rc_service_in_runlevel(s->value, runlevel))
			print_service(s->value, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ساكن فراغ print_services(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
	تعداد format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

ساكن فراغ print_stacked_services(ثابت حرف *runlevel, تعداد format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ܐܢ (rc_stringlist_find(levels, stackedlevel->value) != عدم)
			استمر;
		print_level("Stacked", stackedlevel->value, format);
		servicelist = rc_services_in_runlevel(stackedlevel->value);
		print_services_in_state(stackedlevel->value, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = عدم;
}

صحيح main(صحيح argc, حرف **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ثابت rc_service_state_name_t *it;
	تعداد format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	حرف *p, *runlevel = عدم;
	صحيح opt, retval = 0;

	applet = basename_c(argv[0]);
	ܟܡܐ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة 'a':
			show_all = true;
			levels = rc_runlevel_list();
			اكسر;
		حالة 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ܐܢ (rc_service_daemons_crashed(s->value)) {
					printf("%s\n", s->value);
					retval = 0;
				}
			اذهب exit;
			/* NOTREACHED */
		حالة 'f':
			ܐܢ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ܐܚܪܢܐ
				eerrorx("%s: invalid argument to --format switch", applet);
			اكسر;
		حالة 'i':
			ܐܢ (accept == (RC_SERVICE)-1)
				accept = 0;
			لكل (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ܐܢ (it->state == RC_SERVICE_WASINACTIVE)
					استمر;
				ܐܢ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					اكسر;
				}
			}
			ܐܢ (it->name == عدم)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ܐܢ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ܐܚܪܢܐ ܐܢ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ܐܢ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ܐܚܪܢܐ ܐܢ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			اكسر;
		حالة 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->value);
			اذهب exit;
			/* NOTREACHED */
		حالة 'm':
			services = rc_services_in_runlevel(عدم);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ܐܢ (rc_service_in_runlevel(s->value, l->value)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->value);
						حرر(s);
						اكسر;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				ܐܢ (rc_service_state(s->value) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->value);
					حرر(s);
				}
			print_services(عدم, services, format);
			اذهب exit;
			/* NOTREACHED */
		حالة 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			اذهب exit;
			/* NOTREACHED */
		حالة 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				حرف *ret = rc_service_value_get(s->value, "child_pid");
				ܐܢ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->value);
					حرر(s);
				}
				حرر(ret);
			}
			print_services(عدم, services, format);
			اذهب exit;
			/* NOTREACHED */
		حالة 's':
			services = rc_services_in_runlevel(عدم);
			print_services(عدم, services, format);
			اذهب exit;
			/* NOTREACHED */
		حالة 'u':
			services = rc_services_in_runlevel(عدم);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ܐܢ (rc_service_in_runlevel(s->value, l->value)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->value);
						حرر(s);
						اكسر;
					}
			}
			print_services(عدم, services, format);
			اذهب exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ܐܢ (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ܟܡܐ (optind < argc) {
		ܐܢ (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} ܐܚܪܢܐ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ܐܢ (opt == 0)
		exit(EXIT_FAILURE);
	ܐܢ (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, عدم);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(عدم, l->value, format);
		services = rc_services_in_runlevel(l->value);
		print_services_in_state(l->value, services, format, accept, reject);
		print_stacked_services(l->value, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = عدم;
		rc_stringlist_free(services);
		services = عدم;
	}

	ܐܢ (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(عدم, services, format, accept, reject);
		rc_stringlist_free(services);
		services = عدم;

		/* Show manually started and unassigned depended services */
		ܐܢ (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			ܐܢ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(عدم);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->value);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->value);
			ܐܢ ((rc_stringlist_find(sservices, s->value) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ܐܢ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->value);
					حرر(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->value;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->value = s->value;
				setenv("RC_SVCNAME", l->value, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->value, RC_DEP_TRACE);
				ܐܢ (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_INSERT_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->value = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(عدم, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(عدم, services, format, accept, reject);
	}

exit:
	حرر(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	ܦܢܐ retval;
}
