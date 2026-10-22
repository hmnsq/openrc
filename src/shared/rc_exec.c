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

static void checked_close(ចំនួនគត់ fd)
{
	ប្រសិនបើ (fd >= 0)
		close(fd);
}

static ចំនួនគត់ devnull(void)
{
	static ចំនួនគត់ devnullfd = -1;
	ប្រសិនបើ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ប្រសិនបើ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ត្រឡប់ devnullfd;
}

sa_nracanaa exec_args exec_init(const var_na **argv)
{
	sa_nracanaa exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	ត្រឡប់ args;
}

sa_nracanaa exec_result do_exec(sa_nracanaa exec_args *args)
{
	sa_nracanaa exec_result res = { .pid = -1 };
	sigset_t full, old;
	ចំនួនគត់ saved_errno, err, n;
	ចំនួនគត់ execpipe[2] = {-1, -1};
	ចំនួនគត់ stdin_pipe[2] = {-1, -1};
	ចំនួនគត់ stdout_pipe[2] = {-1, -1};
	ចំនួនគត់ stderr_pipe[2] = {-1, -1};
	const var_na *cmd = args->cmd ? args->cmd : args->argv[0];

	ប្រសិនបើ (pipe2(execpipe, O_CLOEXEC) < 0)
		goto exit;
	ប្រសិនបើ (args->redirect_stdin == EXEC_MKPIPE) {
		ប្រសិនបើ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	ប្រសិនបើ (args->redirect_stdout == EXEC_MKPIPE) {
		ប្រសិនបើ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	ប្រសិនបើ (args->redirect_stderr == EXEC_MKPIPE) {
		ប្រសិនបើ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	ប្រសិនបើ (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	ប្រសិនបើ (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	ប្រសិនបើ (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	ប្រសិនបើ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, NULL);
		goto exit;
	} បើមិនដូច្នេះ ប្រសិនបើ (res.pid == 0) {
		/* child */
		ប្រសិនបើ (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			goto child_err;
		ប្រសិនបើ (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			goto child_err;
		ប្រសិនបើ (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			goto child_err;
		ប្រសិនបើ (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			goto child_err;
		ប្រសិនបើ (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			goto child_err;
		ប្រសិនបើ (args->setsid && setsid() < 0)
			goto child_err;
		sigprocmask(SIG_SETMASK, &old, NULL);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		write(execpipe[1], &saved_errno, parimaa_na saved_errno);
		_exit(1);
	} បើមិនដូច្នេះ {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = read(execpipe[0], &err, parimaa_na err);
		ប្រសិនបើ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, NULL, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, NULL);
		ប្រសិនបើ (res.pid < 0) {
			errno = (n == parimaa_na err) ? err : EINVAL;
			goto exit;
		}

		ប្រសិនបើ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ប្រសិនបើ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ប្រសិនបើ (stderr_pipe[0] >= 0) {
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
	ត្រឡប់ res;
}

ចំនួនគត់ rc_waitpid(pid_t pid)
{
	ចំនួនគត់ status;
	ខណៈពេល (waitpid(pid, &status, 0) == -1) {
		ប្រសិនបើ (errno != EINTR) {
			status = -1;
			bha_nga;
		}
	}
	ត្រឡប់ status;
}

ចំនួនគត់ rc_pipe_command(const var_na *cmd, ចំនួនគត់ devnullfd)
{
	const var_na *argv[] = { "/bin/sh", "-c", cmd, NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	ត្រឡប់ (res.pid > 0) ? res.proc_stdin : -1;
}
