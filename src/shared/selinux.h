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

#अगर_अघोषित RC_SELINUX_UTIL_H
#घोषणा RC_SELINUX_UTIL_H

#अगर_घोषित HAVE_SELINUX

पूर्णांक selinux_util_open(व्योम);
पूर्णांक selinux_util_label(स्थिर अक्षर *path);
पूर्णांक selinux_util_close(व्योम);

व्योम selinux_setup(अक्षर **argv);

#अन्यथा

/* always return false for selinux_util_open() */
#घोषणा selinux_util_open() (0)
#घोषणा selinux_util_label(x) करो { } जबतक (0)
#घोषणा selinux_util_close() करो { } जबतक (0)

#घोषणा selinux_setup(x) करो { } जबतक (0)

#पूर्ण_अगर


#पूर्ण_अगर
