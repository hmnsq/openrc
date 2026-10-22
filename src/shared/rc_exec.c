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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "rc_exec.h"
#অন্তর্ভাব "helpers.h"

স্থির শূন্য checked_close(পূর্ণসংখ্যা fd)
{
	যদি (fd >= 0)
		close(fd);
}

স্থির পূর্ণসংখ্যা devnull(শূন্য)
{
	স্থির পূর্ণসংখ্যা devnullfd = -1;
	যদি (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	যদি (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ফেরত devnullfd;
}

কাঠামো exec_args exec_init(ধ্রুবক অক্ষর **argv)
{
	কাঠামো exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	ফেরত args;
}

কাঠামো exec_ফলাফল do_exec(কাঠামো exec_args *args)
{
	কাঠামো exec_ফলাফল res = { .pid = -1 };
	sigset_t full, old;
	পূর্ণসংখ্যা saved_errno, err, n;
	পূর্ণসংখ্যা execpipe[2] = {-1, -1};
	পূর্ণসংখ্যা stdin_pipe[2] = {-1, -1};
	পূর্ণসংখ্যা stdout_pipe[2] = {-1, -1};
	পূর্ণসংখ্যা stderr_pipe[2] = {-1, -1};
	ধ্রুবক অক্ষর *cmd = args->cmd ? args->cmd : args->argv[0];

	যদি (pipe2(execpipe, O_CLOEXEC) < 0)
		যাও exit;
	যদি (args->redirect_stdin == EXEC_MKPIPE) {
		যদি (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			যাও exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	যদি (args->redirect_stdout == EXEC_MKPIPE) {
		যদি (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			যাও exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	যদি (args->redirect_stderr == EXEC_MKPIPE) {
		যদি (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			যাও exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	যদি (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	যদি (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	যদি (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	যদি (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ফাঁকা);
		যাও exit;
	} অন্যথা যদি (res.pid == 0) {
		/* child */
		যদি (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			যাও child_err;
		যদি (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			যাও child_err;
		যদি (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			যাও child_err;
		যদি (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			যাও child_err;
		যদি (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			যাও child_err;
		যদি (args->setsid && setsid() < 0)
			যাও child_err;
		sigprocmask(SIG_SETMASK, &old, ফাঁকা);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		লিখ(execpipe[1], &saved_errno, আকার saved_errno);
		_exit(1);
	} অন্যথা {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = পঠ(execpipe[0], &err, আকার err);
		যদি (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ফাঁকা, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ফাঁকা);
		যদি (res.pid < 0) {
			errno = (n == আকার err) ? err : EINVAL;
			যাও exit;
		}

		যদি (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		যদি (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		যদি (stderr_pipe[0] >= 0) {
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
	ফেরত res;
}

পূর্ণসংখ্যা rc_waitpid(pid_t pid)
{
	পূর্ণসংখ্যা status;
	যখন (waitpid(pid, &status, 0) == -1) {
		যদি (errno != EINTR) {
			status = -1;
			বিরতি;
		}
	}
	ফেরত status;
}

পূর্ণসংখ্যা rc_pipe_command(ধ্রুবক অক্ষর *cmd, পূর্ণসংখ্যা devnullfd)
{
	ধ্রুবক অক্ষর *argv[] = { "/bin/sh", "-c", cmd, ফাঁকা };
	কাঠামো exec_ফলাফল res;
	কাঠামো exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	ফেরত (res.pid > 0) ? res.proc_stdin : -1;
}
