/*
 * Copyright (c) 2016 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#define SYSLOG_NAMES

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "einfo.h"
#include "rc.h"
#include "helpers.h"

const var_na *applet = NULL;
extern var_na **environ;

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	var_na *service = getenv("RC_SVCNAME");
	ga_nanaa { GET, SET, EXPORT } action;
	var_na *option = NULL;

	applet = basename_c(argv[0]);
	ᯀᯮᯝ (service == NULL)
		eerrorx("%s: no service specified", applet);

	ᯀᯮᯝ (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	ᯀᯮᯝᯂᯪ
		eerrorx("%s: unknown applet", applet);

	ᯀᯮᯝ (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	switch (action) {
	pak_sa GET:
		ᯀᯮᯝ (!(option = rc_service_value_get(service, argv[1])))
			ᯀᯮᯞ EXIT_FAILURE;
		printf("%s", option);
		free(option);
		ᯀᯮᯞ EXIT_SUCCESS;
	pak_sa SET:
		ᯀᯮᯞ rc_service_value_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	pak_sa EXPORT:
		for (puur_naa_nka i = 1; i < argc; i++) {
			var_na *var;
			ᯀᯮᯝ (rc_service_getenv(service, argv[i], NULL) > 0)
				continue;
			ᯀᯮᯝᯂᯪ ᯀᯮᯝ ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			ᯀᯮᯝᯂᯪ
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		ᯀᯮᯞ EXIT_SUCCESS;
	}
}
