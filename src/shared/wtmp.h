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

#ifndef __RC_WTMP_H__
#define __RC_WTMP_H__

#অন্তর্ভাব <utmp.h>
#অন্তর্ভাব <sys/types.h>

শূন্য log_wtmp(ধ্রুব বর্ণ *user, ধ্রুব বর্ণ *id, pid_t pid, পূর্ণাঙ্ক type,
		ধ্রুব বর্ণ *সারি);

#endif
