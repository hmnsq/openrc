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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರ ಶೂನ್ಯ checked_close(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಯದಿ (fd >= 0)
		close(fd);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devnull(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ devnullfd = -1;
	ಯದಿ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ಯದಿ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ಪ್ರತ್ಯಾಗಮ devnullfd;
}

ಸಂರಚನಾ exec_args exec_init(ಧ್ರುವ ವರ್ಣ **argv)
{
	ಸಂರಚನಾ exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	ಪ್ರತ್ಯಾಗಮ args;
}

ಸಂರಚನಾ exec_result do_exec(ಸಂರಚನಾ exec_args *args)
{
	ಸಂರಚನಾ exec_result res = { .pid = -1 };
	sigset_t full, old;
	ಪೂರ್ಣಾಙ್ಕ saved_errno, err, n;
	ಪೂರ್ಣಾಙ್ಕ execpipe[2] = {-1, -1};
	ಪೂರ್ಣಾಙ್ಕ stdin_pipe[2] = {-1, -1};
	ಪೂರ್ಣಾಙ್ಕ stdout_pipe[2] = {-1, -1};
	ಪೂರ್ಣಾಙ್ಕ stderr_pipe[2] = {-1, -1};
	ಧ್ರುವ ವರ್ಣ *cmd = args->cmd ? args->cmd : args->argv[0];

	ಯದಿ (pipe2(execpipe, O_CLOEXEC) < 0)
		ಗಚ್ಛ exit;
	ಯದಿ (args->redirect_stdin == EXEC_MKPIPE) {
		ಯದಿ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			ಗಚ್ಛ exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	ಯದಿ (args->redirect_stdout == EXEC_MKPIPE) {
		ಯದಿ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			ಗಚ್ಛ exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	ಯದಿ (args->redirect_stderr == EXEC_MKPIPE) {
		ಯದಿ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			ಗಚ್ಛ exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	ಯದಿ (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	ಯದಿ (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	ಯದಿ (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	ಯದಿ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
		ಗಚ್ಛ exit;
	} ಅನ್ಯಥಾ ಯದಿ (res.pid == 0) {
		/* child */
		ಯದಿ (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			ಗಚ್ಛ child_err;
		ಯದಿ (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			ಗಚ್ಛ child_err;
		ಯದಿ (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			ಗಚ್ಛ child_err;
		ಯದಿ (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			ಗಚ್ಛ child_err;
		ಯದಿ (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			ಗಚ್ಛ child_err;
		ಯದಿ (args->setsid && setsid() < 0)
			ಗಚ್ಛ child_err;
		sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		ಲಿಖ(execpipe[1], &saved_errno, ಪರಿಮಾಣ saved_errno);
		_exit(1);
	} ಅನ್ಯಥಾ {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = ಪಠ(execpipe[0], &err, ಪರಿಮಾಣ err);
		ಯದಿ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ರಿಕ್ತ, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ರಿಕ್ತ);
		ಯದಿ (res.pid < 0) {
			errno = (n == ಪರಿಮಾಣ err) ? err : EINVAL;
			ಗಚ್ಛ exit;
		}

		ಯದಿ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ಯದಿ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ಯದಿ (stderr_pipe[0] >= 0) {
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
	ಪ್ರತ್ಯಾಗಮ res;
}

ಪೂರ್ಣಾಙ್ಕ rc_waitpid(pid_t pid)
{
	ಪೂರ್ಣಾಙ್ಕ status;
	ಯಾವತ್ (waitpid(pid, &status, 0) == -1) {
		ಯದಿ (errno != EINTR) {
			status = -1;
			ಭಙ್ಗ;
		}
	}
	ಪ್ರತ್ಯಾಗಮ status;
}

ಪೂರ್ಣಾಙ್ಕ rc_pipe_command(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ devnullfd)
{
	ಧ್ರುವ ವರ್ಣ *argv[] = { "/bin/sh", "-c", cmd, ರಿಕ್ತ };
	ಸಂರಚನಾ exec_result res;
	ಸಂರಚನಾ exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	ಪ್ರತ್ಯಾಗಮ (res.pid > 0) ? res.proc_stdin : -1;
}
