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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#ਜੇ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#ਜੇ_ਅੰਤ

ਸਥਿਰ int64_t ts_ਮੰਜ਼ਿਲ_ms(ਬਣਤਰ timespec tv)
{
	int64_t sec_ਮੰਜ਼ਿਲ_ms = 1000, ਗੇੜਾ_ਉੱਪਰ = 500000, ns_ਮੰਜ਼ਿਲ_ms = 1000000;
	ਵਾਪਸ (tv.tv_sec * sec_ਮੰਜ਼ਿਲ_ms) + ((tv.tv_nsec + ਗੇੜਾ_ਉੱਪਰ) / ns_ਮੰਜ਼ਿਲ_ms);
}

int64_t tm_now(ਖਾਲੀ)
{
	ਬਣਤਰ timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	ਵਾਪਸ ts_ਮੰਜ਼ਿਲ_ms(tv);
}

int64_t tm_sleep(int64_t ms, ਗਿਣਤੀ tm_sleep_flags flags)
{
	ਪੂਰਨ ret;
	ਬਣਤਰ timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	ਲਈ (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		ਜੇ (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			ਵਾਪਸ ts_ਮੰਜ਼ਿਲ_ms(rem);
		ਜੇ (ret == 0)
			ਰੋਕੋ;
		tv = rem;
	}
	ਵਾਪਸ 0;
}

int64_t parse_duration(ਅਚਲ ਅੱਖਰ *duration)
{
	ਗਿਣਤੀ delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	ਬਣਤਰ { ਅਚਲ ਅੱਖਰ *ext; ਗਿਣਤੀ delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	ਅੱਖਰ *ਸਿਰਾ;
	ਲੰਮਾ val;

	errno = 0;
	val = strtol(duration, &ਸਿਰਾ, 10);
	ਜੇ (errno == ERANGE || *duration == '\0' || val < 0)
		ਵਾਪਸ -1;
	ਲਈ (size_t i = 0; i < ARRAY_ਮਾਪ(exts); ++i) {
		ਜੇ (strcmp(ਸਿਰਾ, exts[i].ext) == 0) {
			unit = exts[i].unit;
			ਰੋਕੋ;
		}
	}
	ਚੋਣ (unit) {
	ਹਾਲਤ UNIT_INVALID:
		ਵਾਪਸ -1;
	ਮੂਲ:
		ਜੇ (INT64_MAX/unit < val) /*ਉਛਾਲ*/
			ਵਾਪਸ -1;
		ਵਾਪਸ (int64_t)val * unit;
	}
}
