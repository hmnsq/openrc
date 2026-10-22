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
#اگر_غیر_تعریف_شدہ RC_TIMEUTILS_H
#تعریف_کرو RC_TIMEUTILS_H

#شامل <stdint.h>
#شامل <time.h>

/*
 * A couple helper utilities to deal with monotonic clock at milliseconds
 * resolution.
 */

شمار tm_sleep_flags {
	TM_NO_EINTR = 1 << 0,
};

int64_t tm_now(خالی);
/* returns 0 on success, amount of time remaining on failure.
 * if TM_NO_EINTR is specified in flags, it will restart the sleep if it was
 * failed due to EINTR.
 */
int64_t tm_sleep(int64_t, شمار tm_sleep_flags);
/* parse a integer followed by an optional time unit (e.g "sec", "ms").
 * returns negative on failure
 */
int64_t parse_duration(ثابت حرف *duration);

#تعریف_کرو TM_MS(N)   ((N) * INT64_C(1))
#تعریف_کرو TM_SEC(N)  ((N) * INT64_C(1000))

#اگر_ختم
