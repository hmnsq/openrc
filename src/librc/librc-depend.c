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

#شامل <sys/utsname.h>
#شامل <ctype.h>
#شامل <dirent.h>
#شامل <errno.h>
#شامل <limits.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <time.h>
#شامل <unistd.h>

#شامل "queue.h"
#شامل "librc.h"
#شامل "helpers.h"
#شامل "misc.h"

#تعریف_کرو GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ساکن ثابت حرف *bootlevel = تہی;

ساکن حرف *
get_shell_قدر(حرف *string)
{
	حرف *p = string;
	حرف *e;

	اگر (!string)
		واپس تہی;

	اگر (*p == '\'')
		p++;

	e = p + لڑی_لمبائی(p) - 1;
	اگر (*e == '\n')
		*e-- = 0;
	اگر (*e == '\'')
		*e-- = 0;

	اگر (*p != 0)
		واپس p;

	واپس تہی;
}

خالی
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	اگر (!deptree)
		واپس;

	TAILQ_FOREACH_محفوظ(di, deptree, entries, di_save) {
		TAILQ_FOREACH_محفوظ(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			آزاد(dt->type);
			آزاد(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		آزاد(di->service);
		آزاد(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	آزاد(deptree);
}

ساکن RC_DEPINFO *
get_depinfo(ثابت RC_DEPTREE *deptree, ثابت حرف *service)
{
	RC_DEPINFO *di;
	اگر (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			اگر (strcmp(di->service, service) == 0)
				واپس di;
	}
	واپس تہی;
}

ساکن RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ثابت حرف *service)
{
	RC_DEPINFO *depinfo = xmalloc(حجم(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_داخل_کرو_دم(deptree, depinfo, entries);

	واپس depinfo;
}

ساکن RC_DEPTYPE *
get_deptype(ثابت RC_DEPINFO *depinfo, ثابت حرف *type)
{
	RC_DEPTYPE *dt;

	اگر (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			اگر (strcmp(dt->type, type) == 0)
				واپس dt;
	}
	واپس تہی;
}

ساکن RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ثابت حرف *type)
{
	RC_DEPTYPE *deptype = xmalloc(حجم(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_داخل_کرو_دم(&depinfo->depends, deptype, entries);

	واپس deptype;
}

#اگر_تعریف_شدہ HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((حافظہ_دو (rc_deptree_free, 1)))
#اگر_ختم
ساکن RC_DEPTREE *
make_deptree(خالی) {
	RC_DEPTREE *deptree = xmalloc(حجم(*deptree));
	TAILQ_INIT(deptree);
	واپس deptree;
}

ساکن RC_DEPTREE *
deptree_بھرو_file(صحیح dirfd, ثابت حرف *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = تہی;
	RC_DEPTYPE *deptype = تہی;
	حرف *سطر = تہی;
	size_t سائز;
	حرف *type;
	حرف *p;
	حرف *e;
	صحیح i;
	FILE *fp;

	اگر (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		واپس تہی;

	deptree = make_deptree();
	جب (xgetline(&سطر, &سائز, fp) != -1) {
		p = سطر;
		e = strsep(&p, "_");
		اگر (!e || strcmp(e, "depinfo") != 0)
			جاری;
		e = strsep(&p, "_");
		اگر (!e || لڑی_سے_پڑھو(e, "%d", &i) != 1)
			جاری;
		اگر (!(type = strsep(&p, "_=")))
			جاری;
		اگر (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_قدر(p);
			اگر (!e || *e == '\0')
				جاری;
			depinfo = make_depinfo(deptree, e);
			deptype = تہی;
			جاری;
		}
		e = strsep(&p, "=");
		اگر (!e || لڑی_سے_پڑھو(e, "%d", &i) != 1)
			جاری;
		/* Sanity */
		e = get_shell_قدر(p);
		اگر (!e || *e == '\0')
			جاری;
		اگر (!depinfo)
			جاری;
		اگر (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_جوڑو(deptype->services, e);
	}
	آزاد(سطر);
	fclose(fp);

	واپس deptree;
}

RC_DEPTREE *
rc_deptree_بھرو(خالی)
{
	واپس deptree_بھرو_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_بھرو_file(ثابت حرف *deptree_file)
{
	واپس deptree_بھرو_file(AT_FDCWD, deptree_file);
}

ساکن bool
valid_service(ثابت حرف *runlevel, ثابت حرف *service, ثابت حرف *type)
{
	RC_SERVICE state;

	اگر (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		واپس true;

	اگر (rc_service_اندر_runlevel(service, runlevel))
		واپس true;
	اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    واپس false;
	اگر (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    واپس false;
	اگر (strcmp(runlevel, bootlevel) != 0) {
		اگر (rc_service_اندر_runlevel(service, bootlevel))
			واپس true;
	}

	state = rc_service_state(service);
	اگر (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		واپس true;

	واپس false;
}

ساکن bool
get_provided1(ثابت حرف *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ثابت حرف *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ثابت حرف *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->قدر;
		st = rc_service_state(svc);

		اگر (level)
			ok = rc_service_اندر_runlevel(svc, level);
		ورنہ اگر (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_اندر_runlevel(svc, runlevel) &&
			      !rc_service_اندر_runlevel(svc, bootlevel));
		اگر (!ok)
			جاری;
		انتخاب (state) {
			حالت RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				توڑو;
			حالت RC_SERVICE_INACTIVE:
			حالت RC_SERVICE_STARTING:
			حالت RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				توڑو;
			طےشدہ:
				توڑو;
		}
		اگر (!ok)
			جاری;
		retval = true;
		rc_stringlist_جوڑو(providers, svc);
	}

	واپس retval;
}

/* Work out if a service is provided by another service.
   For example metalog provides logger.
   We need to be able to handle syslogd providing logger too.
   We do this by checking what's running, then what's starting/stopping,
   then what's run in the runlevels and finally alphabetical order.

   If there are any bugs in rc-depend, they will probably be here as
   provided dependency can change depending on runlevel state.
   */
ساکن RC_STRINGLIST *
get_provided(ثابت RC_DEPINFO *depinfo, ثابت حرف *runlevel, صحیح options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	اگر (!dt)
		واپس providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	اگر (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_جوڑو(providers, service->قدر);
		واپس providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	اگر (options & RC_DEP_STRICT || options & RC_DEP_آغاز) {
		TAILQ_FOREACH(service, dt->services, entries)
			اگر (rc_service_اندر_runlevel(service->قدر, runlevel) ||
			    rc_service_اندر_runlevel(service->قدر, bootlevel) ||
			    (options & RC_DEP_آغاز &&
			     rc_service_state(service->قدر) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_جوڑو(providers, service->قدر);
		اگر (TAILQ_پہلا(providers))
			واپس providers;
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
#تعریف_کرو DO \
	اگر (TAILQ_پہلا(providers)) { \
		اگر (TAILQ_اگلا(TAILQ_پہلا(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		واپس providers; \
	}

	/* Anything running has to come first */
	اگر (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	اگر (get_provided1(runlevel, providers, dt, تہی, true, RC_SERVICE_STARTED))
	{ DO }
	اگر (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	اگر (get_provided1(runlevel, providers, dt, تہی, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	اگر (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		واپس providers;
	اگر (get_provided1(runlevel, providers, dt, تہی, true, RC_SERVICE_STARTING))
		واپس providers;
	اگر (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    واپس providers;
	اگر (get_provided1(runlevel, providers, dt, تہی, false, RC_SERVICE_STARTING))
		واپس providers;

	/* Nothing started then. OK, lets get the stopped services */
	اگر (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		واپس providers;
	اگر (get_provided1(runlevel, providers, dt, تہی, true, RC_SERVICE_STOPPED))
	{ DO }
	اگر (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		واپس providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_پہلا(dt->services);
	اگر (service != تہی)
		rc_stringlist_جوڑو(providers, service->قدر);

	واپس providers;
}

ساکن خالی
دورہ_service(ثابت RC_DEPTREE *deptree,
	      ثابت RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *دیکھا,
	      ثابت RC_DEPINFO *depinfo,
	      ثابت حرف *runlevel, صحیح options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ثابت حرف *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, دیکھا, entries)
		اگر (strcmp(type->قدر, depinfo->service) == 0)
			واپس;
	/* Add ourselves as a visited service */
	rc_stringlist_جوڑو(دیکھا, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		اگر (!(dt = get_deptype(depinfo, type->قدر)))
			جاری;

		TAILQ_FOREACH(service, dt->services, entries) {
			اگر (!(options & RC_DEP_TRACE) ||
			    strcmp(type->قدر, "iprovide") == 0)
			{
				rc_stringlist_جوڑو(sorted, service->قدر);
				جاری;
			}

			اگر (!(di = get_depinfo(deptree, service->قدر)))
				جاری;
			provided = get_provided(di, runlevel, options);

			اگر (TAILQ_پہلا(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->قدر);
					اگر (di && valid_service(runlevel, di->service, type->قدر))
						دورہ_service(deptree, types, sorted, دیکھا, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ورنہ اگر (di && valid_service(runlevel, service->قدر, type->قدر))
				دورہ_service(deptree, types, sorted, دیکھا, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	اگر (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			اگر (!(di = get_depinfo(deptree, service->قدر)))
				جاری;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				اگر (strcmp(p->قدر, depinfo->service) == 0) {
					دورہ_service(deptree, types, sorted, دیکھا, di,
						       runlevel, options | RC_DEP_TRACE);
					توڑو;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	اگر (!svcname || strcmp(svcname, depinfo->service) != 0) {
		اگر (!get_deptype(depinfo, "providedby"))
			rc_stringlist_جوڑو(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ثابت RC_DEPTREE *deptree,
		  ثابت حرف *service, ثابت حرف *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	اگر (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		واپس svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_جوڑو(svcs, svc->قدر);
	واپس svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ثابت RC_DEPTREE *deptree,
		   ثابت RC_STRINGLIST *types,
		   ثابت RC_STRINGLIST *services,
		   ثابت حرف *runlevel, صحیح options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *دیکھا = rc_stringlist_new();
	RC_DEPINFO *di;
	ثابت RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	اگر (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		اگر (!(di = get_depinfo(deptree, service->قدر))) {
			errno = ENOENT;
			جاری;
		}
		اگر (types)
			دورہ_service(deptree, types, sorted, دیکھا,
				      di, runlevel, options);
	}
	rc_stringlist_free(دیکھا);
	واپس sorted;
}

RC_STRINGLIST *
rc_deptree_ترتیب(ثابت RC_DEPTREE *deptree, ثابت حرف *runlevel, صحیح options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	اگر (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	اگر (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_اندر_state(RC_SERVICE_STARTED);
		list2 = rc_services_اندر_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		آزاد(list2);
		list2 = rc_services_اندر_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		آزاد(list2);
	} ورنہ {
		list = rc_services_اندر_runlevel(RC_LEVEL_SYSINIT);
		اگر (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_اندر_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			آزاد(list2);
			list2 = rc_services_اندر_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			آزاد(list2);
			/* If we're not the boot runlevel then add that too */
			اگر (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_اندر_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				آزاد(list2);
			}
		}
	}

	/* Now we have our lists, we need to pull in any dependencies
	   and order them */
	types = rc_stringlist_new();
	rc_stringlist_جوڑو(types, "ineed");
	rc_stringlist_جوڑو(types, "iuse");
	rc_stringlist_جوڑو(types, "iwant");
	rc_stringlist_جوڑو(types, "iafter");
	services = rc_deptree_depends(deptree, types, list, runlevel,
				      RC_DEP_STRICT | RC_DEP_TRACE | options);
	rc_stringlist_free(list);
	rc_stringlist_free(types);
	واپس services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ساکن bool
deep_mtime_check(صحیح ہدف_dir, ثابت حرف *ہدف, bool newer, time_t *rel, حرف *file)
{
	ساخت stat buf;
	bool retval = true;
	DIR *dp;
	ساخت dirent *d;

	/* If target does not exist, return true to mimic shell test */
	اگر (fstatat(ہدف_dir, ہدف, &buf, 0) != 0)
		واپس true;

	اگر (newer) {
		اگر (*rel < buf.st_mtime) {
			retval = false;

			اگر (file)
				strlcpy(file, ہدف, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ورنہ {
		اگر (*rel > buf.st_mtime) {
			retval = false;

			اگر (file)
				strlcpy(file, ہدف, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	اگر (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(ہدف_dir, ہدف)))
		واپس retval;

	/* Check all the entries in the dir */
	جب ((d = readdir(dp))) {
		اگر (d->d_name[0] == '.')
			جاری;
		اگر (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	واپس retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ساکن bool
mtime_check(صحیح dirfd, ثابت حرف *ماخذ, ثابت حرف *ہدف, bool newer, time_t *rel, حرف *file)
{
	ساخت stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	اگر (stat(ماخذ, &buf) != 0)
		واپس false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, ہدف,newer,&mtime,file);
	اگر (rel) {
		*rel = mtime;
	}
	واپس retval;
}

bool
rc_newer_than(ثابت حرف *ماخذ, ثابت حرف *ہدف, time_t *newest, حرف *file)
{

	واپس mtime_check(AT_FDCWD, ماخذ, ہدف, true, newest, file);
}

bool
rc_older_than(ثابت حرف *ماخذ, ثابت حرف *ہدف, time_t *oldest, حرف *file)
{
	واپس mtime_check(AT_FDCWD, ماخذ, ہدف, false, oldest, file);
}

قسمتعریف ساخت deppair
{
	ثابت حرف *depend;
	ثابت حرف *addto;
} DEPPAIR;

ساکن ثابت DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ تہی, تہی }
};

bool
rc_deptree_update_needed(time_t *newest, حرف *file)
{
	bool newer = false;
	ثابت صحیح *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ساخت stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	اگر (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	اگر (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ورنہ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(تہی);
	}

	برائے (size_t i = 0, گنتی = rc_scriptdirfds(&dirfds); i < گنتی; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	اگر (rc_is_user()) {
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf", true, &mtime, file);
		newer |= !deep_mtime_check(rc_dirfd(RC_DIR_USRCONF), "rc.conf.d", true, &mtime, file);
	}

	/* Some init scripts dependencies change depending on config files
	 * outside of baselayout, like syslog-ng, so we check those too. */
	config = config_list(rc_dirfd(RC_DIR_SVCDIR), "depconfig");
	TAILQ_FOREACH(s, config, entries)
		newer |= !deep_mtime_check(AT_FDCWD, s->قدر, true, &mtime, file);
	rc_stringlist_free(config);

	/* Return newest file time, if requested */
	اگر ((newer) && (newest != تہی)) {
	    *newest = mtime;
	}

	واپس newer;
}

ساکن خالی
تیاری_environment(خالی)
{
	ساخت utsname uts;
	size_t env_سائز;
	حرف *env;
	FILE *mem = xopen_memstream(&env, &env_سائز);

	برائے (ثابت حرف * ثابت *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		اگر (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	آزاد(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	اگر (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	اگر (uname(&uts) == 0)
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
rc_deptree_update(خالی)
{

	FILE *fp;
	RC_DEPTREE *deptree, *providers;
	RC_DEPINFO *depinfo = تہی, *depinfo_np, *di;
	RC_DEPTYPE *deptype = تہی, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *دیکھا;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	حرف *سطر = تہی;
	size_t سائز;
	حرف *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ثابت حرف *sys = rc_sys();
	صحیح serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	تیاری_environment();
	اگر (!(fp = popen(GENDEP, "r")))
		واپس false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	جب (xgetline(&سطر, &سائز, fp) != -1) {
		depends = سطر;
		service = strsep(&depends, " ");
		اگر (!service || !*service)
			جاری;

		type = strsep(&depends, " ");
		اگر (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = تہی;
			depinfo = get_depinfo(deptree, service);
			اگر (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		اگر (!type || !depends)
			جاری;

		/* Get the type */
		اگر (strcmp(type, "config") != 0) {
			اگر (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				اگر (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		جب ((depend = strsep(&depends, " "))) {
			اگر (depend[0] == 0)
				جاری;

			اگر (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				جاری;
			}

			/* Don't depend on ourself */
			اگر (strcmp(depend, service) == 0)
				جاری;

			/* .sh files are not init scripts */
			l = لڑی_لمبائی(depend);
			اگر (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				جاری;

			/* Remove our dependency if instructed */
			اگر (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				جاری;
			}

			rc_stringlist_جوڑو(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			اگر (strcmp(type, "ibefore") == 0) {
				اگر ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			اگر (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				اگر ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	آزاد(سطر);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	اگر (sys) {
		حرف *nosys, *onosys;
		size_t len = لڑی_لمبائی(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		برائے (i = 0; i < len; i++)
			nosys[i + 1] = (حرف)tolower((غیرعلامتی حرف)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		برائے (i = 0; i < len; i++)
			onosys[i + 2] = (حرف)tolower((غیرعلامتی حرف)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_محفوظ(depinfo, deptree, entries, depinfo_np) {
			اگر (!(deptype = get_deptype(depinfo, "keyword")))
				جاری;
			TAILQ_FOREACH(s, deptype->services, entries) {
				اگر (strcmp(s->قدر, nosys) != 0 && strcmp(s->قدر, onosys) != 0)
					جاری;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_محفوظ(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						اگر (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->قدر);
						اگر (!TAILQ_پہلا(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							آزاد(dt->type);
							آزاد(dt->services);
							آزاد(dt);
						}
					}
				}
			}
		}
		آزاد(nosys);
		آزاد(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(حجم(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		اگر (!(deptype = get_deptype(depinfo, "iprovide")))
			جاری;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->قدر);
			اگر (!di)
				di = make_depinfo(providers, s->قدر);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	آزاد(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		برائے (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			اگر (!deptype)
				جاری;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->قدر);
				اگر (!di) {
					اگر (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->قدر);
						dt = get_deptype(depinfo, "broken");
						اگر (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->قدر);
					}
					جاری;
				}

				dt = get_deptype(di, deppairs[i].addto);
				اگر (!dt)
					dt = make_deptype(di, deppairs[i].addto);
				rc_stringlist_addu(dt->services, depinfo->service);
			}
		}
	}

	/* Phase 5 - Remove broken before directives */
	types = rc_stringlist_new();
	rc_stringlist_جوڑو(types, "ineed");
	rc_stringlist_جوڑو(types, "iwant");
	rc_stringlist_جوڑو(types, "iuse");
	rc_stringlist_جوڑو(types, "iafter");
	TAILQ_FOREACH(depinfo, deptree, entries) {
		deptype = get_deptype(depinfo, "ibefore");
		اگر (!deptype)
			جاری;
		sorted = rc_stringlist_new();
		دیکھا = rc_stringlist_new();
		دورہ_service(deptree, types, sorted, دیکھا, depinfo,
			      تہی, 0);
		rc_stringlist_free(دیکھا);
		TAILQ_FOREACH_محفوظ(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->قدر);
				اگر (!di)
					جاری;
				اگر (strcmp(s2->قدر, s3->قدر) == 0) {
					dt = get_deptype(di, "iafter");
					اگر (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					توڑو;
				}
				dt = get_deptype(di, "iprovide");
				اگر (!dt)
					جاری;
				TAILQ_FOREACH(s4, dt->services, entries) {
					اگر (strcmp(s4->قدر, s2->قدر) == 0)
						توڑو;
				}
				اگر (s4) {
					di = get_depinfo(deptree, s4->قدر);
					اگر (di) {
						dt = get_deptype(di, "iafter");
						اگر (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					توڑو;
				}
			}
			اگر (s3)
				rc_stringlist_delete(deptype->services, s2->قدر);
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
		اگر (errno == EEXIST) {
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
	اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
		i = 0;
		TAILQ_FOREACH(depinfo, deptree, entries) {
			fprintf(fp, "depinfo_%zu_service='%s'\n", i, depinfo->service);
			TAILQ_FOREACH(deptype, &depinfo->depends, entries) {
				size_t k = 0;
				TAILQ_FOREACH(s, deptype->services, entries) {
					fprintf(fp, "depinfo_%zu_%s_%zu='%s'\n",
							i, deptype->type, k++, s->قدر);
				}
			}
			i++;
		}
		fclose(fp);
	} ورنہ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	اگر (TAILQ_پہلا(config)) {
		اگر ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->قدر);
			fclose(fp);
		} ورنہ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ورنہ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	واپس retval;
}
