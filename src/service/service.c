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

#شامل <errno.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "einfo.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "helpers.h"

ثابت نویسه *applet = پوچ;

صحیح اصلی(صحیح argc, نویسه **argv)
{
	bool ok = false;
	نویسه *service;
	نویسه *exec;
	صحیح idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	اگر (argc > 1)
		service = argv[1];
	وگرنه
		service = getenv("RC_SVCNAME");

	اگر (service == پوچ || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	اگر (bit) {
		ok = (state & bit);
	} وگرنه اگر (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		اگر (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} وگرنه اگر (argc == 3) {
			اگر (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, پوچ, idx);

	} وگرنه اگر (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} وگرنه
		eerrorx("%s: unknown applet", applet);

	بازگشت ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
