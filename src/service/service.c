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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിരം അക്ഷരം *applet = ശൂന്യ;

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	bool ok = false;
	അക്ഷരം *service;
	അക്ഷരം *exec;
	പൂർണ്ണസംഖ്യ idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	എങ്കിൽ (argc > 1)
		service = argv[1];
	അല്ലെങ്കിൽ
		service = getenv("RC_SVCNAME");

	എങ്കിൽ (service == ശൂന്യ || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	എങ്കിൽ (bit) {
		ok = (state & bit);
	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(applet, "service_started_daemon") == 0) {
		service = getenv("RC_SVCNAME");
		exec = argv[1];
		എങ്കിൽ (argc > 3) {
			service = argv[1];
			exec = argv[2];
			sscanf(argv[3], "%d", &idx);
		} അല്ലെങ്കിൽ എങ്കിൽ (argc == 3) {
			എങ്കിൽ (sscanf(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, ശൂന്യ, idx);

	} അല്ലെങ്കിൽ എങ്കിൽ (strcmp(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && errno != EACCES);
	} അല്ലെങ്കിൽ
		eerrorx("%s: unknown applet", applet);

	തിരികെ ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
