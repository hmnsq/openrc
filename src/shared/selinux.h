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

#جيڪڏهن_وصف_نه_ٿيل RC_SELINUX_UTIL_H
#وصف_ڪريو RC_SELINUX_UTIL_H

#جيڪڏهن_وصف_ٿيل HAVE_SELINUX

صحيح selinux_util_open(فراغ);
صحيح selinux_util_label(ثابت حرف *رستو);
صحيح selinux_util_close(فراغ);

فراغ selinux_تياري(حرف **argv);

#وإلا

/* always return false for selinux_util_open() */
#وصف_ڪريو selinux_util_open() (0)
#وصف_ڪريو selinux_util_label(x) افعل { } طالما (0)
#وصف_ڪريو selinux_util_close() افعل { } طالما (0)

#وصف_ڪريو selinux_تياري(x) افعل { } طالما (0)

#جيڪڏهن_پورو


#جيڪڏهن_پورو
