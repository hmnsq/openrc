/*
 * Copyright (c) 2017 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#నిర్వచితం_కాకపోతే __RC_SCHEDULES_H
#నిర్వచించు __RC_SCHEDULES_H

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <sys/types.h>

శూన్య free_schedulelist(శూన్య);
పూర్ణాంకం parse_signal(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *sig);
శూన్య parse_కాలపట్టిక(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *string, పూర్ణాంకం timeout);
పూర్ణాంకం do_stop(స్థిరరాశి అక్షరం *applet, స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
		pid_t pid, uid_t uid, పూర్ణాంకం sig, bool group, bool test, bool quiet);
పూర్ణాంకం నిరంతరం_stop_కాలపట్టిక(స్థిరరాశి అక్షరం *applet,
		స్థిరరాశి అక్షరం *exec, స్థిరరాశి అక్షరం *స్థిరరాశి *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool పురోగతి, bool quiet);

#అయితే_ముగింపు
