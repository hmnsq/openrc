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

পূর্ণসংখ্যা selinux_util_open(শূন্য);
পূর্ণসংখ্যা selinux_util_label(ধ্রুবক অক্ষর *path);
পূর্ণসংখ্যা selinux_util_close(শূন্য);

শূন্য selinux_setup(অক্ষর **argv);

#অন্যথা

/* always return false for selinux_util_open() */
#define selinux_util_open() (0)
#define selinux_util_label(x) করো { } যখন (0)
#define selinux_util_close() করো { } যখন (0)

#define selinux_setup(x) করো { } যখন (0)

#endif


#endif
