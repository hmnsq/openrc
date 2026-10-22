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
#اگر_غیر_تعریف_شدہ RC_EXEC_H
#تعریف_کرو RC_EXEC_H

#شامل <stdbool.h>
#شامل <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

شمار {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

ساخت exec_دلائل {
	ثابت حرف **argv;
	ثابت حرف *cmd;  /* if NULL, argv[0] is used */
	صحیح redirect_stdin;
	صحیح redirect_stdout;
	صحیح redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

ساخت exec_نتیجہ {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	صحیح proc_stdin;
	صحیح proc_stdout;
	صحیح proc_stderr;
};

ساخت exec_دلائل exec_init(ثابت حرف **argv);
ساخت exec_نتیجہ do_exec(ساخت exec_دلائل *دلائل);

/* some exec related helplers */
صحیح rc_waitpid(pid_t pid);
صحیح rc_pipe_command(ثابت حرف *cmd, صحیح devnullfd);

#اگر_ختم
