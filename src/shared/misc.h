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

#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "helpers.h"
#অন্তর্ভাব "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

বর্ণ *rc_conf_মান(ধ্রুব বর্ণ *var);
bool rc_conf_yesno(ধ্রুব বর্ণ *var);
বর্ণ *default_runlevel(শূন্য);
শূন্য env_filter(শূন্য);
শূন্য env_config(শূন্য);
পূর্ণাঙ্ক signal_setup(পূর্ণাঙ্ক sig, শূন্য (*handler)(পূর্ণাঙ্ক));
পূর্ণাঙ্ক svc_lock(ধ্রুব বর্ণ *, bool);
পূর্ণাঙ্ক svc_unlock(ধ্রুব বর্ণ *, পূর্ণাঙ্ক);
pid_t exec_service(ধ্রুব বর্ণ *, ধ্রুব বর্ণ *);

প্রকারনির্দেশ সংরচনা rc_service_state_name {
	RC_SERVICE state;
	গণনা rc_dir dir;
	ধ্রুব বর্ণ *ধ্রুব name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
স্থির ধ্রুব rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, রিক্ত}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

পূর্ণাঙ্ক parse_mode(mode_t *, বর্ণ *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (পূর্ণাঙ্ক, পূর্ণাঙ্ক *);

RC_SERVICE lookup_service_state(ধ্রুব বর্ণ *service);
শূন্য from_time_t(বর্ণ *time_string, time_t tv);
time_t to_time_t(বর্ণ *timestring);
pid_t get_pid(ধ্রুব বর্ণ *applet, ধ্রুব বর্ণ *pidfile);

শূন্য cloexec_fds_from(পূর্ণাঙ্ক);

সংরচনা notify {
	গণনা {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	বর্ণ *path;
	পূর্ণাঙ্ক pipe[2];
	পূর্ণাঙ্ক fd;
};

সংরচনা notify notify_parse(ধ্রুব বর্ণ *applet, ধ্রুব বর্ণ *ready_string);
bool notify_wait(ধ্রুব বর্ণ *applet, সংরচনা notify ready);

#endif
