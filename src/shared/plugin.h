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

#اگر_غیر_تعریف_شدہ __LIBRC_PLUGIN_H__
#تعریف_کرو __LIBRC_PLUGIN_H__

#شامل <stdbool.h>
#شامل <sys/types.h>

#شامل "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
خارجی bool rc_اندر_plugin;

خالی rc_plugin_بھرو(خالی);
خالی rc_plugin_unload(خالی);
خالی rc_plugin_تسلسل(RC_HOOK, ثابت حرف *قدر);

#اگر_ختم
