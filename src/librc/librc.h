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

#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/param.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/time.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <dirent.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <libgen.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <paths.h>
#அந்தர்பாவ <regex.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <strings.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>

#எனில் defined(BSD) && !defined(__GNU__)
  #எனில் !defined(__NetBSD__)
  #அந்தர்பாவ <sys/user.h>
  #endif
#அந்தர்பாவ <sys/param.h>
#அந்தர்பாவ <sys/sysctl.h>
#அந்தர்பாவ <kvm.h>
#இல்லையெனில்
#அந்தர்பாவ <sys/param.h>
#endif

#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"

நிலையான மாறிலி எழுத்து *மாறிலி dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(முழு dirfd, மாறிலி எழுத்து *pathname);
வெறுமை clear_dirfds(வெறுமை);

#endif
