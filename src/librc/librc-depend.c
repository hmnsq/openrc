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

स्थिर स्थिरांक अक्षर *bootlevel = रिक्त;

स्थिर अक्षर *
get_shell_मूल्य(अक्षर *string)
{
	अक्षर *p = string;
	अक्षर *e;

	जर (!string)
		परत रिक्त;

	जर (*p == '\'')
		p++;

	e = p + म_माप(p) - 1;
	जर (*e == '\n')
		*e-- = 0;
	जर (*e == '\'')
		*e-- = 0;

	जर (*p != 0)
		परत p;

	परत रिक्त;
}

शून्य
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	जर (!deptree)
		परत;

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
get_depinfo(स्थिरांक RC_DEPTREE *deptree, स्थिरांक अक्षर *service)
{
	RC_DEPINFO *di;
	जर (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			जर (म_भेद(di->service, service) == 0)
				परत di;
	}
	परत रिक्त;
}

स्थिर RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, स्थिरांक अक्षर *service)
{
	RC_DEPINFO *depinfo = xmalloc(आकार(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_घाला_TAIL(deptree, depinfo, entries);

	परत depinfo;
}

स्थिर RC_DEPTYPE *
get_deptype(स्थिरांक RC_DEPINFO *depinfo, स्थिरांक अक्षर *type)
{
	RC_DEPTYPE *dt;

	जर (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			जर (म_भेद(dt->type, type) == 0)
				परत dt;
	}
	परत रिक्त;
}

स्थिर RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, स्थिरांक अक्षर *type)
{
	RC_DEPTYPE *deptype = xmalloc(आकार(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_घाला_TAIL(&depinfo->depends, deptype, entries);

	परत deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((दो_स्मृति (rc_deptree_free, 1)))
#endif
स्थिर RC_DEPTREE *
make_deptree(शून्य) {
	RC_DEPTREE *deptree = xmalloc(आकार(*deptree));
	TAILQ_INIT(deptree);
	परत deptree;
}

स्थिर RC_DEPTREE *
deptree_load_file(पूर्णांक dirfd, स्थिरांक अक्षर *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = रिक्त;
	RC_DEPTYPE *deptype = रिक्त;
	अक्षर *ओळ = रिक्त;
	माप_प्रकार size;
	अक्षर *type;
	अक्षर *p;
	अक्षर *e;
	पूर्णांक i;
	खाता *fp;

	जर (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		परत रिक्त;

	deptree = make_deptree();
	पर्यंत (xgetline(&ओळ, &size, fp) != -1) {
		p = ओळ;
		e = strsep(&p, "_");
		जर (!e || म_भेद(e, "depinfo") != 0)
			सुरूठेवा;
		e = strsep(&p, "_");
		जर (!e || माला_पूछो(e, "%d", &i) != 1)
			सुरूठेवा;
		जर (!(type = strsep(&p, "_=")))
			सुरूठेवा;
		जर (म_भेद(type, "service") == 0) {
			/* Sanity */
			e = get_shell_मूल्य(p);
			जर (!e || *e == '\0')
				सुरूठेवा;
			depinfo = make_depinfo(deptree, e);
			deptype = रिक्त;
			सुरूठेवा;
		}
		e = strsep(&p, "=");
		जर (!e || माला_पूछो(e, "%d", &i) != 1)
			सुरूठेवा;
		/* Sanity */
		e = get_shell_मूल्य(p);
		जर (!e || *e == '\0')
			सुरूठेवा;
		जर (!depinfo)
			सुरूठेवा;
		जर (!deptype || म_भेद(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	मुक्त(ओळ);
	ख_बंद(fp);

	परत deptree;
}

RC_DEPTREE *
rc_deptree_load(शून्य)
{
	परत deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(स्थिरांक अक्षर *deptree_file)
{
	परत deptree_load_file(AT_FDCWD, deptree_file);
}

स्थिर bool
valid_service(स्थिरांक अक्षर *runlevel, स्थिरांक अक्षर *service, स्थिरांक अक्षर *type)
{
	RC_SERVICE state;

	जर (!runlevel ||
	    म_भेद(type, "ineed") == 0 ||
	    म_भेद(type, "needsme") == 0  ||
	    म_भेद(type, "iwant") == 0 ||
	    म_भेद(type, "wantsme") == 0)
		परत true;

	जर (rc_service_in_runlevel(service, runlevel))
		परत true;
	जर (म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0)
		    परत false;
	जर (म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    म_भेद(type, "iafter") == 0)
		    परत false;
	जर (म_भेद(runlevel, bootlevel) != 0) {
		जर (rc_service_in_runlevel(service, bootlevel))
			परत true;
	}

	state = rc_service_state(service);
	जर (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		परत true;

	परत false;
}

स्थिर bool
get_provided1(स्थिरांक अक्षर *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, स्थिरांक अक्षर *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	स्थिरांक अक्षर *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->मूल्य;
		st = rc_service_state(svc);

		जर (level)
			ok = rc_service_in_runlevel(svc, level);
		अन्यथा जर (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		जर (!ok)
			सुरूठेवा;
		निवड (state) {
			स्थिती RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				थांबा;
			स्थिती RC_SERVICE_INACTIVE:
			स्थिती RC_SERVICE_STARTING:
			स्थिती RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				थांबा;
			पूर्वनिर्धारित:
				थांबा;
		}
		जर (!ok)
			सुरूठेवा;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	परत retval;
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
get_provided(स्थिरांक RC_DEPINFO *depinfo, स्थिरांक अक्षर *runlevel, पूर्णांक options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	जर (!dt)
		परत providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	जर (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->मूल्य);
		परत providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	जर (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			जर (rc_service_in_runlevel(service->मूल्य, runlevel) ||
			    rc_service_in_runlevel(service->मूल्य, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->मूल्य) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->मूल्य);
		जर (TAILQ_FIRST(providers))
			परत providers;
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
	जर (TAILQ_FIRST(providers)) { \
		जर (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		परत providers; \
	}

	/* Anything running has to come first */
	जर (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	जर (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STARTED))
	{ DO }
	जर (bootlevel && म_भेद(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	जर (get_provided1(runlevel, providers, dt, रिक्त, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	जर (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		परत providers;
	जर (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STARTING))
		परत providers;
	जर (bootlevel && म_भेद(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    परत providers;
	जर (get_provided1(runlevel, providers, dt, रिक्त, false, RC_SERVICE_STARTING))
		परत providers;

	/* Nothing started then. OK, lets get the stopped services */
	जर (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		परत providers;
	जर (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STOPPED))
	{ DO }
	जर (bootlevel && (म_भेद(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		परत providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	जर (service != रिक्त)
		rc_stringlist_add(providers, service->मूल्य);

	परत providers;
}

स्थिर शून्य
visit_service(स्थिरांक RC_DEPTREE *deptree,
	      स्थिरांक RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      स्थिरांक RC_DEPINFO *depinfo,
	      स्थिरांक अक्षर *runlevel, पूर्णांक options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	स्थिरांक अक्षर *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		जर (म_भेद(type->मूल्य, depinfo->service) == 0)
			परत;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		जर (!(dt = get_deptype(depinfo, type->मूल्य)))
			सुरूठेवा;

		TAILQ_FOREACH(service, dt->services, entries) {
			जर (!(options & RC_DEP_TRACE) ||
			    म_भेद(type->मूल्य, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->मूल्य);
				सुरूठेवा;
			}

			जर (!(di = get_depinfo(deptree, service->मूल्य)))
				सुरूठेवा;
			provided = get_provided(di, runlevel, options);

			जर (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->मूल्य);
					जर (di && valid_service(runlevel, di->service, type->मूल्य))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			अन्यथा जर (di && valid_service(runlevel, service->मूल्य, type->मूल्य))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	जर (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			जर (!(di = get_depinfo(deptree, service->मूल्य)))
				सुरूठेवा;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				जर (म_भेद(p->मूल्य, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					थांबा;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = दो_पर्या("RC_SVCNAME");
	जर (!svcname || म_भेद(svcname, depinfo->service) != 0) {
		जर (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(स्थिरांक RC_DEPTREE *deptree,
		  स्थिरांक अक्षर *service, स्थिरांक अक्षर *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	जर (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		त्रुटि_सं = ENOENT;
		परत svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->मूल्य);
	परत svcs;
}

RC_STRINGLIST *
rc_deptree_depends(स्थिरांक RC_DEPTREE *deptree,
		   स्थिरांक RC_STRINGLIST *types,
		   स्थिरांक RC_STRINGLIST *services,
		   स्थिरांक अक्षर *runlevel, पूर्णांक options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	स्थिरांक RC_STRING *service;

	bootlevel = दो_पर्या("RC_BOOTLEVEL");
	जर (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		जर (!(di = get_depinfo(deptree, service->मूल्य))) {
			त्रुटि_सं = ENOENT;
			सुरूठेवा;
		}
		जर (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	परत sorted;
}

RC_STRINGLIST *
rc_deptree_order(स्थिरांक RC_DEPTREE *deptree, स्थिरांक अक्षर *runlevel, पूर्णांक options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = दो_पर्या("RC_BOOTLEVEL");
	जर (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	जर (म_भेद(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		मुक्त(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		मुक्त(list2);
	} अन्यथा {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		जर (म_भेद(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			मुक्त(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			मुक्त(list2);
			/* If we're not the boot runlevel then add that too */
			जर (म_भेद(runlevel, bootlevel) != 0) {
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
	परत services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
स्थिर bool
deep_mtime_check(पूर्णांक target_dir, स्थिरांक अक्षर *target, bool newer, समय_प्रकार *rel, अक्षर *file)
{
	संरचना stat buf;
	bool retval = true;
	सूची *dp;
	संरचना dirent *d;

	/* If target does not exist, return true to mimic shell test */
	जर (fstatat(target_dir, target, &buf, 0) != 0)
		परत true;

	जर (newer) {
		जर (*rel < buf.st_mtime) {
			retval = false;

			जर (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} अन्यथा {
		जर (*rel > buf.st_mtime) {
			retval = false;

			जर (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	जर (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		परत retval;

	/* Check all the entries in the dir */
	पर्यंत ((d = सूची_पढ़ो(dp))) {
		जर (d->d_name[0] == '.')
			सुरूठेवा;
		जर (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	बंद_सूची(dp);
	परत retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
स्थिर bool
mtime_check(पूर्णांक dirfd, स्थिरांक अक्षर *source, स्थिरांक अक्षर *target, bool newer, समय_प्रकार *rel, अक्षर *file)
{
	संरचना stat buf;
	समय_प्रकार mtime;
	bool retval = true;

	/* We have to exist */
	जर (stat(source, &buf) != 0)
		परत false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	जर (rel) {
		*rel = mtime;
	}
	परत retval;
}

bool
rc_newer_than(स्थिरांक अक्षर *source, स्थिरांक अक्षर *target, समय_प्रकार *newest, अक्षर *file)
{

	परत mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(स्थिरांक अक्षर *source, स्थिरांक अक्षर *target, समय_प्रकार *oldest, अक्षर *file)
{
	परत mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

प्रकारव्याख्या संरचना deppair
{
	स्थिरांक अक्षर *depend;
	स्थिरांक अक्षर *addto;
} DEPPAIR;

स्थिर स्थिरांक DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ रिक्त, रिक्त }
};

bool
rc_deptree_update_needed(समय_प्रकार *newest, अक्षर *file)
{
	bool newer = false;
	स्थिरांक पूर्णांक *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	संरचना stat buf;
	समय_प्रकार mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	जर (सूची_गढ़ो(rc_svcdir(), 0755) != 0 && त्रुटि_सं != EEXIST)
		ख_लिखो(मानक_त्रुटि, "mkdir '%s': %s\n", rc_svcdir(), म_त्रुटि(त्रुटि_सं));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	जर (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} अन्यथा {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = समय(रिक्त);
	}

	साठी (माप_प्रकार i = 0, मोजणी = rc_scriptdirfds(&dirfds); i < मोजणी; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	जर (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->मूल्य, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	जर ((newer) && (newest != रिक्त)) {
	    *newest = mtime;
	}

	परत newer;
}

स्थिर शून्य
setup_environment(शून्य)
{
	संरचना utsname uts;
	माप_प्रकार env_size;
	अक्षर *env;
	खाता *mem = xopen_memstream(&env, &env_size);

	साठी (स्थिरांक अक्षर * स्थिरांक *dirs = rc_scriptdirs(); *dirs; dirs++) {
		ख_माला_दो(*dirs, mem);
		जर (dirs[1])
			ख_अक्षर_दो(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	मुक्त(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	जर (!दो_पर्या("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	जर (uname(&uts) == 0)
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

	खाता *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = रिक्त, *depinfo_np, *di;
	RC_DEPTYPE *deptype = रिक्त, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	अक्षर *ओळ = रिक्त;
	माप_प्रकार size;
	अक्षर *depend, *depends, *service, *type;
	माप_प्रकार i, l;
	bool retval = true;
	स्थिरांक अक्षर *sys = rc_sys();
	पूर्णांक serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	जर (!(fp = popen(GENDEP, "r")))
		परत false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	पर्यंत (xgetline(&ओळ, &size, fp) != -1) {
		depends = ओळ;
		service = strsep(&depends, " ");
		जर (!service || !*service)
			सुरूठेवा;

		type = strsep(&depends, " ");
		जर (!depinfo || म_भेद(depinfo->service, service) != 0) {
			deptype = रिक्त;
			depinfo = get_depinfo(deptree, service);
			जर (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		जर (!type || !depends)
			सुरूठेवा;

		/* Get the type */
		जर (म_भेद(type, "config") != 0) {
			जर (!deptype || म_भेद(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				जर (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		पर्यंत ((depend = strsep(&depends, " "))) {
			जर (depend[0] == 0)
				सुरूठेवा;

			जर (म_भेद(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				सुरूठेवा;
			}

			/* Don't depend on ourself */
			जर (म_भेद(depend, service) == 0)
				सुरूठेवा;

			/* .sh files are not init scripts */
			l = म_माप(depend);
			जर (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				सुरूठेवा;

			/* Remove our dependency if instructed */
			जर (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				सुरूठेवा;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			जर (म_भेद(type, "ibefore") == 0) {
				जर ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			जर (म_भेद(type, "iafter") == 0 ||
			    म_भेद(type, "ineed") == 0 ||
			    म_भेद(type, "iwant") == 0 ||
			    म_भेद(type, "iuse") == 0) {
				जर ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	मुक्त(ओळ);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	जर (sys) {
		अक्षर *nosys, *onosys;
		माप_प्रकार len = म_माप(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		साठी (i = 0; i < len; i++)
			nosys[i + 1] = (अक्षर)छोटे((अचिन्हित अक्षर)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		साठी (i = 0; i < len; i++)
			onosys[i + 2] = (अक्षर)छोटे((अचिन्हित अक्षर)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			जर (!(deptype = get_deptype(depinfo, "keyword")))
				सुरूठेवा;
			TAILQ_FOREACH(s, deptype->services, entries) {
				जर (म_भेद(s->मूल्य, nosys) != 0 && म_भेद(s->मूल्य, onosys) != 0)
					सुरूठेवा;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						जर (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->मूल्य);
						जर (!TAILQ_FIRST(dt->services)) {
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
	providers = xmalloc(आकार(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		जर (!(deptype = get_deptype(depinfo, "iprovide")))
			सुरूठेवा;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->मूल्य);
			जर (!di)
				di = make_depinfo(providers, s->मूल्य);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	मुक्त(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		साठी (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			जर (!deptype)
				सुरूठेवा;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->मूल्य);
				जर (!di) {
					जर (म_भेद(deptype->type, "ineed") == 0) {
						ख_लिखो(मानक_त्रुटि, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->मूल्य);
						dt = get_deptype(depinfo, "broken");
						जर (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->मूल्य);
					}
					सुरूठेवा;
				}

				dt = get_deptype(di, deppairs[i].addto);
				जर (!dt)
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
		जर (!deptype)
			सुरूठेवा;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      रिक्त, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->मूल्य);
				जर (!di)
					सुरूठेवा;
				जर (म_भेद(s2->मूल्य, s3->मूल्य) == 0) {
					dt = get_deptype(di, "iafter");
					जर (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					थांबा;
				}
				dt = get_deptype(di, "iprovide");
				जर (!dt)
					सुरूठेवा;
				TAILQ_FOREACH(s4, dt->services, entries) {
					जर (म_भेद(s4->मूल्य, s2->मूल्य) == 0)
						थांबा;
				}
				जर (s4) {
					di = get_depinfo(deptree, s4->मूल्य);
					जर (di) {
						dt = get_deptype(di, "iafter");
						जर (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					थांबा;
				}
			}
			जर (s3)
				rc_stringlist_delete(deptype->services, s2->मूल्य);
		}
		rc_stringlist_free(sorted);
	}
	rc_stringlist_free(types);

	/* Phase 6 - Print errors for duplicate services */
	dupes = rc_stringlist_new();
	TAILQ_FOREACH(depinfo, deptree, entries) {
		serrno = त्रुटि_सं;
		त्रुटि_सं = 0;
		rc_stringlist_addu(dupes,depinfo->service);
		जर (त्रुटि_सं == EEXIST) {
			ख_लिखो(मानक_त्रुटि,
					"Error: %s is the name of a real and virtual service.\n",
					depinfo->service);
		}
		त्रुटि_सं = serrno;
	}
	rc_stringlist_free(dupes);

	/* Phase 7 - save to disk
	   Now that we're purely in C, do we need to keep a shell parseable file?
	   I think yes as then it stays human readable
	   This works and should be entirely shell parseable provided that depend
	   names don't have any non shell variable characters in
	   */
	जर ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			ख_लिखो(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				माप_प्रकार k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					ख_लिखो(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->मूल्य);
				}
			}
			i++;
		}
		ख_बंद(fp);
	} अन्यथा {
		ख_लिखो(मानक_त्रुटि, "fopen '%s/deptree': %s\n", rc_svcdir(), म_त्रुटि(त्रुटि_सं));
		retval = false;
	}

	/* Save our external config files to disk */
	जर (TAILQ_FIRST(config)) {
		जर ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				ख_लिखो(fp, "%s\n", s->मूल्य);
			ख_बंद(fp);
		} अन्यथा {
			ख_लिखो(मानक_त्रुटि, "fopen '%s/depconfig': %s\n", rc_svcdir(), म_त्रुटि(त्रुटि_सं));
			retval = false;
		}
	} अन्यथा {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	परत retval;
}
