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

#నిర్వచితం_కాకపోతే _LIBRC_H_
#నిర్వచించు _LIBRC_H_

#నిర్వచించు లోపల_LIBRC

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/param.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/time.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <dirent.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <libgen.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <paths.h>
#అన్తర్భావ <regex.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <strings.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>

#ఒకవేళ defined(BSD) && !defined(__GNU__)
  #ఒకవేళ !defined(__NetBSD__)
  #అన్తర్భావ <sys/user.h>
  #అయితే_ముగింపు
#అన్తర్భావ <sys/param.h>
#అన్తర్భావ <sys/sysctl.h>
#అన్తర్భావ <kvm.h>
#లేకపోతే
#అన్తర్భావ <sys/param.h>
#అయితే_ముగింపు

#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(పూర్ణాంకం dirfd, స్థిరరాశి అక్షరం *pathname);
శూన్య clear_dirfds(శూన్య);

#అయితే_ముగింపు
