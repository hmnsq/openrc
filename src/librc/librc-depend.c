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

#அந்தர்பாவ <sys/utsname.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "queue.h"
#அந்தர்பாவ "librc.h"
#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

நிலையான மாறிலி எழுத்து *bootlevel = வெற்று;

நிலையான எழுத்து *
get_shell_மதிப்பு(எழுத்து *string)
{
	எழுத்து *p = string;
	எழுத்து *e;

	எனில் (!string)
		திரும்பு வெற்று;

	எனில் (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	எனில் (*e == '\n')
		*e-- = 0;
	எனில் (*e == '\'')
		*e-- = 0;

	எனில் (*p != 0)
		திரும்பு p;

	திரும்பு வெற்று;
}

வெறுமை
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	எனில் (!deptree)
		திரும்பு;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			விடுவி(dt->type);
			விடுவி(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		விடுவி(di->service);
		விடுவி(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	விடுவி(deptree);
}

நிலையான RC_DEPINFO *
get_depinfo(மாறிலி RC_DEPTREE *deptree, மாறிலி எழுத்து *service)
{
	RC_DEPINFO *di;
	எனில் (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			எனில் (strcmp(di->service, service) == 0)
				திரும்பு di;
	}
	திரும்பு வெற்று;
}

நிலையான RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, மாறிலி எழுத்து *service)
{
	RC_DEPINFO *depinfo = xmalloc(அளவு(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_செருகு_TAIL(deptree, depinfo, entries);

	திரும்பு depinfo;
}

நிலையான RC_DEPTYPE *
get_deptype(மாறிலி RC_DEPINFO *depinfo, மாறிலி எழுத்து *type)
{
	RC_DEPTYPE *dt;

	எனில் (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			எனில் (strcmp(dt->type, type) == 0)
				திரும்பு dt;
	}
	திரும்பு வெற்று;
}

நிலையான RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, மாறிலி எழுத்து *type)
{
	RC_DEPTYPE *deptype = xmalloc(அளவு(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_செருகு_TAIL(&depinfo->depends, deptype, entries);

	திரும்பு deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
நிலையான RC_DEPTREE *
make_deptree(வெறுமை) {
	RC_DEPTREE *deptree = xmalloc(அளவு(*deptree));
	TAILQ_INIT(deptree);
	திரும்பு deptree;
}

நிலையான RC_DEPTREE *
deptree_load_file(முழு dirfd, மாறிலி எழுத்து *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = வெற்று;
	RC_DEPTYPE *deptype = வெற்று;
	எழுத்து *வரி = வெற்று;
	size_t size;
	எழுத்து *type;
	எழுத்து *p;
	எழுத்து *e;
	முழு i;
	FILE *fp;

	எனில் (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		திரும்பு வெற்று;

	deptree = make_deptree();
	போது (xgetline(&வரி, &size, fp) != -1) {
		p = வரி;
		e = strsep(&p, "_");
		எனில் (!e || strcmp(e, "depinfo") != 0)
			தொடர்;
		e = strsep(&p, "_");
		எனில் (!e || sscanf(e, "%d", &i) != 1)
			தொடர்;
		எனில் (!(type = strsep(&p, "_=")))
			தொடர்;
		எனில் (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_மதிப்பு(p);
			எனில் (!e || *e == '\0')
				தொடர்;
			depinfo = make_depinfo(deptree, e);
			deptype = வெற்று;
			தொடர்;
		}
		e = strsep(&p, "=");
		எனில் (!e || sscanf(e, "%d", &i) != 1)
			தொடர்;
		/* Sanity */
		e = get_shell_மதிப்பு(p);
		எனில் (!e || *e == '\0')
			தொடர்;
		எனில் (!depinfo)
			தொடர்;
		எனில் (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	விடுவி(வரி);
	fclose(fp);

	திரும்பு deptree;
}

RC_DEPTREE *
rc_deptree_load(வெறுமை)
{
	திரும்பு deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(மாறிலி எழுத்து *deptree_file)
{
	திரும்பு deptree_load_file(AT_FDCWD, deptree_file);
}

நிலையான bool
valid_service(மாறிலி எழுத்து *runlevel, மாறிலி எழுத்து *service, மாறிலி எழுத்து *type)
{
	RC_SERVICE state;

	எனில் (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		திரும்பு true;

	எனில் (rc_service_in_runlevel(service, runlevel))
		திரும்பு true;
	எனில் (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    திரும்பு false;
	எனில் (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    திரும்பு false;
	எனில் (strcmp(runlevel, bootlevel) != 0) {
		எனில் (rc_service_in_runlevel(service, bootlevel))
			திரும்பு true;
	}

	state = rc_service_state(service);
	எனில் (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		திரும்பு true;

	திரும்பு false;
}

நிலையான bool
get_provided1(மாறிலி எழுத்து *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, மாறிலி எழுத்து *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	மாறிலி எழுத்து *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->மதிப்பு;
		st = rc_service_state(svc);

		எனில் (level)
			ok = rc_service_in_runlevel(svc, level);
		இல்லையெனில் எனில் (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		எனில் (!ok)
			தொடர்;
		தேர்வு (state) {
			நிலை RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				நிறுத்து;
			நிலை RC_SERVICE_INACTIVE:
			நிலை RC_SERVICE_STARTING:
			நிலை RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				நிறுத்து;
			இயல்பு:
				நிறுத்து;
		}
		எனில் (!ok)
			தொடர்;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	திரும்பு retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
நிலையான RC_STRINGLIST *
get_provided(மாறிலி RC_DEPINFO *depinfo, மாறிலி எழுத்து *runlevel, முழு options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	எனில் (!dt)
		திரும்பு providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	எனில் (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->மதிப்பு);
		திரும்பு providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	எனில் (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			எனில் (rc_service_in_runlevel(service->மதிப்பு, runlevel) ||
			    rc_service_in_runlevel(service->மதிப்பு, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->மதிப்பு) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->மதிப்பு);
		எனில் (TAILQ_FIRST(providers))
			திரும்பு providers;
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
	எனில் (TAILQ_FIRST(providers)) { \
		எனில் (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		திரும்பு providers; \
	}

	/* Anything running has to come first */
	எனில் (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	எனில் (get_provided1(runlevel, providers, dt, வெற்று, true, RC_SERVICE_STARTED))
	{ DO }
	எனில் (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	எனில் (get_provided1(runlevel, providers, dt, வெற்று, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	எனில் (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		திரும்பு providers;
	எனில் (get_provided1(runlevel, providers, dt, வெற்று, true, RC_SERVICE_STARTING))
		திரும்பு providers;
	எனில் (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    திரும்பு providers;
	எனில் (get_provided1(runlevel, providers, dt, வெற்று, false, RC_SERVICE_STARTING))
		திரும்பு providers;

	/* Nothing started then. OK, lets get the stopped services */
	எனில் (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		திரும்பு providers;
	எனில் (get_provided1(runlevel, providers, dt, வெற்று, true, RC_SERVICE_STOPPED))
	{ DO }
	எனில் (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		திரும்பு providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	எனில் (service != வெற்று)
		rc_stringlist_add(providers, service->மதிப்பு);

	திரும்பு providers;
}

நிலையான வெறுமை
visit_service(மாறிலி RC_DEPTREE *deptree,
	      மாறிலி RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      மாறிலி RC_DEPINFO *depinfo,
	      மாறிலி எழுத்து *runlevel, முழு options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	மாறிலி எழுத்து *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		எனில் (strcmp(type->மதிப்பு, depinfo->service) == 0)
			திரும்பு;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		எனில் (!(dt = get_deptype(depinfo, type->மதிப்பு)))
			தொடர்;

		TAILQ_FOREACH(service, dt->services, entries) {
			எனில் (!(options & RC_DEP_TRACE) ||
			    strcmp(type->மதிப்பு, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->மதிப்பு);
				தொடர்;
			}

			எனில் (!(di = get_depinfo(deptree, service->மதிப்பு)))
				தொடர்;
			provided = get_provided(di, runlevel, options);

			எனில் (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->மதிப்பு);
					எனில் (di && valid_service(runlevel, di->service, type->மதிப்பு))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			இல்லையெனில் எனில் (di && valid_service(runlevel, service->மதிப்பு, type->மதிப்பு))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	எனில் (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			எனில் (!(di = get_depinfo(deptree, service->மதிப்பு)))
				தொடர்;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				எனில் (strcmp(p->மதிப்பு, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					நிறுத்து;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	எனில் (!svcname || strcmp(svcname, depinfo->service) != 0) {
		எனில் (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(மாறிலி RC_DEPTREE *deptree,
		  மாறிலி எழுத்து *service, மாறிலி எழுத்து *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	எனில் (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		திரும்பு svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->மதிப்பு);
	திரும்பு svcs;
}

RC_STRINGLIST *
rc_deptree_depends(மாறிலி RC_DEPTREE *deptree,
		   மாறிலி RC_STRINGLIST *types,
		   மாறிலி RC_STRINGLIST *services,
		   மாறிலி எழுத்து *runlevel, முழு options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	மாறிலி RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	எனில் (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		எனில் (!(di = get_depinfo(deptree, service->மதிப்பு))) {
			errno = ENOENT;
			தொடர்;
		}
		எனில் (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	திரும்பு sorted;
}

RC_STRINGLIST *
rc_deptree_order(மாறிலி RC_DEPTREE *deptree, மாறிலி எழுத்து *runlevel, முழு options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	எனில் (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	எனில் (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		விடுவி(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		விடுவி(list2);
	} இல்லையெனில் {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		எனில் (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			விடுவி(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			விடுவி(list2);
			/* If we're not the boot runlevel then add that too */
			எனில் (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				விடுவி(list2);
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
	திரும்பு services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
நிலையான bool
deep_mtime_check(முழு target_dir, மாறிலி எழுத்து *target, bool newer, time_t *rel, எழுத்து *file)
{
	கட்டமைப்பு stat buf;
	bool retval = true;
	DIR *dp;
	கட்டமைப்பு dirent *d;

	/* If target does not exist, return true to mimic shell test */
	எனில் (fstatat(target_dir, target, &buf, 0) != 0)
		திரும்பு true;

	எனில் (newer) {
		எனில் (*rel < buf.st_mtime) {
			retval = false;

			எனில் (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} இல்லையெனில் {
		எனில் (*rel > buf.st_mtime) {
			retval = false;

			எனில் (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	எனில் (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		திரும்பு retval;

	/* Check all the entries in the dir */
	போது ((d = readdir(dp))) {
		எனில் (d->d_name[0] == '.')
			தொடர்;
		எனில் (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	திரும்பு retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
நிலையான bool
mtime_check(முழு dirfd, மாறிலி எழுத்து *source, மாறிலி எழுத்து *target, bool newer, time_t *rel, எழுத்து *file)
{
	கட்டமைப்பு stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	எனில் (stat(source, &buf) != 0)
		திரும்பு false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	எனில் (rel) {
		*rel = mtime;
	}
	திரும்பு retval;
}

bool
rc_newer_than(மாறிலி எழுத்து *source, மாறிலி எழுத்து *target, time_t *newest, எழுத்து *file)
{

	திரும்பு mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(மாறிலி எழுத்து *source, மாறிலி எழுத்து *target, time_t *oldest, எழுத்து *file)
{
	திரும்பு mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

வகைவரை கட்டமைப்பு deppair
{
	மாறிலி எழுத்து *depend;
	மாறிலி எழுத்து *addto;
} DEPPAIR;

நிலையான மாறிலி DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ வெற்று, வெற்று }
};

bool
rc_deptree_update_needed(time_t *newest, எழுத்து *file)
{
	bool newer = false;
	மாறிலி முழு *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	கட்டமைப்பு stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	எனில் (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	எனில் (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} இல்லையெனில் {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(வெற்று);
	}

	க்கு (size_t i = 0, மொத்தம் = rc_scriptdirfds(&dirfds); i < மொத்தம்; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	எனில் (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->மதிப்பு, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	எனில் ((newer) && (newest != வெற்று)) {
	    *newest = mtime;
	}

	திரும்பு newer;
}

நிலையான வெறுமை
setup_environment(வெறுமை)
{
	கட்டமைப்பு utsname uts;
	size_t env_size;
	எழுத்து *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	க்கு (மாறிலி எழுத்து * மாறிலி *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		எனில் (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	விடுவி(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	எனில் (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	எனில் (uname(&uts) == 0)
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
rc_deptree_update(வெறுமை)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = வெற்று, *depinfo_np, *di;
	RC_DEPTYPE *deptype = வெற்று, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	எழுத்து *வரி = வெற்று;
	size_t size;
	எழுத்து *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	மாறிலி எழுத்து *sys = rc_sys();
	முழு serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	எனில் (!(fp = popen(GENDEP, "r")))
		திரும்பு false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	போது (xgetline(&வரி, &size, fp) != -1) {
		depends = வரி;
		service = strsep(&depends, " ");
		எனில் (!service || !*service)
			தொடர்;

		type = strsep(&depends, " ");
		எனில் (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = வெற்று;
			depinfo = get_depinfo(deptree, service);
			எனில் (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		எனில் (!type || !depends)
			தொடர்;

		/* Get the type */
		எனில் (strcmp(type, "config") != 0) {
			எனில் (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				எனில் (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		போது ((depend = strsep(&depends, " "))) {
			எனில் (depend[0] == 0)
				தொடர்;

			எனில் (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				தொடர்;
			}

			/* Don't depend on ourself */
			எனில் (strcmp(depend, service) == 0)
				தொடர்;

			/* .sh files are not init scripts */
			l = strlen(depend);
			எனில் (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				தொடர்;

			/* Remove our dependency if instructed */
			எனில் (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				தொடர்;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			எனில் (strcmp(type, "ibefore") == 0) {
				எனில் ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			எனில் (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				எனில் ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	விடுவி(வரி);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	எனில் (sys) {
		எழுத்து *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		க்கு (i = 0; i < len; i++)
			nosys[i + 1] = (எழுத்து)tolower((unsigned எழுத்து)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		க்கு (i = 0; i < len; i++)
			onosys[i + 2] = (எழுத்து)tolower((unsigned எழுத்து)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			எனில் (!(deptype = get_deptype(depinfo, "keyword")))
				தொடர்;
			TAILQ_FOREACH(s, deptype->services, entries) {
				எனில் (strcmp(s->மதிப்பு, nosys) != 0 && strcmp(s->மதிப்பு, onosys) != 0)
					தொடர்;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						எனில் (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->மதிப்பு);
						எனில் (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							விடுவி(dt->type);
							விடுவி(dt->services);
							விடுவி(dt);
						}
					}
				}
			}
		}
		விடுவி(nosys);
		விடுவி(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(அளவு(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		எனில் (!(deptype = get_deptype(depinfo, "iprovide")))
			தொடர்;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->மதிப்பு);
			எனில் (!di)
				di = make_depinfo(providers, s->மதிப்பு);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	விடுவி(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		க்கு (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			எனில் (!deptype)
				தொடர்;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->மதிப்பு);
				எனில் (!di) {
					எனில் (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->மதிப்பு);
						dt = get_deptype(depinfo, "broken");
						எனில் (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->மதிப்பு);
					}
					தொடர்;
				}

				dt = get_deptype(di, deppairs[i].addto);
				எனில் (!dt)
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
		எனில் (!deptype)
			தொடர்;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      வெற்று, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->மதிப்பு);
				எனில் (!di)
					தொடர்;
				எனில் (strcmp(s2->மதிப்பு, s3->மதிப்பு) == 0) {
					dt = get_deptype(di, "iafter");
					எனில் (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					நிறுத்து;
				}
				dt = get_deptype(di, "iprovide");
				எனில் (!dt)
					தொடர்;
				TAILQ_FOREACH(s4, dt->services, entries) {
					எனில் (strcmp(s4->மதிப்பு, s2->மதிப்பு) == 0)
						நிறுத்து;
				}
				எனில் (s4) {
					di = get_depinfo(deptree, s4->மதிப்பு);
					எனில் (di) {
						dt = get_deptype(di, "iafter");
						எனில் (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					நிறுத்து;
				}
			}
			எனில் (s3)
				rc_stringlist_delete(deptype->services, s2->மதிப்பு);
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
		எனில் (errno == EEXIST) {
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
	எனில் ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->மதிப்பு);
				}
			}
			i++;
		}
		fclose(fp);
	} இல்லையெனில் {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	எனில் (TAILQ_FIRST(config)) {
		எனில் ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->மதிப்பு);
			fclose(fp);
		} இல்லையெனில் {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} இல்லையெனில் {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	திரும்பு retval;
}
