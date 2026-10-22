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

#ifndef __RC_SCHEDULES_H
#define __RC_SCHEDULES_H

#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

ଶୂନ୍ୟ free_schedulelist(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_signal(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *sig);
ଶୂନ୍ୟ parse_schedule(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *string, ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_stop(ଅଚଳ ଅକ୍ଷର *applet, ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
		pid_t pid, uid_t uid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, bool group, bool test, bool quiet);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_stop_schedule(ଅଚଳ ଅକ୍ଷର *applet,
		ଅଚଳ ଅକ୍ଷର *exec, ଅଚଳ ଅକ୍ଷର *ଅଚଳ *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
