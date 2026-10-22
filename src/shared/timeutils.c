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

#समावेश <errno.h>

#समावेश "helpers.h"
#समावेश "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#अगर defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#पूर्ण_अगर

अटल int64_t ts_to_ms(काष्ठा timespec tv)
{
	int64_t sec_to_ms = 1000, round_up = 500000, ns_to_ms = 1000000;
	वापस (tv.tv_sec * sec_to_ms) + ((tv.tv_nsec + round_up) / ns_to_ms);
}

int64_t tm_now(व्योम)
{
	काष्ठा timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	वापस ts_to_ms(tv);
}

int64_t tm_sleep(int64_t ms, क्रमागत tm_sleep_flags flags)
{
	पूर्णांक ret;
	काष्ठा timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	क्रम (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		अगर (ret < 0 && (!(flags & TM_NO_EINTR) || त्रुटि_सं != EINTR))
			वापस ts_to_ms(rem);
		अगर (ret == 0)
			अवरोध;
		tv = rem;
	}
	वापस 0;
}

int64_t parse_duration(स्थिर अक्षर *duration)
{
	क्रमागत delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	काष्ठा { स्थिर अक्षर *ext; क्रमागत delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	अक्षर *end;
	दीर्घ val;

	त्रुटि_सं = 0;
	val = म_से_दीर्घ(duration, &end, 10);
	अगर (त्रुटि_सं == दुस्फल || *duration == '\0' || val < 0)
		वापस -1;
	क्रम (माप_प्रकार i = 0; i < ARRAY_आकार(exts); ++i) {
		अगर (म_भेद(end, exts[i].ext) == 0) {
			unit = exts[i].unit;
			अवरोध;
		}
	}
	चयन (unit) {
	हाल UNIT_INVALID:
		वापस -1;
	शेष:
		अगर (INT64_MAX/unit < val) /* overflow */
			वापस -1;
		वापस (int64_t)val * unit;
	}
}
