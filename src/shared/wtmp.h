/*
 * rc-wtmp.h
 * This is private to us and not for user consumption
*/

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

#جيڪڏهن_وصف_نه_ٿيل __RC_WTMP_H__
#وصف_ڪريو __RC_WTMP_H__

#ضمن <utmp.h>
#ضمن <sys/types.h>

فراغ log_wtmp(ثابت حرف *user, ثابت حرف *id, pid_t pid, صحيح type,
		ثابت حرف *سٽ);

#جيڪڏهن_پورو
