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

const var_na *applet = NULL;

ཧྲིལ main(ཧྲིལ argc, var_na **argv)
{
	bool ok = false;
	var_na *service;
	var_na *exec;
	ཧྲིལ idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	གལ་སྲིད (argc > 1)
		service = argv[1];
	དེ་མིན
		service = getenv("RC_SVCNAME");

	གལ་སྲིད (service == NULL || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	གལ་སྲིད (bit) {
		ok = (state & bit);
	} དེ་མིན གལ་སྲིད (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		གལ་སྲིད (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} དེ་མིན གལ་སྲིད (argc == 3) {
			གལ་སྲིད (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, NULL, idx);

	} དེ་མིན གལ་སྲིད (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} དེ་མིན
		eerrorx("%s: unknown applet", applet);

	ལོག ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
