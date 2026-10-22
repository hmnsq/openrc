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

#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "helpers.h"
#અન્તર્ભાવ "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

અક્ષર *rc_conf_કિંમત(અચલ અક્ષર *var);
bool rc_conf_yesno(અચલ અક્ષર *var);
અક્ષર *default_runlevel(શૂન્ય);
શૂન્ય env_filter(શૂન્ય);
શૂન્ય env_config(શૂન્ય);
પૂર્ણાંક signal_setup(પૂર્ણાંક sig, શૂન્ય (*handler)(પૂર્ણાંક));
પૂર્ણાંક svc_lock(અચલ અક્ષર *, bool);
પૂર્ણાંક svc_unlock(અચલ અક્ષર *, પૂર્ણાંક);
pid_t exec_service(અચલ અક્ષર *, અચલ અક્ષર *);

પ્રકારવ્યાખ્યા સંરચના rc_service_state_name {
	RC_SERVICE state;
	ગણના rc_dir dir;
	અચલ અક્ષર *અચલ name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
સ્થિર અચલ rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, ખાલી}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

પૂર્ણાંક parse_mode(mode_t *, અક્ષર *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (પૂર્ણાંક, પૂર્ણાંક *);

RC_SERVICE lookup_service_state(અચલ અક્ષર *service);
શૂન્ય from_time_t(અક્ષર *time_string, time_t tv);
time_t to_time_t(અક્ષર *timestring);
pid_t get_pid(અચલ અક્ષર *applet, અચલ અક્ષર *pidfile);

શૂન્ય cloexec_fds_from(પૂર્ણાંક);

સંરચના notify {
	ગણના {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	અક્ષર *path;
	પૂર્ણાંક pipe[2];
	પૂર્ણાંક fd;
};

સંરચના notify notify_parse(અચલ અક્ષર *applet, અચલ અક્ષર *ready_string);
bool notify_wait(અચલ અક્ષર *applet, સંરચના notify ready);

#endif
