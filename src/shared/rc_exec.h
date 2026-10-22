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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ RC_EXEC_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RC_EXEC_H

#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

ਗਿਣਤੀ {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

ਬਣਤਰ exec_ਦਲੀਲਾਂ {
	ਅਚਲ ਅੱਖਰ **argv;
	ਅਚਲ ਅੱਖਰ *cmd;  /* if NULL, argv[0] is used */
	ਪੂਰਨ redirect_stdin;
	ਪੂਰਨ redirect_stdout;
	ਪੂਰਨ redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

ਬਣਤਰ exec_ਨਤੀਜਾ {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	ਪੂਰਨ proc_stdin;
	ਪੂਰਨ proc_stdout;
	ਪੂਰਨ proc_stderr;
};

ਬਣਤਰ exec_ਦਲੀਲਾਂ exec_init(ਅਚਲ ਅੱਖਰ **argv);
ਬਣਤਰ exec_ਨਤੀਜਾ do_exec(ਬਣਤਰ exec_ਦਲੀਲਾਂ *ਦਲੀਲਾਂ);

/* some exec related helplers */
ਪੂਰਨ rc_waitpid(pid_t pid);
ਪੂਰਨ rc_pipe_command(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ devnullfd);

#ਜੇ_ਅੰਤ
