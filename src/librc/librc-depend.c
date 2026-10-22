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

#ضمن <sys/utsname.h>
#ضمن <ctype.h>
#ضمن <dirent.h>
#ضمن <errno.h>
#ضمن <limits.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <time.h>
#ضمن <unistd.h>

#ضمن "queue.h"
#ضمن "librc.h"
#ضمن "helpers.h"
#ضمن "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ساكن ثابت حرف *bootlevel = عدم;

ساكن حرف *
get_shell_نيلاي(حرف *string)
{
	حرف *p = string;
	حرف *e;

	إذا (!string)
		أرجع عدم;

	إذا (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	إذا (*e == '\n')
		*e-- = 0;
	إذا (*e == '\'')
		*e-- = 0;

	إذا (*p != 0)
		أرجع p;

	أرجع عدم;
}

فراغ
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	إذا (!deptree)
		أرجع;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			حرر(dt->type);
			حرر(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		حرر(di->service);
		حرر(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	حرر(deptree);
}

ساكن RC_DEPINFO *
get_depinfo(ثابت RC_DEPTREE *deptree, ثابت حرف *service)
{
	RC_DEPINFO *di;
	إذا (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			إذا (strcmp(di->service, service) == 0)
				أرجع di;
	}
	أرجع عدم;
}

ساكن RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ثابت حرف *service)
{
	RC_DEPINFO *depinfo = xmalloc(حجم(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_سيسيڤ_TAIL(deptree, depinfo, entries);

	أرجع depinfo;
}

ساكن RC_DEPTYPE *
get_deptype(ثابت RC_DEPINFO *depinfo, ثابت حرف *type)
{
	RC_DEPTYPE *dt;

	إذا (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			إذا (strcmp(dt->type, type) == 0)
				أرجع dt;
	}
	أرجع عدم;
}

ساكن RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ثابت حرف *type)
{
	RC_DEPTYPE *deptype = xmalloc(حجم(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_سيسيڤ_TAIL(&depinfo->depends, deptype, entries);

	أرجع deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
ساكن RC_DEPTREE *
make_deptree(فراغ) {
	RC_DEPTREE *deptree = xmalloc(حجم(*deptree));
	TAILQ_INIT(deptree);
	أرجع deptree;
}

ساكن RC_DEPTREE *
deptree_load_file(صحيح dirfd, ثابت حرف *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = عدم;
	RC_DEPTYPE *deptype = عدم;
	حرف *باريس = عدم;
	size_t size;
	حرف *type;
	حرف *p;
	حرف *e;
	صحيح i;
	FILE *fp;

	إذا (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		أرجع عدم;

	deptree = make_deptree();
	طالما (xgetline(&باريس, &size, fp) != -1) {
		p = باريس;
		e = strsep(&p, "_");
		إذا (!e || strcmp(e, "depinfo") != 0)
			استمر;
		e = strsep(&p, "_");
		إذا (!e || sscanf(e, "%d", &i) != 1)
			استمر;
		إذا (!(type = strsep(&p, "_=")))
			استمر;
		إذا (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_نيلاي(p);
			إذا (!e || *e == '\0')
				استمر;
			depinfo = make_depinfo(deptree, e);
			deptype = عدم;
			استمر;
		}
		e = strsep(&p, "=");
		إذا (!e || sscanf(e, "%d", &i) != 1)
			استمر;
		/* Sanity */
		e = get_shell_نيلاي(p);
		إذا (!e || *e == '\0')
			استمر;
		إذا (!depinfo)
			استمر;
		إذا (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	حرر(باريس);
	fclose(fp);

	أرجع deptree;
}

RC_DEPTREE *
rc_deptree_load(فراغ)
{
	أرجع deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ثابت حرف *deptree_file)
{
	أرجع deptree_load_file(AT_FDCWD, deptree_file);
}

ساكن bool
valid_service(ثابت حرف *runlevel, ثابت حرف *service, ثابت حرف *type)
{
	RC_SERVICE state;

	إذا (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		أرجع true;

	إذا (rc_service_in_runlevel(service, runlevel))
		أرجع true;
	إذا (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    أرجع false;
	إذا (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    أرجع false;
	إذا (strcmp(runlevel, bootlevel) != 0) {
		إذا (rc_service_in_runlevel(service, bootlevel))
			أرجع true;
	}

	state = rc_service_state(service);
	إذا (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		أرجع true;

	أرجع false;
}

ساكن bool
get_provided1(ثابت حرف *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ثابت حرف *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ثابت حرف *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->نيلاي;
		st = rc_service_state(svc);

		إذا (level)
			ok = rc_service_in_runlevel(svc, level);
		وإلا إذا (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		إذا (!ok)
			استمر;
		بدل (state) {
			حالة RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				اكسر;
			حالة RC_SERVICE_INACTIVE:
			حالة RC_SERVICE_STARTING:
			حالة RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				اكسر;
			افتراضي:
				اكسر;
		}
		إذا (!ok)
			استمر;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	أرجع retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
ساكن RC_STRINGLIST *
get_provided(ثابت RC_DEPINFO *depinfo, ثابت حرف *runlevel, صحيح options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	إذا (!dt)
		أرجع providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	إذا (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->نيلاي);
		أرجع providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	إذا (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			إذا (rc_service_in_runlevel(service->نيلاي, runlevel) ||
			    rc_service_in_runlevel(service->نيلاي, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->نيلاي) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->نيلاي);
		إذا (TAILQ_FIRST(providers))
			أرجع providers;
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
	إذا (TAILQ_FIRST(providers)) { \
		إذا (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		أرجع providers; \
	}

	/* Anything running has to come first */
	إذا (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	إذا (get_provided1(runlevel, providers, dt, عدم, true, RC_SERVICE_STARTED))
	{ DO }
	إذا (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	إذا (get_provided1(runlevel, providers, dt, عدم, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	إذا (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		أرجع providers;
	إذا (get_provided1(runlevel, providers, dt, عدم, true, RC_SERVICE_STARTING))
		أرجع providers;
	إذا (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    أرجع providers;
	إذا (get_provided1(runlevel, providers, dt, عدم, false, RC_SERVICE_STARTING))
		أرجع providers;

	/* Nothing started then. OK, lets get the stopped services */
	إذا (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		أرجع providers;
	إذا (get_provided1(runlevel, providers, dt, عدم, true, RC_SERVICE_STOPPED))
	{ DO }
	إذا (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		أرجع providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	إذا (service != عدم)
		rc_stringlist_add(providers, service->نيلاي);

	أرجع providers;
}

ساكن فراغ
visit_service(ثابت RC_DEPTREE *deptree,
	      ثابت RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      ثابت RC_DEPINFO *depinfo,
	      ثابت حرف *runlevel, صحيح options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ثابت حرف *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		إذا (strcmp(type->نيلاي, depinfo->service) == 0)
			أرجع;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		إذا (!(dt = get_deptype(depinfo, type->نيلاي)))
			استمر;

		TAILQ_FOREACH(service, dt->services, entries) {
			إذا (!(options & RC_DEP_TRACE) ||
			    strcmp(type->نيلاي, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->نيلاي);
				استمر;
			}

			إذا (!(di = get_depinfo(deptree, service->نيلاي)))
				استمر;
			provided = get_provided(di, runlevel, options);

			إذا (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->نيلاي);
					إذا (di && valid_service(runlevel, di->service, type->نيلاي))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			وإلا إذا (di && valid_service(runlevel, service->نيلاي, type->نيلاي))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	إذا (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			إذا (!(di = get_depinfo(deptree, service->نيلاي)))
				استمر;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				إذا (strcmp(p->نيلاي, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					اكسر;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	إذا (!svcname || strcmp(svcname, depinfo->service) != 0) {
		إذا (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ثابت RC_DEPTREE *deptree,
		  ثابت حرف *service, ثابت حرف *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	إذا (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		أرجع svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->نيلاي);
	أرجع svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ثابت RC_DEPTREE *deptree,
		   ثابت RC_STRINGLIST *types,
		   ثابت RC_STRINGLIST *services,
		   ثابت حرف *runlevel, صحيح options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	ثابت RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	إذا (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		إذا (!(di = get_depinfo(deptree, service->نيلاي))) {
			errno = ENOENT;
			استمر;
		}
		إذا (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	أرجع sorted;
}

RC_STRINGLIST *
rc_deptree_order(ثابت RC_DEPTREE *deptree, ثابت حرف *runlevel, صحيح options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	إذا (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	إذا (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		حرر(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		حرر(list2);
	} وإلا {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		إذا (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			حرر(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			حرر(list2);
			/* If we're not the boot runlevel then add that too */
			إذا (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				حرر(list2);
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
	أرجع services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ساكن bool
deep_mtime_check(صحيح target_dir, ثابت حرف *target, bool newer, time_t *rel, حرف *file)
{
	بنية stat buf;
	bool retval = true;
	DIR *dp;
	بنية dirent *d;

	/* If target does not exist, return true to mimic shell test */
	إذا (fstatat(target_dir, target, &buf, 0) != 0)
		أرجع true;

	إذا (newer) {
		إذا (*rel < buf.st_mtime) {
			retval = false;

			إذا (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} وإلا {
		إذا (*rel > buf.st_mtime) {
			retval = false;

			إذا (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	إذا (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		أرجع retval;

	/* Check all the entries in the dir */
	طالما ((d = readdir(dp))) {
		إذا (d->d_name[0] == '.')
			استمر;
		إذا (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	أرجع retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ساكن bool
mtime_check(صحيح dirfd, ثابت حرف *source, ثابت حرف *target, bool newer, time_t *rel, حرف *file)
{
	بنية stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	إذا (stat(source, &buf) != 0)
		أرجع false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	إذا (rel) {
		*rel = mtime;
	}
	أرجع retval;
}

bool
rc_newer_than(ثابت حرف *source, ثابت حرف *target, time_t *newest, حرف *file)
{

	أرجع mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ثابت حرف *source, ثابت حرف *target, time_t *oldest, حرف *file)
{
	أرجع mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

تعريفنوع بنية deppair
{
	ثابت حرف *depend;
	ثابت حرف *addto;
} DEPPAIR;

ساكن ثابت DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ عدم, عدم }
};

bool
rc_deptree_update_needed(time_t *newest, حرف *file)
{
	bool newer = false;
	ثابت صحيح *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	بنية stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	إذا (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	إذا (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} وإلا {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(عدم);
	}

	لكل (size_t i = 0, کيراءن = rc_scriptdirfds(&dirfds); i < کيراءن; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	إذا (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->نيلاي, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	إذا ((newer) && (newest != عدم)) {
	    *newest = mtime;
	}

	أرجع newer;
}

ساكن فراغ
setup_environment(فراغ)
{
	بنية utsname uts;
	size_t env_size;
	حرف *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	لكل (ثابت حرف * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		إذا (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	حرر(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	إذا (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	إذا (uname(&uts) == 0)
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
rc_deptree_update(فراغ)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = عدم, *depinfo_np, *di;
	RC_DEPTYPE *deptype = عدم, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	حرف *باريس = عدم;
	size_t size;
	حرف *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ثابت حرف *sys = rc_sys();
	صحيح serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	إذا (!(fp = popen(GENDEP, "r")))
		أرجع false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	طالما (xgetline(&باريس, &size, fp) != -1) {
		depends = باريس;
		service = strsep(&depends, " ");
		إذا (!service || !*service)
			استمر;

		type = strsep(&depends, " ");
		إذا (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = عدم;
			depinfo = get_depinfo(deptree, service);
			إذا (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		إذا (!type || !depends)
			استمر;

		/* Get the type */
		إذا (strcmp(type, "config") != 0) {
			إذا (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				إذا (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		طالما ((depend = strsep(&depends, " "))) {
			إذا (depend[0] == 0)
				استمر;

			إذا (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				استمر;
			}

			/* Don't depend on ourself */
			إذا (strcmp(depend, service) == 0)
				استمر;

			/* .sh files are not init scripts */
			l = strlen(depend);
			إذا (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				استمر;

			/* Remove our dependency if instructed */
			إذا (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				استمر;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			إذا (strcmp(type, "ibefore") == 0) {
				إذا ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			إذا (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				إذا ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	حرر(باريس);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	إذا (sys) {
		حرف *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		لكل (i = 0; i < len; i++)
			nosys[i + 1] = (حرف)tolower((unsigned حرف)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		لكل (i = 0; i < len; i++)
			onosys[i + 2] = (حرف)tolower((unsigned حرف)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			إذا (!(deptype = get_deptype(depinfo, "keyword")))
				استمر;
			TAILQ_FOREACH(s, deptype->services, entries) {
				إذا (strcmp(s->نيلاي, nosys) != 0 && strcmp(s->نيلاي, onosys) != 0)
					استمر;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						إذا (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->نيلاي);
						إذا (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							حرر(dt->type);
							حرر(dt->services);
							حرر(dt);
						}
					}
				}
			}
		}
		حرر(nosys);
		حرر(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(حجم(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		إذا (!(deptype = get_deptype(depinfo, "iprovide")))
			استمر;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->نيلاي);
			إذا (!di)
				di = make_depinfo(providers, s->نيلاي);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	حرر(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		لكل (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			إذا (!deptype)
				استمر;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->نيلاي);
				إذا (!di) {
					إذا (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->نيلاي);
						dt = get_deptype(depinfo, "broken");
						إذا (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->نيلاي);
					}
					استمر;
				}

				dt = get_deptype(di, deppairs[i].addto);
				إذا (!dt)
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
		إذا (!deptype)
			استمر;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      عدم, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->نيلاي);
				إذا (!di)
					استمر;
				إذا (strcmp(s2->نيلاي, s3->نيلاي) == 0) {
					dt = get_deptype(di, "iafter");
					إذا (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					اكسر;
				}
				dt = get_deptype(di, "iprovide");
				إذا (!dt)
					استمر;
				TAILQ_FOREACH(s4, dt->services, entries) {
					إذا (strcmp(s4->نيلاي, s2->نيلاي) == 0)
						اكسر;
				}
				إذا (s4) {
					di = get_depinfo(deptree, s4->نيلاي);
					إذا (di) {
						dt = get_deptype(di, "iafter");
						إذا (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					اكسر;
				}
			}
			إذا (s3)
				rc_stringlist_delete(deptype->services, s2->نيلاي);
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
		إذا (errno == EEXIST) {
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
	إذا ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->نيلاي);
				}
			}
			i++;
		}
		fclose(fp);
	} وإلا {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	إذا (TAILQ_FIRST(config)) {
		إذا ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->نيلاي);
			fclose(fp);
		} وإلا {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} وإلا {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	أرجع retval;
}
