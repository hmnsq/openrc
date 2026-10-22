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

#ضمن <errno.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "einfo.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "helpers.h"

ثابت حرف *applet = عدم;

صحيح مکيه(صحيح argc, حرف **argv)
{
	bool ok = false;
	حرف *service;
	حرف *exec;
	صحيح idx = 0;
	RC_SERVICE state, ٻٽو_انگ;

	applet = basename_c(argv[0]);
	إذا (argc > 1)
		service = argv[1];
	وإلا
		service = getenv("RC_SVCNAME");

	إذا (service == عدم || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	ٻٽو_انگ = lookup_service_state(applet);
	إذا (ٻٽو_انگ) {
		ok = (state & ٻٽو_انگ);
	} وإلا إذا (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		إذا (argc > 3) {
			service = argv[1];
			exec = argv[2];
			لڙي_مان_پڙهو(argv[3], "%d", &idx);
		} وإلا إذا (argc == 3) {
			إذا (لڙي_مان_پڙهو(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, عدم, idx);

	} وإلا إذا (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} وإلا
		eerrorx("%s: unknown applet", applet);

	أرجع ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
