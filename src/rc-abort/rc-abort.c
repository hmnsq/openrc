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

#شامل <errno.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <signal.h>
#شامل <string.h>

#شامل "einfo.h"

صحیح مین(خالی)
{
	ثابت حرف *p = getenv("RC_PID");
	صحیح pid;

	اگر (p && لڑی_سے_پڑھو(p, "%d", &pid) == 1) {
		اگر (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		واپس EXIT_SUCCESS;
	}

	واپس EXIT_FAILURE;
}
