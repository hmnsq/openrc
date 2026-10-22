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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <sys/types.h>

બાહ્ય pid_t rc_logger_pid;
બાહ્ય પૂર્ણાંક rc_logger_tty;
બાહ્ય bool rc_in_logger;

શૂન્ય rc_logger_open(અચલ અક્ષર *runlevel);
શૂન્ય rc_logger_close(શૂન્ય);

#endif
