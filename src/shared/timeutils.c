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

#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ಯದಿ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

ಸ್ಥಿರ int64_t ts_to_ms(ಸಂರಚನಾ timespec tv)
{
	int64_t sec_to_ms = 1000, round_up = 500000, ns_to_ms = 1000000;
	ಪ್ರತ್ಯಾಗಮ (tv.tv_sec * sec_to_ms) + ((tv.tv_nsec + round_up) / ns_to_ms);
}

int64_t tm_now(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	ಪ್ರತ್ಯಾಗಮ ts_to_ms(tv);
}

int64_t tm_sleep(int64_t ms, ಗಣನಾ tm_sleep_flags flags)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಸಂರಚನಾ timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	ಪ್ರತಿ (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		ಯದಿ (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			ಪ್ರತ್ಯಾಗಮ ts_to_ms(rem);
		ಯದಿ (ret == 0)
			ಭಙ್ಗ;
		tv = rem;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

int64_t parse_duration(ಧ್ರುವ ವರ್ಣ *duration)
{
	ಗಣನಾ delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	ಸಂರಚನಾ { ಧ್ರುವ ವರ್ಣ *ext; ಗಣನಾ delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	ವರ್ಣ *end;
	long val;

	errno = 0;
	val = strtol(duration, &end, 10);
	ಯದಿ (errno == ERANGE || *duration == '\0' || val < 0)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಪ್ರತಿ (size_t i = 0; i < ARRAY_SIZE(exts); ++i) {
		ಯದಿ (strcmp(end, exts[i].ext) == 0) {
			unit = exts[i].unit;
			ಭಙ್ಗ;
		}
	}
	ವಿಕಲ್ಪ (unit) {
	ಪಕ್ಷ UNIT_INVALID:
		ಪ್ರತ್ಯಾಗಮ -1;
	ಸ್ವಭಾವ:
		ಯದಿ (INT64_MAX/unit < val) /* overflow */
			ಪ್ರತ್ಯಾಗಮ -1;
		ಪ್ರತ್ಯಾಗಮ (int64_t)val * unit;
	}
}
