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

#ifndef __RC_SCHEDULES_H
#define __RC_SCHEDULES_H

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

ಶೂನ್ಯ free_schedulelist(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ parse_signal(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *sig);
ಶೂನ್ಯ parse_schedule(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *string, ಪೂರ್ಣಾಙ್ಕ timeout);
ಪೂರ್ಣಾಙ್ಕ do_stop(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
		pid_t pid, uid_t uid, ಪೂರ್ಣಾಙ್ಕ sig, bool group, bool test, bool quiet);
ಪೂರ್ಣಾಙ್ಕ run_stop_schedule(ಧ್ರುವ ವರ್ಣ *applet,
		ಧ್ರುವ ವರ್ಣ *exec, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
