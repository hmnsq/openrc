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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	bool ok = false;
	ଅକ୍ଷର *service;
	ଅକ୍ଷର *exec;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	ଯଦି (argc > 1)
		service = argv[1];
	ଅନ୍ୟଥା
		service = getenv("RC_SVCNAME");

	ଯଦି (service == ଖାଲି || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	ଯଦି (bit) {
		ok = (state & bit);
	} ଅନ୍ୟଥା ଯଦି (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		ଯଦି (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} ଅନ୍ୟଥା ଯଦି (argc == 3) {
			ଯଦି (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, ଖାଲି, idx);

	} ଅନ୍ୟଥା ଯଦି (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} ଅନ୍ୟଥା
		eerrorx("%s: unknown applet", applet);

	ଫେରନ୍ତୁ ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
