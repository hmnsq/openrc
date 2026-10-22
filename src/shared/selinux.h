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

പൂർണ്ണസംഖ്യ selinux_util_open(ശൂന്യം);
പൂർണ്ണസംഖ്യ selinux_util_label(സ്ഥിരം അക്ഷരം *path);
പൂർണ്ണസംഖ്യ selinux_util_close(ശൂന്യം);

ശൂന്യം selinux_setup(അക്ഷരം **argv);

#അല്ലെങ്കിൽ

/* always return false for selinux_util_open() */
#define selinux_util_open() (0)
#define selinux_util_label(x) ചെയ്യുക { } ആകുമ്പോൾ (0)
#define selinux_util_close() ചെയ്യുക { } ആകുമ്പോൾ (0)

#define selinux_setup(x) ചെയ്യുക { } ആകുമ്പോൾ (0)

#endif


#endif
