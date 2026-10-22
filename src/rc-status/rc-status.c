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

ಎಣಿಕೆ format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extraopts = ಖಾಲಿ;
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ಸ್ಥಿರಾಂಕ ರಚನೆ option longopts[] = {
	{"all",         0, ಖಾಲಿ, 'a'},
	{"crashed",     0, ಖಾಲಿ, 'c'},
	{"format",     1, ಖಾಲಿ, 'f'},
	{"in-state",   1, ಖಾಲಿ, 'i'},
	{"list",        0, ಖಾಲಿ, 'l'},
	{"manual",        0, ಖಾಲಿ, 'm'},
	{"runlevel",    0, ಖಾಲಿ, 'r'},
	{"servicelist", 0, ಖಾಲಿ, 's'},
	{"supervised", 0, ಖಾಲಿ, 'S'},
	{"unused",      0, ಖಾಲಿ, 'u'},
	longopts_COMMON
};
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ longopts_help[] = {
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
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

ಸ್ಥಿರ RC_DEPTREE *deptree;
ಸ್ಥಿರ RC_STRINGLIST *types;

ಸ್ಥಿರ RC_STRINGLIST *ಮಟ್ಟಗಳು, *services, *tmp, *alist;
ಸ್ಥಿರ RC_STRINGLIST *sservices, *nservices, *needsme;

ಸ್ಥಿರ ಶೂನ್ಯ print_level(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *level,
		ಎಣಿಕೆ format_t format)
{
	ಆಯ್ಕೆ (format) {
	ಸ್ಥಿತಿ FORMAT_DEFAULT:
		ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ)
			ಬರೆ("%s ", ಪೂರ್ವಪ್ರತ್ಯಯ);
		ಬರೆ ("Runlevel: ");
		ಆದರೆ (isatty(fileno(stdout)))
			ಬರೆ("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_ಸಾಮಾನ್ಯ));
		ಇಲ್ಲದಿದ್ದರೆ
			ಬರೆ("%s\n", level);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FORMAT_INI:
		ಬರೆ("%s", "[");
		ಆದರೆ (ಪೂರ್ವಪ್ರತ್ಯಯ)
			ಬರೆ("%s ", ಪೂರ್ವಪ್ರತ್ಯಯ);
		ಬರೆ("%s]\n", level);
		ನಿಲ್ಲಿಸು;
	}
}

ಸ್ಥಿರ ಅಕ್ಷರ *get_uptime(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	RC_SERVICE state = rc_service_state(service);
	ಅಕ್ಷರ *ಆರಂಭ_ಎಣಿಕೆ;
	ಅಕ್ಷರ *ಆರಂಭ_ಸಮಯ_string;
	time_t ಆರಂಭ_ಸಮಯ;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	ಅಕ್ಷರ *uptime = ಖಾಲಿ;

	ಆದರೆ (state & RC_SERVICE_STARTED) {
		ಆರಂಭ_ಎಣಿಕೆ = rc_service_ಮೌಲ್ಯ_get(service, "start_count");
		ಆರಂಭ_ಸಮಯ_string = rc_service_ಮೌಲ್ಯ_get(service, "start_time");
		ಆದರೆ (ಆರಂಭ_ಎಣಿಕೆ && ಆರಂಭ_ಸಮಯ_string) {
			ಆರಂಭ_ಸಮಯ = ಗಮ್ಯ_ಸಮಯ_t(ಆರಂಭ_ಸಮಯ_string);
			diff_secs = (int64_t) difftime(time(ಖಾಲಿ), ಆರಂಭ_ಸಮಯ);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ಆದರೆ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						ಆರಂಭ_ಎಣಿಕೆ);
			ಇಲ್ಲದಿದ್ದರೆ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, ಆರಂಭ_ಎಣಿಕೆ);
		}
		ಬಿಡುಗಡೆ(ಆರಂಭ_ಎಣಿಕೆ);
		ಬಿಡುಗಡೆ(ಆರಂಭ_ಸಮಯ_string);
	}
	ಹಿಂತಿರುಗಿ uptime;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_service(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಎಣಿಕೆ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	ಅಕ್ಷರ *ಸ್ಥಿತಿ_2 = ಖಾಲಿ;
	ಅಕ್ಷರ *uptime = ಖಾಲಿ;
	ಅಕ್ಷರ *ಮಗು_pid = ಖಾಲಿ;
	ಅಕ್ಷರ *ಆರಂಭ_ಸಮಯ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ಆದರೆ (!(state & accept) || (state & reject))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (state & RC_SERVICE_STOPPING)
		xasprintf(&ಸ್ಥಿತಿ_2, "stopping ");
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_STARTING) {
		xasprintf(&ಸ್ಥಿತಿ_2, "starting ");
		color = ECOLOR_WARN;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&ಸ್ಥಿತಿ_2, "inactive ");
		color = ECOLOR_WARN;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_STARTED) {
		errno = 0;
		ಆದರೆ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			ಮಗು_pid = rc_service_ಮೌಲ್ಯ_get(service, "child_pid");
			ಆರಂಭ_ಸಮಯ = rc_service_ಮೌಲ್ಯ_get(service, "start_time");
			ಆದರೆ (ಆರಂಭ_ಸಮಯ && ಮಗು_pid)
				xasprintf(&ಸ್ಥಿತಿ_2, " unsupervised ");
			ಇಲ್ಲದಿದ್ದರೆ
				xasprintf(&ಸ್ಥಿತಿ_2, " crashed ");
			ಬಿಡುಗಡೆ(ಮಗು_pid);
			ಬಿಡುಗಡೆ(ಆರಂಭ_ಸಮಯ);
		} ಇಲ್ಲದಿದ್ದರೆ {
			uptime = get_uptime(service);
			ಆದರೆ (uptime) {
				xasprintf(&ಸ್ಥಿತಿ_2, " started %s", uptime);
				ಬಿಡುಗಡೆ(uptime);
			} ಇಲ್ಲದಿದ್ದರೆ
				xasprintf(&ಸ್ಥಿತಿ_2, " started ");
			color = ECOLOR_GOOD;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&ಸ್ಥಿತಿ_2, "scheduled");
		color = ECOLOR_WARN;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state & RC_SERVICE_FAILED) {
		xasprintf(&ಸ್ಥಿತಿ_2, "failed");
		color = ECOLOR_WARN;
	} ಇಲ್ಲದಿದ್ದರೆ
		xasprintf(&ಸ್ಥಿತಿ_2, " stopped ");

	errno = 0;
	ಆಯ್ಕೆ (format) {
	ಸ್ಥಿತಿ FORMAT_DEFAULT:
		ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ =  ಬರೆ(" %s", service);
		ಆದರೆ (c && *c && isatty(fileno(stdout)))
			ಬರೆ("\n");
		ebracket(ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ, color, ಸ್ಥಿತಿ_2);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FORMAT_INI:
		ಬರೆ("%s = %s\n", service, ಸ್ಥಿತಿ_2);
		ನಿಲ್ಲಿಸು;
	}
	ಬಿಡುಗಡೆ(ಸ್ಥಿತಿ_2);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_services_ಒಳ_state(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, RC_STRINGLIST *svcs,
		ಎಣಿಕೆ format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ಖಾಲಿ;
	RC_STRING *s;
	ಅಕ್ಷರ *r = ಖಾಲಿ;

	ಆದರೆ (!svcs)
		ಹಿಂತಿರುಗಿ;
	ಆದರೆ (!deptree)
		deptree = rc_deptree_ತುಂಬು_2(0, ಖಾಲಿ);
	ಆದರೆ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ಆದರೆ (!runlevel ||
			    rc_service_ಒಳ_runlevel(s->ಮೌಲ್ಯ, runlevel))
				print_service(s->ಮೌಲ್ಯ, format, accept, reject);
		ಹಿಂತಿರುಗಿ;
	}
	ಆದರೆ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_ಕೂಡಿಸು(types, "ineed");
		rc_stringlist_ಕೂಡಿಸು(types, "iuse");
		rc_stringlist_ಕೂಡಿಸು(types, "iafter");
	}
	ಆದರೆ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_ಆರಂಭ);
	ಬಿಡುಗಡೆ(r);
	ಆದರೆ (!l)
		ಹಿಂತಿರುಗಿ;
	TAILQ_FOREACH(s, l, entries) {
		ಆದರೆ (!rc_stringlist_ಕಂಡುಹಿಡಿ(svcs, s->ಮೌಲ್ಯ))
			ಮುಂದುವರಿ;
		ಆದರೆ (!runlevel || rc_service_ಒಳ_runlevel(s->ಮೌಲ್ಯ, runlevel))
			print_service(s->ಮೌಲ್ಯ, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_services(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, RC_STRINGLIST *svcs,
	ಎಣಿಕೆ format_t format)
{
	print_services_ಒಳ_state(runlevel, svcs, format, -1, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ print_stacked_services(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಎಣಿಕೆ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ಆದರೆ (rc_stringlist_ಕಂಡುಹಿಡಿ(ಮಟ್ಟಗಳು, stackedlevel->ಮೌಲ್ಯ) != ಖಾಲಿ)
			ಮುಂದುವರಿ;
		print_level("Stacked", stackedlevel->ಮೌಲ್ಯ, format);
		servicelist = rc_services_ಒಳ_runlevel(stackedlevel->ಮೌಲ್ಯ);
		print_services_ಒಳ_state(stackedlevel->ಮೌಲ್ಯ, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ಸ್ಥಿರಾಂಕ rc_service_state_name_t *it;
	ಎಣಿಕೆ format_t format = FORMAT_DEFAULT;
	bool ಮಟ್ಟಗಳು_given = false;
	bool show_all = false;
	ಅಕ್ಷರ *p, *runlevel = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ opt, retval = 0;

	applet = basename_c(argv[0]);
	ಆಗ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (ಪೂರ್ಣಾಂಕ *) 0)) != -1)
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'a':
			show_all = true;
			ಮಟ್ಟಗಳು = rc_runlevel_list();
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'c':
			services = rc_services_ಒಳ_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ಆದರೆ (rc_service_daemons_crashed(s->ಮೌಲ್ಯ)) {
					ಬರೆ("%s\n", s->ಮೌಲ್ಯ);
					retval = 0;
				}
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'f':
			ಆದರೆ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ಇಲ್ಲದಿದ್ದರೆ
				eerrorx("%s: invalid argument to --format switch", applet);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'i':
			ಆದರೆ (accept == (RC_SERVICE)-1)
				accept = 0;
			ಗಾಗಿ (it = rc_service_state_ಹೆಸರುಗಳು; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ಆದರೆ (it->state == RC_SERVICE_WASINACTIVE)
					ಮುಂದುವರಿ;
				ಆದರೆ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					ನಿಲ್ಲಿಸು;
				}
			}
			ಆದರೆ (it->name == ಖಾಲಿ)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ಆದರೆ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ಆದರೆ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'l':
			ಮಟ್ಟಗಳು = rc_runlevel_list();
			TAILQ_FOREACH(l, ಮಟ್ಟಗಳು, entries)
				ಬರೆ("%s\n", l->ಮೌಲ್ಯ);
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'm':
			services = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
			ಮಟ್ಟಗಳು = rc_runlevel_list();
			TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, services, entries, t) {
				TAILQ_FOREACH(l, ಮಟ್ಟಗಳು, entries)
					ಆದರೆ (rc_service_ಒಳ_runlevel(s->ಮೌಲ್ಯ, l->ಮೌಲ್ಯ)) {
						TAILQ_REMOVE(services, s, entries);
						ಬಿಡುಗಡೆ(s->ಮೌಲ್ಯ);
						ಬಿಡುಗಡೆ(s);
						ನಿಲ್ಲಿಸು;
					}
			}
			TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, services, entries, t)
				ಆದರೆ (rc_service_state(s->ಮೌಲ್ಯ) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					ಬಿಡುಗಡೆ(s->ಮೌಲ್ಯ);
					ಬಿಡುಗಡೆ(s);
				}
			print_services(ಖಾಲಿ, services, format);
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'r':
			runlevel = rc_runlevel_get();
			ಬರೆ("%s\n", runlevel ? runlevel : "");
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'S':
			services = rc_services_ಒಳ_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, services, entries, t) {
				ಅಕ್ಷರ *ret = rc_service_ಮೌಲ್ಯ_get(s->ಮೌಲ್ಯ, "child_pid");
				ಆದರೆ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					ಬಿಡುಗಡೆ(s->ಮೌಲ್ಯ);
					ಬಿಡುಗಡೆ(s);
				}
				ಬಿಡುಗಡೆ(ret);
			}
			print_services(ಖಾಲಿ, services, format);
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 's':
			services = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
			print_services(ಖಾಲಿ, services, format);
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */
		ಸ್ಥಿತಿ 'u':
			services = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
			ಮಟ್ಟಗಳು = rc_runlevel_list();
			TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, services, entries, t) {
				TAILQ_FOREACH(l, ಮಟ್ಟಗಳು, entries)
					ಆದರೆ (rc_service_ಒಳ_runlevel(s->ಮೌಲ್ಯ, l->ಮೌಲ್ಯ)) {
						TAILQ_REMOVE(services, s, entries);
						ಬಿಡುಗಡೆ(s->ಮೌಲ್ಯ);
						ಬಿಡುಗಡೆ(s);
						ನಿಲ್ಲಿಸು;
					}
			}
			print_services(ಖಾಲಿ, services, format);
			ಹೋಗು ನಿರ್ಗಮಿಸು;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ಆದರೆ (!ಮಟ್ಟಗಳು)
		ಮಟ್ಟಗಳು = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ಆಗ (optind < argc) {
		ಆದರೆ (rc_runlevel_exists(argv[optind])) {
			ಮಟ್ಟಗಳು_given = true;
			rc_stringlist_ಕೂಡಿಸು(ಮಟ್ಟಗಳು, argv[optind++]);
			opt++;
		} ಇಲ್ಲದಿದ್ದರೆ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ಆದರೆ (opt == 0)
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
	ಆದರೆ (!TAILQ_ಮೊದಲ(ಮಟ್ಟಗಳು)) {
		runlevel = default_runlevel();
		rc_stringlist_ಕೂಡಿಸು(ಮಟ್ಟಗಳು, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = rc_deptree_ತುಂಬು_2(0, ಖಾಲಿ);

	TAILQ_FOREACH(l, ಮಟ್ಟಗಳು, entries) {
		print_level(ಖಾಲಿ, l->ಮೌಲ್ಯ, format);
		services = rc_services_ಒಳ_runlevel(l->ಮೌಲ್ಯ);
		print_services_ಒಳ_state(l->ಮೌಲ್ಯ, services, format, accept, reject);
		print_stacked_services(l->ಮೌಲ್ಯ, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ಖಾಲಿ;
		rc_stringlist_free(services);
		services = ಖಾಲಿ;
	}

	ಆದರೆ (show_all || !ಮಟ್ಟಗಳು_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_ಒಳ_state(RC_SERVICE_HOTPLUGGED);
		print_services_ಒಳ_state(ಖಾಲಿ, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ಖಾಲಿ;

		/* Show manually started and unassigned depended services */
		ಆದರೆ (show_all) {
			rc_stringlist_free(ಮಟ್ಟಗಳು);
			ಮಟ್ಟಗಳು = rc_stringlist_new();
			ಆದರೆ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_ಕೂಡಿಸು(ಮಟ್ಟಗಳು, runlevel);
		}
		rc_stringlist_ಕೂಡಿಸು(ಮಟ್ಟಗಳು, RC_LEVEL_SYSINIT);
		rc_stringlist_ಕೂಡಿಸು(ಮಟ್ಟಗಳು, RC_LEVEL_BOOT);
		services = rc_services_ಒಳ_runlevel(ಖಾಲಿ);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, ಮಟ್ಟಗಳು, entries) {
			nservices = rc_services_ಒಳ_runlevel_stacked(l->ಮೌಲ್ಯ);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, services, entries, t) {
			state = rc_service_state(s->ಮೌಲ್ಯ);
			ಆದರೆ ((rc_stringlist_ಕಂಡುಹಿಡಿ(sservices, s->ಮೌಲ್ಯ) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ಆದರೆ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					ಬಿಡುಗಡೆ(s->ಮೌಲ್ಯ);
					ಬಿಡುಗಡೆ(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_ಕೂಡಿಸು(needsme, "needsme");
		rc_stringlist_ಕೂಡಿಸು(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_ಕೂಡಿಸು(alist, "");
		p = l->ಮೌಲ್ಯ;
		TAILQ_FOREACH(level, ಮಟ್ಟಗಳು, entries) {
			TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s, services, entries, t) {
				l->ಮೌಲ್ಯ = s->ಮೌಲ್ಯ;
				setenv("RC_SVCNAME", l->ಮೌಲ್ಯ, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->ಮೌಲ್ಯ, RC_DEP_TRACE);
				ಆದರೆ (TAILQ_ಮೊದಲ(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_ಸೇರಿಸು_ಬಾಲ(nservices, s, entries);
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
		print_services_ಒಳ_state(ಖಾಲಿ, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_ಒಳ_state(ಖಾಲಿ, services, format, accept, reject);
	}

ನಿರ್ಗಮಿಸು:
	ಬಿಡುಗಡೆ(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(ಮಟ್ಟಗಳು);
	rc_deptree_free(deptree);

	ಹಿಂತಿರುಗಿ retval;
}
