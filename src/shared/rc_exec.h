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

#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

எண்ணிக்கை {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

கட்டமைப்பு exec_args {
	மாறிலி எழுத்து **argv;
	மாறிலி எழுத்து *cmd;  /* if NULL, argv[0] is used */
	முழு redirect_stdin;
	முழு redirect_stdout;
	முழு redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

கட்டமைப்பு exec_result {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	முழு proc_stdin;
	முழு proc_stdout;
	முழு proc_stderr;
};

கட்டமைப்பு exec_args exec_init(மாறிலி எழுத்து **argv);
கட்டமைப்பு exec_result do_exec(கட்டமைப்பு exec_args *args);

/* some exec related helplers */
முழு rc_waitpid(pid_t pid);
முழு rc_pipe_command(மாறிலி எழுத்து *cmd, முழு devnullfd);

#endif
