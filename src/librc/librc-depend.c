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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "queue.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "librc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *bootlevel = ਨਿਰਾ;

ਸਥਿਰ ਅੱਖਰ *
get_shell_ਮੁੱਲ(ਅੱਖਰ *string)
{
	ਅੱਖਰ *p = string;
	ਅੱਖਰ *e;

	ਜੇ (!string)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (*p == '\'')
		p++;

	e = p + ਲੜੀ_ਲੰਬਾਈ(p) - 1;
	ਜੇ (*e == '\n')
		*e-- = 0;
	ਜੇ (*e == '\'')
		*e-- = 0;

	ਜੇ (*p != 0)
		ਵਾਪਸ p;

	ਵਾਪਸ ਨਿਰਾ;
}

ਖਾਲੀ
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ਜੇ (!deptree)
		ਵਾਪਸ;

	TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(di, deptree, entries, di_save) {
		TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			ਮੁਕਤ(dt->type);
			ਮੁਕਤ(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		ਮੁਕਤ(di->service);
		ਮੁਕਤ(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	ਮੁਕਤ(deptree);
}

ਸਥਿਰ RC_DEPINFO *
get_depinfo(ਅਚਲ RC_DEPTREE *deptree, ਅਚਲ ਅੱਖਰ *service)
{
	RC_DEPINFO *di;
	ਜੇ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ਜੇ (strcmp(di->service, service) == 0)
				ਵਾਪਸ di;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ਅਚਲ ਅੱਖਰ *service)
{
	RC_DEPINFO *depinfo = xmalloc(ਆਕਾਰ(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_ਪਾਓ_ਪੂਛ(deptree, depinfo, entries);

	ਵਾਪਸ depinfo;
}

ਸਥਿਰ RC_DEPTYPE *
get_deptype(ਅਚਲ RC_DEPINFO *depinfo, ਅਚਲ ਅੱਖਰ *type)
{
	RC_DEPTYPE *dt;

	ਜੇ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ਜੇ (strcmp(dt->type, type) == 0)
				ਵਾਪਸ dt;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ਅਚਲ ਅੱਖਰ *type)
{
	RC_DEPTYPE *deptype = xmalloc(ਆਕਾਰ(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_ਪਾਓ_ਪੂਛ(&depinfo->depends, deptype, entries);

	ਵਾਪਸ deptype;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((ਸਮ੍ਰਿਤੀ_ਦਿਓ (rc_deptree_free, 1)))
#ਜੇ_ਅੰਤ
ਸਥਿਰ RC_DEPTREE *
make_deptree(ਖਾਲੀ) {
	RC_DEPTREE *deptree = xmalloc(ਆਕਾਰ(*deptree));
	TAILQ_INIT(deptree);
	ਵਾਪਸ deptree;
}

ਸਥਿਰ RC_DEPTREE *
deptree_ਲੱਦੋ_file(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ਨਿਰਾ;
	RC_DEPTYPE *deptype = ਨਿਰਾ;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	size_t ਮਾਪ;
	ਅੱਖਰ *type;
	ਅੱਖਰ *p;
	ਅੱਖਰ *e;
	ਪੂਰਨ i;
	FILE *fp;

	ਜੇ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ਵਾਪਸ ਨਿਰਾ;

	deptree = make_deptree();
	ਜਦੋਂ (xgetline(&ਸਤਰ, &ਮਾਪ, fp) != -1) {
		p = ਸਤਰ;
		e = strsep(&p, "_");
		ਜੇ (!e || strcmp(e, "depinfo") != 0)
			ਜਾਰੀ;
		e = strsep(&p, "_");
		ਜੇ (!e || ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(e, "%d", &i) != 1)
			ਜਾਰੀ;
		ਜੇ (!(type = strsep(&p, "_=")))
			ਜਾਰੀ;
		ਜੇ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_ਮੁੱਲ(p);
			ਜੇ (!e || *e == '\0')
				ਜਾਰੀ;
			depinfo = make_depinfo(deptree, e);
			deptype = ਨਿਰਾ;
			ਜਾਰੀ;
		}
		e = strsep(&p, "=");
		ਜੇ (!e || ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(e, "%d", &i) != 1)
			ਜਾਰੀ;
		/* Sanity */
		e = get_shell_ਮੁੱਲ(p);
		ਜੇ (!e || *e == '\0')
			ਜਾਰੀ;
		ਜੇ (!depinfo)
			ਜਾਰੀ;
		ਜੇ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_ਜੋੜੋ(deptype->services, e);
	}
	ਮੁਕਤ(ਸਤਰ);
	fclose(fp);

	ਵਾਪਸ deptree;
}

RC_DEPTREE *
rc_deptree_ਲੱਦੋ(ਖਾਲੀ)
{
	ਵਾਪਸ deptree_ਲੱਦੋ_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_ਲੱਦੋ_file(ਅਚਲ ਅੱਖਰ *deptree_file)
{
	ਵਾਪਸ deptree_ਲੱਦੋ_file(AT_FDCWD, deptree_file);
}

ਸਥਿਰ bool
valid_service(ਅਚਲ ਅੱਖਰ *runlevel, ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *type)
{
	RC_SERVICE state;

	ਜੇ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ਵਾਪਸ true;

	ਜੇ (rc_service_ਵਿੱਚ_runlevel(service, runlevel))
		ਵਾਪਸ true;
	ਜੇ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ਵਾਪਸ false;
	ਜੇ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ਵਾਪਸ false;
	ਜੇ (strcmp(runlevel, bootlevel) != 0) {
		ਜੇ (rc_service_ਵਿੱਚ_runlevel(service, bootlevel))
			ਵਾਪਸ true;
	}

	state = rc_service_state(service);
	ਜੇ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ਵਾਪਸ true;

	ਵਾਪਸ false;
}

ਸਥਿਰ bool
get_provided1(ਅਚਲ ਅੱਖਰ *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ਅਚਲ ਅੱਖਰ *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ਅਚਲ ਅੱਖਰ *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->ਮੁੱਲ;
		st = rc_service_state(svc);

		ਜੇ (level)
			ok = rc_service_ਵਿੱਚ_runlevel(svc, level);
		ਨਹੀਂਤਾਂ ਜੇ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_ਵਿੱਚ_runlevel(svc, runlevel) &&
			      !rc_service_ਵਿੱਚ_runlevel(svc, bootlevel));
		ਜੇ (!ok)
			ਜਾਰੀ;
		ਚੋਣ (state) {
			ਹਾਲਤ RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				ਰੋਕੋ;
			ਹਾਲਤ RC_SERVICE_INACTIVE:
			ਹਾਲਤ RC_SERVICE_STARTING:
			ਹਾਲਤ RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				ਰੋਕੋ;
			ਮੂਲ:
				ਰੋਕੋ;
		}
		ਜੇ (!ok)
			ਜਾਰੀ;
		retval = true;
		rc_stringlist_ਜੋੜੋ(providers, svc);
	}

	ਵਾਪਸ retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
ਸਥਿਰ RC_STRINGLIST *
get_provided(ਅਚਲ RC_DEPINFO *depinfo, ਅਚਲ ਅੱਖਰ *runlevel, ਪੂਰਨ options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	ਜੇ (!dt)
		ਵਾਪਸ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ਜੇ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_ਜੋੜੋ(providers, service->ਮੁੱਲ);
		ਵਾਪਸ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ਜੇ (options & RC_DEP_STRICT || options & RC_DEP_ਸ਼ੁਰੂ) {
		TAILQ_FOREACH(service, dt->services, entries)
			ਜੇ (rc_service_ਵਿੱਚ_runlevel(service->ਮੁੱਲ, runlevel) ||
			    rc_service_ਵਿੱਚ_runlevel(service->ਮੁੱਲ, bootlevel) ||
			    (options & RC_DEP_ਸ਼ੁਰੂ &&
			     rc_service_state(service->ਮੁੱਲ) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_ਜੋੜੋ(providers, service->ਮੁੱਲ);
		ਜੇ (TAILQ_ਪਹਿਲਾ(providers))
			ਵਾਪਸ providers;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DO \
	ਜੇ (TAILQ_ਪਹਿਲਾ(providers)) { \
		ਜੇ (TAILQ_ਅਗਲਾ(TAILQ_ਪਹਿਲਾ(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ਵਾਪਸ providers; \
	}

	/* Anything running has to come first */
	ਜੇ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ਜੇ (get_provided1(runlevel, providers, dt, ਨਿਰਾ, true, RC_SERVICE_STARTED))
	{ DO }
	ਜੇ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ਜੇ (get_provided1(runlevel, providers, dt, ਨਿਰਾ, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ਜੇ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ਵਾਪਸ providers;
	ਜੇ (get_provided1(runlevel, providers, dt, ਨਿਰਾ, true, RC_SERVICE_STARTING))
		ਵਾਪਸ providers;
	ਜੇ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ਵਾਪਸ providers;
	ਜੇ (get_provided1(runlevel, providers, dt, ਨਿਰਾ, false, RC_SERVICE_STARTING))
		ਵਾਪਸ providers;

	/* Nothing started then. OK, lets get the stopped services */
	ਜੇ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ਵਾਪਸ providers;
	ਜੇ (get_provided1(runlevel, providers, dt, ਨਿਰਾ, true, RC_SERVICE_STOPPED))
	{ DO }
	ਜੇ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ਵਾਪਸ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_ਪਹਿਲਾ(dt->services);
	ਜੇ (service != ਨਿਰਾ)
		rc_stringlist_ਜੋੜੋ(providers, service->ਮੁੱਲ);

	ਵਾਪਸ providers;
}

ਸਥਿਰ ਖਾਲੀ
ਦੇਖੋ_service(ਅਚਲ RC_DEPTREE *deptree,
	      ਅਚਲ RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *ਦੇਖਿਆ,
	      ਅਚਲ RC_DEPINFO *depinfo,
	      ਅਚਲ ਅੱਖਰ *runlevel, ਪੂਰਨ options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ਅਚਲ ਅੱਖਰ *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, ਦੇਖਿਆ, entries)
		ਜੇ (strcmp(type->ਮੁੱਲ, depinfo->service) == 0)
			ਵਾਪਸ;
	/* Add ourselves as a visited service */
	rc_stringlist_ਜੋੜੋ(ਦੇਖਿਆ, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ਜੇ (!(dt = get_deptype(depinfo, type->ਮੁੱਲ)))
			ਜਾਰੀ;

		TAILQ_FOREACH(service, dt->services, entries) {
			ਜੇ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->ਮੁੱਲ, "iprovide") == 0)
			{
				rc_stringlist_ਜੋੜੋ(sorted, service->ਮੁੱਲ);
				ਜਾਰੀ;
			}

			ਜੇ (!(di = get_depinfo(deptree, service->ਮੁੱਲ)))
				ਜਾਰੀ;
			provided = get_provided(di, runlevel, options);

			ਜੇ (TAILQ_ਪਹਿਲਾ(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->ਮੁੱਲ);
					ਜੇ (di && valid_service(runlevel, di->service, type->ਮੁੱਲ))
						ਦੇਖੋ_service(deptree, types, sorted, ਦੇਖਿਆ, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ਨਹੀਂਤਾਂ ਜੇ (di && valid_service(runlevel, service->ਮੁੱਲ, type->ਮੁੱਲ))
				ਦੇਖੋ_service(deptree, types, sorted, ਦੇਖਿਆ, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ਜੇ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ਜੇ (!(di = get_depinfo(deptree, service->ਮੁੱਲ)))
				ਜਾਰੀ;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ਜੇ (strcmp(p->ਮੁੱਲ, depinfo->service) == 0) {
					ਦੇਖੋ_service(deptree, types, sorted, ਦੇਖਿਆ, di,
						       runlevel, options | RC_DEP_TRACE);
					ਰੋਕੋ;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	ਜੇ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ਜੇ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_ਜੋੜੋ(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ਅਚਲ RC_DEPTREE *deptree,
		  ਅਚਲ ਅੱਖਰ *service, ਅਚਲ ਅੱਖਰ *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	ਜੇ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ਵਾਪਸ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_ਜੋੜੋ(svcs, svc->ਮੁੱਲ);
	ਵਾਪਸ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ਅਚਲ RC_DEPTREE *deptree,
		   ਅਚਲ RC_STRINGLIST *types,
		   ਅਚਲ RC_STRINGLIST *services,
		   ਅਚਲ ਅੱਖਰ *runlevel, ਪੂਰਨ options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *ਦੇਖਿਆ = rc_stringlist_new();
	RC_DEPINFO *di;
	ਅਚਲ RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	ਜੇ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ਜੇ (!(di = get_depinfo(deptree, service->ਮੁੱਲ))) {
			errno = ENOENT;
			ਜਾਰੀ;
		}
		ਜੇ (types)
			ਦੇਖੋ_service(deptree, types, sorted, ਦੇਖਿਆ,
				      di, runlevel, options);
	}
	rc_stringlist_free(ਦੇਖਿਆ);
	ਵਾਪਸ sorted;
}

RC_STRINGLIST *
rc_deptree_ਕ੍ਰਮ(ਅਚਲ RC_DEPTREE *deptree, ਅਚਲ ਅੱਖਰ *runlevel, ਪੂਰਨ options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ਜੇ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ਜੇ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTED);
		list2 = rc_services_ਵਿੱਚ_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		ਮੁਕਤ(list2);
		list2 = rc_services_ਵਿੱਚ_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		ਮੁਕਤ(list2);
	} ਨਹੀਂਤਾਂ {
		list = rc_services_ਵਿੱਚ_runlevel(RC_LEVEL_SYSINIT);
		ਜੇ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_ਵਿੱਚ_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			ਮੁਕਤ(list2);
			list2 = rc_services_ਵਿੱਚ_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			ਮੁਕਤ(list2);
			/* If we're not the boot runlevel then add that too */
			ਜੇ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_ਵਿੱਚ_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				ਮੁਕਤ(list2);
			}
		}
	}

	/* Now we have our lists, we need to pull in any dependencies
	   and order them */
	types = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(types, "ineed");
	rc_stringlist_ਜੋੜੋ(types, "iuse");
	rc_stringlist_ਜੋੜੋ(types, "iwant");
	rc_stringlist_ਜੋੜੋ(types, "iafter");
	services = rc_deptree_depends(deptree, types, list, runlevel,
				      RC_DEP_STRICT | RC_DEP_TRACE | options);
	rc_stringlist_free(list);
	rc_stringlist_free(types);
	ਵਾਪਸ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ਸਥਿਰ bool
deep_mtime_check(ਪੂਰਨ ਨਿਸ਼ਾਨਾ_dir, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ, bool newer, time_t *rel, ਅੱਖਰ *file)
{
	ਬਣਤਰ stat buf;
	bool retval = true;
	DIR *dp;
	ਬਣਤਰ dirent *d;

	/* If target does not exist, return true to mimic shell test */
	ਜੇ (fstatat(ਨਿਸ਼ਾਨਾ_dir, ਨਿਸ਼ਾਨਾ, &buf, 0) != 0)
		ਵਾਪਸ true;

	ਜੇ (newer) {
		ਜੇ (*rel < buf.st_mtime) {
			retval = false;

			ਜੇ (file)
				strlcpy(file, ਨਿਸ਼ਾਨਾ, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ਨਹੀਂਤਾਂ {
		ਜੇ (*rel > buf.st_mtime) {
			retval = false;

			ਜੇ (file)
				strlcpy(file, ਨਿਸ਼ਾਨਾ, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ਜੇ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(ਨਿਸ਼ਾਨਾ_dir, ਨਿਸ਼ਾਨਾ)))
		ਵਾਪਸ retval;

	/* Check all the entries in the dir */
	ਜਦੋਂ ((d = readdir(dp))) {
		ਜੇ (d->d_name[0] == '.')
			ਜਾਰੀ;
		ਜੇ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ਵਾਪਸ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ਸਥਿਰ bool
mtime_check(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *ਸਰੋਤ, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ, bool newer, time_t *rel, ਅੱਖਰ *file)
{
	ਬਣਤਰ stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	ਜੇ (stat(ਸਰੋਤ, &buf) != 0)
		ਵਾਪਸ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, ਨਿਸ਼ਾਨਾ,newer,&mtime,file);
	ਜੇ (rel) {
		*rel = mtime;
	}
	ਵਾਪਸ retval;
}

bool
rc_newer_than(ਅਚਲ ਅੱਖਰ *ਸਰੋਤ, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ, time_t *newest, ਅੱਖਰ *file)
{

	ਵਾਪਸ mtime_check(AT_FDCWD, ਸਰੋਤ, ਨਿਸ਼ਾਨਾ, true, newest, file);
}

bool
rc_older_than(ਅਚਲ ਅੱਖਰ *ਸਰੋਤ, ਅਚਲ ਅੱਖਰ *ਨਿਸ਼ਾਨਾ, time_t *oldest, ਅੱਖਰ *file)
{
	ਵਾਪਸ mtime_check(AT_FDCWD, ਸਰੋਤ, ਨਿਸ਼ਾਨਾ, false, oldest, file);
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ deppair
{
	ਅਚਲ ਅੱਖਰ *depend;
	ਅਚਲ ਅੱਖਰ *addto;
} DEPPAIR;

ਸਥਿਰ ਅਚਲ DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ਨਿਰਾ, ਨਿਰਾ }
};

bool
rc_deptree_update_needed(time_t *newest, ਅੱਖਰ *file)
{
	bool newer = false;
	ਅਚਲ ਪੂਰਨ *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ਬਣਤਰ stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	ਜੇ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ਜੇ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ਨਹੀਂਤਾਂ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ਨਿਰਾ);
	}

	ਲਈ (size_t i = 0, ਗਿਣਤੀ_2 = rc_scriptdirfds(&dirfds); i < ਗਿਣਤੀ_2; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ਜੇ (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->ਮੁੱਲ, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	ਜੇ ((newer) && (newest != ਨਿਰਾ)) {
	    *newest = mtime;
	}

	ਵਾਪਸ newer;
}

ਸਥਿਰ ਖਾਲੀ
ਤਿਆਰੀ_environment(ਖਾਲੀ)
{
	ਬਣਤਰ utsname uts;
	size_t env_ਮਾਪ;
	ਅੱਖਰ *env;
	FILE *mem = xopen_memstream(&env, &env_ਮਾਪ);

	ਲਈ (ਅਚਲ ਅੱਖਰ * ਅਚਲ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		ਜੇ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	ਮੁਕਤ(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ਜੇ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ਜੇ (uname(&uts) == 0)
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
rc_deptree_update(ਖਾਲੀ)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = ਨਿਰਾ, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ਨਿਰਾ, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *ਦੇਖਿਆ;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	ਅੱਖਰ *ਸਤਰ = ਨਿਰਾ;
	size_t ਮਾਪ;
	ਅੱਖਰ *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ਅਚਲ ਅੱਖਰ *sys = rc_sys();
	ਪੂਰਨ serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	ਤਿਆਰੀ_environment();
	ਜੇ (!(fp = popen(GENDEP, "r")))
		ਵਾਪਸ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ਜਦੋਂ (xgetline(&ਸਤਰ, &ਮਾਪ, fp) != -1) {
		depends = ਸਤਰ;
		service = strsep(&depends, " ");
		ਜੇ (!service || !*service)
			ਜਾਰੀ;

		type = strsep(&depends, " ");
		ਜੇ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ਨਿਰਾ;
			depinfo = get_depinfo(deptree, service);
			ਜੇ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ਜੇ (!type || !depends)
			ਜਾਰੀ;

		/* Get the type */
		ਜੇ (strcmp(type, "config") != 0) {
			ਜੇ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ਜੇ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ਜਦੋਂ ((depend = strsep(&depends, " "))) {
			ਜੇ (depend[0] == 0)
				ਜਾਰੀ;

			ਜੇ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				ਜਾਰੀ;
			}

			/* Don't depend on ourself */
			ਜੇ (strcmp(depend, service) == 0)
				ਜਾਰੀ;

			/* .sh files are not init scripts */
			l = ਲੜੀ_ਲੰਬਾਈ(depend);
			ਜੇ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				ਜਾਰੀ;

			/* Remove our dependency if instructed */
			ਜੇ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				ਜਾਰੀ;
			}

			rc_stringlist_ਜੋੜੋ(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ਜੇ (strcmp(type, "ibefore") == 0) {
				ਜੇ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ਜੇ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ਜੇ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	ਮੁਕਤ(ਸਤਰ);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ਜੇ (sys) {
		ਅੱਖਰ *nosys, *onosys;
		size_t len = ਲੜੀ_ਲੰਬਾਈ(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		ਲਈ (i = 0; i < len; i++)
			nosys[i + 1] = (ਅੱਖਰ)tolower((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		ਲਈ (i = 0; i < len; i++)
			onosys[i + 2] = (ਅੱਖਰ)tolower((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(depinfo, deptree, entries, depinfo_np) {
			ਜੇ (!(deptype = get_deptype(depinfo, "keyword")))
				ਜਾਰੀ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ਜੇ (strcmp(s->ਮੁੱਲ, nosys) != 0 && strcmp(s->ਮੁੱਲ, onosys) != 0)
					ਜਾਰੀ;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ਜੇ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->ਮੁੱਲ);
						ਜੇ (!TAILQ_ਪਹਿਲਾ(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							ਮੁਕਤ(dt->type);
							ਮੁਕਤ(dt->services);
							ਮੁਕਤ(dt);
						}
					}
				}
			}
		}
		ਮੁਕਤ(nosys);
		ਮੁਕਤ(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(ਆਕਾਰ(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ਜੇ (!(deptype = get_deptype(depinfo, "iprovide")))
			ਜਾਰੀ;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->ਮੁੱਲ);
			ਜੇ (!di)
				di = make_depinfo(providers, s->ਮੁੱਲ);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	ਮੁਕਤ(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ਲਈ (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ਜੇ (!deptype)
				ਜਾਰੀ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->ਮੁੱਲ);
				ਜੇ (!di) {
					ਜੇ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->ਮੁੱਲ);
						dt = get_deptype(depinfo, "broken");
						ਜੇ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->ਮੁੱਲ);
					}
					ਜਾਰੀ;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ਜੇ (!dt)
					dt = make_deptype(di, deppairs[i].addto);
				rc_stringlist_addu(dt->services, depinfo->service);
			}
		}
	}

	/* Phase 5 - Remove broken before directives */
	types = rc_stringlist_new();
	rc_stringlist_ਜੋੜੋ(types, "ineed");
	rc_stringlist_ਜੋੜੋ(types, "iwant");
	rc_stringlist_ਜੋੜੋ(types, "iuse");
	rc_stringlist_ਜੋੜੋ(types, "iafter");
	TAILQ_FOREACH(depinfo, deptree, entries) {
		deptype = get_deptype(depinfo, "ibefore");
		ਜੇ (!deptype)
			ਜਾਰੀ;
		sorted = rc_stringlist_new();
		ਦੇਖਿਆ = rc_stringlist_new();
		ਦੇਖੋ_service(deptree, types, sorted, ਦੇਖਿਆ, depinfo,
			      ਨਿਰਾ, 0);
		rc_stringlist_free(ਦੇਖਿਆ);
		TAILQ_FOREACH_ਸੁਰੱਖਿਅਤ(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->ਮੁੱਲ);
				ਜੇ (!di)
					ਜਾਰੀ;
				ਜੇ (strcmp(s2->ਮੁੱਲ, s3->ਮੁੱਲ) == 0) {
					dt = get_deptype(di, "iafter");
					ਜੇ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					ਰੋਕੋ;
				}
				dt = get_deptype(di, "iprovide");
				ਜੇ (!dt)
					ਜਾਰੀ;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ਜੇ (strcmp(s4->ਮੁੱਲ, s2->ਮੁੱਲ) == 0)
						ਰੋਕੋ;
				}
				ਜੇ (s4) {
					di = get_depinfo(deptree, s4->ਮੁੱਲ);
					ਜੇ (di) {
						dt = get_deptype(di, "iafter");
						ਜੇ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					ਰੋਕੋ;
				}
			}
			ਜੇ (s3)
				rc_stringlist_delete(deptype->services, s2->ਮੁੱਲ);
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
		ਜੇ (errno == EEXIST) {
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
	ਜੇ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->ਮੁੱਲ);
				}
			}
			i++;
		}
		fclose(fp);
	} ਨਹੀਂਤਾਂ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ਜੇ (TAILQ_ਪਹਿਲਾ(config)) {
		ਜੇ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->ਮੁੱਲ);
			fclose(fp);
		} ਨਹੀਂਤਾਂ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ਨਹੀਂਤਾਂ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ਵਾਪਸ retval;
}
