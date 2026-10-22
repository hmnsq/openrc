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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RC_LOGGER_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_LOGGER_H

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

ਬਾਹਰੀ pid_t rc_logger_pid;
ਬਾਹਰੀ ਪੂਰਨ rc_logger_tty;
ਬਾਹਰੀ bool rc_ਵਿੱਚ_logger;

ਖਾਲੀ rc_logger_open(ਅਚਲ ਅੱਖਰ *runlevel);
ਖਾਲੀ rc_logger_close(ਖਾਲੀ);

#ਜੇ_ਅੰਤ
