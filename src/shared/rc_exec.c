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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "rc_exec.h"
#अन्तर्भाव "helpers.h"

स्थिर शून्य checked_close(पूर्णाङ्क fd)
{
	यदि (fd >= 0)
		close(fd);
}

स्थिर पूर्णाङ्क devnull(शून्य)
{
	स्थिर पूर्णाङ्क devnullfd = -1;
	यदि (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	यदि (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	प्रत्यागम devnullfd;
}

संरचना exec_args exec_init(ध्रुव वर्ण **argv)
{
	संरचना exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	प्रत्यागम args;
}

संरचना exec_result do_exec(संरचना exec_args *args)
{
	संरचना exec_result res = { .pid = -1 };
	sigset_t full, old;
	पूर्णाङ्क saved_errno, err, n;
	पूर्णाङ्क execpipe[2] = {-1, -1};
	पूर्णाङ्क stdin_pipe[2] = {-1, -1};
	पूर्णाङ्क stdout_pipe[2] = {-1, -1};
	पूर्णाङ्क stderr_pipe[2] = {-1, -1};
	ध्रुव वर्ण *cmd = args->cmd ? args->cmd : args->argv[0];

	यदि (pipe2(execpipe, O_CLOEXEC) < 0)
		गच्छ exit;
	यदि (args->redirect_stdin == EXEC_MKPIPE) {
		यदि (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			गच्छ exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	यदि (args->redirect_stdout == EXEC_MKPIPE) {
		यदि (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			गच्छ exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	यदि (args->redirect_stderr == EXEC_MKPIPE) {
		यदि (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			गच्छ exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	यदि (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	यदि (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	यदि (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	यदि (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		गच्छ exit;
	} अन्यथा यदि (res.pid == 0) {
		/* child */
		यदि (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			गच्छ child_err;
		यदि (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			गच्छ child_err;
		यदि (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			गच्छ child_err;
		यदि (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			गच्छ child_err;
		यदि (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			गच्छ child_err;
		यदि (args->setsid && setsid() < 0)
			गच्छ child_err;
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		लिख(execpipe[1], &saved_errno, परिमाण saved_errno);
		_exit(1);
	} अन्यथा {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = पठ(execpipe[0], &err, परिमाण err);
		यदि (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, रिक्त, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		यदि (res.pid < 0) {
			errno = (n == परिमाण err) ? err : EINVAL;
			गच्छ exit;
		}

		यदि (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		यदि (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		यदि (stderr_pipe[0] >= 0) {
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
	प्रत्यागम res;
}

पूर्णाङ्क rc_waitpid(pid_t pid)
{
	पूर्णाङ्क status;
	यावत् (waitpid(pid, &status, 0) == -1) {
		यदि (errno != EINTR) {
			status = -1;
			भङ्ग;
		}
	}
	प्रत्यागम status;
}

पूर्णाङ्क rc_pipe_command(ध्रुव वर्ण *cmd, पूर्णाङ्क devnullfd)
{
	ध्रुव वर्ण *argv[] = { "/bin/sh", "-c", cmd, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	प्रत्यागम (res.pid > 0) ? res.proc_stdin : -1;
}
