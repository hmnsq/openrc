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
get_shell_value(حرف *string)
{
	حرف *p = string;
	حرف *e;

	ܐܢ (!string)
		ܦܢܐ عدم;

	ܐܢ (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	ܐܢ (*e == '\n')
		*e-- = 0;
	ܐܢ (*e == '\'')
		*e-- = 0;

	ܐܢ (*p != 0)
		ܦܢܐ p;

	ܦܢܐ عدم;
}

فراغ
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ܐܢ (!deptree)
		ܦܢܐ;

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
	ܐܢ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ܐܢ (strcmp(di->service, service) == 0)
				ܦܢܐ di;
	}
	ܦܢܐ عدم;
}

ساكن RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ثابت حرف *service)
{
	RC_DEPINFO *depinfo = xmalloc(حجم(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_INSERT_TAIL(deptree, depinfo, entries);

	ܦܢܐ depinfo;
}

ساكن RC_DEPTYPE *
get_deptype(ثابت RC_DEPINFO *depinfo, ثابت حرف *type)
{
	RC_DEPTYPE *dt;

	ܐܢ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ܐܢ (strcmp(dt->type, type) == 0)
				ܦܢܐ dt;
	}
	ܦܢܐ عدم;
}

ساكن RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ثابت حرف *type)
{
	RC_DEPTYPE *deptype = xmalloc(حجم(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_INSERT_TAIL(&depinfo->depends, deptype, entries);

	ܦܢܐ deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
ساكن RC_DEPTREE *
make_deptree(فراغ) {
	RC_DEPTREE *deptree = xmalloc(حجم(*deptree));
	TAILQ_INIT(deptree);
	ܦܢܐ deptree;
}

ساكن RC_DEPTREE *
deptree_load_file(صحيح dirfd, ثابت حرف *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = عدم;
	RC_DEPTYPE *deptype = عدم;
	حرف *line = عدم;
	size_t size;
	حرف *type;
	حرف *p;
	حرف *e;
	صحيح i;
	FILE *fp;

	ܐܢ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ܦܢܐ عدم;

	deptree = make_deptree();
	ܟܡܐ (xgetline(&line, &size, fp) != -1) {
		p = line;
		e = strsep(&p, "_");
		ܐܢ (!e || strcmp(e, "depinfo") != 0)
			استمر;
		e = strsep(&p, "_");
		ܐܢ (!e || sscanf(e, "%d", &i) != 1)
			استمر;
		ܐܢ (!(type = strsep(&p, "_=")))
			استمر;
		ܐܢ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_value(p);
			ܐܢ (!e || *e == '\0')
				استمر;
			depinfo = make_depinfo(deptree, e);
			deptype = عدم;
			استمر;
		}
		e = strsep(&p, "=");
		ܐܢ (!e || sscanf(e, "%d", &i) != 1)
			استمر;
		/* Sanity */
		e = get_shell_value(p);
		ܐܢ (!e || *e == '\0')
			استمر;
		ܐܢ (!depinfo)
			استمر;
		ܐܢ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	حرر(line);
	fclose(fp);

	ܦܢܐ deptree;
}

RC_DEPTREE *
rc_deptree_load(فراغ)
{
	ܦܢܐ deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ثابت حرف *deptree_file)
{
	ܦܢܐ deptree_load_file(AT_FDCWD, deptree_file);
}

ساكن bool
valid_service(ثابت حرف *runlevel, ثابت حرف *service, ثابت حرف *type)
{
	RC_SERVICE state;

	ܐܢ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ܦܢܐ true;

	ܐܢ (rc_service_in_runlevel(service, runlevel))
		ܦܢܐ true;
	ܐܢ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ܦܢܐ false;
	ܐܢ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ܦܢܐ false;
	ܐܢ (strcmp(runlevel, bootlevel) != 0) {
		ܐܢ (rc_service_in_runlevel(service, bootlevel))
			ܦܢܐ true;
	}

	state = rc_service_state(service);
	ܐܢ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ܦܢܐ true;

	ܦܢܐ false;
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
		svc = service->value;
		st = rc_service_state(svc);

		ܐܢ (level)
			ok = rc_service_in_runlevel(svc, level);
		ܐܚܪܢܐ ܐܢ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		ܐܢ (!ok)
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
		ܐܢ (!ok)
			استمر;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	ܦܢܐ retval;
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
	ܐܢ (!dt)
		ܦܢܐ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ܐܢ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->value);
		ܦܢܐ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ܐܢ (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			ܐܢ (rc_service_in_runlevel(service->value, runlevel) ||
			    rc_service_in_runlevel(service->value, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->value) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->value);
		ܐܢ (TAILQ_FIRST(providers))
			ܦܢܐ providers;
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
	ܐܢ (TAILQ_FIRST(providers)) { \
		ܐܢ (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ܦܢܐ providers; \
	}

	/* Anything running has to come first */
	ܐܢ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ܐܢ (get_provided1(runlevel, providers, dt, عدم, true, RC_SERVICE_STARTED))
	{ DO }
	ܐܢ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ܐܢ (get_provided1(runlevel, providers, dt, عدم, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ܐܢ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ܦܢܐ providers;
	ܐܢ (get_provided1(runlevel, providers, dt, عدم, true, RC_SERVICE_STARTING))
		ܦܢܐ providers;
	ܐܢ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ܦܢܐ providers;
	ܐܢ (get_provided1(runlevel, providers, dt, عدم, false, RC_SERVICE_STARTING))
		ܦܢܐ providers;

	/* Nothing started then. OK, lets get the stopped services */
	ܐܢ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ܦܢܐ providers;
	ܐܢ (get_provided1(runlevel, providers, dt, عدم, true, RC_SERVICE_STOPPED))
	{ DO }
	ܐܢ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ܦܢܐ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	ܐܢ (service != عدم)
		rc_stringlist_add(providers, service->value);

	ܦܢܐ providers;
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
		ܐܢ (strcmp(type->value, depinfo->service) == 0)
			ܦܢܐ;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ܐܢ (!(dt = get_deptype(depinfo, type->value)))
			استمر;

		TAILQ_FOREACH(service, dt->services, entries) {
			ܐܢ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->value, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->value);
				استمر;
			}

			ܐܢ (!(di = get_depinfo(deptree, service->value)))
				استمر;
			provided = get_provided(di, runlevel, options);

			ܐܢ (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->value);
					ܐܢ (di && valid_service(runlevel, di->service, type->value))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ܐܚܪܢܐ ܐܢ (di && valid_service(runlevel, service->value, type->value))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ܐܢ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ܐܢ (!(di = get_depinfo(deptree, service->value)))
				استمر;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ܐܢ (strcmp(p->value, depinfo->service) == 0) {
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
	ܐܢ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ܐܢ (!get_deptype(depinfo, "providedby"))
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
	ܐܢ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ܦܢܐ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->value);
	ܦܢܐ svcs;
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
	ܐܢ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ܐܢ (!(di = get_depinfo(deptree, service->value))) {
			errno = ENOENT;
			استمر;
		}
		ܐܢ (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	ܦܢܐ sorted;
}

RC_STRINGLIST *
rc_deptree_order(ثابت RC_DEPTREE *deptree, ثابت حرف *runlevel, صحيح options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ܐܢ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ܐܢ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		حرر(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		حرر(list2);
	} ܐܚܪܢܐ {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		ܐܢ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			حرر(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			حرر(list2);
			/* If we're not the boot runlevel then add that too */
			ܐܢ (strcmp(runlevel, bootlevel) != 0) {
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
	ܦܢܐ services;
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
	ܐܢ (fstatat(target_dir, target, &buf, 0) != 0)
		ܦܢܐ true;

	ܐܢ (newer) {
		ܐܢ (*rel < buf.st_mtime) {
			retval = false;

			ܐܢ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ܐܚܪܢܐ {
		ܐܢ (*rel > buf.st_mtime) {
			retval = false;

			ܐܢ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ܐܢ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		ܦܢܐ retval;

	/* Check all the entries in the dir */
	ܟܡܐ ((d = readdir(dp))) {
		ܐܢ (d->d_name[0] == '.')
			استمر;
		ܐܢ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ܦܢܐ retval;
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
	ܐܢ (stat(source, &buf) != 0)
		ܦܢܐ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	ܐܢ (rel) {
		*rel = mtime;
	}
	ܦܢܐ retval;
}

bool
rc_newer_than(ثابت حرف *source, ثابت حرف *target, time_t *newest, حرف *file)
{

	ܦܢܐ mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ثابت حرف *source, ثابت حرف *target, time_t *oldest, حرف *file)
{
	ܦܢܐ mtime_check(AT_FDCWD, source, target, false, oldest, file);
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
	ܐܢ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ܐܢ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ܐܚܪܢܐ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(عدم);
	}

	لكل (size_t i = 0, count = rc_scriptdirfds(&dirfds); i < count; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ܐܢ (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->value, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	ܐܢ ((newer) && (newest != عدم)) {
	    *newest = mtime;
	}

	ܦܢܐ newer;
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
		ܐܢ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	حرر(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ܐܢ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ܐܢ (uname(&uts) == 0)
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
	حرف *line = عدم;
	size_t size;
	حرف *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ثابت حرف *sys = rc_sys();
	صحيح serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	ܐܢ (!(fp = popen(GENDEP, "r")))
		ܦܢܐ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ܟܡܐ (xgetline(&line, &size, fp) != -1) {
		depends = line;
		service = strsep(&depends, " ");
		ܐܢ (!service || !*service)
			استمر;

		type = strsep(&depends, " ");
		ܐܢ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = عدم;
			depinfo = get_depinfo(deptree, service);
			ܐܢ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ܐܢ (!type || !depends)
			استمر;

		/* Get the type */
		ܐܢ (strcmp(type, "config") != 0) {
			ܐܢ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ܐܢ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ܟܡܐ ((depend = strsep(&depends, " "))) {
			ܐܢ (depend[0] == 0)
				استمر;

			ܐܢ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				استمر;
			}

			/* Don't depend on ourself */
			ܐܢ (strcmp(depend, service) == 0)
				استمر;

			/* .sh files are not init scripts */
			l = strlen(depend);
			ܐܢ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				استمر;

			/* Remove our dependency if instructed */
			ܐܢ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				استمر;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ܐܢ (strcmp(type, "ibefore") == 0) {
				ܐܢ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ܐܢ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ܐܢ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	حرر(line);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ܐܢ (sys) {
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
			ܐܢ (!(deptype = get_deptype(depinfo, "keyword")))
				استمر;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ܐܢ (strcmp(s->value, nosys) != 0 && strcmp(s->value, onosys) != 0)
					استمر;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ܐܢ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->value);
						ܐܢ (!TAILQ_FIRST(dt->services)) {
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
		ܐܢ (!(deptype = get_deptype(depinfo, "iprovide")))
			استمر;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->value);
			ܐܢ (!di)
				di = make_depinfo(providers, s->value);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	حرر(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		لكل (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ܐܢ (!deptype)
				استمر;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->value);
				ܐܢ (!di) {
					ܐܢ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->value);
						dt = get_deptype(depinfo, "broken");
						ܐܢ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->value);
					}
					استمر;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ܐܢ (!dt)
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
		ܐܢ (!deptype)
			استمر;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      عدم, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->value);
				ܐܢ (!di)
					استمر;
				ܐܢ (strcmp(s2->value, s3->value) == 0) {
					dt = get_deptype(di, "iafter");
					ܐܢ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					اكسر;
				}
				dt = get_deptype(di, "iprovide");
				ܐܢ (!dt)
					استمر;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ܐܢ (strcmp(s4->value, s2->value) == 0)
						اكسر;
				}
				ܐܢ (s4) {
					di = get_depinfo(deptree, s4->value);
					ܐܢ (di) {
						dt = get_deptype(di, "iafter");
						ܐܢ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					اكسر;
				}
			}
			ܐܢ (s3)
				rc_stringlist_delete(deptype->services, s2->value);
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
		ܐܢ (errno == EEXIST) {
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
	ܐܢ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->value);
				}
			}
			i++;
		}
		fclose(fp);
	} ܐܚܪܢܐ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ܐܢ (TAILQ_FIRST(config)) {
		ܐܢ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->value);
			fclose(fp);
		} ܐܚܪܢܐ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ܐܚܪܢܐ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ܦܢܐ retval;
}
