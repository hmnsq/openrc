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
#ಆದರೆ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ int64_t ts_ಗಮ್ಯ_ms(ರಚನೆ timespec tv)
{
	int64_t sec_ಗಮ್ಯ_ms = 1000, ಸುತ್ತು_ಮೇಲೆ = 500000, ns_ಗಮ್ಯ_ms = 1000000;
	ಹಿಂತಿರುಗಿ (tv.tv_sec * sec_ಗಮ್ಯ_ms) + ((tv.tv_nsec + ಸುತ್ತು_ಮೇಲೆ) / ns_ಗಮ್ಯ_ms);
}

int64_t tm_now(ಶೂನ್ಯ)
{
	ರಚನೆ timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	ಹಿಂತಿರುಗಿ ts_ಗಮ್ಯ_ms(tv);
}

int64_t tm_sleep(int64_t ms, ಎಣಿಕೆ tm_sleep_flags flags)
{
	ಪೂರ್ಣಾಂಕ ret;
	ರಚನೆ timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	ಗಾಗಿ (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		ಆದರೆ (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			ಹಿಂತಿರುಗಿ ts_ಗಮ್ಯ_ms(rem);
		ಆದರೆ (ret == 0)
			ನಿಲ್ಲಿಸು;
		tv = rem;
	}
	ಹಿಂತಿರುಗಿ 0;
}

int64_t parse_duration(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *duration)
{
	ಎಣಿಕೆ delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	ರಚನೆ { ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ext; ಎಣಿಕೆ delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	ಅಕ್ಷರ *ಕೊನೆ;
	ದೀರ್ಘ val;

	errno = 0;
	val = strtol(duration, &ಕೊನೆ, 10);
	ಆದರೆ (errno == ERANGE || *duration == '\0' || val < 0)
		ಹಿಂತಿರುಗಿ -1;
	ಗಾಗಿ (size_t i = 0; i < ARRAY_ಗಾತ್ರ(exts); ++i) {
		ಆದರೆ (strcmp(ಕೊನೆ, exts[i].ext) == 0) {
			unit = exts[i].unit;
			ನಿಲ್ಲಿಸು;
		}
	}
	ಆಯ್ಕೆ (unit) {
	ಸ್ಥಿತಿ UNIT_INVALID:
		ಹಿಂತಿರುಗಿ -1;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಆದರೆ (INT64_MAX/unit < val) /*ಅತಿಪ್ರವಾಹ*/
			ಹಿಂತಿರುಗಿ -1;
		ಹಿಂತಿರುಗಿ (int64_t)val * unit;
	}
}
