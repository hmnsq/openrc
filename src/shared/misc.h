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

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "helpers.h"
#include "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

var_na *rc_conf_value(const var_na *var);
bool rc_conf_yesno(const var_na *var);
var_na *default_runlevel(void);
void env_filter(void);
void env_config(void);
จำนวนเต็ม signal_setup(จำนวนเต็ม sig, void (*handler)(จำนวนเต็ม));
จำนวนเต็ม svc_lock(const var_na *, bool);
จำนวนเต็ม svc_unlock(const var_na *, จำนวนเต็ม);
pid_t exec_service(const var_na *, const var_na *);

typedef sa_nracanaa rc_service_state_name {
	RC_SERVICE state;
	ga_nanaa rc_dir dir;
	const var_na *const name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
static const rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, NULL}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

จำนวนเต็ม parse_mode(mode_t *, var_na *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (จำนวนเต็ม, จำนวนเต็ม *);

RC_SERVICE lookup_service_state(const var_na *service);
void from_time_t(var_na *time_string, time_t tv);
time_t to_time_t(var_na *timestring);
pid_t get_pid(const var_na *applet, const var_na *pidfile);

void cloexec_fds_from(จำนวนเต็ม);

sa_nracanaa notify {
	ga_nanaa {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	var_na *path;
	จำนวนเต็ม pipe[2];
	จำนวนเต็ม fd;
};

sa_nracanaa notify notify_parse(const var_na *applet, const var_na *ready_string);
bool notify_wait(const var_na *applet, sa_nracanaa notify ready);

#endif
