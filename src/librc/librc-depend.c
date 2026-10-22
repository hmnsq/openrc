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

#अन्तर्भाव <sys/utsname.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "queue.h"
#अन्तर्भाव "librc.h"
#अन्तर्भाव "helpers.h"
#अन्तर्भाव "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

स्थिर ध्रुव वर्ण *bootlevel = रिक्त;

स्थिर वर्ण *
get_shell_मानम्(वर्ण *string)
{
	वर्ण *p = string;
	वर्ण *e;

	जइ (!string)
		प्रत्यागम रिक्त;

	जइ (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	जइ (*e == '\n')
		*e-- = 0;
	जइ (*e == '\'')
		*e-- = 0;

	जइ (*p != 0)
		प्रत्यागम p;

	प्रत्यागम रिक्त;
}

शून्य
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	जइ (!deptree)
		प्रत्यागम;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			मुक्त(dt->type);
			मुक्त(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		मुक्त(di->service);
		मुक्त(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	मुक्त(deptree);
}

स्थिर RC_DEPINFO *
get_depinfo(ध्रुव RC_DEPTREE *deptree, ध्रुव वर्ण *service)
{
	RC_DEPINFO *di;
	जइ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			जइ (strcmp(di->service, service) == 0)
				प्रत्यागम di;
	}
	प्रत्यागम रिक्त;
}

स्थिर RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ध्रुव वर्ण *service)
{
	RC_DEPINFO *depinfo = xmalloc(परिमाण(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_निवेशय_TAIL(deptree, depinfo, entries);

	प्रत्यागम depinfo;
}

स्थिर RC_DEPTYPE *
get_deptype(ध्रुव RC_DEPINFO *depinfo, ध्रुव वर्ण *type)
{
	RC_DEPTYPE *dt;

	जइ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			जइ (strcmp(dt->type, type) == 0)
				प्रत्यागम dt;
	}
	प्रत्यागम रिक्त;
}

स्थिर RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ध्रुव वर्ण *type)
{
	RC_DEPTYPE *deptype = xmalloc(परिमाण(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_निवेशय_TAIL(&depinfo->depends, deptype, entries);

	प्रत्यागम deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
स्थिर RC_DEPTREE *
make_deptree(शून्य) {
	RC_DEPTREE *deptree = xmalloc(परिमाण(*deptree));
	TAILQ_INIT(deptree);
	प्रत्यागम deptree;
}

स्थिर RC_DEPTREE *
deptree_load_file(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = रिक्त;
	RC_DEPTYPE *deptype = रिक्त;
	वर्ण *पङ्क्तिः = रिक्त;
	size_t size;
	वर्ण *type;
	वर्ण *p;
	वर्ण *e;
	पूर्णाङ्क i;
	FILE *fp;

	जइ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		प्रत्यागम रिक्त;

	deptree = make_deptree();
	जाव (xgetline(&पङ्क्तिः, &size, fp) != -1) {
		p = पङ्क्तिः;
		e = strsep(&p, "_");
		जइ (!e || strcmp(e, "depinfo") != 0)
			अनुवर्त;
		e = strsep(&p, "_");
		जइ (!e || sscanf(e, "%d", &i) != 1)
			अनुवर्त;
		जइ (!(type = strsep(&p, "_=")))
			अनुवर्त;
		जइ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_मानम्(p);
			जइ (!e || *e == '\0')
				अनुवर्त;
			depinfo = make_depinfo(deptree, e);
			deptype = रिक्त;
			अनुवर्त;
		}
		e = strsep(&p, "=");
		जइ (!e || sscanf(e, "%d", &i) != 1)
			अनुवर्त;
		/* Sanity */
		e = get_shell_मानम्(p);
		जइ (!e || *e == '\0')
			अनुवर्त;
		जइ (!depinfo)
			अनुवर्त;
		जइ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	मुक्त(पङ्क्तिः);
	fclose(fp);

	प्रत्यागम deptree;
}

RC_DEPTREE *
rc_deptree_load(शून्य)
{
	प्रत्यागम deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(ध्रुव वर्ण *deptree_file)
{
	प्रत्यागम deptree_load_file(AT_FDCWD, deptree_file);
}

स्थिर bool
valid_service(ध्रुव वर्ण *runlevel, ध्रुव वर्ण *service, ध्रुव वर्ण *type)
{
	RC_SERVICE state;

	जइ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		प्रत्यागम true;

	जइ (rc_service_in_runlevel(service, runlevel))
		प्रत्यागम true;
	जइ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    प्रत्यागम false;
	जइ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    प्रत्यागम false;
	जइ (strcmp(runlevel, bootlevel) != 0) {
		जइ (rc_service_in_runlevel(service, bootlevel))
			प्रत्यागम true;
	}

	state = rc_service_state(service);
	जइ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		प्रत्यागम true;

	प्रत्यागम false;
}

स्थिर bool
get_provided1(ध्रुव वर्ण *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ध्रुव वर्ण *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ध्रुव वर्ण *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->मानम्;
		st = rc_service_state(svc);

		जइ (level)
			ok = rc_service_in_runlevel(svc, level);
		अण्णहा जइ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		जइ (!ok)
			अनुवर्त;
		विकल्प (state) {
			पक्ष RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				भङ्ग;
			पक्ष RC_SERVICE_INACTIVE:
			पक्ष RC_SERVICE_STARTING:
			पक्ष RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				भङ्ग;
			स्वभाव:
				भङ्ग;
		}
		जइ (!ok)
			अनुवर्त;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	प्रत्यागम retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
स्थिर RC_STRINGLIST *
get_provided(ध्रुव RC_DEPINFO *depinfo, ध्रुव वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	जइ (!dt)
		प्रत्यागम providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	जइ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->मानम्);
		प्रत्यागम providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	जइ (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			जइ (rc_service_in_runlevel(service->मानम्, runlevel) ||
			    rc_service_in_runlevel(service->मानम्, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->मानम्) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->मानम्);
		जइ (TAILQ_FIRST(providers))
			प्रत्यागम providers;
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
	जइ (TAILQ_FIRST(providers)) { \
		जइ (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		प्रत्यागम providers; \
	}

	/* Anything running has to come first */
	जइ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	जइ (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STARTED))
	{ DO }
	जइ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	जइ (get_provided1(runlevel, providers, dt, रिक्त, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	जइ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		प्रत्यागम providers;
	जइ (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STARTING))
		प्रत्यागम providers;
	जइ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    प्रत्यागम providers;
	जइ (get_provided1(runlevel, providers, dt, रिक्त, false, RC_SERVICE_STARTING))
		प्रत्यागम providers;

	/* Nothing started then. OK, lets get the stopped services */
	जइ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		प्रत्यागम providers;
	जइ (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STOPPED))
	{ DO }
	जइ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		प्रत्यागम providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	जइ (service != रिक्त)
		rc_stringlist_add(providers, service->मानम्);

	प्रत्यागम providers;
}

स्थिर शून्य
visit_service(ध्रुव RC_DEPTREE *deptree,
	      ध्रुव RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      ध्रुव RC_DEPINFO *depinfo,
	      ध्रुव वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ध्रुव वर्ण *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		जइ (strcmp(type->मानम्, depinfo->service) == 0)
			प्रत्यागम;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		जइ (!(dt = get_deptype(depinfo, type->मानम्)))
			अनुवर्त;

		TAILQ_FOREACH(service, dt->services, entries) {
			जइ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->मानम्, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->मानम्);
				अनुवर्त;
			}

			जइ (!(di = get_depinfo(deptree, service->मानम्)))
				अनुवर्त;
			provided = get_provided(di, runlevel, options);

			जइ (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->मानम्);
					जइ (di && valid_service(runlevel, di->service, type->मानम्))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			अण्णहा जइ (di && valid_service(runlevel, service->मानम्, type->मानम्))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	जइ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			जइ (!(di = get_depinfo(deptree, service->मानम्)))
				अनुवर्त;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				जइ (strcmp(p->मानम्, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					भङ्ग;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	जइ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		जइ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ध्रुव RC_DEPTREE *deptree,
		  ध्रुव वर्ण *service, ध्रुव वर्ण *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	जइ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		प्रत्यागम svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->मानम्);
	प्रत्यागम svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ध्रुव RC_DEPTREE *deptree,
		   ध्रुव RC_STRINGLIST *types,
		   ध्रुव RC_STRINGLIST *services,
		   ध्रुव वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	ध्रुव RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	जइ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		जइ (!(di = get_depinfo(deptree, service->मानम्))) {
			errno = ENOENT;
			अनुवर्त;
		}
		जइ (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	प्रत्यागम sorted;
}

RC_STRINGLIST *
rc_deptree_order(ध्रुव RC_DEPTREE *deptree, ध्रुव वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	जइ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	जइ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		मुक्त(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		मुक्त(list2);
	} अण्णहा {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		जइ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			मुक्त(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			मुक्त(list2);
			/* If we're not the boot runlevel then add that too */
			जइ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				मुक्त(list2);
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
	प्रत्यागम services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
स्थिर bool
deep_mtime_check(पूर्णाङ्क target_dir, ध्रुव वर्ण *target, bool newer, time_t *rel, वर्ण *file)
{
	संरचना stat buf;
	bool retval = true;
	DIR *dp;
	संरचना dirent *d;

	/* If target does not exist, return true to mimic shell test */
	जइ (fstatat(target_dir, target, &buf, 0) != 0)
		प्रत्यागम true;

	जइ (newer) {
		जइ (*rel < buf.st_mtime) {
			retval = false;

			जइ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} अण्णहा {
		जइ (*rel > buf.st_mtime) {
			retval = false;

			जइ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	जइ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		प्रत्यागम retval;

	/* Check all the entries in the dir */
	जाव ((d = readdir(dp))) {
		जइ (d->d_name[0] == '.')
			अनुवर्त;
		जइ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	प्रत्यागम retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
स्थिर bool
mtime_check(पूर्णाङ्क dirfd, ध्रुव वर्ण *source, ध्रुव वर्ण *target, bool newer, time_t *rel, वर्ण *file)
{
	संरचना stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	जइ (stat(source, &buf) != 0)
		प्रत्यागम false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	जइ (rel) {
		*rel = mtime;
	}
	प्रत्यागम retval;
}

bool
rc_newer_than(ध्रुव वर्ण *source, ध्रुव वर्ण *target, time_t *newest, वर्ण *file)
{

	प्रत्यागम mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(ध्रुव वर्ण *source, ध्रुव वर्ण *target, time_t *oldest, वर्ण *file)
{
	प्रत्यागम mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

प्रकारनिर्देश संरचना deppair
{
	ध्रुव वर्ण *depend;
	ध्रुव वर्ण *addto;
} DEPPAIR;

स्थिर ध्रुव DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ रिक्त, रिक्त }
};

bool
rc_deptree_update_needed(time_t *newest, वर्ण *file)
{
	bool newer = false;
	ध्रुव पूर्णाङ्क *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	संरचना stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	जइ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	जइ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} अण्णहा {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(रिक्त);
	}

	प्रति (size_t i = 0, गणनम् = rc_scriptdirfds(&dirfds); i < गणनम्; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	जइ (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->मानम्, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	जइ ((newer) && (newest != रिक्त)) {
	    *newest = mtime;
	}

	प्रत्यागम newer;
}

स्थिर शून्य
setup_environment(शून्य)
{
	संरचना utsname uts;
	size_t env_size;
	वर्ण *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	प्रति (ध्रुव वर्ण * ध्रुव *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		जइ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	मुक्त(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	जइ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	जइ (uname(&uts) == 0)
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
rc_deptree_update(शून्य)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = रिक्त, *depinfo_np, *di;
	RC_DEPTYPE *deptype = रिक्त, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	वर्ण *पङ्क्तिः = रिक्त;
	size_t size;
	वर्ण *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ध्रुव वर्ण *sys = rc_sys();
	पूर्णाङ्क serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	जइ (!(fp = popen(GENDEP, "r")))
		प्रत्यागम false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	जाव (xgetline(&पङ्क्तिः, &size, fp) != -1) {
		depends = पङ्क्तिः;
		service = strsep(&depends, " ");
		जइ (!service || !*service)
			अनुवर्त;

		type = strsep(&depends, " ");
		जइ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = रिक्त;
			depinfo = get_depinfo(deptree, service);
			जइ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		जइ (!type || !depends)
			अनुवर्त;

		/* Get the type */
		जइ (strcmp(type, "config") != 0) {
			जइ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				जइ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		जाव ((depend = strsep(&depends, " "))) {
			जइ (depend[0] == 0)
				अनुवर्त;

			जइ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				अनुवर्त;
			}

			/* Don't depend on ourself */
			जइ (strcmp(depend, service) == 0)
				अनुवर्त;

			/* .sh files are not init scripts */
			l = strlen(depend);
			जइ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				अनुवर्त;

			/* Remove our dependency if instructed */
			जइ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				अनुवर्त;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			जइ (strcmp(type, "ibefore") == 0) {
				जइ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			जइ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				जइ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	मुक्त(पङ्क्तिः);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	जइ (sys) {
		वर्ण *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		प्रति (i = 0; i < len; i++)
			nosys[i + 1] = (वर्ण)tolower((unsigned वर्ण)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		प्रति (i = 0; i < len; i++)
			onosys[i + 2] = (वर्ण)tolower((unsigned वर्ण)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			जइ (!(deptype = get_deptype(depinfo, "keyword")))
				अनुवर्त;
			TAILQ_FOREACH(s, deptype->services, entries) {
				जइ (strcmp(s->मानम्, nosys) != 0 && strcmp(s->मानम्, onosys) != 0)
					अनुवर्त;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						जइ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->मानम्);
						जइ (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							मुक्त(dt->type);
							मुक्त(dt->services);
							मुक्त(dt);
						}
					}
				}
			}
		}
		मुक्त(nosys);
		मुक्त(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(परिमाण(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		जइ (!(deptype = get_deptype(depinfo, "iprovide")))
			अनुवर्त;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->मानम्);
			जइ (!di)
				di = make_depinfo(providers, s->मानम्);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	मुक्त(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		प्रति (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			जइ (!deptype)
				अनुवर्त;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->मानम्);
				जइ (!di) {
					जइ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->मानम्);
						dt = get_deptype(depinfo, "broken");
						जइ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->मानम्);
					}
					अनुवर्त;
				}

				dt = get_deptype(di, deppairs[i].addto);
				जइ (!dt)
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
		जइ (!deptype)
			अनुवर्त;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      रिक्त, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->मानम्);
				जइ (!di)
					अनुवर्त;
				जइ (strcmp(s2->मानम्, s3->मानम्) == 0) {
					dt = get_deptype(di, "iafter");
					जइ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					भङ्ग;
				}
				dt = get_deptype(di, "iprovide");
				जइ (!dt)
					अनुवर्त;
				TAILQ_FOREACH(s4, dt->services, entries) {
					जइ (strcmp(s4->मानम्, s2->मानम्) == 0)
						भङ्ग;
				}
				जइ (s4) {
					di = get_depinfo(deptree, s4->मानम्);
					जइ (di) {
						dt = get_deptype(di, "iafter");
						जइ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					भङ्ग;
				}
			}
			जइ (s3)
				rc_stringlist_delete(deptype->services, s2->मानम्);
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
		जइ (errno == EEXIST) {
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
	जइ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->मानम्);
				}
			}
			i++;
		}
		fclose(fp);
	} अण्णहा {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	जइ (TAILQ_FIRST(config)) {
		जइ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->मानम्);
			fclose(fp);
		} अण्णहा {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} अण्णहा {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	प्रत्यागम retval;
}
