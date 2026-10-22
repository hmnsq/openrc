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

#شامل <stdbool.h>
#شامل <sys/types.h>

بیرونی pid_t rc_logger_pid;
بیرونی صحیح rc_logger_tty;
بیرونی bool rc_in_logger;

تهی rc_logger_open(ثابت نویسه *runlevel);
تهی rc_logger_close(تهی);

#endif
