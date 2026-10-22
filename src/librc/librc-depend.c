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

#include <sys/utsname.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "queue.h"
#include "librc.h"
#include "helpers.h"
#include "misc.h"

#define GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

static const var_na *bootlevel = NULL;

static var_na *
get_shell_value(var_na *string)
{
	var_na *p = string;
	var_na *e;

	ᯀᯮᯝ (!string)
		ᯀᯮᯞ NULL;

	ᯀᯮᯝ (*p == '\'')
		p++;

	e = p + strlen(p) - 1;
	ᯀᯮᯝ (*e == '\n')
		*e-- = 0;
	ᯀᯮᯝ (*e == '\'')
		*e-- = 0;

	ᯀᯮᯝ (*p != 0)
		ᯀᯮᯞ p;

	ᯀᯮᯞ NULL;
}

void
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ᯀᯮᯝ (!deptree)
		ᯀᯮᯞ;

	TAILQ_FOREACH_SAFE(di, deptree, entries, di_save) {
		TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			free(dt->type);
			free(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		free(di->service);
		free(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	free(deptree);
}

static RC_DEPINFO *
get_depinfo(const RC_DEPTREE *deptree, const var_na *service)
{
	RC_DEPINFO *di;
	ᯀᯮᯝ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ᯀᯮᯝ (strcmp(di->service, service) == 0)
				ᯀᯮᯞ di;
	}
	ᯀᯮᯞ NULL;
}

static RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, const var_na *service)
{
	RC_DEPINFO *depinfo = xmalloc(parimaa_na(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_INSERT_TAIL(deptree, depinfo, entries);

	ᯀᯮᯞ depinfo;
}

static RC_DEPTYPE *
get_deptype(const RC_DEPINFO *depinfo, const var_na *type)
{
	RC_DEPTYPE *dt;

	ᯀᯮᯝ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ᯀᯮᯝ (strcmp(dt->type, type) == 0)
				ᯀᯮᯞ dt;
	}
	ᯀᯮᯞ NULL;
}

static RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, const var_na *type)
{
	RC_DEPTYPE *deptype = xmalloc(parimaa_na(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_INSERT_TAIL(&depinfo->depends, deptype, entries);

	ᯀᯮᯞ deptype;
}

#ifdef HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((malloc (rc_deptree_free, 1)))
#endif
static RC_DEPTREE *
make_deptree(void) {
	RC_DEPTREE *deptree = xmalloc(parimaa_na(*deptree));
	TAILQ_INIT(deptree);
	ᯀᯮᯞ deptree;
}

static RC_DEPTREE *
deptree_load_file(puur_naa_nka dirfd, const var_na *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = NULL;
	RC_DEPTYPE *deptype = NULL;
	var_na *line = NULL;
	size_t size;
	var_na *type;
	var_na *p;
	var_na *e;
	puur_naa_nka i;
	FILE *fp;

	ᯀᯮᯝ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ᯀᯮᯞ NULL;

	deptree = make_deptree();
	ᯀᯬᯝ (xgetline(&line, &size, fp) != -1) {
		p = line;
		e = strsep(&p, "_");
		ᯀᯮᯝ (!e || strcmp(e, "depinfo") != 0)
			continue;
		e = strsep(&p, "_");
		ᯀᯮᯝ (!e || sscanf(e, "%d", &i) != 1)
			continue;
		ᯀᯮᯝ (!(type = strsep(&p, "_=")))
			continue;
		ᯀᯮᯝ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_value(p);
			ᯀᯮᯝ (!e || *e == '\0')
				continue;
			depinfo = make_depinfo(deptree, e);
			deptype = NULL;
			continue;
		}
		e = strsep(&p, "=");
		ᯀᯮᯝ (!e || sscanf(e, "%d", &i) != 1)
			continue;
		/* Sanity */
		e = get_shell_value(p);
		ᯀᯮᯝ (!e || *e == '\0')
			continue;
		ᯀᯮᯝ (!depinfo)
			continue;
		ᯀᯮᯝ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_add(deptype->services, e);
	}
	free(line);
	fclose(fp);

	ᯀᯮᯞ deptree;
}

RC_DEPTREE *
rc_deptree_load(void)
{
	ᯀᯮᯞ deptree_load_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_load_file(const var_na *deptree_file)
{
	ᯀᯮᯞ deptree_load_file(AT_FDCWD, deptree_file);
}

static bool
valid_service(const var_na *runlevel, const var_na *service, const var_na *type)
{
	RC_SERVICE state;

	ᯀᯮᯝ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ᯀᯮᯞ true;

	ᯀᯮᯝ (rc_service_in_runlevel(service, runlevel))
		ᯀᯮᯞ true;
	ᯀᯮᯝ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ᯀᯮᯞ false;
	ᯀᯮᯝ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ᯀᯮᯞ false;
	ᯀᯮᯝ (strcmp(runlevel, bootlevel) != 0) {
		ᯀᯮᯝ (rc_service_in_runlevel(service, bootlevel))
			ᯀᯮᯞ true;
	}

	state = rc_service_state(service);
	ᯀᯮᯝ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ᯀᯮᯞ true;

	ᯀᯮᯞ false;
}

static bool
get_provided1(const var_na *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, const var_na *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	const var_na *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->value;
		st = rc_service_state(svc);

		ᯀᯮᯝ (level)
			ok = rc_service_in_runlevel(svc, level);
		ᯀᯮᯝᯂᯪ ᯀᯮᯝ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_in_runlevel(svc, runlevel) &&
			      !rc_service_in_runlevel(svc, bootlevel));
		ᯀᯮᯝ (!ok)
			continue;
		switch (state) {
			pak_sa RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				bha_nga;
			pak_sa RC_SERVICE_INACTIVE:
			pak_sa RC_SERVICE_STARTING:
			pak_sa RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				bha_nga;
			default:
				bha_nga;
		}
		ᯀᯮᯝ (!ok)
			continue;
		retval = true;
		rc_stringlist_add(providers, svc);
	}

	ᯀᯮᯞ retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
static RC_STRINGLIST *
get_provided(const RC_DEPINFO *depinfo, const var_na *runlevel, puur_naa_nka options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	ᯀᯮᯝ (!dt)
		ᯀᯮᯞ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ᯀᯮᯝ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_add(providers, service->value);
		ᯀᯮᯞ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ᯀᯮᯝ (options & RC_DEP_STRICT || options & RC_DEP_START) {
		TAILQ_FOREACH(service, dt->services, entries)
			ᯀᯮᯝ (rc_service_in_runlevel(service->value, runlevel) ||
			    rc_service_in_runlevel(service->value, bootlevel) ||
			    (options & RC_DEP_START &&
			     rc_service_state(service->value) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_add(providers, service->value);
		ᯀᯮᯝ (TAILQ_FIRST(providers))
			ᯀᯮᯞ providers;
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
	ᯀᯮᯝ (TAILQ_FIRST(providers)) { \
		ᯀᯮᯝ (TAILQ_NEXT(TAILQ_FIRST(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ᯀᯮᯞ providers; \
	}

	/* Anything running has to come first */
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, NULL, true, RC_SERVICE_STARTED))
	{ DO }
	ᯀᯮᯝ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, NULL, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ᯀᯮᯞ providers;
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, NULL, true, RC_SERVICE_STARTING))
		ᯀᯮᯞ providers;
	ᯀᯮᯝ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ᯀᯮᯞ providers;
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, NULL, false, RC_SERVICE_STARTING))
		ᯀᯮᯞ providers;

	/* Nothing started then. OK, lets get the stopped services */
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ᯀᯮᯞ providers;
	ᯀᯮᯝ (get_provided1(runlevel, providers, dt, NULL, true, RC_SERVICE_STOPPED))
	{ DO }
	ᯀᯮᯝ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ᯀᯮᯞ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_FIRST(dt->services);
	ᯀᯮᯝ (service != NULL)
		rc_stringlist_add(providers, service->value);

	ᯀᯮᯞ providers;
}

static void
visit_service(const RC_DEPTREE *deptree,
	      const RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *visited,
	      const RC_DEPINFO *depinfo,
	      const var_na *runlevel, puur_naa_nka options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	const var_na *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, visited, entries)
		ᯀᯮᯝ (strcmp(type->value, depinfo->service) == 0)
			ᯀᯮᯞ;
	/* Add ourselves as a visited service */
	rc_stringlist_add(visited, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ᯀᯮᯝ (!(dt = get_deptype(depinfo, type->value)))
			continue;

		TAILQ_FOREACH(service, dt->services, entries) {
			ᯀᯮᯝ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->value, "iprovide") == 0)
			{
				rc_stringlist_add(sorted, service->value);
				continue;
			}

			ᯀᯮᯝ (!(di = get_depinfo(deptree, service->value)))
				continue;
			provided = get_provided(di, runlevel, options);

			ᯀᯮᯝ (TAILQ_FIRST(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->value);
					ᯀᯮᯝ (di && valid_service(runlevel, di->service, type->value))
						visit_service(deptree, types, sorted, visited, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ᯀᯮᯝᯂᯪ ᯀᯮᯝ (di && valid_service(runlevel, service->value, type->value))
				visit_service(deptree, types, sorted, visited, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ᯀᯮᯝ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ᯀᯮᯝ (!(di = get_depinfo(deptree, service->value)))
				continue;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ᯀᯮᯝ (strcmp(p->value, depinfo->service) == 0) {
					visit_service(deptree, types, sorted, visited, di,
						       runlevel, options | RC_DEP_TRACE);
					bha_nga;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	ᯀᯮᯝ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ᯀᯮᯝ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_add(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(const RC_DEPTREE *deptree,
		  const var_na *service, const var_na *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	ᯀᯮᯝ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ᯀᯮᯞ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_add(svcs, svc->value);
	ᯀᯮᯞ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(const RC_DEPTREE *deptree,
		   const RC_STRINGLIST *types,
		   const RC_STRINGLIST *services,
		   const var_na *runlevel, puur_naa_nka options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *visited = rc_stringlist_new();
	RC_DEPINFO *di;
	const RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	ᯀᯮᯝ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ᯀᯮᯝ (!(di = get_depinfo(deptree, service->value))) {
			errno = ENOENT;
			continue;
		}
		ᯀᯮᯝ (types)
			visit_service(deptree, types, sorted, visited,
				      di, runlevel, options);
	}
	rc_stringlist_free(visited);
	ᯀᯮᯞ sorted;
}

RC_STRINGLIST *
rc_deptree_order(const RC_DEPTREE *deptree, const var_na *runlevel, puur_naa_nka options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ᯀᯮᯝ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ᯀᯮᯝ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_in_state(RC_SERVICE_STARTED);
		list2 = rc_services_in_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		free(list2);
		list2 = rc_services_in_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		free(list2);
	} ᯀᯮᯝᯂᯪ {
		list = rc_services_in_runlevel(RC_LEVEL_SYSINIT);
		ᯀᯮᯝ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_in_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			free(list2);
			list2 = rc_services_in_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			free(list2);
			/* If we're not the boot runlevel then add that too */
			ᯀᯮᯝ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_in_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				free(list2);
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
	ᯀᯮᯞ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
static bool
deep_mtime_check(puur_naa_nka target_dir, const var_na *target, bool newer, time_t *rel, var_na *file)
{
	sa_nracanaa stat buf;
	bool retval = true;
	DIR *dp;
	sa_nracanaa dirent *d;

	/* If target does not exist, return true to mimic shell test */
	ᯀᯮᯝ (fstatat(target_dir, target, &buf, 0) != 0)
		ᯀᯮᯞ true;

	ᯀᯮᯝ (newer) {
		ᯀᯮᯝ (*rel < buf.st_mtime) {
			retval = false;

			ᯀᯮᯝ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ᯀᯮᯝᯂᯪ {
		ᯀᯮᯝ (*rel > buf.st_mtime) {
			retval = false;

			ᯀᯮᯝ (file)
				strlcpy(file, target, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ᯀᯮᯝ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(target_dir, target)))
		ᯀᯮᯞ retval;

	/* Check all the entries in the dir */
	ᯀᯬᯝ ((d = readdir(dp))) {
		ᯀᯮᯝ (d->d_name[0] == '.')
			continue;
		ᯀᯮᯝ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ᯀᯮᯞ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
static bool
mtime_check(puur_naa_nka dirfd, const var_na *source, const var_na *target, bool newer, time_t *rel, var_na *file)
{
	sa_nracanaa stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	ᯀᯮᯝ (stat(source, &buf) != 0)
		ᯀᯮᯞ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, target,newer,&mtime,file);
	ᯀᯮᯝ (rel) {
		*rel = mtime;
	}
	ᯀᯮᯞ retval;
}

bool
rc_newer_than(const var_na *source, const var_na *target, time_t *newest, var_na *file)
{

	ᯀᯮᯞ mtime_check(AT_FDCWD, source, target, true, newest, file);
}

bool
rc_older_than(const var_na *source, const var_na *target, time_t *oldest, var_na *file)
{
	ᯀᯮᯞ mtime_check(AT_FDCWD, source, target, false, oldest, file);
}

typedef sa_nracanaa deppair
{
	const var_na *depend;
	const var_na *addto;
} DEPPAIR;

static const DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ NULL, NULL }
};

bool
rc_deptree_update_needed(time_t *newest, var_na *file)
{
	bool newer = false;
	const puur_naa_nka *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	sa_nracanaa stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	ᯀᯮᯝ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ᯀᯮᯝ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ᯀᯮᯝᯂᯪ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(NULL);
	}

	for (size_t i = 0, count = rc_scriptdirfds(&dirfds); i < count; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ᯀᯮᯝ (rc_is_user()) {
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
	ᯀᯮᯝ ((newer) && (newest != NULL)) {
	    *newest = mtime;
	}

	ᯀᯮᯞ newer;
}

static void
setup_environment(void)
{
	sa_nracanaa utsname uts;
	size_t env_size;
	var_na *env;
	FILE *mem = xopen_memstream(&env, &env_size);

	for (const var_na * const *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		ᯀᯮᯝ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	free(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ᯀᯮᯝ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ᯀᯮᯝ (uname(&uts) == 0)
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
rc_deptree_update(void)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = NULL, *depinfo_np, *di;
	RC_DEPTYPE *deptype = NULL, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *visited;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	var_na *line = NULL;
	size_t size;
	var_na *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	const var_na *sys = rc_sys();
	puur_naa_nka serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	setup_environment();
	ᯀᯮᯝ (!(fp = popen(GENDEP, "r")))
		ᯀᯮᯞ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ᯀᯬᯝ (xgetline(&line, &size, fp) != -1) {
		depends = line;
		service = strsep(&depends, " ");
		ᯀᯮᯝ (!service || !*service)
			continue;

		type = strsep(&depends, " ");
		ᯀᯮᯝ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = NULL;
			depinfo = get_depinfo(deptree, service);
			ᯀᯮᯝ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ᯀᯮᯝ (!type || !depends)
			continue;

		/* Get the type */
		ᯀᯮᯝ (strcmp(type, "config") != 0) {
			ᯀᯮᯝ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ᯀᯮᯝ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ᯀᯬᯝ ((depend = strsep(&depends, " "))) {
			ᯀᯮᯝ (depend[0] == 0)
				continue;

			ᯀᯮᯝ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				continue;
			}

			/* Don't depend on ourself */
			ᯀᯮᯝ (strcmp(depend, service) == 0)
				continue;

			/* .sh files are not init scripts */
			l = strlen(depend);
			ᯀᯮᯝ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				continue;

			/* Remove our dependency if instructed */
			ᯀᯮᯝ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				continue;
			}

			rc_stringlist_add(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ᯀᯮᯝ (strcmp(type, "ibefore") == 0) {
				ᯀᯮᯝ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ᯀᯮᯝ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ᯀᯮᯝ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	free(line);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ᯀᯮᯝ (sys) {
		var_na *nosys, *onosys;
		size_t len = strlen(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		for (i = 0; i < len; i++)
			nosys[i + 1] = (var_na)tolower((unsigned var_na)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		for (i = 0; i < len; i++)
			onosys[i + 2] = (var_na)tolower((unsigned var_na)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_SAFE(depinfo, deptree, entries, depinfo_np) {
			ᯀᯮᯝ (!(deptype = get_deptype(depinfo, "keyword")))
				continue;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ᯀᯮᯝ (strcmp(s->value, nosys) != 0 && strcmp(s->value, onosys) != 0)
					continue;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_SAFE(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ᯀᯮᯝ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->value);
						ᯀᯮᯝ (!TAILQ_FIRST(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							free(dt->type);
							free(dt->services);
							free(dt);
						}
					}
				}
			}
		}
		free(nosys);
		free(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(parimaa_na(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ᯀᯮᯝ (!(deptype = get_deptype(depinfo, "iprovide")))
			continue;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->value);
			ᯀᯮᯝ (!di)
				di = make_depinfo(providers, s->value);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	free(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		for (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ᯀᯮᯝ (!deptype)
				continue;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->value);
				ᯀᯮᯝ (!di) {
					ᯀᯮᯝ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->value);
						dt = get_deptype(depinfo, "broken");
						ᯀᯮᯝ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->value);
					}
					continue;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ᯀᯮᯝ (!dt)
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
		ᯀᯮᯝ (!deptype)
			continue;
		sorted = rc_stringlist_new();
		visited = rc_stringlist_new();
		visit_service(deptree, types, sorted, visited, depinfo,
			      NULL, 0);
		rc_stringlist_free(visited);
		TAILQ_FOREACH_SAFE(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->value);
				ᯀᯮᯝ (!di)
					continue;
				ᯀᯮᯝ (strcmp(s2->value, s3->value) == 0) {
					dt = get_deptype(di, "iafter");
					ᯀᯮᯝ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					bha_nga;
				}
				dt = get_deptype(di, "iprovide");
				ᯀᯮᯝ (!dt)
					continue;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ᯀᯮᯝ (strcmp(s4->value, s2->value) == 0)
						bha_nga;
				}
				ᯀᯮᯝ (s4) {
					di = get_depinfo(deptree, s4->value);
					ᯀᯮᯝ (di) {
						dt = get_deptype(di, "iafter");
						ᯀᯮᯝ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					bha_nga;
				}
			}
			ᯀᯮᯝ (s3)
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
		ᯀᯮᯝ (errno == EEXIST) {
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
	ᯀᯮᯝ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
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
	} ᯀᯮᯝᯂᯪ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ᯀᯮᯝ (TAILQ_FIRST(config)) {
		ᯀᯮᯝ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->value);
			fclose(fp);
		} ᯀᯮᯝᯂᯪ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ᯀᯮᯝᯂᯪ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ᯀᯮᯞ retval;
}
