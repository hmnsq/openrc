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

số_nguyên main(số_nguyên argc, char **argv)
{
	char *service = getenv("RC_SVCNAME");
	enum { GET, SET, EXPORT } action;
	char *option = NULL;

	applet = basename_c(argv[0]);
	nếu (service == NULL)
		eerrorx("%s: no service specified", applet);

	nếu (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	không_thì nếu (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	không_thì nếu (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	không_thì
		eerrorx("%s: unknown applet", applet);

	nếu (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	switch (action) {
	case GET:
		nếu (!(option = rc_service_value_get(service, argv[1])))
			trả_về EXIT_FAILURE;
		printf("%s", option);
		free(option);
		trả_về EXIT_SUCCESS;
	case SET:
		trả_về rc_service_value_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	case EXPORT:
		cho (số_nguyên i = 1; i < argc; i++) {
			char *var;
			nếu (rc_service_getenv(service, argv[i], NULL) > 0)
				continue;
			không_thì nếu ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			không_thì
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		trả_về EXIT_SUCCESS;
	}
}
