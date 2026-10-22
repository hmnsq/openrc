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

#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <sys/types.h>

শূন্য free_schedulelist(শূন্য);
পূর্ণসংখ্যা parse_signal(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *sig);
শূন্য parse_schedule(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *string, পূর্ণসংখ্যা timeout);
পূর্ণসংখ্যা do_stop(ধ্রুবক অক্ষর *applet, ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv,
		pid_t pid, uid_t uid, পূর্ণসংখ্যা sig, bool group, bool test, bool quiet);
পূর্ণসংখ্যা run_stop_schedule(ধ্রুবক অক্ষর *applet,
		ধ্রুবক অক্ষর *exec, ধ্রুবক অক্ষর *ধ্রুবক *argv,
		pid_t pid, uid_t uid, bool group,
		bool test, bool progress, bool quiet);

#endif
