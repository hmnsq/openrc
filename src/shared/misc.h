/*
 * misc.h
 * This is private to us and not for user consumption
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

#ifndef __RC_MISC_H__
#define __RC_MISC_H__

#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "helpers.h"
#अन्तर्भाव "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

वर्ण *rc_conf_मूल्य(ध्रुव वर्ण *var);
bool rc_conf_yesno(ध्रुव वर्ण *var);
वर्ण *default_runlevel(शून्य);
शून्य env_filter(शून्य);
शून्य env_config(शून्य);
पूर्णाङ्क signal_setup(पूर्णाङ्क sig, शून्य (*handler)(पूर्णाङ्क));
पूर्णाङ्क svc_lock(ध्रुव वर्ण *, bool);
पूर्णाङ्क svc_unlock(ध्रुव वर्ण *, पूर्णाङ्क);
pid_t exec_service(ध्रुव वर्ण *, ध्रुव वर्ण *);

प्रकारनिर्देश संरचना rc_service_state_name {
	RC_SERVICE state;
	गणना rc_dir dir;
	ध्रुव वर्ण *ध्रुव name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
स्थिर ध्रुव rc_service_state_name_t rc_service_state_names[] = {
	{ RC_SERVICE_STARTED,     RC_DIR_STARTED,     "started" },
	{ RC_SERVICE_STOPPED,     RC_DIR_INVALID,     "stopped" },
	{ RC_SERVICE_STARTING,    RC_DIR_STARTING,    "starting" },
	{ RC_SERVICE_STOPPING,    RC_DIR_STOPPING,    "stopping" },
	{ RC_SERVICE_INACTIVE,    RC_DIR_INACTIVE,    "inactive" },
	{ RC_SERVICE_WASINACTIVE, RC_DIR_WASINACTIVE, "wasinactive" },
	{ RC_SERVICE_HOTPLUGGED,  RC_DIR_HOTPLUGGED,  "hotplugged" },
	{ RC_SERVICE_FAILED,      RC_DIR_FAILED,      "failed" },
	{ RC_SERVICE_SCHEDULED,   RC_DIR_SCHEDULED,   "scheduled"},
	{ RC_SERVICE_CRASHED,     RC_DIR_INVALID,     "crashed"},
	{ 0, -1, रिक्त}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

पूर्णाङ्क parse_mode(mode_t *, वर्ण *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (पूर्णाङ्क, पूर्णाङ्क *);

RC_SERVICE lookup_service_state(ध्रुव वर्ण *service);
शून्य from_time_t(वर्ण *time_string, time_t tv);
time_t to_time_t(वर्ण *timestring);
pid_t get_pid(ध्रुव वर्ण *applet, ध्रुव वर्ण *pidfile);

शून्य cloexec_fds_from(पूर्णाङ्क);

संरचना notify {
	गणना {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	वर्ण *path;
	पूर्णाङ्क pipe[2];
	पूर्णाङ्क fd;
};

संरचना notify notify_parse(ध्रुव वर्ण *applet, ध्रुव वर्ण *ready_string);
bool notify_wait(ध्रुव वर्ण *applet, संरचना notify ready);

#endif
