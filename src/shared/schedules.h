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

#ifndef __RC_SCHEDULES_H
#define __RC_SCHEDULES_H

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <sys/types.h>

शून्य free_schedulelist(शून्य);
पूर्णाङ्क parse_signal(अचर वर्ण *applet, अचर वर्ण *sig);
शून्य parse_schedule(अचर वर्ण *applet, अचर वर्ण *string, पूर्णाङ्क timeout);
पूर्णाङ्क do_stop(अचर वर्ण *applet, अचर वर्ण *exec, अचर वर्ण *अचर *argv,
		pid_t pid, uid_t uid, पूर्णाङ्क sig, bool group, bool test, bool quiet);
पूर्णाङ्क run_stop_schedule(अचर वर्ण *applet,
		अचर वर्ण *exec, अचर वर्ण *अचर *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
