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

#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "helpers.h"
#അന്തര്ഭാവ "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

അക്ഷരം *rc_conf_മൂല്യം(സ്ഥിരം അക്ഷരം *var);
bool rc_conf_yesno(സ്ഥിരം അക്ഷരം *var);
അക്ഷരം *default_runlevel(ശൂന്യം);
ശൂന്യം env_filter(ശൂന്യം);
ശൂന്യം env_config(ശൂന്യം);
പൂർണ്ണസംഖ്യ signal_setup(പൂർണ്ണസംഖ്യ sig, ശൂന്യം (*handler)(പൂർണ്ണസംഖ്യ));
പൂർണ്ണസംഖ്യ svc_lock(സ്ഥിരം അക്ഷരം *, bool);
പൂർണ്ണസംഖ്യ svc_unlock(സ്ഥിരം അക്ഷരം *, പൂർണ്ണസംഖ്യ);
pid_t exec_service(സ്ഥിരം അക്ഷരം *, സ്ഥിരം അക്ഷരം *);

തരനിർവചനം ഘടന rc_service_state_name {
	RC_SERVICE state;
	എണ്ണൽ rc_dir dir;
	സ്ഥിരം അക്ഷരം *സ്ഥിരം name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
സ്ഥിതം സ്ഥിരം rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, ശൂന്യ}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

പൂർണ്ണസംഖ്യ parse_mode(mode_t *, അക്ഷരം *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ *);

RC_SERVICE lookup_service_state(സ്ഥിരം അക്ഷരം *service);
ശൂന്യം from_time_t(അക്ഷരം *time_string, time_t tv);
time_t to_time_t(അക്ഷരം *timestring);
pid_t get_pid(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *pidfile);

ശൂന്യം cloexec_fds_from(പൂർണ്ണസംഖ്യ);

ഘടന notify {
	എണ്ണൽ {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	അക്ഷരം *path;
	പൂർണ്ണസംഖ്യ pipe[2];
	പൂർണ്ണസംഖ്യ fd;
};

ഘടന notify notify_parse(സ്ഥിരം അക്ഷരം *applet, സ്ഥിരം അക്ഷരം *ready_string);
bool notify_wait(സ്ഥിരം അക്ഷരം *applet, ഘടന notify ready);

#endif
