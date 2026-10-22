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

#घोषणा SYSLOG_NAMES

#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "einfo.h"
#समावेश "rc.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;
बाह्य अक्षर **environ;

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	अक्षर *service = दो_पर्या("RC_SVCNAME");
	क्रमागत { GET, SET, EXPORT } action;
	अक्षर *option = शून्य;

	applet = basename_c(argv[0]);
	अगर (service == शून्य)
		eerrorx("%s: no service specified", applet);

	अगर (म_भेद(applet, "service_get_value") == 0 || म_भेद(applet, "get_options") == 0)
		action = GET;
	अन्यथा अगर (म_भेद(applet, "service_set_value") == 0 || म_भेद(applet, "save_options") == 0)
		action = SET;
	अन्यथा अगर (म_भेद(applet, "service_export") == 0)
		action = EXPORT;
	अन्यथा
		eerrorx("%s: unknown applet", applet);

	अगर (argc < 2 || !argv[1] || *argv[1] == '\0')
		eerrorx("%s: no %s specified", applet, action == EXPORT ? "variable" : "option");

	चयन (action) {
	हाल GET:
		अगर (!(option = rc_service_मूल्य_get(service, argv[1])))
			वापस निकास_त्रुटि;
		म_लिखो("%s", option);
		मुक्त(option);
		वापस निकास_सफल;
	हाल SET:
		वापस rc_service_मूल्य_set(service, argv[1], argv[2]) ? निकास_सफल : निकास_त्रुटि;
	हाल EXPORT:
		क्रम (पूर्णांक i = 1; i < argc; i++) {
			अक्षर *var;
			अगर (rc_service_getenv(service, argv[i], शून्य) > 0)
				जारी;
			अन्यथा अगर ((var = दो_पर्या(argv[i])))
				rc_service_setenv(service, argv[i], var);
			अन्यथा
				ewarn("%s: Missing reexport variable '%s'", service, argv[i]);
		}
		वापस निकास_सफल;
	}
}
