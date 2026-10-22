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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "helpers.h"

ধ্রুব বর্ণ *applet = রিক্ত;

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ **argv)
{
	bool ok = false;
	বর্ণ *service;
	বর্ণ *exec;
	পূর্ণাঙ্ক idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	যদি (argc > 1)
		service = argv[1];
	অন্যথা
		service = getenv("RC_SVCNAME");

	যদি (service == রিক্ত || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	যদি (bit) {
		ok = (state & bit);
	} অন্যথা যদি (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		যদি (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} অন্যথা যদি (argc == 3) {
			যদি (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, রিক্ত, idx);

	} অন্যথা যদি (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} অন্যথা
		eerrorx("%s: unknown applet", applet);

	প্রত্যাগম ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
