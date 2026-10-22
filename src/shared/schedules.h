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

#ضمن <stdbool.h>
#ضمن <sys/types.h>

فراغ free_schedulelist(فراغ);
صحيح parse_signal(ثابت حرف *applet, ثابت حرف *sig);
فراغ parse_schedule(ثابت حرف *applet, ثابت حرف *string, صحيح timeout);
صحيح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, صحيح sig, bool group, bool test, bool quiet);
صحيح run_stop_schedule(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
