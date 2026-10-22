/*
 * plugin.h
 * Private instructions to use plugins
 */

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LIBRC_PLUGIN_H__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __LIBRC_PLUGIN_H__

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
ਬਾਹਰੀ bool rc_ਵਿੱਚ_plugin;

ਖਾਲੀ rc_plugin_ਲੱਦੋ(ਖਾਲੀ);
ਖਾਲੀ rc_plugin_unload(ਖਾਲੀ);
ਖਾਲੀ rc_plugin_ਲੜੀਵਾਰ(RC_HOOK, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ);

#ਜੇ_ਅੰਤ
