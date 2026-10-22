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
	אם (fd >= 0)
		close(fd);
}

ساكن صحيح devnull(فراغ)
{
	ساكن صحيح devnullfd = -1;
	אם (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	אם (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	החזר devnullfd;
}

بنية exec_args exec_init(ثابت حرف **argv)
{
	بنية exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	החזר args;
}

بنية exec_result do_exec(بنية exec_args *args)
{
	بنية exec_result res = { .pid = -1 };
	sigset_t full, old;
	صحيح saved_errno, err, n;
	صحيح execpipe[2] = {-1, -1};
	صحيح stdin_pipe[2] = {-1, -1};
	صحيح stdout_pipe[2] = {-1, -1};
	صحيح stderr_pipe[2] = {-1, -1};
	ثابت حرف *cmd = args->cmd ? args->cmd : args->argv[0];

	אם (pipe2(execpipe, O_CLOEXEC) < 0)
		اذهب exit;
	אם (args->redirect_stdin == EXEC_MKPIPE) {
		אם (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			اذهب exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	אם (args->redirect_stdout == EXEC_MKPIPE) {
		אם (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			اذهب exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	אם (args->redirect_stderr == EXEC_MKPIPE) {
		אם (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			اذهب exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	אם (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	אם (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	אם (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	אם (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, عدم);
		اذهب exit;
	} אחרת אם (res.pid == 0) {
		/* child */
		אם (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			اذهب child_err;
		אם (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			اذهب child_err;
		אם (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			اذهب child_err;
		אם (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			اذهب child_err;
		אם (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			اذهب child_err;
		אם (args->setsid && setsid() < 0)
			اذهب child_err;
		sigprocmask(SIG_SETMASK, &old, عدم);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		اكتب(execpipe[1], &saved_errno, حجم saved_errno);
		_exit(1);
	} אחרת {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = اقرأ(execpipe[0], &err, حجم err);
		אם (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, عدم, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, عدم);
		אם (res.pid < 0) {
			errno = (n == حجم err) ? err : EINVAL;
			اذهب exit;
		}

		אם (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		אם (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		אם (stderr_pipe[0] >= 0) {
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
	החזר res;
}

صحيح rc_waitpid(pid_t pid)
{
	صحيح status;
	כלעוד (waitpid(pid, &status, 0) == -1) {
		אם (errno != EINTR) {
			status = -1;
			اكسر;
		}
	}
	החזר status;
}

صحيح rc_pipe_command(ثابت حرف *cmd, صحيح devnullfd)
{
	ثابت حرف *argv[] = { "/bin/sh", "-c", cmd, عدم };
	بنية exec_result res;
	بنية exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	החזר (res.pid > 0) ? res.proc_stdin : -1;
}
