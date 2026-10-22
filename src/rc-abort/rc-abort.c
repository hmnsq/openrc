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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "einfo.h"

પૂર્ણાંક મુખ્ય(શૂન્ય)
{
	અચલ અક્ષર *p = getenv("RC_PID");
	પૂર્ણાંક pid;

	જો (p && sscanf(p, "%d", &pid) == 1) {
		જો (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		પાછા EXIT_SUCCESS;
	}

	પાછા EXIT_FAILURE;
}
