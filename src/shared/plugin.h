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

#నిర్వచితం_కాకపోతే __LIBRC_PLUGIN_H__
#నిర్వచించు __LIBRC_PLUGIN_H__

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ "rc.h"

/* A simple flag to say if we're in a plugin process or not.
 * Mainly used in atexit code. */
బాహ్య bool rc_లోపల_plugin;

శూన్య rc_plugin_ఎక్కించు(శూన్య);
శూన్య rc_plugin_unload(శూన్య);
శూన్య rc_plugin_నిరంతరం(RC_HOOK, స్థిరరాశి అక్షరం *విలువ);

#అయితే_ముగింపు
