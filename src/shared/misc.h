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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __RC_MISC_H__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __RC_MISC_H__

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_LEVEL_BOOT           "boot"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_LEVEL_DEFAULT        "default"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

ਅੱਖਰ *rc_conf_ਮੁੱਲ(ਅਚਲ ਅੱਖਰ *var);
bool rc_conf_yesno(ਅਚਲ ਅੱਖਰ *var);
ਅੱਖਰ *default_runlevel(ਖਾਲੀ);
ਖਾਲੀ env_filter(ਖਾਲੀ);
ਖਾਲੀ env_config(ਖਾਲੀ);
ਪੂਰਨ signal_ਤਿਆਰੀ(ਪੂਰਨ sig, ਖਾਲੀ (*handler)(ਪੂਰਨ));
ਪੂਰਨ svc_ਤਾਲਾ(ਅਚਲ ਅੱਖਰ *, bool);
ਪੂਰਨ svc_unlock(ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ);
pid_t exec_service(ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ rc_service_state_name {
	RC_SERVICE state;
	ਗਿਣਤੀ rc_dir dir;
	ਅਚਲ ਅੱਖਰ *ਅਚਲ name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
ਸਥਿਰ ਅਚਲ rc_service_state_name_t rc_service_state_ਨਾਂ[] = {
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
	{ 0, -1, ਨਿਰਾ}
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ service_ਸ਼ੁਰੂ(service) exec_service(service, "start")
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ service_stop(service)  exec_service(service, "stop")

ਪੂਰਨ parse_ਬਹੁਲਕ(mode_t *, ਅੱਖਰ *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *rc_deptree_ਲੱਦੋ_2 (ਪੂਰਨ, ਪੂਰਨ *);

RC_SERVICE lookup_service_state(ਅਚਲ ਅੱਖਰ *service);
ਖਾਲੀ from_ਸਮਾਂ_t(ਅੱਖਰ *ਸਮਾਂ_string, time_t tv);
time_t ਮੰਜ਼ਿਲ_ਸਮਾਂ_t(ਅੱਖਰ *timestring);
pid_t get_pid(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *pidfile);

ਖਾਲੀ cloexec_fds_from(ਪੂਰਨ);

ਬਣਤਰ notify {
	ਗਿਣਤੀ {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	ਅੱਖਰ *ਰਸਤਾ;
	ਪੂਰਨ pipe[2];
	ਪੂਰਨ fd;
};

ਬਣਤਰ notify notify_parse(ਅਚਲ ਅੱਖਰ *applet, ਅਚਲ ਅੱਖਰ *ready_string);
bool notify_wait(ਅਚਲ ਅੱਖਰ *applet, ਬਣਤਰ notify ready);

#ਜੇ_ਅੰਤ
