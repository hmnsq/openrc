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
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <signal.h>
#समावेश <string.h>

#समावेश "einfo.h"

पूर्णांक मुख्य(व्योम)
{
	स्थिर अक्षर *p = दो_पर्या("RC_PID");
	पूर्णांक pid;

	अगर (p && माला_पूछो(p, "%d", &pid) == 1) {
		अगर (समाप्त(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, म_त्रुटि(त्रुटि_सं));
		वापस निकास_सफल;
	}

	वापस निकास_त्रुटि;
}
