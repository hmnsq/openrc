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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _RC_SYSVINIT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _RC_SYSVINIT_H

ਖਾਲੀ sysvinit_runlevel(ਅੱਖਰ rl);
ਖਾਲੀ sysvinit_setenv(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ);

#ਜੇ_ਅੰਤ
