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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "einfo.h"

முழு முதன்மை(வெறுமை)
{
	மாறிலி எழுத்து *p = getenv("RC_PID");
	முழு pid;

	எனில் (p && sscanf(p, "%d", &pid) == 1) {
		எனில் (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		திரும்பு EXIT_SUCCESS;
	}

	திரும்பு EXIT_FAILURE;
}
