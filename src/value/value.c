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

स्थिरांक अक्षर *applet = रिक्त;
बाह्य अक्षर **environ;

पूर्णांक main(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *service = दो_पर्या("RC_SVCNAME");
	गणना { GET, SET, EXPORT } action;
	अक्षर *option = रिक्त;

	applet = basename_c(argv[0]);
	जर (service == रिक्त)
		eerrorx("%s: no service specified", applet);

	जर (म_भेद(applet, "service_get_value") == 0 || म_भेद(applet, "get_options") == 0)
		action = GET;
	अन्यथा जर (म_भेद(applet, "service_set_value") == 0 || म_भेद(applet, "save_options") == 0)
		action = SET;
	अन्यथा जर (म_भेद(applet, "service_export") == 0)
		action = EXPORT;
	अन्यथा
		eerrorx("%s: unknown applet", applet);

	जर (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	निवड (action) {
	स्थिती GET:
		जर (!(option = rc_service_मूल्य_get(service, argv[1])))
			परत निकास_त्रुटि;
		म_लिखो("%s", option);
		मुक्त(option);
		परत निकास_सफल;
	स्थिती SET:
		परत rc_service_मूल्य_set(service, argv[1], argv[2]) ? निकास_सफल : निकास_त्रुटि;
	स्थिती EXPORT:
		साठी (पूर्णांक i = 1; i < argc; i++) {
			अक्षर *var;
			जर (rc_service_getenv(service, argv[i], रिक्त) > 0)
				सुरूठेवा;
			अन्यथा जर ((var = दो_पर्या(argv[i])))
				rc_service_setenv(service, argv[i], var);
			अन्यथा
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		परत निकास_सफल;
	}
}
