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

#నిర్వచించు SYSLOG_పేర్లు

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "helpers.h"

స్థిరరాశి అక్షరం *applet = ఖాళీ;
బాహ్య అక్షరం **environ;

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం **argv)
{
	అక్షరం *service = getenv("RC_SVCNAME");
	గణన { GET, సమితి_2, EXPORT } action;
	అక్షరం *option = ఖాళీ;

	applet = basename_c(argv[0]);
	ఒకవేళ (service == ఖాళీ)
		eerrorx("%s: no service specified", applet);

	ఒకవేళ (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	లేకపోతే ఒకవేళ (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = సమితి_2;
	లేకపోతే ఒకవేళ (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	లేకపోతే
		eerrorx("%s: unknown applet", applet);

	ఒకవేళ (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	ఎంపిక (action) {
	స్థితి GET:
		ఒకవేళ (!(option = rc_service_విలువ_get(service, argv[1])))
			తిరిగి EXIT_FAILURE;
		రాయి("%s", option);
		విడుదల(option);
		తిరిగి EXIT_SUCCESS;
	స్థితి సమితి_2:
		తిరిగి rc_service_విలువ_సమితి(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	స్థితి EXPORT:
		కొరకు (పూర్ణాంకం i = 1; i < argc; i++) {
			అక్షరం *var;
			ఒకవేళ (rc_service_getenv(service, argv[i], ఖాళీ) > 0)
				కొనసాగు;
			లేకపోతే ఒకవేళ ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			లేకపోతే
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		తిరిగి EXIT_SUCCESS;
	}
}
