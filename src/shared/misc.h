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

#اگر_غیر_تعریف_شدہ __RC_MISC_H__
#تعریف_کرو __RC_MISC_H__

#شامل <sys/stat.h>
#شامل <sys/types.h>
#شامل <errno.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <time.h>
#شامل <unistd.h>

#شامل "helpers.h"
#شامل "rc.h"

#تعریف_کرو RC_LEVEL_BOOT           "boot"
#تعریف_کرو RC_LEVEL_DEFAULT        "default"

#تعریف_کرو RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

حرف *rc_conf_قدر(ثابت حرف *var);
bool rc_conf_yesno(ثابت حرف *var);
حرف *default_runlevel(خالی);
خالی env_filter(خالی);
خالی env_config(خالی);
صحیح signal_تیاری(صحیح sig, خالی (*handler)(صحیح));
صحیح svc_تالا(ثابت حرف *, bool);
صحیح svc_unlock(ثابت حرف *, صحیح);
pid_t exec_service(ثابت حرف *, ثابت حرف *);

قسمتعریف ساخت rc_service_state_name {
	RC_SERVICE state;
	شمار rc_dir dir;
	ثابت حرف *ثابت name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
ساکن ثابت rc_service_state_name_t rc_service_state_نام[] = {
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
	{ 0, -1, تہی}
};

#تعریف_کرو service_آغاز(service) exec_service(service, "start")
#تعریف_کرو service_stop(service)  exec_service(service, "stop")

صحیح parse_منوال(mode_t *, حرف *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *rc_deptree_بھرو_2 (صحیح, صحیح *);

RC_SERVICE lookup_service_state(ثابت حرف *service);
خالی from_وقت_t(حرف *وقت_string, time_t tv);
time_t مقصد_وقت_t(حرف *timestring);
pid_t get_pid(ثابت حرف *applet, ثابت حرف *pidfile);

خالی cloexec_fds_from(صحیح);

ساخت notify {
	شمار {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	حرف *راستہ;
	صحیح pipe[2];
	صحیح fd;
};

ساخت notify notify_parse(ثابت حرف *applet, ثابت حرف *ready_string);
bool notify_wait(ثابت حرف *applet, ساخت notify ready);

#اگر_ختم
