/*
 * librc.h
 * Internal header file to setup build env for files in librc.so
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

#ifndef _LIBRC_H_
#define _LIBRC_H_

#define _IN_LIBRC

#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/param.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/time.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <dirent.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <libgen.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <paths.h>
#অন্তর্ভাব <regex.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <strings.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>

#যদি defined(BSD) && !defined(__GNU__)
  #যদি !defined(__NetBSD__)
  #অন্তর্ভাব <sys/user.h>
  #endif
#অন্তর্ভাব <sys/param.h>
#অন্তর্ভাব <sys/sysctl.h>
#অন্তর্ভাব <kvm.h>
#অন্যথা
#অন্তর্ভাব <sys/param.h>
#endif

#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"

স্থির ধ্রুবক অক্ষর *ধ্রুবক dirnames[RC_DIR_SYS_MAX] =
{
	[RC_DIR_STARTING] = "starting",
	[RC_DIR_STARTED] = "started",
	[RC_DIR_STOPPING] = "stopping",
	[RC_DIR_INACTIVE] = "inactive",
	[RC_DIR_WASINACTIVE] = "wasinactive",
	[RC_DIR_FAILED] = "failed",
	[RC_DIR_HOTPLUGGED] = "hotplugged",
	[RC_DIR_DAEMONS] = "daemons",
	[RC_DIR_OPTIONS] = "options",
	[RC_DIR_EXCLUSIVE] = "exclusive",
	[RC_DIR_SCHEDULED] = "scheduled",
	[RC_DIR_INITD] = "init.d",
	[RC_DIR_TMP] = "tmp",
	[RC_DIR_ENVIRON] = "environ",
};

RC_STRINGLIST *config_list(পূর্ণসংখ্যা dirfd, ধ্রুবক অক্ষর *pathname);
শূন্য clear_dirfds(শূন্য);

#endif
