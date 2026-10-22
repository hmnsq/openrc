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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "einfo.h"

పూర్ణాంకం ప్రధాన(శూన్య)
{
	స్థిరరాశి అక్షరం *p = getenv("RC_PID");
	పూర్ణాంకం pid;

	ఒకవేళ (p && అక్షరశ్రేణి_చదువు(p, "%d", &pid) == 1) {
		ఒకవేళ (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		తిరిగి EXIT_SUCCESS;
	}

	తిరిగి EXIT_FAILURE;
}
