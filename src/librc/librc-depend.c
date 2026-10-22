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

#شامل <sys/utsname.h>
#شامل <ctype.h>
#شامل <dirent.h>
#شامل <errno.h>
#شامل <limits.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <time.h>
#شامل <unistd.h>

#شامل "queue.h"
#شامل "librc.h"
#شامل "helpers.h"
#شامل "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ایستا ثابت نویسه *bootlevel = پوچ;

ایستا نویسه *
get_shell_مقدار(نویسه *string)
{
	نویسه *p = string;
	نویسه *e;

	اگر (!string)
		بازگشت پوچ;

	اگر (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	اگر (*e == '\n')
		*e-- = 0;
	اگر (*e == '\'')
		*e-- = 0;

	اگر (*p != 0)
		بازگشت p;

	بازگشت پوچ;
}

تهی
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	اگر (!deptree)
		بازگشت;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			آزاد(dt->type);
			آزاد(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		آزاد(di->service);
		آزاد(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	آزاد(deptree);
}

ایستا RC_DEPINFO *
get_depinfo(ثابت RC_DEPTREE *deptree, ثابت نویسه *service)
{
	RC_DEPINFO *di;
	اگر (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			اگر (strcmp(di->service, service) == 0)
				بازگشت di;
	}
	بازگشت پوچ;
}

ایستا RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ثابت نویسه *service)
{
	RC_DEPINFO *depinfo = xmalloc(اندازه(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_درج_TAIL(deptree, depinfo, entries);

	بازگشت depinfo;
}

ایستا RC_DEPTYPE *
get_deptype(ثابت RC_DEPINFO *depinfo, ثابت نویسه *type)
{
	RC_DEPTYPE *dt;

	اگر (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			اگر (strcmp(dt->type, type) == 0)
				بازگشت dt;
	}
	بازگشت پوچ;
}

ایستا RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ثابت نویسه *type)
{
	RC_DEPTYPE *deptype = xmalloc(اندازه(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_درج_TAIL(&depinfo->depends, deptype, entries);

	بازگشت deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
ایستا RC_DEPTREE *
make_deptree(تهی) {
	RC_DEPTREE *deptree = xmalloc(اندازه(*deptree));
	TAILQ_INIT(deptree);
	بازگشت deptree;
}

ایستا RC_DEPTREE *
deptree_load_file(صحیح dirfd, ثابت نویسه *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = پوچ;
	RC_DEPTYPE *deptype = پوچ;
	نویسه *سطر = پوچ;
	size_t size;
	نویسه *type;
	نویسه *p;
	نویسه *e;
	صحیح i;
	FILE *fp;

	اگر (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		بازگشت پوچ;

	deptree = make_deptree();
	وقتی (xgetline(&سطر, &size, fp) != -1) {
		p = سطر;
		e = strsep(&p, "_");
		اگر (!e || strcmp(e, "depinfo") != 0)
			ادامه;
		e = strsep(&p, "_");
		اگر (!e || sscanf(e, "%d", &i) != 1)
			ادامه;
		اگر (!(type = strsep(&p, "_=")))
			ادامه;
		اگر (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_مقدار(p);
			اگر (!e || *e == '\0')
				ادامه;
			depinfo = make_depinfo(deptree, e);
			deptype = پوچ;
			ادامه;
		}
		e = strsep(&p, "=");
		اگر (!e || sscanf(e, "%d", &i) != 1)
			ادامه;
		/* Sanity */
		e = get_shell_مقدار(p);
		اگر (!e || *e == '\0')
			ادامه;
		اگر (!depinfo)
			ادامه;
		اگر (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	آزاد(سطر);
	fclose(fp);

	بازگشت deptree;
}

RC_DEPTREE *
rc_deptree_load(تهی)
{
	بازگشت deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ثابت نویسه *deptree_file)
{
	بازگشت deptree_load_file(AT_FDCWD, deptree_file);
}

ایستا bool
valid_service(ثابت نویسه *runlevel, ثابت نویسه *service, ثابت نویسه *type)
{
	RC_SERVICE state;

	اگر (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		بازگشت true;

	اگر (rc_service_in_runlevel(service, runlevel))
		بازگشت true;
	اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    بازگشت false;
	اگر (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    بازگشت false;
	اگر (strcmp(runlevel, bootlevel) != 0) {
		اگر (rc_service_in_runlevel(service, bootlevel))
			بازگشت true;
	}

	state = rc_service_state(service);
	اگر (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		بازگشت true;

	بازگشت false;
}

ایستا bool
get_provided1(ثابت نویسه *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ثابت نویسه *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ثابت نویسه *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->مقدار;
		st = rc_service_state(svc);

		اگر (level)
			ok = rc_service_in_runlevel(svc, level);
		وگرنه اگر (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		اگر (!ok)
			ادامه;
		انتخاب (state) {
			حالت RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				بشکن;
			حالت RC_SERVICE_INACTIVE:
			حالت RC_SERVICE_STARTING:
			حالت RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				بشکن;
			پیشفرض:
				بشکن;
		}
		اگر (!ok)
			ادامه;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	بازگشت retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
ایستا RC_STRINGLIST *
get_provided(ثابت RC_DEPINFO *depinfo, ثابت نویسه *runlevel, صحیح options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	اگر (!dt)
		بازگشت providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	اگر (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->مقدار);
		بازگشت providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	اگر (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			اگر (rc_service_in_runlevel(service->مقدار, runlevel) ||
			    rc_service_in_runlevel(service->مقدار, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->مقدار) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->مقدار);
		اگر (TAILQ_FIRST(providers))
			بازگشت providers;
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
	اگر (TAILQ_FIRST(providers)) { \
		اگر (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		بازگشت providers; \
	}

	/* Anything running has to come first */
	اگر (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	اگر (get_provided1(runlevel, providers, dt, پوچ, true, RC_SERVICE_STARTED))
	{ DO }
	اگر (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	اگر (get_provided1(runlevel, providers, dt, پوچ, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	اگر (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		بازگشت providers;
	اگر (get_provided1(runlevel, providers, dt, پوچ, true, RC_SERVICE_STARTING))
		بازگشت providers;
	اگر (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    بازگشت providers;
	اگر (get_provided1(runlevel, providers, dt, پوچ, false, RC_SERVICE_STARTING))
		بازگشت providers;

	/* Nothing started then. OK, lets get the stopped services */
	اگر (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		بازگشت providers;
	اگر (get_provided1(runlevel, providers, dt, پوچ, true, RC_SERVICE_STOPPED))
	{ DO }
	اگر (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		بازگشت providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	اگر (service != پوچ)
		rc_stringlist_add(providers, service->مقدار);

	بازگشت providers;
}

ایستا تهی
visit_service(ثابت RC_DEPTREE *deptree,
	      ثابت RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      ثابت RC_DEPINFO *depinfo,
	      ثابت نویسه *runlevel, صحیح options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ثابت نویسه *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		اگر (strcmp(type->مقدار, depinfo->service) == 0)
			بازگشت;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		اگر (!(dt = get_deptype(depinfo, type->مقدار)))
			ادامه;

		TAILQ_FOREACH(service, dt->services, entries) {
			اگر (!(options & RC_DEP_TRACE) ||
			    strcmp(type->مقدار, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->مقدار);
				ادامه;
			}

			اگر (!(di = get_depinfo(deptree, service->مقدار)))
				ادامه;
			provided = get_provided(di, runlevel, options);

			اگر (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->مقدار);
					اگر (di && valid_service(runlevel, di->service, type->مقدار))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			وگرنه اگر (di && valid_service(runlevel, service->مقدار, type->مقدار))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	اگر (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			اگر (!(di = get_depinfo(deptree, service->مقدار)))
				ادامه;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				اگر (strcmp(p->مقدار, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					بشکن;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	اگر (!svcname || strcmp(svcname, depinfo->service) != 0) {
		اگر (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ثابت RC_DEPTREE *deptree,
		  ثابت نویسه *service, ثابت نویسه *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	اگر (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		بازگشت svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->مقدار);
	بازگشت svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ثابت RC_DEPTREE *deptree,
		   ثابت RC_STRINGLIST *types,
		   ثابت RC_STRINGLIST *services,
		   ثابت نویسه *runlevel, صحیح options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	ثابت RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	اگر (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		اگر (!(di = get_depinfo(deptree, service->مقدار))) {
			errno = ENOENT;
			ادامه;
		}
		اگر (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	بازگشت sorted;
}

RC_STRINGLIST *
rc_deptree_order(ثابت RC_DEPTREE *deptree, ثابت نویسه *runlevel, صحیح options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	اگر (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	اگر (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		آزاد(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		آزاد(list2);
	} وگرنه {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			آزاد(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			آزاد(list2);
			/* If we're not the boot runlevel then add that too */
			اگر (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				آزاد(list2);
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
	بازگشت services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ایستا bool
deep_mtime_check(صحیح target_dir, ثابت نویسه *target, bool newer, time_t *rel, نویسه *file)
{
	ساختار stat buf;
	bool retval = true;
	DIR *dp;
	ساختار dirent *d;

	/* If target does not exist, return true to mimic shell test */
	اگر (fstatat(target_dir, target, &buf, 0) != 0)
		بازگشت true;

	اگر (newer) {
		اگر (*rel < buf.st_mtime) {
			retval = false;

			اگر (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} وگرنه {
		اگر (*rel > buf.st_mtime) {
			retval = false;

			اگر (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	اگر (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		بازگشت retval;

	/* Check all the entries in the dir */
	وقتی ((d = readdir(dp))) {
		اگر (d->d_name[0] == '.')
			ادامه;
		اگر (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	بازگشت retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ایستا bool
mtime_check(صحیح dirfd, ثابت نویسه *source, ثابت نویسه *target, bool newer, time_t *rel, نویسه *file)
{
	ساختار stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	اگر (stat(source, &buf) != 0)
		بازگشت false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	اگر (rel) {
		*rel = mtime;
	}
	بازگشت retval;
}

bool
rc_newer_than(ثابت نویسه *source, ثابت نویسه *target, time_t *newest, نویسه *file)
{

	بازگشت mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ثابت نویسه *source, ثابت نویسه *target, time_t *oldest, نویسه *file)
{
	بازگشت mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

تعریفنوع ساختار deppair
{
	ثابت نویسه *depend;
	ثابت نویسه *addto;
} DEPPAIR;

ایستا ثابت DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ پوچ, پوچ }
};

bool
rc_deptree_update_needed(time_t *newest, نویسه *file)
{
	bool newer = false;
	ثابت صحیح *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ساختار stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	اگر (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	اگر (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} وگرنه {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(پوچ);
	}

	برای (size_t i = 0, شمار = rc_scriptdirfds(&dirfds); i < شمار; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	اگر (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->مقدار, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	اگر ((newer) && (newest != پوچ)) {
	    *newest = mtime;
	}

	بازگشت newer;
}

ایستا تهی
setup_environment(تهی)
{
	ساختار utsname uts;
	size_t env_size;
	نویسه *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	برای (ثابت نویسه * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		اگر (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	آزاد(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	اگر (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	اگر (uname(&uts) == 0)
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
rc_deptree_update(تهی)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = پوچ, *depinfo_np, *di;
	RC_DEPTYPE *deptype = پوچ, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	نویسه *سطر = پوچ;
	size_t size;
	نویسه *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ثابت نویسه *sys = rc_sys();
	صحیح serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	اگر (!(fp = popen(GENDEP, "r")))
		بازگشت false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	وقتی (xgetline(&سطر, &size, fp) != -1) {
		depends = سطر;
		service = strsep(&depends, " ");
		اگر (!service || !*service)
			ادامه;

		type = strsep(&depends, " ");
		اگر (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = پوچ;
			depinfo = get_depinfo(deptree, service);
			اگر (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		اگر (!type || !depends)
			ادامه;

		/* Get the type */
		اگر (strcmp(type, "config") != 0) {
			اگر (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				اگر (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		وقتی ((depend = strsep(&depends, " "))) {
			اگر (depend[0] == 0)
				ادامه;

			اگر (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				ادامه;
			}

			/* Don't depend on ourself */
			اگر (strcmp(depend, service) == 0)
				ادامه;

			/* .sh files are not init scripts */
			l = strlen(depend);
			اگر (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				ادامه;

			/* Remove our dependency if instructed */
			اگر (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				ادامه;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			اگر (strcmp(type, "ibefore") == 0) {
				اگر ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			اگر (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				اگر ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	آزاد(سطر);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	اگر (sys) {
		نویسه *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		برای (i = 0; i < len; i++)
			nosys[i + 1] = (نویسه)tolower((unsigned نویسه)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		برای (i = 0; i < len; i++)
			onosys[i + 2] = (نویسه)tolower((unsigned نویسه)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			اگر (!(deptype = get_deptype(depinfo, "keyword")))
				ادامه;
			TAILQ_FOREACH(s, deptype->services, entries) {
				اگر (strcmp(s->مقدار, nosys) != 0 && strcmp(s->مقدار, onosys) != 0)
					ادامه;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						اگر (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->مقدار);
						اگر (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							آزاد(dt->type);
							آزاد(dt->services);
							آزاد(dt);
						}
					}
				}
			}
		}
		آزاد(nosys);
		آزاد(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(اندازه(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		اگر (!(deptype = get_deptype(depinfo, "iprovide")))
			ادامه;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->مقدار);
			اگر (!di)
				di = make_depinfo(providers, s->مقدار);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	آزاد(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		برای (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			اگر (!deptype)
				ادامه;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->مقدار);
				اگر (!di) {
					اگر (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->مقدار);
						dt = get_deptype(depinfo, "broken");
						اگر (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->مقدار);
					}
					ادامه;
				}

				dt = get_deptype(di, deppairs[i].addto);
				اگر (!dt)
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
		اگر (!deptype)
			ادامه;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      پوچ, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->مقدار);
				اگر (!di)
					ادامه;
				اگر (strcmp(s2->مقدار, s3->مقدار) == 0) {
					dt = get_deptype(di, "iafter");
					اگر (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					بشکن;
				}
				dt = get_deptype(di, "iprovide");
				اگر (!dt)
					ادامه;
				TAILQ_FOREACH(s4, dt->services, entries) {
					اگر (strcmp(s4->مقدار, s2->مقدار) == 0)
						بشکن;
				}
				اگر (s4) {
					di = get_depinfo(deptree, s4->مقدار);
					اگر (di) {
						dt = get_deptype(di, "iafter");
						اگر (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					بشکن;
				}
			}
			اگر (s3)
				rc_stringlist_delete(deptype->services, s2->مقدار);
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
		اگر (errno == EEXIST) {
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
	اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->مقدار);
				}
			}
			i++;
		}
		fclose(fp);
	} وگرنه {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	اگر (TAILQ_FIRST(config)) {
		اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->مقدار);
			fclose(fp);
		} وگرنه {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} وگرنه {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	بازگشت retval;
}
