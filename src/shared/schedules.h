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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <sys/types.h>

வெறுமை free_schedulelist(வெறுமை);
முழு parse_signal(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *sig);
வெறுமை parse_schedule(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *string, முழு timeout);
முழு do_stop(மாறிலி எழுத்து *applet, மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv,
		pid_t pid, uid_t uid, முழு sig, bool group, bool test, bool quiet);
முழு run_stop_schedule(மாறிலி எழுத்து *applet,
		மாறிலி எழுத்து *exec, மாறிலி எழுத்து *மாறிலி *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
