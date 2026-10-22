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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ RC_EXEC_H
#ವ್ಯಾಖ್ಯಾನಿಸು RC_EXEC_H

#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

ಎಣಿಕೆ {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

ರಚನೆ exec_ವಾದಗಳು {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd;  /* if NULL, argv[0] is used */
	ಪೂರ್ಣಾಂಕ redirect_stdin;
	ಪೂರ್ಣಾಂಕ redirect_stdout;
	ಪೂರ್ಣಾಂಕ redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

ರಚನೆ exec_ಫಲಿತಾಂಶ {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	ಪೂರ್ಣಾಂಕ proc_stdin;
	ಪೂರ್ಣಾಂಕ proc_stdout;
	ಪೂರ್ಣಾಂಕ proc_stderr;
};

ರಚನೆ exec_ವಾದಗಳು exec_init(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv);
ರಚನೆ exec_ಫಲಿತಾಂಶ do_exec(ರಚನೆ exec_ವಾದಗಳು *ವಾದಗಳು);

/* some exec related helplers */
ಪೂರ್ಣಾಂಕ rc_waitpid(pid_t pid);
ಪೂರ್ಣಾಂಕ rc_pipe_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ devnullfd);

#ಆದರೆ_ಅಂತ್ಯ
