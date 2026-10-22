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

#ifndef RC_LOGGER_H
#define RC_LOGGER_H

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <sys/types.h>

வெளி pid_t rc_logger_pid;
வெளி முழு rc_logger_tty;
வெளி bool rc_in_logger;

வெறுமை rc_logger_open(மாறிலி எழுத்து *runlevel);
வெறுமை rc_logger_close(வெறுமை);

#endif
