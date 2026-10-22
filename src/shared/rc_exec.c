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

#شامل <errno.h>
#شامل <fcntl.h>
#شامل <signal.h>
#شامل <sys/wait.h>
#شامل <unistd.h>

#شامل "rc_exec.h"
#شامل "helpers.h"

ایستا تهی checked_close(صحیح fd)
{
	اگر (fd >= 0)
		close(fd);
}

ایستا صحیح devnull(تهی)
{
	ایستا صحیح devnullfd = -1;
	اگر (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	اگر (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	بازگشت devnullfd;
}

ساختار exec_args exec_init(ثابت نویسه **argv)
{
	ساختار exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	بازگشت args;
}

ساختار exec_result do_exec(ساختار exec_args *args)
{
	ساختار exec_result res = { .pid = -1 };
	sigset_t full, old;
	صحیح saved_errno, err, n;
	صحیح execpipe[2] = {-1, -1};
	صحیح stdin_pipe[2] = {-1, -1};
	صحیح stdout_pipe[2] = {-1, -1};
	صحیح stderr_pipe[2] = {-1, -1};
	ثابت نویسه *cmd = args->cmd ? args->cmd : args->argv[0];

	اگر (pipe2(execpipe, O_CLOEXEC) < 0)
		برو exit;
	اگر (args->redirect_stdin == EXEC_MKPIPE) {
		اگر (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			برو exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	اگر (args->redirect_stdout == EXEC_MKPIPE) {
		اگر (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			برو exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	اگر (args->redirect_stderr == EXEC_MKPIPE) {
		اگر (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			برو exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	اگر (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	اگر (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	اگر (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	اگر (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, پوچ);
		برو exit;
	} وگرنه اگر (res.pid == 0) {
		/* child */
		اگر (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			برو child_err;
		اگر (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			برو child_err;
		اگر (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			برو child_err;
		اگر (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			برو child_err;
		اگر (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			برو child_err;
		اگر (args->setsid && setsid() < 0)
			برو child_err;
		sigprocmask(SIG_SETMASK, &old, پوچ);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		اكتب(execpipe[1], &saved_errno, اندازه saved_errno);
		_exit(1);
	} وگرنه {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = اقرأ(execpipe[0], &err, اندازه err);
		اگر (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, پوچ, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, پوچ);
		اگر (res.pid < 0) {
			errno = (n == اندازه err) ? err : EINVAL;
			برو exit;
		}

		اگر (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		اگر (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		اگر (stderr_pipe[0] >= 0) {
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
	بازگشت res;
}

صحیح rc_waitpid(pid_t pid)
{
	صحیح status;
	وقتی (waitpid(pid, &status, 0) == -1) {
		اگر (errno != EINTR) {
			status = -1;
			بشکن;
		}
	}
	بازگشت status;
}

صحیح rc_pipe_command(ثابت نویسه *cmd, صحیح devnullfd)
{
	ثابت نویسه *argv[] = { "/bin/sh", "-c", cmd, پوچ };
	ساختار exec_result res;
	ساختار exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	بازگشت (res.pid > 0) ? res.proc_stdin : -1;
}
