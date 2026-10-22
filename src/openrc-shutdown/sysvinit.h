/*
 * sysvinit.h	- Interface to communicate with sysvinit via /run/initctl.
 */

/*
 * Copyright (c) 2019 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#که_نه_تعریف_شوی _RC_SYSVINIT_H
#تعریف_کړه _RC_SYSVINIT_H

فراغ sysvinit_runlevel(حرف rl);
فراغ sysvinit_setenv(ثابت حرف *name, ثابت حرف *ارزښت);

#د_که_پای
