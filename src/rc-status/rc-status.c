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

#అన్తర్భావ <getopt.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <time.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "queue.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "helpers.h"

గణన format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

స్థిరరాశి అక్షరం *applet = ఖాళీ;
స్థిరరాశి అక్షరం *extraopts = ఖాళీ;
స్థిరరాశి అక్షరం getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
స్థిరరాశి నిర్మాణం option longopts[] = {
	{"all",         0, ఖాళీ, 'a'},
	{"crashed",     0, ఖాళీ, 'c'},
	{"format",     1, ఖాళీ, 'f'},
	{"in-state",   1, ఖాళీ, 'i'},
	{"list",        0, ఖాళీ, 'l'},
	{"manual",        0, ఖాళీ, 'm'},
	{"runlevel",    0, ఖాళీ, 'r'},
	{"servicelist", 0, ఖాళీ, 's'},
	{"supervised", 0, ఖాళీ, 'S'},
	{"unused",      0, ఖాళీ, 'u'},
	longopts_COMMON
};
స్థిరరాశి అక్షరం * స్థిరరాశి longopts_help[] = {
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
స్థిరరాశి అక్షరం *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

స్థిర RC_DEPTREE *deptree;
స్థిర RC_STRINGLIST *types;

స్థిర RC_STRINGLIST *స్థాయిలు, *services, *tmp, *alist;
స్థిర RC_STRINGLIST *sservices, *nservices, *needsme;

స్థిర శూన్య print_level(స్థిరరాశి అక్షరం *ఉపసర్గ, స్థిరరాశి అక్షరం *level,
		గణన format_t format)
{
	ఎంపిక (format) {
	స్థితి FORMAT_DEFAULT:
		ఒకవేళ (ఉపసర్గ)
			రాయి("%s ", ఉపసర్గ);
		రాయి ("Runlevel: ");
		ఒకవేళ (isatty(fileno(stdout)))
			రాయి("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_సాధారణ));
		లేకపోతే
			రాయి("%s\n", level);
		ఆపు;
	స్థితి FORMAT_INI:
		రాయి("%s", "[");
		ఒకవేళ (ఉపసర్గ)
			రాయి("%s ", ఉపసర్గ);
		రాయి("%s]\n", level);
		ఆపు;
	}
}

స్థిర అక్షరం *get_uptime(స్థిరరాశి అక్షరం *service)
{
	RC_SERVICE state = rc_service_state(service);
	అక్షరం *ప్రారంభం_లెక్క;
	అక్షరం *ప్రారంభం_సమయం_string;
	time_t ప్రారంభం_సమయం;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	అక్షరం *uptime = ఖాళీ;

	ఒకవేళ (state & RC_SERVICE_STARTED) {
		ప్రారంభం_లెక్క = rc_service_విలువ_get(service, "start_count");
		ప్రారంభం_సమయం_string = rc_service_విలువ_get(service, "start_time");
		ఒకవేళ (ప్రారంభం_లెక్క && ప్రారంభం_సమయం_string) {
			ప్రారంభం_సమయం = గమ్యం_సమయం_t(ప్రారంభం_సమయం_string);
			diff_secs = (int64_t) difftime(time(ఖాళీ), ప్రారంభం_సమయం);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ఒకవేళ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						ప్రారంభం_లెక్క);
			లేకపోతే
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, ప్రారంభం_లెక్క);
		}
		విడుదల(ప్రారంభం_లెక్క);
		విడుదల(ప్రారంభం_సమయం_string);
	}
	తిరిగి uptime;
}

స్థిర శూన్య print_service(స్థిరరాశి అక్షరం *service, గణన format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	అక్షరం *స్థితి_2 = ఖాళీ;
	అక్షరం *uptime = ఖాళీ;
	అక్షరం *బిడ్డ_pid = ఖాళీ;
	అక్షరం *ప్రారంభం_సమయం = ఖాళీ;
	పూర్ణాంకం నిలువుల_సంఖ్య;
	స్థిరరాశి అక్షరం *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ఒకవేళ (!(state & accept) || (state & reject))
		తిరిగి;

	ఒకవేళ (state & RC_SERVICE_STOPPING)
		xasprintf(&స్థితి_2, "stopping ");
	లేకపోతే ఒకవేళ (state & RC_SERVICE_STARTING) {
		xasprintf(&స్థితి_2, "starting ");
		color = ECOLOR_WARN;
	} లేకపోతే ఒకవేళ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&స్థితి_2, "inactive ");
		color = ECOLOR_WARN;
	} లేకపోతే ఒకవేళ (state & RC_SERVICE_STARTED) {
		errno = 0;
		ఒకవేళ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			బిడ్డ_pid = rc_service_విలువ_get(service, "child_pid");
			ప్రారంభం_సమయం = rc_service_విలువ_get(service, "start_time");
			ఒకవేళ (ప్రారంభం_సమయం && బిడ్డ_pid)
				xasprintf(&స్థితి_2, " unsupervised ");
			లేకపోతే
				xasprintf(&స్థితి_2, " crashed ");
			విడుదల(బిడ్డ_pid);
			విడుదల(ప్రారంభం_సమయం);
		} లేకపోతే {
			uptime = get_uptime(service);
			ఒకవేళ (uptime) {
				xasprintf(&స్థితి_2, " started %s", uptime);
				విడుదల(uptime);
			} లేకపోతే
				xasprintf(&స్థితి_2, " started ");
			color = ECOLOR_GOOD;
		}
	} లేకపోతే ఒకవేళ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&స్థితి_2, "scheduled");
		color = ECOLOR_WARN;
	} లేకపోతే ఒకవేళ (state & RC_SERVICE_FAILED) {
		xasprintf(&స్థితి_2, "failed");
		color = ECOLOR_WARN;
	} లేకపోతే
		xasprintf(&స్థితి_2, " stopped ");

	errno = 0;
	ఎంపిక (format) {
	స్థితి FORMAT_DEFAULT:
		నిలువుల_సంఖ్య =  రాయి(" %s", service);
		ఒకవేళ (c && *c && isatty(fileno(stdout)))
			రాయి("\n");
		ebracket(నిలువుల_సంఖ్య, color, స్థితి_2);
		ఆపు;
	స్థితి FORMAT_INI:
		రాయి("%s = %s\n", service, స్థితి_2);
		ఆపు;
	}
	విడుదల(స్థితి_2);
}

స్థిర శూన్య print_services_లోపల_state(స్థిరరాశి అక్షరం *runlevel, RC_STRINGLIST *svcs,
		గణన format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ఖాళీ;
	RC_STRING *s;
	అక్షరం *r = ఖాళీ;

	ఒకవేళ (!svcs)
		తిరిగి;
	ఒకవేళ (!deptree)
		deptree = rc_deptree_ఎక్కించు_2(0, ఖాళీ);
	ఒకవేళ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ఒకవేళ (!runlevel ||
			    rc_service_లోపల_runlevel(s->విలువ, runlevel))
				print_service(s->విలువ, format, accept, reject);
		తిరిగి;
	}
	ఒకవేళ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_కూడు(types, "ineed");
		rc_stringlist_కూడు(types, "iuse");
		rc_stringlist_కూడు(types, "iafter");
	}
	ఒకవేళ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_ప్రారంభం);
	విడుదల(r);
	ఒకవేళ (!l)
		తిరిగి;
	TAILQ_FOREACH(s, l, entries) {
		ఒకవేళ (!rc_stringlist_కనుగొను(svcs, s->విలువ))
			కొనసాగు;
		ఒకవేళ (!runlevel || rc_service_లోపల_runlevel(s->విలువ, runlevel))
			print_service(s->విలువ, format, accept, reject);
	}
	rc_stringlist_free(l);
}

స్థిర శూన్య print_services(స్థిరరాశి అక్షరం *runlevel, RC_STRINGLIST *svcs,
	గణన format_t format)
{
	print_services_లోపల_state(runlevel, svcs, format, -1, 0);
}

స్థిర శూన్య print_stacked_services(స్థిరరాశి అక్షరం *runlevel, గణన format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ఒకవేళ (rc_stringlist_కనుగొను(స్థాయిలు, stackedlevel->విలువ) != ఖాళీ)
			కొనసాగు;
		print_level("Stacked", stackedlevel->విలువ, format);
		servicelist = rc_services_లోపల_runlevel(stackedlevel->విలువ);
		print_services_లోపల_state(stackedlevel->విలువ, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ఖాళీ;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	స్థిరరాశి rc_service_state_name_t *it;
	గణన format_t format = FORMAT_DEFAULT;
	bool స్థాయిలు_given = false;
	bool show_all = false;
	అక్షరం *p, *runlevel = ఖాళీ;
	పూర్ణాంకం opt, retval = 0;

	applet = basename_c(argv[0]);
	అయితే ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (పూర్ణాంకం *) 0)) != -1)
		ఎంపిక (opt) {
		స్థితి 'a':
			show_all = true;
			స్థాయిలు = rc_runlevel_list();
			ఆపు;
		స్థితి 'c':
			services = rc_services_లోపల_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ఒకవేళ (rc_service_daemons_crashed(s->విలువ)) {
					రాయి("%s\n", s->విలువ);
					retval = 0;
				}
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */
		స్థితి 'f':
			ఒకవేళ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} లేకపోతే
				eerrorx("%s: invalid argument to --format switch", applet);
			ఆపు;
		స్థితి 'i':
			ఒకవేళ (accept == (RC_SERVICE)-1)
				accept = 0;
			కొరకు (it = rc_service_state_పేర్లు; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ఒకవేళ (it->state == RC_SERVICE_WASINACTIVE)
					కొనసాగు;
				ఒకవేళ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					ఆపు;
				}
			}
			ఒకవేళ (it->name == ఖాళీ)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ఒకవేళ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} లేకపోతే ఒకవేళ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ఒకవేళ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} లేకపోతే ఒకవేళ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			ఆపు;
		స్థితి 'l':
			స్థాయిలు = rc_runlevel_list();
			TAILQ_FOREACH(l, స్థాయిలు, entries)
				రాయి("%s\n", l->విలువ);
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */
		స్థితి 'm':
			services = rc_services_లోపల_runlevel(ఖాళీ);
			స్థాయిలు = rc_runlevel_list();
			TAILQ_FOREACH_సురక్షిత(s, services, entries, t) {
				TAILQ_FOREACH(l, స్థాయిలు, entries)
					ఒకవేళ (rc_service_లోపల_runlevel(s->విలువ, l->విలువ)) {
						TAILQ_REMOVE(services, s, entries);
						విడుదల(s->విలువ);
						విడుదల(s);
						ఆపు;
					}
			}
			TAILQ_FOREACH_సురక్షిత(s, services, entries, t)
				ఒకవేళ (rc_service_state(s->విలువ) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					విడుదల(s->విలువ);
					విడుదల(s);
				}
			print_services(ఖాళీ, services, format);
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */
		స్థితి 'r':
			runlevel = rc_runlevel_get();
			రాయి("%s\n", runlevel ? runlevel : "");
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */
		స్థితి 'S':
			services = rc_services_లోపల_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_సురక్షిత(s, services, entries, t) {
				అక్షరం *ret = rc_service_విలువ_get(s->విలువ, "child_pid");
				ఒకవేళ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					విడుదల(s->విలువ);
					విడుదల(s);
				}
				విడుదల(ret);
			}
			print_services(ఖాళీ, services, format);
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */
		స్థితి 's':
			services = rc_services_లోపల_runlevel(ఖాళీ);
			print_services(ఖాళీ, services, format);
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */
		స్థితి 'u':
			services = rc_services_లోపల_runlevel(ఖాళీ);
			స్థాయిలు = rc_runlevel_list();
			TAILQ_FOREACH_సురక్షిత(s, services, entries, t) {
				TAILQ_FOREACH(l, స్థాయిలు, entries)
					ఒకవేళ (rc_service_లోపల_runlevel(s->విలువ, l->విలువ)) {
						TAILQ_REMOVE(services, s, entries);
						విడుదల(s->విలువ);
						విడుదల(s);
						ఆపు;
					}
			}
			print_services(ఖాళీ, services, format);
			వెళ్ళు నిష్క్రమించు;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ఒకవేళ (!స్థాయిలు)
		స్థాయిలు = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	అయితే (optind < argc) {
		ఒకవేళ (rc_runlevel_exists(argv[optind])) {
			స్థాయిలు_given = true;
			rc_stringlist_కూడు(స్థాయిలు, argv[optind++]);
			opt++;
		} లేకపోతే
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ఒకవేళ (opt == 0)
		నిష్క్రమించు(EXIT_FAILURE);
	ఒకవేళ (!TAILQ_మొదటి(స్థాయిలు)) {
		runlevel = default_runlevel();
		rc_stringlist_కూడు(స్థాయిలు, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = rc_deptree_ఎక్కించు_2(0, ఖాళీ);

	TAILQ_FOREACH(l, స్థాయిలు, entries) {
		print_level(ఖాళీ, l->విలువ, format);
		services = rc_services_లోపల_runlevel(l->విలువ);
		print_services_లోపల_state(l->విలువ, services, format, accept, reject);
		print_stacked_services(l->విలువ, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ఖాళీ;
		rc_stringlist_free(services);
		services = ఖాళీ;
	}

	ఒకవేళ (show_all || !స్థాయిలు_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_లోపల_state(RC_SERVICE_HOTPLUGGED);
		print_services_లోపల_state(ఖాళీ, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ఖాళీ;

		/* Show manually started and unassigned depended services */
		ఒకవేళ (show_all) {
			rc_stringlist_free(స్థాయిలు);
			స్థాయిలు = rc_stringlist_new();
			ఒకవేళ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_కూడు(స్థాయిలు, runlevel);
		}
		rc_stringlist_కూడు(స్థాయిలు, RC_LEVEL_SYSINIT);
		rc_stringlist_కూడు(స్థాయిలు, RC_LEVEL_BOOT);
		services = rc_services_లోపల_runlevel(ఖాళీ);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, స్థాయిలు, entries) {
			nservices = rc_services_లోపల_runlevel_stacked(l->విలువ);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_సురక్షిత(s, services, entries, t) {
			state = rc_service_state(s->విలువ);
			ఒకవేళ ((rc_stringlist_కనుగొను(sservices, s->విలువ) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ఒకవేళ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					విడుదల(s->విలువ);
					విడుదల(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_కూడు(needsme, "needsme");
		rc_stringlist_కూడు(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_కూడు(alist, "");
		p = l->విలువ;
		TAILQ_FOREACH(level, స్థాయిలు, entries) {
			TAILQ_FOREACH_సురక్షిత(s, services, entries, t) {
				l->విలువ = s->విలువ;
				setenv("RC_SVCNAME", l->విలువ, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->విలువ, RC_DEP_TRACE);
				ఒకవేళ (TAILQ_మొదటి(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_చొప్పించు_తోక(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->విలువ = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_లోపల_state(ఖాళీ, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_లోపల_state(ఖాళీ, services, format, accept, reject);
	}

నిష్క్రమించు:
	విడుదల(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(స్థాయిలు);
	rc_deptree_free(deptree);

	తిరిగి retval;
}
