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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "rc_exec.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਸਥਿਰ ਖਾਲੀ checked_close(ਪੂਰਨ fd)
{
	ਜੇ (fd >= 0)
		close(fd);
}

ਸਥਿਰ ਪੂਰਨ devnull(ਖਾਲੀ)
{
	ਸਥਿਰ ਪੂਰਨ devnullfd = -1;
	ਜੇ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ਜੇ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ਵਾਪਸ devnullfd;
}

ਬਣਤਰ exec_ਦਲੀਲਾਂ exec_init(ਅਚਲ ਅੱਖਰ **argv)
{
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ = {0};
	ਦਲੀਲਾਂ.argv = argv;
	ਦਲੀਲਾਂ.redirect_stdin  = EXEC_NO_REDIRECT;
	ਦਲੀਲਾਂ.redirect_stdout = EXEC_NO_REDIRECT;
	ਦਲੀਲਾਂ.redirect_stderr = EXEC_NO_REDIRECT;
	ਦਲੀਲਾਂ.uid = (uid_t)-1;
	ਦਲੀਲਾਂ.gid = (gid_t)-1;
	ਵਾਪਸ ਦਲੀਲਾਂ;
}

ਬਣਤਰ exec_ਨਤੀਜਾ do_exec(ਬਣਤਰ exec_ਦਲੀਲਾਂ *ਦਲੀਲਾਂ)
{
	ਬਣਤਰ exec_ਨਤੀਜਾ res = { .pid = -1 };
	sigset_t ਭਰਿਆ, old;
	ਪੂਰਨ saved_errno, err, n;
	ਪੂਰਨ execpipe[2] = {-1, -1};
	ਪੂਰਨ stdin_pipe[2] = {-1, -1};
	ਪੂਰਨ stdout_pipe[2] = {-1, -1};
	ਪੂਰਨ stderr_pipe[2] = {-1, -1};
	ਅਚਲ ਅੱਖਰ *cmd = ਦਲੀਲਾਂ->cmd ? ਦਲੀਲਾਂ->cmd : ਦਲੀਲਾਂ->argv[0];

	ਜੇ (pipe2(execpipe, O_CLOEXEC) < 0)
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	ਜੇ (ਦਲੀਲਾਂ->redirect_stdin == EXEC_MKPIPE) {
		ਜੇ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
		ਦਲੀਲਾਂ->redirect_stdin = stdin_pipe[0];
	}
	ਜੇ (ਦਲੀਲਾਂ->redirect_stdout == EXEC_MKPIPE) {
		ਜੇ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
		ਦਲੀਲਾਂ->redirect_stdout = stdout_pipe[1];
	}
	ਜੇ (ਦਲੀਲਾਂ->redirect_stderr == EXEC_MKPIPE) {
		ਜੇ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
		ਦਲੀਲਾਂ->redirect_stderr = stderr_pipe[1];
	}

	ਜੇ (ਦਲੀਲਾਂ->redirect_stdin == EXEC_DEVNULL)
		ਦਲੀਲਾਂ->redirect_stdin = devnull();
	ਜੇ (ਦਲੀਲਾਂ->redirect_stdout == EXEC_DEVNULL)
		ਦਲੀਲਾਂ->redirect_stdout = devnull();
	ਜੇ (ਦਲੀਲਾਂ->redirect_stderr == EXEC_DEVNULL)
		ਦਲੀਲਾਂ->redirect_stderr = devnull();

	sigfillset(&ਭਰਿਆ);
	sigprocmask(SIG_SETMASK, &ਭਰਿਆ, &old);
	res.pid = fork();

	ਜੇ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
		ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
	} ਨਹੀਂਤਾਂ ਜੇ (res.pid == 0) {
		/*ਬੱਚਾ*/
		ਜੇ (ਦਲੀਲਾਂ->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(ਦਲੀਲਾਂ->redirect_stdin, STDIN_FILENO) < 0)
			ਜਾਓ ਬੱਚਾ_err;
		ਜੇ (ਦਲੀਲਾਂ->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(ਦਲੀਲਾਂ->redirect_stdout, STDOUT_FILENO) < 0)
			ਜਾਓ ਬੱਚਾ_err;
		ਜੇ (ਦਲੀਲਾਂ->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(ਦਲੀਲਾਂ->redirect_stderr, STDERR_FILENO) < 0)
			ਜਾਓ ਬੱਚਾ_err;
		ਜੇ (ਦਲੀਲਾਂ->gid != (gid_t)-1 && setgid(ਦਲੀਲਾਂ->gid) < 0)
			ਜਾਓ ਬੱਚਾ_err;
		ਜੇ (ਦਲੀਲਾਂ->uid != (uid_t)-1 && setuid(ਦਲੀਲਾਂ->uid) < 0)
			ਜਾਓ ਬੱਚਾ_err;
		ਜੇ (ਦਲੀਲਾਂ->setsid && setsid() < 0)
			ਜਾਓ ਬੱਚਾ_err;
		sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
		execvp(cmd, UNCONST(ਦਲੀਲਾਂ->argv));
ਬੱਚਾ_err:
		saved_errno = errno;
		ਲਿਖ(execpipe[1], &saved_errno, ਆਕਾਰ saved_errno);
		_exit(1);
	} ਨਹੀਂਤਾਂ {
		/*ਮਾਪਾ*/
		close(execpipe[1]);
		execpipe[1] = -1;
		n = ਪਠ(execpipe[0], &err, ਆਕਾਰ err);
		ਜੇ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ਨਿਰਾ, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ਨਿਰਾ);
		ਜੇ (res.pid < 0) {
			errno = (n == ਆਕਾਰ err) ? err : EINVAL;
			ਜਾਓ ਬਾਹਰ_ਨਿਕਲੋ;
		}

		ਜੇ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ਜੇ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ਜੇ (stderr_pipe[0] >= 0) {
			res.proc_stderr = stderr_pipe[0];
			stderr_pipe[0] = -1;
		}
	}

ਬਾਹਰ_ਨਿਕਲੋ:
	saved_errno = errno;
	checked_close(execpipe[0]); checked_close(execpipe[1]);
	checked_close(stdin_pipe[0]); checked_close(stdin_pipe[1]);
	checked_close(stdout_pipe[0]); checked_close(stdout_pipe[1]);
	checked_close(stderr_pipe[0]); checked_close(stderr_pipe[1]);
	errno = saved_errno;
	ਵਾਪਸ res;
}

ਪੂਰਨ rc_waitpid(pid_t pid)
{
	ਪੂਰਨ ਸਥਿਤੀ;
	ਜਦੋਂ (waitpid(pid, &ਸਥਿਤੀ, 0) == -1) {
		ਜੇ (errno != EINTR) {
			ਸਥਿਤੀ = -1;
			ਰੋਕੋ;
		}
	}
	ਵਾਪਸ ਸਥਿਤੀ;
}

ਪੂਰਨ rc_pipe_command(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ devnullfd)
{
	ਅਚਲ ਅੱਖਰ *argv[] = { "/bin/sh", "-c", cmd, ਨਿਰਾ };
	ਬਣਤਰ exec_ਨਤੀਜਾ res;
	ਬਣਤਰ exec_ਦਲੀਲਾਂ ਦਲੀਲਾਂ = exec_init(argv);
	ਦਲੀਲਾਂ.redirect_stdin = EXEC_MKPIPE;
	ਦਲੀਲਾਂ.redirect_stdout = ਦਲੀਲਾਂ.redirect_stderr = devnullfd;
	res = do_exec(&ਦਲੀਲਾਂ);
	ਵਾਪਸ (res.pid > 0) ? res.proc_stdin : -1;
}
