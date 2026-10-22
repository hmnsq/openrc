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

#شامل <stdbool.h>
#شامل <sys/types.h>

تهی free_schedulelist(تهی);
صحیح parse_signal(ثابت نویسه *applet, ثابت نویسه *sig);
تهی parse_schedule(ثابت نویسه *applet, ثابت نویسه *string, صحیح timeout);
صحیح do_stop(ثابت نویسه *applet, ثابت نویسه *exec, ثابت نویسه *ثابت *argv,
		pid_t pid, uid_t uid, صحیح sig, bool group, bool test, bool quiet);
صحیح run_stop_schedule(ثابت نویسه *applet,
		ثابت نویسه *exec, ثابت نویسه *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
