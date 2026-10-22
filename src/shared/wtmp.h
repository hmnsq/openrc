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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __RC_WTMP_H__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __RC_WTMP_H__

#ਅਨ੍ਤਰ੍ਭਾਵ <utmp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

ਖਾਲੀ log_wtmp(ਅਚਲ ਅੱਖਰ *user, ਅਚਲ ਅੱਖਰ *id, pid_t pid, ਪੂਰਨ type,
		ਅਚਲ ਅੱਖਰ *ਸਤਰ);

#ਜੇ_ਅੰਤ
