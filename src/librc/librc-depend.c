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

#ଅନ୍ତର୍ଭାଵ <sys/utsname.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <dirent.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "queue.h"
#ଅନ୍ତର୍ଭାଵ "librc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *bootlevel = ଖାଲି;

ସ୍ଥିର ଅକ୍ଷର *
get_shell_ମୂଲ୍ୟ(ଅକ୍ଷର *string)
{
	ଅକ୍ଷର *p = string;
	ଅକ୍ଷର *e;

	ଯଦି (!string)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	ଯଦି (*e == '\n')
		*e-- = 0;
	ଯଦି (*e == '\'')
		*e-- = 0;

	ଯଦି (*p != 0)
		ଫେରନ୍ତୁ p;

	ଫେରନ୍ତୁ ଖାଲି;
}

ଶୂନ୍ୟ
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ଯଦି (!deptree)
		ଫେରନ୍ତୁ;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			ମୁକ୍ତ(dt->type);
			ମୁକ୍ତ(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		ମୁକ୍ତ(di->service);
		ମୁକ୍ତ(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	ମୁକ୍ତ(deptree);
}

ସ୍ଥିର RC_DEPINFO *
get_depinfo(ଅଚଳ RC_DEPTREE *deptree, ଅଚଳ ଅକ୍ଷର *service)
{
	RC_DEPINFO *di;
	ଯଦି (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ଯଦି (strcmp(di->service, service) == 0)
				ଫେରନ୍ତୁ di;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ଅଚଳ ଅକ୍ଷର *service)
{
	RC_DEPINFO *depinfo = xmalloc(ଆକାର(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_ଭର୍ତ୍ତି_TAIL(deptree, depinfo, entries);

	ଫେରନ୍ତୁ depinfo;
}

ସ୍ଥିର RC_DEPTYPE *
get_deptype(ଅଚଳ RC_DEPINFO *depinfo, ଅଚଳ ଅକ୍ଷର *type)
{
	RC_DEPTYPE *dt;

	ଯଦି (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ଯଦି (strcmp(dt->type, type) == 0)
				ଫେରନ୍ତୁ dt;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ଅଚଳ ଅକ୍ଷର *type)
{
	RC_DEPTYPE *deptype = xmalloc(ଆକାର(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_ଭର୍ତ୍ତି_TAIL(&depinfo->depends, deptype, entries);

	ଫେରନ୍ତୁ deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
ସ୍ଥିର RC_DEPTREE *
make_deptree(ଶୂନ୍ୟ) {
	RC_DEPTREE *deptree = xmalloc(ଆକାର(*deptree));
	TAILQ_INIT(deptree);
	ଫେରନ୍ତୁ deptree;
}

ସ୍ଥିର RC_DEPTREE *
deptree_load_file(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ଖାଲି;
	RC_DEPTYPE *deptype = ଖାଲି;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	size_t size;
	ଅକ୍ଷର *type;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *e;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	FILE *fp;

	ଯଦି (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ଫେରନ୍ତୁ ଖାଲି;

	deptree = make_deptree();
	ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &size, fp) != -1) {
		p = ଧାଡ଼ି;
		e = strsep(&p, "_");
		ଯଦି (!e || strcmp(e, "depinfo") != 0)
			ଜାରି;
		e = strsep(&p, "_");
		ଯଦି (!e || sscanf(e, "%d", &i) != 1)
			ଜାରି;
		ଯଦି (!(type = strsep(&p, "_=")))
			ଜାରି;
		ଯଦି (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_ମୂଲ୍ୟ(p);
			ଯଦି (!e || *e == '\0')
				ଜାରି;
			depinfo = make_depinfo(deptree, e);
			deptype = ଖାଲି;
			ଜାରି;
		}
		e = strsep(&p, "=");
		ଯଦି (!e || sscanf(e, "%d", &i) != 1)
			ଜାରି;
		/* Sanity */
		e = get_shell_ମୂଲ୍ୟ(p);
		ଯଦି (!e || *e == '\0')
			ଜାରି;
		ଯଦି (!depinfo)
			ଜାରି;
		ଯଦି (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	ମୁକ୍ତ(ଧାଡ଼ି);
	fclose(fp);

	ଫେରନ୍ତୁ deptree;
}

RC_DEPTREE *
rc_deptree_load(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ଅଚଳ ଅକ୍ଷର *deptree_file)
{
	ଫେରନ୍ତୁ deptree_load_file(AT_FDCWD, deptree_file);
}

ସ୍ଥିର bool
valid_service(ଅଚଳ ଅକ୍ଷର *runlevel, ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *type)
{
	RC_SERVICE state;

	ଯଦି (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ଫେରନ୍ତୁ true;

	ଯଦି (rc_service_in_runlevel(service, runlevel))
		ଫେରନ୍ତୁ true;
	ଯଦି (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ଫେରନ୍ତୁ false;
	ଯଦି (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ଫେରନ୍ତୁ false;
	ଯଦି (strcmp(runlevel, bootlevel) != 0) {
		ଯଦି (rc_service_in_runlevel(service, bootlevel))
			ଫେରନ୍ତୁ true;
	}

	state = rc_service_state(service);
	ଯଦି (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ଫେରନ୍ତୁ true;

	ଫେରନ୍ତୁ false;
}

ସ୍ଥିର bool
get_provided1(ଅଚଳ ଅକ୍ଷର *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ଅଚଳ ଅକ୍ଷର *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ଅଚଳ ଅକ୍ଷର *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->ମୂଲ୍ୟ;
		st = rc_service_state(svc);

		ଯଦି (level)
			ok = rc_service_in_runlevel(svc, level);
		ଅନ୍ୟଥା ଯଦି (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		ଯଦି (!ok)
			ଜାରି;
		ବାଛ (state) {
			ସ୍ଥିତି RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				ଭାଙ୍ଗ;
			ସ୍ଥିତି RC_SERVICE_INACTIVE:
			ସ୍ଥିତି RC_SERVICE_STARTING:
			ସ୍ଥିତି RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				ଭାଙ୍ଗ;
		}
		ଯଦି (!ok)
			ଜାରି;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	ଫେରନ୍ତୁ retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
ସ୍ଥିର RC_STRINGLIST *
get_provided(ଅଚଳ RC_DEPINFO *depinfo, ଅଚଳ ଅକ୍ଷର *runlevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	ଯଦି (!dt)
		ଫେରନ୍ତୁ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ଯଦି (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->ମୂଲ୍ୟ);
		ଫେରନ୍ତୁ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ଯଦି (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			ଯଦି (rc_service_in_runlevel(service->ମୂଲ୍ୟ, runlevel) ||
			    rc_service_in_runlevel(service->ମୂଲ୍ୟ, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->ମୂଲ୍ୟ) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->ମୂଲ୍ୟ);
		ଯଦି (TAILQ_FIRST(providers))
			ଫେରନ୍ତୁ providers;
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
	ଯଦି (TAILQ_FIRST(providers)) { \
		ଯଦି (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ଫେରନ୍ତୁ providers; \
	}

	/* Anything running has to come first */
	ଯଦି (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ଯଦି (get_provided1(runlevel, providers, dt, ଖାଲି, true, RC_SERVICE_STARTED))
	{ DO }
	ଯଦି (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ଯଦି (get_provided1(runlevel, providers, dt, ଖାଲି, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ଯଦି (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ଫେରନ୍ତୁ providers;
	ଯଦି (get_provided1(runlevel, providers, dt, ଖାଲି, true, RC_SERVICE_STARTING))
		ଫେରନ୍ତୁ providers;
	ଯଦି (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ଫେରନ୍ତୁ providers;
	ଯଦି (get_provided1(runlevel, providers, dt, ଖାଲି, false, RC_SERVICE_STARTING))
		ଫେରନ୍ତୁ providers;

	/* Nothing started then. OK, lets get the stopped services */
	ଯଦି (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ଫେରନ୍ତୁ providers;
	ଯଦି (get_provided1(runlevel, providers, dt, ଖାଲି, true, RC_SERVICE_STOPPED))
	{ DO }
	ଯଦି (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ଫେରନ୍ତୁ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	ଯଦି (service != ଖାଲି)
		rc_stringlist_add(providers, service->ମୂଲ୍ୟ);

	ଫେରନ୍ତୁ providers;
}

ସ୍ଥିର ଶୂନ୍ୟ
visit_service(ଅଚଳ RC_DEPTREE *deptree,
	      ଅଚଳ RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      ଅଚଳ RC_DEPINFO *depinfo,
	      ଅଚଳ ଅକ୍ଷର *runlevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ଅଚଳ ଅକ୍ଷର *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		ଯଦି (strcmp(type->ମୂଲ୍ୟ, depinfo->service) == 0)
			ଫେରନ୍ତୁ;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ଯଦି (!(dt = get_deptype(depinfo, type->ମୂଲ୍ୟ)))
			ଜାରି;

		TAILQ_FOREACH(service, dt->services, entries) {
			ଯଦି (!(options & RC_DEP_TRACE) ||
			    strcmp(type->ମୂଲ୍ୟ, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->ମୂଲ୍ୟ);
				ଜାରି;
			}

			ଯଦି (!(di = get_depinfo(deptree, service->ମୂଲ୍ୟ)))
				ଜାରି;
			provided = get_provided(di, runlevel, options);

			ଯଦି (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->ମୂଲ୍ୟ);
					ଯଦି (di && valid_service(runlevel, di->service, type->ମୂଲ୍ୟ))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ଅନ୍ୟଥା ଯଦି (di && valid_service(runlevel, service->ମୂଲ୍ୟ, type->ମୂଲ୍ୟ))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ଯଦି (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ଯଦି (!(di = get_depinfo(deptree, service->ମୂଲ୍ୟ)))
				ଜାରି;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ଯଦି (strcmp(p->ମୂଲ୍ୟ, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					ଭାଙ୍ଗ;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	ଯଦି (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ଯଦି (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ଅଚଳ RC_DEPTREE *deptree,
		  ଅଚଳ ଅକ୍ଷର *service, ଅଚଳ ଅକ୍ଷର *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	ଯଦି (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ଫେରନ୍ତୁ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->ମୂଲ୍ୟ);
	ଫେରନ୍ତୁ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ଅଚଳ RC_DEPTREE *deptree,
		   ଅଚଳ RC_STRINGLIST *types,
		   ଅଚଳ RC_STRINGLIST *services,
		   ଅଚଳ ଅକ୍ଷର *runlevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	ଅଚଳ RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	ଯଦି (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ଯଦି (!(di = get_depinfo(deptree, service->ମୂଲ୍ୟ))) {
			errno = ENOENT;
			ଜାରି;
		}
		ଯଦି (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	ଫେରନ୍ତୁ sorted;
}

RC_STRINGLIST *
rc_deptree_order(ଅଚଳ RC_DEPTREE *deptree, ଅଚଳ ଅକ୍ଷର *runlevel, ପୂର୍ଣ୍ଣସଂଖ୍ୟା options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ଯଦି (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ଯଦି (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		ମୁକ୍ତ(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		ମୁକ୍ତ(list2);
	} ଅନ୍ୟଥା {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		ଯଦି (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			ମୁକ୍ତ(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			ମୁକ୍ତ(list2);
			/* If we're not the boot runlevel then add that too */
			ଯଦି (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				ମୁକ୍ତ(list2);
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
	ଫେରନ୍ତୁ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ସ୍ଥିର bool
deep_mtime_check(ପୂର୍ଣ୍ଣସଂଖ୍ୟା target_dir, ଅଚଳ ଅକ୍ଷର *target, bool newer, time_t *rel, ଅକ୍ଷର *file)
{
	ସଂରଚନା stat buf;
	bool retval = true;
	DIR *dp;
	ସଂରଚନା dirent *d;

	/* If target does not exist, return true to mimic shell test */
	ଯଦି (fstatat(target_dir, target, &buf, 0) != 0)
		ଫେରନ୍ତୁ true;

	ଯଦି (newer) {
		ଯଦି (*rel < buf.st_mtime) {
			retval = false;

			ଯଦି (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ଅନ୍ୟଥା {
		ଯଦି (*rel > buf.st_mtime) {
			retval = false;

			ଯଦି (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ଯଦି (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		ଫେରନ୍ତୁ retval;

	/* Check all the entries in the dir */
	ଯେତେବେଳେ ((d = readdir(dp))) {
		ଯଦି (d->d_name[0] == '.')
			ଜାରି;
		ଯଦି (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ଫେରନ୍ତୁ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ସ୍ଥିର bool
mtime_check(ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirfd, ଅଚଳ ଅକ୍ଷର *source, ଅଚଳ ଅକ୍ଷର *target, bool newer, time_t *rel, ଅକ୍ଷର *file)
{
	ସଂରଚନା stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	ଯଦି (stat(source, &buf) != 0)
		ଫେରନ୍ତୁ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	ଯଦି (rel) {
		*rel = mtime;
	}
	ଫେରନ୍ତୁ retval;
}

bool
rc_newer_than(ଅଚଳ ଅକ୍ଷର *source, ଅଚଳ ଅକ୍ଷର *target, time_t *newest, ଅକ୍ଷର *file)
{

	ଫେରନ୍ତୁ mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ଅଚଳ ଅକ୍ଷର *source, ଅଚଳ ଅକ୍ଷର *target, time_t *oldest, ଅକ୍ଷର *file)
{
	ଫେରନ୍ତୁ mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା deppair
{
	ଅଚଳ ଅକ୍ଷର *depend;
	ଅଚଳ ଅକ୍ଷର *addto;
} DEPPAIR;

ସ୍ଥିର ଅଚଳ DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ଖାଲି, ଖାଲି }
};

bool
rc_deptree_update_needed(time_t *newest, ଅକ୍ଷର *file)
{
	bool newer = false;
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ସଂରଚନା stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	ଯଦି (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ଯଦି (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ଅନ୍ୟଥା {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ଖାଲି);
	}

	ପାଇଁ (size_t i = 0, ଗଣନା_2 = rc_scriptdirfds(&dirfds); i < ଗଣନା_2; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ଯଦି (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->ମୂଲ୍ୟ, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	ଯଦି ((newer) && (newest != ଖାଲି)) {
	    *newest = mtime;
	}

	ଫେରନ୍ତୁ newer;
}

ସ୍ଥିର ଶୂନ୍ୟ
setup_environment(ଶୂନ୍ୟ)
{
	ସଂରଚନା utsname uts;
	size_t env_size;
	ଅକ୍ଷର *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	ପାଇଁ (ଅଚଳ ଅକ୍ଷର * ଅଚଳ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		ଯଦି (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	ମୁକ୍ତ(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ଯଦି (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ଯଦି (uname(&uts) == 0)
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
rc_deptree_update(ଶୂନ୍ୟ)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = ଖାଲି, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ଖାଲି, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	ଅକ୍ଷର *ଧାଡ଼ି = ଖାଲି;
	size_t size;
	ଅକ୍ଷର *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ଅଚଳ ଅକ୍ଷର *sys = rc_sys();
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	ଯଦି (!(fp = popen(GENDEP, "r")))
		ଫେରନ୍ତୁ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ଯେତେବେଳେ (xgetline(&ଧାଡ଼ି, &size, fp) != -1) {
		depends = ଧାଡ଼ି;
		service = strsep(&depends, " ");
		ଯଦି (!service || !*service)
			ଜାରି;

		type = strsep(&depends, " ");
		ଯଦି (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ଖାଲି;
			depinfo = get_depinfo(deptree, service);
			ଯଦି (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ଯଦି (!type || !depends)
			ଜାରି;

		/* Get the type */
		ଯଦି (strcmp(type, "config") != 0) {
			ଯଦି (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ଯଦି (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ଯେତେବେଳେ ((depend = strsep(&depends, " "))) {
			ଯଦି (depend[0] == 0)
				ଜାରି;

			ଯଦି (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				ଜାରି;
			}

			/* Don't depend on ourself */
			ଯଦି (strcmp(depend, service) == 0)
				ଜାରି;

			/* .sh files are not init scripts */
			l = strlen(depend);
			ଯଦି (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				ଜାରି;

			/* Remove our dependency if instructed */
			ଯଦି (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				ଜାରି;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ଯଦି (strcmp(type, "ibefore") == 0) {
				ଯଦି ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ଯଦି (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ଯଦି ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	ମୁକ୍ତ(ଧାଡ଼ି);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ଯଦି (sys) {
		ଅକ୍ଷର *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		ପାଇଁ (i = 0; i < len; i++)
			nosys[i + 1] = (ଅକ୍ଷର)tolower((unsigned ଅକ୍ଷର)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		ପାଇଁ (i = 0; i < len; i++)
			onosys[i + 2] = (ଅକ୍ଷର)tolower((unsigned ଅକ୍ଷର)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			ଯଦି (!(deptype = get_deptype(depinfo, "keyword")))
				ଜାରି;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ଯଦି (strcmp(s->ମୂଲ୍ୟ, nosys) != 0 && strcmp(s->ମୂଲ୍ୟ, onosys) != 0)
					ଜାରି;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ଯଦି (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->ମୂଲ୍ୟ);
						ଯଦି (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							ମୁକ୍ତ(dt->type);
							ମୁକ୍ତ(dt->services);
							ମୁକ୍ତ(dt);
						}
					}
				}
			}
		}
		ମୁକ୍ତ(nosys);
		ମୁକ୍ତ(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(ଆକାର(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ଯଦି (!(deptype = get_deptype(depinfo, "iprovide")))
			ଜାରି;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->ମୂଲ୍ୟ);
			ଯଦି (!di)
				di = make_depinfo(providers, s->ମୂଲ୍ୟ);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	ମୁକ୍ତ(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ପାଇଁ (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ଯଦି (!deptype)
				ଜାରି;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->ମୂଲ୍ୟ);
				ଯଦି (!di) {
					ଯଦି (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->ମୂଲ୍ୟ);
						dt = get_deptype(depinfo, "broken");
						ଯଦି (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->ମୂଲ୍ୟ);
					}
					ଜାରି;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ଯଦି (!dt)
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
		ଯଦି (!deptype)
			ଜାରି;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      ଖାଲି, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->ମୂଲ୍ୟ);
				ଯଦି (!di)
					ଜାରି;
				ଯଦି (strcmp(s2->ମୂଲ୍ୟ, s3->ମୂଲ୍ୟ) == 0) {
					dt = get_deptype(di, "iafter");
					ଯଦି (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					ଭାଙ୍ଗ;
				}
				dt = get_deptype(di, "iprovide");
				ଯଦି (!dt)
					ଜାରି;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ଯଦି (strcmp(s4->ମୂଲ୍ୟ, s2->ମୂଲ୍ୟ) == 0)
						ଭାଙ୍ଗ;
				}
				ଯଦି (s4) {
					di = get_depinfo(deptree, s4->ମୂଲ୍ୟ);
					ଯଦି (di) {
						dt = get_deptype(di, "iafter");
						ଯଦି (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					ଭାଙ୍ଗ;
				}
			}
			ଯଦି (s3)
				rc_stringlist_delete(deptype->services, s2->ମୂଲ୍ୟ);
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
		ଯଦି (errno == EEXIST) {
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
	ଯଦି ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->ମୂଲ୍ୟ);
				}
			}
			i++;
		}
		fclose(fp);
	} ଅନ୍ୟଥା {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ଯଦି (TAILQ_FIRST(config)) {
		ଯଦି ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->ମୂଲ୍ୟ);
			fclose(fp);
		} ଅନ୍ୟଥା {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ଅନ୍ୟଥା {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ଫେରନ୍ତୁ retval;
}
