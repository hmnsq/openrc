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

#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "einfo.h"
#ضمن "rc.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;
خارجي حرف **environ;

صحيح الرئيسية(صحيح argc, حرف **argv)
{
	حرف *service = getenv("RC_SVCNAME");
	تعداد { GET, SET, EXPORT } action;
	حرف *option = عدم;

	applet = basename_c(argv[0]);
	إذا (service == عدم)
		eerrorx("%s: no service specified", applet);

	إذا (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	وإلا إذا (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	وإلا إذا (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	وإلا
		eerrorx("%s: unknown applet", applet);

	إذا (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	بدل (action) {
	حالة GET:
		إذا (!(option = rc_service_قيمة_get(service, argv[1])))
			إرجاع EXIT_FAILURE;
		printf("%s", option);
		حرر(option);
		إرجاع EXIT_SUCCESS;
	حالة SET:
		إرجاع rc_service_قيمة_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	حالة EXPORT:
		لكل (صحيح i = 1; i < argc; i++) {
			حرف *var;
			إذا (rc_service_getenv(service, argv[i], عدم) > 0)
				استمر;
			وإلا إذا ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			وإلا
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		إرجاع EXIT_SUCCESS;
	}
}
