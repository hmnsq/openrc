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

#यदि_अपरिभाषितम् RC_SELINUX_UTIL_H
#परिभाषय RC_SELINUX_UTIL_H

#यदि_परिभाषितम् HAVE_SELINUX

पूर्णाङ्क selinux_util_open(शून्य);
पूर्णाङ्क selinux_util_label(ध्रुव वर्ण *मार्गः);
पूर्णाङ्क selinux_util_close(शून्य);

शून्य selinux_सज्जता(वर्ण **argv);

#अन्यथा

/* always return false for selinux_util_open() */
#परिभाषय selinux_util_open() (0)
#परिभाषय selinux_util_label(x) कुरु { } यावत् (0)
#परिभाषय selinux_util_close() कुरु { } यावत् (0)

#परिभाषय selinux_सज्जता(x) कुरु { } यावत् (0)

#यदि_अन्तः


#यदि_अन्तः
