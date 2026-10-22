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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "rc_exec.h"
#అన్తర్భావ "helpers.h"

స్థిర శూన్య checked_close(పూర్ణాంకం fd)
{
	ఒకవేళ (fd >= 0)
		close(fd);
}

స్థిర పూర్ణాంకం devnull(శూన్య)
{
	స్థిర పూర్ణాంకం devnullfd = -1;
	ఒకవేళ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ఒకవేళ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	తిరిగి devnullfd;
}

నిర్మాణం exec_వాదనలు exec_init(స్థిరరాశి అక్షరం **argv)
{
	నిర్మాణం exec_వాదనలు వాదనలు = {0};
	వాదనలు.argv = argv;
	వాదనలు.redirect_stdin  = EXEC_NO_REDIRECT;
	వాదనలు.redirect_stdout = EXEC_NO_REDIRECT;
	వాదనలు.redirect_stderr = EXEC_NO_REDIRECT;
	వాదనలు.uid = (uid_t)-1;
	వాదనలు.gid = (gid_t)-1;
	తిరిగి వాదనలు;
}

నిర్మాణం exec_ఫలితం do_exec(నిర్మాణం exec_వాదనలు *వాదనలు)
{
	నిర్మాణం exec_ఫలితం res = { .pid = -1 };
	sigset_t నిండిన, old;
	పూర్ణాంకం saved_errno, err, n;
	పూర్ణాంకం execpipe[2] = {-1, -1};
	పూర్ణాంకం stdin_pipe[2] = {-1, -1};
	పూర్ణాంకం stdout_pipe[2] = {-1, -1};
	పూర్ణాంకం stderr_pipe[2] = {-1, -1};
	స్థిరరాశి అక్షరం *cmd = వాదనలు->cmd ? వాదనలు->cmd : వాదనలు->argv[0];

	ఒకవేళ (pipe2(execpipe, O_CLOEXEC) < 0)
		వెళ్ళు నిష్క్రమించు;
	ఒకవేళ (వాదనలు->redirect_stdin == EXEC_MKPIPE) {
		ఒకవేళ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			వెళ్ళు నిష్క్రమించు;
		వాదనలు->redirect_stdin = stdin_pipe[0];
	}
	ఒకవేళ (వాదనలు->redirect_stdout == EXEC_MKPIPE) {
		ఒకవేళ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			వెళ్ళు నిష్క్రమించు;
		వాదనలు->redirect_stdout = stdout_pipe[1];
	}
	ఒకవేళ (వాదనలు->redirect_stderr == EXEC_MKPIPE) {
		ఒకవేళ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			వెళ్ళు నిష్క్రమించు;
		వాదనలు->redirect_stderr = stderr_pipe[1];
	}

	ఒకవేళ (వాదనలు->redirect_stdin == EXEC_DEVNULL)
		వాదనలు->redirect_stdin = devnull();
	ఒకవేళ (వాదనలు->redirect_stdout == EXEC_DEVNULL)
		వాదనలు->redirect_stdout = devnull();
	ఒకవేళ (వాదనలు->redirect_stderr == EXEC_DEVNULL)
		వాదనలు->redirect_stderr = devnull();

	sigfillset(&నిండిన);
	sigprocmask(SIG_SETMASK, &నిండిన, &old);
	res.pid = fork();

	ఒకవేళ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ఖాళీ);
		వెళ్ళు నిష్క్రమించు;
	} లేకపోతే ఒకవేళ (res.pid == 0) {
		/*బిడ్డ*/
		ఒకవేళ (వాదనలు->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(వాదనలు->redirect_stdin, STDIN_FILENO) < 0)
			వెళ్ళు బిడ్డ_err;
		ఒకవేళ (వాదనలు->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(వాదనలు->redirect_stdout, STDOUT_FILENO) < 0)
			వెళ్ళు బిడ్డ_err;
		ఒకవేళ (వాదనలు->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(వాదనలు->redirect_stderr, STDERR_FILENO) < 0)
			వెళ్ళు బిడ్డ_err;
		ఒకవేళ (వాదనలు->gid != (gid_t)-1 && setgid(వాదనలు->gid) < 0)
			వెళ్ళు బిడ్డ_err;
		ఒకవేళ (వాదనలు->uid != (uid_t)-1 && setuid(వాదనలు->uid) < 0)
			వెళ్ళు బిడ్డ_err;
		ఒకవేళ (వాదనలు->setsid && setsid() < 0)
			వెళ్ళు బిడ్డ_err;
		sigprocmask(SIG_SETMASK, &old, ఖాళీ);
		execvp(cmd, UNCONST(వాదనలు->argv));
బిడ్డ_err:
		saved_errno = errno;
		లిఖ(execpipe[1], &saved_errno, పరిమాణం saved_errno);
		_exit(1);
	} లేకపోతే {
		/*మాతృ*/
		close(execpipe[1]);
		execpipe[1] = -1;
		n = పఠ(execpipe[0], &err, పరిమాణం err);
		ఒకవేళ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ఖాళీ, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ఖాళీ);
		ఒకవేళ (res.pid < 0) {
			errno = (n == పరిమాణం err) ? err : EINVAL;
			వెళ్ళు నిష్క్రమించు;
		}

		ఒకవేళ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ఒకవేళ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ఒకవేళ (stderr_pipe[0] >= 0) {
			res.proc_stderr = stderr_pipe[0];
			stderr_pipe[0] = -1;
		}
	}

నిష్క్రమించు:
	saved_errno = errno;
	checked_close(execpipe[0]); checked_close(execpipe[1]);
	checked_close(stdin_pipe[0]); checked_close(stdin_pipe[1]);
	checked_close(stdout_pipe[0]); checked_close(stdout_pipe[1]);
	checked_close(stderr_pipe[0]); checked_close(stderr_pipe[1]);
	errno = saved_errno;
	తిరిగి res;
}

పూర్ణాంకం rc_waitpid(pid_t pid)
{
	పూర్ణాంకం స్థితి_2;
	అయితే (waitpid(pid, &స్థితి_2, 0) == -1) {
		ఒకవేళ (errno != EINTR) {
			స్థితి_2 = -1;
			ఆపు;
		}
	}
	తిరిగి స్థితి_2;
}

పూర్ణాంకం rc_pipe_command(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం devnullfd)
{
	స్థిరరాశి అక్షరం *argv[] = { "/bin/sh", "-c", cmd, ఖాళీ };
	నిర్మాణం exec_ఫలితం res;
	నిర్మాణం exec_వాదనలు వాదనలు = exec_init(argv);
	వాదనలు.redirect_stdin = EXEC_MKPIPE;
	వాదనలు.redirect_stdout = వాదనలు.redirect_stderr = devnullfd;
	res = do_exec(&వాదనలు);
	తిరిగి (res.pid > 0) ? res.proc_stdin : -1;
}
