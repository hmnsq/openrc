/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#यदि_अपरिभाषितम् __RC_SCHEDULES_H
#परिभाषय __RC_SCHEDULES_H

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <sys/types.h>

शून्य free_schedulelist(शून्य);
पूर्णाङ्क parse_signal(ध्रुव वर्ण *applet, ध्रुव वर्ण *sig);
शून्य parse_कालसारणी(ध्रुव वर्ण *applet, ध्रुव वर्ण *string, पूर्णाङ्क timeout);
पूर्णाङ्क do_stop(ध्रुव वर्ण *applet, ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv,
		pid_t pid, uid_t uid, पूर्णाङ्क sig, bool group, bool test, bool quiet);
पूर्णाङ्क सततम्_stop_कालसारणी(ध्रुव वर्ण *applet,
		ध्रुव वर्ण *exec, ध्रुव वर्ण *ध्रुव *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool प्रगतिः, bool quiet);

#यदि_अन्तः
