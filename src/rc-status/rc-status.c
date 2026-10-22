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

ساكن RC_STRINGLIST *سطوح, *services, *tmp, *alist;
ساكن RC_STRINGLIST *sservices, *nservices, *needsme;

ساكن فراغ print_level(ثابت حرف *پیشوند, ثابت حرف *level,
		تعداد format_t format)
{
	بدل (format) {
	حالة FORMAT_DEFAULT:
		إذا (پیشوند)
			چاپ_کن("%s ", پیشوند);
		چاپ_کن ("Runlevel: ");
		إذا (isatty(fileno(stdout)))
			چاپ_کن("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_نرمال));
		وإلا
			چاپ_کن("%s\n", level);
		اكسر;
	حالة FORMAT_INI:
		چاپ_کن("%s", "[");
		إذا (پیشوند)
			چاپ_کن("%s ", پیشوند);
		چاپ_کن("%s]\n", level);
		اكسر;
	}
}

ساكن حرف *get_uptime(ثابت حرف *service)
{
	RC_SERVICE state = rc_service_state(service);
	حرف *شروع_شمار;
	حرف *شروع_زمان_string;
	time_t شروع_زمان;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	حرف *uptime = عدم;

	إذا (state & RC_SERVICE_STARTED) {
		شروع_شمار = rc_service_مقدار_get(service, "start_count");
		شروع_زمان_string = rc_service_مقدار_get(service, "start_time");
		إذا (شروع_شمار && شروع_زمان_string) {
			شروع_زمان = مقصد_زمان_t(شروع_زمان_string);
			diff_secs = (int64_t) difftime(time(عدم), شروع_زمان);
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
						شروع_شمار);
			وإلا
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, شروع_شمار);
		}
		حرر(شروع_شمار);
		حرر(شروع_زمان_string);
	}
	أرجع uptime;
}

ساكن فراغ print_service(ثابت حرف *service, تعداد format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	حرف *وضعیت = عدم;
	حرف *uptime = عدم;
	حرف *فرزند_pid = عدم;
	حرف *شروع_زمان = عدم;
	صحيح تعداد_ستون;
	ثابت حرف *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	إذا (!(state & accept) || (state & reject))
		أرجع;

	إذا (state & RC_SERVICE_STOPPING)
		xasprintf(&وضعیت, "stopping ");
	وإلا إذا (state & RC_SERVICE_STARTING) {
		xasprintf(&وضعیت, "starting ");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_INACTIVE) {
		xasprintf(&وضعیت, "inactive ");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_STARTED) {
		errno = 0;
		إذا (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			فرزند_pid = rc_service_مقدار_get(service, "child_pid");
			شروع_زمان = rc_service_مقدار_get(service, "start_time");
			إذا (شروع_زمان && فرزند_pid)
				xasprintf(&وضعیت, " unsupervised ");
			وإلا
				xasprintf(&وضعیت, " crashed ");
			حرر(فرزند_pid);
			حرر(شروع_زمان);
		} وإلا {
			uptime = get_uptime(service);
			إذا (uptime) {
				xasprintf(&وضعیت, " started %s", uptime);
				حرر(uptime);
			} وإلا
				xasprintf(&وضعیت, " started ");
			color = ECOLOR_GOOD;
		}
	} وإلا إذا (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&وضعیت, "scheduled");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_FAILED) {
		xasprintf(&وضعیت, "failed");
		color = ECOLOR_WARN;
	} وإلا
		xasprintf(&وضعیت, " stopped ");

	errno = 0;
	بدل (format) {
	حالة FORMAT_DEFAULT:
		تعداد_ستون =  چاپ_کن(" %s", service);
		إذا (c && *c && isatty(fileno(stdout)))
			چاپ_کن("\n");
		ebracket(تعداد_ستون, color, وضعیت);
		اكسر;
	حالة FORMAT_INI:
		چاپ_کن("%s = %s\n", service, وضعیت);
		اكسر;
	}
	حرر(وضعیت);
}

ساكن فراغ print_services_درون_state(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
		تعداد format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = عدم;
	RC_STRING *s;
	حرف *r = عدم;

	إذا (!svcs)
		أرجع;
	إذا (!deptree)
		deptree = rc_deptree_بارگیری_2(0, عدم);
	إذا (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			إذا (!runlevel ||
			    rc_service_درون_runlevel(s->مقدار, runlevel))
				print_service(s->مقدار, format, accept, reject);
		أرجع;
	}
	إذا (!types) {
		types = rc_stringlist_new();
		rc_stringlist_جمع_کن(types, "ineed");
		rc_stringlist_جمع_کن(types, "iuse");
		rc_stringlist_جمع_کن(types, "iafter");
	}
	إذا (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_شروع);
	حرر(r);
	إذا (!l)
		أرجع;
	TAILQ_FOREACH(s, l, entries) {
		إذا (!rc_stringlist_پیدا_کن(svcs, s->مقدار))
			استمر;
		إذا (!runlevel || rc_service_درون_runlevel(s->مقدار, runlevel))
			print_service(s->مقدار, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ساكن فراغ print_services(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
	تعداد format_t format)
{
	print_services_درون_state(runlevel, svcs, format, -1, 0);
}

ساكن فراغ print_stacked_services(ثابت حرف *runlevel, تعداد format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		إذا (rc_stringlist_پیدا_کن(سطوح, stackedlevel->مقدار) != عدم)
			استمر;
		print_level("Stacked", stackedlevel->مقدار, format);
		servicelist = rc_services_درون_runlevel(stackedlevel->مقدار);
		print_services_درون_state(stackedlevel->مقدار, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = عدم;
}

صحيح اصلی(صحيح argc, حرف **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ثابت rc_service_state_name_t *it;
	تعداد format_t format = FORMAT_DEFAULT;
	bool سطوح_given = false;
	bool show_all = false;
	حرف *p, *runlevel = عدم;
	صحيح opt, retval = 0;

	applet = basename_c(argv[0]);
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة 'a':
			show_all = true;
			سطوح = rc_runlevel_list();
			اكسر;
		حالة 'c':
			services = rc_services_درون_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				إذا (rc_service_daemons_crashed(s->مقدار)) {
					چاپ_کن("%s\n", s->مقدار);
					retval = 0;
				}
			اذهب خروج;
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
			لكل (it = rc_service_state_نامها; it->name; ++it) {
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
			سطوح = rc_runlevel_list();
			TAILQ_FOREACH(l, سطوح, entries)
				چاپ_کن("%s\n", l->مقدار);
			اذهب خروج;
			/* NOTREACHED */
		حالة 'm':
			services = rc_services_درون_runlevel(عدم);
			سطوح = rc_runlevel_list();
			TAILQ_FOREACH_امن(s, services, entries, t) {
				TAILQ_FOREACH(l, سطوح, entries)
					إذا (rc_service_درون_runlevel(s->مقدار, l->مقدار)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->مقدار);
						حرر(s);
						اكسر;
					}
			}
			TAILQ_FOREACH_امن(s, services, entries, t)
				إذا (rc_service_state(s->مقدار) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->مقدار);
					حرر(s);
				}
			print_services(عدم, services, format);
			اذهب خروج;
			/* NOTREACHED */
		حالة 'r':
			runlevel = rc_runlevel_get();
			چاپ_کن("%s\n", runlevel ? runlevel : "");
			اذهب خروج;
			/* NOTREACHED */
		حالة 'S':
			services = rc_services_درون_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_امن(s, services, entries, t) {
				حرف *ret = rc_service_مقدار_get(s->مقدار, "child_pid");
				إذا (!ret) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->مقدار);
					حرر(s);
				}
				حرر(ret);
			}
			print_services(عدم, services, format);
			اذهب خروج;
			/* NOTREACHED */
		حالة 's':
			services = rc_services_درون_runlevel(عدم);
			print_services(عدم, services, format);
			اذهب خروج;
			/* NOTREACHED */
		حالة 'u':
			services = rc_services_درون_runlevel(عدم);
			سطوح = rc_runlevel_list();
			TAILQ_FOREACH_امن(s, services, entries, t) {
				TAILQ_FOREACH(l, سطوح, entries)
					إذا (rc_service_درون_runlevel(s->مقدار, l->مقدار)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->مقدار);
						حرر(s);
						اكسر;
					}
			}
			print_services(عدم, services, format);
			اذهب خروج;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	إذا (!سطوح)
		سطوح = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	طالما (optind < argc) {
		إذا (rc_runlevel_exists(argv[optind])) {
			سطوح_given = true;
			rc_stringlist_جمع_کن(سطوح, argv[optind++]);
			opt++;
		} وإلا
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	إذا (opt == 0)
		خروج(EXIT_FAILURE);
	إذا (!TAILQ_اولی(سطوح)) {
		runlevel = default_runlevel();
		rc_stringlist_جمع_کن(سطوح, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = rc_deptree_بارگیری_2(0, عدم);

	TAILQ_FOREACH(l, سطوح, entries) {
		print_level(عدم, l->مقدار, format);
		services = rc_services_درون_runlevel(l->مقدار);
		print_services_درون_state(l->مقدار, services, format, accept, reject);
		print_stacked_services(l->مقدار, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = عدم;
		rc_stringlist_free(services);
		services = عدم;
	}

	إذا (show_all || !سطوح_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_درون_state(RC_SERVICE_HOTPLUGGED);
		print_services_درون_state(عدم, services, format, accept, reject);
		rc_stringlist_free(services);
		services = عدم;

		/* Show manually started and unassigned depended services */
		إذا (show_all) {
			rc_stringlist_free(سطوح);
			سطوح = rc_stringlist_new();
			إذا (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_جمع_کن(سطوح, runlevel);
		}
		rc_stringlist_جمع_کن(سطوح, RC_LEVEL_SYSINIT);
		rc_stringlist_جمع_کن(سطوح, RC_LEVEL_BOOT);
		services = rc_services_درون_runlevel(عدم);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, سطوح, entries) {
			nservices = rc_services_درون_runlevel_stacked(l->مقدار);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_امن(s, services, entries, t) {
			state = rc_service_state(s->مقدار);
			إذا ((rc_stringlist_پیدا_کن(sservices, s->مقدار) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				إذا (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->مقدار);
					حرر(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_جمع_کن(needsme, "needsme");
		rc_stringlist_جمع_کن(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_جمع_کن(alist, "");
		p = l->مقدار;
		TAILQ_FOREACH(level, سطوح, entries) {
			TAILQ_FOREACH_امن(s, services, entries, t) {
				l->مقدار = s->مقدار;
				setenv("RC_SVCNAME", l->مقدار, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->مقدار, RC_DEP_TRACE);
				إذا (TAILQ_اولی(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_درج_دم(nservices, s, entries);
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
		print_services_درون_state(عدم, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_درون_state(عدم, services, format, accept, reject);
	}

خروج:
	حرر(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(سطوح);
	rc_deptree_free(deptree);

	أرجع retval;
}
