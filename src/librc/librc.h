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

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/time.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <dirent.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <libgen.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <paths.h>
#अन्तर्भाव <regex.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <strings.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>

#यदि defined(BSD) && !defined(__GNU__)
  #यदि !defined(__NetBSD__)
  #अन्तर्भाव <sys/user.h>
  #endif
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <sys/sysctl.h>
#अन्तर्भाव <kvm.h>
#अन्यथा
#अन्तर्भाव <sys/param.h>
#endif

#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"

स्थिर ध्रुव वर्ण *ध्रुव dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(पूर्णाङ्क dirfd, ध्रुव वर्ण *pathname);
शून्य clear_dirfds(शून्य);

#endif
