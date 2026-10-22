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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "rc_exec.h"
#અન્તર્ભાવ "helpers.h"

સ્થિર શૂન્ય checked_close(પૂર્ણાંક fd)
{
	જો (fd >= 0)
		close(fd);
}

સ્થિર પૂર્ણાંક devnull(શૂન્ય)
{
	સ્થિર પૂર્ણાંક devnullfd = -1;
	જો (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	જો (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	પાછા devnullfd;
}

સંરચના exec_args exec_init(અચલ અક્ષર **argv)
{
	સંરચના exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	પાછા args;
}

સંરચના exec_result do_exec(સંરચના exec_args *args)
{
	સંરચના exec_result res = { .pid = -1 };
	sigset_t full, old;
	પૂર્ણાંક saved_errno, err, n;
	પૂર્ણાંક execpipe[2] = {-1, -1};
	પૂર્ણાંક stdin_pipe[2] = {-1, -1};
	પૂર્ણાંક stdout_pipe[2] = {-1, -1};
	પૂર્ણાંક stderr_pipe[2] = {-1, -1};
	અચલ અક્ષર *cmd = args->cmd ? args->cmd : args->argv[0];

	જો (pipe2(execpipe, O_CLOEXEC) < 0)
		જાઓ exit;
	જો (args->redirect_stdin == EXEC_MKPIPE) {
		જો (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			જાઓ exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	જો (args->redirect_stdout == EXEC_MKPIPE) {
		જો (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			જાઓ exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	જો (args->redirect_stderr == EXEC_MKPIPE) {
		જો (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			જાઓ exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	જો (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	જો (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	જો (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	જો (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ખાલી);
		જાઓ exit;
	} નહીંતર જો (res.pid == 0) {
		/* child */
		જો (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			જાઓ child_err;
		જો (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			જાઓ child_err;
		જો (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			જાઓ child_err;
		જો (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			જાઓ child_err;
		જો (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			જાઓ child_err;
		જો (args->setsid && setsid() < 0)
			જાઓ child_err;
		sigprocmask(SIG_SETMASK, &old, ખાલી);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		લિખ(execpipe[1], &saved_errno, કદ saved_errno);
		_exit(1);
	} નહીંતર {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = પઠ(execpipe[0], &err, કદ err);
		જો (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ખાલી, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ખાલી);
		જો (res.pid < 0) {
			errno = (n == કદ err) ? err : EINVAL;
			જાઓ exit;
		}

		જો (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		જો (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		જો (stderr_pipe[0] >= 0) {
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
	પાછા res;
}

પૂર્ણાંક rc_waitpid(pid_t pid)
{
	પૂર્ણાંક status;
	જ્યારે (waitpid(pid, &status, 0) == -1) {
		જો (errno != EINTR) {
			status = -1;
			તોડો;
		}
	}
	પાછા status;
}

પૂર્ણાંક rc_pipe_command(અચલ અક્ષર *cmd, પૂર્ણાંક devnullfd)
{
	અચલ અક્ષર *argv[] = { "/bin/sh", "-c", cmd, ખાલી };
	સંરચના exec_result res;
	સંરચના exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	પાછા (res.pid > 0) ? res.proc_stdin : -1;
}
