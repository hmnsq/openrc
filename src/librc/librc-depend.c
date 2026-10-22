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

#ಅನ್ತರ್ಭಾವ <sys/utsname.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <dirent.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "queue.h"
#ಅನ್ತರ್ಭಾವ "librc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *bootlevel = ರಿಕ್ತ;

ಸ್ಥಿರ ವರ್ಣ *
get_shell_ಮೌಲ್ಯ(ವರ್ಣ *string)
{
	ವರ್ಣ *p = string;
	ವರ್ಣ *e;

	ಯದಿ (!string)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	ಯದಿ (*e == '\n')
		*e-- = 0;
	ಯದಿ (*e == '\'')
		*e-- = 0;

	ಯದಿ (*p != 0)
		ಪ್ರತ್ಯಾಗಮ p;

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಶೂನ್ಯ
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ಯದಿ (!deptree)
		ಪ್ರತ್ಯಾಗಮ;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			ಮುಕ್ತ(dt->type);
			ಮುಕ್ತ(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		ಮುಕ್ತ(di->service);
		ಮುಕ್ತ(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	ಮುಕ್ತ(deptree);
}

ಸ್ಥಿರ RC_DEPINFO *
get_depinfo(ಧ್ರುವ RC_DEPTREE *deptree, ಧ್ರುವ ವರ್ಣ *service)
{
	RC_DEPINFO *di;
	ಯದಿ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ಯದಿ (strcmp(di->service, service) == 0)
				ಪ್ರತ್ಯಾಗಮ di;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ಧ್ರುವ ವರ್ಣ *service)
{
	RC_DEPINFO *depinfo = xmalloc(ಪರಿಮಾಣ(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_ಸೇರಿಸು_TAIL(deptree, depinfo, entries);

	ಪ್ರತ್ಯಾಗಮ depinfo;
}

ಸ್ಥಿರ RC_DEPTYPE *
get_deptype(ಧ್ರುವ RC_DEPINFO *depinfo, ಧ್ರುವ ವರ್ಣ *type)
{
	RC_DEPTYPE *dt;

	ಯದಿ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ಯದಿ (strcmp(dt->type, type) == 0)
				ಪ್ರತ್ಯಾಗಮ dt;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸ್ಥಿರ RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ಧ್ರುವ ವರ್ಣ *type)
{
	RC_DEPTYPE *deptype = xmalloc(ಪರಿಮಾಣ(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_ಸೇರಿಸು_TAIL(&depinfo->depends, deptype, entries);

	ಪ್ರತ್ಯಾಗಮ deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
ಸ್ಥಿರ RC_DEPTREE *
make_deptree(ಶೂನ್ಯ) {
	RC_DEPTREE *deptree = xmalloc(ಪರಿಮಾಣ(*deptree));
	TAILQ_INIT(deptree);
	ಪ್ರತ್ಯಾಗಮ deptree;
}

ಸ್ಥಿರ RC_DEPTREE *
deptree_load_file(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ರಿಕ್ತ;
	RC_DEPTYPE *deptype = ರಿಕ್ತ;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	size_t size;
	ವರ್ಣ *type;
	ವರ್ಣ *p;
	ವರ್ಣ *e;
	ಪೂರ್ಣಾಙ್ಕ i;
	FILE *fp;

	ಯದಿ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	deptree = make_deptree();
	ಯಾವತ್ (xgetline(&ಸಾಲು, &size, fp) != -1) {
		p = ಸಾಲು;
		e = strsep(&p, "_");
		ಯದಿ (!e || strcmp(e, "depinfo") != 0)
			ಅನುವರ್ತ;
		e = strsep(&p, "_");
		ಯದಿ (!e || sscanf(e, "%d", &i) != 1)
			ಅನುವರ್ತ;
		ಯದಿ (!(type = strsep(&p, "_=")))
			ಅನುವರ್ತ;
		ಯದಿ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_ಮೌಲ್ಯ(p);
			ಯದಿ (!e || *e == '\0')
				ಅನುವರ್ತ;
			depinfo = make_depinfo(deptree, e);
			deptype = ರಿಕ್ತ;
			ಅನುವರ್ತ;
		}
		e = strsep(&p, "=");
		ಯದಿ (!e || sscanf(e, "%d", &i) != 1)
			ಅನುವರ್ತ;
		/* Sanity */
		e = get_shell_ಮೌಲ್ಯ(p);
		ಯದಿ (!e || *e == '\0')
			ಅನುವರ್ತ;
		ಯದಿ (!depinfo)
			ಅನುವರ್ತ;
		ಯದಿ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	ಮುಕ್ತ(ಸಾಲು);
	fclose(fp);

	ಪ್ರತ್ಯಾಗಮ deptree;
}

RC_DEPTREE *
rc_deptree_load(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ಧ್ರುವ ವರ್ಣ *deptree_file)
{
	ಪ್ರತ್ಯಾಗಮ deptree_load_file(AT_FDCWD, deptree_file);
}

ಸ್ಥಿರ bool
valid_service(ಧ್ರುವ ವರ್ಣ *runlevel, ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *type)
{
	RC_SERVICE state;

	ಯದಿ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (rc_service_in_runlevel(service, runlevel))
		ಪ್ರತ್ಯಾಗಮ true;
	ಯದಿ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ಪ್ರತ್ಯಾಗಮ false;
	ಯದಿ (strcmp(runlevel, bootlevel) != 0) {
		ಯದಿ (rc_service_in_runlevel(service, bootlevel))
			ಪ್ರತ್ಯಾಗಮ true;
	}

	state = rc_service_state(service);
	ಯದಿ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ಪ್ರತ್ಯಾಗಮ true;

	ಪ್ರತ್ಯಾಗಮ false;
}

ಸ್ಥಿರ bool
get_provided1(ಧ್ರುವ ವರ್ಣ *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ಧ್ರುವ ವರ್ಣ *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ಧ್ರುವ ವರ್ಣ *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->ಮೌಲ್ಯ;
		st = rc_service_state(svc);

		ಯದಿ (level)
			ok = rc_service_in_runlevel(svc, level);
		ಅನ್ಯಥಾ ಯದಿ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		ಯದಿ (!ok)
			ಅನುವರ್ತ;
		ವಿಕಲ್ಪ (state) {
			ಪಕ್ಷ RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				ಭಙ್ಗ;
			ಪಕ್ಷ RC_SERVICE_INACTIVE:
			ಪಕ್ಷ RC_SERVICE_STARTING:
			ಪಕ್ಷ RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
				ಭಙ್ಗ;
		}
		ಯದಿ (!ok)
			ಅನುವರ್ತ;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	ಪ್ರತ್ಯಾಗಮ retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
ಸ್ಥಿರ RC_STRINGLIST *
get_provided(ಧ್ರುವ RC_DEPINFO *depinfo, ಧ್ರುವ ವರ್ಣ *runlevel, ಪೂರ್ಣಾಙ್ಕ options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	ಯದಿ (!dt)
		ಪ್ರತ್ಯಾಗಮ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ಯದಿ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->ಮೌಲ್ಯ);
		ಪ್ರತ್ಯಾಗಮ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ಯದಿ (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			ಯದಿ (rc_service_in_runlevel(service->ಮೌಲ್ಯ, runlevel) ||
			    rc_service_in_runlevel(service->ಮೌಲ್ಯ, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->ಮೌಲ್ಯ) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->ಮೌಲ್ಯ);
		ಯದಿ (TAILQ_FIRST(providers))
			ಪ್ರತ್ಯಾಗಮ providers;
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
	ಯದಿ (TAILQ_FIRST(providers)) { \
		ಯದಿ (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ಪ್ರತ್ಯಾಗಮ providers; \
	}

	/* Anything running has to come first */
	ಯದಿ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ಯದಿ (get_provided1(runlevel, providers, dt, ರಿಕ್ತ, true, RC_SERVICE_STARTED))
	{ DO }
	ಯದಿ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ಯದಿ (get_provided1(runlevel, providers, dt, ರಿಕ್ತ, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ಯದಿ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ಪ್ರತ್ಯಾಗಮ providers;
	ಯದಿ (get_provided1(runlevel, providers, dt, ರಿಕ್ತ, true, RC_SERVICE_STARTING))
		ಪ್ರತ್ಯಾಗಮ providers;
	ಯದಿ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ಪ್ರತ್ಯಾಗಮ providers;
	ಯದಿ (get_provided1(runlevel, providers, dt, ರಿಕ್ತ, false, RC_SERVICE_STARTING))
		ಪ್ರತ್ಯಾಗಮ providers;

	/* Nothing started then. OK, lets get the stopped services */
	ಯದಿ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ಪ್ರತ್ಯಾಗಮ providers;
	ಯದಿ (get_provided1(runlevel, providers, dt, ರಿಕ್ತ, true, RC_SERVICE_STOPPED))
	{ DO }
	ಯದಿ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ಪ್ರತ್ಯಾಗಮ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	ಯದಿ (service != ರಿಕ್ತ)
		rc_stringlist_add(providers, service->ಮೌಲ್ಯ);

	ಪ್ರತ್ಯಾಗಮ providers;
}

ಸ್ಥಿರ ಶೂನ್ಯ
visit_service(ಧ್ರುವ RC_DEPTREE *deptree,
	      ಧ್ರುವ RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      ಧ್ರುವ RC_DEPINFO *depinfo,
	      ಧ್ರುವ ವರ್ಣ *runlevel, ಪೂರ್ಣಾಙ್ಕ options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ಧ್ರುವ ವರ್ಣ *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		ಯದಿ (strcmp(type->ಮೌಲ್ಯ, depinfo->service) == 0)
			ಪ್ರತ್ಯಾಗಮ;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ಯದಿ (!(dt = get_deptype(depinfo, type->ಮೌಲ್ಯ)))
			ಅನುವರ್ತ;

		TAILQ_FOREACH(service, dt->services, entries) {
			ಯದಿ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->ಮೌಲ್ಯ, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->ಮೌಲ್ಯ);
				ಅನುವರ್ತ;
			}

			ಯದಿ (!(di = get_depinfo(deptree, service->ಮೌಲ್ಯ)))
				ಅನುವರ್ತ;
			provided = get_provided(di, runlevel, options);

			ಯದಿ (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->ಮೌಲ್ಯ);
					ಯದಿ (di && valid_service(runlevel, di->service, type->ಮೌಲ್ಯ))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ಅನ್ಯಥಾ ಯದಿ (di && valid_service(runlevel, service->ಮೌಲ್ಯ, type->ಮೌಲ್ಯ))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ಯದಿ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ಯದಿ (!(di = get_depinfo(deptree, service->ಮೌಲ್ಯ)))
				ಅನುವರ್ತ;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ಯದಿ (strcmp(p->ಮೌಲ್ಯ, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					ಭಙ್ಗ;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	ಯದಿ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ಯದಿ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ಧ್ರುವ RC_DEPTREE *deptree,
		  ಧ್ರುವ ವರ್ಣ *service, ಧ್ರುವ ವರ್ಣ *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	ಯದಿ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ಪ್ರತ್ಯಾಗಮ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->ಮೌಲ್ಯ);
	ಪ್ರತ್ಯಾಗಮ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ಧ್ರುವ RC_DEPTREE *deptree,
		   ಧ್ರುವ RC_STRINGLIST *types,
		   ಧ್ರುವ RC_STRINGLIST *services,
		   ಧ್ರುವ ವರ್ಣ *runlevel, ಪೂರ್ಣಾಙ್ಕ options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	ಧ್ರುವ RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	ಯದಿ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ಯದಿ (!(di = get_depinfo(deptree, service->ಮೌಲ್ಯ))) {
			errno = ENOENT;
			ಅನುವರ್ತ;
		}
		ಯದಿ (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	ಪ್ರತ್ಯಾಗಮ sorted;
}

RC_STRINGLIST *
rc_deptree_order(ಧ್ರುವ RC_DEPTREE *deptree, ಧ್ರುವ ವರ್ಣ *runlevel, ಪೂರ್ಣಾಙ್ಕ options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ಯದಿ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ಯದಿ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		ಮುಕ್ತ(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		ಮುಕ್ತ(list2);
	} ಅನ್ಯಥಾ {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		ಯದಿ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			ಮುಕ್ತ(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			ಮುಕ್ತ(list2);
			/* If we're not the boot runlevel then add that too */
			ಯದಿ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				ಮುಕ್ತ(list2);
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
	ಪ್ರತ್ಯಾಗಮ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ಸ್ಥಿರ bool
deep_mtime_check(ಪೂರ್ಣಾಙ್ಕ target_dir, ಧ್ರುವ ವರ್ಣ *target, bool newer, time_t *rel, ವರ್ಣ *file)
{
	ಸಂರಚನಾ stat buf;
	bool retval = true;
	DIR *dp;
	ಸಂರಚನಾ dirent *d;

	/* If target does not exist, return true to mimic shell test */
	ಯದಿ (fstatat(target_dir, target, &buf, 0) != 0)
		ಪ್ರತ್ಯಾಗಮ true;

	ಯದಿ (newer) {
		ಯದಿ (*rel < buf.st_mtime) {
			retval = false;

			ಯದಿ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ಅನ್ಯಥಾ {
		ಯದಿ (*rel > buf.st_mtime) {
			retval = false;

			ಯದಿ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ಯದಿ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		ಪ್ರತ್ಯಾಗಮ retval;

	/* Check all the entries in the dir */
	ಯಾವತ್ ((d = readdir(dp))) {
		ಯದಿ (d->d_name[0] == '.')
			ಅನುವರ್ತ;
		ಯದಿ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ಪ್ರತ್ಯಾಗಮ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ಸ್ಥಿರ bool
mtime_check(ಪೂರ್ಣಾಙ್ಕ dirfd, ಧ್ರುವ ವರ್ಣ *source, ಧ್ರುವ ವರ್ಣ *target, bool newer, time_t *rel, ವರ್ಣ *file)
{
	ಸಂರಚನಾ stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	ಯದಿ (stat(source, &buf) != 0)
		ಪ್ರತ್ಯಾಗಮ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	ಯದಿ (rel) {
		*rel = mtime;
	}
	ಪ್ರತ್ಯಾಗಮ retval;
}

bool
rc_newer_than(ಧ್ರುವ ವರ್ಣ *source, ಧ್ರುವ ವರ್ಣ *target, time_t *newest, ವರ್ಣ *file)
{

	ಪ್ರತ್ಯಾಗಮ mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ಧ್ರುವ ವರ್ಣ *source, ಧ್ರುವ ವರ್ಣ *target, time_t *oldest, ವರ್ಣ *file)
{
	ಪ್ರತ್ಯಾಗಮ mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ deppair
{
	ಧ್ರುವ ವರ್ಣ *depend;
	ಧ್ರುವ ವರ್ಣ *addto;
} DEPPAIR;

ಸ್ಥಿರ ಧ್ರುವ DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ರಿಕ್ತ, ರಿಕ್ತ }
};

bool
rc_deptree_update_needed(time_t *newest, ವರ್ಣ *file)
{
	bool newer = false;
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ಸಂರಚನಾ stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	ಯದಿ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ಯದಿ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ಅನ್ಯಥಾ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ರಿಕ್ತ);
	}

	ಪ್ರತಿ (size_t i = 0, ಎಣಿಕೆ = rc_scriptdirfds(&dirfds); i < ಎಣಿಕೆ; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ಯದಿ (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->ಮೌಲ್ಯ, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	ಯದಿ ((newer) && (newest != ರಿಕ್ತ)) {
	    *newest = mtime;
	}

	ಪ್ರತ್ಯಾಗಮ newer;
}

ಸ್ಥಿರ ಶೂನ್ಯ
setup_environment(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ utsname uts;
	size_t env_size;
	ವರ್ಣ *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		ಯದಿ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	ಮುಕ್ತ(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ಯದಿ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ಯದಿ (uname(&uts) == 0)
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
rc_deptree_update(ಶೂನ್ಯ)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = ರಿಕ್ತ, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ರಿಕ್ತ, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	ವರ್ಣ *ಸಾಲು = ರಿಕ್ತ;
	size_t size;
	ವರ್ಣ *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ಧ್ರುವ ವರ್ಣ *sys = rc_sys();
	ಪೂರ್ಣಾಙ್ಕ serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	ಯದಿ (!(fp = popen(GENDEP, "r")))
		ಪ್ರತ್ಯಾಗಮ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ಯಾವತ್ (xgetline(&ಸಾಲು, &size, fp) != -1) {
		depends = ಸಾಲು;
		service = strsep(&depends, " ");
		ಯದಿ (!service || !*service)
			ಅನುವರ್ತ;

		type = strsep(&depends, " ");
		ಯದಿ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ರಿಕ್ತ;
			depinfo = get_depinfo(deptree, service);
			ಯದಿ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ಯದಿ (!type || !depends)
			ಅನುವರ್ತ;

		/* Get the type */
		ಯದಿ (strcmp(type, "config") != 0) {
			ಯದಿ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ಯದಿ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ಯಾವತ್ ((depend = strsep(&depends, " "))) {
			ಯದಿ (depend[0] == 0)
				ಅನುವರ್ತ;

			ಯದಿ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				ಅನುವರ್ತ;
			}

			/* Don't depend on ourself */
			ಯದಿ (strcmp(depend, service) == 0)
				ಅನುವರ್ತ;

			/* .sh files are not init scripts */
			l = strlen(depend);
			ಯದಿ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				ಅನುವರ್ತ;

			/* Remove our dependency if instructed */
			ಯದಿ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				ಅನುವರ್ತ;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ಯದಿ (strcmp(type, "ibefore") == 0) {
				ಯದಿ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ಯದಿ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ಯದಿ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	ಮುಕ್ತ(ಸಾಲು);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ಯದಿ (sys) {
		ವರ್ಣ *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		ಪ್ರತಿ (i = 0; i < len; i++)
			nosys[i + 1] = (ವರ್ಣ)tolower((unsigned ವರ್ಣ)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		ಪ್ರತಿ (i = 0; i < len; i++)
			onosys[i + 2] = (ವರ್ಣ)tolower((unsigned ವರ್ಣ)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			ಯದಿ (!(deptype = get_deptype(depinfo, "keyword")))
				ಅನುವರ್ತ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ಯದಿ (strcmp(s->ಮೌಲ್ಯ, nosys) != 0 && strcmp(s->ಮೌಲ್ಯ, onosys) != 0)
					ಅನುವರ್ತ;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ಯದಿ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->ಮೌಲ್ಯ);
						ಯದಿ (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							ಮುಕ್ತ(dt->type);
							ಮುಕ್ತ(dt->services);
							ಮುಕ್ತ(dt);
						}
					}
				}
			}
		}
		ಮುಕ್ತ(nosys);
		ಮುಕ್ತ(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(ಪರಿಮಾಣ(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ಯದಿ (!(deptype = get_deptype(depinfo, "iprovide")))
			ಅನುವರ್ತ;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->ಮೌಲ್ಯ);
			ಯದಿ (!di)
				di = make_depinfo(providers, s->ಮೌಲ್ಯ);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	ಮುಕ್ತ(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ಪ್ರತಿ (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ಯದಿ (!deptype)
				ಅನುವರ್ತ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->ಮೌಲ್ಯ);
				ಯದಿ (!di) {
					ಯದಿ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->ಮೌಲ್ಯ);
						dt = get_deptype(depinfo, "broken");
						ಯದಿ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->ಮೌಲ್ಯ);
					}
					ಅನುವರ್ತ;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ಯದಿ (!dt)
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
		ಯದಿ (!deptype)
			ಅನುವರ್ತ;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      ರಿಕ್ತ, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->ಮೌಲ್ಯ);
				ಯದಿ (!di)
					ಅನುವರ್ತ;
				ಯದಿ (strcmp(s2->ಮೌಲ್ಯ, s3->ಮೌಲ್ಯ) == 0) {
					dt = get_deptype(di, "iafter");
					ಯದಿ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					ಭಙ್ಗ;
				}
				dt = get_deptype(di, "iprovide");
				ಯದಿ (!dt)
					ಅನುವರ್ತ;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ಯದಿ (strcmp(s4->ಮೌಲ್ಯ, s2->ಮೌಲ್ಯ) == 0)
						ಭಙ್ಗ;
				}
				ಯದಿ (s4) {
					di = get_depinfo(deptree, s4->ಮೌಲ್ಯ);
					ಯದಿ (di) {
						dt = get_deptype(di, "iafter");
						ಯದಿ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					ಭಙ್ಗ;
				}
			}
			ಯದಿ (s3)
				rc_stringlist_delete(deptype->services, s2->ಮೌಲ್ಯ);
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
		ಯದಿ (errno == EEXIST) {
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
	ಯದಿ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->ಮೌಲ್ಯ);
				}
			}
			i++;
		}
		fclose(fp);
	} ಅನ್ಯಥಾ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ಯದಿ (TAILQ_FIRST(config)) {
		ಯದಿ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->ಮೌಲ್ಯ);
			fclose(fp);
		} ಅನ್ಯಥಾ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ಅನ್ಯಥಾ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ಪ್ರತ್ಯಾಗಮ retval;
}
