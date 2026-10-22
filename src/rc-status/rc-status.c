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

#অন্তর্ভাব <getopt.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <time.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "queue.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "helpers.h"

গণনা format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ধ্রুবক অক্ষর *applet = ফাঁকা;
ধ্রুবক অক্ষর *extraopts = ফাঁকা;
ধ্রুবক অক্ষর getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ধ্রুবক কাঠামো option longopts[] = {
	{"all",         0, ফাঁকা, 'a'},
	{"crashed",     0, ফাঁকা, 'c'},
	{"format",     1, ফাঁকা, 'f'},
	{"in-state",   1, ফাঁকা, 'i'},
	{"list",        0, ফাঁকা, 'l'},
	{"manual",        0, ফাঁকা, 'm'},
	{"runlevel",    0, ফাঁকা, 'r'},
	{"servicelist", 0, ফাঁকা, 's'},
	{"supervised", 0, ফাঁকা, 'S'},
	{"unused",      0, ফাঁকা, 'u'},
	longopts_COMMON
};
ধ্রুবক অক্ষর * ধ্রুবক longopts_help[] = {
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
ধ্রুবক অক্ষর *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

স্থির RC_DEPTREE *deptree;
স্থির RC_STRINGLIST *types;

স্থির RC_STRINGLIST *levels, *services, *tmp, *alist;
স্থির RC_STRINGLIST *sservices, *nservices, *needsme;

স্থির শূন্য print_level(ধ্রুবক অক্ষর *prefix, ধ্রুবক অক্ষর *level,
		গণনা format_t format)
{
	নির্বাচন (format) {
	ক্ষেত্র FORMAT_DEFAULT:
		যদি (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		যদি (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		অন্যথা
			printf("%s\n", level);
		বিরতি;
	ক্ষেত্র FORMAT_INI:
		printf("%s", "[");
		যদি (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		বিরতি;
	}
}

স্থির অক্ষর *get_uptime(ধ্রুবক অক্ষর *service)
{
	RC_SERVICE state = rc_service_state(service);
	অক্ষর *start_গুনতি;
	অক্ষর *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	অক্ষর *uptime = ফাঁকা;

	যদি (state & RC_SERVICE_STARTED) {
		start_গুনতি = rc_service_মান_get(service, "start_count");
		start_time_string = rc_service_মান_get(service, "start_time");
		যদি (start_গুনতি && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(ফাঁকা), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			যদি (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_গুনতি);
			অন্যথা
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_গুনতি);
		}
		মুক্ত(start_গুনতি);
		মুক্ত(start_time_string);
	}
	ফেরত uptime;
}

স্থির শূন্য print_service(ধ্রুবক অক্ষর *service, গণনা format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	অক্ষর *status = ফাঁকা;
	অক্ষর *uptime = ফাঁকা;
	অক্ষর *child_pid = ফাঁকা;
	অক্ষর *start_time = ফাঁকা;
	পূর্ণসংখ্যা cols;
	ধ্রুবক অক্ষর *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	যদি (!(state & accept) || (state & reject))
		ফেরত;

	যদি (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	অন্যথা যদি (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} অন্যথা যদি (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} অন্যথা যদি (state & RC_SERVICE_STARTED) {
		errno = 0;
		যদি (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_মান_get(service, "child_pid");
			start_time = rc_service_মান_get(service, "start_time");
			যদি (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			অন্যথা
				xasprintf(&status, " crashed ");
			মুক্ত(child_pid);
			মুক্ত(start_time);
		} অন্যথা {
			uptime = get_uptime(service);
			যদি (uptime) {
				xasprintf(&status, " started %s", uptime);
				মুক্ত(uptime);
			} অন্যথা
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} অন্যথা যদি (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} অন্যথা যদি (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} অন্যথা
		xasprintf(&status, " stopped ");

	errno = 0;
	নির্বাচন (format) {
	ক্ষেত্র FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		যদি (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		বিরতি;
	ক্ষেত্র FORMAT_INI:
		printf("%s = %s\n", service, status);
		বিরতি;
	}
	মুক্ত(status);
}

স্থির শূন্য print_services_in_state(ধ্রুবক অক্ষর *runlevel, RC_STRINGLIST *svcs,
		গণনা format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ফাঁকা;
	RC_STRING *s;
	অক্ষর *r = ফাঁকা;

	যদি (!svcs)
		ফেরত;
	যদি (!deptree)
		deptree = _rc_deptree_load(0, ফাঁকা);
	যদি (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			যদি (!runlevel ||
			    rc_service_in_runlevel(s->মান, runlevel))
				print_service(s->মান, format, accept, reject);
		ফেরত;
	}
	যদি (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	যদি (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	মুক্ত(r);
	যদি (!l)
		ফেরত;
	TAILQ_FOREACH(s, l, entries) {
		যদি (!rc_stringlist_find(svcs, s->মান))
			চালু;
		যদি (!runlevel || rc_service_in_runlevel(s->মান, runlevel))
			print_service(s->মান, format, accept, reject);
	}
	rc_stringlist_free(l);
}

স্থির শূন্য print_services(ধ্রুবক অক্ষর *runlevel, RC_STRINGLIST *svcs,
	গণনা format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

স্থির শূন্য print_stacked_services(ধ্রুবক অক্ষর *runlevel, গণনা format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		যদি (rc_stringlist_find(levels, stackedlevel->মান) != ফাঁকা)
			চালু;
		print_level("Stacked", stackedlevel->মান, format);
		servicelist = rc_services_in_runlevel(stackedlevel->মান);
		print_services_in_state(stackedlevel->মান, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ফাঁকা;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ধ্রুবক rc_service_state_name_t *it;
	গণনা format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	অক্ষর *p, *runlevel = ফাঁকা;
	পূর্ণসংখ্যা opt, retval = 0;

	applet = basename_c(argv[0]);
	যখন ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (পূর্ণসংখ্যা *) 0)) != -1)
		নির্বাচন (opt) {
		ক্ষেত্র 'a':
			show_all = true;
			levels = rc_runlevel_list();
			বিরতি;
		ক্ষেত্র 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				যদি (rc_service_daemons_crashed(s->মান)) {
					printf("%s\n", s->মান);
					retval = 0;
				}
			যাও exit;
			/* NOTREACHED */
		ক্ষেত্র 'f':
			যদি (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} অন্যথা
				eerrorx("%s: invalid argument to --format switch", applet);
			বিরতি;
		ক্ষেত্র 'i':
			যদি (accept == (RC_SERVICE)-1)
				accept = 0;
			জন্য (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				যদি (it->state == RC_SERVICE_WASINACTIVE)
					চালু;
				যদি (strcmp(optarg, it->name) == 0) {
					state = it->state;
					বিরতি;
				}
			}
			যদি (it->name == ফাঁকা)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			যদি (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} অন্যথা যদি (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			যদি (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} অন্যথা যদি (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			বিরতি;
		ক্ষেত্র 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->মান);
			যাও exit;
			/* NOTREACHED */
		ক্ষেত্র 'm':
			services = rc_services_in_runlevel(ফাঁকা);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					যদি (rc_service_in_runlevel(s->মান, l->মান)) {
						TAILQ_REMOVE(services, s, entries);
						মুক্ত(s->মান);
						মুক্ত(s);
						বিরতি;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				যদি (rc_service_state(s->মান) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					মুক্ত(s->মান);
					মুক্ত(s);
				}
			print_services(ফাঁকা, services, format);
			যাও exit;
			/* NOTREACHED */
		ক্ষেত্র 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			যাও exit;
			/* NOTREACHED */
		ক্ষেত্র 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				অক্ষর *ret = rc_service_মান_get(s->মান, "child_pid");
				যদি (!ret) {
					TAILQ_REMOVE(services, s, entries);
					মুক্ত(s->মান);
					মুক্ত(s);
				}
				মুক্ত(ret);
			}
			print_services(ফাঁকা, services, format);
			যাও exit;
			/* NOTREACHED */
		ক্ষেত্র 's':
			services = rc_services_in_runlevel(ফাঁকা);
			print_services(ফাঁকা, services, format);
			যাও exit;
			/* NOTREACHED */
		ক্ষেত্র 'u':
			services = rc_services_in_runlevel(ফাঁকা);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					যদি (rc_service_in_runlevel(s->মান, l->মান)) {
						TAILQ_REMOVE(services, s, entries);
						মুক্ত(s->মান);
						মুক্ত(s);
						বিরতি;
					}
			}
			print_services(ফাঁকা, services, format);
			যাও exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	যদি (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	যখন (optind < argc) {
		যদি (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} অন্যথা
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	যদি (opt == 0)
		exit(EXIT_FAILURE);
	যদি (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, ফাঁকা);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(ফাঁকা, l->মান, format);
		services = rc_services_in_runlevel(l->মান);
		print_services_in_state(l->মান, services, format, accept, reject);
		print_stacked_services(l->মান, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ফাঁকা;
		rc_stringlist_free(services);
		services = ফাঁকা;
	}

	যদি (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(ফাঁকা, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ফাঁকা;

		/* Show manually started and unassigned depended services */
		যদি (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			যদি (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(ফাঁকা);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->মান);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->মান);
			যদি ((rc_stringlist_find(sservices, s->মান) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				যদি (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					মুক্ত(s->মান);
					মুক্ত(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->মান;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->মান = s->মান;
				setenv("RC_SVCNAME", l->মান, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->মান, RC_DEP_TRACE);
				যদি (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_ঢোকাও_লেজ(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->মান = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(ফাঁকা, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(ফাঁকা, services, format, accept, reject);
	}

exit:
	মুক্ত(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	ফেরত retval;
}
