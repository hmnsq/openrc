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

#اگر_غیر_تعریف_شدہ __RC_SCHEDULES_H
#تعریف_کرو __RC_SCHEDULES_H

#شامل <stdbool.h>
#شامل <sys/types.h>

خالی free_schedulelist(خالی);
صحیح parse_signal(ثابت حرف *applet, ثابت حرف *sig);
خالی parse_جدول_وقت(ثابت حرف *applet, ثابت حرف *string, صحیح timeout);
صحیح do_stop(ثابت حرف *applet, ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, صحیح sig, bool group, bool test, bool quiet);
صحیح تسلسل_stop_جدول_وقت(ثابت حرف *applet,
		ثابت حرف *exec, ثابت حرف *ثابت *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool پیش_رفت, bool quiet);

#اگر_ختم
