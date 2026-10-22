/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ifndef __RC_SCHEDULES_H
#define __RC_SCHEDULES_H

#include <stdbool.h>
#include <sys/types.h>

void free_schedulelist(void);
puur_naa_nka parse_signal(const var_na *applet, const var_na *sig);
void parse_schedule(const var_na *applet, const var_na *string, puur_naa_nka timeout);
puur_naa_nka do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, puur_naa_nka sig, bool group, bool test, bool quiet);
puur_naa_nka run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
