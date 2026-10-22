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

#അന്തര്ഭാവ <getopt.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <time.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "_usage.h"
#അന്തര്ഭാവ "helpers.h"

എണ്ണൽ format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;
സ്ഥിരം അക്ഷരം *extraopts = ശൂന്യ;
സ്ഥിരം അക്ഷരം getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
സ്ഥിരം ഘടന option longopts[] = {
	{"all",         0, ശൂന്യ, 'a'},
	{"crashed",     0, ശൂന്യ, 'c'},
	{"format",     1, ശൂന്യ, 'f'},
	{"in-state",   1, ശൂന്യ, 'i'},
	{"list",        0, ശൂന്യ, 'l'},
	{"manual",        0, ശൂന്യ, 'm'},
	{"runlevel",    0, ശൂന്യ, 'r'},
	{"servicelist", 0, ശൂന്യ, 's'},
	{"supervised", 0, ശൂന്യ, 'S'},
	{"unused",      0, ശൂന്യ, 'u'},
	longopts_COMMON
};
സ്ഥിരം അക്ഷരം * സ്ഥിരം longopts_help[] = {
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
സ്ഥിരം അക്ഷരം *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

സ്ഥിതം RC_DEPTREE *deptree;
സ്ഥിതം RC_STRINGLIST *types;

സ്ഥിതം RC_STRINGLIST *levels, *services, *tmp, *alist;
സ്ഥിതം RC_STRINGLIST *sservices, *nservices, *needsme;

സ്ഥിതം ശൂന്യം print_level(സ്ഥിരം അക്ഷരം *prefix, സ്ഥിരം അക്ഷരം *level,
		എണ്ണൽ format_t format)
{
	തിരഞ്ഞെടുപ്പ് (format) {
	അവസ്ഥ FORMAT_DEFAULT:
		എങ്കിൽ (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		എങ്കിൽ (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		അല്ലെങ്കിൽ
			printf("%s\n", level);
		നിർത്തുക;
	അവസ്ഥ FORMAT_INI:
		printf("%s", "[");
		എങ്കിൽ (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		നിർത്തുക;
	}
}

സ്ഥിതം അക്ഷരം *get_uptime(സ്ഥിരം അക്ഷരം *service)
{
	RC_SERVICE state = rc_service_state(service);
	അക്ഷരം *start_എണ്ണം;
	അക്ഷരം *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	അക്ഷരം *uptime = ശൂന്യ;

	എങ്കിൽ (state & RC_SERVICE_STARTED) {
		start_എണ്ണം = rc_service_മൂല്യം_get(service, "start_count");
		start_time_string = rc_service_മൂല്യം_get(service, "start_time");
		എങ്കിൽ (start_എണ്ണം && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(ശൂന്യ), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			എങ്കിൽ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_എണ്ണം);
			അല്ലെങ്കിൽ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_എണ്ണം);
		}
		സ്വതന്ത്രം(start_എണ്ണം);
		സ്വതന്ത്രം(start_time_string);
	}
	തിരികെ uptime;
}

സ്ഥിതം ശൂന്യം print_service(സ്ഥിരം അക്ഷരം *service, എണ്ണൽ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	അക്ഷരം *status = ശൂന്യ;
	അക്ഷരം *uptime = ശൂന്യ;
	അക്ഷരം *child_pid = ശൂന്യ;
	അക്ഷരം *start_time = ശൂന്യ;
	പൂർണ്ണസംഖ്യ cols;
	സ്ഥിരം അക്ഷരം *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	എങ്കിൽ (!(state & accept) || (state & reject))
		തിരികെ;

	എങ്കിൽ (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_STARTED) {
		errno = 0;
		എങ്കിൽ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_മൂല്യം_get(service, "child_pid");
			start_time = rc_service_മൂല്യം_get(service, "start_time");
			എങ്കിൽ (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			അല്ലെങ്കിൽ
				xasprintf(&status, " crashed ");
			സ്വതന്ത്രം(child_pid);
			സ്വതന്ത്രം(start_time);
		} അല്ലെങ്കിൽ {
			uptime = get_uptime(service);
			എങ്കിൽ (uptime) {
				xasprintf(&status, " started %s", uptime);
				സ്വതന്ത്രം(uptime);
			} അല്ലെങ്കിൽ
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} അല്ലെങ്കിൽ എങ്കിൽ (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} അല്ലെങ്കിൽ
		xasprintf(&status, " stopped ");

	errno = 0;
	തിരഞ്ഞെടുപ്പ് (format) {
	അവസ്ഥ FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		എങ്കിൽ (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		നിർത്തുക;
	അവസ്ഥ FORMAT_INI:
		printf("%s = %s\n", service, status);
		നിർത്തുക;
	}
	സ്വതന്ത്രം(status);
}

സ്ഥിതം ശൂന്യം print_services_in_state(സ്ഥിരം അക്ഷരം *runlevel, RC_STRINGLIST *svcs,
		എണ്ണൽ format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = ശൂന്യ;
	RC_STRING *s;
	അക്ഷരം *r = ശൂന്യ;

	എങ്കിൽ (!svcs)
		തിരികെ;
	എങ്കിൽ (!deptree)
		deptree = _rc_deptree_load(0, ശൂന്യ);
	എങ്കിൽ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			എങ്കിൽ (!runlevel ||
			    rc_service_in_runlevel(s->മൂല്യം, runlevel))
				print_service(s->മൂല്യം, format, accept, reject);
		തിരികെ;
	}
	എങ്കിൽ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	എങ്കിൽ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	സ്വതന്ത്രം(r);
	എങ്കിൽ (!l)
		തിരികെ;
	TAILQ_FOREACH(s, l, entries) {
		എങ്കിൽ (!rc_stringlist_find(svcs, s->മൂല്യം))
			തുടരുക;
		എങ്കിൽ (!runlevel || rc_service_in_runlevel(s->മൂല്യം, runlevel))
			print_service(s->മൂല്യം, format, accept, reject);
	}
	rc_stringlist_free(l);
}

സ്ഥിതം ശൂന്യം print_services(സ്ഥിരം അക്ഷരം *runlevel, RC_STRINGLIST *svcs,
	എണ്ണൽ format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

സ്ഥിതം ശൂന്യം print_stacked_services(സ്ഥിരം അക്ഷരം *runlevel, എണ്ണൽ format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		എങ്കിൽ (rc_stringlist_find(levels, stackedlevel->മൂല്യം) != ശൂന്യ)
			തുടരുക;
		print_level("Stacked", stackedlevel->മൂല്യം, format);
		servicelist = rc_services_in_runlevel(stackedlevel->മൂല്യം);
		print_services_in_state(stackedlevel->മൂല്യം, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = ശൂന്യ;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	സ്ഥിരം rc_service_state_name_t *it;
	എണ്ണൽ format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	അക്ഷരം *p, *runlevel = ശൂന്യ;
	പൂർണ്ണസംഖ്യ opt, retval = 0;

	applet = basename_c(argv[0]);
	ആകുമ്പോൾ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (പൂർണ്ണസംഖ്യ *) 0)) != -1)
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'a':
			show_all = true;
			levels = rc_runlevel_list();
			നിർത്തുക;
		അവസ്ഥ 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				എങ്കിൽ (rc_service_daemons_crashed(s->മൂല്യം)) {
					printf("%s\n", s->മൂല്യം);
					retval = 0;
				}
			പോകുക exit;
			/* NOTREACHED */
		അവസ്ഥ 'f':
			എങ്കിൽ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} അല്ലെങ്കിൽ
				eerrorx("%s: invalid argument to --format switch", applet);
			നിർത്തുക;
		അവസ്ഥ 'i':
			എങ്കിൽ (accept == (RC_SERVICE)-1)
				accept = 0;
			വേണ്ടി (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				എങ്കിൽ (it->state == RC_SERVICE_WASINACTIVE)
					തുടരുക;
				എങ്കിൽ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					നിർത്തുക;
				}
			}
			എങ്കിൽ (it->name == ശൂന്യ)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			എങ്കിൽ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} അല്ലെങ്കിൽ എങ്കിൽ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			എങ്കിൽ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} അല്ലെങ്കിൽ എങ്കിൽ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			നിർത്തുക;
		അവസ്ഥ 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->മൂല്യം);
			പോകുക exit;
			/* NOTREACHED */
		അവസ്ഥ 'm':
			services = rc_services_in_runlevel(ശൂന്യ);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					എങ്കിൽ (rc_service_in_runlevel(s->മൂല്യം, l->മൂല്യം)) {
						TAILQ_REMOVE(services, s, entries);
						സ്വതന്ത്രം(s->മൂല്യം);
						സ്വതന്ത്രം(s);
						നിർത്തുക;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				എങ്കിൽ (rc_service_state(s->മൂല്യം) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					സ്വതന്ത്രം(s->മൂല്യം);
					സ്വതന്ത്രം(s);
				}
			print_services(ശൂന്യ, services, format);
			പോകുക exit;
			/* NOTREACHED */
		അവസ്ഥ 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			പോകുക exit;
			/* NOTREACHED */
		അവസ്ഥ 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				അക്ഷരം *ret = rc_service_മൂല്യം_get(s->മൂല്യം, "child_pid");
				എങ്കിൽ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					സ്വതന്ത്രം(s->മൂല്യം);
					സ്വതന്ത്രം(s);
				}
				സ്വതന്ത്രം(ret);
			}
			print_services(ശൂന്യ, services, format);
			പോകുക exit;
			/* NOTREACHED */
		അവസ്ഥ 's':
			services = rc_services_in_runlevel(ശൂന്യ);
			print_services(ശൂന്യ, services, format);
			പോകുക exit;
			/* NOTREACHED */
		അവസ്ഥ 'u':
			services = rc_services_in_runlevel(ശൂന്യ);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					എങ്കിൽ (rc_service_in_runlevel(s->മൂല്യം, l->മൂല്യം)) {
						TAILQ_REMOVE(services, s, entries);
						സ്വതന്ത്രം(s->മൂല്യം);
						സ്വതന്ത്രം(s);
						നിർത്തുക;
					}
			}
			print_services(ശൂന്യ, services, format);
			പോകുക exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	എങ്കിൽ (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ആകുമ്പോൾ (optind < argc) {
		എങ്കിൽ (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} അല്ലെങ്കിൽ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	എങ്കിൽ (opt == 0)
		exit(EXIT_FAILURE);
	എങ്കിൽ (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, ശൂന്യ);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(ശൂന്യ, l->മൂല്യം, format);
		services = rc_services_in_runlevel(l->മൂല്യം);
		print_services_in_state(l->മൂല്യം, services, format, accept, reject);
		print_stacked_services(l->മൂല്യം, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = ശൂന്യ;
		rc_stringlist_free(services);
		services = ശൂന്യ;
	}

	എങ്കിൽ (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(ശൂന്യ, services, format, accept, reject);
		rc_stringlist_free(services);
		services = ശൂന്യ;

		/* Show manually started and unassigned depended services */
		എങ്കിൽ (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			എങ്കിൽ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(ശൂന്യ);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->മൂല്യം);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->മൂല്യം);
			എങ്കിൽ ((rc_stringlist_find(sservices, s->മൂല്യം) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				എങ്കിൽ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					സ്വതന്ത്രം(s->മൂല്യം);
					സ്വതന്ത്രം(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->മൂല്യം;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->മൂല്യം = s->മൂല്യം;
				setenv("RC_SVCNAME", l->മൂല്യം, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->മൂല്യം, RC_DEP_TRACE);
				എങ്കിൽ (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_തിരുകുക_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->മൂല്യം = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(ശൂന്യ, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(ശൂന്യ, services, format, accept, reject);
	}

exit:
	സ്വതന്ത്രം(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	തിരികെ retval;
}
