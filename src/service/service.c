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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	bool ok = false;
	అక్షరం *service;
	అక్షరం *exec;
	పూర్ణాంకం idx = 0;
	RC_SERVICE state, బిట్టు;

	applet = basename_c(argv[0]);
	ఒకవేళ (argc > 1)
		service = argv[1];
	లేకపోతే
		service = getenv("RC_SVCNAME");

	ఒకవేళ (service == ఖాళీ || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	బిట్టు = lookup_service_state(applet);
	ఒకవేళ (బిట్టు) {
		ok = (state & బిట్టు);
	} లేకపోతే ఒకవేళ (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		ఒకవేళ (argc > 3) {
			service = argv[1];
			exec = argv[2];
			అక్షరశ్రేణి_చదువు(argv[3], "%d", &idx);
		} లేకపోతే ఒకవేళ (argc == 3) {
			ఒకవేళ (అక్షరశ్రేణి_చదువు(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, ఖాళీ, idx);

	} లేకపోతే ఒకవేళ (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} లేకపోతే
		eerrorx("%s: unknown applet", applet);

	తిరిగి ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
