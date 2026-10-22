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

#அந்தர்பாவ <getopt.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <time.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "helpers.h"

எண்ணிக்கை format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

மாறிலி எழுத்து *applet = வெற்று;
மாறிலி எழுத்து *extraopts = வெற்று;
மாறிலி எழுத்து getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
மாறிலி கட்டமைப்பு option longopts[] = {
	{"all",         0, வெற்று, 'a'},
	{"crashed",     0, வெற்று, 'c'},
	{"format",     1, வெற்று, 'f'},
	{"in-state",   1, வெற்று, 'i'},
	{"list",        0, வெற்று, 'l'},
	{"manual",        0, வெற்று, 'm'},
	{"runlevel",    0, வெற்று, 'r'},
	{"servicelist", 0, வெற்று, 's'},
	{"supervised", 0, வெற்று, 'S'},
	{"unused",      0, வெற்று, 'u'},
	longopts_COMMON
};
மாறிலி எழுத்து * மாறிலி longopts_help[] = {
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
மாறிலி எழுத்து *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

நிலையான RC_DEPTREE *deptree;
நிலையான RC_STRINGLIST *types;

நிலையான RC_STRINGLIST *levels, *services, *tmp, *alist;
நிலையான RC_STRINGLIST *sservices, *nservices, *needsme;

நிலையான வெறுமை print_level(மாறிலி எழுத்து *prefix, மாறிலி எழுத்து *level,
		எண்ணிக்கை format_t format)
{
	தேர்வு (format) {
	நிலை FORMAT_DEFAULT:
		எனில் (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		எனில் (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		இல்லையெனில்
			printf("%s\n", level);
		நிறுத்து;
	நிலை FORMAT_INI:
		printf("%s", "[");
		எனில் (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		நிறுத்து;
	}
}

நிலையான எழுத்து *get_uptime(மாறிலி எழுத்து *service)
{
	RC_SERVICE state = rc_service_state(service);
	எழுத்து *start_மொத்தம்;
	எழுத்து *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	எழுத்து *uptime = வெற்று;

	எனில் (state & RC_SERVICE_STARTED) {
		start_மொத்தம் = rc_service_மதிப்பு_get(service, "start_count");
		start_time_string = rc_service_மதிப்பு_get(service, "start_time");
		எனில் (start_மொத்தம் && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(வெற்று), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			எனில் (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_மொத்தம்);
			இல்லையெனில்
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_மொத்தம்);
		}
		விடுவி(start_மொத்தம்);
		விடுவி(start_time_string);
	}
	திரும்பு uptime;
}

நிலையான வெறுமை print_service(மாறிலி எழுத்து *service, எண்ணிக்கை format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	எழுத்து *status = வெற்று;
	எழுத்து *uptime = வெற்று;
	எழுத்து *child_pid = வெற்று;
	எழுத்து *start_time = வெற்று;
	முழு cols;
	மாறிலி எழுத்து *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	எனில் (!(state & accept) || (state & reject))
		திரும்பு;

	எனில் (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	இல்லையெனில் எனில் (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} இல்லையெனில் எனில் (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} இல்லையெனில் எனில் (state & RC_SERVICE_STARTED) {
		errno = 0;
		எனில் (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_மதிப்பு_get(service, "child_pid");
			start_time = rc_service_மதிப்பு_get(service, "start_time");
			எனில் (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			இல்லையெனில்
				xasprintf(&status, " crashed ");
			விடுவி(child_pid);
			விடுவி(start_time);
		} இல்லையெனில் {
			uptime = get_uptime(service);
			எனில் (uptime) {
				xasprintf(&status, " started %s", uptime);
				விடுவி(uptime);
			} இல்லையெனில்
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} இல்லையெனில் எனில் (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} இல்லையெனில் எனில் (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} இல்லையெனில்
		xasprintf(&status, " stopped ");

	errno = 0;
	தேர்வு (format) {
	நிலை FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		எனில் (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		நிறுத்து;
	நிலை FORMAT_INI:
		printf("%s = %s\n", service, status);
		நிறுத்து;
	}
	விடுவி(status);
}

நிலையான வெறுமை print_services_in_state(மாறிலி எழுத்து *runlevel, RC_STRINGLIST *svcs,
		எண்ணிக்கை format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = வெற்று;
	RC_STRING *s;
	எழுத்து *r = வெற்று;

	எனில் (!svcs)
		திரும்பு;
	எனில் (!deptree)
		deptree = _rc_deptree_load(0, வெற்று);
	எனில் (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			எனில் (!runlevel ||
			    rc_service_in_runlevel(s->மதிப்பு, runlevel))
				print_service(s->மதிப்பு, format, accept, reject);
		திரும்பு;
	}
	எனில் (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	எனில் (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	விடுவி(r);
	எனில் (!l)
		திரும்பு;
	TAILQ_FOREACH(s, l, entries) {
		எனில் (!rc_stringlist_find(svcs, s->மதிப்பு))
			தொடர்;
		எனில் (!runlevel || rc_service_in_runlevel(s->மதிப்பு, runlevel))
			print_service(s->மதிப்பு, format, accept, reject);
	}
	rc_stringlist_free(l);
}

நிலையான வெறுமை print_services(மாறிலி எழுத்து *runlevel, RC_STRINGLIST *svcs,
	எண்ணிக்கை format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

நிலையான வெறுமை print_stacked_services(மாறிலி எழுத்து *runlevel, எண்ணிக்கை format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		எனில் (rc_stringlist_find(levels, stackedlevel->மதிப்பு) != வெற்று)
			தொடர்;
		print_level("Stacked", stackedlevel->மதிப்பு, format);
		servicelist = rc_services_in_runlevel(stackedlevel->மதிப்பு);
		print_services_in_state(stackedlevel->மதிப்பு, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = வெற்று;
}

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	மாறிலி rc_service_state_name_t *it;
	எண்ணிக்கை format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	எழுத்து *p, *runlevel = வெற்று;
	முழு opt, retval = 0;

	applet = basename_c(argv[0]);
	போது ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (முழு *) 0)) != -1)
		தேர்வு (opt) {
		நிலை 'a':
			show_all = true;
			levels = rc_runlevel_list();
			நிறுத்து;
		நிலை 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				எனில் (rc_service_daemons_crashed(s->மதிப்பு)) {
					printf("%s\n", s->மதிப்பு);
					retval = 0;
				}
			செல் exit;
			/* NOTREACHED */
		நிலை 'f':
			எனில் (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} இல்லையெனில்
				eerrorx("%s: invalid argument to --format switch", applet);
			நிறுத்து;
		நிலை 'i':
			எனில் (accept == (RC_SERVICE)-1)
				accept = 0;
			க்கு (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				எனில் (it->state == RC_SERVICE_WASINACTIVE)
					தொடர்;
				எனில் (strcmp(optarg, it->name) == 0) {
					state = it->state;
					நிறுத்து;
				}
			}
			எனில் (it->name == வெற்று)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			எனில் (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} இல்லையெனில் எனில் (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			எனில் (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} இல்லையெனில் எனில் (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			நிறுத்து;
		நிலை 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->மதிப்பு);
			செல் exit;
			/* NOTREACHED */
		நிலை 'm':
			services = rc_services_in_runlevel(வெற்று);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					எனில் (rc_service_in_runlevel(s->மதிப்பு, l->மதிப்பு)) {
						TAILQ_REMOVE(services, s, entries);
						விடுவி(s->மதிப்பு);
						விடுவி(s);
						நிறுத்து;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				எனில் (rc_service_state(s->மதிப்பு) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					விடுவி(s->மதிப்பு);
					விடுவி(s);
				}
			print_services(வெற்று, services, format);
			செல் exit;
			/* NOTREACHED */
		நிலை 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			செல் exit;
			/* NOTREACHED */
		நிலை 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				எழுத்து *ret = rc_service_மதிப்பு_get(s->மதிப்பு, "child_pid");
				எனில் (!ret) {
					TAILQ_REMOVE(services, s, entries);
					விடுவி(s->மதிப்பு);
					விடுவி(s);
				}
				விடுவி(ret);
			}
			print_services(வெற்று, services, format);
			செல் exit;
			/* NOTREACHED */
		நிலை 's':
			services = rc_services_in_runlevel(வெற்று);
			print_services(வெற்று, services, format);
			செல் exit;
			/* NOTREACHED */
		நிலை 'u':
			services = rc_services_in_runlevel(வெற்று);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					எனில் (rc_service_in_runlevel(s->மதிப்பு, l->மதிப்பு)) {
						TAILQ_REMOVE(services, s, entries);
						விடுவி(s->மதிப்பு);
						விடுவி(s);
						நிறுத்து;
					}
			}
			print_services(வெற்று, services, format);
			செல் exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	எனில் (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	போது (optind < argc) {
		எனில் (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} இல்லையெனில்
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	எனில் (opt == 0)
		exit(EXIT_FAILURE);
	எனில் (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, வெற்று);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(வெற்று, l->மதிப்பு, format);
		services = rc_services_in_runlevel(l->மதிப்பு);
		print_services_in_state(l->மதிப்பு, services, format, accept, reject);
		print_stacked_services(l->மதிப்பு, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = வெற்று;
		rc_stringlist_free(services);
		services = வெற்று;
	}

	எனில் (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(வெற்று, services, format, accept, reject);
		rc_stringlist_free(services);
		services = வெற்று;

		/* Show manually started and unassigned depended services */
		எனில் (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			எனில் (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(வெற்று);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->மதிப்பு);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->மதிப்பு);
			எனில் ((rc_stringlist_find(sservices, s->மதிப்பு) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				எனில் (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					விடுவி(s->மதிப்பு);
					விடுவி(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->மதிப்பு;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->மதிப்பு = s->மதிப்பு;
				setenv("RC_SVCNAME", l->மதிப்பு, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->மதிப்பு, RC_DEP_TRACE);
				எனில் (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_செருகு_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->மதிப்பு = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(வெற்று, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(வெற்று, services, format, accept, reject);
	}

exit:
	விடுவி(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	திரும்பு retval;
}
