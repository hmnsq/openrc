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

#ضمن <errno.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <signal.h>
#ضمن <string.h>

#ضمن "einfo.h"

صحيح مکھ(فراغ)
{
	ثابت حرف *p = getenv("RC_PID");
	صحيح pid;

	إذا (p && لڑی_توں_پڑھو(p, "%d", &pid) == 1) {
		إذا (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		أرجع EXIT_SUCCESS;
	}

	أرجع EXIT_FAILURE;
}
