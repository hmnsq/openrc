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

#شامل <sys/types.h>
#شامل <sys/param.h>
#شامل <sys/stat.h>
#شامل <sys/time.h>
#شامل <sys/wait.h>
#شامل <ctype.h>
#شامل <dirent.h>
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <libgen.h>
#شامل <limits.h>
#شامل <paths.h>
#شامل <regex.h>
#شامل <signal.h>
#شامل <stdarg.h>
#شامل <stdbool.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <strings.h>
#شامل <time.h>
#شامل <unistd.h>

#اگر defined(BSD) && !defined(__GNU__)
  #اگر !defined(__NetBSD__)
  #شامل <sys/user.h>
  #endif
#شامل <sys/param.h>
#شامل <sys/sysctl.h>
#شامل <kvm.h>
#وگرنه
#شامل <sys/param.h>
#endif

#شامل "rc.h"
#شامل "misc.h"

ایستا ثابت نویسه *ثابت dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(صحیح dirfd, ثابت نویسه *pathname);
تهی clear_dirfds(تهی);

#endif
