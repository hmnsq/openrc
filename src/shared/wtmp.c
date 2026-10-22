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

#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/time.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/utsname.h>
#ଅନ୍ତର୍ଭାଵ <utmp.h>

#ଅନ୍ତର୍ଭାଵ "wtmp.h"

ଶୂନ୍ୟ log_wtmp(ଅଚଳ ଅକ୍ଷର *user, ଅଚଳ ଅକ୍ଷର *id, pid_t pid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type,
		ଅଚଳ ଅକ୍ଷର *ଧାଡ଼ି)
{
	ସଂରଚନା timeval tv;
	ସଂରଚନା utmp utmp;
	ସଂରଚନା utsname uname_buf;

	memset(&utmp, 0, ଆକାର(utmp));
	gettimeofday(&tv, ଖାଲି);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, ଆକାର(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , ଆକାର(utmp.ut_id  ));
	strncpy(utmp.ut_line, ଧାଡ଼ି, ଆକାର(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	ଯଦି (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, ଆକାର(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
