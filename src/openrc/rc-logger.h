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

#अगर_अघोषित RC_LOGGER_H
#घोषणा RC_LOGGER_H

#समावेश <stdbool.h>
#समावेश <sys/types.h>

बाह्य pid_t rc_logger_pid;
बाह्य पूर्णांक rc_logger_tty;
बाह्य bool rc_in_logger;

व्योम rc_logger_open(स्थिर अक्षर *runlevel);
व्योम rc_logger_close(व्योम);

#पूर्ण_अगर
