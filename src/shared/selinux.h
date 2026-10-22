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

#اگر_تعریف_نشده RC_SELINUX_UTIL_H
#تعریف_کن RC_SELINUX_UTIL_H

#اگر_تعریف_شده HAVE_SELINUX

صحيح selinux_util_open(فراغ);
صحيح selinux_util_label(ثابت حرف *مسیر);
صحيح selinux_util_close(فراغ);

فراغ selinux_راهاندازی(حرف **argv);

#وإلا

/* always return false for selinux_util_open() */
#تعریف_کن selinux_util_open() (0)
#تعریف_کن selinux_util_label(x) افعل { } طالما (0)
#تعریف_کن selinux_util_close() افعل { } طالما (0)

#تعریف_کن selinux_راهاندازی(x) افعل { } طالما (0)

#پایان_اگر


#پایان_اگر
