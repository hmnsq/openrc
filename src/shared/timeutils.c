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

#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#എങ്കിൽ defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#endif

സ്ഥിതം int64_t ts_to_ms(ഘടന timespec tv)
{
	int64_t sec_to_ms = 1000, round_up = 500000, ns_to_ms = 1000000;
	തിരികെ (tv.tv_sec * sec_to_ms) + ((tv.tv_nsec + round_up) / ns_to_ms);
}

int64_t tm_now(ശൂന്യം)
{
	ഘടന timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	തിരികെ ts_to_ms(tv);
}

int64_t tm_sleep(int64_t ms, എണ്ണൽ tm_sleep_flags flags)
{
	പൂർണ്ണസംഖ്യ ret;
	ഘടന timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	വേണ്ടി (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		എങ്കിൽ (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			തിരികെ ts_to_ms(rem);
		എങ്കിൽ (ret == 0)
			നിർത്തുക;
		tv = rem;
	}
	തിരികെ 0;
}

int64_t parse_duration(സ്ഥിരം അക്ഷരം *duration)
{
	എണ്ണൽ delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	ഘടന { സ്ഥിരം അക്ഷരം *ext; എണ്ണൽ delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	അക്ഷരം *end;
	long val;

	errno = 0;
	val = strtol(duration, &end, 10);
	എങ്കിൽ (errno == ERANGE || *duration == '\0' || val < 0)
		തിരികെ -1;
	വേണ്ടി (size_t i = 0; i < ARRAY_SIZE(exts); ++i) {
		എങ്കിൽ (strcmp(end, exts[i].ext) == 0) {
			unit = exts[i].unit;
			നിർത്തുക;
		}
	}
	തിരഞ്ഞെടുപ്പ് (unit) {
	അവസ്ഥ UNIT_INVALID:
		തിരികെ -1;
	സ്ഥിരസ്ഥിതി:
		എങ്കിൽ (INT64_MAX/unit < val) /* overflow */
			തിരികെ -1;
		തിരികെ (int64_t)val * unit;
	}
}
