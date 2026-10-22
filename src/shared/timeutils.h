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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ RC_TIMEUTILS_H
#ವ್ಯಾಖ್ಯಾನಿಸು RC_TIMEUTILS_H

#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <time.h>

/*
 * A couple helper utilities to deal with monotonic clock at milliseconds
 * resolution.
 */

ಎಣಿಕೆ tm_sleep_flags {
	TM_NO_EINTR = 1 << 0,
};

int64_t tm_now(ಶೂನ್ಯ);
/* returns 0 on success, amount of time remaining on failure.
 * if TM_NO_EINTR is specified in flags, it will restart the sleep if it was
 * failed due to EINTR.
 */
int64_t tm_sleep(int64_t, ಎಣಿಕೆ tm_sleep_flags);
/* parse a integer followed by an optional time unit (e.g "sec", "ms").
 * returns negative on failure
 */
int64_t parse_duration(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *duration);

#ವ್ಯಾಖ್ಯಾನಿಸು TM_MS(N)   ((N) * INT64_C(1))
#ವ್ಯಾಖ್ಯಾನಿಸು TM_SEC(N)  ((N) * INT64_C(1000))

#ಆದರೆ_ಅಂತ್ಯ
