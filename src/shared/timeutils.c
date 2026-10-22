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

#अन्तर्भाव <errno.h>

#अन्तर्भाव "helpers.h"
#अन्तर्भाव "timeutils.h"

/* on linux CLOCK_BOOTTIME provides proper POSIX CLOCK_MONOTONIC behavior. */
#यदि defined(__linux__)
#  undef  CLOCK_MONOTONIC
#  define CLOCK_MONOTONIC CLOCK_BOOTTIME
#यदि_अन्तः

स्थिर int64_t ts_गन्तव्यम्_ms(संरचना timespec tv)
{
	int64_t sec_गन्तव्यम्_ms = 1000, आवर्तनम्_ऊर्ध्वम् = 500000, ns_गन्तव्यम्_ms = 1000000;
	प्रत्यागम (tv.tv_sec * sec_गन्तव्यम्_ms) + ((tv.tv_nsec + आवर्तनम्_ऊर्ध्वम्) / ns_गन्तव्यम्_ms);
}

int64_t tm_now(शून्य)
{
	संरचना timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	प्रत्यागम ts_गन्तव्यम्_ms(tv);
}

int64_t tm_sleep(int64_t ms, गणना tm_sleep_flags flags)
{
	पूर्णाङ्क ret;
	संरचना timespec tv, rem;
	tv.tv_sec  = ms / 1000;
	tv.tv_nsec = (ms % 1000) * 1000000l;
	प्रति (;;) {
		ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &tv, &rem);
		यदि (ret < 0 && (!(flags & TM_NO_EINTR) || errno != EINTR))
			प्रत्यागम ts_गन्तव्यम्_ms(rem);
		यदि (ret == 0)
			भङ्ग;
		tv = rem;
	}
	प्रत्यागम 0;
}

int64_t parse_duration(ध्रुव वर्ण *duration)
{
	गणना delay_unit {
		UNIT_INVALID = -1,
		UNIT_MS   = 1,
		UNIT_SEC  = 1000,
		UNIT_MIN  = UNIT_SEC * 60,
		UNIT_HOUR = UNIT_MIN * 60,
	} unit = UNIT_INVALID;
	संरचना { ध्रुव वर्ण *ext; गणना delay_unit unit; } exts[] = {
		{ "ms", UNIT_MS }, { "miliseconds", UNIT_MS },
		{ "s", UNIT_SEC }, { "sec", UNIT_SEC }, { "seconds", UNIT_SEC }, { "", UNIT_SEC },
		{ "m", UNIT_MIN }, { "min", UNIT_MIN }, { "minutes", UNIT_MIN },
		{ "h", UNIT_HOUR }, { "hour", UNIT_HOUR }
	};
	वर्ण *अन्तः;
	दीर्घ val;

	errno = 0;
	val = strtol(duration, &अन्तः, 10);
	यदि (errno == ERANGE || *duration == '\0' || val < 0)
		प्रत्यागम -1;
	प्रति (size_t i = 0; i < ARRAY_आकार(exts); ++i) {
		यदि (वाक्यं_तुलय(अन्तः, exts[i].ext) == 0) {
			unit = exts[i].unit;
			भङ्ग;
		}
	}
	विकल्प (unit) {
	पक्ष UNIT_INVALID:
		प्रत्यागम -1;
	स्वभाव:
		यदि (INT64_MAX/unit < val) /*अतिप्रवाहः*/
			प्रत्यागम -1;
		प्रत्यागम (int64_t)val * unit;
	}
}
