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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण **argv)
{
	bool ok = false;
	वर्ण *service;
	वर्ण *exec;
	पूर्णाङ्क idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	यदि (argc > 1)
		service = argv[1];
	अन्यथा
		service = getenv("RC_SVCNAME");

	यदि (service == रिक्त || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	यदि (bit) {
		ok = (state & bit);
	} अन्यथा यदि (वाक्यं_तुलय(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		यदि (argc > 3) {
			service = argv[1];
			exec = argv[2];
			वाक्यात्_पठ(argv[3], "%d", &idx);
		} अन्यथा यदि (argc == 3) {
			यदि (वाक्यात्_पठ(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, रिक्त, idx);

	} अन्यथा यदि (वाक्यं_तुलय(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} अन्यथा
		eerrorx("%s: unknown applet", applet);

	प्रत्यागम ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
