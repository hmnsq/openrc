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

शून्य log_wtmp(स्थिरांक अक्षर *user, स्थिरांक अक्षर *id, pid_t pid, पूर्णांक type,
		स्थिरांक अक्षर *ओळ)
{
	संरचना timeval tv;
	संरचना utmp utmp;
	संरचना utsname uname_buf;

	स_रखो(&utmp, 0, आकार(utmp));
	gettimeofday(&tv, रिक्त);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	म_नकलन(utmp.ut_name, user, आकार(utmp.ut_name));
	म_नकलन(utmp.ut_id  , id  , आकार(utmp.ut_id  ));
	म_नकलन(utmp.ut_line, ओळ, आकार(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	जर (uname(&uname_buf) == 0)
		म_नकलन(utmp.ut_host, uname_buf.release, आकार(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
