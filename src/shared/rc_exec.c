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

#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <signal.h>
#ضمن <sys/wait.h>
#ضمن <unistd.h>

#ضمن "rc_exec.h"
#ضمن "helpers.h"

ساكن فراغ checked_close(صحيح fd)
{
	إذا (fd >= 0)
		close(fd);
}

ساكن صحيح devnull(فراغ)
{
	ساكن صحيح devnullfd = -1;
	إذا (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	إذا (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	أرجع devnullfd;
}

بنية exec_آرگومانها exec_init(ثابت حرف **argv)
{
	بنية exec_آرگومانها آرگومانها = {0};
	آرگومانها.argv = argv;
	آرگومانها.redirect_stdin  = EXEC_NO_REDIRECT;
	آرگومانها.redirect_stdout = EXEC_NO_REDIRECT;
	آرگومانها.redirect_stderr = EXEC_NO_REDIRECT;
	آرگومانها.uid = (uid_t)-1;
	آرگومانها.gid = (gid_t)-1;
	أرجع آرگومانها;
}

بنية exec_نتیجه do_exec(بنية exec_آرگومانها *آرگومانها)
{
	بنية exec_نتیجه res = { .pid = -1 };
	sigset_t پر, old;
	صحيح saved_errno, err, n;
	صحيح execpipe[2] = {-1, -1};
	صحيح stdin_pipe[2] = {-1, -1};
	صحيح stdout_pipe[2] = {-1, -1};
	صحيح stderr_pipe[2] = {-1, -1};
	ثابت حرف *cmd = آرگومانها->cmd ? آرگومانها->cmd : آرگومانها->argv[0];

	إذا (pipe2(execpipe, O_CLOEXEC) < 0)
		اذهب خروج;
	إذا (آرگومانها->redirect_stdin == EXEC_MKPIPE) {
		إذا (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			اذهب خروج;
		آرگومانها->redirect_stdin = stdin_pipe[0];
	}
	إذا (آرگومانها->redirect_stdout == EXEC_MKPIPE) {
		إذا (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			اذهب خروج;
		آرگومانها->redirect_stdout = stdout_pipe[1];
	}
	إذا (آرگومانها->redirect_stderr == EXEC_MKPIPE) {
		إذا (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			اذهب خروج;
		آرگومانها->redirect_stderr = stderr_pipe[1];
	}

	إذا (آرگومانها->redirect_stdin == EXEC_DEVNULL)
		آرگومانها->redirect_stdin = devnull();
	إذا (آرگومانها->redirect_stdout == EXEC_DEVNULL)
		آرگومانها->redirect_stdout = devnull();
	إذا (آرگومانها->redirect_stderr == EXEC_DEVNULL)
		آرگومانها->redirect_stderr = devnull();

	sigfillset(&پر);
	sigprocmask(SIG_SETMASK, &پر, &old);
	res.pid = fork();

	إذا (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, عدم);
		اذهب خروج;
	} وإلا إذا (res.pid == 0) {
		/*فرزند*/
		إذا (آرگومانها->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(آرگومانها->redirect_stdin, STDIN_FILENO) < 0)
			اذهب فرزند_err;
		إذا (آرگومانها->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(آرگومانها->redirect_stdout, STDOUT_FILENO) < 0)
			اذهب فرزند_err;
		إذا (آرگومانها->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(آرگومانها->redirect_stderr, STDERR_FILENO) < 0)
			اذهب فرزند_err;
		إذا (آرگومانها->gid != (gid_t)-1 && setgid(آرگومانها->gid) < 0)
			اذهب فرزند_err;
		إذا (آرگومانها->uid != (uid_t)-1 && setuid(آرگومانها->uid) < 0)
			اذهب فرزند_err;
		إذا (آرگومانها->setsid && setsid() < 0)
			اذهب فرزند_err;
		sigprocmask(SIG_SETMASK, &old, عدم);
		execvp(cmd, UNCONST(آرگومانها->argv));
فرزند_err:
		saved_errno = errno;
		اكتب(execpipe[1], &saved_errno, حجم saved_errno);
		_exit(1);
	} وإلا {
		/*والد*/
		close(execpipe[1]);
		execpipe[1] = -1;
		n = اقرأ(execpipe[0], &err, حجم err);
		إذا (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, عدم, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, عدم);
		إذا (res.pid < 0) {
			errno = (n == حجم err) ? err : EINVAL;
			اذهب خروج;
		}

		إذا (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		إذا (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		إذا (stderr_pipe[0] >= 0) {
			res.proc_stderr = stderr_pipe[0];
			stderr_pipe[0] = -1;
		}
	}

خروج:
	saved_errno = errno;
	checked_close(execpipe[0]); checked_close(execpipe[1]);
	checked_close(stdin_pipe[0]); checked_close(stdin_pipe[1]);
	checked_close(stdout_pipe[0]); checked_close(stdout_pipe[1]);
	checked_close(stderr_pipe[0]); checked_close(stderr_pipe[1]);
	errno = saved_errno;
	أرجع res;
}

صحيح rc_waitpid(pid_t pid)
{
	صحيح وضعیت;
	طالما (waitpid(pid, &وضعیت, 0) == -1) {
		إذا (errno != EINTR) {
			وضعیت = -1;
			اكسر;
		}
	}
	أرجع وضعیت;
}

صحيح rc_pipe_command(ثابت حرف *cmd, صحيح devnullfd)
{
	ثابت حرف *argv[] = { "/bin/sh", "-c", cmd, عدم };
	بنية exec_نتیجه res;
	بنية exec_آرگومانها آرگومانها = exec_init(argv);
	آرگومانها.redirect_stdin = EXEC_MKPIPE;
	آرگومانها.redirect_stdout = آرگومانها.redirect_stderr = devnullfd;
	res = do_exec(&آرگومانها);
	أرجع (res.pid > 0) ? res.proc_stdin : -1;
}
