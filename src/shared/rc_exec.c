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

static void checked_close(puur_naa_nka fd)
{
	ᯀᯮᯝ (fd >= 0)
		close(fd);
}

static puur_naa_nka devnull(void)
{
	static puur_naa_nka devnullfd = -1;
	ᯀᯮᯝ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ᯀᯮᯝ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ᯀᯮᯞ devnullfd;
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
	ᯀᯮᯞ args;
}

sa_nracanaa exec_result do_exec(sa_nracanaa exec_args *args)
{
	sa_nracanaa exec_result res = { .pid = -1 };
	sigset_t full, old;
	puur_naa_nka saved_errno, err, n;
	puur_naa_nka execpipe[2] = {-1, -1};
	puur_naa_nka stdin_pipe[2] = {-1, -1};
	puur_naa_nka stdout_pipe[2] = {-1, -1};
	puur_naa_nka stderr_pipe[2] = {-1, -1};
	const var_na *cmd = args->cmd ? args->cmd : args->argv[0];

	ᯀᯮᯝ (pipe2(execpipe, O_CLOEXEC) < 0)
		goto exit;
	ᯀᯮᯝ (args->redirect_stdin == EXEC_MKPIPE) {
		ᯀᯮᯝ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	ᯀᯮᯝ (args->redirect_stdout == EXEC_MKPIPE) {
		ᯀᯮᯝ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	ᯀᯮᯝ (args->redirect_stderr == EXEC_MKPIPE) {
		ᯀᯮᯝ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			goto exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	ᯀᯮᯝ (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	ᯀᯮᯝ (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	ᯀᯮᯝ (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	ᯀᯮᯝ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, NULL);
		goto exit;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (res.pid == 0) {
		/* child */
		ᯀᯮᯝ (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			goto child_err;
		ᯀᯮᯝ (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			goto child_err;
		ᯀᯮᯝ (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			goto child_err;
		ᯀᯮᯝ (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			goto child_err;
		ᯀᯮᯝ (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			goto child_err;
		ᯀᯮᯝ (args->setsid && setsid() < 0)
			goto child_err;
		sigprocmask(SIG_SETMASK, &old, NULL);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		write(execpipe[1], &saved_errno, parimaa_na saved_errno);
		_exit(1);
	} ᯀᯮᯝᯂᯪ {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = read(execpipe[0], &err, parimaa_na err);
		ᯀᯮᯝ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, NULL, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, NULL);
		ᯀᯮᯝ (res.pid < 0) {
			errno = (n == parimaa_na err) ? err : EINVAL;
			goto exit;
		}

		ᯀᯮᯝ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ᯀᯮᯝ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ᯀᯮᯝ (stderr_pipe[0] >= 0) {
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
	ᯀᯮᯞ res;
}

puur_naa_nka rc_waitpid(pid_t pid)
{
	puur_naa_nka status;
	ᯀᯬᯝ (waitpid(pid, &status, 0) == -1) {
		ᯀᯮᯝ (errno != EINTR) {
			status = -1;
			bha_nga;
		}
	}
	ᯀᯮᯞ status;
}

puur_naa_nka rc_pipe_command(const var_na *cmd, puur_naa_nka devnullfd)
{
	const var_na *argv[] = { "/bin/sh", "-c", cmd, NULL };
	sa_nracanaa exec_result res;
	sa_nracanaa exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	ᯀᯮᯞ (res.pid > 0) ? res.proc_stdin : -1;
}
