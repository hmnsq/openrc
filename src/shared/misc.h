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

#अगर_अघोषित __RC_MISC_H__
#घोषणा __RC_MISC_H__

#समावेश <sys/stat.h>
#समावेश <sys/types.h>
#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <time.h>
#समावेश <unistd.h>

#समावेश "helpers.h"
#समावेश "rc.h"

#घोषणा RC_LEVEL_BOOT           "boot"
#घोषणा RC_LEVEL_DEFAULT        "default"

#घोषणा RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

अक्षर *rc_conf_मूल्य(स्थिर अक्षर *var);
bool rc_conf_yesno(स्थिर अक्षर *var);
अक्षर *default_runlevel(व्योम);
व्योम env_filter(व्योम);
व्योम env_config(व्योम);
पूर्णांक signal_setup(पूर्णांक sig, व्योम (*handler)(पूर्णांक));
पूर्णांक svc_lock(स्थिर अक्षर *, bool);
पूर्णांक svc_unlock(स्थिर अक्षर *, पूर्णांक);
pid_t exec_service(स्थिर अक्षर *, स्थिर अक्षर *);

प्रकार काष्ठा rc_service_state_name {
	RC_SERVICE state;
	क्रमागत rc_dir dir;
	स्थिर अक्षर *स्थिर name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
अटल स्थिर rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, शून्य}
};

#घोषणा service_start(service) exec_service(service, "start")
#घोषणा service_stop(service)  exec_service(service, "stop")

पूर्णांक parse_mode(mode_t *, अक्षर *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (पूर्णांक, पूर्णांक *);

RC_SERVICE lookup_service_state(स्थिर अक्षर *service);
व्योम from_time_t(अक्षर *time_string, समय_प्रकार tv);
समय_प्रकार to_time_t(अक्षर *timestring);
pid_t get_pid(स्थिर अक्षर *applet, स्थिर अक्षर *pidfile);

व्योम cloexec_fds_from(पूर्णांक);

काष्ठा notify {
	क्रमागत {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	अक्षर *path;
	पूर्णांक pipe[2];
	पूर्णांक fd;
};

काष्ठा notify notify_parse(स्थिर अक्षर *applet, स्थिर अक्षर *ready_string);
bool notify_wait(स्थिर अक्षर *applet, काष्ठा notify ready);

#पूर्ण_अगर
