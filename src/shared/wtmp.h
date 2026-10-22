/*
 * rc-wtmp.h
 * This is private to us and not for user consumption
*/

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

#अगर_अघोषित __RC_WTMP_H__
#घोषणा __RC_WTMP_H__

#समावेश <utmp.h>
#समावेश <sys/types.h>

व्योम log_wtmp(स्थिर अक्षर *user, स्थिर अक्षर *id, pid_t pid, पूर्णांक type,
		स्थिर अक्षर *पंक्ति);

#पूर्ण_अगर
