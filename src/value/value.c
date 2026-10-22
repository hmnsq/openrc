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

정수 main(정수 argc, char **argv)
{
	char *service = getenv("RC_SVCNAME");
	enum { GET, SET, EXPORT } action;
	char *option = NULL;

	applet = basename_c(argv[0]);
	만약 (service == NULL)
		eerrorx("%s: no service specified", applet);

	만약 (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	아니면 만약 (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	아니면 만약 (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	아니면
		eerrorx("%s: unknown applet", applet);

	만약 (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	switch (action) {
	case GET:
		만약 (!(option = rc_service_value_get(service, argv[1])))
			반환 EXIT_FAILURE;
		printf("%s", option);
		free(option);
		반환 EXIT_SUCCESS;
	case SET:
		반환 rc_service_value_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	case EXPORT:
		위해 (정수 i = 1; i < argc; i++) {
			char *var;
			만약 (rc_service_getenv(service, argv[i], NULL) > 0)
				continue;
			아니면 만약 ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			아니면
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		반환 EXIT_SUCCESS;
	}
}
