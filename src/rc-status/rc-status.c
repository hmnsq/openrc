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

ساكن RC_STRINGLIST *پدھراں, *services, *tmp, *alist;
ساكن RC_STRINGLIST *sservices, *nservices, *needsme;

ساكن فراغ print_level(ثابت حرف *اگیتر, ثابت حرف *level,
		تعداد format_t format)
{
	بدل (format) {
	حالة FORMAT_DEFAULT:
		إذا (اگیتر)
			لکھو("%s ", اگیتر);
		لکھو ("Runlevel: ");
		إذا (isatty(fileno(stdout)))
			لکھو("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_عام));
		وإلا
			لکھو("%s\n", level);
		اكسر;
	حالة FORMAT_INI:
		لکھو("%s", "[");
		إذا (اگیتر)
			لکھو("%s ", اگیتر);
		لکھو("%s]\n", level);
		اكسر;
	}
}

ساكن حرف *get_uptime(ثابت حرف *service)
{
	RC_SERVICE state = rc_service_state(service);
	حرف *شروع_گنتی;
	حرف *شروع_ویلا_string;
	time_t شروع_ویلا;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	حرف *uptime = عدم;

	إذا (state & RC_SERVICE_STARTED) {
		شروع_گنتی = rc_service_قدر_get(service, "start_count");
		شروع_ویلا_string = rc_service_قدر_get(service, "start_time");
		إذا (شروع_گنتی && شروع_ویلا_string) {
			شروع_ویلا = منزل_ویلا_t(شروع_ویلا_string);
			diff_secs = (int64_t) difftime(time(عدم), شروع_ویلا);
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
						شروع_گنتی);
			وإلا
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, شروع_گنتی);
		}
		حرر(شروع_گنتی);
		حرر(شروع_ویلا_string);
	}
	أرجع uptime;
}

ساكن فراغ print_service(ثابت حرف *service, تعداد format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	حرف *حالت = عدم;
	حرف *uptime = عدم;
	حرف *بچہ_pid = عدم;
	حرف *شروع_ویلا = عدم;
	صحيح خانے_گنتی;
	ثابت حرف *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	إذا (!(state & accept) || (state & reject))
		أرجع;

	إذا (state & RC_SERVICE_STOPPING)
		xasprintf(&حالت, "stopping ");
	وإلا إذا (state & RC_SERVICE_STARTING) {
		xasprintf(&حالت, "starting ");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_INACTIVE) {
		xasprintf(&حالت, "inactive ");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_STARTED) {
		errno = 0;
		إذا (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			بچہ_pid = rc_service_قدر_get(service, "child_pid");
			شروع_ویلا = rc_service_قدر_get(service, "start_time");
			إذا (شروع_ویلا && بچہ_pid)
				xasprintf(&حالت, " unsupervised ");
			وإلا
				xasprintf(&حالت, " crashed ");
			حرر(بچہ_pid);
			حرر(شروع_ویلا);
		} وإلا {
			uptime = get_uptime(service);
			إذا (uptime) {
				xasprintf(&حالت, " started %s", uptime);
				حرر(uptime);
			} وإلا
				xasprintf(&حالت, " started ");
			color = ECOLOR_GOOD;
		}
	} وإلا إذا (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&حالت, "scheduled");
		color = ECOLOR_WARN;
	} وإلا إذا (state & RC_SERVICE_FAILED) {
		xasprintf(&حالت, "failed");
		color = ECOLOR_WARN;
	} وإلا
		xasprintf(&حالت, " stopped ");

	errno = 0;
	بدل (format) {
	حالة FORMAT_DEFAULT:
		خانے_گنتی =  لکھو(" %s", service);
		إذا (c && *c && isatty(fileno(stdout)))
			لکھو("\n");
		ebracket(خانے_گنتی, color, حالت);
		اكسر;
	حالة FORMAT_INI:
		لکھو("%s = %s\n", service, حالت);
		اكسر;
	}
	حرر(حالت);
}

ساكن فراغ print_services_وچ_state(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
		تعداد format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = عدم;
	RC_STRING *s;
	حرف *r = عدم;

	إذا (!svcs)
		أرجع;
	إذا (!deptree)
		deptree = rc_deptree_لدو_2(0, عدم);
	إذا (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			إذا (!runlevel ||
			    rc_service_وچ_runlevel(s->قدر, runlevel))
				print_service(s->قدر, format, accept, reject);
		أرجع;
	}
	إذا (!types) {
		types = rc_stringlist_new();
		rc_stringlist_جوڑو(types, "ineed");
		rc_stringlist_جوڑو(types, "iuse");
		rc_stringlist_جوڑو(types, "iafter");
	}
	إذا (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_شروع);
	حرر(r);
	إذا (!l)
		أرجع;
	TAILQ_FOREACH(s, l, entries) {
		إذا (!rc_stringlist_لبھو(svcs, s->قدر))
			استمر;
		إذا (!runlevel || rc_service_وچ_runlevel(s->قدر, runlevel))
			print_service(s->قدر, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ساكن فراغ print_services(ثابت حرف *runlevel, RC_STRINGLIST *svcs,
	تعداد format_t format)
{
	print_services_وچ_state(runlevel, svcs, format, -1, 0);
}

ساكن فراغ print_stacked_services(ثابت حرف *runlevel, تعداد format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		إذا (rc_stringlist_لبھو(پدھراں, stackedlevel->قدر) != عدم)
			استمر;
		print_level("Stacked", stackedlevel->قدر, format);
		servicelist = rc_services_وچ_runlevel(stackedlevel->قدر);
		print_services_وچ_state(stackedlevel->قدر, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = عدم;
}

صحيح مکھ(صحيح argc, حرف **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ثابت rc_service_state_name_t *it;
	تعداد format_t format = FORMAT_DEFAULT;
	bool پدھراں_given = false;
	bool show_all = false;
	حرف *p, *runlevel = عدم;
	صحيح opt, retval = 0;

	applet = basename_c(argv[0]);
	طالما ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (صحيح *) 0)) != -1)
		بدل (opt) {
		حالة 'a':
			show_all = true;
			پدھراں = rc_runlevel_list();
			اكسر;
		حالة 'c':
			services = rc_services_وچ_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				إذا (rc_service_daemons_crashed(s->قدر)) {
					لکھو("%s\n", s->قدر);
					retval = 0;
				}
			اذهب باہر_نکلو;
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
			لكل (it = rc_service_state_ناں; it->name; ++it) {
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
			پدھراں = rc_runlevel_list();
			TAILQ_FOREACH(l, پدھراں, entries)
				لکھو("%s\n", l->قدر);
			اذهب باہر_نکلو;
			/* NOTREACHED */
		حالة 'm':
			services = rc_services_وچ_runlevel(عدم);
			پدھراں = rc_runlevel_list();
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				TAILQ_FOREACH(l, پدھراں, entries)
					إذا (rc_service_وچ_runlevel(s->قدر, l->قدر)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->قدر);
						حرر(s);
						اكسر;
					}
			}
			TAILQ_FOREACH_محفوظ(s, services, entries, t)
				إذا (rc_service_state(s->قدر) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->قدر);
					حرر(s);
				}
			print_services(عدم, services, format);
			اذهب باہر_نکلو;
			/* NOTREACHED */
		حالة 'r':
			runlevel = rc_runlevel_get();
			لکھو("%s\n", runlevel ? runlevel : "");
			اذهب باہر_نکلو;
			/* NOTREACHED */
		حالة 'S':
			services = rc_services_وچ_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				حرف *ret = rc_service_قدر_get(s->قدر, "child_pid");
				إذا (!ret) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->قدر);
					حرر(s);
				}
				حرر(ret);
			}
			print_services(عدم, services, format);
			اذهب باہر_نکلو;
			/* NOTREACHED */
		حالة 's':
			services = rc_services_وچ_runlevel(عدم);
			print_services(عدم, services, format);
			اذهب باہر_نکلو;
			/* NOTREACHED */
		حالة 'u':
			services = rc_services_وچ_runlevel(عدم);
			پدھراں = rc_runlevel_list();
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				TAILQ_FOREACH(l, پدھراں, entries)
					إذا (rc_service_وچ_runlevel(s->قدر, l->قدر)) {
						TAILQ_REMOVE(services, s, entries);
						حرر(s->قدر);
						حرر(s);
						اكسر;
					}
			}
			print_services(عدم, services, format);
			اذهب باہر_نکلو;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	إذا (!پدھراں)
		پدھراں = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	طالما (optind < argc) {
		إذا (rc_runlevel_exists(argv[optind])) {
			پدھراں_given = true;
			rc_stringlist_جوڑو(پدھراں, argv[optind++]);
			opt++;
		} وإلا
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	إذا (opt == 0)
		باہر_نکلو(EXIT_FAILURE);
	إذا (!TAILQ_پہلا(پدھراں)) {
		runlevel = default_runlevel();
		rc_stringlist_جوڑو(پدھراں, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = rc_deptree_لدو_2(0, عدم);

	TAILQ_FOREACH(l, پدھراں, entries) {
		print_level(عدم, l->قدر, format);
		services = rc_services_وچ_runlevel(l->قدر);
		print_services_وچ_state(l->قدر, services, format, accept, reject);
		print_stacked_services(l->قدر, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = عدم;
		rc_stringlist_free(services);
		services = عدم;
	}

	إذا (show_all || !پدھراں_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_وچ_state(RC_SERVICE_HOTPLUGGED);
		print_services_وچ_state(عدم, services, format, accept, reject);
		rc_stringlist_free(services);
		services = عدم;

		/* Show manually started and unassigned depended services */
		إذا (show_all) {
			rc_stringlist_free(پدھراں);
			پدھراں = rc_stringlist_new();
			إذا (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_جوڑو(پدھراں, runlevel);
		}
		rc_stringlist_جوڑو(پدھراں, RC_LEVEL_SYSINIT);
		rc_stringlist_جوڑو(پدھراں, RC_LEVEL_BOOT);
		services = rc_services_وچ_runlevel(عدم);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, پدھراں, entries) {
			nservices = rc_services_وچ_runlevel_stacked(l->قدر);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_محفوظ(s, services, entries, t) {
			state = rc_service_state(s->قدر);
			إذا ((rc_stringlist_لبھو(sservices, s->قدر) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				إذا (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					حرر(s->قدر);
					حرر(s);
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
		TAILQ_FOREACH(level, پدھراں, entries) {
			TAILQ_FOREACH_محفوظ(s, services, entries, t) {
				l->قدر = s->قدر;
				setenv("RC_SVCNAME", l->قدر, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->قدر, RC_DEP_TRACE);
				إذا (TAILQ_پہلا(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_داخل_کرو_پوچھ(nservices, s, entries);
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
		print_services_وچ_state(عدم, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_وچ_state(عدم, services, format, accept, reject);
	}

باہر_نکلو:
	حرر(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(پدھراں);
	rc_deptree_free(deptree);

	أرجع retval;
}
