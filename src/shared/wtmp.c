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

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/utsname.h>
#अन्तर्भाव <utmp.h>

#अन्तर्भाव "wtmp.h"

शून्य log_wtmp(ध्रुव वर्ण *user, ध्रुव वर्ण *id, pid_t pid, पूर्णाङ्क type,
		ध्रुव वर्ण *ओळ)
{
	संरचना timeval tv;
	संरचना utmp utmp;
	संरचना utsname uname_buf;

	memset(&utmp, 0, परिमाण(utmp));
	gettimeofday(&tv, रिक्त);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, परिमाण(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , परिमाण(utmp.ut_id  ));
	strncpy(utmp.ut_line, ओळ, परिमाण(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	यदि (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, परिमाण(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
