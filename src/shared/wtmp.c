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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/time.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h>
#ಅನ್ತರ್ಭಾವ <utmp.h>

#ಅನ್ತರ್ಭಾವ "wtmp.h"

ಶೂನ್ಯ log_wtmp(ಧ್ರುವ ವರ್ಣ *user, ಧ್ರುವ ವರ್ಣ *id, pid_t pid, ಪೂರ್ಣಾಙ್ಕ type,
		ಧ್ರುವ ವರ್ಣ *ಸಾಲು)
{
	ಸಂರಚನಾ timeval tv;
	ಸಂರಚನಾ utmp utmp;
	ಸಂರಚನಾ utsname uname_buf;

	memset(&utmp, 0, ಪರಿಮಾಣ(utmp));
	gettimeofday(&tv, ರಿಕ್ತ);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, ಪರಿಮಾಣ(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , ಪರಿಮಾಣ(utmp.ut_id  ));
	strncpy(utmp.ut_line, ಸಾಲು, ಪರಿಮಾಣ(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	ಯದಿ (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, ಪರಿಮಾಣ(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
