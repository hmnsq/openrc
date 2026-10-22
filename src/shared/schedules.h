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

#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <sys/types.h>

શૂન્ય free_schedulelist(શૂન્ય);
પૂર્ણાંક parse_signal(અચલ અક્ષર *applet, અચલ અક્ષર *sig);
શૂન્ય parse_schedule(અચલ અક્ષર *applet, અચલ અક્ષર *string, પૂર્ણાંક timeout);
પૂર્ણાંક do_stop(અચલ અક્ષર *applet, અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv,
		pid_t pid, uid_t uid, પૂર્ણાંક sig, bool group, bool test, bool quiet);
પૂર્ણાંક run_stop_schedule(અચલ અક્ષર *applet,
		અચલ અક્ષર *exec, અચલ અક્ષર *અચલ *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
