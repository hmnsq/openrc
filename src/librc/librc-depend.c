/*
 *  librc-depend
 *  rc service dependency and ordering
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

#অন্তর্ভাৱ <sys/utsname.h>
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <dirent.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "queue.h"
#অন্তর্ভাৱ "librc.h"
#অন্তর্ভাৱ "helpers.h"
#অন্তর্ভাৱ "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

স্থির ধ্রুৱ ৱর্ণ *bootlevel = রিক্ত;

স্থির ৱর্ণ *
get_shell_মান(ৱর্ণ *string)
{
	ৱর্ণ *p = string;
	ৱর্ণ *e;

	যদি (!string)
		ঘূৰাই রিক্ত;

	যদি (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	যদি (*e == '\n')
		*e-- = 0;
	যদি (*e == '\'')
		*e-- = 0;

	যদি (*p != 0)
		ঘূৰাই p;

	ঘূৰাই রিক্ত;
}

শূন্য
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	যদি (!deptree)
		ঘূৰাই;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			মুক্ত(dt->type);
			মুক্ত(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		মুক্ত(di->service);
		মুক্ত(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	মুক্ত(deptree);
}

স্থির RC_DEPINFO *
get_depinfo(ধ্রুৱ RC_DEPTREE *deptree, ধ্রুৱ ৱর্ণ *service)
{
	RC_DEPINFO *di;
	যদি (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			যদি (strcmp(di->service, service) == 0)
				ঘূৰাই di;
	}
	ঘূৰাই রিক্ত;
}

স্থির RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ধ্রুৱ ৱর্ণ *service)
{
	RC_DEPINFO *depinfo = xmalloc(পরিমাণ(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_ঢোকাও_TAIL(deptree, depinfo, entries);

	ঘূৰাই depinfo;
}

স্থির RC_DEPTYPE *
get_deptype(ধ্রুৱ RC_DEPINFO *depinfo, ধ্রুৱ ৱর্ণ *type)
{
	RC_DEPTYPE *dt;

	যদি (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			যদি (strcmp(dt->type, type) == 0)
				ঘূৰাই dt;
	}
	ঘূৰাই রিক্ত;
}

স্থির RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ধ্রুৱ ৱর্ণ *type)
{
	RC_DEPTYPE *deptype = xmalloc(পরিমাণ(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_ঢোকাও_TAIL(&depinfo->depends, deptype, entries);

	ঘূৰাই deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
স্থির RC_DEPTREE *
make_deptree(শূন্য) {
	RC_DEPTREE *deptree = xmalloc(পরিমাণ(*deptree));
	TAILQ_INIT(deptree);
	ঘূৰাই deptree;
}

স্থির RC_DEPTREE *
deptree_load_file(পূর্ণাঙ্ক dirfd, ধ্রুৱ ৱর্ণ *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = রিক্ত;
	RC_DEPTYPE *deptype = রিক্ত;
	ৱর্ণ *সাৰি = রিক্ত;
	size_t size;
	ৱর্ণ *type;
	ৱর্ণ *p;
	ৱর্ণ *e;
	পূর্ণাঙ্ক i;
	FILE *fp;

	যদি (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ঘূৰাই রিক্ত;

	deptree = make_deptree();
	যেতিয়ালৈ (xgetline(&সাৰি, &size, fp) != -1) {
		p = সাৰি;
		e = strsep(&p, "_");
		যদি (!e || strcmp(e, "depinfo") != 0)
			অনুৱর্ত;
		e = strsep(&p, "_");
		যদি (!e || sscanf(e, "%d", &i) != 1)
			অনুৱর্ত;
		যদি (!(type = strsep(&p, "_=")))
			অনুৱর্ত;
		যদি (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_মান(p);
			যদি (!e || *e == '\0')
				অনুৱর্ত;
			depinfo = make_depinfo(deptree, e);
			deptype = রিক্ত;
			অনুৱর্ত;
		}
		e = strsep(&p, "=");
		যদি (!e || sscanf(e, "%d", &i) != 1)
			অনুৱর্ত;
		/* Sanity */
		e = get_shell_মান(p);
		যদি (!e || *e == '\0')
			অনুৱর্ত;
		যদি (!depinfo)
			অনুৱর্ত;
		যদি (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	মুক্ত(সাৰি);
	fclose(fp);

	ঘূৰাই deptree;
}

RC_DEPTREE *
rc_deptree_load(শূন্য)
{
	ঘূৰাই deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ধ্রুৱ ৱর্ণ *deptree_file)
{
	ঘূৰাই deptree_load_file(AT_FDCWD, deptree_file);
}

স্থির bool
valid_service(ধ্রুৱ ৱর্ণ *runlevel, ধ্রুৱ ৱর্ণ *service, ধ্রুৱ ৱর্ণ *type)
{
	RC_SERVICE state;

	যদি (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ঘূৰাই true;

	যদি (rc_service_in_runlevel(service, runlevel))
		ঘূৰাই true;
	যদি (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ঘূৰাই false;
	যদি (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ঘূৰাই false;
	যদি (strcmp(runlevel, bootlevel) != 0) {
		যদি (rc_service_in_runlevel(service, bootlevel))
			ঘূৰাই true;
	}

	state = rc_service_state(service);
	যদি (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ঘূৰাই true;

	ঘূৰাই false;
}

স্থির bool
get_provided1(ধ্রুৱ ৱর্ণ *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ধ্রুৱ ৱর্ণ *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ধ্রুৱ ৱর্ণ *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->মান;
		st = rc_service_state(svc);

		যদি (level)
			ok = rc_service_in_runlevel(svc, level);
		অন্যথা যদি (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		যদি (!ok)
			অনুৱর্ত;
		ৱিকল্প (state) {
			পক্ষ RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				ভঙ্গ;
			পক্ষ RC_SERVICE_INACTIVE:
			পক্ষ RC_SERVICE_STARTING:
			পক্ষ RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				ভঙ্গ;
			স্ৱভাৱ:
				ভঙ্গ;
		}
		যদি (!ok)
			অনুৱর্ত;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	ঘূৰাই retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
স্থির RC_STRINGLIST *
get_provided(ধ্রুৱ RC_DEPINFO *depinfo, ধ্রুৱ ৱর্ণ *runlevel, পূর্ণাঙ্ক options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	যদি (!dt)
		ঘূৰাই providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	যদি (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->মান);
		ঘূৰাই providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	যদি (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			যদি (rc_service_in_runlevel(service->মান, runlevel) ||
			    rc_service_in_runlevel(service->মান, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->মান) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->মান);
		যদি (TAILQ_FIRST(providers))
			ঘূৰাই providers;
	}

	/* OK, we're not strict or there were no services in our runlevel.
	 * This is now where the logic gets a little fuzzy :)
	 * If there is >1 running service then we return NULL.
	 * We do this so we don't hang around waiting for inactive services and
	 * our need has already been satisfied as it's not strict.
	 * We apply this to these states in order:-
	 *     started, starting | stopping | inactive, stopped
	 * Our sub preference in each of these is in order:-
	 *     runlevel, hotplugged, bootlevel, any
	 */
#define DO \
	যদি (TAILQ_FIRST(providers)) { \
		যদি (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ঘূৰাই providers; \
	}

	/* Anything running has to come first */
	যদি (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	যদি (get_provided1(runlevel, providers, dt, রিক্ত, true, RC_SERVICE_STARTED))
	{ DO }
	যদি (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	যদি (get_provided1(runlevel, providers, dt, রিক্ত, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	যদি (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ঘূৰাই providers;
	যদি (get_provided1(runlevel, providers, dt, রিক্ত, true, RC_SERVICE_STARTING))
		ঘূৰাই providers;
	যদি (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ঘূৰাই providers;
	যদি (get_provided1(runlevel, providers, dt, রিক্ত, false, RC_SERVICE_STARTING))
		ঘূৰাই providers;

	/* Nothing started then. OK, lets get the stopped services */
	যদি (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ঘূৰাই providers;
	যদি (get_provided1(runlevel, providers, dt, রিক্ত, true, RC_SERVICE_STOPPED))
	{ DO }
	যদি (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ঘূৰাই providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	যদি (service != রিক্ত)
		rc_stringlist_add(providers, service->মান);

	ঘূৰাই providers;
}

স্থির শূন্য
visit_service(ধ্রুৱ RC_DEPTREE *deptree,
	      ধ্রুৱ RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      ধ্রুৱ RC_DEPINFO *depinfo,
	      ধ্রুৱ ৱর্ণ *runlevel, পূর্ণাঙ্ক options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ধ্রুৱ ৱর্ণ *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		যদি (strcmp(type->মান, depinfo->service) == 0)
			ঘূৰাই;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		যদি (!(dt = get_deptype(depinfo, type->মান)))
			অনুৱর্ত;

		TAILQ_FOREACH(service, dt->services, entries) {
			যদি (!(options & RC_DEP_TRACE) ||
			    strcmp(type->মান, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->মান);
				অনুৱর্ত;
			}

			যদি (!(di = get_depinfo(deptree, service->মান)))
				অনুৱর্ত;
			provided = get_provided(di, runlevel, options);

			যদি (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->মান);
					যদি (di && valid_service(runlevel, di->service, type->মান))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			অন্যথা যদি (di && valid_service(runlevel, service->মান, type->মান))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	যদি (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			যদি (!(di = get_depinfo(deptree, service->মান)))
				অনুৱর্ত;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				যদি (strcmp(p->মান, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					ভঙ্গ;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	যদি (!svcname || strcmp(svcname, depinfo->service) != 0) {
		যদি (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ধ্রুৱ RC_DEPTREE *deptree,
		  ধ্রুৱ ৱর্ণ *service, ধ্রুৱ ৱর্ণ *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	যদি (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ঘূৰাই svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->মান);
	ঘূৰাই svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ধ্রুৱ RC_DEPTREE *deptree,
		   ধ্রুৱ RC_STRINGLIST *types,
		   ধ্রুৱ RC_STRINGLIST *services,
		   ধ্রুৱ ৱর্ণ *runlevel, পূর্ণাঙ্ক options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	ধ্রুৱ RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	যদি (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		যদি (!(di = get_depinfo(deptree, service->মান))) {
			errno = ENOENT;
			অনুৱর্ত;
		}
		যদি (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	ঘূৰাই sorted;
}

RC_STRINGLIST *
rc_deptree_order(ধ্রুৱ RC_DEPTREE *deptree, ধ্রুৱ ৱর্ণ *runlevel, পূর্ণাঙ্ক options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	যদি (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	যদি (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		মুক্ত(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		মুক্ত(list2);
	} অন্যথা {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		যদি (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			মুক্ত(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			মুক্ত(list2);
			/* If we're not the boot runlevel then add that too */
			যদি (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				মুক্ত(list2);
			}
		}
	}

	/* Now we have our lists, we need to pull in any dependencies
	   and order them */
	types = rc_stringlist_new();
	rc_stringlist_add(types, "ineed");
	rc_stringlist_add(types, "iuse");
	rc_stringlist_add(types, "iwant");
	rc_stringlist_add(types, "iafter");
	services = rc_deptree_depends(deptree, types, list, runlevel,
				      RC_DEP_STRICT | RC_DEP_TRACE | options);
	rc_stringlist_free(list);
	rc_stringlist_free(types);
	ঘূৰাই services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
স্থির bool
deep_mtime_check(পূর্ণাঙ্ক target_dir, ধ্রুৱ ৱর্ণ *target, bool newer, time_t *rel, ৱর্ণ *file)
{
	সংরচনা stat buf;
	bool retval = true;
	DIR *dp;
	সংরচনা dirent *d;

	/* If target does not exist, return true to mimic shell test */
	যদি (fstatat(target_dir, target, &buf, 0) != 0)
		ঘূৰাই true;

	যদি (newer) {
		যদি (*rel < buf.st_mtime) {
			retval = false;

			যদি (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} অন্যথা {
		যদি (*rel > buf.st_mtime) {
			retval = false;

			যদি (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	যদি (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		ঘূৰাই retval;

	/* Check all the entries in the dir */
	যেতিয়ালৈ ((d = readdir(dp))) {
		যদি (d->d_name[0] == '.')
			অনুৱর্ত;
		যদি (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ঘূৰাই retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
স্থির bool
mtime_check(পূর্ণাঙ্ক dirfd, ধ্রুৱ ৱর্ণ *source, ধ্রুৱ ৱর্ণ *target, bool newer, time_t *rel, ৱর্ণ *file)
{
	সংরচনা stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	যদি (stat(source, &buf) != 0)
		ঘূৰাই false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	যদি (rel) {
		*rel = mtime;
	}
	ঘূৰাই retval;
}

bool
rc_newer_than(ধ্রুৱ ৱর্ণ *source, ধ্রুৱ ৱর্ণ *target, time_t *newest, ৱর্ণ *file)
{

	ঘূৰাই mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ধ্রুৱ ৱর্ণ *source, ধ্রুৱ ৱর্ণ *target, time_t *oldest, ৱর্ণ *file)
{
	ঘূৰাই mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

প্রকারনির্দেশ সংরচনা deppair
{
	ধ্রুৱ ৱর্ণ *depend;
	ধ্রুৱ ৱর্ণ *addto;
} DEPPAIR;

স্থির ধ্রুৱ DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ রিক্ত, রিক্ত }
};

bool
rc_deptree_update_needed(time_t *newest, ৱর্ণ *file)
{
	bool newer = false;
	ধ্রুৱ পূর্ণাঙ্ক *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	সংরচনা stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	যদি (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	যদি (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} অন্যথা {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(রিক্ত);
	}

	বাবে (size_t i = 0, গুনতি = rc_scriptdirfds(&dirfds); i < গুনতি; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	যদি (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->মান, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	যদি ((newer) && (newest != রিক্ত)) {
	    *newest = mtime;
	}

	ঘূৰাই newer;
}

স্থির শূন্য
setup_environment(শূন্য)
{
	সংরচনা utsname uts;
	size_t env_size;
	ৱর্ণ *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	বাবে (ধ্রুৱ ৱর্ণ * ধ্রুৱ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		যদি (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	মুক্ত(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	যদি (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	যদি (uname(&uts) == 0)
		setenv("RC_UNAME", uts.sysname, 1);
}

/* This is a 7 phase operation
   Phase 1 is a shell script which loads each init script and config in turn
   and echos their dependency info to stdout
   Phase 2 takes that and populates a depinfo object with that data
   Phase 3 adds any provided services to the depinfo object
   Phase 4 scans that depinfo object and puts in backlinks
   Phase 5 removes broken before dependencies
   Phase 6 looks for duplicate services indicating a real and virtual service
   with the same names
   Phase 7 saves the depinfo object to disk
   */
bool
rc_deptree_update(শূন্য)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = রিক্ত, *depinfo_np, *di;
	RC_DEPTYPE *deptype = রিক্ত, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	ৱর্ণ *সাৰি = রিক্ত;
	size_t size;
	ৱর্ণ *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ধ্রুৱ ৱর্ণ *sys = rc_sys();
	পূর্ণাঙ্ক serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	যদি (!(fp = popen(GENDEP, "r")))
		ঘূৰাই false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	যেতিয়ালৈ (xgetline(&সাৰি, &size, fp) != -1) {
		depends = সাৰি;
		service = strsep(&depends, " ");
		যদি (!service || !*service)
			অনুৱর্ত;

		type = strsep(&depends, " ");
		যদি (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = রিক্ত;
			depinfo = get_depinfo(deptree, service);
			যদি (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		যদি (!type || !depends)
			অনুৱর্ত;

		/* Get the type */
		যদি (strcmp(type, "config") != 0) {
			যদি (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				যদি (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		যেতিয়ালৈ ((depend = strsep(&depends, " "))) {
			যদি (depend[0] == 0)
				অনুৱর্ত;

			যদি (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				অনুৱর্ত;
			}

			/* Don't depend on ourself */
			যদি (strcmp(depend, service) == 0)
				অনুৱর্ত;

			/* .sh files are not init scripts */
			l = strlen(depend);
			যদি (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				অনুৱর্ত;

			/* Remove our dependency if instructed */
			যদি (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				অনুৱর্ত;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			যদি (strcmp(type, "ibefore") == 0) {
				যদি ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			যদি (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				যদি ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	মুক্ত(সাৰি);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	যদি (sys) {
		ৱর্ণ *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		বাবে (i = 0; i < len; i++)
			nosys[i + 1] = (ৱর্ণ)tolower((unsigned ৱর্ণ)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		বাবে (i = 0; i < len; i++)
			onosys[i + 2] = (ৱর্ণ)tolower((unsigned ৱর্ণ)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			যদি (!(deptype = get_deptype(depinfo, "keyword")))
				অনুৱর্ত;
			TAILQ_FOREACH(s, deptype->services, entries) {
				যদি (strcmp(s->মান, nosys) != 0 && strcmp(s->মান, onosys) != 0)
					অনুৱর্ত;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						যদি (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->মান);
						যদি (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							মুক্ত(dt->type);
							মুক্ত(dt->services);
							মুক্ত(dt);
						}
					}
				}
			}
		}
		মুক্ত(nosys);
		মুক্ত(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(পরিমাণ(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		যদি (!(deptype = get_deptype(depinfo, "iprovide")))
			অনুৱর্ত;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->মান);
			যদি (!di)
				di = make_depinfo(providers, s->মান);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	মুক্ত(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		বাবে (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			যদি (!deptype)
				অনুৱর্ত;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->মান);
				যদি (!di) {
					যদি (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->মান);
						dt = get_deptype(depinfo, "broken");
						যদি (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->মান);
					}
					অনুৱর্ত;
				}

				dt = get_deptype(di, deppairs[i].addto);
				যদি (!dt)
					dt = make_deptype(di, deppairs[i].addto);
				rc_stringlist_addu(dt->services, depinfo->service);
			}
		}
	}

	/* Phase 5 - Remove broken before directives */
	types = rc_stringlist_new();
	rc_stringlist_add(types, "ineed");
	rc_stringlist_add(types, "iwant");
	rc_stringlist_add(types, "iuse");
	rc_stringlist_add(types, "iafter");
	TAILQ_FOREACH(depinfo, deptree, entries) {
		deptype = get_deptype(depinfo, "ibefore");
		যদি (!deptype)
			অনুৱর্ত;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      রিক্ত, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->মান);
				যদি (!di)
					অনুৱর্ত;
				যদি (strcmp(s2->মান, s3->মান) == 0) {
					dt = get_deptype(di, "iafter");
					যদি (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					ভঙ্গ;
				}
				dt = get_deptype(di, "iprovide");
				যদি (!dt)
					অনুৱর্ত;
				TAILQ_FOREACH(s4, dt->services, entries) {
					যদি (strcmp(s4->মান, s2->মান) == 0)
						ভঙ্গ;
				}
				যদি (s4) {
					di = get_depinfo(deptree, s4->মান);
					যদি (di) {
						dt = get_deptype(di, "iafter");
						যদি (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					ভঙ্গ;
				}
			}
			যদি (s3)
				rc_stringlist_delete(deptype->services, s2->মান);
		}
		rc_stringlist_free(sorted);
	}
	rc_stringlist_free(types);

	/* Phase 6 - Print errors for duplicate services */
	dupes = rc_stringlist_new();
	TAILQ_FOREACH(depinfo, deptree, entries) {
		serrno = errno;
		errno = 0;
		rc_stringlist_addu(dupes,depinfo->service);
		যদি (errno == EEXIST) {
			fprintf(stderr,
					"Error: %s is the name of a real and virtual service.\n",
					depinfo->service);
		}
		errno = serrno;
	}
	rc_stringlist_free(dupes);

	/* Phase 7 - save to disk
	   Now that we're purely in C, do we need to keep a shell parseable file?
	   I think yes as then it stays human readable
	   This works and should be entirely shell parseable provided that depend
	   names don't have any non shell variable characters in
	   */
	যদি ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->মান);
				}
			}
			i++;
		}
		fclose(fp);
	} অন্যথা {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	যদি (TAILQ_FIRST(config)) {
		যদি ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->মান);
			fclose(fp);
		} অন্যথা {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} অন্যথা {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ঘূৰাই retval;
}
