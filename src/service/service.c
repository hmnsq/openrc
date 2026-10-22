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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	bool ok = false;
	எழுத்து *service;
	எழுத்து *exec;
	முழு idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	எனில் (argc > 1)
		service = argv[1];
	இல்லையெனில்
		service = getenv("RC_SVCNAME");

	எனில் (service == வெற்று || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	எனில் (bit) {
		ok = (state & bit);
	} இல்லையெனில் எனில் (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		எனில் (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} இல்லையெனில் எனில் (argc == 3) {
			எனில் (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, வெற்று, idx);

	} இல்லையெனில் எனில் (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} இல்லையெனில்
		eerrorx("%s: unknown applet", applet);

	திரும்பு ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
