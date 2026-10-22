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

#ضمن <stdio.h>
#ضمن <string.h>
#ضمن <sys/time.h>
#ضمن <sys/types.h>
#ضمن <sys/utsname.h>
#ضمن <utmp.h>

#ضمن "wtmp.h"

فراغ log_wtmp(ثابت حرف *user, ثابت حرف *id, pid_t pid, صحيح type,
		ثابت حرف *line)
{
	بنية timeval tv;
	بنية utmp utmp;
	بنية utsname uname_buf;

	memset(&utmp, 0, حجم(utmp));
	gettimeofday(&tv, عدم);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, حجم(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , حجم(utmp.ut_id  ));
	strncpy(utmp.ut_line, line, حجم(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	𐡄𐡍 (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, حجم(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
