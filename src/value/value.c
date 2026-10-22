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

#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "einfo.h"
#شامل "rc.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;
بیرونی نویسه **environ;

صحیح اصلی(صحیح argc, نویسه **argv)
{
	نویسه *service = getenv("RC_SVCNAME");
	شمارش { GET, SET, EXPORT } action;
	نویسه *option = پوچ;

	applet = basename_c(argv[0]);
	اگر (service == پوچ)
		eerrorx("%s: no service specified", applet);

	اگر (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	وگرنه اگر (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	وگرنه اگر (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	وگرنه
		eerrorx("%s: unknown applet", applet);

	اگر (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	انتخاب (action) {
	حالت GET:
		اگر (!(option = rc_service_مقدار_get(service, argv[1])))
			بازگشت EXIT_FAILURE;
		printf("%s", option);
		آزاد(option);
		بازگشت EXIT_SUCCESS;
	حالت SET:
		بازگشت rc_service_مقدار_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	حالت EXPORT:
		برای (صحیح i = 1; i < argc; i++) {
			نویسه *var;
			اگر (rc_service_getenv(service, argv[i], پوچ) > 0)
				ادامه;
			وگرنه اگر ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			وگرنه
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		بازگشت EXIT_SUCCESS;
	}
}
