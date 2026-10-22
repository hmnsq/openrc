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

شمار format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ثابت حرف *applet = تہی;
ثابت حرف *extraopts = تہی;
ثابت حرف getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ثابت ساخت option longopts[] = {
	{"all",         0, تہی, 'a'},
	{"crashed",     0, تہی, 'c'},
	{"format",     1, تہی, 'f'},
	{"in-state",   1, تہی, 'i'},
	{"list",        0, تہی, 'l'},
	{"manual",        0, تہی, 'm'},
	{"runlevel",    0, تہی, 'r'},
	{"servicelist", 0, تہی, 's'},
	{"supervised", 0, تہی, 'S'},
	{"unused",      0, تہی, 'u'},
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

ساکن RC_DEPTREE *deptree;
ساکن RC_STRINGLIST *types;

ساکن RC_STRINGLIST *سطحیں, *services, *tmp, *alist;
ساکن RC_STRINGLIST *sservices, *nservices, *needsme;

ساکن خالی print_level(ثابت حرف *سابقہ, ثابت حرف *level,
		شمار format_t format)
{
	انتخاب (format) {
	حالت FORMAT_DEFAULT:
		اگر (سابقہ)
			لکھو("%s ", سابقہ);
		لکھو ("Runlevel: ");
		اگر (isatty(fileno(stdout)))
			لکھو("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_معمول));
		ورنہ
			لکھو("%s\n", level);
		توڑو;
	حالت FORMAT_INI:
		لکھو("%s", "[");
		اگر (سابقہ)
			لکھو("%s ", سابقہ);
		لکھو("%s]\n", level);
		توڑو;
	}
}

ساکن حرف *get_uptime(ثابت حرف *service)
{
	RC_SERVICE state = rc_service_state(service);
	حرف *آغاز_گنتی;
	حرف *آغاز_وقت_string;
	time_t آغاز_وقت;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	حرف *uptime = تہی;

	اگر (state & RC_SERVICE_STARTED) {
		آغاز_گنتی = rc_service_قدر_get(service, "start_count");
		آغاز_وقت_string = rc_service_قدر_get(service, "start_time");
		اگر (آغاز_گنتی && آغاز_وقت_string) {
			آغاز_وقت = مقصد_وقت_t(آغاز_وقت_string);
			diff_secs = (int64_t) difftime(time(تہی), آغاز_وقت);
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
						آغاز_گنتی);
			ورنہ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, آغاز_گنتی);
		}
		آزاد(آغاز_گنتی);
		آزاد(آغاز_وقت_string);
	}
	واپس uptime;
}

ساکن خالی print_service(ثابت حرف *service, شمار format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	حرف *حالت_2 = تہی;
	حرف *uptime = تہی;
	حرف *بچہ_pid = تہی;
	حرف *آغاز_وقت = تہی;
	صحیح ستون_تعداد;
	ثابت حرف *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	اگر (!(state & accept) || (state & reject))
		واپس;

	اگر (state & RC_SERVICE_STOPPING)
		xasprintf(&حالت_2, "stopping ");
	ورنہ اگر (state & RC_SERVICE_STARTING) {
		xasprintf(&حالت_2, "starting ");
		color = ECOLOR_WARN;
	} ورنہ اگر (state & RC_SERVICE_INACTIVE) {
		xasprintf(&حالت_2, "inactive ");
		color = ECOLOR_WARN;
	} ورنہ اگر (state & RC_SERVICE_STARTED) {
		errno = 0;
		اگر (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			بچہ_pid = rc_service_قدر_get(service, "child_pid");
			آغاز_وقت = rc_service_قدر_get(service, "start_time");
			اگر (آغاز_وقت && بچہ_pid)
				xasprintf(&حالت_2, " unsupervised ");
			ورنہ
				xasprintf(&حالت_2, " crashed ");
			آزاد(بچہ_pid);
			آزاد(آغاز_وقت);
		} ورنہ {
			uptime = get_uptime(service);
			اگر (uptime) {
				xasprintf(&حالت_2, " started %s", uptime);
				آزاد(uptime);
			} ورنہ
				xasprintf(&حالت_2, " started ");
			color = ECOLOR_GOOD;
		}
	} ورنہ اگر (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&حالت_2, "scheduled");
		color = ECOLOR_WARN;
	} ورنہ اگر (state & RC_SERVICE_FAILED) {
		xasprintf(&حالت_2, "failed");
		color = ECOLOR_WARN;
	} ورنہ
		xasprintf(&حالت_2, " stopped ");

	errno = 0;
	انتخاب (format) {
	حالت FORMAT_DEFAULT:
		ستون_تعداد =  لکھو(" %s", service);
		اگر (c && *c && isatty(fileno(stdout)))
			لکھو("\n");
		ebracket(ستون_تعداد, color, حالت_2);
		توڑو;
	حالت FORMAT_INI:
		لکھو("%s = %s\n", service, حالت_2);
		توڑو;
	}
	آزاد(حالت_2);
}

ساکن خالی print_services_اندر_state(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
		شمار format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = تہی;
	RC_STRING *s;
	حرف *r = تہی;

	اگر (!svcs)
		واپس;
	اگر (!deptree)
		deptree = rc_deptree_بھرو_2(0, تہی);
	اگر (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			اگر (!runlevel ||
			    rc_service_اندر_runlevel(s->قدر, runlevel))
				print_service(s->قدر, format, accept, reject);
		واپس;
	}
	اگر (!types) {
		types = rc_stringlist_new();
		rc_stringlist_جوڑو(types, "ineed");
		rc_stringlist_جوڑو(types, "iuse");
		rc_stringlist_جوڑو(types, "iafter");
	}
	اگر (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_آغاز);
	آزاد(r);
	اگر (!l)
		واپس;
	TAILQ_FOREACH(s, l, entries) {
		اگر (!rc_stringlist_ڈھونڈو(svcs, s->قدر))
			جاری;
		اگر (!runlevel || rc_service_اندر_runlevel(s->قدر, runlevel))
			print_service(s->قدر, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ساکن خالی print_services(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
	شمار format_t format)
{
	print_services_اندر_state(runlevel, svcs, format, -1, 0);
}

ساکن خالی print_stacked_services(ثابت حرف *runlevel, شمار format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		اگر (rc_stringlist_ڈھونڈو(سطحیں, stackedlevel->قدر) != تہی)
			جاری;
		print_level("Stacked", stackedlevel->قدر, format);
		servicelist = rc_services_اندر_runlevel(stackedlevel->قدر);
		print_services_اندر_state(stackedlevel->قدر, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = تہی;
}

صحیح مین(صحیح argc, حرف **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ثابت rc_service_state_name_t *it;
	شمار format_t format = FORMAT_DEFAULT;
	bool سطحیں_given = false;
	bool show_all = false;
	حرف *p, *runlevel = تہی;
	صحیح opt, retval = 0;

	applet = basename_c(argv[0]);
	جب ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (صحیح *) 0)) != -1)
		انتخاب (opt) {
		حالت 'a':
			show_all = true;
			سطحیں = rc_runlevel_list();
			توڑو;
		حالت 'c':
			services = rc_services_اندر_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				اگر (rc_service_daemons_crashed(s->قدر)) {
					لکھو("%s\n", s->قدر);
					retval = 0;
				}
			جاؤ باہر_نکلو;
			/* NOTREACHED */
		حالت 'f':
			اگر (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ورنہ
				eerrorx("%s: invalid argument to --format switch", applet);
			توڑو;
		حالت 'i':
			اگر (accept == (RC_SERVICE)-1)
				accept = 0;
			برائے (it = rc_service_state_نام; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				اگر (it->state == RC_SERVICE_WASINACTIVE)
					جاری;
				اگر (strcmp(optarg, it->name) == 0) {
					state = it->state;
					توڑو;
				}
			}
			اگر (it->name == تہی)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			اگر (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ورنہ اگر (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			اگر (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ورنہ اگر (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			توڑو;
		حالت 'l':
			سطحیں = rc_runlevel_list();
			TAILQ_FOREACH(l, سطحیں, entries)
				لکھو("%s\n", l->قدر);
			جاؤ باہر_نکلو;
			/* NOTREACHED */
		حالت 'm':
			services = rc_services_اندر_runlevel(تہی);
			سطحیں = rc_runlevel_list();
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				TAILQ_FOREACH(l, سطحیں, entries)
					اگر (rc_service_اندر_runlevel(s->قدر, l->قدر)) {
						TAILQ_REMOVE(services, s, entries);
						آزاد(s->قدر);
						آزاد(s);
						توڑو;
					}
			}
			TAILQ_FOREACH_محفوظ(s, services, entries, t)
				اگر (rc_service_state(s->قدر) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					آزاد(s->قدر);
					آزاد(s);
				}
			print_services(تہی, services, format);
			جاؤ باہر_نکلو;
			/* NOTREACHED */
		حالت 'r':
			runlevel = rc_runlevel_get();
			لکھو("%s\n", runlevel ? runlevel : "");
			جاؤ باہر_نکلو;
			/* NOTREACHED */
		حالت 'S':
			services = rc_services_اندر_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				حرف *ret = rc_service_قدر_get(s->قدر, "child_pid");
				اگر (!ret) {
					TAILQ_REMOVE(services, s, entries);
					آزاد(s->قدر);
					آزاد(s);
				}
				آزاد(ret);
			}
			print_services(تہی, services, format);
			جاؤ باہر_نکلو;
			/* NOTREACHED */
		حالت 's':
			services = rc_services_اندر_runlevel(تہی);
			print_services(تہی, services, format);
			جاؤ باہر_نکلو;
			/* NOTREACHED */
		حالت 'u':
			services = rc_services_اندر_runlevel(تہی);
			سطحیں = rc_runlevel_list();
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				TAILQ_FOREACH(l, سطحیں, entries)
					اگر (rc_service_اندر_runlevel(s->قدر, l->قدر)) {
						TAILQ_REMOVE(services, s, entries);
						آزاد(s->قدر);
						آزاد(s);
						توڑو;
					}
			}
			print_services(تہی, services, format);
			جاؤ باہر_نکلو;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	اگر (!سطحیں)
		سطحیں = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	جب (optind < argc) {
		اگر (rc_runlevel_exists(argv[optind])) {
			سطحیں_given = true;
			rc_stringlist_جوڑو(سطحیں, argv[optind++]);
			opt++;
		} ورنہ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	اگر (opt == 0)
		باہر_نکلو(EXIT_FAILURE);
	اگر (!TAILQ_پہلا(سطحیں)) {
		runlevel = default_runlevel();
		rc_stringlist_جوڑو(سطحیں, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = rc_deptree_بھرو_2(0, تہی);

	TAILQ_FOREACH(l, سطحیں, entries) {
		print_level(تہی, l->قدر, format);
		services = rc_services_اندر_runlevel(l->قدر);
		print_services_اندر_state(l->قدر, services, format, accept, reject);
		print_stacked_services(l->قدر, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = تہی;
		rc_stringlist_free(services);
		services = تہی;
	}

	اگر (show_all || !سطحیں_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_اندر_state(RC_SERVICE_HOTPLUGGED);
		print_services_اندر_state(تہی, services, format, accept, reject);
		rc_stringlist_free(services);
		services = تہی;

		/* Show manually started and unassigned depended services */
		اگر (show_all) {
			rc_stringlist_free(سطحیں);
			سطحیں = rc_stringlist_new();
			اگر (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_جوڑو(سطحیں, runlevel);
		}
		rc_stringlist_جوڑو(سطحیں, RC_LEVEL_SYSINIT);
		rc_stringlist_جوڑو(سطحیں, RC_LEVEL_BOOT);
		services = rc_services_اندر_runlevel(تہی);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, سطحیں, entries) {
			nservices = rc_services_اندر_runlevel_stacked(l->قدر);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_محفوظ(s, services, entries, t) {
			state = rc_service_state(s->قدر);
			اگر ((rc_stringlist_ڈھونڈو(sservices, s->قدر) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				اگر (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					آزاد(s->قدر);
					آزاد(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_جوڑو(needsme, "needsme");
		rc_stringlist_جوڑو(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_جوڑو(alist, "");
		p = l->قدر;
		TAILQ_FOREACH(level, سطحیں, entries) {
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				l->قدر = s->قدر;
				setenv("RC_SVCNAME", l->قدر, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->قدر, RC_DEP_TRACE);
				اگر (TAILQ_پہلا(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_داخل_کرو_دم(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->قدر = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_اندر_state(تہی, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_اندر_state(تہی, services, format, accept, reject);
	}

باہر_نکلو:
	آزاد(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(سطحیں);
	rc_deptree_free(deptree);

	واپس retval;
}
