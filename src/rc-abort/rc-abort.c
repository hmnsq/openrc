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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "einfo.h"

पूर्णाङ्क main(शून्य)
{
	ध्रुव वर्ण *p = getenv("RC_PID");
	पूर्णाङ्क pid;

	सचे (p && sscanf(p, "%d", &pid) == 1) {
		सचे (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		पटिनिवत्त EXIT_SUCCESS;
	}

	पटिनिवत्त EXIT_FAILURE;
}
