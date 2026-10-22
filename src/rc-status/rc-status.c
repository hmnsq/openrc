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
		إذا (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		إذا (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		وإلا
			printf("%s\n", level);
		اكسر;
	حالة FORMAT_INI:
		printf("%s", "[");
		إذا (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		اكسر;
	}
}

ساكن حرف *get_uptime(ثابت حرف *service)
{
	RC_SERVICE state = rc_service_state(service);
	حرف *start_ژماردن;
	حرف *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	حرف *uptime = عدم;

	إذا (state & RC_SERVICE_STARTED) {
		start_ژماردن = rc_service_بەها_get(service, "start_count");
		start_time_string = rc_service_بەها_get(service, "start_time");
		إذا (start_ژماردن && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(عدم), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			إذا (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_ژماردن);
			وإلا
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_ژماردن);
		}
		حرر(start_ژماردن);
		حرر(start_time_string);
	}
	أرجع uptime;
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

	إذا (!(state & accept) || (state & reject))
		أرجع;

	إذا (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	وإلا إذا (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_STARTED) {
		errno = 0;
		إذا (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_بەها_get(service, "child_pid");
			start_time = rc_service_بەها_get(service, "start_time");
			إذا (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			وإلا
				xasprintf(&status, " crashed ");
			حرر(child_pid);
			حرر(start_time);
		} وإلا {
			uptime = get_uptime(service);
			إذا (uptime) {
				xasprintf(&status, " started %s", uptime);
				حرر(uptime);
			} وإلا
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} وإلا إذا (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} وإلا
		xasprintf(&status, " stopped ");

	errno = 0;
	بدل (format) {
	حالة FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		إذا (c && *c && isatty(fileno(stdout)))
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

	إذا (!svcs)
		أرجع;
	إذا (!deptree)
		deptree = _rc_deptree_load(0, عدم);
	إذا (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			إذا (!runlevel ||
			    rc_service_in_runlevel(s->بەها, runlevel))
				print_service(s->بەها, format, accept, reject);
		أرجع;
	}
	إذا (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	إذا (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	حرر(r);
	إذا (!l)
		أرجع;
	TAILQ_FOREACH(s, l, entries) {
		إذا (!rc_stringlist_find(svcs, s->بەها))
			استمر;
		إذا (!runlevel || rc_service_in_runlevel(s->بەها, runlevel))
			print_service(s->بەها, format, accept, reject);
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
		إذا (rc_stringlist_find(levels, stackedlevel->بەها) != عدم)
			استمر;
		print_level("Stacked", stackedlevel->بەها, format);
		servicelist = rc_services_in_runlevel(stackedlevel->بەها);
		print_services_in_state(stackedlevel->بەها, servicelist,
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
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts,
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
				إذا (rc_service_daemons_crashed(s->بەها)) {
					printf("%s\n", s->بەها);
					retval = 0;
				}
			اذهب exit;
			/* NOTREACHED */
		حالة 'f':
			إذا (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} وإلا
				eerrorx("%s: invalid argument to --format switch", applet);
			اكسر;
		حالة 'i':
			إذا (accept == (RC_SERVICE)-1)
				accept = 0;
			لكل (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				إذا (it->state == RC_SERVICE_WASINACTIVE)
					استمر;
				إذا (strcmp(optarg, it->name) == 0) {
					state = it->state;
					اكسر;
				}
			}
			إذا (it->name == عدم)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			إذا (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} وإلا إذا (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			إذا (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} وإلا إذا (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			اكسر;
		حالة 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->بەها);
			اذهب exit;
			/* NOTREACHED */
		حالة 'm':
			services = rc_services_in_runlevel(عدم);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					إذا (rc_service_in_runlevel(s->بەها, l->بەها)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->بەها);
						حرر(s);
						اكسر;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				إذا (rc_service_state(s->بەها) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->بەها);
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
				حرف *ret = rc_service_بەها_get(s->بەها, "child_pid");
				إذا (!ret) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->بەها);
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
					إذا (rc_service_in_runlevel(s->بەها, l->بەها)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->بەها);
						حرر(s);
						اكسر;
					}
			}
			print_services(عدم, services, format);
			اذهب exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	إذا (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	طالما (optind < argc) {
		إذا (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} وإلا
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	إذا (opt == 0)
		exit(EXIT_FAILURE);
	إذا (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, عدم);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(عدم, l->بەها, format);
		services = rc_services_in_runlevel(l->بەها);
		print_services_in_state(l->بەها, services, format, accept, reject);
		print_stacked_services(l->بەها, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = عدم;
		rc_stringlist_free(services);
		services = عدم;
	}

	إذا (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(عدم, services, format, accept, reject);
		rc_stringlist_free(services);
		services = عدم;

		/* Show manually started and unassigned depended services */
		إذا (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			إذا (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(عدم);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->بەها);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->بەها);
			إذا ((rc_stringlist_find(sservices, s->بەها) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				إذا (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->بەها);
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
		p = l->بەها;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->بەها = s->بەها;
				setenv("RC_SVCNAME", l->بەها, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->بەها, RC_DEP_TRACE);
				إذا (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_زیادکردن_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->بەها = p;
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

	أرجع retval;
}
