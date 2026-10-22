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

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <utmp.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "wtmp.h"

ਖਾਲੀ log_wtmp(ਅਚਲ ਅੱਖਰ *user, ਅਚਲ ਅੱਖਰ *id, pid_t pid, ਪੂਰਨ type,
		ਅਚਲ ਅੱਖਰ *ਸਤਰ)
{
	ਬਣਤਰ timeval tv;
	ਬਣਤਰ utmp utmp;
	ਬਣਤਰ utsname uname_buf;

	memset(&utmp, 0, ਆਕਾਰ(utmp));
	gettimeofday(&tv, ਨਿਰਾ);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, ਆਕਾਰ(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , ਆਕਾਰ(utmp.ut_id  ));
	strncpy(utmp.ut_line, ਸਤਰ, ਆਕਾਰ(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	ਜੇ (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, ਆਕਾਰ(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
