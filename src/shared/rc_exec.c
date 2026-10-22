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
		devnullfd = उद्घाटय("/dev/null", O_RDWR | O_CLOEXEC);
	यदि (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	प्रत्यागम devnullfd;
}

संरचना exec_तर्काः exec_init(ध्रुव वर्ण **argv)
{
	संरचना exec_तर्काः तर्काः = {0};
	तर्काः.argv = argv;
	तर्काः.redirect_stdin  = EXEC_NO_REDIRECT;
	तर्काः.redirect_stdout = EXEC_NO_REDIRECT;
	तर्काः.redirect_stderr = EXEC_NO_REDIRECT;
	तर्काः.uid = (uid_t)-1;
	तर्काः.gid = (gid_t)-1;
	प्रत्यागम तर्काः;
}

संरचना exec_परिणाम do_exec(संरचना exec_तर्काः *तर्काः)
{
	संरचना exec_परिणाम res = { .pid = -1 };
	sigset_t पूर्णम्, old;
	पूर्णाङ्क saved_errno, err, n;
	पूर्णाङ्क execpipe[2] = {-1, -1};
	पूर्णाङ्क stdin_pipe[2] = {-1, -1};
	पूर्णाङ्क stdout_pipe[2] = {-1, -1};
	पूर्णाङ्क stderr_pipe[2] = {-1, -1};
	ध्रुव वर्ण *cmd = तर्काः->cmd ? तर्काः->cmd : तर्काः->argv[0];

	यदि (pipe2(execpipe, O_CLOEXEC) < 0)
		गच्छ निर्गच्छ;
	यदि (तर्काः->redirect_stdin == EXEC_MKPIPE) {
		यदि (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			गच्छ निर्गच्छ;
		तर्काः->redirect_stdin = stdin_pipe[0];
	}
	यदि (तर्काः->redirect_stdout == EXEC_MKPIPE) {
		यदि (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			गच्छ निर्गच्छ;
		तर्काः->redirect_stdout = stdout_pipe[1];
	}
	यदि (तर्काः->redirect_stderr == EXEC_MKPIPE) {
		यदि (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			गच्छ निर्गच्छ;
		तर्काः->redirect_stderr = stderr_pipe[1];
	}

	यदि (तर्काः->redirect_stdin == EXEC_DEVNULL)
		तर्काः->redirect_stdin = devnull();
	यदि (तर्काः->redirect_stdout == EXEC_DEVNULL)
		तर्काः->redirect_stdout = devnull();
	यदि (तर्काः->redirect_stderr == EXEC_DEVNULL)
		तर्काः->redirect_stderr = devnull();

	sigfillset(&पूर्णम्);
	sigprocmask(SIG_SETMASK, &पूर्णम्, &old);
	res.pid = fork();

	यदि (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		गच्छ निर्गच्छ;
	} अन्यथा यदि (res.pid == 0) {
		/* child */
		यदि (तर्काः->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(तर्काः->redirect_stdin, STDIN_FILENO) < 0)
			गच्छ child_err;
		यदि (तर्काः->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(तर्काः->redirect_stdout, STDOUT_FILENO) < 0)
			गच्छ child_err;
		यदि (तर्काः->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(तर्काः->redirect_stderr, STDERR_FILENO) < 0)
			गच्छ child_err;
		यदि (तर्काः->gid != (gid_t)-1 && setgid(तर्काः->gid) < 0)
			गच्छ child_err;
		यदि (तर्काः->uid != (uid_t)-1 && setuid(तर्काः->uid) < 0)
			गच्छ child_err;
		यदि (तर्काः->setsid && setsid() < 0)
			गच्छ child_err;
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		execvp(cmd, UNCONST(तर्काः->argv));
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
			गच्छ निर्गच्छ;
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

निर्गच्छ:
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
	पूर्णाङ्क स्थितिः;
	यावत् (waitpid(pid, &स्थितिः, 0) == -1) {
		यदि (errno != EINTR) {
			स्थितिः = -1;
			भङ्ग;
		}
	}
	प्रत्यागम स्थितिः;
}

पूर्णाङ्क rc_pipe_command(ध्रुव वर्ण *cmd, पूर्णाङ्क devnullfd)
{
	ध्रुव वर्ण *argv[] = { "/bin/sh", "-c", cmd, रिक्त };
	संरचना exec_परिणाम res;
	संरचना exec_तर्काः तर्काः = exec_init(argv);
	तर्काः.redirect_stdin = EXEC_MKPIPE;
	तर्काः.redirect_stdout = तर्काः.redirect_stderr = devnullfd;
	res = do_exec(&तर्काः);
	प्रत्यागम (res.pid > 0) ? res.proc_stdin : -1;
}
