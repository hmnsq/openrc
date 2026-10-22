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

#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/param.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/time.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <dirent.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <libgen.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <paths.h>
#അന്തര്ഭാവ <regex.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <strings.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>

#എങ്കിൽ defined(BSD) && !defined(__GNU__)
  #എങ്കിൽ !defined(__NetBSD__)
  #അന്തര്ഭാവ <sys/user.h>
  #endif
#അന്തര്ഭാവ <sys/param.h>
#അന്തര്ഭാവ <sys/sysctl.h>
#അന്തര്ഭാവ <kvm.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <sys/param.h>
#endif

#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(പൂർണ്ണസംഖ്യ dirfd, സ്ഥിരം അക്ഷരം *pathname);
ശൂന്യം clear_dirfds(ശൂന്യം);

#endif
