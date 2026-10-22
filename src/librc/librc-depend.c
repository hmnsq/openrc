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

#അന്തര്ഭാവ <sys/utsname.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "queue.h"
#അന്തര്ഭാവ "librc.h"
#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

സ്ഥിതം സ്ഥിരം അക്ഷരം *bootlevel = ശൂന്യ;

സ്ഥിതം അക്ഷരം *
get_shell_മൂല്യം(അക്ഷരം *string)
{
	അക്ഷരം *p = string;
	അക്ഷരം *e;

	എങ്കിൽ (!string)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	എങ്കിൽ (*e == '\n')
		*e-- = 0;
	എങ്കിൽ (*e == '\'')
		*e-- = 0;

	എങ്കിൽ (*p != 0)
		തിരികെ p;

	തിരികെ ശൂന്യ;
}

ശൂന്യം
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	എങ്കിൽ (!deptree)
		തിരികെ;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			സ്വതന്ത്രം(dt->type);
			സ്വതന്ത്രം(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		സ്വതന്ത്രം(di->service);
		സ്വതന്ത്രം(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	സ്വതന്ത്രം(deptree);
}

സ്ഥിതം RC_DEPINFO *
get_depinfo(സ്ഥിരം RC_DEPTREE *deptree, സ്ഥിരം അക്ഷരം *service)
{
	RC_DEPINFO *di;
	എങ്കിൽ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			എങ്കിൽ (strcmp(di->service, service) == 0)
				തിരികെ di;
	}
	തിരികെ ശൂന്യ;
}

സ്ഥിതം RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, സ്ഥിരം അക്ഷരം *service)
{
	RC_DEPINFO *depinfo = xmalloc(വലുപ്പം(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_തിരുകുക_TAIL(deptree, depinfo, entries);

	തിരികെ depinfo;
}

സ്ഥിതം RC_DEPTYPE *
get_deptype(സ്ഥിരം RC_DEPINFO *depinfo, സ്ഥിരം അക്ഷരം *type)
{
	RC_DEPTYPE *dt;

	എങ്കിൽ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			എങ്കിൽ (strcmp(dt->type, type) == 0)
				തിരികെ dt;
	}
	തിരികെ ശൂന്യ;
}

സ്ഥിതം RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, സ്ഥിരം അക്ഷരം *type)
{
	RC_DEPTYPE *deptype = xmalloc(വലുപ്പം(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_തിരുകുക_TAIL(&depinfo->depends, deptype, entries);

	തിരികെ deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
സ്ഥിതം RC_DEPTREE *
make_deptree(ശൂന്യം) {
	RC_DEPTREE *deptree = xmalloc(വലുപ്പം(*deptree));
	TAILQ_INIT(deptree);
	തിരികെ deptree;
}

സ്ഥിതം RC_DEPTREE *
deptree_load_file(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ശൂന്യ;
	RC_DEPTYPE *deptype = ശൂന്യ;
	അക്ഷരം *വരി = ശൂന്യ;
	size_t size;
	അക്ഷരം *type;
	അക്ഷരം *p;
	അക്ഷരം *e;
	പൂർണ്ണസംഖ്യ i;
	FILE *fp;

	എങ്കിൽ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		തിരികെ ശൂന്യ;

	deptree = make_deptree();
	ആകുമ്പോൾ (xgetline(&വരി, &size, fp) != -1) {
		p = വരി;
		e = strsep(&p, "_");
		എങ്കിൽ (!e || strcmp(e, "depinfo") != 0)
			തുടരുക;
		e = strsep(&p, "_");
		എങ്കിൽ (!e || sscanf(e, "%d", &i) != 1)
			തുടരുക;
		എങ്കിൽ (!(type = strsep(&p, "_=")))
			തുടരുക;
		എങ്കിൽ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_മൂല്യം(p);
			എങ്കിൽ (!e || *e == '\0')
				തുടരുക;
			depinfo = make_depinfo(deptree, e);
			deptype = ശൂന്യ;
			തുടരുക;
		}
		e = strsep(&p, "=");
		എങ്കിൽ (!e || sscanf(e, "%d", &i) != 1)
			തുടരുക;
		/* Sanity */
		e = get_shell_മൂല്യം(p);
		എങ്കിൽ (!e || *e == '\0')
			തുടരുക;
		എങ്കിൽ (!depinfo)
			തുടരുക;
		എങ്കിൽ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	സ്വതന്ത്രം(വരി);
	fclose(fp);

	തിരികെ deptree;
}

RC_DEPTREE *
rc_deptree_load(ശൂന്യം)
{
	തിരികെ deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(സ്ഥിരം അക്ഷരം *deptree_file)
{
	തിരികെ deptree_load_file(AT_FDCWD, deptree_file);
}

സ്ഥിതം bool
valid_service(സ്ഥിരം അക്ഷരം *runlevel, സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *type)
{
	RC_SERVICE state;

	എങ്കിൽ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		തിരികെ true;

	എങ്കിൽ (rc_service_in_runlevel(service, runlevel))
		തിരികെ true;
	എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    തിരികെ false;
	എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    തിരികെ false;
	എങ്കിൽ (strcmp(runlevel, bootlevel) != 0) {
		എങ്കിൽ (rc_service_in_runlevel(service, bootlevel))
			തിരികെ true;
	}

	state = rc_service_state(service);
	എങ്കിൽ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		തിരികെ true;

	തിരികെ false;
}

സ്ഥിതം bool
get_provided1(സ്ഥിരം അക്ഷരം *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, സ്ഥിരം അക്ഷരം *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	സ്ഥിരം അക്ഷരം *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->മൂല്യം;
		st = rc_service_state(svc);

		എങ്കിൽ (level)
			ok = rc_service_in_runlevel(svc, level);
		അല്ലെങ്കിൽ എങ്കിൽ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		എങ്കിൽ (!ok)
			തുടരുക;
		തിരഞ്ഞെടുപ്പ് (state) {
			അവസ്ഥ RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				നിർത്തുക;
			അവസ്ഥ RC_SERVICE_INACTIVE:
			അവസ്ഥ RC_SERVICE_STARTING:
			അവസ്ഥ RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
				നിർത്തുക;
		}
		എങ്കിൽ (!ok)
			തുടരുക;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	തിരികെ retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
സ്ഥിതം RC_STRINGLIST *
get_provided(സ്ഥിരം RC_DEPINFO *depinfo, സ്ഥിരം അക്ഷരം *runlevel, പൂർണ്ണസംഖ്യ options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	എങ്കിൽ (!dt)
		തിരികെ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	എങ്കിൽ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->മൂല്യം);
		തിരികെ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	എങ്കിൽ (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			എങ്കിൽ (rc_service_in_runlevel(service->മൂല്യം, runlevel) ||
			    rc_service_in_runlevel(service->മൂല്യം, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->മൂല്യം) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->മൂല്യം);
		എങ്കിൽ (TAILQ_FIRST(providers))
			തിരികെ providers;
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
	എങ്കിൽ (TAILQ_FIRST(providers)) { \
		എങ്കിൽ (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		തിരികെ providers; \
	}

	/* Anything running has to come first */
	എങ്കിൽ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	എങ്കിൽ (get_provided1(runlevel, providers, dt, ശൂന്യ, true, RC_SERVICE_STARTED))
	{ DO }
	എങ്കിൽ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	എങ്കിൽ (get_provided1(runlevel, providers, dt, ശൂന്യ, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	എങ്കിൽ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		തിരികെ providers;
	എങ്കിൽ (get_provided1(runlevel, providers, dt, ശൂന്യ, true, RC_SERVICE_STARTING))
		തിരികെ providers;
	എങ്കിൽ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    തിരികെ providers;
	എങ്കിൽ (get_provided1(runlevel, providers, dt, ശൂന്യ, false, RC_SERVICE_STARTING))
		തിരികെ providers;

	/* Nothing started then. OK, lets get the stopped services */
	എങ്കിൽ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		തിരികെ providers;
	എങ്കിൽ (get_provided1(runlevel, providers, dt, ശൂന്യ, true, RC_SERVICE_STOPPED))
	{ DO }
	എങ്കിൽ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		തിരികെ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	എങ്കിൽ (service != ശൂന്യ)
		rc_stringlist_add(providers, service->മൂല്യം);

	തിരികെ providers;
}

സ്ഥിതം ശൂന്യം
visit_service(സ്ഥിരം RC_DEPTREE *deptree,
	      സ്ഥിരം RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      സ്ഥിരം RC_DEPINFO *depinfo,
	      സ്ഥിരം അക്ഷരം *runlevel, പൂർണ്ണസംഖ്യ options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	സ്ഥിരം അക്ഷരം *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		എങ്കിൽ (strcmp(type->മൂല്യം, depinfo->service) == 0)
			തിരികെ;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		എങ്കിൽ (!(dt = get_deptype(depinfo, type->മൂല്യം)))
			തുടരുക;

		TAILQ_FOREACH(service, dt->services, entries) {
			എങ്കിൽ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->മൂല്യം, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->മൂല്യം);
				തുടരുക;
			}

			എങ്കിൽ (!(di = get_depinfo(deptree, service->മൂല്യം)))
				തുടരുക;
			provided = get_provided(di, runlevel, options);

			എങ്കിൽ (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->മൂല്യം);
					എങ്കിൽ (di && valid_service(runlevel, di->service, type->മൂല്യം))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			അല്ലെങ്കിൽ എങ്കിൽ (di && valid_service(runlevel, service->മൂല്യം, type->മൂല്യം))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	എങ്കിൽ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			എങ്കിൽ (!(di = get_depinfo(deptree, service->മൂല്യം)))
				തുടരുക;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				എങ്കിൽ (strcmp(p->മൂല്യം, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					നിർത്തുക;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	എങ്കിൽ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		എങ്കിൽ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(സ്ഥിരം RC_DEPTREE *deptree,
		  സ്ഥിരം അക്ഷരം *service, സ്ഥിരം അക്ഷരം *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	എങ്കിൽ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		തിരികെ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->മൂല്യം);
	തിരികെ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(സ്ഥിരം RC_DEPTREE *deptree,
		   സ്ഥിരം RC_STRINGLIST *types,
		   സ്ഥിരം RC_STRINGLIST *services,
		   സ്ഥിരം അക്ഷരം *runlevel, പൂർണ്ണസംഖ്യ options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	സ്ഥിരം RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	എങ്കിൽ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		എങ്കിൽ (!(di = get_depinfo(deptree, service->മൂല്യം))) {
			errno = ENOENT;
			തുടരുക;
		}
		എങ്കിൽ (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	തിരികെ sorted;
}

RC_STRINGLIST *
rc_deptree_order(സ്ഥിരം RC_DEPTREE *deptree, സ്ഥിരം അക്ഷരം *runlevel, പൂർണ്ണസംഖ്യ options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	എങ്കിൽ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		സ്വതന്ത്രം(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		സ്വതന്ത്രം(list2);
	} അല്ലെങ്കിൽ {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		എങ്കിൽ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			സ്വതന്ത്രം(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			സ്വതന്ത്രം(list2);
			/* If we're not the boot runlevel then add that too */
			എങ്കിൽ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				സ്വതന്ത്രം(list2);
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
	തിരികെ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
സ്ഥിതം bool
deep_mtime_check(പൂർണ്ണസംഖ്യ target_dir, സ്ഥിരം അക്ഷരം *target, bool newer, time_t *rel, അക്ഷരം *file)
{
	ഘടന stat buf;
	bool retval = true;
	DIR *dp;
	ഘടന dirent *d;

	/* If target does not exist, return true to mimic shell test */
	എങ്കിൽ (fstatat(target_dir, target, &buf, 0) != 0)
		തിരികെ true;

	എങ്കിൽ (newer) {
		എങ്കിൽ (*rel < buf.st_mtime) {
			retval = false;

			എങ്കിൽ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (*rel > buf.st_mtime) {
			retval = false;

			എങ്കിൽ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	എങ്കിൽ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		തിരികെ retval;

	/* Check all the entries in the dir */
	ആകുമ്പോൾ ((d = readdir(dp))) {
		എങ്കിൽ (d->d_name[0] == '.')
			തുടരുക;
		എങ്കിൽ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	തിരികെ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
സ്ഥിതം bool
mtime_check(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *source, സ്ഥിരം അക്ഷരം *target, bool newer, time_t *rel, അക്ഷരം *file)
{
	ഘടന stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	എങ്കിൽ (stat(source, &buf) != 0)
		തിരികെ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	എങ്കിൽ (rel) {
		*rel = mtime;
	}
	തിരികെ retval;
}

bool
rc_newer_than(സ്ഥിരം അക്ഷരം *source, സ്ഥിരം അക്ഷരം *target, time_t *newest, അക്ഷരം *file)
{

	തിരികെ mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(സ്ഥിരം അക്ഷരം *source, സ്ഥിരം അക്ഷരം *target, time_t *oldest, അക്ഷരം *file)
{
	തിരികെ mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

തരനിർവചനം ഘടന deppair
{
	സ്ഥിരം അക്ഷരം *depend;
	സ്ഥിരം അക്ഷരം *addto;
} DEPPAIR;

സ്ഥിതം സ്ഥിരം DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ശൂന്യ, ശൂന്യ }
};

bool
rc_deptree_update_needed(time_t *newest, അക്ഷരം *file)
{
	bool newer = false;
	സ്ഥിരം പൂർണ്ണസംഖ്യ *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ഘടന stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	എങ്കിൽ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	എങ്കിൽ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} അല്ലെങ്കിൽ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ശൂന്യ);
	}

	വേണ്ടി (size_t i = 0, എണ്ണം = rc_scriptdirfds(&dirfds); i < എണ്ണം; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	എങ്കിൽ (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->മൂല്യം, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	എങ്കിൽ ((newer) && (newest != ശൂന്യ)) {
	    *newest = mtime;
	}

	തിരികെ newer;
}

സ്ഥിതം ശൂന്യം
setup_environment(ശൂന്യം)
{
	ഘടന utsname uts;
	size_t env_size;
	അക്ഷരം *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	വേണ്ടി (സ്ഥിരം അക്ഷരം * സ്ഥിരം *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		എങ്കിൽ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	സ്വതന്ത്രം(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	എങ്കിൽ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	എങ്കിൽ (uname(&uts) == 0)
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
rc_deptree_update(ശൂന്യം)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = ശൂന്യ, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ശൂന്യ, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	അക്ഷരം *വരി = ശൂന്യ;
	size_t size;
	അക്ഷരം *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	സ്ഥിരം അക്ഷരം *sys = rc_sys();
	പൂർണ്ണസംഖ്യ serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	എങ്കിൽ (!(fp = popen(GENDEP, "r")))
		തിരികെ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ആകുമ്പോൾ (xgetline(&വരി, &size, fp) != -1) {
		depends = വരി;
		service = strsep(&depends, " ");
		എങ്കിൽ (!service || !*service)
			തുടരുക;

		type = strsep(&depends, " ");
		എങ്കിൽ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ശൂന്യ;
			depinfo = get_depinfo(deptree, service);
			എങ്കിൽ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		എങ്കിൽ (!type || !depends)
			തുടരുക;

		/* Get the type */
		എങ്കിൽ (strcmp(type, "config") != 0) {
			എങ്കിൽ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				എങ്കിൽ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ആകുമ്പോൾ ((depend = strsep(&depends, " "))) {
			എങ്കിൽ (depend[0] == 0)
				തുടരുക;

			എങ്കിൽ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				തുടരുക;
			}

			/* Don't depend on ourself */
			എങ്കിൽ (strcmp(depend, service) == 0)
				തുടരുക;

			/* .sh files are not init scripts */
			l = strlen(depend);
			എങ്കിൽ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				തുടരുക;

			/* Remove our dependency if instructed */
			എങ്കിൽ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				തുടരുക;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			എങ്കിൽ (strcmp(type, "ibefore") == 0) {
				എങ്കിൽ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			എങ്കിൽ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				എങ്കിൽ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	സ്വതന്ത്രം(വരി);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	എങ്കിൽ (sys) {
		അക്ഷരം *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		വേണ്ടി (i = 0; i < len; i++)
			nosys[i + 1] = (അക്ഷരം)tolower((unsigned അക്ഷരം)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		വേണ്ടി (i = 0; i < len; i++)
			onosys[i + 2] = (അക്ഷരം)tolower((unsigned അക്ഷരം)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			എങ്കിൽ (!(deptype = get_deptype(depinfo, "keyword")))
				തുടരുക;
			TAILQ_FOREACH(s, deptype->services, entries) {
				എങ്കിൽ (strcmp(s->മൂല്യം, nosys) != 0 && strcmp(s->മൂല്യം, onosys) != 0)
					തുടരുക;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						എങ്കിൽ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->മൂല്യം);
						എങ്കിൽ (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							സ്വതന്ത്രം(dt->type);
							സ്വതന്ത്രം(dt->services);
							സ്വതന്ത്രം(dt);
						}
					}
				}
			}
		}
		സ്വതന്ത്രം(nosys);
		സ്വതന്ത്രം(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(വലുപ്പം(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		എങ്കിൽ (!(deptype = get_deptype(depinfo, "iprovide")))
			തുടരുക;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->മൂല്യം);
			എങ്കിൽ (!di)
				di = make_depinfo(providers, s->മൂല്യം);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	സ്വതന്ത്രം(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		വേണ്ടി (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			എങ്കിൽ (!deptype)
				തുടരുക;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->മൂല്യം);
				എങ്കിൽ (!di) {
					എങ്കിൽ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->മൂല്യം);
						dt = get_deptype(depinfo, "broken");
						എങ്കിൽ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->മൂല്യം);
					}
					തുടരുക;
				}

				dt = get_deptype(di, deppairs[i].addto);
				എങ്കിൽ (!dt)
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
		എങ്കിൽ (!deptype)
			തുടരുക;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      ശൂന്യ, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->മൂല്യം);
				എങ്കിൽ (!di)
					തുടരുക;
				എങ്കിൽ (strcmp(s2->മൂല്യം, s3->മൂല്യം) == 0) {
					dt = get_deptype(di, "iafter");
					എങ്കിൽ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					നിർത്തുക;
				}
				dt = get_deptype(di, "iprovide");
				എങ്കിൽ (!dt)
					തുടരുക;
				TAILQ_FOREACH(s4, dt->services, entries) {
					എങ്കിൽ (strcmp(s4->മൂല്യം, s2->മൂല്യം) == 0)
						നിർത്തുക;
				}
				എങ്കിൽ (s4) {
					di = get_depinfo(deptree, s4->മൂല്യം);
					എങ്കിൽ (di) {
						dt = get_deptype(di, "iafter");
						എങ്കിൽ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					നിർത്തുക;
				}
			}
			എങ്കിൽ (s3)
				rc_stringlist_delete(deptype->services, s2->മൂല്യം);
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
		എങ്കിൽ (errno == EEXIST) {
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
	എങ്കിൽ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->മൂല്യം);
				}
			}
			i++;
		}
		fclose(fp);
	} അല്ലെങ്കിൽ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	എങ്കിൽ (TAILQ_FIRST(config)) {
		എങ്കിൽ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->മൂല്യം);
			fclose(fp);
		} അല്ലെങ്കിൽ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} അല്ലെങ്കിൽ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	തിരികെ retval;
}
