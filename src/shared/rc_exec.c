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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "rc_exec.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിതം ശൂന്യം checked_close(പൂർണ്ണസംഖ്യ fd)
{
	എങ്കിൽ (fd >= 0)
		close(fd);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ devnull(ശൂന്യം)
{
	സ്ഥിതം പൂർണ്ണസംഖ്യ devnullfd = -1;
	എങ്കിൽ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	എങ്കിൽ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	തിരികെ devnullfd;
}

ഘടന exec_args exec_init(സ്ഥിരം അക്ഷരം **argv)
{
	ഘടന exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	തിരികെ args;
}

ഘടന exec_result do_exec(ഘടന exec_args *args)
{
	ഘടന exec_result res = { .pid = -1 };
	sigset_t full, old;
	പൂർണ്ണസംഖ്യ saved_errno, err, n;
	പൂർണ്ണസംഖ്യ execpipe[2] = {-1, -1};
	പൂർണ്ണസംഖ്യ stdin_pipe[2] = {-1, -1};
	പൂർണ്ണസംഖ്യ stdout_pipe[2] = {-1, -1};
	പൂർണ്ണസംഖ്യ stderr_pipe[2] = {-1, -1};
	സ്ഥിരം അക്ഷരം *cmd = args->cmd ? args->cmd : args->argv[0];

	എങ്കിൽ (pipe2(execpipe, O_CLOEXEC) < 0)
		പോകുക exit;
	എങ്കിൽ (args->redirect_stdin == EXEC_MKPIPE) {
		എങ്കിൽ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			പോകുക exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	എങ്കിൽ (args->redirect_stdout == EXEC_MKPIPE) {
		എങ്കിൽ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			പോകുക exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	എങ്കിൽ (args->redirect_stderr == EXEC_MKPIPE) {
		എങ്കിൽ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			പോകുക exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	എങ്കിൽ (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	എങ്കിൽ (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	എങ്കിൽ (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	എങ്കിൽ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
		പോകുക exit;
	} അല്ലെങ്കിൽ എങ്കിൽ (res.pid == 0) {
		/* child */
		എങ്കിൽ (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			പോകുക child_err;
		എങ്കിൽ (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			പോകുക child_err;
		എങ്കിൽ (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			പോകുക child_err;
		എങ്കിൽ (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			പോകുക child_err;
		എങ്കിൽ (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			പോകുക child_err;
		എങ്കിൽ (args->setsid && setsid() < 0)
			പോകുക child_err;
		sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		ലിഖ(execpipe[1], &saved_errno, വലുപ്പം saved_errno);
		_exit(1);
	} അല്ലെങ്കിൽ {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = പഠ(execpipe[0], &err, വലുപ്പം err);
		എങ്കിൽ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ശൂന്യ, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ശൂന്യ);
		എങ്കിൽ (res.pid < 0) {
			errno = (n == വലുപ്പം err) ? err : EINVAL;
			പോകുക exit;
		}

		എങ്കിൽ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		എങ്കിൽ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		എങ്കിൽ (stderr_pipe[0] >= 0) {
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
	തിരികെ res;
}

പൂർണ്ണസംഖ്യ rc_waitpid(pid_t pid)
{
	പൂർണ്ണസംഖ്യ status;
	ആകുമ്പോൾ (waitpid(pid, &status, 0) == -1) {
		എങ്കിൽ (errno != EINTR) {
			status = -1;
			നിർത്തുക;
		}
	}
	തിരികെ status;
}

പൂർണ്ണസംഖ്യ rc_pipe_command(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ devnullfd)
{
	സ്ഥിരം അക്ഷരം *argv[] = { "/bin/sh", "-c", cmd, ശൂന്യ };
	ഘടന exec_result res;
	ഘടന exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	തിരികെ (res.pid > 0) ? res.proc_stdin : -1;
}
