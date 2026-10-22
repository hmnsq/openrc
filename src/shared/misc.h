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

अक्षर *rc_conf_मूल्य(स्थिरांक अक्षर *var);
bool rc_conf_yesno(स्थिरांक अक्षर *var);
अक्षर *default_runlevel(शून्य);
शून्य env_filter(शून्य);
शून्य env_config(शून्य);
पूर्णांक signal_setup(पूर्णांक sig, शून्य (*handler)(पूर्णांक));
पूर्णांक svc_lock(स्थिरांक अक्षर *, bool);
पूर्णांक svc_unlock(स्थिरांक अक्षर *, पूर्णांक);
pid_t exec_service(स्थिरांक अक्षर *, स्थिरांक अक्षर *);

प्रकारव्याख्या संरचना rc_service_state_name {
	RC_SERVICE state;
	गणना rc_dir dir;
	स्थिरांक अक्षर *स्थिरांक name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
स्थिर स्थिरांक rc_service_state_name_t rc_service_state_names[] = {
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

पूर्णांक parse_mode(mode_t *, अक्षर *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (पूर्णांक, पूर्णांक *);

RC_SERVICE lookup_service_state(स्थिरांक अक्षर *service);
शून्य from_time_t(अक्षर *time_string, समय_प्रकार tv);
समय_प्रकार to_time_t(अक्षर *timestring);
pid_t get_pid(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *pidfile);

शून्य cloexec_fds_from(पूर्णांक);

संरचना notify {
	गणना {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	अक्षर *path;
	पूर्णांक pipe[2];
	पूर्णांक fd;
};

संरचना notify notify_parse(स्थिरांक अक्षर *applet, स्थिरांक अक्षर *ready_string);
bool notify_wait(स्थिरांक अक्षर *applet, संरचना notify ready);

#endif
