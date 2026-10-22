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

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "einfo.h"
#include "rc.h"
#include "misc.h"
#include "helpers.h"

const char *applet = NULL;

정수 main(정수 argc, char **argv)
{
	bool ok = false;
	char *service;
	char *exec;
	정수 idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	만약 (argc > 1)
		service = argv[1];
	아니면
		service = getenv("RC_SVCNAME");

	만약 (service == NULL || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	만약 (bit) {
		ok = (state & bit);
	} 아니면 만약 (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		만약 (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} 아니면 만약 (argc == 3) {
			만약 (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, NULL, idx);

	} 아니면 만약 (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} 아니면
		eerrorx("%s: unknown applet", applet);

	반환 ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
