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

const char *applet = NULL;
extern char **environ;

ціле main(ціле argc, char **argv)
{
	char *service = getenv("RC_SVCNAME");
	enum { GET, SET, EXPORT } action;
	char *option = NULL;

	applet = basename_c(argv[0]);
	якщо (service == NULL)
		eerrorx("%s: no service specified", applet);

	якщо (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	інакше якщо (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	інакше якщо (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	інакше
		eerrorx("%s: unknown applet", applet);

	якщо (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	switch (action) {
	case GET:
		якщо (!(option = rc_service_value_get(service, argv[1])))
			повернути EXIT_FAILURE;
		printf("%s", option);
		free(option);
		повернути EXIT_SUCCESS;
	case SET:
		повернути rc_service_value_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	case EXPORT:
		для (ціле i = 1; i < argc; i++) {
			char *var;
			якщо (rc_service_getenv(service, argv[i], NULL) > 0)
				continue;
			інакше якщо ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			інакше
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		повернути EXIT_SUCCESS;
	}
}
