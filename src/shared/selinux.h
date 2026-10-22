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

पूर्णांक selinux_util_open(शून्य);
पूर्णांक selinux_util_label(स्थिरांक अक्षर *path);
पूर्णांक selinux_util_close(शून्य);

शून्य selinux_setup(अक्षर **argv);

#अन्यथा

/* always return false for selinux_util_open() */
#define selinux_util_open() (0)
#define selinux_util_label(x) करा { } पर्यंत (0)
#define selinux_util_close() करा { } पर्यंत (0)

#define selinux_setup(x) करा { } पर्यंत (0)

#endif


#endif
