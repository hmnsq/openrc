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

#ವ್ಯಾಖ್ಯಾನಿಸು GENDEP          RC_LIBEXECDIR "/sh/gendepends.sh"

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *bootlevel = ಖಾಲಿ;

ಸ್ಥಿರ ಅಕ್ಷರ *
get_shell_ಮೌಲ್ಯ(ಅಕ್ಷರ *string)
{
	ಅಕ್ಷರ *p = string;
	ಅಕ್ಷರ *e;

	ಆದರೆ (!string)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (*p == '\'')
		p++;

	e = p + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) - 1;
	ಆದರೆ (*e == '\n')
		*e-- = 0;
	ಆದರೆ (*e == '\'')
		*e-- = 0;

	ಆದರೆ (*p != 0)
		ಹಿಂತಿರುಗಿ p;

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಶೂನ್ಯ
rc_deptree_free(RC_DEPTREE *deptree)
{
	RC_DEPINFO *di;
	RC_DEPINFO *di_save;
	RC_DEPTYPE *dt;
	RC_DEPTYPE *dt_save;

	ಆದರೆ (!deptree)
		ಹಿಂತಿರುಗಿ;

	TAILQ_FOREACH_ಸುರಕ್ಷಿತ(di, deptree, entries, di_save) {
		TAILQ_FOREACH_ಸುರಕ್ಷಿತ(dt, &di->depends, entries, dt_save) {
			TAILQ_REMOVE(&di->depends, dt, entries);
			rc_stringlist_free(dt->services);
			ಬಿಡುಗಡೆ(dt->type);
			ಬಿಡುಗಡೆ(dt);
		}
		TAILQ_REMOVE(deptree, di, entries);
		ಬಿಡುಗಡೆ(di->service);
		ಬಿಡುಗಡೆ(di);
	}

	/* Use free() here since rc_deptree_free should not call itself */
	ಬಿಡುಗಡೆ(deptree);
}

ಸ್ಥಿರ RC_DEPINFO *
get_depinfo(ಸ್ಥಿರಾಂಕ RC_DEPTREE *deptree, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	RC_DEPINFO *di;
	ಆದರೆ (deptree) {
		TAILQ_FOREACH(di, deptree, entries)
			ಆದರೆ (strcmp(di->service, service) == 0)
				ಹಿಂತಿರುಗಿ di;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ RC_DEPINFO *
make_depinfo(RC_DEPTREE *deptree, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service)
{
	RC_DEPINFO *depinfo = xmalloc(ಗಾತ್ರ(*depinfo));
	TAILQ_INIT(&depinfo->depends);
	depinfo->service = xstrdup(service);
	TAILQ_ಸೇರಿಸು_ಬಾಲ(deptree, depinfo, entries);

	ಹಿಂತಿರುಗಿ depinfo;
}

ಸ್ಥಿರ RC_DEPTYPE *
get_deptype(ಸ್ಥಿರಾಂಕ RC_DEPINFO *depinfo, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type)
{
	RC_DEPTYPE *dt;

	ಆದರೆ (depinfo) {
		TAILQ_FOREACH(dt, &depinfo->depends, entries)
			ಆದರೆ (strcmp(dt->type, type) == 0)
				ಹಿಂತಿರುಗಿ dt;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ RC_DEPTYPE *
make_deptype(RC_DEPINFO *depinfo, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type)
{
	RC_DEPTYPE *deptype = xmalloc(ಗಾತ್ರ(*deptype));
	deptype->type = xstrdup(type);
	deptype->services = rc_stringlist_new();
	TAILQ_ಸೇರಿಸು_ಬಾಲ(&depinfo->depends, deptype, entries);

	ಹಿಂತಿರುಗಿ deptype;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MALLOC_EXTENDED_ATTRIBUTE
__attribute__ ((ಸ್ಮರಣೆ_ಕೊಡು (rc_deptree_free, 1)))
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ RC_DEPTREE *
make_deptree(ಶೂನ್ಯ) {
	RC_DEPTREE *deptree = xmalloc(ಗಾತ್ರ(*deptree));
	TAILQ_INIT(deptree);
	ಹಿಂತಿರುಗಿ deptree;
}

ಸ್ಥಿರ RC_DEPTREE *
deptree_ತುಂಬು_file(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pathname)
{
	RC_DEPTREE *deptree;
	RC_DEPINFO *depinfo = ಖಾಲಿ;
	RC_DEPTYPE *deptype = ಖಾಲಿ;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *type;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *e;
	ಪೂರ್ಣಾಂಕ i;
	FILE *fp;

	ಆದರೆ (!(fp = do_fopenat(dirfd, pathname, O_RDONLY)))
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	deptree = make_deptree();
	ಆಗ (xgetline(&ಸಾಲು, &ಗಾತ್ರ_2, fp) != -1) {
		p = ಸಾಲು;
		e = strsep(&p, "_");
		ಆದರೆ (!e || strcmp(e, "depinfo") != 0)
			ಮುಂದುವರಿ;
		e = strsep(&p, "_");
		ಆದರೆ (!e || ಅಕ್ಷರಸರಣಿ_ಓದು(e, "%d", &i) != 1)
			ಮುಂದುವರಿ;
		ಆದರೆ (!(type = strsep(&p, "_=")))
			ಮುಂದುವರಿ;
		ಆದರೆ (strcmp(type, "service") == 0) {
			/* Sanity */
			e = get_shell_ಮೌಲ್ಯ(p);
			ಆದರೆ (!e || *e == '\0')
				ಮುಂದುವರಿ;
			depinfo = make_depinfo(deptree, e);
			deptype = ಖಾಲಿ;
			ಮುಂದುವರಿ;
		}
		e = strsep(&p, "=");
		ಆದರೆ (!e || ಅಕ್ಷರಸರಣಿ_ಓದು(e, "%d", &i) != 1)
			ಮುಂದುವರಿ;
		/* Sanity */
		e = get_shell_ಮೌಲ್ಯ(p);
		ಆದರೆ (!e || *e == '\0')
			ಮುಂದುವರಿ;
		ಆದರೆ (!depinfo)
			ಮುಂದುವರಿ;
		ಆದರೆ (!deptype || strcmp(deptype->type, type) != 0)
			deptype = make_deptype(depinfo, type);
		rc_stringlist_ಕೂಡಿಸು(deptype->services, e);
	}
	ಬಿಡುಗಡೆ(ಸಾಲು);
	fclose(fp);

	ಹಿಂತಿರುಗಿ deptree;
}

RC_DEPTREE *
rc_deptree_ತುಂಬು(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ deptree_ತುಂಬು_file(rc_dirfd(RC_DIR_SVCDIR), "deptree");
}

RC_DEPTREE *
rc_deptree_ತುಂಬು_file(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *deptree_file)
{
	ಹಿಂತಿರುಗಿ deptree_ತುಂಬು_file(AT_FDCWD, deptree_file);
}

ಸ್ಥಿರ bool
valid_service(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type)
{
	RC_SERVICE state;

	ಆದರೆ (!runlevel ||
	    strcmp(type, "ineed") == 0 ||
	    strcmp(type, "needsme") == 0  ||
	    strcmp(type, "iwant") == 0 ||
	    strcmp(type, "wantsme") == 0)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (rc_service_ಒಳ_runlevel(service, runlevel))
		ಹಿಂತಿರುಗಿ true;
	ಆದರೆ (strcmp(runlevel, RC_LEVEL_SYSINIT) == 0)
		    ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0 &&
	    strcmp(type, "iafter") == 0)
		    ಹಿಂತಿರುಗಿ false;
	ಆದರೆ (strcmp(runlevel, bootlevel) != 0) {
		ಆದರೆ (rc_service_ಒಳ_runlevel(service, bootlevel))
			ಹಿಂತಿರುಗಿ true;
	}

	state = rc_service_state(service);
	ಆದರೆ (state & RC_SERVICE_HOTPLUGGED ||
	    state & RC_SERVICE_STARTED)
		ಹಿಂತಿರುಗಿ true;

	ಹಿಂತಿರುಗಿ false;
}

ಸ್ಥಿರ bool
get_provided1(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, RC_STRINGLIST *providers,
	      RC_DEPTYPE *deptype, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *level,
	      bool hotplugged, RC_SERVICE state)
{
	RC_STRING *service;
	RC_SERVICE st;
	bool retval = false;
	bool ok;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svc;

	TAILQ_FOREACH(service, deptype->services, entries) {
		ok = true;
		svc = service->ಮೌಲ್ಯ;
		st = rc_service_state(svc);

		ಆದರೆ (level)
			ok = rc_service_ಒಳ_runlevel(svc, level);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (hotplugged)
			ok = (st & RC_SERVICE_HOTPLUGGED &&
			      !rc_service_ಒಳ_runlevel(svc, runlevel) &&
			      !rc_service_ಒಳ_runlevel(svc, bootlevel));
		ಆದರೆ (!ok)
			ಮುಂದುವರಿ;
		ಆಯ್ಕೆ (state) {
			ಸ್ಥಿತಿ RC_SERVICE_STARTED:
				ok = (st & RC_SERVICE_STARTED);
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ RC_SERVICE_INACTIVE:
			ಸ್ಥಿತಿ RC_SERVICE_STARTING:
			ಸ್ಥಿತಿ RC_SERVICE_STOPPING:
				ok = (st & RC_SERVICE_STARTING ||
				      st & RC_SERVICE_STOPPING ||
				      st & RC_SERVICE_INACTIVE);
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
				ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (!ok)
			ಮುಂದುವರಿ;
		retval = true;
		rc_stringlist_ಕೂಡಿಸು(providers, svc);
	}

	ಹಿಂತಿರುಗಿ retval;
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
get_provided(ಸ್ಥಿರಾಂಕ RC_DEPINFO *depinfo, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಪೂರ್ಣಾಂಕ options)
{
	RC_DEPTYPE *dt;
	RC_STRINGLIST *providers = rc_stringlist_new();
	RC_STRING *service;

	dt = get_deptype(depinfo, "providedby");
	ಆದರೆ (!dt)
		ಹಿಂತಿರುಗಿ providers;

	/* If we are stopping then all depends are true, regardless of state.
	   This is especially true for net services as they could force a restart
	   of the local dns resolver which may depend on net. */
	ಆದರೆ (options & RC_DEP_STOP) {
		TAILQ_FOREACH(service, dt->services, entries)
			rc_stringlist_ಕೂಡಿಸು(providers, service->ಮೌಲ್ಯ);
		ಹಿಂತಿರುಗಿ providers;
	}

	/* If we're strict or starting, then only use what we have in our
	 * runlevel and bootlevel. If we starting then check hotplugged too. */
	ಆದರೆ (options & RC_DEP_STRICT || options & RC_DEP_ಆರಂಭ) {
		TAILQ_FOREACH(service, dt->services, entries)
			ಆದರೆ (rc_service_ಒಳ_runlevel(service->ಮೌಲ್ಯ, runlevel) ||
			    rc_service_ಒಳ_runlevel(service->ಮೌಲ್ಯ, bootlevel) ||
			    (options & RC_DEP_ಆರಂಭ &&
			     rc_service_state(service->ಮೌಲ್ಯ) & RC_SERVICE_HOTPLUGGED))
				rc_stringlist_ಕೂಡಿಸು(providers, service->ಮೌಲ್ಯ);
		ಆದರೆ (TAILQ_ಮೊದಲ(providers))
			ಹಿಂತಿರುಗಿ providers;
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
#ವ್ಯಾಖ್ಯಾನಿಸು DO \
	ಆದರೆ (TAILQ_ಮೊದಲ(providers)) { \
		ಆದರೆ (TAILQ_ಮುಂದಿನ(TAILQ_ಮೊದಲ(providers), entries)) { \
			rc_stringlist_free(providers); \
			providers = rc_stringlist_new(); \
		} \
		ಹಿಂತಿರುಗಿ providers; \
	}

	/* Anything running has to come first */
	ಆದರೆ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ಆದರೆ (get_provided1(runlevel, providers, dt, ಖಾಲಿ, true, RC_SERVICE_STARTED))
	{ DO }
	ಆದರೆ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTED))
	{ DO }
	ಆದರೆ (get_provided1(runlevel, providers, dt, ಖಾಲಿ, false, RC_SERVICE_STARTED))
	{ DO }

	/* Check starting services */
	ಆದರೆ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STARTING))
		ಹಿಂತಿರುಗಿ providers;
	ಆದರೆ (get_provided1(runlevel, providers, dt, ಖಾಲಿ, true, RC_SERVICE_STARTING))
		ಹಿಂತಿರುಗಿ providers;
	ಆದರೆ (bootlevel && strcmp(runlevel, bootlevel) != 0 &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STARTING))
	    ಹಿಂತಿರುಗಿ providers;
	ಆದರೆ (get_provided1(runlevel, providers, dt, ಖಾಲಿ, false, RC_SERVICE_STARTING))
		ಹಿಂತಿರುಗಿ providers;

	/* Nothing started then. OK, lets get the stopped services */
	ಆದರೆ (get_provided1(runlevel, providers, dt, runlevel, false, RC_SERVICE_STOPPED))
		ಹಿಂತಿರುಗಿ providers;
	ಆದರೆ (get_provided1(runlevel, providers, dt, ಖಾಲಿ, true, RC_SERVICE_STOPPED))
	{ DO }
	ಆದರೆ (bootlevel && (strcmp(runlevel, bootlevel) != 0) &&
	    get_provided1(runlevel, providers, dt, bootlevel, false, RC_SERVICE_STOPPED))
		ಹಿಂತಿರುಗಿ providers;

	/* Still nothing? OK, list our first provided service. */
	service = TAILQ_ಮೊದಲ(dt->services);
	ಆದರೆ (service != ಖಾಲಿ)
		rc_stringlist_ಕೂಡಿಸು(providers, service->ಮೌಲ್ಯ);

	ಹಿಂತಿರುಗಿ providers;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಭೇಟಿ_service(ಸ್ಥಿರಾಂಕ RC_DEPTREE *deptree,
	      ಸ್ಥಿರಾಂಕ RC_STRINGLIST *types,
	      RC_STRINGLIST *sorted,
	      RC_STRINGLIST *ಭೇಟಿಯಾದ,
	      ಸ್ಥಿರಾಂಕ RC_DEPINFO *depinfo,
	      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಪೂರ್ಣಾಂಕ options)
{
	RC_STRING *type;
	RC_STRING *service;
	RC_DEPTYPE *dt;
	RC_DEPINFO *di;
	RC_STRINGLIST *provided;
	RC_STRING *p;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *svcname;

	/* Check if we have already visited this service or not */
	TAILQ_FOREACH(type, ಭೇಟಿಯಾದ, entries)
		ಆದರೆ (strcmp(type->ಮೌಲ್ಯ, depinfo->service) == 0)
			ಹಿಂತಿರುಗಿ;
	/* Add ourselves as a visited service */
	rc_stringlist_ಕೂಡಿಸು(ಭೇಟಿಯಾದ, depinfo->service);

	TAILQ_FOREACH(type, types, entries)
	{
		ಆದರೆ (!(dt = get_deptype(depinfo, type->ಮೌಲ್ಯ)))
			ಮುಂದುವರಿ;

		TAILQ_FOREACH(service, dt->services, entries) {
			ಆದರೆ (!(options & RC_DEP_TRACE) ||
			    strcmp(type->ಮೌಲ್ಯ, "iprovide") == 0)
			{
				rc_stringlist_ಕೂಡಿಸು(sorted, service->ಮೌಲ್ಯ);
				ಮುಂದುವರಿ;
			}

			ಆದರೆ (!(di = get_depinfo(deptree, service->ಮೌಲ್ಯ)))
				ಮುಂದುವರಿ;
			provided = get_provided(di, runlevel, options);

			ಆದರೆ (TAILQ_ಮೊದಲ(provided)) {
				TAILQ_FOREACH(p, provided, entries) {
					di = get_depinfo(deptree, p->ಮೌಲ್ಯ);
					ಆದರೆ (di && valid_service(runlevel, di->service, type->ಮೌಲ್ಯ))
						ಭೇಟಿ_service(deptree, types, sorted, ಭೇಟಿಯಾದ, di,
							      runlevel, options | RC_DEP_TRACE);
				}
			}
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (di && valid_service(runlevel, service->ಮೌಲ್ಯ, type->ಮೌಲ್ಯ))
				ಭೇಟಿ_service(deptree, types, sorted, ಭೇಟಿಯಾದ, di,
					      runlevel, options | RC_DEP_TRACE);

			rc_stringlist_free(provided);
		}
	}

	/* Now visit the stuff we provide for */
	ಆದರೆ (options & RC_DEP_TRACE &&
	    (dt = get_deptype(depinfo, "iprovide")))
	{
		TAILQ_FOREACH(service, dt->services, entries) {
			ಆದರೆ (!(di = get_depinfo(deptree, service->ಮೌಲ್ಯ)))
				ಮುಂದುವರಿ;
			provided = get_provided(di, runlevel, options);
			TAILQ_FOREACH(p, provided, entries)
				ಆದರೆ (strcmp(p->ಮೌಲ್ಯ, depinfo->service) == 0) {
					ಭೇಟಿ_service(deptree, types, sorted, ಭೇಟಿಯಾದ, di,
						       runlevel, options | RC_DEP_TRACE);
					ನಿಲ್ಲಿಸು;
				}
			rc_stringlist_free(provided);
		}
	}

	/* We've visited everything we need, so add ourselves unless we
	   are also the service calling us or we are provided by something */
	svcname = getenv("RC_SVCNAME");
	ಆದರೆ (!svcname || strcmp(svcname, depinfo->service) != 0) {
		ಆದರೆ (!get_deptype(depinfo, "providedby"))
			rc_stringlist_ಕೂಡಿಸು(sorted, depinfo->service);
	}
}

RC_STRINGLIST *
rc_deptree_depend(ಸ್ಥಿರಾಂಕ RC_DEPTREE *deptree,
		  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *service, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type)
{
	RC_DEPINFO *di;
	RC_DEPTYPE *dt;
	RC_STRINGLIST *svcs;
	RC_STRING *svc;

	svcs = rc_stringlist_new();
	ಆದರೆ (!(di = get_depinfo(deptree, service)) ||
	    !(dt = get_deptype(di, type)))
	{
		errno = ENOENT;
		ಹಿಂತಿರುಗಿ svcs;
	}

	/* For consistency, we copy the array */
	TAILQ_FOREACH(svc, dt->services, entries)
		rc_stringlist_ಕೂಡಿಸು(svcs, svc->ಮೌಲ್ಯ);
	ಹಿಂತಿರುಗಿ svcs;
}

RC_STRINGLIST *
rc_deptree_depends(ಸ್ಥಿರಾಂಕ RC_DEPTREE *deptree,
		   ಸ್ಥಿರಾಂಕ RC_STRINGLIST *types,
		   ಸ್ಥಿರಾಂಕ RC_STRINGLIST *services,
		   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಪೂರ್ಣಾಂಕ options)
{
	RC_STRINGLIST *sorted = rc_stringlist_new();
	RC_STRINGLIST *ಭೇಟಿಯಾದ = rc_stringlist_new();
	RC_DEPINFO *di;
	ಸ್ಥಿರಾಂಕ RC_STRING *service;

	bootlevel = getenv("RC_BOOTLEVEL");
	ಆದರೆ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;
	TAILQ_FOREACH(service, services, entries) {
		ಆದರೆ (!(di = get_depinfo(deptree, service->ಮೌಲ್ಯ))) {
			errno = ENOENT;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (types)
			ಭೇಟಿ_service(deptree, types, sorted, ಭೇಟಿಯಾದ,
				      di, runlevel, options);
	}
	rc_stringlist_free(ಭೇಟಿಯಾದ);
	ಹಿಂತಿರುಗಿ sorted;
}

RC_STRINGLIST *
rc_deptree_ಕ್ರಮ(ಸ್ಥಿರಾಂಕ RC_DEPTREE *deptree, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel, ಪೂರ್ಣಾಂಕ options)
{
	RC_STRINGLIST *list;
	RC_STRINGLIST *list2;
	RC_STRINGLIST *types;
	RC_STRINGLIST *services;

	bootlevel = getenv("RC_BOOTLEVEL");
	ಆದರೆ (!bootlevel)
		bootlevel = RC_LEVEL_BOOT;

	/* When shutting down, list all running services */
	ಆದರೆ (strcmp(runlevel, RC_LEVEL_SINGLE) == 0 ||
	    strcmp(runlevel, RC_LEVEL_SHUTDOWN) == 0)
	{
		list = rc_services_ಒಳ_state(RC_SERVICE_STARTED);
		list2 = rc_services_ಒಳ_state(RC_SERVICE_INACTIVE);
		TAILQ_CONCAT(list, list2, entries);
		ಬಿಡುಗಡೆ(list2);
		list2 = rc_services_ಒಳ_state(RC_SERVICE_STARTING);
		TAILQ_CONCAT(list, list2, entries);
		ಬಿಡುಗಡೆ(list2);
	} ಇಲ್ಲದಿದ್ದರೆ {
		list = rc_services_ಒಳ_runlevel(RC_LEVEL_SYSINIT);
		ಆದರೆ (strcmp(runlevel, RC_LEVEL_SYSINIT) != 0) {
			list2 = rc_services_ಒಳ_runlevel(runlevel);
			TAILQ_CONCAT(list, list2, entries);
			ಬಿಡುಗಡೆ(list2);
			list2 = rc_services_ಒಳ_state(RC_SERVICE_HOTPLUGGED);
			TAILQ_CONCAT(list, list2, entries);
			ಬಿಡುಗಡೆ(list2);
			/* If we're not the boot runlevel then add that too */
			ಆದರೆ (strcmp(runlevel, bootlevel) != 0) {
				list2 = rc_services_ಒಳ_runlevel(bootlevel);
				TAILQ_CONCAT(list, list2, entries);
				ಬಿಡುಗಡೆ(list2);
			}
		}
	}

	/* Now we have our lists, we need to pull in any dependencies
	   and order them */
	types = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(types, "ineed");
	rc_stringlist_ಕೂಡಿಸು(types, "iuse");
	rc_stringlist_ಕೂಡಿಸು(types, "iwant");
	rc_stringlist_ಕೂಡಿಸು(types, "iafter");
	services = rc_deptree_depends(deptree, types, list, runlevel,
				      RC_DEP_STRICT | RC_DEP_TRACE | options);
	rc_stringlist_free(list);
	rc_stringlist_free(types);
	ಹಿಂತಿರುಗಿ services;
}


/* Given a time, recurse the target path to find out if there are
   any older (or newer) files.   If false, sets the time to the
   oldest (or newest) found.
*/
ಸ್ಥಿರ bool
deep_mtime_check(ಪೂರ್ಣಾಂಕ ಗುರಿ_dir, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ, bool newer, time_t *rel, ಅಕ್ಷರ *file)
{
	ರಚನೆ stat buf;
	bool retval = true;
	DIR *dp;
	ರಚನೆ dirent *d;

	/* If target does not exist, return true to mimic shell test */
	ಆದರೆ (fstatat(ಗುರಿ_dir, ಗುರಿ, &buf, 0) != 0)
		ಹಿಂತಿರುಗಿ true;

	ಆದರೆ (newer) {
		ಆದರೆ (*rel < buf.st_mtime) {
			retval = false;

			ಆದರೆ (file)
				strlcpy(file, ಗುರಿ, PATH_MAX);
			*rel = buf.st_mtime;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (*rel > buf.st_mtime) {
			retval = false;

			ಆದರೆ (file)
				strlcpy(file, ಗುರಿ, PATH_MAX);
			*rel = buf.st_mtime;
		}
	}

	ಆದರೆ (!S_ISDIR(buf.st_mode) || !(dp = do_opendirat(ಗುರಿ_dir, ಗುರಿ)))
		ಹಿಂತಿರುಗಿ retval;

	/* Check all the entries in the dir */
	ಆಗ ((d = readdir(dp))) {
		ಆದರೆ (d->d_name[0] == '.')
			ಮುಂದುವರಿ;
		ಆದರೆ (!deep_mtime_check(dirfd(dp), d->d_name, newer, rel, file)) {
			retval = false;
		}
	}

	closedir(dp);
	ಹಿಂತಿರುಗಿ retval;
}

/* Recursively check if target is older/newer than source.
 * If false, return the filename and most different time (if
 * the return value arguments are non-null).
 */
ಸ್ಥಿರ bool
mtime_check(ಪೂರ್ಣಾಂಕ dirfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ, bool newer, time_t *rel, ಅಕ್ಷರ *file)
{
	ರಚನೆ stat buf;
	time_t mtime;
	bool retval = true;

	/* We have to exist */
	ಆದರೆ (stat(ಮೂಲ, &buf) != 0)
		ಹಿಂತಿರುಗಿ false;
	mtime = buf.st_mtime;

	retval = deep_mtime_check(dirfd, ಗುರಿ,newer,&mtime,file);
	ಆದರೆ (rel) {
		*rel = mtime;
	}
	ಹಿಂತಿರುಗಿ retval;
}

bool
rc_newer_than(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ, time_t *newest, ಅಕ್ಷರ *file)
{

	ಹಿಂತಿರುಗಿ mtime_check(AT_FDCWD, ಮೂಲ, ಗುರಿ, true, newest, file);
}

bool
rc_older_than(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೂಲ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗುರಿ, time_t *oldest, ಅಕ್ಷರ *file)
{
	ಹಿಂತಿರುಗಿ mtime_check(AT_FDCWD, ಮೂಲ, ಗುರಿ, false, oldest, file);
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ deppair
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *depend;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *addto;
} DEPPAIR;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ DEPPAIR deppairs[] = {
	{ "ineed",	"needsme" },
	{ "iuse",	"usesme" },
	{ "iwant",	"wantsme" },
	{ "iafter",	"ibefore" },
	{ "ibefore",	"iafter" },
	{ "iprovide",	"providedby" },
	{ ಖಾಲಿ, ಖಾಲಿ }
};

bool
rc_deptree_update_needed(time_t *newest, ಅಕ್ಷರ *file)
{
	bool newer = false;
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ *dirfds;
	RC_STRINGLIST *config;
	RC_STRING *s;
	ರಚನೆ stat buf;
	time_t mtime;

	/* Quick test to see if anything we use has changed and we have
	 * data in our deptree. */
	ಆದರೆ (mkdir(rc_svcdir(), 0755) != 0 && errno != EEXIST)
		fprintf(stderr, "mkdir '%s': %s\n", rc_svcdir(), strerror(errno));

	clear_dirfds(); /* clear our cached dirfds as a sanity check */

	ಆದರೆ (fstatat(rc_dirfd(RC_DIR_SVCDIR), "deptree", &buf, 0) == 0) {
		mtime = buf.st_mtime;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* No previous cache found.
		 * We still run the scan, in case of clock skew; we still need to return
		 * the newest time.
		 */
		newer = true;
		mtime = time(ಖಾಲಿ);
	}

	ಗಾಗಿ (size_t i = 0, ಎಣಿಕೆ_2 = rc_scriptdirfds(&dirfds); i < ಎಣಿಕೆ_2; i++) {
		newer |= !deep_mtime_check(dirfds[i], "init.d", true, &mtime, file);
		newer |= !deep_mtime_check(dirfds[i], "conf.d", true, &mtime, file);
	}

	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf", true, &mtime, file);
	newer |= !deep_mtime_check(rc_dirfd(RC_DIR_SYSCONF), "rc.conf.d", true, &mtime, file);
	ಆದರೆ (rc_is_user()) {
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
	ಆದರೆ ((newer) && (newest != ಖಾಲಿ)) {
	    *newest = mtime;
	}

	ಹಿಂತಿರುಗಿ newer;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಸಿದ್ಧತೆ_environment(ಶೂನ್ಯ)
{
	ರಚನೆ utsname uts;
	size_t env_ಗಾತ್ರ;
	ಅಕ್ಷರ *env;
	FILE *mem = xopen_memstream(&env, &env_ಗಾತ್ರ);

	ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *dirs = rc_scriptdirs(); *dirs; dirs++) {
		fputs(*dirs, mem);
		ಆದರೆ (dirs[1])
			fputc(' ', mem);
	}
	xclose_memstream(mem);

	setenv("RC_SCRIPTDIRS", env, 1);
	ಬಿಡುಗಡೆ(env);

	/* Some init scripts need RC_LIBEXECDIR to source stuff
	   Ideally we should be setting our full env instead */
	ಆದರೆ (!getenv("RC_LIBEXECDIR"))
		setenv("RC_LIBEXECDIR", RC_LIBEXECDIR, 0);

	ಆದರೆ (uname(&uts) == 0)
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
	RC_DEPINFO *depinfo = ಖಾಲಿ, *depinfo_np, *di;
	RC_DEPTYPE *deptype = ಖಾಲಿ, *dt_np, *dt, *provide;
	RC_STRINGLIST *config, *dupes, *types, *sorted, *ಭೇಟಿಯಾದ;
	RC_STRING *s, *s2, *s2_np, *s3, *s4;
	ಅಕ್ಷರ *ಸಾಲು = ಖಾಲಿ;
	size_t ಗಾತ್ರ_2;
	ಅಕ್ಷರ *depend, *depends, *service, *type;
	size_t i, l;
	bool retval = true;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sys = rc_sys();
	ಪೂರ್ಣಾಂಕ serrno;

	/* Phase 1 - source all init scripts and print dependencies */
	ಸಿದ್ಧತೆ_environment();
	ಆದರೆ (!(fp = popen(GENDEP, "r")))
		ಹಿಂತಿರುಗಿ false;

	config = rc_stringlist_new();

	deptree = make_deptree();
	ಆಗ (xgetline(&ಸಾಲು, &ಗಾತ್ರ_2, fp) != -1) {
		depends = ಸಾಲು;
		service = strsep(&depends, " ");
		ಆದರೆ (!service || !*service)
			ಮುಂದುವರಿ;

		type = strsep(&depends, " ");
		ಆದರೆ (!depinfo || strcmp(depinfo->service, service) != 0) {
			deptype = ಖಾಲಿ;
			depinfo = get_depinfo(deptree, service);
			ಆದರೆ (!depinfo)
				depinfo = make_depinfo(deptree, service);
		}

		/* We may not have any depends */
		ಆದರೆ (!type || !depends)
			ಮುಂದುವರಿ;

		/* Get the type */
		ಆದರೆ (strcmp(type, "config") != 0) {
			ಆದರೆ (!deptype || strcmp(deptype->type, type) != 0) {
				deptype = get_deptype(depinfo, type);
				ಆದರೆ (!deptype)
					deptype = make_deptype(depinfo, type);
			}
		}

		/* Now add each depend to our type.
		   We do this individually so we handle multiple spaces gracefully */
		ಆಗ ((depend = strsep(&depends, " "))) {
			ಆದರೆ (depend[0] == 0)
				ಮುಂದುವರಿ;

			ಆದರೆ (strcmp(type, "config") == 0) {
				rc_stringlist_addu(config, depend);
				ಮುಂದುವರಿ;
			}

			/* Don't depend on ourself */
			ಆದರೆ (strcmp(depend, service) == 0)
				ಮುಂದುವರಿ;

			/* .sh files are not init scripts */
			l = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(depend);
			ಆದರೆ (l > 2 &&
			    depend[l - 3] == '.' &&
			    depend[l - 2] == 's' &&
			    depend[l - 1] == 'h')
				ಮುಂದುವರಿ;

			/* Remove our dependency if instructed */
			ಆದರೆ (depend[0] == '!') {
				rc_stringlist_delete(deptype->services, depend + 1);
				ಮುಂದುವರಿ;
			}

			rc_stringlist_ಕೂಡಿಸು(deptype->services, depend);

			/* We need to allow `after *; before local;` to work.
			 * Conversely, we need to allow 'before *; after modules' also */
			/* If we're before something, remove us from the after list */
			ಆದರೆ (strcmp(type, "ibefore") == 0) {
				ಆದರೆ ((dt = get_deptype(depinfo, "iafter")))
					rc_stringlist_delete(dt->services, depend);
			}
			/* If we're after something, remove us from the before list */
			ಆದರೆ (strcmp(type, "iafter") == 0 ||
			    strcmp(type, "ineed") == 0 ||
			    strcmp(type, "iwant") == 0 ||
			    strcmp(type, "iuse") == 0) {
				ಆದರೆ ((dt = get_deptype(depinfo, "ibefore")))
					rc_stringlist_delete(dt->services, depend);
			}
		}
	}
	ಬಿಡುಗಡೆ(ಸಾಲು);
	pclose(fp);

	/* Phase 2 - if we're a special system, remove services that don't
	 * work for them. This doesn't stop them from being run directly. */
	ಆದರೆ (sys) {
		ಅಕ್ಷರ *nosys, *onosys;
		size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(sys);

		nosys = xmalloc(len + 2);
		nosys[0] = '-';
		ಗಾಗಿ (i = 0; i < len; i++)
			nosys[i + 1] = (ಅಕ್ಷರ)tolower((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)sys[i]);
		nosys[i + 1] = '\0';

		onosys = xmalloc(len + 3);
		onosys[0] = 'n';
		onosys[1] = 'o';
		ಗಾಗಿ (i = 0; i < len; i++)
			onosys[i + 2] = (ಅಕ್ಷರ)tolower((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)sys[i]);
		onosys[i + 2] = '\0';

		TAILQ_FOREACH_ಸುರಕ್ಷಿತ(depinfo, deptree, entries, depinfo_np) {
			ಆದರೆ (!(deptype = get_deptype(depinfo, "keyword")))
				ಮುಂದುವರಿ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				ಆದರೆ (strcmp(s->ಮೌಲ್ಯ, nosys) != 0 && strcmp(s->ಮೌಲ್ಯ, onosys) != 0)
					ಮುಂದುವರಿ;
				provide = get_deptype(depinfo, "iprovide");
				TAILQ_REMOVE(deptree, depinfo, entries);
				TAILQ_FOREACH(di, deptree, entries) {
					TAILQ_FOREACH_ಸುರಕ್ಷಿತ(dt, &di->depends, entries, dt_np) {
						rc_stringlist_delete(dt->services, depinfo->service);
						ಆದರೆ (provide)
							TAILQ_FOREACH(s2, provide->services, entries)
								rc_stringlist_delete(dt->services, s2->ಮೌಲ್ಯ);
						ಆದರೆ (!TAILQ_ಮೊದಲ(dt->services)) {
							TAILQ_REMOVE(&di->depends, dt, entries);
							ಬಿಡುಗಡೆ(dt->type);
							ಬಿಡುಗಡೆ(dt->services);
							ಬಿಡುಗಡೆ(dt);
						}
					}
				}
			}
		}
		ಬಿಡುಗಡೆ(nosys);
		ಬಿಡುಗಡೆ(onosys);
	}

	/* Phase 3 - add our providers to the tree */
	providers = xmalloc(ಗಾತ್ರ(*providers));
	TAILQ_INIT(providers);
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ಆದರೆ (!(deptype = get_deptype(depinfo, "iprovide")))
			ಮುಂದುವರಿ;
		TAILQ_FOREACH(s, deptype->services, entries) {
			di = get_depinfo(providers, s->ಮೌಲ್ಯ);
			ಆದರೆ (!di)
				di = make_depinfo(providers, s->ಮೌಲ್ಯ);
		}
	}
	TAILQ_CONCAT(deptree, providers, entries);
	ಬಿಡುಗಡೆ(providers);

	/* Phase 4 - backreference our depends */
	TAILQ_FOREACH(depinfo, deptree, entries) {
		ಗಾಗಿ (i = 0; deppairs[i].depend; i++) {
			deptype = get_deptype(depinfo, deppairs[i].depend);
			ಆದರೆ (!deptype)
				ಮುಂದುವರಿ;
			TAILQ_FOREACH(s, deptype->services, entries) {
				di = get_depinfo(deptree, s->ಮೌಲ್ಯ);
				ಆದರೆ (!di) {
					ಆದರೆ (strcmp(deptype->type, "ineed") == 0) {
						fprintf(stderr, "Service '%s' needs non existent service '%s'\n",
							 depinfo->service, s->ಮೌಲ್ಯ);
						dt = get_deptype(depinfo, "broken");
						ಆದರೆ (!dt)
							dt = make_deptype(depinfo, "broken");
						rc_stringlist_addu(dt->services, s->ಮೌಲ್ಯ);
					}
					ಮುಂದುವರಿ;
				}

				dt = get_deptype(di, deppairs[i].addto);
				ಆದರೆ (!dt)
					dt = make_deptype(di, deppairs[i].addto);
				rc_stringlist_addu(dt->services, depinfo->service);
			}
		}
	}

	/* Phase 5 - Remove broken before directives */
	types = rc_stringlist_new();
	rc_stringlist_ಕೂಡಿಸು(types, "ineed");
	rc_stringlist_ಕೂಡಿಸು(types, "iwant");
	rc_stringlist_ಕೂಡಿಸು(types, "iuse");
	rc_stringlist_ಕೂಡಿಸು(types, "iafter");
	TAILQ_FOREACH(depinfo, deptree, entries) {
		deptype = get_deptype(depinfo, "ibefore");
		ಆದರೆ (!deptype)
			ಮುಂದುವರಿ;
		sorted = rc_stringlist_new();
		ಭೇಟಿಯಾದ = rc_stringlist_new();
		ಭೇಟಿ_service(deptree, types, sorted, ಭೇಟಿಯಾದ, depinfo,
			      ಖಾಲಿ, 0);
		rc_stringlist_free(ಭೇಟಿಯಾದ);
		TAILQ_FOREACH_ಸುರಕ್ಷಿತ(s2, deptype->services, entries, s2_np) {
			TAILQ_FOREACH(s3, sorted, entries) {
				di = get_depinfo(deptree, s3->ಮೌಲ್ಯ);
				ಆದರೆ (!di)
					ಮುಂದುವರಿ;
				ಆದರೆ (strcmp(s2->ಮೌಲ್ಯ, s3->ಮೌಲ್ಯ) == 0) {
					dt = get_deptype(di, "iafter");
					ಆದರೆ (dt)
						rc_stringlist_delete(dt->services, depinfo->service);
					ನಿಲ್ಲಿಸು;
				}
				dt = get_deptype(di, "iprovide");
				ಆದರೆ (!dt)
					ಮುಂದುವರಿ;
				TAILQ_FOREACH(s4, dt->services, entries) {
					ಆದರೆ (strcmp(s4->ಮೌಲ್ಯ, s2->ಮೌಲ್ಯ) == 0)
						ನಿಲ್ಲಿಸು;
				}
				ಆದರೆ (s4) {
					di = get_depinfo(deptree, s4->ಮೌಲ್ಯ);
					ಆದರೆ (di) {
						dt = get_deptype(di, "iafter");
						ಆದರೆ (dt)
							rc_stringlist_delete(dt->services, depinfo->service);
					}
					ನಿಲ್ಲಿಸು;
				}
			}
			ಆದರೆ (s3)
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
		ಆದರೆ (errno == EEXIST) {
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
	ಆದರೆ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "deptree", O_WRONLY | O_CREAT | O_TRUNC))) {
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
	} ಇಲ್ಲದಿದ್ದರೆ {
		fprintf(stderr, "fopen '%s/deptree': %s\n", rc_svcdir(), strerror(errno));
		retval = false;
	}

	/* Save our external config files to disk */
	ಆದರೆ (TAILQ_ಮೊದಲ(config)) {
		ಆದರೆ ((fp = do_fopenat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", O_WRONLY | O_CREAT | O_TRUNC))) {
			TAILQ_FOREACH(s, config, entries)
				fprintf(fp, "%s\n", s->ಮೌಲ್ಯ);
			fclose(fp);
		} ಇಲ್ಲದಿದ್ದರೆ {
			fprintf(stderr, "fopen '%s/depconfig': %s\n", rc_svcdir(), strerror(errno));
			retval = false;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		unlinkat(rc_dirfd(RC_DIR_SVCDIR), "depconfig", 0);
	}

	rc_stringlist_free(config);
	rc_deptree_free(deptree);
	ಹಿಂತಿರುಗಿ retval;
}
