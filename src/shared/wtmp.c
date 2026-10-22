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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/utsname.h>
#અન્તર્ભાવ <utmp.h>

#અન્તર્ભાવ "wtmp.h"

શૂન્ય log_wtmp(અચલ અક્ષર *user, અચલ અક્ષર *id, pid_t pid, પૂર્ણાંક type,
		અચલ અક્ષર *લીટી)
{
	સંરચના timeval tv;
	સંરચના utmp utmp;
	સંરચના utsname uname_buf;

	memset(&utmp, 0, કદ(utmp));
	gettimeofday(&tv, ખાલી);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, કદ(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , કદ(utmp.ut_id  ));
	strncpy(utmp.ut_line, લીટી, કદ(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	જો (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, કદ(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
