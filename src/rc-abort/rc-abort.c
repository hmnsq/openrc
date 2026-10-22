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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"

ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *p = getenv("RC_PID");
	ਪੂਰਨ pid;

	ਜੇ (p && ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(p, "%d", &pid) == 1) {
		ਜੇ (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		ਵਾਪਸ EXIT_SUCCESS;
	}

	ਵਾਪਸ EXIT_FAILURE;
}
