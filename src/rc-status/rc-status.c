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

#ଅନ୍ତର୍ଭାଵ <getopt.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <strings.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <time.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଗଣନା format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ଅଚଳ ଅକ୍ଷର *extraopts = ଖାଲି;
ଅଚଳ ଅକ୍ଷର getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ଅଚଳ ସଂରଚନା option longopts[] = {
	{"all",         0, ଖାଲି, 'a'},
	{"crashed",     0, ଖାଲି, 'c'},
	{"format",     1, ଖାଲି, 'f'},
	{"in-state",   1, ଖାଲି, 'i'},
	{"list",        0, ଖାଲି, 'l'},
	{"manual",        0, ଖାଲି, 'm'},
	{"runlevel",    0, ଖାଲି, 'r'},
	{"servicelist", 0, ଖାଲି, 's'},
	{"supervised", 0, ଖାଲି, 'S'},
	{"unused",      0, ଖାଲି, 'u'},
	longopts_COMMON
};
ଅଚଳ ଅକ୍ଷର * ଅଚଳ longopts_help[] = {
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
ଅଚଳ ଅକ୍ଷର *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

ସ୍ଥିର RC_DEPTREE *deptree;
ସ୍ଥିର RC_STRINGLIST *types;

ସ୍ଥିର RC_STRINGLIST *levels, *services, *tmp, *alist;
ସ୍ଥିର RC_STRINGLIST *sservices, *nservices, *needsme;

ସ୍ଥିର ଶୂନ୍ୟ print_level(ଅଚଳ ଅକ୍ଷର *prefix, ଅଚଳ ଅକ୍ଷର *level,
		ଗଣନା format_t format)
{
	ବାଛ (format) {
	ସ୍ଥିତି FORMAT_DEFAULT:
		ଯଦି (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		ଯଦି (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		ଅନ୍ୟଥା
			printf("%s\n", level);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FORMAT_INI:
		printf("%s", "[");
		ଯଦି (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		ଭାଙ୍ଗ;
	}
}

ସ୍ଥିର ଅକ୍ଷର *get_uptime(ଅଚଳ ଅକ୍ଷର *service)
{
	RC_SERVICE state = rc_service_state(service);
	ଅକ୍ଷର *start_ଗଣନା;
	ଅକ୍ଷର *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	ଅକ୍ଷର *uptime = ଖାଲି;

	ଯଦି (state & RC_SERVICE_STARTED) {
		start_ଗଣନା = rc_service_ମୂଲ୍ୟ_get(service, "start_count");
		start_time_string = rc_service_ମୂଲ୍ୟ_get(service, "start_time");
		ଯଦି (start_ଗଣନା && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(ଖାଲି), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ଯଦି (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_ଗଣନା);
			ଅନ୍ୟଥା
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_ଗଣନା);
		}
		ମୁକ୍ତ(start_ଗଣନା);
		ମୁକ୍ତ(start_time_string);
	}
	ଫେରନ୍ତୁ uptime;
}

ସ୍ଥିର ଶୂନ୍ୟ print_service(ଅଚଳ ଅକ୍ଷର *service, ଗଣନା format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	ଅକ୍ଷର *status = ଖାଲି;
	ଅକ୍ଷର *uptime = ଖାଲି;
	ଅକ୍ଷର *child_pid = ଖାଲି;
	ଅକ୍ଷର *start_time = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cols;
	ଅଚଳ ଅକ୍ଷର *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ଯଦି (!(state & accept) || (state & reject))
		ଫେରନ୍ତୁ;

	ଯଦି (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_STARTED) {
		errno = 0;
		ଯଦି (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_ମୂଲ୍ୟ_get(service, "child_pid");
			start_time = rc_service_ମୂଲ୍ୟ_get(service, "start_time");
			ଯଦି (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			ଅନ୍ୟଥା
				xasprintf(&status, " crashed ");
			ମୁକ୍ତ(child_pid);
			ମୁକ୍ତ(start_time);
		} ଅନ୍ୟଥା {
			uptime = get_uptime(service);
			ଯଦି (uptime) {
				xasprintf(&status, " started %s", uptime);
				ମୁକ୍ତ(uptime);
			} ଅନ୍ୟଥା
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} ଅନ୍ୟଥା ଯଦି (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} ଅନ୍ୟଥା
		xasprintf(&status, " stopped ");

	errno = 0;
	ବାଛ (format) {
	ସ୍ଥିତି FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		ଯଦି (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି FORMAT_INI:
		printf("%s = %s\n", service, status);
		ଭାଙ୍ଗ;
	}
	ମୁକ୍ତ(status);
}

ସ୍ଥିର ଶୂନ୍ୟ print_services_in_state(ଅଚଳ ଅକ୍ଷର *runlevel, RC_STRINGLIST *svcs,
		ଗଣନା format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ଖାଲି;
	RC_STRING *s;
	ଅକ୍ଷର *r = ଖାଲି;

	ଯଦି (!svcs)
		ଫେରନ୍ତୁ;
	ଯଦି (!deptree)
		deptree = _rc_deptree_load(0, ଖାଲି);
	ଯଦି (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ଯଦି (!runlevel ||
			    rc_service_in_runlevel(s->ମୂଲ୍ୟ, runlevel))
				print_service(s->ମୂଲ୍ୟ, format, accept, reject);
		ଫେରନ୍ତୁ;
	}
	ଯଦି (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	ଯଦି (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	ମୁକ୍ତ(r);
	ଯଦି (!l)
		ଫେରନ୍ତୁ;
	TAILQ_FOREACH(s, l, entries) {
		ଯଦି (!rc_stringlist_find(svcs, s->ମୂଲ୍ୟ))
			ଜାରି;
		ଯଦି (!runlevel || rc_service_in_runlevel(s->ମୂଲ୍ୟ, runlevel))
			print_service(s->ମୂଲ୍ୟ, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ସ୍ଥିର ଶୂନ୍ୟ print_services(ଅଚଳ ଅକ୍ଷର *runlevel, RC_STRINGLIST *svcs,
	ଗଣନା format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

ସ୍ଥିର ଶୂନ୍ୟ print_stacked_services(ଅଚଳ ଅକ୍ଷର *runlevel, ଗଣନା format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ଯଦି (rc_stringlist_find(levels, stackedlevel->ମୂଲ୍ୟ) != ଖାଲି)
			ଜାରି;
		print_level("Stacked", stackedlevel->ମୂଲ୍ୟ, format);
		servicelist = rc_services_in_runlevel(stackedlevel->ମୂଲ୍ୟ);
		print_services_in_state(stackedlevel->ମୂଲ୍ୟ, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ଅଚଳ rc_service_state_name_t *it;
	ଗଣନା format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	ଅକ୍ଷର *p, *runlevel = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt, retval = 0;

	applet = basename_c(argv[0]);
	ଯେତେବେଳେ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) 0)) != -1)
		ବାଛ (opt) {
		ସ୍ଥିତି 'a':
			show_all = true;
			levels = rc_runlevel_list();
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ଯଦି (rc_service_daemons_crashed(s->ମୂଲ୍ୟ)) {
					printf("%s\n", s->ମୂଲ୍ୟ);
					retval = 0;
				}
			ଯାଅ exit;
			/* NOTREACHED */
		ସ୍ଥିତି 'f':
			ଯଦି (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ଅନ୍ୟଥା
				eerrorx("%s: invalid argument to --format switch", applet);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'i':
			ଯଦି (accept == (RC_SERVICE)-1)
				accept = 0;
			ପାଇଁ (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ଯଦି (it->state == RC_SERVICE_WASINACTIVE)
					ଜାରି;
				ଯଦି (strcmp(optarg, it->name) == 0) {
					state = it->state;
					ଭାଙ୍ଗ;
				}
			}
			ଯଦି (it->name == ଖାଲି)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ଯଦି (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ଅନ୍ୟଥା ଯଦି (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ଯଦି (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ଅନ୍ୟଥା ଯଦି (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->ମୂଲ୍ୟ);
			ଯାଅ exit;
			/* NOTREACHED */
		ସ୍ଥିତି 'm':
			services = rc_services_in_runlevel(ଖାଲି);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ଯଦି (rc_service_in_runlevel(s->ମୂଲ୍ୟ, l->ମୂଲ୍ୟ)) {
						TAILQ_REMOVE(services, s, entries);
						ମୁକ୍ତ(s->ମୂଲ୍ୟ);
						ମୁକ୍ତ(s);
						ଭାଙ୍ଗ;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				ଯଦି (rc_service_state(s->ମୂଲ୍ୟ) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					ମୁକ୍ତ(s->ମୂଲ୍ୟ);
					ମୁକ୍ତ(s);
				}
			print_services(ଖାଲି, services, format);
			ଯାଅ exit;
			/* NOTREACHED */
		ସ୍ଥିତି 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			ଯାଅ exit;
			/* NOTREACHED */
		ସ୍ଥିତି 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				ଅକ୍ଷର *ret = rc_service_ମୂଲ୍ୟ_get(s->ମୂଲ୍ୟ, "child_pid");
				ଯଦି (!ret) {
					TAILQ_REMOVE(services, s, entries);
					ମୁକ୍ତ(s->ମୂଲ୍ୟ);
					ମୁକ୍ତ(s);
				}
				ମୁକ୍ତ(ret);
			}
			print_services(ଖାଲି, services, format);
			ଯାଅ exit;
			/* NOTREACHED */
		ସ୍ଥିତି 's':
			services = rc_services_in_runlevel(ଖାଲି);
			print_services(ଖାଲି, services, format);
			ଯାଅ exit;
			/* NOTREACHED */
		ସ୍ଥିତି 'u':
			services = rc_services_in_runlevel(ଖାଲି);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ଯଦି (rc_service_in_runlevel(s->ମୂଲ୍ୟ, l->ମୂଲ୍ୟ)) {
						TAILQ_REMOVE(services, s, entries);
						ମୁକ୍ତ(s->ମୂଲ୍ୟ);
						ମୁକ୍ତ(s);
						ଭାଙ୍ଗ;
					}
			}
			print_services(ଖାଲି, services, format);
			ଯାଅ exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ଯଦି (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ଯେତେବେଳେ (optind < argc) {
		ଯଦି (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} ଅନ୍ୟଥା
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ଯଦି (opt == 0)
		exit(EXIT_FAILURE);
	ଯଦି (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, ଖାଲି);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(ଖାଲି, l->ମୂଲ୍ୟ, format);
		services = rc_services_in_runlevel(l->ମୂଲ୍ୟ);
		print_services_in_state(l->ମୂଲ୍ୟ, services, format, accept, reject);
		print_stacked_services(l->ମୂଲ୍ୟ, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ଖାଲି;
		rc_stringlist_free(services);
		services = ଖାଲି;
	}

	ଯଦି (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(ଖାଲି, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ଖାଲି;

		/* Show manually started and unassigned depended services */
		ଯଦି (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			ଯଦି (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(ଖାଲି);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->ମୂଲ୍ୟ);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->ମୂଲ୍ୟ);
			ଯଦି ((rc_stringlist_find(sservices, s->ମୂଲ୍ୟ) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ଯଦି (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					ମୁକ୍ତ(s->ମୂଲ୍ୟ);
					ମୁକ୍ତ(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->ମୂଲ୍ୟ;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->ମୂଲ୍ୟ = s->ମୂଲ୍ୟ;
				setenv("RC_SVCNAME", l->ମୂଲ୍ୟ, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->ମୂଲ୍ୟ, RC_DEP_TRACE);
				ଯଦି (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_ଭର୍ତ୍ତି_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->ମୂଲ୍ୟ = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(ଖାଲି, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(ଖାଲି, services, format, accept, reject);
	}

exit:
	ମୁକ୍ତ(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	ଫେରନ୍ତୁ retval;
}
