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

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

ବାହ୍ୟ pid_t rc_logger_pid;
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc_logger_tty;
ବାହ୍ୟ bool rc_in_logger;

ଶୂନ୍ୟ rc_logger_open(ଅଚଳ ଅକ୍ଷର *runlevel);
ଶୂନ୍ୟ rc_logger_close(ଶୂନ୍ୟ);

#endif
