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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "helpers.h"

અચલ અક્ષર *applet = ખાલી;

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર **argv)
{
	bool ok = false;
	અક્ષર *service;
	અક્ષર *exec;
	પૂર્ણાંક idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	જો (argc > 1)
		service = argv[1];
	નહીંતર
		service = getenv("RC_SVCNAME");

	જો (service == ખાલી || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	જો (bit) {
		ok = (state & bit);
	} નહીંતર જો (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		જો (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} નહીંતર જો (argc == 3) {
			જો (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, ખાલી, idx);

	} નહીંતર જો (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} નહીંતર
		eerrorx("%s: unknown applet", applet);

	પાછા ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
