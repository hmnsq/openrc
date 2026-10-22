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

#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "helpers.h"
#অন্তর্ভাব "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#যদি defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

স্থির int64_t ts_to_ms(সংরচনা timespec tv)
{
	int64_t sec_to_ms = 1000, round_up = 500000, ns_to_ms = 1000000;
	প্রত্যাগম (tv.tv_sec * sec_to_ms) + ((tv.tv_nsec + round_up) / ns_to_ms);
}

int64_t tm_now(শূন্য)
{
	সংরচনা timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	প্রত্যাগম ts_to_ms(tv);
}

int64_t tm_sleep(int64_t ms, গণনা tm_sleep_flags flags)
{
	পূর্ণাঙ্ক ret;
	সংরচনা timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	প্রতি (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		যদি (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			প্রত্যাগম ts_to_ms(rem);
		যদি (ret == 0)
			ভঙ্গ;
		tv = rem;
	}
	প্রত্যাগম 0;
}

int64_t parse_duration(ধ্রুব বর্ণ *duration)
{
	গণনা delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	সংরচনা { ধ্রুব বর্ণ *ext; গণনা delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	বর্ণ *end;
	long val;

	errno = 0;
	val = strtol(duration, &end, 10);
	যদি (errno == ERANGE || *duration == '\0' || val < 0)
		প্রত্যাগম -1;
	প্রতি (size_t i = 0; i < ARRAY_SIZE(exts); ++i) {
		যদি (strcmp(end, exts[i].ext) == 0) {
			unit = exts[i].unit;
			ভঙ্গ;
		}
	}
	বিকল্প (unit) {
	পক্ষ UNIT_INVALID:
		প্রত্যাগম -1;
	স্বভাব:
		যদি (INT64_MAX/unit < val) /* overflow */
			প্রত্যাগম -1;
		প্রত্যাগম (int64_t)val * unit;
	}
}
