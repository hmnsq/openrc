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

#నిర్వచితం_కాకపోతే __RC_MISC_H__
#నిర్వచించు __RC_MISC_H__

#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "helpers.h"
#అన్తర్భావ "rc.h"

#నిర్వచించు RC_LEVEL_BOOT           "boot"
#నిర్వచించు RC_LEVEL_DEFAULT        "default"

#నిర్వచించు RC_KRUNLEVEL            RC_SVCDIR "/krunlevel"

అక్షరం *rc_conf_విలువ(స్థిరరాశి అక్షరం *var);
bool rc_conf_yesno(స్థిరరాశి అక్షరం *var);
అక్షరం *default_runlevel(శూన్య);
శూన్య env_filter(శూన్య);
శూన్య env_config(శూన్య);
పూర్ణాంకం signal_అమరిక(పూర్ణాంకం sig, శూన్య (*handler)(పూర్ణాంకం));
పూర్ణాంకం svc_తాళం(స్థిరరాశి అక్షరం *, bool);
పూర్ణాంకం svc_unlock(స్థిరరాశి అక్షరం *, పూర్ణాంకం);
pid_t exec_service(స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *);

రకనిర్వచనం నిర్మాణం rc_service_state_name {
	RC_SERVICE state;
	గణన rc_dir dir;
	స్థిరరాశి అక్షరం *స్థిరరాశి name;
} rc_service_state_name_t;

/* We MUST list the states below 0x10 first
 * The rest can be in any order */
స్థిర స్థిరరాశి rc_service_state_name_t rc_service_state_పేర్లు[] = {
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
	{ 0, -1, ఖాళీ}
};

#నిర్వచించు service_ప్రారంభం(service) exec_service(service, "start")
#నిర్వచించు service_stop(service)  exec_service(service, "stop")

పూర్ణాంకం parse_బహుళకం(mode_t *, అక్షరం *);

/* Handy function so we can wrap einfo around our deptree */
RC_DEPTREE *rc_deptree_ఎక్కించు_2 (పూర్ణాంకం, పూర్ణాంకం *);

RC_SERVICE lookup_service_state(స్థిరరాశి అక్షరం *service);
శూన్య from_సమయం_t(అక్షరం *సమయం_string, time_t tv);
time_t గమ్యం_సమయం_t(అక్షరం *timestring);
pid_t get_pid(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *pidfile);

శూన్య cloexec_fds_from(పూర్ణాంకం);

నిర్మాణం notify {
	గణన {
		NOTIFY_NONE = 0,
		NOTIFY_FD,
		NOTIFY_SOCKET
	} type;

	అక్షరం *మార్గం;
	పూర్ణాంకం pipe[2];
	పూర్ణాంకం fd;
};

నిర్మాణం notify notify_parse(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *ready_string);
bool notify_wait(స్థిరరాశి అక్షరం *applet, నిర్మాణం notify ready);

#అయితే_ముగింపు
