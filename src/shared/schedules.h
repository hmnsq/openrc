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

#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <sys/types.h>

শূন্য free_schedulelist(শূন্য);
পূর্ণাঙ্ক parse_signal(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *sig);
শূন্য parse_schedule(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *string, পূর্ণাঙ্ক timeout);
পূর্ণাঙ্ক do_stop(ধ্রুৱ ৱর্ণ *applet, ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
		pid_t pid, uid_t uid, পূর্ণাঙ্ক sig, bool group, bool test, bool quiet);
পূর্ণাঙ্ক run_stop_schedule(ধ্রুৱ ৱর্ণ *applet,
		ধ্রুৱ ৱর্ণ *exec, ধ্রুৱ ৱর্ণ *ধ্রুৱ *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
