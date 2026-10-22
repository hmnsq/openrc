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
#నిర్వచితం_కాకపోతే RC_EXEC_H
#నిర్వచించు RC_EXEC_H

#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <sys/types.h>

/*
 * goal of this api is to provide a convenient and centralized way to do most
 * of the fork() + exec() work. it doesn't attempt to do everything, but simple
 * use-cases should be covered. if any simple functionality is missing, add it
 * here instead of ad-hoc coding fork+exec yourself.
 */

గణన {
	/* default */
	EXEC_NO_REDIRECT = -1,
	/* make a pipe and redirect to it */
	EXEC_MKPIPE = -2,
	/* redirect to /dev/null */
	EXEC_DEVNULL = -3,
};

నిర్మాణం exec_వాదనలు {
	స్థిరరాశి అక్షరం **argv;
	స్థిరరాశి అక్షరం *cmd;  /* if NULL, argv[0] is used */
	పూర్ణాంకం redirect_stdin;
	పూర్ణాంకం redirect_stdout;
	పూర్ణాంకం redirect_stderr;
	uid_t uid;
	gid_t gid;
	bool setsid : 1;
};

నిర్మాణం exec_ఫలితం {
	pid_t pid;
	/* returned in case of RC_EXEC_MKPIPE */
	పూర్ణాంకం proc_stdin;
	పూర్ణాంకం proc_stdout;
	పూర్ణాంకం proc_stderr;
};

నిర్మాణం exec_వాదనలు exec_init(స్థిరరాశి అక్షరం **argv);
నిర్మాణం exec_ఫలితం do_exec(నిర్మాణం exec_వాదనలు *వాదనలు);

/* some exec related helplers */
పూర్ణాంకం rc_waitpid(pid_t pid);
పూర్ణాంకం rc_pipe_command(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం devnullfd);

#అయితే_ముగింపు
