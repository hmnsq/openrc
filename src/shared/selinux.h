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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_util_open(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_util_label(ଅଚଳ ଅକ୍ଷର *path);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_util_close(ଶୂନ୍ୟ);

ଶୂନ୍ୟ selinux_setup(ଅକ୍ଷର **argv);

#ଅନ୍ୟଥା

/* always return false for selinux_util_open() */
#define selinux_util_open() (0)
#define selinux_util_label(x) କର { } ଯେତେବେଳେ (0)
#define selinux_util_close() କର { } ଯେତେବେଳେ (0)

#define selinux_setup(x) କର { } ଯେତେବେଳେ (0)

#endif


#endif
