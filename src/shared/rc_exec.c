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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "rc_exec.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರ ಶೂನ್ಯ checked_close(ಪೂರ್ಣಾಂಕ fd)
{
	ಆದರೆ (fd >= 0)
		close(fd);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devnull(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ devnullfd = -1;
	ಆದರೆ (devnullfd < 0)
		devnullfd = open("/dev/null", O_RDWR | O_CLOEXEC);
	ಆದರೆ (devnullfd < 0) {
		fprintf(stderr, "Failed to open /dev/null: %s", strerror(errno));
		_exit(1);
	}
	ಹಿಂತಿರುಗಿ devnullfd;
}

ರಚನೆ exec_ವಾದಗಳು exec_init(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv)
{
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು = {0};
	ವಾದಗಳು.argv = argv;
	ವಾದಗಳು.redirect_stdin  = EXEC_NO_REDIRECT;
	ವಾದಗಳು.redirect_stdout = EXEC_NO_REDIRECT;
	ವಾದಗಳು.redirect_stderr = EXEC_NO_REDIRECT;
	ವಾದಗಳು.uid = (uid_t)-1;
	ವಾದಗಳು.gid = (gid_t)-1;
	ಹಿಂತಿರುಗಿ ವಾದಗಳು;
}

ರಚನೆ exec_ಫಲಿತಾಂಶ do_exec(ರಚನೆ exec_ವಾದಗಳು *ವಾದಗಳು)
{
	ರಚನೆ exec_ಫಲಿತಾಂಶ res = { .pid = -1 };
	sigset_t ತುಂಬಿದ, old;
	ಪೂರ್ಣಾಂಕ saved_errno, err, n;
	ಪೂರ್ಣಾಂಕ execpipe[2] = {-1, -1};
	ಪೂರ್ಣಾಂಕ stdin_pipe[2] = {-1, -1};
	ಪೂರ್ಣಾಂಕ stdout_pipe[2] = {-1, -1};
	ಪೂರ್ಣಾಂಕ stderr_pipe[2] = {-1, -1};
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd = ವಾದಗಳು->cmd ? ವಾದಗಳು->cmd : ವಾದಗಳು->argv[0];

	ಆದರೆ (pipe2(execpipe, O_CLOEXEC) < 0)
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	ಆದರೆ (ವಾದಗಳು->redirect_stdin == EXEC_MKPIPE) {
		ಆದರೆ (pipe2(stdin_pipe, O_CLOEXEC) < 0)
			ಹೋಗು ನಿರ್ಗಮಿಸು;
		ವಾದಗಳು->redirect_stdin = stdin_pipe[0];
	}
	ಆದರೆ (ವಾದಗಳು->redirect_stdout == EXEC_MKPIPE) {
		ಆದರೆ (pipe2(stdout_pipe, O_CLOEXEC) < 0)
			ಹೋಗು ನಿರ್ಗಮಿಸು;
		ವಾದಗಳು->redirect_stdout = stdout_pipe[1];
	}
	ಆದರೆ (ವಾದಗಳು->redirect_stderr == EXEC_MKPIPE) {
		ಆದರೆ (pipe2(stderr_pipe, O_CLOEXEC) < 0)
			ಹೋಗು ನಿರ್ಗಮಿಸು;
		ವಾದಗಳು->redirect_stderr = stderr_pipe[1];
	}

	ಆದರೆ (ವಾದಗಳು->redirect_stdin == EXEC_DEVNULL)
		ವಾದಗಳು->redirect_stdin = devnull();
	ಆದರೆ (ವಾದಗಳು->redirect_stdout == EXEC_DEVNULL)
		ವಾದಗಳು->redirect_stdout = devnull();
	ಆದರೆ (ವಾದಗಳು->redirect_stderr == EXEC_DEVNULL)
		ವಾದಗಳು->redirect_stderr = devnull();

	sigfillset(&ತುಂಬಿದ);
	sigprocmask(SIG_SETMASK, &ತುಂಬಿದ, &old);
	res.pid = fork();

	ಆದರೆ (res.pid < 0) {
		sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
		ಹೋಗು ನಿರ್ಗಮಿಸು;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (res.pid == 0) {
		/*ಮಗು*/
		ಆದರೆ (ವಾದಗಳು->redirect_stdin != EXEC_NO_REDIRECT &&
		    dup2(ವಾದಗಳು->redirect_stdin, STDIN_FILENO) < 0)
			ಹೋಗು ಮಗು_err;
		ಆದರೆ (ವಾದಗಳು->redirect_stdout != EXEC_NO_REDIRECT &&
		    dup2(ವಾದಗಳು->redirect_stdout, STDOUT_FILENO) < 0)
			ಹೋಗು ಮಗು_err;
		ಆದರೆ (ವಾದಗಳು->redirect_stderr != EXEC_NO_REDIRECT &&
		    dup2(ವಾದಗಳು->redirect_stderr, STDERR_FILENO) < 0)
			ಹೋಗು ಮಗು_err;
		ಆದರೆ (ವಾದಗಳು->gid != (gid_t)-1 && setgid(ವಾದಗಳು->gid) < 0)
			ಹೋಗು ಮಗು_err;
		ಆದರೆ (ವಾದಗಳು->uid != (uid_t)-1 && setuid(ವಾದಗಳು->uid) < 0)
			ಹೋಗು ಮಗು_err;
		ಆದರೆ (ವಾದಗಳು->setsid && setsid() < 0)
			ಹೋಗು ಮಗು_err;
		sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
		execvp(cmd, UNCONST(ವಾದಗಳು->argv));
ಮಗು_err:
		saved_errno = errno;
		ಲಿಖ(execpipe[1], &saved_errno, ಗಾತ್ರ saved_errno);
		_exit(1);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/*ಪೋಷಕ*/
		close(execpipe[1]);
		execpipe[1] = -1;
		n = ಪಠ(execpipe[0], &err, ಗಾತ್ರ err);
		ಆದರೆ (n != 0) {
			/* exec failed, reap the child and cleanup */
			waitpid(res.pid, ಖಾಲಿ, 0); /* do we care about waitpid failures? */
			res.pid = -1;
		}
		sigprocmask(SIG_SETMASK, &old, ಖಾಲಿ);
		ಆದರೆ (res.pid < 0) {
			errno = (n == ಗಾತ್ರ err) ? err : EINVAL;
			ಹೋಗು ನಿರ್ಗಮಿಸು;
		}

		ಆದರೆ (stdin_pipe[0] >= 0) {
			res.proc_stdin = stdin_pipe[1];
			stdin_pipe[1] = -1; /* to avoid closing it below */
		}
		ಆದರೆ (stdout_pipe[0] >= 0) {
			res.proc_stdout = stdout_pipe[0];
			stdout_pipe[0] = -1;
		}
		ಆದರೆ (stderr_pipe[0] >= 0) {
			res.proc_stderr = stderr_pipe[0];
			stderr_pipe[0] = -1;
		}
	}

ನಿರ್ಗಮಿಸು:
	saved_errno = errno;
	checked_close(execpipe[0]); checked_close(execpipe[1]);
	checked_close(stdin_pipe[0]); checked_close(stdin_pipe[1]);
	checked_close(stdout_pipe[0]); checked_close(stdout_pipe[1]);
	checked_close(stderr_pipe[0]); checked_close(stderr_pipe[1]);
	errno = saved_errno;
	ಹಿಂತಿರುಗಿ res;
}

ಪೂರ್ಣಾಂಕ rc_waitpid(pid_t pid)
{
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	ಆಗ (waitpid(pid, &ಸ್ಥಿತಿ_2, 0) == -1) {
		ಆದರೆ (errno != EINTR) {
			ಸ್ಥಿತಿ_2 = -1;
			ನಿಲ್ಲಿಸು;
		}
	}
	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

ಪೂರ್ಣಾಂಕ rc_pipe_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ devnullfd)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argv[] = { "/bin/sh", "-c", cmd, ಖಾಲಿ };
	ರಚನೆ exec_ಫಲಿತಾಂಶ res;
	ರಚನೆ exec_ವಾದಗಳು ವಾದಗಳು = exec_init(argv);
	ವಾದಗಳು.redirect_stdin = EXEC_MKPIPE;
	ವಾದಗಳು.redirect_stdout = ವಾದಗಳು.redirect_stderr = devnullfd;
	res = do_exec(&ವಾದಗಳು);
	ಹಿಂತಿರುಗಿ (res.pid > 0) ? res.proc_stdin : -1;
}
