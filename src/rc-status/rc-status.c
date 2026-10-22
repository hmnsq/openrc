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

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>

#include "einfo.h"
#include "queue.h"
#include "rc.h"
#include "misc.h"
#include "_usage.h"
#include "helpers.h"

ga_nanaa format_t {
	FORMAT_DEFAULT,
	FORMAT_INI,
};

const var_na *applet = NULL;
const var_na *extraopts = NULL;
const var_na getoptstring[] = "acf:i:lmrsSu" getoptstring_COMMON;
const sa_nracanaa option longopts[] = {
	{"all",         0, NULL, 'a'},
	{"crashed",     0, NULL, 'c'},
	{"format",     1, NULL, 'f'},
	{"in-state",   1, NULL, 'i'},
	{"list",        0, NULL, 'l'},
	{"manual",        0, NULL, 'm'},
	{"runlevel",    0, NULL, 'r'},
	{"servicelist", 0, NULL, 's'},
	{"supervised", 0, NULL, 'S'},
	{"unused",      0, NULL, 'u'},
	longopts_COMMON
};
const var_na * const longopts_help[] = {
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
const var_na *usagestring = ""
	"Usage: rc-status [-C] [-f ini] [-i state] [runlevel]\n"
	"   or: rc-status [-C] [-f ini] [-a | -m | -S | -s | -u]\n"
	"   or: rc-status [-C] [-c | -l | -r]";

static RC_DEPTREE *deptree;
static RC_STRINGLIST *types;

static RC_STRINGLIST *levels, *services, *tmp, *alist;
static RC_STRINGLIST *sservices, *nservices, *needsme;

static void print_level(const var_na *prefix, const var_na *level,
		ga_nanaa format_t format)
{
	switch (format) {
	pak_sa FORMAT_DEFAULT:
		ꦪꦼꦤ꧀ (prefix)
			printf("%s ", prefix);
		printf ("Runlevel: ");
		ꦪꦼꦤ꧀ (isatty(fileno(stdout)))
			printf("%s%s%s\n",
					ecolor(ECOLOR_HILITE), level, ecolor(ECOLOR_NORMAL));
		ꦭꦶꦪ
			printf("%s\n", level);
		bha_nga;
	pak_sa FORMAT_INI:
		printf("%s", "[");
		ꦪꦼꦤ꧀ (prefix)
			printf("%s ", prefix);
		printf("%s]\n", level);
		bha_nga;
	}
}

static var_na *get_uptime(const var_na *service)
{
	RC_SERVICE state = rc_service_state(service);
	var_na *start_count;
	var_na *start_time_string;
	time_t start_time;
	int64_t diff_days;
	int64_t diff_hours;
	int64_t diff_mins;
	int64_t diff_secs;
	var_na *uptime = NULL;

	ꦪꦼꦤ꧀ (state & RC_SERVICE_STARTED) {
		start_count = rc_service_value_get(service, "start_count");
		start_time_string = rc_service_value_get(service, "start_time");
		ꦪꦼꦤ꧀ (start_count && start_time_string) {
			start_time = to_time_t(start_time_string);
			diff_secs = (int64_t) difftime(time(NULL), start_time);
			diff_days = diff_secs / 86400;
			diff_secs = diff_secs % 86400;
			diff_hours = diff_secs / 3600;
			diff_secs = diff_secs % 3600;
			diff_mins = diff_secs / 60;
			diff_secs = diff_secs % 60;
			ꦪꦼꦤ꧀ (diff_days > 0)
				xasprintf(&uptime,
						"%"PRId64" day(s) %02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_days, diff_hours, diff_mins, diff_secs,
						start_count);
			ꦭꦶꦪ
				xasprintf(&uptime,
						"%02"PRId64":%02"PRId64":%02"PRId64" (%s)",
						diff_hours, diff_mins, diff_secs, start_count);
		}
		free(start_count);
		free(start_time_string);
	}
	ꦮꦭꦶ uptime;
}

static void print_service(const var_na *service, ga_nanaa format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	var_na *status = NULL;
	var_na *uptime = NULL;
	var_na *child_pid = NULL;
	var_na *start_time = NULL;
	puur_naa_nka cols;
	const var_na *c = ecolor(ECOLOR_GOOD);
	RC_SERVICE state = rc_service_state(service);
	ECOLOR color = ECOLOR_BAD;

	ꦪꦼꦤ꧀ (!(state & accept) || (state & reject))
		ꦮꦭꦶ;

	ꦪꦼꦤ꧀ (state & RC_SERVICE_STOPPING)
		xasprintf(&status, "stopping ");
	ꦭꦶꦪ ꦪꦼꦤ꧀ (state & RC_SERVICE_STARTING) {
		xasprintf(&status, "starting ");
		color = ECOLOR_WARN;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (state & RC_SERVICE_INACTIVE) {
		xasprintf(&status, "inactive ");
		color = ECOLOR_WARN;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (state & RC_SERVICE_STARTED) {
		errno = 0;
		ꦪꦼꦤ꧀ (rc_service_daemons_crashed(service) && errno != EACCES)
		{
			child_pid = rc_service_value_get(service, "child_pid");
			start_time = rc_service_value_get(service, "start_time");
			ꦪꦼꦤ꧀ (start_time && child_pid)
				xasprintf(&status, " unsupervised ");
			ꦭꦶꦪ
				xasprintf(&status, " crashed ");
			free(child_pid);
			free(start_time);
		} ꦭꦶꦪ {
			uptime = get_uptime(service);
			ꦪꦼꦤ꧀ (uptime) {
				xasprintf(&status, " started %s", uptime);
				free(uptime);
			} ꦭꦶꦪ
				xasprintf(&status, " started ");
			color = ECOLOR_GOOD;
		}
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (state & RC_SERVICE_SCHEDULED) {
		xasprintf(&status, "scheduled");
		color = ECOLOR_WARN;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (state & RC_SERVICE_FAILED) {
		xasprintf(&status, "failed");
		color = ECOLOR_WARN;
	} ꦭꦶꦪ
		xasprintf(&status, " stopped ");

	errno = 0;
	switch (format) {
	pak_sa FORMAT_DEFAULT:
		cols =  printf(" %s", service);
		ꦪꦼꦤ꧀ (c && *c && isatty(fileno(stdout)))
			printf("\n");
		ebracket(cols, color, status);
		bha_nga;
	pak_sa FORMAT_INI:
		printf("%s = %s\n", service, status);
		bha_nga;
	}
	free(status);
}

static void print_services_in_state(const var_na *runlevel, RC_STRINGLIST *svcs,
		ga_nanaa format_t format, RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *l = NULL;
	RC_STRING *s;
	var_na *r = NULL;

	ꦪꦼꦤ꧀ (!svcs)
		ꦮꦭꦶ;
	ꦪꦼꦤ꧀ (!deptree)
		deptree = _rc_deptree_load(0, NULL);
	ꦪꦼꦤ꧀ (!deptree) {
		TAILQ_FOREACH(s, svcs, entries)
			ꦪꦼꦤ꧀ (!runlevel ||
			    rc_service_in_runlevel(s->value, runlevel))
				print_service(s->value, format, accept, reject);
		ꦮꦭꦶ;
	}
	ꦪꦼꦤ꧀ (!types) {
		types = rc_stringlist_new();
		rc_stringlist_add(types, "ineed");
		rc_stringlist_add(types, "iuse");
		rc_stringlist_add(types, "iafter");
	}
	ꦪꦼꦤ꧀ (!runlevel)
		r = default_runlevel();
	l = rc_deptree_depends(deptree, types, svcs, r ? r : runlevel,
			       RC_DEP_STRICT | RC_DEP_TRACE | RC_DEP_START);
	free(r);
	ꦪꦼꦤ꧀ (!l)
		ꦮꦭꦶ;
	TAILQ_FOREACH(s, l, entries) {
		ꦪꦼꦤ꧀ (!rc_stringlist_find(svcs, s->value))
			continue;
		ꦪꦼꦤ꧀ (!runlevel || rc_service_in_runlevel(s->value, runlevel))
			print_service(s->value, format, accept, reject);
	}
	rc_stringlist_free(l);
}

static void print_services(const var_na *runlevel, RC_STRINGLIST *svcs,
	ga_nanaa format_t format)
{
	print_services_in_state(runlevel, svcs, format, -1, 0);
}

static void print_stacked_services(const var_na *runlevel, ga_nanaa format_t format,
	RC_SERVICE accept, RC_SERVICE reject)
{
	RC_STRINGLIST *stackedlevels, *servicelist;
	RC_STRING *stackedlevel;

	stackedlevels = rc_runlevel_stacks(runlevel);
	TAILQ_FOREACH(stackedlevel, stackedlevels, entries) {
		ꦪꦼꦤ꧀ (rc_stringlist_find(levels, stackedlevel->value) != NULL)
			continue;
		print_level("Stacked", stackedlevel->value, format);
		servicelist = rc_services_in_runlevel(stackedlevel->value);
		print_services_in_state(stackedlevel->value, servicelist,
			format, accept, reject);
		rc_stringlist_free(servicelist);
	}
	rc_stringlist_free(stackedlevels);
	stackedlevels = NULL;
}

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	RC_SERVICE state;
	RC_SERVICE accept = -1;
	RC_SERVICE reject = 0;
	RC_STRING *s, *l, *t, *level;
	const rc_service_state_name_t *it;
	ga_nanaa format_t format = FORMAT_DEFAULT;
	bool levels_given = false;
	bool show_all = false;
	var_na *p, *runlevel = NULL;
	puur_naa_nka opt, retval = 0;

	applet = basename_c(argv[0]);
	ꦱꦼꦢꦼꦭ ((opt = getopt_long(argc, argv, getoptstring, longopts,
				  (puur_naa_nka *) 0)) != -1)
		switch (opt) {
		pak_sa 'a':
			show_all = true;
			levels = rc_runlevel_list();
			bha_nga;
		pak_sa 'c':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			retval = 1;
			TAILQ_FOREACH(s, services, entries)
				ꦪꦼꦤ꧀ (rc_service_daemons_crashed(s->value)) {
					printf("%s\n", s->value);
					retval = 0;
				}
			goto exit;
			/* NOTREACHED */
		pak_sa 'f':
			ꦪꦼꦤ꧀ (strcasecmp(optarg, "ini") == 0) {
				format = FORMAT_INI;
				setenv("EINFO_QUIET", "YES", 1);
			} ꦭꦶꦪ
				eerrorx("%s: invalid argument to --format switch", applet);
			bha_nga;
		pak_sa 'i':
			ꦪꦼꦤ꧀ (accept == (RC_SERVICE)-1)
				accept = 0;
			ꦏꦁꦒꦺꦴ (it = rc_service_state_names; it->name; ++it) {
				/*
				 * print_service() doesn't print wasinactive
				 */
				ꦪꦼꦤ꧀ (it->state == RC_SERVICE_WASINACTIVE)
					continue;
				ꦪꦼꦤ꧀ (strcmp(optarg, it->name) == 0) {
					state = it->state;
					bha_nga;
				}
			}
			ꦪꦼꦤ꧀ (it->name == NULL)
				eerrorx("%s: invalid argument to --in-state"
					" switch: %s", applet, optarg);
			accept |= state;
			/*
			 * for `-i started` we don't want to show crashed
			 * services unless `-i crashed` was used beforehand
			 */
			ꦪꦼꦤ꧀ (state == RC_SERVICE_STARTED && !(accept & RC_SERVICE_CRASHED)) {
				reject |= RC_SERVICE_CRASHED;
			} ꦭꦶꦪ ꦪꦼꦤ꧀ (state == RC_SERVICE_CRASHED) {
				/* clear it in case `-i started` set it beforehand */
				reject &= ~RC_SERVICE_CRASHED;
			}
			/* similar situation here with `failed` and `stopped` */
			ꦪꦼꦤ꧀ (state == RC_SERVICE_STOPPED && !(accept & RC_SERVICE_FAILED)) {
				reject |= RC_SERVICE_FAILED;
			} ꦭꦶꦪ ꦪꦼꦤ꧀ (state == RC_SERVICE_FAILED) {
				reject &= ~RC_SERVICE_FAILED;
			}
			bha_nga;
		pak_sa 'l':
			levels = rc_runlevel_list();
			TAILQ_FOREACH(l, levels, entries)
				printf("%s\n", l->value);
			goto exit;
			/* NOTREACHED */
		pak_sa 'm':
			services = rc_services_in_runlevel(NULL);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ꦪꦼꦤ꧀ (rc_service_in_runlevel(s->value, l->value)) {
						TAILQ_REMOVE(services, s, entries);
						free(s->value);
						free(s);
						bha_nga;
					}
			}
			TAILQ_FOREACH_SAFE(s, services, entries, t)
				ꦪꦼꦤ꧀ (rc_service_state(s->value) &
					(RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)) {
					TAILQ_REMOVE(services, s, entries);
					free(s->value);
					free(s);
				}
			print_services(NULL, services, format);
			goto exit;
			/* NOTREACHED */
		pak_sa 'r':
			runlevel = rc_runlevel_get();
			printf("%s\n", runlevel ? runlevel : "");
			goto exit;
			/* NOTREACHED */
		pak_sa 'S':
			services = rc_services_in_state(RC_SERVICE_STARTED);
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				var_na *ret = rc_service_value_get(s->value, "child_pid");
				ꦪꦼꦤ꧀ (!ret) {
					TAILQ_REMOVE(services, s, entries);
					free(s->value);
					free(s);
				}
				free(ret);
			}
			print_services(NULL, services, format);
			goto exit;
			/* NOTREACHED */
		pak_sa 's':
			services = rc_services_in_runlevel(NULL);
			print_services(NULL, services, format);
			goto exit;
			/* NOTREACHED */
		pak_sa 'u':
			services = rc_services_in_runlevel(NULL);
			levels = rc_runlevel_list();
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				TAILQ_FOREACH(l, levels, entries)
					ꦪꦼꦤ꧀ (rc_service_in_runlevel(s->value, l->value)) {
						TAILQ_REMOVE(services, s, entries);
						free(s->value);
						free(s);
						bha_nga;
					}
			}
			print_services(NULL, services, format);
			goto exit;
			/* NOTREACHED */

		case_RC_COMMON_GETOPT
		}

	ꦪꦼꦤ꧀ (!levels)
		levels = rc_stringlist_new();
	opt = (optind < argc) ? 0 : 1;
	ꦱꦼꦢꦼꦭ (optind < argc) {
		ꦪꦼꦤ꧀ (rc_runlevel_exists(argv[optind])) {
			levels_given = true;
			rc_stringlist_add(levels, argv[optind++]);
			opt++;
		} ꦭꦶꦪ
			eerror("runlevel `%s' does not exist", argv[optind++]);
	}
	ꦪꦼꦤ꧀ (opt == 0)
		exit(EXIT_FAILURE);
	ꦪꦼꦤ꧀ (!TAILQ_FIRST(levels)) {
		runlevel = default_runlevel();
		rc_stringlist_add(levels, runlevel);
	}

	/* Output the services in the order in which they would start */
	deptree = _rc_deptree_load(0, NULL);

	TAILQ_FOREACH(l, levels, entries) {
		print_level(NULL, l->value, format);
		services = rc_services_in_runlevel(l->value);
		print_services_in_state(l->value, services, format, accept, reject);
		print_stacked_services(l->value, format, accept, reject);
		rc_stringlist_free(nservices);
		nservices = NULL;
		rc_stringlist_free(services);
		services = NULL;
	}

	ꦪꦼꦤ꧀ (show_all || !levels_given) {
		/* Show hotplugged services */
		print_level("Dynamic", "hotplugged", format);
		services = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
		print_services_in_state(NULL, services, format, accept, reject);
		rc_stringlist_free(services);
		services = NULL;

		/* Show manually started and unassigned depended services */
		ꦪꦼꦤ꧀ (show_all) {
			rc_stringlist_free(levels);
			levels = rc_stringlist_new();
			ꦪꦼꦤ꧀ (!runlevel)
				runlevel = default_runlevel();
			rc_stringlist_add(levels, runlevel);
		}
		rc_stringlist_add(levels, RC_LEVEL_SYSINIT);
		rc_stringlist_add(levels, RC_LEVEL_BOOT);
		services = rc_services_in_runlevel(NULL);
		sservices = rc_stringlist_new();
		TAILQ_FOREACH(l, levels, entries) {
			nservices = rc_services_in_runlevel_stacked(l->value);
			TAILQ_CONCAT(sservices, nservices, entries);
			rc_stringlist_free(nservices);
		}
		TAILQ_FOREACH_SAFE(s, services, entries, t) {
			state = rc_service_state(s->value);
			ꦪꦼꦤ꧀ ((rc_stringlist_find(sservices, s->value) ||
			    (state & ( RC_SERVICE_STOPPED | RC_SERVICE_HOTPLUGGED)))) {
				ꦪꦼꦤ꧀ (!(state & RC_SERVICE_FAILED)) {
					TAILQ_REMOVE(services, s, entries);
					free(s->value);
					free(s);
				}
			}
		}
		needsme = rc_stringlist_new();
		rc_stringlist_add(needsme, "needsme");
		rc_stringlist_add(needsme, "wantsme");
		nservices = rc_stringlist_new();
		alist = rc_stringlist_new();
		l = rc_stringlist_add(alist, "");
		p = l->value;
		TAILQ_FOREACH(level, levels, entries) {
			TAILQ_FOREACH_SAFE(s, services, entries, t) {
				l->value = s->value;
				setenv("RC_SVCNAME", l->value, 1);
				tmp = rc_deptree_depends(deptree, needsme, alist, level->value, RC_DEP_TRACE);
				ꦪꦼꦤ꧀ (TAILQ_FIRST(tmp)) {
					TAILQ_REMOVE(services, s, entries);
					TAILQ_INSERT_TAIL(nservices, s, entries);
				}
				rc_stringlist_free(tmp);
			}
		}
		l->value = p;
		/*
		 * we are unsetting RC_SVCNAME because last loaded service will not
		 * be added to the list
		 */
		unsetenv("RC_SVCNAME");

		print_level("Dynamic", "needed/wanted", format);
		print_services_in_state(NULL, nservices, format, accept, reject);
		print_level("Dynamic", "manual", format);
		print_services_in_state(NULL, services, format, accept, reject);
	}

exit:
	free(runlevel);
	rc_stringlist_free(alist);
	rc_stringlist_free(needsme);
	rc_stringlist_free(sservices);
	rc_stringlist_free(nservices);
	rc_stringlist_free(services);
	rc_stringlist_free(types);
	rc_stringlist_free(levels);
	rc_deptree_free(deptree);

	ꦮꦭꦶ retval;
}
