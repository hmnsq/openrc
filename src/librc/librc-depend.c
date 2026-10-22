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

#అన్తర్భావ <sys/utsname.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "queue.h"
#అన్తర్భావ "librc.h"
#అన్తర్భావ "helpers.h"
#అన్తర్భావ "misc.h"

#నిర్వచించు GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

స్థిర స్థిరరాశి అక్షరం *bootlevel = ఖాళీ;

స్థిర అక్షరం *
get_shell_విలువ(అక్షరం *string)
{
	అక్షరం *p = string;
	అక్షరం *e;

	ఒకవేళ (!string)
		తిరిగి ఖాళీ;

	ఒకవేళ (*p == '\'')
		p++;

	e = p + అక్షరశ్రేణి_పొడవు(p) - 1;
	ఒకవేళ (*e == '\n')
		*e-- = 0;
	ఒకవేళ (*e == '\'')
		*e-- = 0;

	ఒకవేళ (*p != 0)
		తిరిగి p;

	తిరిగి ఖాళీ;
}

శూన్య
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ఒకవేళ (!deptree)
		తిరిగి;

	TAILQ_FOREACH_సురక్షిత(di, deptree, entries, di_save) {
		TAILQ_FOREACH_సురక్షిత(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			విడుదల(dt->type);
			విడుదల(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		విడుదల(di->service);
		విడుదల(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	విడుదల(deptree);
}

స్థిర RC_DEPINFO *
get_depinfo(స్థిరరాశి RC_DEPTREE *deptree, స్థిరరాశి అక్షరం *service)
{
	RC_DEPINFO *di;
	ఒకవేళ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ఒకవేళ (strcmp(di->service, service) == 0)
				తిరిగి di;
	}
	తిరిగి ఖాళీ;
}

స్థిర RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, స్థిరరాశి అక్షరం *service)
{
	RC_DEPINFO *depinfo = xmalloc(పరిమాణం(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_చొప్పించు_తోక(deptree, depinfo, entries);

	తిరిగి depinfo;
}

స్థిర RC_DEPTYPE *
get_deptype(స్థిరరాశి RC_DEPINFO *depinfo, స్థిరరాశి అక్షరం *type)
{
	RC_DEPTYPE *dt;

	ఒకవేళ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ఒకవేళ (strcmp(dt->type, type) == 0)
				తిరిగి dt;
	}
	తిరిగి ఖాళీ;
}

స్థిర RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, స్థిరరాశి అక్షరం *type)
{
	RC_DEPTYPE *deptype = xmalloc(పరిమాణం(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_చొప్పించు_తోక(&depinfo->depends, deptype, entries);

	తిరిగి deptype;
}

#నిర్వచితమైతే HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((జ్ఞాపకం_ఇవ్వు (rc_deptree_free, 1)))
#అయితే_ముగింపు
స్థిర RC_DEPTREE *
make_deptree(శూన్య) {
	RC_DEPTREE *deptree = xmalloc(పరిమాణం(*deptree));
	TAILQ_INIT(deptree);
	తిరిగి deptree;
}

స్థిర RC_DEPTREE *
deptree_ఎక్కించు_file(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ఖాళీ;
	RC_DEPTYPE *deptype = ఖాళీ;
	అక్షరం *పంక్తి = ఖాళీ;
	size_t కొలత;
	అక్షరం *type;
	అక్షరం *p;
	అక్షరం *e;
	పూర్ణాంకం i;
	FILE *fp;

	ఒకవేళ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		తిరిగి ఖాళీ;

	deptree = make_deptree();
	అయితే (xgetline(&పంక్తి, &కొలత, fp) != -1) {
		p = పంక్తి;
		e = strsep(&p, "_");
		ఒకవేళ (!e || strcmp(e, "depinfo") != 0)
			కొనసాగు;
		e = strsep(&p, "_");
		ఒకవేళ (!e || అక్షరశ్రేణి_చదువు(e, "%d", &i) != 1)
			కొనసాగు;
		ఒకవేళ (!(type = strsep(&p, "_=")))
			కొనసాగు;
		ఒకవేళ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_విలువ(p);
			ఒకవేళ (!e || *e == '\0')
				కొనసాగు;
			depinfo = make_depinfo(deptree, e);
			deptype = ఖాళీ;
			కొనసాగు;
		}
		e = strsep(&p, "=");
		ఒకవేళ (!e || అక్షరశ్రేణి_చదువు(e, "%d", &i) != 1)
			కొనసాగు;
		/* Sanity */
		e = get_shell_విలువ(p);
		ఒకవేళ (!e || *e == '\0')
			కొనసాగు;
		ఒకవేళ (!depinfo)
			కొనసాగు;
		ఒకవేళ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_కూడు(deptype->services, e);
	}
	విడుదల(పంక్తి);
	fclose(fp);

	తిరిగి deptree;
}

RC_DEPTREE *
rc_deptree_ఎక్కించు(శూన్య)
{
	తిరిగి deptree_ఎక్కించు_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_ఎక్కించు_file(స్థిరరాశి అక్షరం *deptree_file)
{
	తిరిగి deptree_ఎక్కించు_file(AT_FDCWD, deptree_file);
}

స్థిర bool
valid_service(స్థిరరాశి అక్షరం *runlevel, స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *type)
{
	RC_SERVICE state;

	ఒకవేళ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		తిరిగి true;

	ఒకవేళ (rc_service_లోపల_runlevel(service, runlevel))
		తిరిగి true;
	ఒకవేళ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    తిరిగి false;
	ఒకవేళ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    తిరిగి false;
	ఒకవేళ (strcmp(runlevel, bootlevel) != 0) {
		ఒకవేళ (rc_service_లోపల_runlevel(service, bootlevel))
			తిరిగి true;
	}

	state = rc_service_state(service);
	ఒకవేళ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		తిరిగి true;

	తిరిగి false;
}

స్థిర bool
get_provided1(స్థిరరాశి అక్షరం *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, స్థిరరాశి అక్షరం *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	స్థిరరాశి అక్షరం *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->విలువ;
		st = rc_service_state(svc);

		ఒకవేళ (level)
			ok = rc_service_లోపల_runlevel(svc, level);
		లేకపోతే ఒకవేళ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_లోపల_runlevel(svc, runlevel) &&
			      !rc_service_లోపల_runlevel(svc, bootlevel));
		ఒకవేళ (!ok)
			కొనసాగు;
		ఎంపిక (state) {
			స్థితి RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				ఆపు;
			స్థితి RC_SERVICE_INACTIVE:
			స్థితి RC_SERVICE_STARTING:
			స్థితి RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				ఆపు;
			అప్రమేయ:
				ఆపు;
		}
		ఒకవేళ (!ok)
			కొనసాగు;
		retval = true;
		rc_stringlist_కూడు(providers, svc);
	}

	తిరిగి retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
స్థిర RC_STRINGLIST *
get_provided(స్థిరరాశి RC_DEPINFO *depinfo, స్థిరరాశి అక్షరం *runlevel, పూర్ణాంకం options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	ఒకవేళ (!dt)
		తిరిగి providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ఒకవేళ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_కూడు(providers, service->విలువ);
		తిరిగి providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ఒకవేళ (options & RC_DEP_STRICT || options & RC_DEP_ప్రారంభం) {
		TAILQ_FOREACH(service, dt->services, entries)
			ఒకవేళ (rc_service_లోపల_runlevel(service->విలువ, runlevel) ||
			    rc_service_లోపల_runlevel(service->విలువ, bootlevel) ||
			    (options & RC_DEP_ప్రారంభం &&
			     rc_service_state(service->విలువ) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_కూడు(providers, service->విలువ);
		ఒకవేళ (TAILQ_మొదటి(providers))
			తిరిగి providers;
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
#నిర్వచించు DO \
	ఒకవేళ (TAILQ_మొదటి(providers)) { \
		ఒకవేళ (TAILQ_తదుపరి(TAILQ_మొదటి(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		తిరిగి providers; \
	}

	/* Anything running has to come first */
	ఒకవేళ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ఒకవేళ (get_provided1(runlevel, providers, dt, ఖాళీ, true, RC_SERVICE_STARTED))
	{ DO }
	ఒకవేళ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ఒకవేళ (get_provided1(runlevel, providers, dt, ఖాళీ, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ఒకవేళ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		తిరిగి providers;
	ఒకవేళ (get_provided1(runlevel, providers, dt, ఖాళీ, true, RC_SERVICE_STARTING))
		తిరిగి providers;
	ఒకవేళ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    తిరిగి providers;
	ఒకవేళ (get_provided1(runlevel, providers, dt, ఖాళీ, false, RC_SERVICE_STARTING))
		తిరిగి providers;

	/* Nothing started then. OK, lets get the stopped services */
	ఒకవేళ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		తిరిగి providers;
	ఒకవేళ (get_provided1(runlevel, providers, dt, ఖాళీ, true, RC_SERVICE_STOPPED))
	{ DO }
	ఒకవేళ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		తిరిగి providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_మొదటి(dt->services);
	ఒకవేళ (service != ఖాళీ)
		rc_stringlist_కూడు(providers, service->విలువ);

	తిరిగి providers;
}

స్థిర శూన్య
సందర్శించు_service(స్థిరరాశి RC_DEPTREE *deptree,
	      స్థిరరాశి RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *సందర్శించిన,
	      స్థిరరాశి RC_DEPINFO *depinfo,
	      స్థిరరాశి అక్షరం *runlevel, పూర్ణాంకం options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	స్థిరరాశి అక్షరం *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, సందర్శించిన, entries)
		ఒకవేళ (strcmp(type->విలువ, depinfo->service) == 0)
			తిరిగి;
	/* Add ourselves as a visited service */
	rc_stringlist_కూడు(సందర్శించిన, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ఒకవేళ (!(dt = get_deptype(depinfo, type->విలువ)))
			కొనసాగు;

		TAILQ_FOREACH(service, dt->services, entries) {
			ఒకవేళ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->విలువ, "iprovide") == 0)
			{
				rc_stringlist_కూడు(sorted, service->విలువ);
				కొనసాగు;
			}

			ఒకవేళ (!(di = get_depinfo(deptree, service->విలువ)))
				కొనసాగు;
			provided = get_provided(di, runlevel, options);

			ఒకవేళ (TAILQ_మొదటి(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->విలువ);
					ఒకవేళ (di && valid_service(runlevel, di->service, type->విలువ))
						సందర్శించు_service(deptree, types, sorted, సందర్శించిన, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			లేకపోతే ఒకవేళ (di && valid_service(runlevel, service->విలువ, type->విలువ))
				సందర్శించు_service(deptree, types, sorted, సందర్శించిన, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ఒకవేళ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ఒకవేళ (!(di = get_depinfo(deptree, service->విలువ)))
				కొనసాగు;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ఒకవేళ (strcmp(p->విలువ, depinfo->service) == 0) {
					సందర్శించు_service(deptree, types, sorted, సందర్శించిన, di,
						       runlevel, options | RC_DEP_TRACE);
					ఆపు;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	ఒకవేళ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ఒకవేళ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_కూడు(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(స్థిరరాశి RC_DEPTREE *deptree,
		  స్థిరరాశి అక్షరం *service, స్థిరరాశి అక్షరం *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	ఒకవేళ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		తిరిగి svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_కూడు(svcs, svc->విలువ);
	తిరిగి svcs;
}

RC_STRINGLIST *
rc_deptree_depends(స్థిరరాశి RC_DEPTREE *deptree,
		   స్థిరరాశి RC_STRINGLIST *types,
		   స్థిరరాశి RC_STRINGLIST *services,
		   స్థిరరాశి అక్షరం *runlevel, పూర్ణాంకం options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *సందర్శించిన = rc_stringlist_new();
	RC_DEPINFO *di;
	స్థిరరాశి RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	ఒకవేళ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ఒకవేళ (!(di = get_depinfo(deptree, service->విలువ))) {
			errno = ENOENT;
			కొనసాగు;
		}
		ఒకవేళ (types)
			సందర్శించు_service(deptree, types, sorted, సందర్శించిన,
				      di, runlevel, options);
	}
	rc_stringlist_free(సందర్శించిన);
	తిరిగి sorted;
}

RC_STRINGLIST *
rc_deptree_క్రమం(స్థిరరాశి RC_DEPTREE *deptree, స్థిరరాశి అక్షరం *runlevel, పూర్ణాంకం options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ఒకవేళ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ఒకవేళ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_లోపల_state(RC_SERVICE_STARTED);
		list2 = rc_services_లోపల_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		విడుదల(list2);
		list2 = rc_services_లోపల_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		విడుదల(list2);
	} లేకపోతే {
		list = rc_services_లోపల_runlevel(RC_LEVEL_SYSINIT);
		ఒకవేళ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_లోపల_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			విడుదల(list2);
			list2 = rc_services_లోపల_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			విడుదల(list2);
			/* If we're not the boot runlevel then add that too */
			ఒకవేళ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_లోపల_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				విడుదల(list2);
			}
		}
	}

	/* Now we have our lists, we need to pull in any dependencies
	   and order them */
	types = rc_stringlist_new();
	rc_stringlist_కూడు(types, "ineed");
	rc_stringlist_కూడు(types, "iuse");
	rc_stringlist_కూడు(types, "iwant");
	rc_stringlist_కూడు(types, "iafter");
	services = rc_deptree_depends(deptree, types, list, runlevel,
				      RC_DEP_STRICT | RC_DEP_TRACE | options);
	rc_stringlist_free(list);
	rc_stringlist_free(types);
	తిరిగి services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
స్థిర bool
deep_mtime_check(పూర్ణాంకం లక్ష్యం_dir, స్థిరరాశి అక్షరం *లక్ష్యం, bool newer, time_t *rel, అక్షరం *file)
{
	నిర్మాణం stat buf;
	bool retval = true;
	DIR *dp;
	నిర్మాణం dirent *d;

	/* If target does not exist, return true to mimic shell test */
	ఒకవేళ (fstatat(లక్ష్యం_dir, లక్ష్యం, &buf, 0) != 0)
		తిరిగి true;

	ఒకవేళ (newer) {
		ఒకవేళ (*rel < buf.st_mtime) {
			retval = false;

			ఒకవేళ (file)
				strlcpy(file, లక్ష్యం, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} లేకపోతే {
		ఒకవేళ (*rel > buf.st_mtime) {
			retval = false;

			ఒకవేళ (file)
				strlcpy(file, లక్ష్యం, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ఒకవేళ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(లక్ష్యం_dir, లక్ష్యం)))
		తిరిగి retval;

	/* Check all the entries in the dir */
	అయితే ((d = readdir(dp))) {
		ఒకవేళ (d->d_name[0] == '.')
			కొనసాగు;
		ఒకవేళ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	తిరిగి retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
స్థిర bool
mtime_check(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *ఆధారం, స్థిరరాశి అక్షరం *లక్ష్యం, bool newer, time_t *rel, అక్షరం *file)
{
	నిర్మాణం stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	ఒకవేళ (stat(ఆధారం, &buf) != 0)
		తిరిగి false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, లక్ష్యం,newer,&mtime,file);
	ఒకవేళ (rel) {
		*rel = mtime;
	}
	తిరిగి retval;
}

bool
rc_newer_than(స్థిరరాశి అక్షరం *ఆధారం, స్థిరరాశి అక్షరం *లక్ష్యం, time_t *newest, అక్షరం *file)
{

	తిరిగి mtime_check(AT_FDCWD, ఆధారం, లక్ష్యం, true, newest, file);
}

bool
rc_older_than(స్థిరరాశి అక్షరం *ఆధారం, స్థిరరాశి అక్షరం *లక్ష్యం, time_t *oldest, అక్షరం *file)
{
	తిరిగి mtime_check(AT_FDCWD, ఆధారం, లక్ష్యం, false, oldest, file);
}

రకనిర్వచనం నిర్మాణం deppair
{
	స్థిరరాశి అక్షరం *depend;
	స్థిరరాశి అక్షరం *addto;
} DEPPAIR;

స్థిర స్థిరరాశి DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ఖాళీ, ఖాళీ }
};

bool
rc_deptree_update_needed(time_t *newest, అక్షరం *file)
{
	bool newer = false;
	స్థిరరాశి పూర్ణాంకం *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	నిర్మాణం stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	ఒకవేళ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ఒకవేళ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} లేకపోతే {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ఖాళీ);
	}

	కొరకు (size_t i = 0, లెక్క = rc_scriptdirfds(&dirfds); i < లెక్క; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ఒకవేళ (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->విలువ, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	ఒకవేళ ((newer) && (newest != ఖాళీ)) {
	    *newest = mtime;
	}

	తిరిగి newer;
}

స్థిర శూన్య
అమరిక_environment(శూన్య)
{
	నిర్మాణం utsname uts;
	size_t env_కొలత;
	అక్షరం *env;
	FILE *mem = xopen_memstream(&env, &env_కొలత);

	కొరకు (స్థిరరాశి అక్షరం * స్థిరరాశి *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		ఒకవేళ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	విడుదల(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ఒకవేళ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ఒకవేళ (uname(&uts) == 0)
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
rc_deptree_update(శూన్య)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = ఖాళీ, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ఖాళీ, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *సందర్శించిన;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	అక్షరం *పంక్తి = ఖాళీ;
	size_t కొలత;
	అక్షరం *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	స్థిరరాశి అక్షరం *sys = rc_sys();
	పూర్ణాంకం serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	అమరిక_environment();
	ఒకవేళ (!(fp = popen(GENDEP, "r")))
		తిరిగి false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	అయితే (xgetline(&పంక్తి, &కొలత, fp) != -1) {
		depends = పంక్తి;
		service = strsep(&depends, " ");
		ఒకవేళ (!service || !*service)
			కొనసాగు;

		type = strsep(&depends, " ");
		ఒకవేళ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ఖాళీ;
			depinfo = get_depinfo(deptree, service);
			ఒకవేళ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ఒకవేళ (!type || !depends)
			కొనసాగు;

		/* Get the type */
		ఒకవేళ (strcmp(type, "config") != 0) {
			ఒకవేళ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ఒకవేళ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		అయితే ((depend = strsep(&depends, " "))) {
			ఒకవేళ (depend[0] == 0)
				కొనసాగు;

			ఒకవేళ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				కొనసాగు;
			}

			/* Don't depend on ourself */
			ఒకవేళ (strcmp(depend, service) == 0)
				కొనసాగు;

			/* .sh files are not init scripts */
			l = అక్షరశ్రేణి_పొడవు(depend);
			ఒకవేళ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				కొనసాగు;

			/* Remove our dependency if instructed */
			ఒకవేళ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				కొనసాగు;
			}

			rc_stringlist_కూడు(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ఒకవేళ (strcmp(type, "ibefore") == 0) {
				ఒకవేళ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ఒకవేళ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ఒకవేళ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	విడుదల(పంక్తి);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ఒకవేళ (sys) {
		అక్షరం *nosys, *onosys;
		size_t len = అక్షరశ్రేణి_పొడవు(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		కొరకు (i = 0; i < len; i++)
			nosys[i + 1] = (అక్షరం)tolower((గుర్తులేని అక్షరం)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		కొరకు (i = 0; i < len; i++)
			onosys[i + 2] = (అక్షరం)tolower((గుర్తులేని అక్షరం)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_సురక్షిత(depinfo, deptree, entries, depinfo_np) {
			ఒకవేళ (!(deptype = get_deptype(depinfo, "keyword")))
				కొనసాగు;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ఒకవేళ (strcmp(s->విలువ, nosys) != 0 && strcmp(s->విలువ, onosys) != 0)
					కొనసాగు;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_సురక్షిత(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ఒకవేళ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->విలువ);
						ఒకవేళ (!TAILQ_మొదటి(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							విడుదల(dt->type);
							విడుదల(dt->services);
							విడుదల(dt);
						}
					}
				}
			}
		}
		విడుదల(nosys);
		విడుదల(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(పరిమాణం(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ఒకవేళ (!(deptype = get_deptype(depinfo, "iprovide")))
			కొనసాగు;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->విలువ);
			ఒకవేళ (!di)
				di = make_depinfo(providers, s->విలువ);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	విడుదల(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		కొరకు (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ఒకవేళ (!deptype)
				కొనసాగు;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->విలువ);
				ఒకవేళ (!di) {
					ఒకవేళ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->విలువ);
						dt = get_deptype(depinfo, "broken");
						ఒకవేళ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->విలువ);
					}
					కొనసాగు;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ఒకవేళ (!dt)
					dt = make_deptype(di, deppairs[i].addto);
				rc_stringlist_addu(dt->services, depinfo->service);
			}
		}
	}

	/* Phase 5 - Remove broken before directives */
	types = rc_stringlist_new();
	rc_stringlist_కూడు(types, "ineed");
	rc_stringlist_కూడు(types, "iwant");
	rc_stringlist_కూడు(types, "iuse");
	rc_stringlist_కూడు(types, "iafter");
	TAILQ_FOREACH(depinfo, deptree, entries) {
		deptype = get_deptype(depinfo, "ibefore");
		ఒకవేళ (!deptype)
			కొనసాగు;
		sorted = rc_stringlist_new();
		సందర్శించిన = rc_stringlist_new();
		సందర్శించు_service(deptree, types, sorted, సందర్శించిన, depinfo,
			      ఖాళీ, 0);
		rc_stringlist_free(సందర్శించిన);
		TAILQ_FOREACH_సురక్షిత(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->విలువ);
				ఒకవేళ (!di)
					కొనసాగు;
				ఒకవేళ (strcmp(s2->విలువ, s3->విలువ) == 0) {
					dt = get_deptype(di, "iafter");
					ఒకవేళ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					ఆపు;
				}
				dt = get_deptype(di, "iprovide");
				ఒకవేళ (!dt)
					కొనసాగు;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ఒకవేళ (strcmp(s4->విలువ, s2->విలువ) == 0)
						ఆపు;
				}
				ఒకవేళ (s4) {
					di = get_depinfo(deptree, s4->విలువ);
					ఒకవేళ (di) {
						dt = get_deptype(di, "iafter");
						ఒకవేళ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					ఆపు;
				}
			}
			ఒకవేళ (s3)
				rc_stringlist_delete(deptype->services, s2->విలువ);
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
		ఒకవేళ (errno == EEXIST) {
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
	ఒకవేళ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->విలువ);
				}
			}
			i++;
		}
		fclose(fp);
	} లేకపోతే {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ఒకవేళ (TAILQ_మొదటి(config)) {
		ఒకవేళ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->విలువ);
			fclose(fp);
		} లేకపోతే {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} లేకపోతే {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	తిరిగి retval;
}
