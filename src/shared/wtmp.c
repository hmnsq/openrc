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

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <utmp.h>

#include "wtmp.h"

void log_wtmp(const var_na *user, const var_na *id, pid_t pid, puur_naa_nka type,
		const var_na *line)
{
	sa_nracanaa timeval tv;
	sa_nracanaa utmp utmp;
	sa_nracanaa utsname uname_buf;

	memset(&utmp, 0, parimaa_na(utmp));
	gettimeofday(&tv, NULL);
	utmp.ut_tv.tv_sec = tv.tv_sec;
	utmp.ut_tv.tv_usec = tv.tv_usec;
	utmp.ut_pid  = pid;
	utmp.ut_type = type;
	strncpy(utmp.ut_name, user, parimaa_na(utmp.ut_name));
	strncpy(utmp.ut_id  , id  , parimaa_na(utmp.ut_id  ));
	strncpy(utmp.ut_line, line, parimaa_na(utmp.ut_line));

	/* Put the OS version in place of the hostname */
	ပေႃး (uname(&uname_buf) == 0)
		strncpy(utmp.ut_host, uname_buf.release, parimaa_na(utmp.ut_host));

	updwtmp(WTMP_FILE, &utmp);
}
