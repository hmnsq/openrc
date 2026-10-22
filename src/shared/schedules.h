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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __RC_SCHEDULES_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __RC_SCHEDULES_H

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

ਖਾਲੀ free_schedulelist(ਖਾਲੀ);
ਪੂਰਨ parse_signal(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *sig);
ਖਾਲੀ parse_ਸਮਾਂ_ਸਾਰਣੀ(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *string, ਪੂਰਨ timeout);
ਪੂਰਨ do_stop(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
		pid_t pid, uid_t uid, ਪੂਰਨ sig, bool group, bool test, bool quiet);
ਪੂਰਨ ਲੜੀਵਾਰ_stop_ਸਮਾਂ_ਸਾਰਣੀ(ਅਚਲ ਅੱਖਰ *applet,
		ਅਚਲ ਅੱਖਰ *exec, ਅਚਲ ਅੱਖਰ *ਅਚਲ *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool ਤਰੱਕੀ, bool quiet);

#ਜੇ_ਅੰਤ
