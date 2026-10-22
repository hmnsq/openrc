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

#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/param.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/time.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <dirent.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <libgen.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <paths.h>
#અન્તર્ભાવ <regex.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <strings.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>

#જો defined(BSD) && !defined(__GNU__)
  #જો !defined(__NetBSD__)
  #અન્તર્ભાવ <sys/user.h>
  #endif
#અન્તર્ભાવ <sys/param.h>
#અન્તર્ભાવ <sys/sysctl.h>
#અન્તર્ભાવ <kvm.h>
#નહીંતર
#અન્તર્ભાવ <sys/param.h>
#endif

#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"

સ્થિર અચલ અક્ષર *અચલ dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(પૂર્ણાંક dirfd, અચલ અક્ષર *pathname);
શૂન્ય clear_dirfds(શૂન્ય);

#endif
