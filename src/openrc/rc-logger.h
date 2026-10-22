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

#جے_بنا_تعریف RC_LOGGER_H
#تعریف_کرو RC_LOGGER_H

#ضمن <stdbool.h>
#ضمن <sys/types.h>

خارجي pid_t rc_logger_pid;
خارجي صحيح rc_logger_tty;
خارجي bool rc_وچ_logger;

فراغ rc_logger_open(ثابت حرف *runlevel);
فراغ rc_logger_close(فراغ);

#جے_انت
