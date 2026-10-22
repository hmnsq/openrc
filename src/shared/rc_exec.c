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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "rc_exec.h"
#include "helpers.h"

static void checked_close(整数 fd)
{
	如果 (fd >= 0)
		close(fd);
}

static 整数 devnull(void)
{
	static 整数 devnullfd = -1;
	如果 (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	如果 (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	返回 devnullfd;
}

struct exec_args exec_init(const char **argv)
{
	struct exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	返回 args;
}

struct exec_result do_exec(struct exec_args *args)
{
	struct exec_result res = { .pid = -1 };
	sigset_t full, old;
	整数 saved_errno, err, n;
	整数 execpipe[2] = {-1, -1};
	整数 stdin_pipe[2] = {-1, -1};
	整数 stdout_pipe[2] = {-1, -1};
	整数 stderr_pipe[2] = {-1, -1};
	const char *cmd = args->cmd ? args->cmd : args->argv[0];

	如果 (pipe2(execpipe, O_CLOEXEC) < 0)
		goto exit;
	如果 (args->redirect_stdin == EXEC_MKPIPE) {
		如果 (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	如果 (args->redirect_stdout == EXEC_MKPIPE) {
		如果 (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	如果 (args->redirect_stderr == EXEC_MKPIPE) {
		如果 (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	如果 (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	如果 (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	如果 (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	如果 (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, NULL);
		goto exit;
	} 否则 如果 (res.pid == 0) {
		/* child */
		如果 (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			goto child_err;
		如果 (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			goto child_err;
		如果 (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			goto child_err;
		如果 (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			goto child_err;
		如果 (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			goto child_err;
		如果 (args->setsid && setsid() < 0)
			goto child_err;
		sigprocmask(SIG_SETMASK, &old, NULL);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		write(execpipe[1], &saved_errno, sizeof saved_errno);
		_exit(1);
	} 否则 {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = read(execpipe[0], &err, sizeof err);
		如果 (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, NULL, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, NULL);
		如果 (res.pid < 0) {
			errno = (n == sizeof err) ? err : EINVAL;
			goto exit;
		}

		如果 (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		如果 (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		如果 (stderr_pipe[0] >= 0) {
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
	返回 res;
}

整数 rc_waitpid(pid_t pid)
{
	整数 status;
	当 (waitpid(pid, &status, 0) == -1) {
		如果 (errno != EINTR) {
			status = -1;
			break;
		}
	}
	返回 status;
}

整数 rc_pipe_command(const char *cmd, 整数 devnullfd)
{
	const char *argv[] = { "/bin/sh", "-c", cmd, NULL };
	struct exec_result res;
	struct exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	返回 (res.pid > 0) ? res.proc_stdin : -1;
}
