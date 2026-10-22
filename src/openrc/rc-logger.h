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

#اگر_غیر_تعریف_شدہ RC_LOGGER_H
#تعریف_کرو RC_LOGGER_H

#شامل <stdbool.h>
#شامل <sys/types.h>

خارجی pid_t rc_logger_pid;
خارجی صحیح rc_logger_tty;
خارجی bool rc_اندر_logger;

خالی rc_logger_open(ثابت حرف *runlevel);
خالی rc_logger_close(خالی);

#اگر_ختم
