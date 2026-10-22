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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __RC_SCHEDULES_H
#ವ್ಯಾಖ್ಯಾನಿಸು __RC_SCHEDULES_H

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

ಶೂನ್ಯ free_schedulelist(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ parse_signal(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sig);
ಶೂನ್ಯ parse_ವೇಳಾಪಟ್ಟಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *string, ಪೂರ್ಣಾಂಕ timeout);
ಪೂರ್ಣಾಂಕ do_stop(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
		pid_t pid, uid_t uid, ಪೂರ್ಣಾಂಕ sig, bool group, bool test, bool quiet);
ಪೂರ್ಣಾಂಕ ಸತತ_stop_ವೇಳಾಪಟ್ಟಿ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *exec, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool ಪ್ರಗತಿ, bool quiet);

#ಆದರೆ_ಅಂತ್ಯ
