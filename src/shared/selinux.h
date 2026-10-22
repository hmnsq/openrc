/*
 * Copyright (c) 2014-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ RC_SELINUX_UTIL_H
#ವ್ಯಾಖ್ಯಾನಿಸು RC_SELINUX_UTIL_H

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_SELINUX

ಪೂರ್ಣಾಂಕ selinux_util_open(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ selinux_util_label(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ);
ಪೂರ್ಣಾಂಕ selinux_util_close(ಶೂನ್ಯ);

ಶೂನ್ಯ selinux_ಸಿದ್ಧತೆ(ಅಕ್ಷರ **argv);

#ಇಲ್ಲದಿದ್ದರೆ

/* always return false for selinux_util_open() */
#ವ್ಯಾಖ್ಯಾನಿಸು selinux_util_open() (0)
#ವ್ಯಾಖ್ಯಾನಿಸು selinux_util_label(x) ಮಾಡು { } ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು selinux_util_close() ಮಾಡು { } ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು selinux_ಸಿದ್ಧತೆ(x) ಮಾಡು { } ಆಗ (0)

#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ_ಅಂತ್ಯ
