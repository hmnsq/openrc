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

स्थिर अचर वर्ण *bootlevel = रिक्त;

स्थिर वर्ण *
get_shell_मान(वर्ण *string)
{
	वर्ण *p = string;
	वर्ण *e;

	यदि (!string)
		फर्काऊ रिक्त;

	यदि (*p == '\'')
		p++;

	e = p + म_माप(p) - 1;
	यदि (*e == '\n')
		*e-- = 0;
	यदि (*e == '\'')
		*e-- = 0;

	यदि (*p != 0)
		फर्काऊ p;

	फर्काऊ रिक्त;
}

शून्य
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	यदि (!deptree)
		फर्काऊ;

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
get_depinfo(अचर RC_DEPTREE *deptree, अचर वर्ण *service)
{
	RC_DEPINFO *di;
	यदि (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			यदि (म_भेद(di->service, service) == 0)
				फर्काऊ di;
	}
	फर्काऊ रिक्त;
}

स्थिर RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, अचर वर्ण *service)
{
	RC_DEPINFO *depinfo = xmalloc(परिमाण(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_घुसाउ_TAIL(deptree, depinfo, entries);

	फर्काऊ depinfo;
}

स्थिर RC_DEPTYPE *
get_deptype(अचर RC_DEPINFO *depinfo, अचर वर्ण *type)
{
	RC_DEPTYPE *dt;

	यदि (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			यदि (म_भेद(dt->type, type) == 0)
				फर्काऊ dt;
	}
	फर्काऊ रिक्त;
}

स्थिर RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, अचर वर्ण *type)
{
	RC_DEPTYPE *deptype = xmalloc(परिमाण(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_घुसाउ_TAIL(&depinfo->depends, deptype, entries);

	फर्काऊ deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((दो_स्मृति (rc_deptree_free, 1)))
#endif
स्थिर RC_DEPTREE *
make_deptree(शून्य) {
	RC_DEPTREE *deptree = xmalloc(परिमाण(*deptree));
	TAILQ_INIT(deptree);
	फर्काऊ deptree;
}

स्थिर RC_DEPTREE *
deptree_load_file(पूर्णाङ्क dirfd, अचर वर्ण *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = रिक्त;
	RC_DEPTYPE *deptype = रिक्त;
	वर्ण *पङ्क्ति = रिक्त;
	माप_प्रकार size;
	वर्ण *type;
	वर्ण *p;
	वर्ण *e;
	पूर्णाङ्क i;
	खाता *fp;

	यदि (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		फर्काऊ रिक्त;

	deptree = make_deptree();
	जबसम्म (xgetline(&पङ्क्ति, &size, fp) != -1) {
		p = पङ्क्ति;
		e = strsep(&p, "_");
		यदि (!e || म_भेद(e, "depinfo") != 0)
			जारी;
		e = strsep(&p, "_");
		यदि (!e || माला_पूछो(e, "%d", &i) != 1)
			जारी;
		यदि (!(type = strsep(&p, "_=")))
			जारी;
		यदि (म_भेद(type, "service") == 0) {
			/* Sanity */
			e = get_shell_मान(p);
			यदि (!e || *e == '\0')
				जारी;
			depinfo = make_depinfo(deptree, e);
			deptype = रिक्त;
			जारी;
		}
		e = strsep(&p, "=");
		यदि (!e || माला_पूछो(e, "%d", &i) != 1)
			जारी;
		/* Sanity */
		e = get_shell_मान(p);
		यदि (!e || *e == '\0')
			जारी;
		यदि (!depinfo)
			जारी;
		यदि (!deptype || म_भेद(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	मुक्त(पङ्क्ति);
	ख_बंद(fp);

	फर्काऊ deptree;
}

RC_DEPTREE *
rc_deptree_load(शून्य)
{
	फर्काऊ deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(अचर वर्ण *deptree_file)
{
	फर्काऊ deptree_load_file(AT_FDCWD, deptree_file);
}

स्थिर bool
valid_service(अचर वर्ण *runlevel, अचर वर्ण *service, अचर वर्ण *type)
{
	RC_SERVICE state;

	यदि (!runlevel ||
	    म_भेद(type, "ineed") == 0 ||
	    म_भेद(type, "needsme") == 0  ||
	    म_भेद(type, "iwant") == 0 ||
	    म_भेद(type, "wantsme") == 0)
		फर्काऊ true;

	यदि (rc_service_in_runlevel(service, runlevel))
		फर्काऊ true;
	यदि (म_भेद(runlevel, RC_LEVEL_SYSINIT) == 0)
		    फर्काऊ false;
	यदि (म_भेद(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    म_भेद(type, "iafter") == 0)
		    फर्काऊ false;
	यदि (म_भेद(runlevel, bootlevel) != 0) {
		यदि (rc_service_in_runlevel(service, bootlevel))
			फर्काऊ true;
	}

	state = rc_service_state(service);
	यदि (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		फर्काऊ true;

	फर्काऊ false;
}

स्थिर bool
get_provided1(अचर वर्ण *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, अचर वर्ण *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	अचर वर्ण *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->मान;
		st = rc_service_state(svc);

		यदि (level)
			ok = rc_service_in_runlevel(svc, level);
		अन्यथा यदि (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		यदि (!ok)
			जारी;
		विकल्प (state) {
			पक्ष RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				रोक;
			पक्ष RC_SERVICE_INACTIVE:
			पक्ष RC_SERVICE_STARTING:
			पक्ष RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				रोक;
			स्वभाव:
				रोक;
		}
		यदि (!ok)
			जारी;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	फर्काऊ retval;
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
get_provided(अचर RC_DEPINFO *depinfo, अचर वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	यदि (!dt)
		फर्काऊ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	यदि (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->मान);
		फर्काऊ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	यदि (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			यदि (rc_service_in_runlevel(service->मान, runlevel) ||
			    rc_service_in_runlevel(service->मान, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->मान) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->मान);
		यदि (TAILQ_FIRST(providers))
			फर्काऊ providers;
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
	यदि (TAILQ_FIRST(providers)) { \
		यदि (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		फर्काऊ providers; \
	}

	/* Anything running has to come first */
	यदि (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	यदि (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STARTED))
	{ DO }
	यदि (bootlevel && म_भेद(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	यदि (get_provided1(runlevel, providers, dt, रिक्त, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	यदि (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		फर्काऊ providers;
	यदि (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STARTING))
		फर्काऊ providers;
	यदि (bootlevel && म_भेद(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    फर्काऊ providers;
	यदि (get_provided1(runlevel, providers, dt, रिक्त, false, RC_SERVICE_STARTING))
		फर्काऊ providers;

	/* Nothing started then. OK, lets get the stopped services */
	यदि (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		फर्काऊ providers;
	यदि (get_provided1(runlevel, providers, dt, रिक्त, true, RC_SERVICE_STOPPED))
	{ DO }
	यदि (bootlevel && (म_भेद(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		फर्काऊ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	यदि (service != रिक्त)
		rc_stringlist_add(providers, service->मान);

	फर्काऊ providers;
}

स्थिर शून्य
visit_service(अचर RC_DEPTREE *deptree,
	      अचर RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      अचर RC_DEPINFO *depinfo,
	      अचर वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	अचर वर्ण *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		यदि (म_भेद(type->मान, depinfo->service) == 0)
			फर्काऊ;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		यदि (!(dt = get_deptype(depinfo, type->मान)))
			जारी;

		TAILQ_FOREACH(service, dt->services, entries) {
			यदि (!(options & RC_DEP_TRACE) ||
			    म_भेद(type->मान, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->मान);
				जारी;
			}

			यदि (!(di = get_depinfo(deptree, service->मान)))
				जारी;
			provided = get_provided(di, runlevel, options);

			यदि (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->मान);
					यदि (di && valid_service(runlevel, di->service, type->मान))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			अन्यथा यदि (di && valid_service(runlevel, service->मान, type->मान))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	यदि (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			यदि (!(di = get_depinfo(deptree, service->मान)))
				जारी;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				यदि (म_भेद(p->मान, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					रोक;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = दो_पर्या("RC_SVCNAME");
	यदि (!svcname || म_भेद(svcname, depinfo->service) != 0) {
		यदि (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(अचर RC_DEPTREE *deptree,
		  अचर वर्ण *service, अचर वर्ण *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	यदि (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		त्रुटि_सं = ENOENT;
		फर्काऊ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->मान);
	फर्काऊ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(अचर RC_DEPTREE *deptree,
		   अचर RC_STRINGLIST *types,
		   अचर RC_STRINGLIST *services,
		   अचर वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	अचर RC_STRING *service;

	bootlevel = दो_पर्या("RC_BOOTLEVEL");
	यदि (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		यदि (!(di = get_depinfo(deptree, service->मान))) {
			त्रुटि_सं = ENOENT;
			जारी;
		}
		यदि (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	फर्काऊ sorted;
}

RC_STRINGLIST *
rc_deptree_order(अचर RC_DEPTREE *deptree, अचर वर्ण *runlevel, पूर्णाङ्क options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = दो_पर्या("RC_BOOTLEVEL");
	यदि (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	यदि (म_भेद(runlevel, RC_LEVEL_SINGLE) == 0 ||
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
		यदि (म_भेद(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			मुक्त(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			मुक्त(list2);
			/* If we're not the boot runlevel then add that too */
			यदि (म_भेद(runlevel, bootlevel) != 0) {
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
	फर्काऊ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
स्थिर bool
deep_mtime_check(पूर्णाङ्क target_dir, अचर वर्ण *target, bool newer, समय_प्रकार *rel, वर्ण *file)
{
	संरचना stat buf;
	bool retval = true;
	सूची *dp;
	संरचना dirent *d;

	/* If target does not exist, return true to mimic shell test */
	यदि (fstatat(target_dir, target, &buf, 0) != 0)
		फर्काऊ true;

	यदि (newer) {
		यदि (*rel < buf.st_mtime) {
			retval = false;

			यदि (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} अन्यथा {
		यदि (*rel > buf.st_mtime) {
			retval = false;

			यदि (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	यदि (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		फर्काऊ retval;

	/* Check all the entries in the dir */
	जबसम्म ((d = सूची_पढ़ो(dp))) {
		यदि (d->d_name[0] == '.')
			जारी;
		यदि (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	बंद_सूची(dp);
	फर्काऊ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
स्थिर bool
mtime_check(पूर्णाङ्क dirfd, अचर वर्ण *source, अचर वर्ण *target, bool newer, समय_प्रकार *rel, वर्ण *file)
{
	संरचना stat buf;
	समय_प्रकार mtime;
	bool retval = true;

	/* We have to exist */
	यदि (stat(source, &buf) != 0)
		फर्काऊ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	यदि (rel) {
		*rel = mtime;
	}
	फर्काऊ retval;
}

bool
rc_newer_than(अचर वर्ण *source, अचर वर्ण *target, समय_प्रकार *newest, वर्ण *file)
{

	फर्काऊ mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(अचर वर्ण *source, अचर वर्ण *target, समय_प्रकार *oldest, वर्ण *file)
{
	फर्काऊ mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

प्रकारनिर्देश संरचना deppair
{
	अचर वर्ण *depend;
	अचर वर्ण *addto;
} DEPPAIR;

स्थिर अचर DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ रिक्त, रिक्त }
};

bool
rc_deptree_update_needed(समय_प्रकार *newest, वर्ण *file)
{
	bool newer = false;
	अचर पूर्णाङ्क *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	संरचना stat buf;
	समय_प्रकार mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	यदि (सूची_गढ़ो(rc_svcdir(), 0755) != 0 && त्रुटि_सं != EEXIST)
		ख_लिखो(मानक_त्रुटि, "mkdir '%s': %s\n", rc_svcdir(), म_त्रुटि(त्रुटि_सं));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	यदि (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} अन्यथा {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = समय(रिक्त);
	}

	लागि (माप_प्रकार i = 0, गन्ती = rc_scriptdirfds(&dirfds); i < गन्ती; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	यदि (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->मान, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	यदि ((newer) && (newest != रिक्त)) {
	    *newest = mtime;
	}

	फर्काऊ newer;
}

स्थिर शून्य
setup_environment(शून्य)
{
	संरचना utsname uts;
	माप_प्रकार env_size;
	वर्ण *env;
	खाता *mem = xopen_memstream(&env, &env_size);

	लागि (अचर वर्ण * अचर *dirs = rc_scriptdirs(); *dirs; dirs++) {
		ख_माला_दो(*dirs, mem);
		यदि (dirs[1])
			ख_अक्षर_दो(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	मुक्त(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	यदि (!दो_पर्या("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	यदि (uname(&uts) == 0)
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
	वर्ण *पङ्क्ति = रिक्त;
	माप_प्रकार size;
	वर्ण *depend, *depends, *service, *type;
	माप_प्रकार i, l;
	bool retval = true;
	अचर वर्ण *sys = rc_sys();
	पूर्णाङ्क serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	यदि (!(fp = popen(GENDEP, "r")))
		फर्काऊ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	जबसम्म (xgetline(&पङ्क्ति, &size, fp) != -1) {
		depends = पङ्क्ति;
		service = strsep(&depends, " ");
		यदि (!service || !*service)
			जारी;

		type = strsep(&depends, " ");
		यदि (!depinfo || म_भेद(depinfo->service, service) != 0) {
			deptype = रिक्त;
			depinfo = get_depinfo(deptree, service);
			यदि (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		यदि (!type || !depends)
			जारी;

		/* Get the type */
		यदि (म_भेद(type, "config") != 0) {
			यदि (!deptype || म_भेद(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				यदि (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		जबसम्म ((depend = strsep(&depends, " "))) {
			यदि (depend[0] == 0)
				जारी;

			यदि (म_भेद(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				जारी;
			}

			/* Don't depend on ourself */
			यदि (म_भेद(depend, service) == 0)
				जारी;

			/* .sh files are not init scripts */
			l = म_माप(depend);
			यदि (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				जारी;

			/* Remove our dependency if instructed */
			यदि (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				जारी;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			यदि (म_भेद(type, "ibefore") == 0) {
				यदि ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			यदि (म_भेद(type, "iafter") == 0 ||
			    म_भेद(type, "ineed") == 0 ||
			    म_भेद(type, "iwant") == 0 ||
			    म_भेद(type, "iuse") == 0) {
				यदि ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	मुक्त(पङ्क्ति);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	यदि (sys) {
		वर्ण *nosys, *onosys;
		माप_प्रकार len = म_माप(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		लागि (i = 0; i < len; i++)
			nosys[i + 1] = (वर्ण)छोटे((अचिन्हित वर्ण)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		लागि (i = 0; i < len; i++)
			onosys[i + 2] = (वर्ण)छोटे((अचिन्हित वर्ण)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			यदि (!(deptype = get_deptype(depinfo, "keyword")))
				जारी;
			TAILQ_FOREACH(s, deptype->services, entries) {
				यदि (म_भेद(s->मान, nosys) != 0 && म_भेद(s->मान, onosys) != 0)
					जारी;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						यदि (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->मान);
						यदि (!TAILQ_FIRST(dt->services)) {
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
		यदि (!(deptype = get_deptype(depinfo, "iprovide")))
			जारी;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->मान);
			यदि (!di)
				di = make_depinfo(providers, s->मान);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	मुक्त(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		लागि (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			यदि (!deptype)
				जारी;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->मान);
				यदि (!di) {
					यदि (म_भेद(deptype->type, "ineed") == 0) {
						ख_लिखो(मानक_त्रुटि, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->मान);
						dt = get_deptype(depinfo, "broken");
						यदि (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->मान);
					}
					जारी;
				}

				dt = get_deptype(di, deppairs[i].addto);
				यदि (!dt)
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
		यदि (!deptype)
			जारी;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      रिक्त, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->मान);
				यदि (!di)
					जारी;
				यदि (म_भेद(s2->मान, s3->मान) == 0) {
					dt = get_deptype(di, "iafter");
					यदि (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					रोक;
				}
				dt = get_deptype(di, "iprovide");
				यदि (!dt)
					जारी;
				TAILQ_FOREACH(s4, dt->services, entries) {
					यदि (म_भेद(s4->मान, s2->मान) == 0)
						रोक;
				}
				यदि (s4) {
					di = get_depinfo(deptree, s4->मान);
					यदि (di) {
						dt = get_deptype(di, "iafter");
						यदि (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					रोक;
				}
			}
			यदि (s3)
				rc_stringlist_delete(deptype->services, s2->मान);
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
		यदि (त्रुटि_सं == EEXIST) {
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
	यदि ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			ख_लिखो(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				माप_प्रकार k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					ख_लिखो(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->मान);
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
	यदि (TAILQ_FIRST(config)) {
		यदि ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				ख_लिखो(fp, "%s\n", s->मान);
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
	फर्काऊ retval;
}
