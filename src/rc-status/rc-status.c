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

ধ্রুব বর্ণ *applet = রিক্ত;
ধ্রুব বর্ণ *extraopts = রিক্ত;
ধ্রুব বর্ণ getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ধ্রুব সংরচনা option longopts[] = {
	{"all",         0, রিক্ত, 'a'},
	{"crashed",     0, রিক্ত, 'c'},
	{"format",     1, রিক্ত, 'f'},
	{"in-state",   1, রিক্ত, 'i'},
	{"list",        0, রিক্ত, 'l'},
	{"manual",        0, রিক্ত, 'm'},
	{"runlevel",    0, রিক্ত, 'r'},
	{"servicelist", 0, রিক্ত, 's'},
	{"supervised", 0, রিক্ত, 'S'},
	{"unused",      0, রিক্ত, 'u'},
	longopts_COMMON
};
ধ্রুব বর্ণ * ধ্রুব longopts_help[] = {
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
ধ্রুব বর্ণ *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

স্থির RC_DEPTREE *deptree;
স্থির RC_STRINGLIST *types;

স্থির RC_STRINGLIST *levels, *services, *tmp, *alist;
স্থির RC_STRINGLIST *sservices, *nservices, *needsme;

স্থির শূন্য print_level(ধ্রুব বর্ণ *prefix, ধ্রুব বর্ণ *level,
		গণনা format_t format)
{
	বিকল্প (format) {
	পক্ষ FORMAT_DEFAULT:
		যদি (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		যদি (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		অন্যথা
			printf("%s\n", level);
		ভঙ্গ;
	পক্ষ FORMAT_INI:
		printf("%s", "[");
		যদি (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		ভঙ্গ;
	}
}

স্থির বর্ণ *get_uptime(ধ্রুব বর্ণ *service)
{
	RC_SERVICE state = rc_service_state(service);
	বর্ণ *start_গুনতি;
	বর্ণ *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	বর্ণ *uptime = রিক্ত;

	যদি (state & RC_SERVICE_STARTED) {
		start_গুনতি = rc_service_মান_get(service, "start_count");
		start_time_string = rc_service_মান_get(service, "start_time");
		যদি (start_গুনতি && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(রিক্ত), start_time);
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
	প্রত্যাগম uptime;
}

স্থির শূন্য print_service(ধ্রুব বর্ণ *service, গণনা format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	বর্ণ *status = রিক্ত;
	বর্ণ *uptime = রিক্ত;
	বর্ণ *child_pid = রিক্ত;
	বর্ণ *start_time = রিক্ত;
	পূর্ণাঙ্ক cols;
	ধ্রুব বর্ণ *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	যদি (!(state & accept) || (state & reject))
		প্রত্যাগম;

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
	বিকল্প (format) {
	পক্ষ FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		যদি (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		ভঙ্গ;
	পক্ষ FORMAT_INI:
		printf("%s = %s\n", service, status);
		ভঙ্গ;
	}
	মুক্ত(status);
}

স্থির শূন্য print_services_in_state(ধ্রুব বর্ণ *runlevel, RC_STRINGLIST *svcs,
		গণনা format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = রিক্ত;
	RC_STRING *s;
	বর্ণ *r = রিক্ত;

	যদি (!svcs)
		প্রত্যাগম;
	যদি (!deptree)
		deptree = _rc_deptree_load(0, রিক্ত);
	যদি (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			যদি (!runlevel ||
			    rc_service_in_runlevel(s->মান, runlevel))
				print_service(s->মান, format, accept, reject);
		প্রত্যাগম;
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
		প্রত্যাগম;
	TAILQ_FOREACH(s, l, entries) {
		যদি (!rc_stringlist_find(svcs, s->মান))
			অনুবর্ত;
		যদি (!runlevel || rc_service_in_runlevel(s->মান, runlevel))
			print_service(s->মান, format, accept, reject);
	}
	rc_stringlist_free(l);
}

স্থির শূন্য print_services(ধ্রুব বর্ণ *runlevel, RC_STRINGLIST *svcs,
	গণনা format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

স্থির শূন্য print_stacked_services(ধ্রুব বর্ণ *runlevel, গণনা format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		যদি (rc_stringlist_find(levels, stackedlevel->মান) != রিক্ত)
			অনুবর্ত;
		print_level("Stacked", stackedlevel->মান, format);
		servicelist = rc_services_in_runlevel(stackedlevel->মান);
		print_services_in_state(stackedlevel->মান, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = রিক্ত;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ধ্রুব rc_service_state_name_t *it;
	গণনা format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	বর্ণ *p, *runlevel = রিক্ত;
	পূর্ণাঙ্ক opt, retval = 0;

	applet = basename_c(argv[0]);
	যাবত্ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (পূর্ণাঙ্ক *) 0)) != -1)
		বিকল্প (opt) {
		পক্ষ 'a':
			show_all = true;
			levels = rc_runlevel_list();
			ভঙ্গ;
		পক্ষ 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				যদি (rc_service_daemons_crashed(s->মান)) {
					printf("%s\n", s->মান);
					retval = 0;
				}
			গচ্ছ exit;
			/* NOTREACHED */
		পক্ষ 'f':
			যদি (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} অন্যথা
				eerrorx("%s: invalid argument to --format switch", applet);
			ভঙ্গ;
		পক্ষ 'i':
			যদি (accept == (RC_SERVICE)-1)
				accept = 0;
			প্রতি (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				যদি (it->state == RC_SERVICE_WASINACTIVE)
					অনুবর্ত;
				যদি (strcmp(optarg, it->name) == 0) {
					state = it->state;
					ভঙ্গ;
				}
			}
			যদি (it->name == রিক্ত)
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
			ভঙ্গ;
		পক্ষ 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->মান);
			গচ্ছ exit;
			/* NOTREACHED */
		পক্ষ 'm':
			services = rc_services_in_runlevel(রিক্ত);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					যদি (rc_service_in_runlevel(s->মান, l->মান)) {
						TAILQ_REMOVE(services, s, entries);
						মুক্ত(s->মান);
						মুক্ত(s);
						ভঙ্গ;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				যদি (rc_service_state(s->মান) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					মুক্ত(s->মান);
					মুক্ত(s);
				}
			print_services(রিক্ত, services, format);
			গচ্ছ exit;
			/* NOTREACHED */
		পক্ষ 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			গচ্ছ exit;
			/* NOTREACHED */
		পক্ষ 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				বর্ণ *ret = rc_service_মান_get(s->মান, "child_pid");
				যদি (!ret) {
					TAILQ_REMOVE(services, s, entries);
					মুক্ত(s->মান);
					মুক্ত(s);
				}
				মুক্ত(ret);
			}
			print_services(রিক্ত, services, format);
			গচ্ছ exit;
			/* NOTREACHED */
		পক্ষ 's':
			services = rc_services_in_runlevel(রিক্ত);
			print_services(রিক্ত, services, format);
			গচ্ছ exit;
			/* NOTREACHED */
		পক্ষ 'u':
			services = rc_services_in_runlevel(রিক্ত);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					যদি (rc_service_in_runlevel(s->মান, l->মান)) {
						TAILQ_REMOVE(services, s, entries);
						মুক্ত(s->মান);
						মুক্ত(s);
						ভঙ্গ;
					}
			}
			print_services(রিক্ত, services, format);
			গচ্ছ exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	যদি (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	যাবত্ (optind < argc) {
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
	deptree = _rc_deptree_load(0, রিক্ত);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(রিক্ত, l->মান, format);
		services = rc_services_in_runlevel(l->মান);
		print_services_in_state(l->মান, services, format, accept, reject);
		print_stacked_services(l->মান, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = রিক্ত;
		rc_stringlist_free(services);
		services = রিক্ত;
	}

	যদি (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(রিক্ত, services, format, accept, reject);
		rc_stringlist_free(services);
		services = রিক্ত;

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
		services = rc_services_in_runlevel(রিক্ত);
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
					TAILQ_ঢোকাও_TAIL(nservices, s, entries);
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
		print_services_in_state(রিক্ত, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(রিক্ত, services, format, accept, reject);
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

	প্রত্যাগম retval;
}
