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

#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "helpers.h"
#அந்தர்பாவ "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

எழுத்து *rc_conf_மதிப்பு(மாறிலி எழுத்து *var);
bool rc_conf_yesno(மாறிலி எழுத்து *var);
எழுத்து *default_runlevel(வெறுமை);
வெறுமை env_filter(வெறுமை);
வெறுமை env_config(வெறுமை);
முழு signal_setup(முழு sig, வெறுமை (*handler)(முழு));
முழு svc_lock(மாறிலி எழுத்து *, bool);
முழு svc_unlock(மாறிலி எழுத்து *, முழு);
pid_t exec_service(மாறிலி எழுத்து *, மாறிலி எழுத்து *);

வகைவரை கட்டமைப்பு rc_service_state_name {
	RC_SERVICE state;
	எண்ணிக்கை rc_dir dir;
	மாறிலி எழுத்து *மாறிலி name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
நிலையான மாறிலி rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, வெற்று}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

முழு parse_mode(mode_t *, எழுத்து *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (முழு, முழு *);

RC_SERVICE lookup_service_state(மாறிலி எழுத்து *service);
வெறுமை from_time_t(எழுத்து *time_string, time_t tv);
time_t to_time_t(எழுத்து *timestring);
pid_t get_pid(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *pidfile);

வெறுமை cloexec_fds_from(முழு);

கட்டமைப்பு notify {
	எண்ணிக்கை {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	எழுத்து *path;
	முழு pipe[2];
	முழு fd;
};

கட்டமைப்பு notify notify_parse(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *ready_string);
bool notify_wait(மாறிலி எழுத்து *applet, கட்டமைப்பு notify ready);

#endif
