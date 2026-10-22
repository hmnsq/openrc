/*
 * rc-wtmp.h
 * This is private to us and not for user consumption
*/

/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ifndef __RC_WTMP_H__
#define __RC_WTMP_H__

#ಅನ್ತರ್ಭಾವ <utmp.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

ಶೂನ್ಯ log_wtmp(ಧ್ರುವ ವರ್ಣ *user, ಧ್ರುವ ವರ್ಣ *id, pid_t pid, ಪೂರ್ಣಾಙ್ಕ type,
		ಧ್ರುವ ವರ್ಣ *ಸಾಲು);

#endif
