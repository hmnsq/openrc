/*
 * Copyright (c) 2025 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE file.
 */
#ifndef RC_EXEC_H
#define RC_EXEC_H

#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

गणना {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

संरचना exec_args {
	स्थिरांक अक्षर **argv;
	स्थिरांक अक्षर *cmd;  /* if NULL, argv[0] is used */
	पूर्णांक redirect_stdin;
	पूर्णांक redirect_stdout;
	पूर्णांक redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

संरचना exec_result {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	पूर्णांक proc_stdin;
	पूर्णांक proc_stdout;
	पूर्णांक proc_stderr;
};

संरचना exec_args exec_init(स्थिरांक अक्षर **argv);
संरचना exec_result do_exec(संरचना exec_args *args);

/* some exec related helplers */
पूर्णांक rc_waitpid(pid_t pid);
पूर्णांक rc_pipe_command(स्थिरांक अक्षर *cmd, पूर्णांक devnullfd);

#endif
