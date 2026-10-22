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

#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "helpers.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"

#define RC_LEVEL_BOOT           "boot"
#define RC_LEVEL_DEFAULT        "default"

#define RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

ଅକ୍ଷର *rc_conf_ମୂଲ୍ୟ(ଅଚଳ ଅକ୍ଷର *var);
bool rc_conf_yesno(ଅଚଳ ଅକ୍ଷର *var);
ଅକ୍ଷର *default_runlevel(ଶୂନ୍ୟ);
ଶୂନ୍ୟ env_filter(ଶୂନ୍ୟ);
ଶୂନ୍ୟ env_config(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା signal_setup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, ଶୂନ୍ୟ (*handler)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
ପୂର୍ଣ୍ଣସଂଖ୍ୟା svc_lock(ଅଚଳ ଅକ୍ଷର *, bool);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା svc_unlock(ଅଚଳ ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
pid_t exec_service(ଅଚଳ ଅକ୍ଷର *, ଅଚଳ ଅକ୍ଷର *);

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା rc_service_state_name {
	RC_SERVICE state;
	ଗଣନା rc_dir dir;
	ଅଚଳ ଅକ୍ଷର *ଅଚଳ name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
ସ୍ଥିର ଅଚଳ rc_service_state_name_t rc_service_state_names[] = {
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
	{ 0, -1, ଖାଲି}
};

#define service_start(service) exec_service(service, "start")
#define service_stop(service)  exec_service(service, "stop")

ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_mode(mode_t *, ଅକ୍ଷର *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *_rc_deptree_load (ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);

RC_SERVICE lookup_service_state(ଅଚଳ ଅକ୍ଷର *service);
ଶୂନ୍ୟ from_time_t(ଅକ୍ଷର *time_string, time_t tv);
time_t to_time_t(ଅକ୍ଷର *timestring);
pid_t get_pid(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *pidfile);

ଶୂନ୍ୟ cloexec_fds_from(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

ସଂରଚନା notify {
	ଗଣନା {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	ଅକ୍ଷର *path;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipe[2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
};

ସଂରଚନା notify notify_parse(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *ready_string);
bool notify_wait(ଅଚଳ ଅକ୍ଷର *applet, ସଂରଚନା notify ready);

#endif
