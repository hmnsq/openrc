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

#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
ബാഹ്യ bool rc_in_plugin;

ശൂന്യം rc_plugin_load(ശൂന്യം);
ശൂന്യം rc_plugin_unload(ശൂന്യം);
ശൂന്യം rc_plugin_run(RC_HOOK, സ്ഥിരം അക്ഷരം *മൂല്യം);

#endif
