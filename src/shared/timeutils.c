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

#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#எனில் defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

நிலையான int64_t ts_to_ms(கட்டமைப்பு timespec tv)
{
	int64_t sec_to_ms = 1000, round_up = 500000, ns_to_ms = 1000000;
	திரும்பு (tv.tv_sec * sec_to_ms) + ((tv.tv_nsec + round_up) / ns_to_ms);
}

int64_t tm_now(வெறுமை)
{
	கட்டமைப்பு timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	திரும்பு ts_to_ms(tv);
}

int64_t tm_sleep(int64_t ms, எண்ணிக்கை tm_sleep_flags flags)
{
	முழு ret;
	கட்டமைப்பு timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	க்கு (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		எனில் (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			திரும்பு ts_to_ms(rem);
		எனில் (ret == 0)
			நிறுத்து;
		tv = rem;
	}
	திரும்பு 0;
}

int64_t parse_duration(மாறிலி எழுத்து *duration)
{
	எண்ணிக்கை delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	கட்டமைப்பு { மாறிலி எழுத்து *ext; எண்ணிக்கை delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	எழுத்து *end;
	long val;

	errno = 0;
	val = strtol(duration, &end, 10);
	எனில் (errno == ERANGE || *duration == '\0' || val < 0)
		திரும்பு -1;
	க்கு (size_t i = 0; i < ARRAY_SIZE(exts); ++i) {
		எனில் (strcmp(end, exts[i].ext) == 0) {
			unit = exts[i].unit;
			நிறுத்து;
		}
	}
	தேர்வு (unit) {
	நிலை UNIT_INVALID:
		திரும்பு -1;
	இயல்பு:
		எனில் (INT64_MAX/unit < val) /* overflow */
			திரும்பு -1;
		திரும்பு (int64_t)val * unit;
	}
}
