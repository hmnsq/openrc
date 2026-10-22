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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "einfo.h"

പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *p = getenv("RC_PID");
	പൂർണ്ണസംഖ്യ pid;

	എങ്കിൽ (p && sscanf(p, "%d", &pid) == 1) {
		എങ്കിൽ (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		തിരികെ EXIT_SUCCESS;
	}

	തിരികെ EXIT_FAILURE;
}
