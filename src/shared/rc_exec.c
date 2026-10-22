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

स्थिर शून्य checked_close(पूर्णांक fd)
{
	जर (fd >= 0)
		बंद(fd);
}

स्थिर पूर्णांक devnull(शून्य)
{
	स्थिर पूर्णांक devnullfd = -1;
	जर (devnullfd < 0)
		devnullfd = खोलो("/dev/null", O_RDWR | O_CLOEXEC);
	जर (devnullfd < 0) {
		ख_लिखो(मानक_त्रुटि, "Failed to open /dev/null: %s", म_त्रुटि(त्रुटि_सं));
		_exit(1);
	}
	परत devnullfd;
}

संरचना exec_args exec_init(स्थिरांक अक्षर **argv)
{
	संरचना exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	परत args;
}

संरचना exec_result do_exec(संरचना exec_args *args)
{
	संरचना exec_result res = { .pid = -1 };
	sigset_t full, old;
	पूर्णांक saved_errno, err, n;
	पूर्णांक execpipe[2] = {-1, -1};
	पूर्णांक stdin_pipe[2] = {-1, -1};
	पूर्णांक stdout_pipe[2] = {-1, -1};
	पूर्णांक stderr_pipe[2] = {-1, -1};
	स्थिरांक अक्षर *cmd = args->cmd ? args->cmd : args->argv[0];

	जर (pipe2(execpipe, O_CLOEXEC) < 0)
		जा निकास;
	जर (args->redirect_stdin == EXEC_MKPIPE) {
		जर (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			जा निकास;
		args->redirect_stdin = stdin_pipe[0];
	}
	जर (args->redirect_stdout == EXEC_MKPIPE) {
		जर (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			जा निकास;
		args->redirect_stdout = stdout_pipe[1];
	}
	जर (args->redirect_stderr == EXEC_MKPIPE) {
		जर (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			जा निकास;
		args->redirect_stderr = stderr_pipe[1];
	}

	जर (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	जर (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	जर (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = विभाजन();

	जर (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		जा निकास;
	} अन्यथा जर (res.pid == 0) {
		/* child */
		जर (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			जा child_err;
		जर (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			जा child_err;
		जर (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			जा child_err;
		जर (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			जा child_err;
		जर (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			जा child_err;
		जर (args->setsid && setsid() < 0)
			जा child_err;
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = त्रुटि_सं;
		लिख(execpipe[1], &saved_errno, आकार saved_errno);
		_exit(1);
	} अन्यथा {
		/* parent */
		बंद(execpipe[1]);
		execpipe[1] = -1;
		n = पठ(execpipe[0], &err, आकार err);
		जर (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, रिक्त, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, रिक्त);
		जर (res.pid < 0) {
			त्रुटि_सं = (n == आकार err) ? err : EINVAL;
			जा निकास;
		}

		जर (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		जर (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		जर (stderr_pipe[0] >= 0) {
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
	परत res;
}

पूर्णांक rc_waitpid(pid_t pid)
{
	पूर्णांक status;
	पर्यंत (waitpid(pid, &status, 0) == -1) {
		जर (त्रुटि_सं != EINTR) {
			status = -1;
			थांबा;
		}
	}
	परत status;
}

पूर्णांक rc_pipe_command(स्थिरांक अक्षर *cmd, पूर्णांक devnullfd)
{
	स्थिरांक अक्षर *argv[] = { "/bin/sh", "-c", cmd, रिक्त };
	संरचना exec_result res;
	संरचना exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	परत (res.pid > 0) ? res.proc_stdin : -1;
}
