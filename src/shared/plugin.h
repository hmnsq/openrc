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

#جے_بنا_تعریف __LIBRC_PLUGIN_H__
#تعریف_کرو __LIBRC_PLUGIN_H__

#ضمن <stdbool.h>
#ضمن <sys/types.h>

#ضمن "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
خارجي bool rc_وچ_plugin;

فراغ rc_plugin_لدو(فراغ);
فراغ rc_plugin_unload(فراغ);
فراغ rc_plugin_لگاتار(RC_HOOK, ثابت حرف *قدر);

#جے_انت
