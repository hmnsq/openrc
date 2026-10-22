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

#अगर_अघोषित __LIBRC_PLUGIN_H__
#घोषणा __LIBRC_PLUGIN_H__

#समावेश <stdbool.h>
#समावेश <sys/types.h>

#समावेश "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
बाह्य bool rc_in_plugin;

व्योम rc_plugin_load(व्योम);
व्योम rc_plugin_unload(व्योम);
व्योम rc_plugin_run(RC_HOOK, स्थिर अक्षर *मूल्य);

#पूर्ण_अगर
