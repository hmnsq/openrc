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

#अगर_अघोषित __RC_SCHEDULES_H
#घोषणा __RC_SCHEDULES_H

#समावेश <stdbool.h>
#समावेश <sys/types.h>

व्योम free_schedulelist(व्योम);
पूर्णांक parse_signal(स्थिर अक्षर *applet, स्थिर अक्षर *sig);
व्योम parse_schedule(स्थिर अक्षर *applet, स्थिर अक्षर *string, पूर्णांक timeout);
पूर्णांक do_stop(स्थिर अक्षर *applet, स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv,
		pid_t pid, uid_t uid, पूर्णांक sig, bool group, bool test, bool quiet);
पूर्णांक run_stop_schedule(स्थिर अक्षर *applet,
		स्थिर अक्षर *exec, स्थिर अक्षर *स्थिर *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#पूर्ण_अगर
