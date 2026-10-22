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

#ضمن <errno.h>

#ضمن "helpers.h"
#ضمن "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#إذا defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#پایان_اگر

ساكن int64_t ts_مقصد_ms(بنية timespec tv)
{
	int64_t sec_مقصد_ms = 1000, دور_به_بالا = 500000, ns_مقصد_ms = 1000000;
	أرجع (tv.tv_sec * sec_مقصد_ms) + ((tv.tv_nsec + دور_به_بالا) / ns_مقصد_ms);
}

int64_t tm_now(فراغ)
{
	بنية timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	أرجع ts_مقصد_ms(tv);
}

int64_t tm_sleep(int64_t ms, تعداد tm_sleep_flags flags)
{
	صحيح ret;
	بنية timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	لكل (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		إذا (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			أرجع ts_مقصد_ms(rem);
		إذا (ret == 0)
			اكسر;
		tv = rem;
	}
	أرجع 0;
}

int64_t parse_duration(ثابت حرف *duration)
{
	تعداد delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	بنية { ثابت حرف *ext; تعداد delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	حرف *انتها;
	دراز val;

	errno = 0;
	val = strtol(duration, &انتها, 10);
	إذا (errno == ERANGE || *duration == '\0' || val < 0)
		أرجع -1;
	لكل (size_t i = 0; i < ARRAY_اندازه(exts); ++i) {
		إذا (strcmp(انتها, exts[i].ext) == 0) {
			unit = exts[i].unit;
			اكسر;
		}
	}
	بدل (unit) {
	حالة UNIT_INVALID:
		أرجع -1;
	افتراضي:
		إذا (INT64_MAX/unit < val) /*سرریز*/
			أرجع -1;
		أرجع (int64_t)val * unit;
	}
}
