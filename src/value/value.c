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

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;
बाह्य वर्ण **environ;

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	वर्ण *service = दो_पर्या("RC_SVCNAME");
	गणना { GET, SET, EXPORT } action;
	वर्ण *option = रिक्त;

	applet = basename_c(argv[0]);
	यदि (service == रिक्त)
		eerrorx("%s: no service specified", applet);

	यदि (म_भेद(applet, "service_get_value") == 0 || म_भेद(applet, "get_options") == 0)
		action = GET;
	अन्यथा यदि (म_भेद(applet, "service_set_value") == 0 || म_भेद(applet, "save_options") == 0)
		action = SET;
	अन्यथा यदि (म_भेद(applet, "service_export") == 0)
		action = EXPORT;
	अन्यथा
		eerrorx("%s: unknown applet", applet);

	यदि (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	विकल्प (action) {
	पक्ष GET:
		यदि (!(option = rc_service_मूल्य_get(service, argv[1])))
			प्रत्यागम निकास_त्रुटि;
		म_लिखो("%s", option);
		मुक्त(option);
		प्रत्यागम निकास_सफल;
	पक्ष SET:
		प्रत्यागम rc_service_मूल्य_set(service, argv[1], argv[2]) ? निकास_सफल : निकास_त्रुटि;
	पक्ष EXPORT:
		प्रति (पूर्णाङ्क i = 1; i < argc; i++) {
			वर्ण *var;
			यदि (rc_service_getenv(service, argv[i], रिक्त) > 0)
				अनुवर्त;
			अन्यथा यदि ((var = दो_पर्या(argv[i])))
				rc_service_setenv(service, argv[i], var);
			अन्यथा
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		प्रत्यागम निकास_सफल;
	}
}
