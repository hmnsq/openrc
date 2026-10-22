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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "rc_exec.h"
#அந்தர்பாவ "helpers.h"

நிலையான வெறுமை checked_close(முழு fd)
{
	எனில் (fd >= 0)
		close(fd);
}

நிலையான முழு devnull(வெறுமை)
{
	நிலையான முழு devnullfd = -1;
	எனில் (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	எனில் (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	திரும்பு devnullfd;
}

கட்டமைப்பு exec_args exec_init(மாறிலி எழுத்து **argv)
{
	கட்டமைப்பு exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	திரும்பு args;
}

கட்டமைப்பு exec_result do_exec(கட்டமைப்பு exec_args *args)
{
	கட்டமைப்பு exec_result res = { .pid = -1 };
	sigset_t full, old;
	முழு saved_errno, err, n;
	முழு execpipe[2] = {-1, -1};
	முழு stdin_pipe[2] = {-1, -1};
	முழு stdout_pipe[2] = {-1, -1};
	முழு stderr_pipe[2] = {-1, -1};
	மாறிலி எழுத்து *cmd = args->cmd ? args->cmd : args->argv[0];

	எனில் (pipe2(execpipe, O_CLOEXEC) < 0)
		செல் exit;
	எனில் (args->redirect_stdin == EXEC_MKPIPE) {
		எனில் (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			செல் exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	எனில் (args->redirect_stdout == EXEC_MKPIPE) {
		எனில் (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			செல் exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	எனில் (args->redirect_stderr == EXEC_MKPIPE) {
		எனில் (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			செல் exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	எனில் (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	எனில் (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	எனில் (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	எனில் (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, வெற்று);
		செல் exit;
	} இல்லையெனில் எனில் (res.pid == 0) {
		/* child */
		எனில் (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			செல் child_err;
		எனில் (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			செல் child_err;
		எனில் (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			செல் child_err;
		எனில் (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			செல் child_err;
		எனில் (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			செல் child_err;
		எனில் (args->setsid && setsid() < 0)
			செல் child_err;
		sigprocmask(SIG_SETMASK, &old, வெற்று);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		லிக(execpipe[1], &saved_errno, அளவு saved_errno);
		_exit(1);
	} இல்லையெனில் {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = பட(execpipe[0], &err, அளவு err);
		எனில் (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, வெற்று, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, வெற்று);
		எனில் (res.pid < 0) {
			errno = (n == அளவு err) ? err : EINVAL;
			செல் exit;
		}

		எனில் (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		எனில் (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		எனில் (stderr_pipe[0] >= 0) {
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
	திரும்பு res;
}

முழு rc_waitpid(pid_t pid)
{
	முழு status;
	போது (waitpid(pid, &status, 0) == -1) {
		எனில் (errno != EINTR) {
			status = -1;
			நிறுத்து;
		}
	}
	திரும்பு status;
}

முழு rc_pipe_command(மாறிலி எழுத்து *cmd, முழு devnullfd)
{
	மாறிலி எழுத்து *argv[] = { "/bin/sh", "-c", cmd, வெற்று };
	கட்டமைப்பு exec_result res;
	கட்டமைப்பு exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	திரும்பு (res.pid > 0) ? res.proc_stdin : -1;
}
