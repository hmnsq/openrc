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

#अगर_अघोषित _LIBRC_H_
#घोषणा _LIBRC_H_

#घोषणा _IN_LIBRC

#समावेश <sys/types.h>
#समावेश <sys/param.h>
#समावेश <sys/stat.h>
#समावेश <sys/time.h>
#समावेश <sys/wait.h>
#समावेश <ctype.h>
#समावेश <dirent.h>
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <libgen.h>
#समावेश <limits.h>
#समावेश <paths.h>
#समावेश <regex.h>
#समावेश <signal.h>
#समावेश <stdarg.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <strings.h>
#समावेश <time.h>
#समावेश <unistd.h>

#अगर defined(BSD) && !defined(__GNU__)
  #अगर !defined(__NetBSD__)
  #समावेश <sys/user.h>
  #पूर्ण_अगर
#समावेश <sys/param.h>
#समावेश <sys/sysctl.h>
#समावेश <kvm.h>
#अन्यथा
#समावेश <sys/param.h>
#पूर्ण_अगर

#समावेश "rc.h"
#समावेश "misc.h"

अटल स्थिर अक्षर *स्थिर dirnames[RC_DIR_SYS_MAX] =
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

RC_STRINGLIST *config_list(पूर्णांक dirfd, स्थिर अक्षर *pathname);
व्योम clear_dirfds(व्योम);

#पूर्ण_अगर
