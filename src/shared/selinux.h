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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RC_SELINUX_UTIL_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_SELINUX_UTIL_H

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SELINUX

ਪੂਰਨ selinux_util_open(ਖਾਲੀ);
ਪੂਰਨ selinux_util_label(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ);
ਪੂਰਨ selinux_util_close(ਖਾਲੀ);

ਖਾਲੀ selinux_ਤਿਆਰੀ(ਅੱਖਰ **argv);

#ਨਹੀਂਤਾਂ

/* always return false for selinux_util_open() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ selinux_util_open() (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ selinux_util_label(x) ਕਰੋ { } ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ selinux_util_close() ਕਰੋ { } ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ selinux_ਤਿਆਰੀ(x) ਕਰੋ { } ਜਦੋਂ (0)

#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ
