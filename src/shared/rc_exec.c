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

#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <signal.h>
#समावेश <sys/wait.h>
#समावेश <unistd.h>

#समावेश "rc_exec.h"
#समावेश "helpers.h"

अटल व्योम checked_close(पूर्णांक fd)
{
	अगर (fd >= 0)
		बंद(fd);
}

अटल पूर्णांक devnull(व्योम)
{
	अटल पूर्णांक devnullfd = -1;
	अगर (devnullfd < 0)
		devnullfd = खोलो("/dev/null", O_RDWR | O_CLOEXEC);
	अगर (devnullfd < 0) {
		ख_लिखो(मानक_त्रुटि, "Failed to open /dev/null: %s", म_त्रुटि(त्रुटि_सं));
		_निकास(1);
	}
	वापस devnullfd;
}

काष्ठा exec_args exec_init(स्थिर अक्षर **argv)
{
	काष्ठा exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	वापस args;
}

काष्ठा exec_परिणाम do_exec(काष्ठा exec_args *args)
{
	काष्ठा exec_परिणाम res = { .pid = -1 };
	sigset_t full, old;
	पूर्णांक saved_errno, err, n;
	पूर्णांक execpipe[2] = {-1, -1};
	पूर्णांक stdin_pipe[2] = {-1, -1};
	पूर्णांक stdout_pipe[2] = {-1, -1};
	पूर्णांक stderr_pipe[2] = {-1, -1};
	स्थिर अक्षर *cmd = args->cmd ? args->cmd : args->argv[0];

	अगर (pipe2(execpipe, O_CLOEXEC) < 0)
		जाओ निकास;
	अगर (args->redirect_stdin == EXEC_MKPIPE) {
		अगर (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			जाओ निकास;
		args->redirect_stdin = stdin_pipe[0];
	}
	अगर (args->redirect_stdout == EXEC_MKPIPE) {
		अगर (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			जाओ निकास;
		args->redirect_stdout = stdout_pipe[1];
	}
	अगर (args->redirect_stderr == EXEC_MKPIPE) {
		अगर (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			जाओ निकास;
		args->redirect_stderr = stderr_pipe[1];
	}

	अगर (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	अगर (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	अगर (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = विभाजन();

	अगर (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, शून्य);
		जाओ निकास;
	} अन्यथा अगर (res.pid == 0) {
		/* child */
		अगर (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			जाओ child_err;
		अगर (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			जाओ child_err;
		अगर (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			जाओ child_err;
		अगर (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			जाओ child_err;
		अगर (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			जाओ child_err;
		अगर (args->setsid && setsid() < 0)
			जाओ child_err;
		sigprocmask(SIG_SETMASK, &old, शून्य);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = त्रुटि_सं;
		ग_लिखो(execpipe[1], &saved_errno, माप saved_errno);
		_निकास(1);
	} अन्यथा {
		/* parent */
		बंद(execpipe[1]);
		execpipe[1] = -1;
		n = पढ़ो(execpipe[0], &err, माप err);
		अगर (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, शून्य, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, शून्य);
		अगर (res.pid < 0) {
			त्रुटि_सं = (n == माप err) ? err : EINVAL;
			जाओ निकास;
		}

		अगर (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		अगर (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		अगर (stderr_pipe[0] >= 0) {
			res.proc_stderr = stderr_pipe[0];
			stderr_pipe[0] = -1;
		}
	}

निकास:
	saved_errno = त्रुटि_सं;
	checked_close(execpipe[0]); checked_close(execpipe[1]);
	checked_close(stdin_pipe[0]); checked_close(stdin_pipe[1]);
	checked_close(stdout_pipe[0]); checked_close(stdout_pipe[1]);
	checked_close(stderr_pipe[0]); checked_close(stderr_pipe[1]);
	त्रुटि_सं = saved_errno;
	वापस res;
}

पूर्णांक rc_waitpid(pid_t pid)
{
	पूर्णांक status;
	जबतक (waitpid(pid, &status, 0) == -1) {
		अगर (त्रुटि_सं != EINTR) {
			status = -1;
			अवरोध;
		}
	}
	वापस status;
}

पूर्णांक rc_pipe_command(स्थिर अक्षर *cmd, पूर्णांक devnullfd)
{
	स्थिर अक्षर *argv[] = { "/bin/sh", "-c", cmd, शून्य };
	काष्ठा exec_परिणाम res;
	काष्ठा exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	वापस (res.pid > 0) ? res.proc_stdin : -1;
}
