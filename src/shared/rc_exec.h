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

#شامل <stdbool.h>
#شامل <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

شمارش {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

ساختار exec_args {
	ثابت نویسه **argv;
	ثابت نویسه *cmd;  /* if NULL, argv[0] is used */
	صحیح redirect_stdin;
	صحیح redirect_stdout;
	صحیح redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

ساختار exec_result {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	صحیح proc_stdin;
	صحیح proc_stdout;
	صحیح proc_stderr;
};

ساختار exec_args exec_init(ثابت نویسه **argv);
ساختار exec_result do_exec(ساختار exec_args *args);

/* some exec related helplers */
صحیح rc_waitpid(pid_t pid);
صحیح rc_pipe_command(ثابت نویسه *cmd, صحیح devnullfd);

#endif
