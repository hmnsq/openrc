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

#నిర్వచితం_కాకపోతే RC_LOGGER_H
#నిర్వచించు RC_LOGGER_H

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <sys/types.h>

బాహ్య pid_t rc_logger_pid;
బాహ్య పూర్ణాంకం rc_logger_tty;
బాహ్య bool rc_లోపల_logger;

శూన్య rc_logger_open(స్థిరరాశి అక్షరం *runlevel);
శూన్య rc_logger_close(శూన్య);

#అయితే_ముగింపు
