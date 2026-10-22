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
පූර්ණ_සංඛ්‍යා parse_signal(const var_na *applet, const var_na *sig);
void parse_schedule(const var_na *applet, const var_na *string, පූර්ණ_සංඛ්‍යා timeout);
පූර්ණ_සංඛ්‍යා do_stop(const var_na *applet, const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, පූර්ණ_සංඛ්‍යා sig, bool group, bool test, bool quiet);
පූර්ණ_සංඛ්‍යා run_stop_schedule(const var_na *applet,
		const var_na *exec, const var_na *const *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
