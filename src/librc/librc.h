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

#که_نه_تعریف_شوی _LIBRC_H_
#تعریف_کړه _LIBRC_H_

#تعریف_کړه دننه_LIBRC

#ضمن <sys/types.h>
#ضمن <sys/param.h>
#ضمن <sys/stat.h>
#ضمن <sys/time.h>
#ضمن <sys/wait.h>
#ضمن <ctype.h>
#ضمن <dirent.h>
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <libgen.h>
#ضمن <limits.h>
#ضمن <paths.h>
#ضمن <regex.h>
#ضمن <signal.h>
#ضمن <stdarg.h>
#ضمن <stdbool.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <strings.h>
#ضمن <time.h>
#ضمن <unistd.h>

#إذا defined(BSD) && !defined(__GNU__)
  #إذا !defined(__NetBSD__)
  #ضمن <sys/user.h>
  #د_که_پای
#ضمن <sys/param.h>
#ضمن <sys/sysctl.h>
#ضمن <kvm.h>
#وإلا
#ضمن <sys/param.h>
#د_که_پای

#ضمن "rc.h"
#ضمن "misc.h"

ساكن ثابت حرف *ثابت dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(صحيح dirfd, ثابت حرف *pathname);
فراغ clear_dirfds(فراغ);

#د_که_پای
