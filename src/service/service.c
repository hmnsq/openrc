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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	bool ok = false;
	ಅಕ್ಷರ *service;
	ಅಕ್ಷರ *exec;
	ಪೂರ್ಣಾಂಕ idx = 0;
	RC_SERVICE state, ಬಿಟ್;

	applet = basename_c(argv[0]);
	ಆದರೆ (argc > 1)
		service = argv[1];
	ಇಲ್ಲದಿದ್ದರೆ
		service = getenv("RC_SVCNAME");

	ಆದರೆ (service == ಖಾಲಿ || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	ಬಿಟ್ = lookup_service_state(applet);
	ಆದರೆ (ಬಿಟ್) {
		ok = (state & ಬಿಟ್);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		ಆದರೆ (argc > 3) {
			service = argv[1];
			exec = argv[2];
			ಅಕ್ಷರಸರಣಿ_ಓದು(argv[3], "%d", &idx);
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argc == 3) {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, ಖಾಲಿ, idx);

	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} ಇಲ್ಲದಿದ್ದರೆ
		eerrorx("%s: unknown applet", applet);

	ಹಿಂತಿರುಗಿ ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
