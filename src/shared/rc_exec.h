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

#include <stdbool.h>
#include <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

ga_nanaa {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

sa_nracanaa exec_args {
	const var_na **argv;
	const var_na *cmd;  /* if NULL, argv[0] is used */
	ចំនួនគត់ redirect_stdin;
	ចំនួនគត់ redirect_stdout;
	ចំនួនគត់ redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

sa_nracanaa exec_result {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	ចំនួនគត់ proc_stdin;
	ចំនួនគត់ proc_stdout;
	ចំនួនគត់ proc_stderr;
};

sa_nracanaa exec_args exec_init(const var_na **argv);
sa_nracanaa exec_result do_exec(sa_nracanaa exec_args *args);

/* some exec related helplers */
ចំនួនគត់ rc_waitpid(pid_t pid);
ចំនួនគត់ rc_pipe_command(const var_na *cmd, ចំនួនគត់ devnullfd);

#endif
