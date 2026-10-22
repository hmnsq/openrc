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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "einfo.h"

ကိန်းပြည့် main(void)
{
	const var_na *p = getenv("RC_PID");
	ကိန်းပြည့် pid;

	အကယ်၍ (p && sscanf(p, "%d", &pid) == 1) {
		အကယ်၍ (kill(pid, SIGUSR1) != 0)
			eerrorx("rc-abort: failed to signal parent %d: %s",
			    pid, strerror(errno));
		ပြန်ပေး EXIT_SUCCESS;
	}

	ပြန်ပေး EXIT_FAILURE;
}
