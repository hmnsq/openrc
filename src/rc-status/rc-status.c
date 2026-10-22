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

#અન્તર્ભાવ <getopt.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <time.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "_usage.h"
#અન્તર્ભાવ "helpers.h"

ગણના format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

અચલ અક્ષર *applet = ખાલી;
અચલ અક્ષર *extraopts = ખાલી;
અચલ અક્ષર getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
અચલ સંરચના option longopts[] = {
	{"all",         0, ખાલી, 'a'},
	{"crashed",     0, ખાલી, 'c'},
	{"format",     1, ખાલી, 'f'},
	{"in-state",   1, ખાલી, 'i'},
	{"list",        0, ખાલી, 'l'},
	{"manual",        0, ખાલી, 'm'},
	{"runlevel",    0, ખાલી, 'r'},
	{"servicelist", 0, ખાલી, 's'},
	{"supervised", 0, ખાલી, 'S'},
	{"unused",      0, ખાલી, 'u'},
	longopts_COMMON
};
અચલ અક્ષર * અચલ longopts_help[] = {
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
અચલ અક્ષર *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

સ્થિર RC_DEPTREE *deptree;
સ્થિર RC_STRINGLIST *types;

સ્થિર RC_STRINGLIST *levels, *services, *tmp, *alist;
સ્થિર RC_STRINGLIST *sservices, *nservices, *needsme;

સ્થિર શૂન્ય print_level(અચલ અક્ષર *prefix, અચલ અક્ષર *level,
		ગણના format_t format)
{
	પસંદગી (format) {
	સ્થિતિ FORMAT_DEFAULT:
		જો (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		જો (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		નહીંતર
			printf("%s\n", level);
		તોડો;
	સ્થિતિ FORMAT_INI:
		printf("%s", "[");
		જો (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		તોડો;
	}
}

સ્થિર અક્ષર *get_uptime(અચલ અક્ષર *service)
{
	RC_SERVICE state = rc_service_state(service);
	અક્ષર *start_ગણતરી;
	અક્ષર *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	અક્ષર *uptime = ખાલી;

	જો (state & RC_SERVICE_STARTED) {
		start_ગણતરી = rc_service_કિંમત_get(service, "start_count");
		start_time_string = rc_service_કિંમત_get(service, "start_time");
		જો (start_ગણતરી && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(ખાલી), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			જો (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_ગણતરી);
			નહીંતર
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_ગણતરી);
		}
		મુક્ત(start_ગણતરી);
		મુક્ત(start_time_string);
	}
	પાછા uptime;
}

સ્થિર શૂન્ય print_service(અચલ અક્ષર *service, ગણના format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	અક્ષર *status = ખાલી;
	અક્ષર *uptime = ખાલી;
	અક્ષર *child_pid = ખાલી;
	અક્ષર *start_time = ખાલી;
	પૂર્ણાંક cols;
	અચલ અક્ષર *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	જો (!(state & accept) || (state & reject))
		પાછા;

	જો (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	નહીંતર જો (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} નહીંતર જો (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} નહીંતર જો (state & RC_SERVICE_STARTED) {
		errno = 0;
		જો (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_કિંમત_get(service, "child_pid");
			start_time = rc_service_કિંમત_get(service, "start_time");
			જો (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			નહીંતર
				xasprintf(&status, " crashed ");
			મુક્ત(child_pid);
			મુક્ત(start_time);
		} નહીંતર {
			uptime = get_uptime(service);
			જો (uptime) {
				xasprintf(&status, " started %s", uptime);
				મુક્ત(uptime);
			} નહીંતર
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} નહીંતર જો (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} નહીંતર જો (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} નહીંતર
		xasprintf(&status, " stopped ");

	errno = 0;
	પસંદગી (format) {
	સ્થિતિ FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		જો (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		તોડો;
	સ્થિતિ FORMAT_INI:
		printf("%s = %s\n", service, status);
		તોડો;
	}
	મુક્ત(status);
}

સ્થિર શૂન્ય print_services_in_state(અચલ અક્ષર *runlevel, RC_STRINGLIST *svcs,
		ગણના format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ખાલી;
	RC_STRING *s;
	અક્ષર *r = ખાલી;

	જો (!svcs)
		પાછા;
	જો (!deptree)
		deptree = _rc_deptree_load(0, ખાલી);
	જો (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			જો (!runlevel ||
			    rc_service_in_runlevel(s->કિંમત, runlevel))
				print_service(s->કિંમત, format, accept, reject);
		પાછા;
	}
	જો (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	જો (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	મુક્ત(r);
	જો (!l)
		પાછા;
	TAILQ_FOREACH(s, l, entries) {
		જો (!rc_stringlist_find(svcs, s->કિંમત))
			ચાલુ;
		જો (!runlevel || rc_service_in_runlevel(s->કિંમત, runlevel))
			print_service(s->કિંમત, format, accept, reject);
	}
	rc_stringlist_free(l);
}

સ્થિર શૂન્ય print_services(અચલ અક્ષર *runlevel, RC_STRINGLIST *svcs,
	ગણના format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

સ્થિર શૂન્ય print_stacked_services(અચલ અક્ષર *runlevel, ગણના format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		જો (rc_stringlist_find(levels, stackedlevel->કિંમત) != ખાલી)
			ચાલુ;
		print_level("Stacked", stackedlevel->કિંમત, format);
		servicelist = rc_services_in_runlevel(stackedlevel->કિંમત);
		print_services_in_state(stackedlevel->કિંમત, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ખાલી;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	અચલ rc_service_state_name_t *it;
	ગણના format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	અક્ષર *p, *runlevel = ખાલી;
	પૂર્ણાંક opt, retval = 0;

	applet = basename_c(argv[0]);
	જ્યારે ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (પૂર્ણાંક *) 0)) != -1)
		પસંદગી (opt) {
		સ્થિતિ 'a':
			show_all = true;
			levels = rc_runlevel_list();
			તોડો;
		સ્થિતિ 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				જો (rc_service_daemons_crashed(s->કિંમત)) {
					printf("%s\n", s->કિંમત);
					retval = 0;
				}
			જાઓ exit;
			/* NOTREACHED */
		સ્થિતિ 'f':
			જો (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} નહીંતર
				eerrorx("%s: invalid argument to --format switch", applet);
			તોડો;
		સ્થિતિ 'i':
			જો (accept == (RC_SERVICE)-1)
				accept = 0;
			માટે (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				જો (it->state == RC_SERVICE_WASINACTIVE)
					ચાલુ;
				જો (strcmp(optarg, it->name) == 0) {
					state = it->state;
					તોડો;
				}
			}
			જો (it->name == ખાલી)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			જો (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} નહીંતર જો (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			જો (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} નહીંતર જો (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			તોડો;
		સ્થિતિ 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->કિંમત);
			જાઓ exit;
			/* NOTREACHED */
		સ્થિતિ 'm':
			services = rc_services_in_runlevel(ખાલી);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					જો (rc_service_in_runlevel(s->કિંમત, l->કિંમત)) {
						TAILQ_REMOVE(services, s, entries);
						મુક્ત(s->કિંમત);
						મુક્ત(s);
						તોડો;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				જો (rc_service_state(s->કિંમત) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					મુક્ત(s->કિંમત);
					મુક્ત(s);
				}
			print_services(ખાલી, services, format);
			જાઓ exit;
			/* NOTREACHED */
		સ્થિતિ 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			જાઓ exit;
			/* NOTREACHED */
		સ્થિતિ 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				અક્ષર *ret = rc_service_કિંમત_get(s->કિંમત, "child_pid");
				જો (!ret) {
					TAILQ_REMOVE(services, s, entries);
					મુક્ત(s->કિંમત);
					મુક્ત(s);
				}
				મુક્ત(ret);
			}
			print_services(ખાલી, services, format);
			જાઓ exit;
			/* NOTREACHED */
		સ્થિતિ 's':
			services = rc_services_in_runlevel(ખાલી);
			print_services(ખાલી, services, format);
			જાઓ exit;
			/* NOTREACHED */
		સ્થિતિ 'u':
			services = rc_services_in_runlevel(ખાલી);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					જો (rc_service_in_runlevel(s->કિંમત, l->કિંમત)) {
						TAILQ_REMOVE(services, s, entries);
						મુક્ત(s->કિંમત);
						મુક્ત(s);
						તોડો;
					}
			}
			print_services(ખાલી, services, format);
			જાઓ exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	જો (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	જ્યારે (optind < argc) {
		જો (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} નહીંતર
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	જો (opt == 0)
		exit(EXIT_FAILURE);
	જો (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, ખાલી);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(ખાલી, l->કિંમત, format);
		services = rc_services_in_runlevel(l->કિંમત);
		print_services_in_state(l->કિંમત, services, format, accept, reject);
		print_stacked_services(l->કિંમત, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ખાલી;
		rc_stringlist_free(services);
		services = ખાલી;
	}

	જો (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(ખાલી, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ખાલી;

		/* Show manually started and unassigned depended services */
		જો (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			જો (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(ખાલી);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->કિંમત);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->કિંમત);
			જો ((rc_stringlist_find(sservices, s->કિંમત) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				જો (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					મુક્ત(s->કિંમત);
					મુક્ત(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->કિંમત;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->કિંમત = s->કિંમત;
				setenv("RC_SVCNAME", l->કિંમત, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->કિંમત, RC_DEP_TRACE);
				જો (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_ઉમેરો_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->કિંમત = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(ખાલી, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(ખાલી, services, format, accept, reject);
	}

exit:
	મુક્ત(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	પાછા retval;
}
