/*
 * Copyright (c) 2025 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE file.
 */

#అన్తర్భావ <errno.h>

#అన్తర్భావ "helpers.h"
#అన్తర్భావ "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ఒకవేళ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#అయితే_ముగింపు

స్థిర int64_t ts_గమ్యం_ms(నిర్మాణం timespec tv)
{
	int64_t sec_గమ్యం_ms = 1000, విడత_పైకి = 500000, ns_గమ్యం_ms = 1000000;
	తిరిగి (tv.tv_sec * sec_గమ్యం_ms) + ((tv.tv_nsec + విడత_పైకి) / ns_గమ్యం_ms);
}

int64_t tm_now(శూన్య)
{
	నిర్మాణం timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	తిరిగి ts_గమ్యం_ms(tv);
}

int64_t tm_sleep(int64_t ms, గణన tm_sleep_flags flags)
{
	పూర్ణాంకం ret;
	నిర్మాణం timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	కొరకు (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		ఒకవేళ (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			తిరిగి ts_గమ్యం_ms(rem);
		ఒకవేళ (ret == 0)
			ఆపు;
		tv = rem;
	}
	తిరిగి 0;
}

int64_t parse_duration(స్థిరరాశి అక్షరం *duration)
{
	గణన delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	నిర్మాణం { స్థిరరాశి అక్షరం *ext; గణన delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	అక్షరం *చివర;
	దీర్ఘ val;

	errno = 0;
	val = strtol(duration, &చివర, 10);
	ఒకవేళ (errno == ERANGE || *duration == '\0' || val < 0)
		తిరిగి -1;
	కొరకు (size_t i = 0; i < ARRAY_కొలత(exts); ++i) {
		ఒకవేళ (strcmp(చివర, exts[i].ext) == 0) {
			unit = exts[i].unit;
			ఆపు;
		}
	}
	ఎంపిక (unit) {
	స్థితి UNIT_INVALID:
		తిరిగి -1;
	అప్రమేయ:
		ఒకవేళ (INT64_MAX/unit < val) /*అధికప్రవాహం*/
			తిరిగి -1;
		తిరిగి (int64_t)val * unit;
	}
}
