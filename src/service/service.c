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

จำนวนเต็ม หลัก(จำนวนเต็ม argc, var_na **argv)
{
	bool ok = false;
	var_na *service;
	var_na *exec;
	จำนวนเต็ม idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	ถ้า (argc > 1)
		service = argv[1];
	มิฉะนั้น
		service = getenv("RC_SVCNAME");

	ถ้า (service == NULL || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	ถ้า (bit) {
		ok = (state & bit);
	} มิฉะนั้น ถ้า (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		ถ้า (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} มิฉะนั้น ถ้า (argc == 3) {
			ถ้า (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, NULL, idx);

	} มิฉะนั้น ถ้า (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} มิฉะนั้น
		eerrorx("%s: unknown applet", applet);

	คืน ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
