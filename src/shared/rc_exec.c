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
	सचे (fd >= 0)
		close(fd);
}

स्थिर पूर्णाङ्क devnull(शून्य)
{
	स्थिर पूर्णाङ्क devnullfd = -1;
	सचे (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	सचे (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	पटिनिवत्त devnullfd;
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
	पटिनिवत्त args;
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

	सचे (pipe2(execpipe, O_CLOEXEC) < 0)
		गच्छ exit;
	सचे (args->redirect_stdin == EXEC_MKPIPE) {
		सचे (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			गच्छ exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	सचे (args->redirect_stdout == EXEC_MKPIPE) {
		सचे (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			गच्छ exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	सचे (args->redirect_stderr == EXEC_MKPIPE) {
		सचे (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			गच्छ exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	सचे (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	सचे (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	सचे (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	सचे (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		गच्छ exit;
	} अञ्ञथा सचे (res.pid == 0) {
		/* child */
		सचे (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			गच्छ child_err;
		सचे (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			गच्छ child_err;
		सचे (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			गच्छ child_err;
		सचे (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			गच्छ child_err;
		सचे (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			गच्छ child_err;
		सचे (args->setsid && setsid() < 0)
			गच्छ child_err;
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		लिख(execpipe[1], &saved_errno, परिमाण saved_errno);
		_exit(1);
	} अञ्ञथा {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = पठ(execpipe[0], &err, परिमाण err);
		सचे (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, रिक्त, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		सचे (res.pid < 0) {
			errno = (n == परिमाण err) ? err : EINVAL;
			गच्छ exit;
		}

		सचे (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		सचे (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		सचे (stderr_pipe[0] >= 0) {
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
	पटिनिवत्त res;
}

पूर्णाङ्क rc_waitpid(pid_t pid)
{
	पूर्णाङ्क status;
	याव (waitpid(pid, &status, 0) == -1) {
		सचे (errno != EINTR) {
			status = -1;
			भङ्ग;
		}
	}
	पटिनिवत्त status;
}

पूर्णाङ्क rc_pipe_command(ध्रुव वर्ण *cmd, पूर्णाङ्क devnullfd)
{
	ध्रुव वर्ण *argv[] = { "/bin/sh", "-c", cmd, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	पटिनिवत्त (res.pid > 0) ? res.proc_stdin : -1;
}
