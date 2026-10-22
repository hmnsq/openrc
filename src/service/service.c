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

#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "einfo.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	bool ok = false;
	अक्षर *service;
	अक्षर *exec;
	पूर्णांक idx = 0;
	RC_SERVICE state, bit;

	applet = basename_c(argv[0]);
	अगर (argc > 1)
		service = argv[1];
	अन्यथा
		service = दो_पर्या("RC_SVCNAME");

	अगर (service == शून्य || *service == '\0')
		eerrorx("%s: no service specified", applet);

	state = rc_service_state(service);
	bit = lookup_service_state(applet);
	अगर (bit) {
		ok = (state & bit);
	} अन्यथा अगर (म_भेद(applet, "service_started_daemon") == 0) {
		service = दो_पर्या("RC_SVCNAME");
		exec = argv[1];
		अगर (argc > 3) {
			service = argv[1];
			exec = argv[2];
			माला_पूछो(argv[3], "%d", &idx);
		} अन्यथा अगर (argc == 3) {
			अगर (माला_पूछो(argv[2], "%d", &idx) != 1) {
				service = argv[1];
				exec = argv[2];
			}
		}
		ok = rc_service_started_daemon(service, exec, शून्य, idx);

	} अन्यथा अगर (म_भेद(applet, "service_crashed") == 0) {
		ok = ( rc_service_daemons_crashed(service) && त्रुटि_सं != EACCES);
	} अन्यथा
		eerrorx("%s: unknown applet", applet);

	वापस ok ? निकास_सफल : निकास_त्रुटि;
}
