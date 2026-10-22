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

#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "helpers.h"
#ಅನ್ತರ್ಭಾವ "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

ವರ್ಣ *rc_conf_ಮೌಲ್ಯ(ಧ್ರುವ ವರ್ಣ *var);
bool rc_conf_yesno(ಧ್ರುವ ವರ್ಣ *var);
ವರ್ಣ *default_runlevel(ಶೂನ್ಯ);
ಶೂನ್ಯ env_filter(ಶೂನ್ಯ);
ಶೂನ್ಯ env_config(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ signal_setup(ಪೂರ್ಣಾಙ್ಕ sig, ಶೂನ್ಯ (*handler)(ಪೂರ್ಣಾಙ್ಕ));
ಪೂರ್ಣಾಙ್ಕ svc_lock(ಧ್ರುವ ವರ್ಣ *, bool);
ಪೂರ್ಣಾಙ್ಕ svc_unlock(ಧ್ರುವ ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ);
pid_t exec_service(ಧ್ರುವ ವರ್ಣ *, ಧ್ರುವ ವರ್ಣ *);

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ rc_service_state_name {
	RC_SERVICE state;
	ಗಣನಾ rc_dir dir;
	ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
ಸ್ಥಿರ ಧ್ರುವ rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, ರಿಕ್ತ}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

ಪೂರ್ಣಾಙ್ಕ parse_mode(mode_t *, ವರ್ಣ *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ *);

RC_SERVICE lookup_service_state(ಧ್ರುವ ವರ್ಣ *service);
ಶೂನ್ಯ from_time_t(ವರ್ಣ *time_string, time_t tv);
time_t to_time_t(ವರ್ಣ *timestring);
pid_t get_pid(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *pidfile);

ಶೂನ್ಯ cloexec_fds_from(ಪೂರ್ಣಾಙ್ಕ);

ಸಂರಚನಾ notify {
	ಗಣನಾ {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	ವರ್ಣ *path;
	ಪೂರ್ಣಾಙ್ಕ pipe[2];
	ಪೂರ್ಣಾಙ್ಕ fd;
};

ಸಂರಚನಾ notify notify_parse(ಧ್ರುವ ವರ್ಣ *applet, ಧ್ರುವ ವರ್ಣ *ready_string);
bool notify_wait(ಧ್ರುವ ವರ್ಣ *applet, ಸಂರಚನಾ notify ready);

#endif
