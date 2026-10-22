/*
 * rc-wtmp.h
 * This is private to us and not for user consumption
*/

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

#నిర్వచితం_కాకపోతే __RC_WTMP_H__
#నిర్వచించు __RC_WTMP_H__

#అన్తర్భావ <utmp.h>
#అన్తర్భావ <sys/types.h>

శూన్య log_wtmp(స్థిరరాశి అక్షరం *user, స్థిరరాశి అక్షరం *id, pid_t pid, పూర్ణాంకం type,
		స్థిరరాశి అక్షరం *పంక్తి);

#అయితే_ముగింపు
