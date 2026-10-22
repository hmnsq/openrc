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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"

ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *p = getenv("RC_PID");
	ಪೂರ್ಣಾಙ್ಕ pid;

	ಯದಿ (p && sscanf(p, "%d", &pid) == 1) {
		ಯದಿ (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	}

	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
