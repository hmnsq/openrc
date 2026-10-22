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

#ifndef __RC_SCHEDULES_H
#define __RC_SCHEDULES_H

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <sys/types.h>

शून्य free_schedulelist(शून्य);
पूर्णांक parse_signal(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *sig);
शून्य parse_schedule(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *string, पूर्णांक timeout);
पूर्णांक do_stop(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv,
		pid_t pid, uid_t uid, पूर्णांक sig, bool group, bool test, bool quiet);
पूर्णांक run_stop_schedule(स्थिरांक अक्षर *applet,
		स्थिरांक अक्षर *exec, स्थिरांक अक्षर *स्थिरांक *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
