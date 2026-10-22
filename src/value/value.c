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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SYSLOG_ਨਾਂ

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਅਚਲ ਅੱਖਰ *applet = ਨਿਰਾ;
ਬਾਹਰੀ ਅੱਖਰ **environ;

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *service = getenv("RC_SVCNAME");
	ਗਿਣਤੀ { GET, ਸੈੱਟ_2, EXPORT } action;
	ਅੱਖਰ *option = ਨਿਰਾ;

	applet = basename_c(argv[0]);
	ਜੇ (service == ਨਿਰਾ)
		eerrorx("%s: no service specified", applet);

	ਜੇ (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = ਸੈੱਟ_2;
	ਨਹੀਂਤਾਂ ਜੇ (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	ਨਹੀਂਤਾਂ
		eerrorx("%s: unknown applet", applet);

	ਜੇ (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	ਚੋਣ (action) {
	ਹਾਲਤ GET:
		ਜੇ (!(option = rc_service_ਮੁੱਲ_get(service, argv[1])))
			ਵਾਪਸ EXIT_FAILURE;
		ਲਿਖੋ("%s", option);
		ਮੁਕਤ(option);
		ਵਾਪਸ EXIT_SUCCESS;
	ਹਾਲਤ ਸੈੱਟ_2:
		ਵਾਪਸ rc_service_ਮੁੱਲ_ਸੈੱਟ(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	ਹਾਲਤ EXPORT:
		ਲਈ (ਪੂਰਨ i = 1; i < argc; i++) {
			ਅੱਖਰ *var;
			ਜੇ (rc_service_getenv(service, argv[i], ਨਿਰਾ) > 0)
				ਜਾਰੀ;
			ਨਹੀਂਤਾਂ ਜੇ ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			ਨਹੀਂਤਾਂ
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		ਵਾਪਸ EXIT_SUCCESS;
	}
}
