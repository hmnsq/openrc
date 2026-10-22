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

#ಅನ್ತರ್ಭಾವ <getopt.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <strings.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <time.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಗಣನಾ format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ಧ್ರುವ ವರ್ಣ *applet = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ *extraopts = ರಿಕ್ತ;
ಧ್ರುವ ವರ್ಣ getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	{"all",         0, ರಿಕ್ತ, 'a'},
	{"crashed",     0, ರಿಕ್ತ, 'c'},
	{"format",     1, ರಿಕ್ತ, 'f'},
	{"in-state",   1, ರಿಕ್ತ, 'i'},
	{"list",        0, ರಿಕ್ತ, 'l'},
	{"manual",        0, ರಿಕ್ತ, 'm'},
	{"runlevel",    0, ರಿಕ್ತ, 'r'},
	{"servicelist", 0, ರಿಕ್ತ, 's'},
	{"supervised", 0, ರಿಕ್ತ, 'S'},
	{"unused",      0, ರಿಕ್ತ, 'u'},
	longopts_COMMON
};
ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ longopts_help[] = {
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
ಧ್ರುವ ವರ್ಣ *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

ಸ್ಥಿರ RC_DEPTREE *deptree;
ಸ್ಥಿರ RC_STRINGLIST *types;

ಸ್ಥಿರ RC_STRINGLIST *levels, *services, *tmp, *alist;
ಸ್ಥಿರ RC_STRINGLIST *sservices, *nservices, *needsme;

ಸ್ಥಿರ ಶೂನ್ಯ print_level(ಧ್ರುವ ವರ್ಣ *prefix, ಧ್ರುವ ವರ್ಣ *level,
		ಗಣನಾ format_t format)
{
	ವಿಕಲ್ಪ (format) {
	ಪಕ್ಷ FORMAT_DEFAULT:
		ಯದಿ (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		ಯದಿ (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		ಅನ್ಯಥಾ
			printf("%s\n", level);
		ಭಙ್ಗ;
	ಪಕ್ಷ FORMAT_INI:
		printf("%s", "[");
		ಯದಿ (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		ಭಙ್ಗ;
	}
}

ಸ್ಥಿರ ವರ್ಣ *get_uptime(ಧ್ರುವ ವರ್ಣ *service)
{
	RC_SERVICE state = rc_service_state(service);
	ವರ್ಣ *start_ಎಣಿಕೆ;
	ವರ್ಣ *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	ವರ್ಣ *uptime = ರಿಕ್ತ;

	ಯದಿ (state & RC_SERVICE_STARTED) {
		start_ಎಣಿಕೆ = rc_service_ಮೌಲ್ಯ_get(service, "start_count");
		start_time_string = rc_service_ಮೌಲ್ಯ_get(service, "start_time");
		ಯದಿ (start_ಎಣಿಕೆ && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(ರಿಕ್ತ), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ಯದಿ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_ಎಣಿಕೆ);
			ಅನ್ಯಥಾ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_ಎಣಿಕೆ);
		}
		ಮುಕ್ತ(start_ಎಣಿಕೆ);
		ಮುಕ್ತ(start_time_string);
	}
	ಪ್ರತ್ಯಾಗಮ uptime;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_service(ಧ್ರುವ ವರ್ಣ *service, ಗಣನಾ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	ವರ್ಣ *status = ರಿಕ್ತ;
	ವರ್ಣ *uptime = ರಿಕ್ತ;
	ವರ್ಣ *child_pid = ರಿಕ್ತ;
	ವರ್ಣ *start_time = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ cols;
	ಧ್ರುವ ವರ್ಣ *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ಯದಿ (!(state & accept) || (state & reject))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_STARTED) {
		errno = 0;
		ಯದಿ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_ಮೌಲ್ಯ_get(service, "child_pid");
			start_time = rc_service_ಮೌಲ್ಯ_get(service, "start_time");
			ಯದಿ (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			ಅನ್ಯಥಾ
				xasprintf(&status, " crashed ");
			ಮುಕ್ತ(child_pid);
			ಮುಕ್ತ(start_time);
		} ಅನ್ಯಥಾ {
			uptime = get_uptime(service);
			ಯದಿ (uptime) {
				xasprintf(&status, " started %s", uptime);
				ಮುಕ್ತ(uptime);
			} ಅನ್ಯಥಾ
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} ಅನ್ಯಥಾ ಯದಿ (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} ಅನ್ಯಥಾ
		xasprintf(&status, " stopped ");

	errno = 0;
	ವಿಕಲ್ಪ (format) {
	ಪಕ್ಷ FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		ಯದಿ (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		ಭಙ್ಗ;
	ಪಕ್ಷ FORMAT_INI:
		printf("%s = %s\n", service, status);
		ಭಙ್ಗ;
	}
	ಮುಕ್ತ(status);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_services_in_state(ಧ್ರುವ ವರ್ಣ *runlevel, RC_STRINGLIST *svcs,
		ಗಣನಾ format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ರಿಕ್ತ;
	RC_STRING *s;
	ವರ್ಣ *r = ರಿಕ್ತ;

	ಯದಿ (!svcs)
		ಪ್ರತ್ಯಾಗಮ;
	ಯದಿ (!deptree)
		deptree = _rc_deptree_load(0, ರಿಕ್ತ);
	ಯದಿ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ಯದಿ (!runlevel ||
			    rc_service_in_runlevel(s->ಮೌಲ್ಯ, runlevel))
				print_service(s->ಮೌಲ್ಯ, format, accept, reject);
		ಪ್ರತ್ಯಾಗಮ;
	}
	ಯದಿ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	ಯದಿ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	ಮುಕ್ತ(r);
	ಯದಿ (!l)
		ಪ್ರತ್ಯಾಗಮ;
	TAILQ_FOREACH(s, l, entries) {
		ಯದಿ (!rc_stringlist_find(svcs, s->ಮೌಲ್ಯ))
			ಅನುವರ್ತ;
		ಯದಿ (!runlevel || rc_service_in_runlevel(s->ಮೌಲ್ಯ, runlevel))
			print_service(s->ಮೌಲ್ಯ, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_services(ಧ್ರುವ ವರ್ಣ *runlevel, RC_STRINGLIST *svcs,
	ಗಣನಾ format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_stacked_services(ಧ್ರುವ ವರ್ಣ *runlevel, ಗಣನಾ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ಯದಿ (rc_stringlist_find(levels, stackedlevel->ಮೌಲ್ಯ) != ರಿಕ್ತ)
			ಅನುವರ್ತ;
		print_level("Stacked", stackedlevel->ಮೌಲ್ಯ, format);
		servicelist = rc_services_in_runlevel(stackedlevel->ಮೌಲ್ಯ);
		print_services_in_state(stackedlevel->ಮೌಲ್ಯ, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ಧ್ರುವ rc_service_state_name_t *it;
	ಗಣನಾ format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	ವರ್ಣ *p, *runlevel = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ opt, retval = 0;

	applet = basename_c(argv[0]);
	ಯಾವತ್ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (ಪೂರ್ಣಾಙ್ಕ *) 0)) != -1)
		ವಿಕಲ್ಪ (opt) {
		ಪಕ್ಷ 'a':
			show_all = true;
			levels = rc_runlevel_list();
			ಭಙ್ಗ;
		ಪಕ್ಷ 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ಯದಿ (rc_service_daemons_crashed(s->ಮೌಲ್ಯ)) {
					printf("%s\n", s->ಮೌಲ್ಯ);
					retval = 0;
				}
			ಗಚ್ಛ exit;
			/* NOTREACHED */
		ಪಕ್ಷ 'f':
			ಯದಿ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ಅನ್ಯಥಾ
				eerrorx("%s: invalid argument to --format switch", applet);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'i':
			ಯದಿ (accept == (RC_SERVICE)-1)
				accept = 0;
			ಪ್ರತಿ (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ಯದಿ (it->state == RC_SERVICE_WASINACTIVE)
					ಅನುವರ್ತ;
				ಯದಿ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					ಭಙ್ಗ;
				}
			}
			ಯದಿ (it->name == ರಿಕ್ತ)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ಯದಿ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ಅನ್ಯಥಾ ಯದಿ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ಯದಿ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ಅನ್ಯಥಾ ಯದಿ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->ಮೌಲ್ಯ);
			ಗಚ್ಛ exit;
			/* NOTREACHED */
		ಪಕ್ಷ 'm':
			services = rc_services_in_runlevel(ರಿಕ್ತ);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ಯದಿ (rc_service_in_runlevel(s->ಮೌಲ್ಯ, l->ಮೌಲ್ಯ)) {
						TAILQ_REMOVE(services, s, entries);
						ಮುಕ್ತ(s->ಮೌಲ್ಯ);
						ಮುಕ್ತ(s);
						ಭಙ್ಗ;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				ಯದಿ (rc_service_state(s->ಮೌಲ್ಯ) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					ಮುಕ್ತ(s->ಮೌಲ್ಯ);
					ಮುಕ್ತ(s);
				}
			print_services(ರಿಕ್ತ, services, format);
			ಗಚ್ಛ exit;
			/* NOTREACHED */
		ಪಕ್ಷ 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			ಗಚ್ಛ exit;
			/* NOTREACHED */
		ಪಕ್ಷ 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				ವರ್ಣ *ret = rc_service_ಮೌಲ್ಯ_get(s->ಮೌಲ್ಯ, "child_pid");
				ಯದಿ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					ಮುಕ್ತ(s->ಮೌಲ್ಯ);
					ಮುಕ್ತ(s);
				}
				ಮುಕ್ತ(ret);
			}
			print_services(ರಿಕ್ತ, services, format);
			ಗಚ್ಛ exit;
			/* NOTREACHED */
		ಪಕ್ಷ 's':
			services = rc_services_in_runlevel(ರಿಕ್ತ);
			print_services(ರಿಕ್ತ, services, format);
			ಗಚ್ಛ exit;
			/* NOTREACHED */
		ಪಕ್ಷ 'u':
			services = rc_services_in_runlevel(ರಿಕ್ತ);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ಯದಿ (rc_service_in_runlevel(s->ಮೌಲ್ಯ, l->ಮೌಲ್ಯ)) {
						TAILQ_REMOVE(services, s, entries);
						ಮುಕ್ತ(s->ಮೌಲ್ಯ);
						ಮುಕ್ತ(s);
						ಭಙ್ಗ;
					}
			}
			print_services(ರಿಕ್ತ, services, format);
			ಗಚ್ಛ exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ಯದಿ (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ಯಾವತ್ (optind < argc) {
		ಯದಿ (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} ಅನ್ಯಥಾ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ಯದಿ (opt == 0)
		exit(EXIT_FAILURE);
	ಯದಿ (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, ರಿಕ್ತ);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(ರಿಕ್ತ, l->ಮೌಲ್ಯ, format);
		services = rc_services_in_runlevel(l->ಮೌಲ್ಯ);
		print_services_in_state(l->ಮೌಲ್ಯ, services, format, accept, reject);
		print_stacked_services(l->ಮೌಲ್ಯ, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ರಿಕ್ತ;
		rc_stringlist_free(services);
		services = ರಿಕ್ತ;
	}

	ಯದಿ (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(ರಿಕ್ತ, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ರಿಕ್ತ;

		/* Show manually started and unassigned depended services */
		ಯದಿ (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			ಯದಿ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(ರಿಕ್ತ);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->ಮೌಲ್ಯ);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->ಮೌಲ್ಯ);
			ಯದಿ ((rc_stringlist_find(sservices, s->ಮೌಲ್ಯ) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ಯದಿ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					ಮುಕ್ತ(s->ಮೌಲ್ಯ);
					ಮುಕ್ತ(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->ಮೌಲ್ಯ;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->ಮೌಲ್ಯ = s->ಮೌಲ್ಯ;
				setenv("RC_SVCNAME", l->ಮೌಲ್ಯ, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->ಮೌಲ್ಯ, RC_DEP_TRACE);
				ಯದಿ (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_ಸೇರಿಸು_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->ಮೌಲ್ಯ = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(ರಿಕ್ತ, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(ರಿಕ್ತ, services, format, accept, reject);
	}

exit:
	ಮುಕ್ತ(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	ಪ್ರತ್ಯಾಗಮ retval;
}
