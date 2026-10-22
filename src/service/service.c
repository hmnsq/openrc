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

puur_naa_nka main(puur_naa_nka argc, var_na **argv)
{
	bool ok = false;
	var_na *service;
	var_na *exec;
	puur_naa_nka idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	ᰀᰧ (argc > 1)
		service = argv[1];
	ᰀᰧᰀ
		service = getenv("RC_SVCNAME");

	ᰀᰧ (service == NULL || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	ᰀᰧ (bit) {
		ok = (state & bit);
	} ᰀᰧᰀ ᰀᰧ (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		ᰀᰧ (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} ᰀᰧᰀ ᰀᰧ (argc == 3) {
			ᰀᰧ (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, NULL, idx);

	} ᰀᰧᰀ ᰀᰧ (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} ᰀᰧᰀ
		eerrorx("%s: unknown applet", applet);

	ᰀᰧᰌ ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
