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

#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਗਿਣਤੀ format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ *extraopts = ਨਿਰਾ;
ਅਚਲ ਅੱਖਰ getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
ਅਚਲ ਬਣਤਰ option longopts[] = {
	{"all",         0, ਨਿਰਾ, 'a'},
	{"crashed",     0, ਨਿਰਾ, 'c'},
	{"format",     1, ਨਿਰਾ, 'f'},
	{"in-state",   1, ਨਿਰਾ, 'i'},
	{"list",        0, ਨਿਰਾ, 'l'},
	{"manual",        0, ਨਿਰਾ, 'm'},
	{"runlevel",    0, ਨਿਰਾ, 'r'},
	{"servicelist", 0, ਨਿਰਾ, 's'},
	{"supervised", 0, ਨਿਰਾ, 'S'},
	{"unused",      0, ਨਿਰਾ, 'u'},
	longopts_COMMON
};
ਅਚਲ ਅੱਖਰ * ਅਚਲ longopts_help[] = {
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
ਅਚਲ ਅੱਖਰ *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

ਸਥਿਰ RC_DEPTREE *deptree;
ਸਥਿਰ RC_STRINGLIST *types;

ਸਥਿਰ RC_STRINGLIST *ਪੱਧਰ, *services, *tmp, *alist;
ਸਥਿਰ RC_STRINGLIST *sservices, *nservices, *needsme;

ਸਥਿਰ ਖਾਲੀ print_level(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਅਚਲ ਅੱਖਰ *level,
		ਗਿਣਤੀ format_t format)
{
	ਚੋਣ (format) {
	ਹਾਲਤ FORMAT_DEFAULT:
		ਜੇ (ਅਗੇਤਰ)
			ਲਿਖੋ("%s ", ਅਗੇਤਰ);
		ਲਿਖੋ ("Runlevel: ");
		ਜੇ (isatty(fileno(stdout)))
			ਲਿਖੋ("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_ਸਾਧਾਰਨ));
		ਨਹੀਂਤਾਂ
			ਲਿਖੋ("%s\n", level);
		ਰੋਕੋ;
	ਹਾਲਤ FORMAT_INI:
		ਲਿਖੋ("%s", "[");
		ਜੇ (ਅਗੇਤਰ)
			ਲਿਖੋ("%s ", ਅਗੇਤਰ);
		ਲਿਖੋ("%s]\n", level);
		ਰੋਕੋ;
	}
}

ਸਥਿਰ ਅੱਖਰ *get_uptime(ਅਚਲ ਅੱਖਰ *service)
{
	RC_SERVICE state = rc_service_state(service);
	ਅੱਖਰ *ਸ਼ੁਰੂ_ਗਿਣਤੀ;
	ਅੱਖਰ *ਸ਼ੁਰੂ_ਸਮਾਂ_string;
	time_t ਸ਼ੁਰੂ_ਸਮਾਂ;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	ਅੱਖਰ *uptime = ਨਿਰਾ;

	ਜੇ (state & RC_SERVICE_STARTED) {
		ਸ਼ੁਰੂ_ਗਿਣਤੀ = rc_service_ਮੁੱਲ_get(service, "start_count");
		ਸ਼ੁਰੂ_ਸਮਾਂ_string = rc_service_ਮੁੱਲ_get(service, "start_time");
		ਜੇ (ਸ਼ੁਰੂ_ਗਿਣਤੀ && ਸ਼ੁਰੂ_ਸਮਾਂ_string) {
			ਸ਼ੁਰੂ_ਸਮਾਂ = ਮੰਜ਼ਿਲ_ਸਮਾਂ_t(ਸ਼ੁਰੂ_ਸਮਾਂ_string);
			diff_secs = (int64_t) difftime(time(ਨਿਰਾ), ਸ਼ੁਰੂ_ਸਮਾਂ);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ਜੇ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						ਸ਼ੁਰੂ_ਗਿਣਤੀ);
			ਨਹੀਂਤਾਂ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, ਸ਼ੁਰੂ_ਗਿਣਤੀ);
		}
		ਮੁਕਤ(ਸ਼ੁਰੂ_ਗਿਣਤੀ);
		ਮੁਕਤ(ਸ਼ੁਰੂ_ਸਮਾਂ_string);
	}
	ਵਾਪਸ uptime;
}

ਸਥਿਰ ਖਾਲੀ print_service(ਅਚਲ ਅੱਖਰ *service, ਗਿਣਤੀ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	ਅੱਖਰ *ਸਥਿਤੀ = ਨਿਰਾ;
	ਅੱਖਰ *uptime = ਨਿਰਾ;
	ਅੱਖਰ *ਬੱਚਾ_pid = ਨਿਰਾ;
	ਅੱਖਰ *ਸ਼ੁਰੂ_ਸਮਾਂ = ਨਿਰਾ;
	ਪੂਰਨ ਖਾਨੇ_ਗਿਣਤੀ;
	ਅਚਲ ਅੱਖਰ *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ਜੇ (!(state & accept) || (state & reject))
		ਵਾਪਸ;

	ਜੇ (state & RC_SERVICE_STOPPING)
		xasprintf(&ਸਥਿਤੀ, "stopping ");
	ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_STARTING) {
		xasprintf(&ਸਥਿਤੀ, "starting ");
		color = ECOLOR_WARN;
	} ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&ਸਥਿਤੀ, "inactive ");
		color = ECOLOR_WARN;
	} ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_STARTED) {
		errno = 0;
		ਜੇ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			ਬੱਚਾ_pid = rc_service_ਮੁੱਲ_get(service, "child_pid");
			ਸ਼ੁਰੂ_ਸਮਾਂ = rc_service_ਮੁੱਲ_get(service, "start_time");
			ਜੇ (ਸ਼ੁਰੂ_ਸਮਾਂ && ਬੱਚਾ_pid)
				xasprintf(&ਸਥਿਤੀ, " unsupervised ");
			ਨਹੀਂਤਾਂ
				xasprintf(&ਸਥਿਤੀ, " crashed ");
			ਮੁਕਤ(ਬੱਚਾ_pid);
			ਮੁਕਤ(ਸ਼ੁਰੂ_ਸਮਾਂ);
		} ਨਹੀਂਤਾਂ {
			uptime = get_uptime(service);
			ਜੇ (uptime) {
				xasprintf(&ਸਥਿਤੀ, " started %s", uptime);
				ਮੁਕਤ(uptime);
			} ਨਹੀਂਤਾਂ
				xasprintf(&ਸਥਿਤੀ, " started ");
			color = ECOLOR_GOOD;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&ਸਥਿਤੀ, "scheduled");
		color = ECOLOR_WARN;
	} ਨਹੀਂਤਾਂ ਜੇ (state & RC_SERVICE_FAILED) {
		xasprintf(&ਸਥਿਤੀ, "failed");
		color = ECOLOR_WARN;
	} ਨਹੀਂਤਾਂ
		xasprintf(&ਸਥਿਤੀ, " stopped ");

	errno = 0;
	ਚੋਣ (format) {
	ਹਾਲਤ FORMAT_DEFAULT:
		ਖਾਨੇ_ਗਿਣਤੀ =  ਲਿਖੋ(" %s", service);
		ਜੇ (c && *c && isatty(fileno(stdout)))
			ਲਿਖੋ("\n");
		ebracket(ਖਾਨੇ_ਗਿਣਤੀ, color, ਸਥਿਤੀ);
		ਰੋਕੋ;
	ਹਾਲਤ FORMAT_INI:
		ਲਿਖੋ("%s = %s\n", service, ਸਥਿਤੀ);
		ਰੋਕੋ;
	}
	ਮੁਕਤ(ਸਥਿਤੀ);
}

ਸਥਿਰ ਖਾਲੀ print_services_ਵਿੱਚ_state(ਅਚਲ ਅੱਖਰ *runlevel, RC_STRINGLIST *svcs,
		ਗਿਣਤੀ format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ਨਿਰਾ;
	RC_STRING *s;
	ਅੱਖਰ *r = ਨਿਰਾ;

	ਜੇ (!svcs)
		ਵਾਪਸ;
	ਜੇ (!deptree)
		deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ);
	ਜੇ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ਜੇ (!runlevel ||
			    rc_service_ਵਿੱਚ_runlevel(s->ਮੁੱਲ, runlevel))
				print_service(s->ਮੁੱਲ, format, accept, reject);
		ਵਾਪਸ;
	}
	ਜੇ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_ਜੋੜੋ(types, "ineed");
		rc_stringlist_ਜੋੜੋ(types, "iuse");
		rc_stringlist_ਜੋੜੋ(types, "iafter");
	}
	ਜੇ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_ਸ਼ੁਰੂ);
	ਮੁਕਤ(r);
	ਜੇ (!l)
		ਵਾਪਸ;
	TAILQ_FOREACH(s, l, entries) {
		ਜੇ (!rc_stringlist_ਲੱਭੋ(svcs, s->ਮੁੱਲ))
			ਜਾਰੀ;
		ਜੇ (!runlevel || rc_service_ਵਿੱਚ_runlevel(s->ਮੁੱਲ, runlevel))
			print_service(s->ਮੁੱਲ, format, accept, reject);
	}
	rc_stringlist_free(l);
}

ਸਥਿਰ ਖਾਲੀ print_services(ਅਚਲ ਅੱਖਰ *runlevel, RC_STRINGLIST *svcs,
	ਗਿਣਤੀ format_t format)
{
	print_services_ਵਿੱਚ_state(runlevel, svcs, format, -1, 0);
}

ਸਥਿਰ ਖਾਲੀ print_stacked_services(ਅਚਲ ਅੱਖਰ *runlevel, ਗਿਣਤੀ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ਜੇ (rc_stringlist_ਲੱਭੋ(ਪੱਧਰ, stackedlevel->ਮੁੱਲ) != ਨਿਰਾ)
			ਜਾਰੀ;
		print_level("Stacked", stackedlevel->ਮੁੱਲ, format);
		servicelist = rc_services_ਵਿੱਚ_runlevel(stackedlevel->ਮੁੱਲ);
		print_services_ਵਿੱਚ_state(stackedlevel->ਮੁੱਲ, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ਨਿਰਾ;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	ਅਚਲ rc_service_state_name_t *it;
	ਗਿਣਤੀ format_t format = FORMAT_DEFAULT;
	bool ਪੱਧਰ_given = false;
	bool show_all = false;
	ਅੱਖਰ *p, *runlevel = ਨਿਰਾ;
	ਪੂਰਨ opt, retval = 0;

	applet = basename_c(argv[0]);
	ਜਦੋਂ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (ਪੂਰਨ *) 0)) != -1)
		ਚੋਣ (opt) {
		ਹਾਲਤ 'a':
			show_all = true;
			ਪੱਧਰ = rc_runlevel_list();
			ਰੋਕੋ;
		ਹਾਲਤ 'c':
			services = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ਜੇ (rc_service_daemons_crashed(s->ਮੁੱਲ)) {
					ਲਿਖੋ("%s\n", s->ਮੁੱਲ);
					retval = 0;
				}
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */
		ਹਾਲਤ 'f':
			ਜੇ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ਨਹੀਂਤਾਂ
				eerrorx("%s: invalid argument to --format switch", applet);
			ਰੋਕੋ;
		ਹਾਲਤ 'i':
			ਜੇ (accept == (RC_SERVICE)-1)
				accept = 0;
			ਲਈ (it = rc_service_state_ਨਾਂ; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ਜੇ (it->state == RC_SERVICE_WASINACTIVE)
					ਜਾਰੀ;
				ਜੇ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					ਰੋਕੋ;
				}
			}
			ਜੇ (it->name == ਨਿਰਾ)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ਜੇ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ਨਹੀਂਤਾਂ ਜੇ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ਜੇ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ਨਹੀਂਤਾਂ ਜੇ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			ਰੋਕੋ;
		ਹਾਲਤ 'l':
			ਪੱਧਰ = rc_runlevel_list();
			TAILQ_FOREACH(l, ਪੱਧਰ, entries)
				ਲਿਖੋ("%s\n", l->ਮੁੱਲ);
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */
		ਹਾਲਤ 'm':
			services = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
			ਪੱਧਰ = rc_runlevel_list();
			TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, services, entries, t) {
				TAILQ_FOREACH(l, ਪੱਧਰ, entries)
					ਜੇ (rc_service_ਵਿੱਚ_runlevel(s->ਮੁੱਲ, l->ਮੁੱਲ)) {
						TAILQ_REMOVE(services, s, entries);
						ਮੁਕਤ(s->ਮੁੱਲ);
						ਮੁਕਤ(s);
						ਰੋਕੋ;
					}
			}
			TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, services, entries, t)
				ਜੇ (rc_service_state(s->ਮੁੱਲ) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					ਮੁਕਤ(s->ਮੁੱਲ);
					ਮੁਕਤ(s);
				}
			print_services(ਨਿਰਾ, services, format);
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */
		ਹਾਲਤ 'r':
			runlevel = rc_runlevel_get();
			ਲਿਖੋ("%s\n", runlevel ? runlevel : "");
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */
		ਹਾਲਤ 'S':
			services = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, services, entries, t) {
				ਅੱਖਰ *ret = rc_service_ਮੁੱਲ_get(s->ਮੁੱਲ, "child_pid");
				ਜੇ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					ਮੁਕਤ(s->ਮੁੱਲ);
					ਮੁਕਤ(s);
				}
				ਮੁਕਤ(ret);
			}
			print_services(ਨਿਰਾ, services, format);
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */
		ਹਾਲਤ 's':
			services = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
			print_services(ਨਿਰਾ, services, format);
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */
		ਹਾਲਤ 'u':
			services = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
			ਪੱਧਰ = rc_runlevel_list();
			TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, services, entries, t) {
				TAILQ_FOREACH(l, ਪੱਧਰ, entries)
					ਜੇ (rc_service_ਵਿੱਚ_runlevel(s->ਮੁੱਲ, l->ਮੁੱਲ)) {
						TAILQ_REMOVE(services, s, entries);
						ਮੁਕਤ(s->ਮੁੱਲ);
						ਮੁਕਤ(s);
						ਰੋਕੋ;
					}
			}
			print_services(ਨਿਰਾ, services, format);
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ਜੇ (!ਪੱਧਰ)
		ਪੱਧਰ = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ਜਦੋਂ (optind < argc) {
		ਜੇ (rc_runlevel_exists(argv[optind])) {
			ਪੱਧਰ_given = true;
			rc_stringlist_ਜੋੜੋ(ਪੱਧਰ, argv[optind++]);
			opt++;
		} ਨਹੀਂਤਾਂ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ਜੇ (opt == 0)
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
	ਜੇ (!TAILQ_ਪਹਿਲਾ(ਪੱਧਰ)) {
		runlevel = default_runlevel();
		rc_stringlist_ਜੋੜੋ(ਪੱਧਰ, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = rc_deptree_ਲੱਦੋ_2(0, ਨਿਰਾ);

	TAILQ_FOREACH(l, ਪੱਧਰ, entries) {
		print_level(ਨਿਰਾ, l->ਮੁੱਲ, format);
		services = rc_services_ਵਿੱਚ_runlevel(l->ਮੁੱਲ);
		print_services_ਵਿੱਚ_state(l->ਮੁੱਲ, services, format, accept, reject);
		print_stacked_services(l->ਮੁੱਲ, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ਨਿਰਾ;
		rc_stringlist_free(services);
		services = ਨਿਰਾ;
	}

	ਜੇ (show_all || !ਪੱਧਰ_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_ਵਿੱਚ_state(RC_SERVICE_HOTPLUGGED);
		print_services_ਵਿੱਚ_state(ਨਿਰਾ, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ਨਿਰਾ;

		/* Show manually started and unassigned depended services */
		ਜੇ (show_all) {
			rc_stringlist_free(ਪੱਧਰ);
			ਪੱਧਰ = rc_stringlist_new();
			ਜੇ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_ਜੋੜੋ(ਪੱਧਰ, runlevel);
		}
		rc_stringlist_ਜੋੜੋ(ਪੱਧਰ, RC_LEVEL_SYSINIT);
		rc_stringlist_ਜੋੜੋ(ਪੱਧਰ, RC_LEVEL_BOOT);
		services = rc_services_ਵਿੱਚ_runlevel(ਨਿਰਾ);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, ਪੱਧਰ, entries) {
			nservices = rc_services_ਵਿੱਚ_runlevel_stacked(l->ਮੁੱਲ);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, services, entries, t) {
			state = rc_service_state(s->ਮੁੱਲ);
			ਜੇ ((rc_stringlist_ਲੱਭੋ(sservices, s->ਮੁੱਲ) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ਜੇ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					ਮੁਕਤ(s->ਮੁੱਲ);
					ਮੁਕਤ(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_ਜੋੜੋ(needsme, "needsme");
		rc_stringlist_ਜੋੜੋ(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_ਜੋੜੋ(alist, "");
		p = l->ਮੁੱਲ;
		TAILQ_FOREACH(level, ਪੱਧਰ, entries) {
			TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s, services, entries, t) {
				l->ਮੁੱਲ = s->ਮੁੱਲ;
				setenv("RC_SVCNAME", l->ਮੁੱਲ, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->ਮੁੱਲ, RC_DEP_TRACE);
				ਜੇ (TAILQ_ਪਹਿਲਾ(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_ਪਾਓ_ਪੂਛ(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->ਮੁੱਲ = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_ਵਿੱਚ_state(ਨਿਰਾ, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_ਵਿੱਚ_state(ਨਿਰਾ, services, format, accept, reject);
	}

ਬਾਹਰ_ਨਿਕਲੋ:
	ਮੁਕਤ(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(ਪੱਧਰ);
	rc_deptree_free(deptree);

	ਵਾਪਸ retval;
}
