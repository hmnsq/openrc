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

#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/time.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/utsname.h>
#অন্তর্ভাব <utmp.h>

#অন্তর্ভাব "wtmp.h"

শূন্য log_wtmp(ধ্রুব বর্ণ *user, ধ্রুব বর্ণ *id, pid_t pid, পূর্ণাঙ্ক type,
		ধ্রুব বর্ণ *সারি)
{
	সংরচনা timeval tv;
	সংরচনা utmp utmp;
	সংরচনা utsname uname_buf;

	memset(&utmp, 0, পরিমাণ(utmp));
	gettimeofday(&tv, রিক্ত);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, পরিমাণ(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , পরিমাণ(utmp.ut_id  ));
	strncpy(utmp.ut_line, সারি, পরিমাণ(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	যদি (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, পরিমাণ(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
