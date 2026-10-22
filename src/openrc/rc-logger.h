/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ RC_LOGGER_H
#ವ್ಯಾಖ್ಯಾನಿಸು RC_LOGGER_H

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

ಬಾಹ್ಯ pid_t rc_logger_pid;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ rc_logger_tty;
ಬಾಹ್ಯ bool rc_ಒಳ_logger;

ಶೂನ್ಯ rc_logger_open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel);
ಶೂನ್ಯ rc_logger_close(ಶೂನ್ಯ);

#ಆದರೆ_ಅಂತ್ಯ
