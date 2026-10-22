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

ಶೂನ್ಯ log_wtmp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *user, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *id, pid_t pid, ಪೂರ್ಣಾಂಕ type,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಾಲು)
{
	ರಚನೆ timeval tv;
	ರಚನೆ utmp utmp;
	ರಚನೆ utsname uname_buf;

	memset(&utmp, 0, ಗಾತ್ರ(utmp));
	gettimeofday(&tv, ಖಾಲಿ);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, ಗಾತ್ರ(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , ಗಾತ್ರ(utmp.ut_id  ));
	strncpy(utmp.ut_line, ಸಾಲು, ಗಾತ್ರ(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	ಆದರೆ (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, ಗಾತ್ರ(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
