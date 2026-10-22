/*
 * wtmp.c
 * This file contains routines to deal with the wtmp file.
 */

/*
 * Copyright 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <sys/time.h>
#समावेश <sys/types.h>
#समावेश <sys/utsname.h>
#समावेश <utmp.h>

#समावेश "wtmp.h"

व्योम log_wtmp(स्थिर अक्षर *user, स्थिर अक्षर *id, pid_t pid, पूर्णांक type,
		स्थिर अक्षर *पंक्ति)
{
	काष्ठा timeval tv;
	काष्ठा utmp utmp;
	काष्ठा utsname uname_buf;

	स_रखो(&utmp, 0, माप(utmp));
	gettimeofday(&tv, शून्य);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	म_नकलन(utmp.ut_name, user, माप(utmp.ut_name));
	म_नकलन(utmp.ut_id  , id  , माप(utmp.ut_id  ));
	म_नकलन(utmp.ut_line, पंक्ति, माप(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	अगर (uname(&uname_buf) == 0)
		म_नकलन(utmp.ut_host, uname_buf.release, माप(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
