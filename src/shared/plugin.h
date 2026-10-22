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

#ifndef __LIBRC_PLUGIN_H__
#define __LIBRC_PLUGIN_H__

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
বাহ্যিক bool rc_in_plugin;

শূন্য rc_plugin_load(শূন্য);
শূন্য rc_plugin_unload(শূন্য);
শূন্য rc_plugin_run(RC_HOOK, ধ্রুবক অক্ষর *মান);

#endif
