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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _LIBRC_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _LIBRC_H_

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_LIBRC

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dirent.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <libgen.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <paths.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <regex.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <strings.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਜੇ defined(BSD) && !defined(__GNU__)
  #ਜੇ !defined(__NetBSD__)
  #ਅਨ੍ਤਰ੍ਭਾਵ <sys/user.h>
  #ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/sysctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kvm.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(ਪੂਰਨ dirfd, ਅਚਲ ਅੱਖਰ *pathname);
ਖਾਲੀ clear_dirfds(ਖਾਲੀ);

#ਜੇ_ਅੰਤ
