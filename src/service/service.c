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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	bool ok = false;
	ਅੱਖਰ *service;
	ਅੱਖਰ *exec;
	ਪੂਰਨ idx = 0;
	RC_SERVICE state, ਦੋਆਧਾਰੀ_ਅੰਕ;

	applet = basename_c(argv[0]);
	ਜੇ (argc > 1)
		service = argv[1];
	ਨਹੀਂਤਾਂ
		service = getenv("RC_SVCNAME");

	ਜੇ (service == ਨਿਰਾ || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	ਦੋਆਧਾਰੀ_ਅੰਕ = lookup_service_state(applet);
	ਜੇ (ਦੋਆਧਾਰੀ_ਅੰਕ) {
		ok = (state & ਦੋਆਧਾਰੀ_ਅੰਕ);
	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		ਜੇ (argc > 3) {
			service = argv[1];
			exec = argv[2];
			ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(argv[3], "%d", &idx);
		} ਨਹੀਂਤਾਂ ਜੇ (argc == 3) {
			ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, ਨਿਰਾ, idx);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} ਨਹੀਂਤਾਂ
		eerrorx("%s: unknown applet", applet);

	ਵਾਪਸ ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
