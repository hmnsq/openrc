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

#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/utsname.h>
#അന്തര്ഭാവ <utmp.h>

#അന്തര്ഭാവ "wtmp.h"

ശൂന്യം log_wtmp(സ്ഥിരം അക്ഷരം *user, സ്ഥിരം അക്ഷരം *id, pid_t pid, പൂർണ്ണസംഖ്യ type,
		സ്ഥിരം അക്ഷരം *വരി)
{
	ഘടന timeval tv;
	ഘടന utmp utmp;
	ഘടന utsname uname_buf;

	memset(&utmp, 0, വലുപ്പം(utmp));
	gettimeofday(&tv, ശൂന്യ);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, വലുപ്പം(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , വലുപ്പം(utmp.ut_id  ));
	strncpy(utmp.ut_line, വരി, വലുപ്പം(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	എങ്കിൽ (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, വലുപ്പം(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
