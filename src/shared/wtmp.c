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

#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/utsname.h>
#அந்தர்பாவ <utmp.h>

#அந்தர்பாவ "wtmp.h"

வெறுமை log_wtmp(மாறிலி எழுத்து *user, மாறிலி எழுத்து *id, pid_t pid, முழு type,
		மாறிலி எழுத்து *வரி)
{
	கட்டமைப்பு timeval tv;
	கட்டமைப்பு utmp utmp;
	கட்டமைப்பு utsname uname_buf;

	memset(&utmp, 0, அளவு(utmp));
	gettimeofday(&tv, வெற்று);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, அளவு(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , அளவு(utmp.ut_id  ));
	strncpy(utmp.ut_line, வரி, அளவு(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	எனில் (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, அளவு(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
