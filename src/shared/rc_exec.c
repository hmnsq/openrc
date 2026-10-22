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

بنية exec_وسائط exec_init(ثابت حرف **argv)
{
	بنية exec_وسائط وسائط = {0};
	وسائط.argv = argv;
	وسائط.redirect_stdin  = EXEC_NO_REDIRECT;
	وسائط.redirect_stdout = EXEC_NO_REDIRECT;
	وسائط.redirect_stderr = EXEC_NO_REDIRECT;
	وسائط.uid = (uid_t)-1;
	وسائط.gid = (gid_t)-1;
	أرجع وسائط;
}

بنية exec_result do_exec(بنية exec_وسائط *وسائط)
{
	بنية exec_result res = { .pid = -1 };
	sigset_t full, old;
	صحيح saved_errno, err, n;
	صحيح execpipe[2] = {-1, -1};
	صحيح stdin_pipe[2] = {-1, -1};
	صحيح stdout_pipe[2] = {-1, -1};
	صحيح stderr_pipe[2] = {-1, -1};
	ثابت حرف *cmd = وسائط->cmd ? وسائط->cmd : وسائط->argv[0];

	إذا (pipe2(execpipe, O_CLOEXEC) < 0)
		اذهب exit;
	إذا (وسائط->redirect_stdin == EXEC_MKPIPE) {
		إذا (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			اذهب exit;
		وسائط->redirect_stdin = stdin_pipe[0];
	}
	إذا (وسائط->redirect_stdout == EXEC_MKPIPE) {
		إذا (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			اذهب exit;
		وسائط->redirect_stdout = stdout_pipe[1];
	}
	إذا (وسائط->redirect_stderr == EXEC_MKPIPE) {
		إذا (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			اذهب exit;
		وسائط->redirect_stderr = stderr_pipe[1];
	}

	إذا (وسائط->redirect_stdin == EXEC_DEVNULL)
		وسائط->redirect_stdin = devnull();
	إذا (وسائط->redirect_stdout == EXEC_DEVNULL)
		وسائط->redirect_stdout = devnull();
	إذا (وسائط->redirect_stderr == EXEC_DEVNULL)
		وسائط->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	إذا (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, عدم);
		اذهب exit;
	} وإلا إذا (res.pid == 0) {
		/* child */
		إذا (وسائط->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(وسائط->redirect_stdin, STDIN_FILENO) < 0)
			اذهب child_err;
		إذا (وسائط->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(وسائط->redirect_stdout, STDOUT_FILENO) < 0)
			اذهب child_err;
		إذا (وسائط->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(وسائط->redirect_stderr, STDERR_FILENO) < 0)
			اذهب child_err;
		إذا (وسائط->gid != (gid_t)-1 && setgid(وسائط->gid) < 0)
			اذهب child_err;
		إذا (وسائط->uid != (uid_t)-1 && setuid(وسائط->uid) < 0)
			اذهب child_err;
		إذا (وسائط->setsid && setsid() < 0)
			اذهب child_err;
		sigprocmask(SIG_SETMASK, &old, عدم);
		execvp(cmd, UNCONST(وسائط->argv));
child_err:
		saved_errno = errno;
		اكتب(execpipe[1], &saved_errno, حجم saved_errno);
		_exit(1);
	} وإلا {
		/* parent */
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
			اذهب exit;
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

exit:
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
	صحيح status;
	طالما (waitpid(pid, &status, 0) == -1) {
		إذا (errno != EINTR) {
			status = -1;
			اكسر;
		}
	}
	أرجع status;
}

صحيح rc_pipe_command(ثابت حرف *cmd, صحيح devnullfd)
{
	ثابت حرف *argv[] = { "/bin/sh", "-c", cmd, عدم };
	بنية exec_result res;
	بنية exec_وسائط وسائط = exec_init(argv);
	وسائط.redirect_stdin = EXEC_MKPIPE;
	وسائط.redirect_stdout = وسائط.redirect_stderr = devnullfd;
	res = do_exec(&وسائط);
	أرجع (res.pid > 0) ? res.proc_stdin : -1;
}
