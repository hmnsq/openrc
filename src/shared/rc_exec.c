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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "rc_exec.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ସ୍ଥିର ଶୂନ୍ୟ checked_close(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ଯଦି (fd >= 0)
		close(fd);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnull(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnullfd = -1;
	ଯଦି (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ଯଦି (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ଫେରନ୍ତୁ devnullfd;
}

ସଂରଚନା exec_args exec_init(ଅଚଳ ଅକ୍ଷର **argv)
{
	ସଂରଚନା exec_args args = {0};
	args.argv = argv;
	args.redirect_stdin  = EXEC_NO_REDIRECT;
	args.redirect_stdout = EXEC_NO_REDIRECT;
	args.redirect_stderr = EXEC_NO_REDIRECT;
	args.uid = (uid_t)-1;
	args.gid = (gid_t)-1;
	ଫେରନ୍ତୁ args;
}

ସଂରଚନା exec_result do_exec(ସଂରଚନା exec_args *args)
{
	ସଂରଚନା exec_result res = { .pid = -1 };
	sigset_t full, old;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା saved_errno, err, n;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା execpipe[2] = {-1, -1};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdin_pipe[2] = {-1, -1};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stdout_pipe[2] = {-1, -1};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stderr_pipe[2] = {-1, -1};
	ଅଚଳ ଅକ୍ଷର *cmd = args->cmd ? args->cmd : args->argv[0];

	ଯଦି (pipe2(execpipe, O_CLOEXEC) < 0)
		ଯାଅ exit;
	ଯଦି (args->redirect_stdin == EXEC_MKPIPE) {
		ଯଦି (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			ଯାଅ exit;
		args->redirect_stdin = stdin_pipe[0];
	}
	ଯଦି (args->redirect_stdout == EXEC_MKPIPE) {
		ଯଦି (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			ଯାଅ exit;
		args->redirect_stdout = stdout_pipe[1];
	}
	ଯଦି (args->redirect_stderr == EXEC_MKPIPE) {
		ଯଦି (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			ଯାଅ exit;
		args->redirect_stderr = stderr_pipe[1];
	}

	ଯଦି (args->redirect_stdin == EXEC_DEVNULL)
		args->redirect_stdin = devnull();
	ଯଦି (args->redirect_stdout == EXEC_DEVNULL)
		args->redirect_stdout = devnull();
	ଯଦି (args->redirect_stderr == EXEC_DEVNULL)
		args->redirect_stderr = devnull();

	sigfillset(&full);
	sigprocmask(SIG_SETMASK, &full, &old);
	res.pid = fork();

	ଯଦି (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ଖାଲି);
		ଯାଅ exit;
	} ଅନ୍ୟଥା ଯଦି (res.pid == 0) {
		/* child */
		ଯଦି (args->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdin, STDIN_FILENO) < 0)
			ଯାଅ child_err;
		ଯଦି (args->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stdout, STDOUT_FILENO) < 0)
			ଯାଅ child_err;
		ଯଦି (args->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(args->redirect_stderr, STDERR_FILENO) < 0)
			ଯାଅ child_err;
		ଯଦି (args->gid != (gid_t)-1 && setgid(args->gid) < 0)
			ଯାଅ child_err;
		ଯଦି (args->uid != (uid_t)-1 && setuid(args->uid) < 0)
			ଯାଅ child_err;
		ଯଦି (args->setsid && setsid() < 0)
			ଯାଅ child_err;
		sigprocmask(SIG_SETMASK, &old, ଖାଲି);
		execvp(cmd, UNCONST(args->argv));
child_err:
		saved_errno = errno;
		ଲିଖ(execpipe[1], &saved_errno, ଆକାର saved_errno);
		_exit(1);
	} ଅନ୍ୟଥା {
		/* parent */
		close(execpipe[1]);
		execpipe[1] = -1;
		n = ପଠ(execpipe[0], &err, ଆକାର err);
		ଯଦି (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ଖାଲି, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ଖାଲି);
		ଯଦି (res.pid < 0) {
			errno = (n == ଆକାର err) ? err : EINVAL;
			ଯାଅ exit;
		}

		ଯଦି (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ଯଦି (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ଯଦି (stderr_pipe[0] >= 0) {
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
	ଫେରନ୍ତୁ res;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc_waitpid(pid_t pid)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	ଯେତେବେଳେ (waitpid(pid, &status, 0) == -1) {
		ଯଦି (errno != EINTR) {
			status = -1;
			ଭାଙ୍ଗ;
		}
	}
	ଫେରନ୍ତୁ status;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc_pipe_command(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnullfd)
{
	ଅଚଳ ଅକ୍ଷର *argv[] = { "/bin/sh", "-c", cmd, ଖାଲି };
	ସଂରଚନା exec_result res;
	ସଂରଚନା exec_args args = exec_init(argv);
	args.redirect_stdin = EXEC_MKPIPE;
	args.redirect_stdout = args.redirect_stderr = devnullfd;
	res = do_exec(&args);
	ଫେରନ୍ତୁ (res.pid > 0) ? res.proc_stdin : -1;
}
