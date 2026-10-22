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

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/utsname.h>
#అన్తర్భావ <utmp.h>

#అన్తర్భావ "wtmp.h"

శూన్య log_wtmp(స్థిరరాశి అక్షరం *user, స్థిరరాశి అక్షరం *id, pid_t pid, పూర్ణాంకం type,
		స్థిరరాశి అక్షరం *పంక్తి)
{
	నిర్మాణం timeval tv;
	నిర్మాణం utmp utmp;
	నిర్మాణం utsname uname_buf;

	memset(&utmp, 0, పరిమాణం(utmp));
	gettimeofday(&tv, ఖాళీ);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, పరిమాణం(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , పరిమాణం(utmp.ut_id  ));
	strncpy(utmp.ut_line, పంక్తి, పరిమాణం(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	ఒకవేళ (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, పరిమాణం(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
