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

#اگر_غیر_تعریف_شدہ RC_SELINUX_UTIL_H
#تعریف_کرو RC_SELINUX_UTIL_H

#اگر_تعریف_شدہ HAVE_SELINUX

صحیح selinux_util_open(خالی);
صحیح selinux_util_label(ثابت حرف *راستہ);
صحیح selinux_util_close(خالی);

خالی selinux_تیاری(حرف **argv);

#ورنہ

/* always return false for selinux_util_open() */
#تعریف_کرو selinux_util_open() (0)
#تعریف_کرو selinux_util_label(x) کرو { } جب (0)
#تعریف_کرو selinux_util_close() کرو { } جب (0)

#تعریف_کرو selinux_تیاری(x) کرو { } جب (0)

#اگر_ختم


#اگر_ختم
