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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "einfo.h"

পূর্ণাঙ্ক main(শূন্য)
{
	ধ্রুৱ ৱর্ণ *p = getenv("RC_PID");
	পূর্ণাঙ্ক pid;

	যদি (p && sscanf(p, "%d", &pid) == 1) {
		যদি (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		ঘূৰাই EXIT_SUCCESS;
	}

	ঘূৰাই EXIT_FAILURE;
}
