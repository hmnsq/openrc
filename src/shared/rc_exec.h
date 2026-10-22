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

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

ಗಣನಾ {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

ಸಂರಚನಾ exec_args {
	ಧ್ರುವ ವರ್ಣ **argv;
	ಧ್ರುವ ವರ್ಣ *cmd;  /* if NULL, argv[0] is used */
	ಪೂರ್ಣಾಙ್ಕ redirect_stdin;
	ಪೂರ್ಣಾಙ್ಕ redirect_stdout;
	ಪೂರ್ಣಾಙ್ಕ redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

ಸಂರಚನಾ exec_result {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	ಪೂರ್ಣಾಙ್ಕ proc_stdin;
	ಪೂರ್ಣಾಙ್ಕ proc_stdout;
	ಪೂರ್ಣಾಙ್ಕ proc_stderr;
};

ಸಂರಚನಾ exec_args exec_init(ಧ್ರುವ ವರ್ಣ **argv);
ಸಂರಚನಾ exec_result do_exec(ಸಂರಚನಾ exec_args *args);

/* some exec related helplers */
ಪೂರ್ಣಾಙ್ಕ rc_waitpid(pid_t pid);
ಪೂರ್ಣಾಙ್ಕ rc_pipe_command(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ devnullfd);

#endif
