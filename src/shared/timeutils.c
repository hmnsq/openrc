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

#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ଯଦି defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

ସ୍ଥିର int64_t ts_to_ms(ସଂରଚନା timespec tv)
{
	int64_t sec_to_ms = 1000, round_up = 500000, ns_to_ms = 1000000;
	ଫେରନ୍ତୁ (tv.tv_sec * sec_to_ms) + ((tv.tv_nsec + round_up) / ns_to_ms);
}

int64_t tm_now(ଶୂନ୍ୟ)
{
	ସଂରଚନା timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	ଫେରନ୍ତୁ ts_to_ms(tv);
}

int64_t tm_sleep(int64_t ms, ଗଣନା tm_sleep_flags flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ସଂରଚନା timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	ପାଇଁ (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		ଯଦି (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			ଫେରନ୍ତୁ ts_to_ms(rem);
		ଯଦି (ret == 0)
			ଭାଙ୍ଗ;
		tv = rem;
	}
	ଫେରନ୍ତୁ 0;
}

int64_t parse_duration(ଅଚଳ ଅକ୍ଷର *duration)
{
	ଗଣନା delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	ସଂରଚନା { ଅଚଳ ଅକ୍ଷର *ext; ଗଣନା delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	ଅକ୍ଷର *end;
	long val;

	errno = 0;
	val = strtol(duration, &end, 10);
	ଯଦି (errno == ERANGE || *duration == '\0' || val < 0)
		ଫେରନ୍ତୁ -1;
	ପାଇଁ (size_t i = 0; i < ARRAY_SIZE(exts); ++i) {
		ଯଦି (strcmp(end, exts[i].ext) == 0) {
			unit = exts[i].unit;
			ଭାଙ୍ଗ;
		}
	}
	ବାଛ (unit) {
	ସ୍ଥିତି UNIT_INVALID:
		ଫେରନ୍ତୁ -1;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଯଦି (INT64_MAX/unit < val) /* overflow */
			ଫେରନ୍ତୁ -1;
		ଫେରନ୍ତୁ (int64_t)val * unit;
	}
}
