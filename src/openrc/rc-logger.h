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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <sys/types.h>

বাহ্যিক pid_t rc_logger_pid;
বাহ্যিক পূর্ণসংখ্যা rc_logger_tty;
বাহ্যিক bool rc_in_logger;

শূন্য rc_logger_open(ধ্রুবক অক্ষর *runlevel);
শূন্য rc_logger_close(শূন্য);

#endif
