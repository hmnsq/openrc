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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ଅଚଳ ଅକ୍ଷର *applet = ଖାଲି;
ବାହ୍ୟ ଅକ୍ଷର **environ;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *service = getenv("RC_SVCNAME");
	ଗଣନା { GET, SET, EXPORT } action;
	ଅକ୍ଷର *option = ଖାଲି;

	applet = basename_c(argv[0]);
	ଯଦି (service == ଖାଲି)
		eerrorx("%s: no service specified", applet);

	ଯଦି (strcmp(applet, "service_get_value") == 0 || strcmp(applet, "get_options") == 0)
		action = GET;
	ଅନ୍ୟଥା ଯଦି (strcmp(applet, "service_set_value") == 0 || strcmp(applet, "save_options") == 0)
		action = SET;
	ଅନ୍ୟଥା ଯଦି (strcmp(applet, "service_export") == 0)
		action = EXPORT;
	ଅନ୍ୟଥା
		eerrorx("%s: unknown applet", applet);

	ଯଦି (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	ବାଛ (action) {
	ସ୍ଥିତି GET:
		ଯଦି (!(option = rc_service_ମୂଲ୍ୟ_get(service, argv[1])))
			ଫେରନ୍ତୁ EXIT_FAILURE;
		printf("%s", option);
		ମୁକ୍ତ(option);
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	ସ୍ଥିତି SET:
		ଫେରନ୍ତୁ rc_service_ମୂଲ୍ୟ_set(service, argv[1], argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
	ସ୍ଥିତି EXPORT:
		ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 1; i < argc; i++) {
			ଅକ୍ଷର *var;
			ଯଦି (rc_service_getenv(service, argv[i], ଖାଲି) > 0)
				ଜାରି;
			ଅନ୍ୟଥା ଯଦି ((var = getenv(argv[i])))
				rc_service_setenv(service, argv[i], var);
			ଅନ୍ୟଥା
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}
}
