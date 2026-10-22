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

ساکن خالی checked_close(صحیح fd)
{
	اگر (fd >= 0)
		close(fd);
}

ساکن صحیح devnull(خالی)
{
	ساکن صحیح devnullfd = -1;
	اگر (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	اگر (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	واپس devnullfd;
}

ساخت exec_دلائل exec_init(ثابت حرف **argv)
{
	ساخت exec_دلائل دلائل = {0};
	دلائل.argv = argv;
	دلائل.redirect_stdin  = EXEC_NO_REDIRECT;
	دلائل.redirect_stdout = EXEC_NO_REDIRECT;
	دلائل.redirect_stderr = EXEC_NO_REDIRECT;
	دلائل.uid = (uid_t)-1;
	دلائل.gid = (gid_t)-1;
	واپس دلائل;
}

ساخت exec_نتیجہ do_exec(ساخت exec_دلائل *دلائل)
{
	ساخت exec_نتیجہ res = { .pid = -1 };
	sigset_t بھرا, old;
	صحیح saved_errno, err, n;
	صحیح execpipe[2] = {-1, -1};
	صحیح stdin_pipe[2] = {-1, -1};
	صحیح stdout_pipe[2] = {-1, -1};
	صحیح stderr_pipe[2] = {-1, -1};
	ثابت حرف *cmd = دلائل->cmd ? دلائل->cmd : دلائل->argv[0];

	اگر (pipe2(execpipe, O_CLOEXEC) < 0)
		جاؤ باہر_نکلو;
	اگر (دلائل->redirect_stdin == EXEC_MKPIPE) {
		اگر (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			جاؤ باہر_نکلو;
		دلائل->redirect_stdin = stdin_pipe[0];
	}
	اگر (دلائل->redirect_stdout == EXEC_MKPIPE) {
		اگر (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			جاؤ باہر_نکلو;
		دلائل->redirect_stdout = stdout_pipe[1];
	}
	اگر (دلائل->redirect_stderr == EXEC_MKPIPE) {
		اگر (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			جاؤ باہر_نکلو;
		دلائل->redirect_stderr = stderr_pipe[1];
	}

	اگر (دلائل->redirect_stdin == EXEC_DEVNULL)
		دلائل->redirect_stdin = devnull();
	اگر (دلائل->redirect_stdout == EXEC_DEVNULL)
		دلائل->redirect_stdout = devnull();
	اگر (دلائل->redirect_stderr == EXEC_DEVNULL)
		دلائل->redirect_stderr = devnull();

	sigfillset(&بھرا);
	sigprocmask(SIG_SETMASK, &بھرا, &old);
	res.pid = fork();

	اگر (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, تہی);
		جاؤ باہر_نکلو;
	} ورنہ اگر (res.pid == 0) {
		/*بچہ*/
		اگر (دلائل->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(دلائل->redirect_stdin, STDIN_FILENO) < 0)
			جاؤ بچہ_err;
		اگر (دلائل->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(دلائل->redirect_stdout, STDOUT_FILENO) < 0)
			جاؤ بچہ_err;
		اگر (دلائل->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(دلائل->redirect_stderr, STDERR_FILENO) < 0)
			جاؤ بچہ_err;
		اگر (دلائل->gid != (gid_t)-1 && setgid(دلائل->gid) < 0)
			جاؤ بچہ_err;
		اگر (دلائل->uid != (uid_t)-1 && setuid(دلائل->uid) < 0)
			جاؤ بچہ_err;
		اگر (دلائل->setsid && setsid() < 0)
			جاؤ بچہ_err;
		sigprocmask(SIG_SETMASK, &old, تہی);
		execvp(cmd, UNCONST(دلائل->argv));
بچہ_err:
		saved_errno = errno;
		اكتب(execpipe[1], &saved_errno, حجم saved_errno);
		_exit(1);
	} ورنہ {
		/*والد*/
		close(execpipe[1]);
		execpipe[1] = -1;
		n = اقرأ(execpipe[0], &err, حجم err);
		اگر (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, تہی, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, تہی);
		اگر (res.pid < 0) {
			errno = (n == حجم err) ? err : EINVAL;
			جاؤ باہر_نکلو;
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

باہر_نکلو:
	saved_errno = errno;
	checked_close(execpipe[0]); checked_close(execpipe[1]);
	checked_close(stdin_pipe[0]); checked_close(stdin_pipe[1]);
	checked_close(stdout_pipe[0]); checked_close(stdout_pipe[1]);
	checked_close(stderr_pipe[0]); checked_close(stderr_pipe[1]);
	errno = saved_errno;
	واپس res;
}

صحیح rc_waitpid(pid_t pid)
{
	صحیح حالت_2;
	جب (waitpid(pid, &حالت_2, 0) == -1) {
		اگر (errno != EINTR) {
			حالت_2 = -1;
			توڑو;
		}
	}
	واپس حالت_2;
}

صحیح rc_pipe_command(ثابت حرف *cmd, صحیح devnullfd)
{
	ثابت حرف *argv[] = { "/bin/sh", "-c", cmd, تہی };
	ساخت exec_نتیجہ res;
	ساخت exec_دلائل دلائل = exec_init(argv);
	دلائل.redirect_stdin = EXEC_MKPIPE;
	دلائل.redirect_stdout = دلائل.redirect_stderr = devnullfd;
	res = do_exec(&دلائل);
	واپس (res.pid > 0) ? res.proc_stdin : -1;
}
