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

#ifndef RC_SELINUX_UTIL_H
#define RC_SELINUX_UTIL_H

#ifdef HAVE_SELINUX

ಪೂರ್ಣಾಙ್ಕ selinux_util_open(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ selinux_util_label(ಧ್ರುವ ವರ್ಣ *path);
ಪೂರ್ಣಾಙ್ಕ selinux_util_close(ಶೂನ್ಯ);

ಶೂನ್ಯ selinux_setup(ವರ್ಣ **argv);

#ಅನ್ಯಥಾ

/* always return false for selinux_util_open() */
#define selinux_util_open() (0)
#define selinux_util_label(x) ಕುರು { } ಯಾವತ್ (0)
#define selinux_util_close() ಕುರು { } ಯಾವತ್ (0)

#define selinux_setup(x) ಕುರು { } ಯಾವತ್ (0)

#endif


#endif
