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

#شامل <errno.h>

#شامل "helpers.h"
#شامل "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#اگر defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#اگر_ختم

ساکن int64_t ts_مقصد_ms(ساخت timespec tv)
{
	int64_t sec_مقصد_ms = 1000, باری_اوپر = 500000, ns_مقصد_ms = 1000000;
	واپس (tv.tv_sec * sec_مقصد_ms) + ((tv.tv_nsec + باری_اوپر) / ns_مقصد_ms);
}

int64_t tm_now(خالی)
{
	ساخت timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	واپس ts_مقصد_ms(tv);
}

int64_t tm_sleep(int64_t ms, شمار tm_sleep_flags flags)
{
	صحیح ret;
	ساخت timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	برائے (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		اگر (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			واپس ts_مقصد_ms(rem);
		اگر (ret == 0)
			توڑو;
		tv = rem;
	}
	واپس 0;
}

int64_t parse_duration(ثابت حرف *duration)
{
	شمار delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	ساخت { ثابت حرف *ext; شمار delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	حرف *انتہا;
	طویل val;

	errno = 0;
	val = strtol(duration, &انتہا, 10);
	اگر (errno == ERANGE || *duration == '\0' || val < 0)
		واپس -1;
	برائے (size_t i = 0; i < ARRAY_سائز(exts); ++i) {
		اگر (strcmp(انتہا, exts[i].ext) == 0) {
			unit = exts[i].unit;
			توڑو;
		}
	}
	انتخاب (unit) {
	حالت UNIT_INVALID:
		واپس -1;
	طےشدہ:
		اگر (INT64_MAX/unit < val) /*حد_سے_تجاوز*/
			واپس -1;
		واپس (int64_t)val * unit;
	}
}
