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

#અન્તર્ભાવ <sys/utsname.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "queue.h"
#અન્તર્ભાવ "librc.h"
#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

સ્થિર અચલ અક્ષર *bootlevel = ખાલી;

સ્થિર અક્ષર *
get_shell_કિંમત(અક્ષર *string)
{
	અક્ષર *p = string;
	અક્ષર *e;

	જો (!string)
		પાછા ખાલી;

	જો (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	જો (*e == '\n')
		*e-- = 0;
	જો (*e == '\'')
		*e-- = 0;

	જો (*p != 0)
		પાછા p;

	પાછા ખાલી;
}

શૂન્ય
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	જો (!deptree)
		પાછા;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			મુક્ત(dt->type);
			મુક્ત(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		મુક્ત(di->service);
		મુક્ત(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	મુક્ત(deptree);
}

સ્થિર RC_DEPINFO *
get_depinfo(અચલ RC_DEPTREE *deptree, અચલ અક્ષર *service)
{
	RC_DEPINFO *di;
	જો (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			જો (strcmp(di->service, service) == 0)
				પાછા di;
	}
	પાછા ખાલી;
}

સ્થિર RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, અચલ અક્ષર *service)
{
	RC_DEPINFO *depinfo = xmalloc(કદ(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_ઉમેરો_TAIL(deptree, depinfo, entries);

	પાછા depinfo;
}

સ્થિર RC_DEPTYPE *
get_deptype(અચલ RC_DEPINFO *depinfo, અચલ અક્ષર *type)
{
	RC_DEPTYPE *dt;

	જો (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			જો (strcmp(dt->type, type) == 0)
				પાછા dt;
	}
	પાછા ખાલી;
}

સ્થિર RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, અચલ અક્ષર *type)
{
	RC_DEPTYPE *deptype = xmalloc(કદ(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_ઉમેરો_TAIL(&depinfo->depends, deptype, entries);

	પાછા deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
સ્થિર RC_DEPTREE *
make_deptree(શૂન્ય) {
	RC_DEPTREE *deptree = xmalloc(કદ(*deptree));
	TAILQ_INIT(deptree);
	પાછા deptree;
}

સ્થિર RC_DEPTREE *
deptree_load_file(પૂર્ણાંક dirfd, અચલ અક્ષર *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ખાલી;
	RC_DEPTYPE *deptype = ખાલી;
	અક્ષર *લીટી = ખાલી;
	size_t size;
	અક્ષર *type;
	અક્ષર *p;
	અક્ષર *e;
	પૂર્ણાંક i;
	FILE *fp;

	જો (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		પાછા ખાલી;

	deptree = make_deptree();
	જ્યારે (xgetline(&લીટી, &size, fp) != -1) {
		p = લીટી;
		e = strsep(&p, "_");
		જો (!e || strcmp(e, "depinfo") != 0)
			ચાલુ;
		e = strsep(&p, "_");
		જો (!e || sscanf(e, "%d", &i) != 1)
			ચાલુ;
		જો (!(type = strsep(&p, "_=")))
			ચાલુ;
		જો (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_કિંમત(p);
			જો (!e || *e == '\0')
				ચાલુ;
			depinfo = make_depinfo(deptree, e);
			deptype = ખાલી;
			ચાલુ;
		}
		e = strsep(&p, "=");
		જો (!e || sscanf(e, "%d", &i) != 1)
			ચાલુ;
		/* Sanity */
		e = get_shell_કિંમત(p);
		જો (!e || *e == '\0')
			ચાલુ;
		જો (!depinfo)
			ચાલુ;
		જો (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	મુક્ત(લીટી);
	fclose(fp);

	પાછા deptree;
}

RC_DEPTREE *
rc_deptree_load(શૂન્ય)
{
	પાછા deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(અચલ અક્ષર *deptree_file)
{
	પાછા deptree_load_file(AT_FDCWD, deptree_file);
}

સ્થિર bool
valid_service(અચલ અક્ષર *runlevel, અચલ અક્ષર *service, અચલ અક્ષર *type)
{
	RC_SERVICE state;

	જો (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		પાછા true;

	જો (rc_service_in_runlevel(service, runlevel))
		પાછા true;
	જો (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    પાછા false;
	જો (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    પાછા false;
	જો (strcmp(runlevel, bootlevel) != 0) {
		જો (rc_service_in_runlevel(service, bootlevel))
			પાછા true;
	}

	state = rc_service_state(service);
	જો (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		પાછા true;

	પાછા false;
}

સ્થિર bool
get_provided1(અચલ અક્ષર *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, અચલ અક્ષર *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	અચલ અક્ષર *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->કિંમત;
		st = rc_service_state(svc);

		જો (level)
			ok = rc_service_in_runlevel(svc, level);
		નહીંતર જો (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		જો (!ok)
			ચાલુ;
		પસંદગી (state) {
			સ્થિતિ RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				તોડો;
			સ્થિતિ RC_SERVICE_INACTIVE:
			સ્થિતિ RC_SERVICE_STARTING:
			સ્થિતિ RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				તોડો;
			મૂળભૂત:
				તોડો;
		}
		જો (!ok)
			ચાલુ;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	પાછા retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
સ્થિર RC_STRINGLIST *
get_provided(અચલ RC_DEPINFO *depinfo, અચલ અક્ષર *runlevel, પૂર્ણાંક options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	જો (!dt)
		પાછા providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	જો (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->કિંમત);
		પાછા providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	જો (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			જો (rc_service_in_runlevel(service->કિંમત, runlevel) ||
			    rc_service_in_runlevel(service->કિંમત, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->કિંમત) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->કિંમત);
		જો (TAILQ_FIRST(providers))
			પાછા providers;
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
	જો (TAILQ_FIRST(providers)) { \
		જો (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		પાછા providers; \
	}

	/* Anything running has to come first */
	જો (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	જો (get_provided1(runlevel, providers, dt, ખાલી, true, RC_SERVICE_STARTED))
	{ DO }
	જો (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	જો (get_provided1(runlevel, providers, dt, ખાલી, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	જો (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		પાછા providers;
	જો (get_provided1(runlevel, providers, dt, ખાલી, true, RC_SERVICE_STARTING))
		પાછા providers;
	જો (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    પાછા providers;
	જો (get_provided1(runlevel, providers, dt, ખાલી, false, RC_SERVICE_STARTING))
		પાછા providers;

	/* Nothing started then. OK, lets get the stopped services */
	જો (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		પાછા providers;
	જો (get_provided1(runlevel, providers, dt, ખાલી, true, RC_SERVICE_STOPPED))
	{ DO }
	જો (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		પાછા providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	જો (service != ખાલી)
		rc_stringlist_add(providers, service->કિંમત);

	પાછા providers;
}

સ્થિર શૂન્ય
visit_service(અચલ RC_DEPTREE *deptree,
	      અચલ RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      અચલ RC_DEPINFO *depinfo,
	      અચલ અક્ષર *runlevel, પૂર્ણાંક options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	અચલ અક્ષર *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		જો (strcmp(type->કિંમત, depinfo->service) == 0)
			પાછા;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		જો (!(dt = get_deptype(depinfo, type->કિંમત)))
			ચાલુ;

		TAILQ_FOREACH(service, dt->services, entries) {
			જો (!(options & RC_DEP_TRACE) ||
			    strcmp(type->કિંમત, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->કિંમત);
				ચાલુ;
			}

			જો (!(di = get_depinfo(deptree, service->કિંમત)))
				ચાલુ;
			provided = get_provided(di, runlevel, options);

			જો (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->કિંમત);
					જો (di && valid_service(runlevel, di->service, type->કિંમત))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			નહીંતર જો (di && valid_service(runlevel, service->કિંમત, type->કિંમત))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	જો (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			જો (!(di = get_depinfo(deptree, service->કિંમત)))
				ચાલુ;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				જો (strcmp(p->કિંમત, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					તોડો;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	જો (!svcname || strcmp(svcname, depinfo->service) != 0) {
		જો (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(અચલ RC_DEPTREE *deptree,
		  અચલ અક્ષર *service, અચલ અક્ષર *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	જો (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		પાછા svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->કિંમત);
	પાછા svcs;
}

RC_STRINGLIST *
rc_deptree_depends(અચલ RC_DEPTREE *deptree,
		   અચલ RC_STRINGLIST *types,
		   અચલ RC_STRINGLIST *services,
		   અચલ અક્ષર *runlevel, પૂર્ણાંક options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	અચલ RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	જો (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		જો (!(di = get_depinfo(deptree, service->કિંમત))) {
			errno = ENOENT;
			ચાલુ;
		}
		જો (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	પાછા sorted;
}

RC_STRINGLIST *
rc_deptree_order(અચલ RC_DEPTREE *deptree, અચલ અક્ષર *runlevel, પૂર્ણાંક options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	જો (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	જો (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		મુક્ત(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		મુક્ત(list2);
	} નહીંતર {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		જો (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			મુક્ત(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			મુક્ત(list2);
			/* If we're not the boot runlevel then add that too */
			જો (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				મુક્ત(list2);
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
	પાછા services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
સ્થિર bool
deep_mtime_check(પૂર્ણાંક target_dir, અચલ અક્ષર *target, bool newer, time_t *rel, અક્ષર *file)
{
	સંરચના stat buf;
	bool retval = true;
	DIR *dp;
	સંરચના dirent *d;

	/* If target does not exist, return true to mimic shell test */
	જો (fstatat(target_dir, target, &buf, 0) != 0)
		પાછા true;

	જો (newer) {
		જો (*rel < buf.st_mtime) {
			retval = false;

			જો (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} નહીંતર {
		જો (*rel > buf.st_mtime) {
			retval = false;

			જો (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	જો (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		પાછા retval;

	/* Check all the entries in the dir */
	જ્યારે ((d = readdir(dp))) {
		જો (d->d_name[0] == '.')
			ચાલુ;
		જો (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	પાછા retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
સ્થિર bool
mtime_check(પૂર્ણાંક dirfd, અચલ અક્ષર *source, અચલ અક્ષર *target, bool newer, time_t *rel, અક્ષર *file)
{
	સંરચના stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	જો (stat(source, &buf) != 0)
		પાછા false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	જો (rel) {
		*rel = mtime;
	}
	પાછા retval;
}

bool
rc_newer_than(અચલ અક્ષર *source, અચલ અક્ષર *target, time_t *newest, અક્ષર *file)
{

	પાછા mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(અચલ અક્ષર *source, અચલ અક્ષર *target, time_t *oldest, અક્ષર *file)
{
	પાછા mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

પ્રકારવ્યાખ્યા સંરચના deppair
{
	અચલ અક્ષર *depend;
	અચલ અક્ષર *addto;
} DEPPAIR;

સ્થિર અચલ DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ખાલી, ખાલી }
};

bool
rc_deptree_update_needed(time_t *newest, અક્ષર *file)
{
	bool newer = false;
	અચલ પૂર્ણાંક *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	સંરચના stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	જો (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	જો (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} નહીંતર {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ખાલી);
	}

	માટે (size_t i = 0, ગણતરી = rc_scriptdirfds(&dirfds); i < ગણતરી; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	જો (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->કિંમત, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	જો ((newer) && (newest != ખાલી)) {
	    *newest = mtime;
	}

	પાછા newer;
}

સ્થિર શૂન્ય
setup_environment(શૂન્ય)
{
	સંરચના utsname uts;
	size_t env_size;
	અક્ષર *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	માટે (અચલ અક્ષર * અચલ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		જો (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	મુક્ત(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	જો (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	જો (uname(&uts) == 0)
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
rc_deptree_update(શૂન્ય)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = ખાલી, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ખાલી, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	અક્ષર *લીટી = ખાલી;
	size_t size;
	અક્ષર *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	અચલ અક્ષર *sys = rc_sys();
	પૂર્ણાંક serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	જો (!(fp = popen(GENDEP, "r")))
		પાછા false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	જ્યારે (xgetline(&લીટી, &size, fp) != -1) {
		depends = લીટી;
		service = strsep(&depends, " ");
		જો (!service || !*service)
			ચાલુ;

		type = strsep(&depends, " ");
		જો (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ખાલી;
			depinfo = get_depinfo(deptree, service);
			જો (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		જો (!type || !depends)
			ચાલુ;

		/* Get the type */
		જો (strcmp(type, "config") != 0) {
			જો (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				જો (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		જ્યારે ((depend = strsep(&depends, " "))) {
			જો (depend[0] == 0)
				ચાલુ;

			જો (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				ચાલુ;
			}

			/* Don't depend on ourself */
			જો (strcmp(depend, service) == 0)
				ચાલુ;

			/* .sh files are not init scripts */
			l = strlen(depend);
			જો (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				ચાલુ;

			/* Remove our dependency if instructed */
			જો (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				ચાલુ;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			જો (strcmp(type, "ibefore") == 0) {
				જો ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			જો (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				જો ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	મુક્ત(લીટી);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	જો (sys) {
		અક્ષર *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		માટે (i = 0; i < len; i++)
			nosys[i + 1] = (અક્ષર)tolower((unsigned અક્ષર)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		માટે (i = 0; i < len; i++)
			onosys[i + 2] = (અક્ષર)tolower((unsigned અક્ષર)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			જો (!(deptype = get_deptype(depinfo, "keyword")))
				ચાલુ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				જો (strcmp(s->કિંમત, nosys) != 0 && strcmp(s->કિંમત, onosys) != 0)
					ચાલુ;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						જો (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->કિંમત);
						જો (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							મુક્ત(dt->type);
							મુક્ત(dt->services);
							મુક્ત(dt);
						}
					}
				}
			}
		}
		મુક્ત(nosys);
		મુક્ત(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(કદ(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		જો (!(deptype = get_deptype(depinfo, "iprovide")))
			ચાલુ;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->કિંમત);
			જો (!di)
				di = make_depinfo(providers, s->કિંમત);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	મુક્ત(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		માટે (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			જો (!deptype)
				ચાલુ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->કિંમત);
				જો (!di) {
					જો (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->કિંમત);
						dt = get_deptype(depinfo, "broken");
						જો (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->કિંમત);
					}
					ચાલુ;
				}

				dt = get_deptype(di, deppairs[i].addto);
				જો (!dt)
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
		જો (!deptype)
			ચાલુ;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      ખાલી, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->કિંમત);
				જો (!di)
					ચાલુ;
				જો (strcmp(s2->કિંમત, s3->કિંમત) == 0) {
					dt = get_deptype(di, "iafter");
					જો (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					તોડો;
				}
				dt = get_deptype(di, "iprovide");
				જો (!dt)
					ચાલુ;
				TAILQ_FOREACH(s4, dt->services, entries) {
					જો (strcmp(s4->કિંમત, s2->કિંમત) == 0)
						તોડો;
				}
				જો (s4) {
					di = get_depinfo(deptree, s4->કિંમત);
					જો (di) {
						dt = get_deptype(di, "iafter");
						જો (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					તોડો;
				}
			}
			જો (s3)
				rc_stringlist_delete(deptype->services, s2->કિંમત);
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
		જો (errno == EEXIST) {
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
	જો ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->કિંમત);
				}
			}
			i++;
		}
		fclose(fp);
	} નહીંતર {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	જો (TAILQ_FIRST(config)) {
		જો ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->કિંમત);
			fclose(fp);
		} નહીંતર {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} નહીંતર {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	પાછા retval;
}
