/*
 * rc-logger.c
 * Spawns a logging daemon to capture stdout and stderr so we can log
 * them to a buffer and/or files.
 */

/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/ioctl.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <termios.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಯದಿ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ನೋಚೇದ್ಯದಿ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ಅನ್ಯಥಾ
#  include <libutil.h>
#endif

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "rc-logger.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ signal_pipe[2] = { -1, -1 };
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fd_stdout = -1;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fd_stderr = -1;
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *runlevel = ರಿಕ್ತ;
ಸ್ಥಿರ bool in_escape = false;
ಸ್ಥಿರ bool in_term = false;

ಸ್ಥಿರ ವರ್ಣ *logbuf = ರಿಕ್ತ;
ಸ್ಥಿರ size_t logbuf_size = 0;
ಸ್ಥಿರ size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
ಪೂರ್ಣಾಙ್ಕ rc_logger_tty = -1;
bool rc_in_logger = false;

ಸ್ಥಿರ ಶೂನ್ಯ
write_log(ಪೂರ್ಣಾಙ್ಕ logfd, ಧ್ರುವ ವರ್ಣ *buffer, size_t bytes)
{
	ಧ್ರುವ ವರ್ಣ *p = buffer;

	ಯಾವತ್ ((size_t)(p - buffer) < bytes) {
		ವಿಕಲ್ಪ (*p) {
		ಪಕ್ಷ '\r':
			ಗಚ್ಛ cont;
		ಪಕ್ಷ '\033':
			in_escape = true;
			in_term = false;
			ಗಚ್ಛ cont;
		ಪಕ್ಷ '\n':
			in_escape = in_term = false;
			ಭಙ್ಗ;
		ಪಕ್ಷ '[':
			ಯದಿ (in_escape)
				in_term = true;
			ಭಙ್ಗ;
		}

		ಯದಿ (!in_escape) {
			ಯದಿ (!isprint((ಪೂರ್ಣಾಙ್ಕ) *p) && *p != '\n')
				ಗಚ್ಛ cont;
			ಯದಿ (ಲಿಖ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			ಅನುವರ್ತ;
		}

		ಯದಿ (!in_term || isalpha((unsigned ವರ್ಣ)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
write_time(FILE *f, ಧ್ರುವ ವರ್ಣ *s)
{
	time_t now = time(ರಿಕ್ತ);
	ಸಂರಚನಾ tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mkpath(ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ *dir = xstrdup(path), *slash = dir;
	ಯಾವತ್ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		ಯದಿ (mkdir(path, 0755) == -1 && errno != EEXIST)
			ಭಙ್ಗ;
		*slash = '/';
	}
	mkdir(path, 0755);
	ಮುಕ್ತ(dir);
}

ಶೂನ್ಯ
rc_logger_close(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ sig = SIGTERM;

	ಯದಿ (signal_pipe[1] > -1) {
		ಯದಿ (ಲಿಖ(signal_pipe[1], &sig, ಪರಿಮಾಣ(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	ಯದಿ (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	ಯದಿ (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	ಯದಿ (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

ಶೂನ್ಯ
rc_logger_open(ಧ್ರುವ ವರ್ಣ *level)
{
	ಪೂರ್ಣಾಙ್ಕ slave_tty;
	ಸಂರಚನಾ termios tt;
	ಸಂರಚನಾ winsize ws;
	ವರ್ಣ buffer[BUFSIZ];
	ಸಂರಚನಾ pollfd fd[2];
	ಪೂರ್ಣಾಙ್ಕ s = 0;
	size_t bytes;
	FILE *log = ರಿಕ್ತ;
	FILE *plog = ರಿಕ್ತ;
	ಧ್ರುವ ವರ್ಣ *logfile;
	ವರ್ಣ *tmplog, *usrlog = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ log_error = 0;

	ಯದಿ (!rc_conf_yesno("rc_logger"))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	ಯದಿ (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		ಯದಿ (openpty(&rc_logger_tty, &slave_tty, ರಿಕ್ತ, &tt, &ws))
			ಪ್ರತ್ಯಾಗಮ;
	} ಅನ್ಯಥಾ
		ಯದಿ (openpty(&rc_logger_tty, &slave_tty, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ))
			ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	ಯದಿ ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	ವಿಕಲ್ಪ (rc_logger_pid) {
	ಪಕ್ಷ -1:
		eerror("fork: %s", strerror(errno));
		ಭಙ್ಗ;
	ಪಕ್ಷ 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		ಯದಿ ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		ಅನ್ಯಥಾ {
			ಮುಕ್ತ(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(ಪರಿಮಾಣ (ವರ್ಣ) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		ಯದಿ (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		ಪ್ರತಿ (;;) {
			ಯದಿ ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				ಭಙ್ಗ;
			} ಅನ್ಯಥಾ ಯದಿ (s == 0)
				ಅನುವರ್ತ;

			ಯದಿ (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = ಪಠ(rc_logger_tty, buffer, BUFSIZ);
				ಯದಿ (ಲಿಖ(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				ಯದಿ (log)
					write_log(fileno (log), buffer, bytes);
				ಅನ್ಯಥಾ {
					ಯದಿ (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    ಪರಿಮಾಣ(ವರ್ಣ ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			ಯದಿ (fd[0].revents & (POLLIN | POLLHUP))
				ಭಙ್ಗ;
		}
		ಯದಿ (logbuf) {
			ಯದಿ ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			ಮುಕ್ತ(logbuf);
		}
		ಯದಿ (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_ಮೌಲ್ಯ("rc_log_path");
		ಯದಿ (logfile == ರಿಕ್ತ) {
			ಯದಿ (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} ಅನ್ಯಥಾ {
				ಧ್ರುವ ವರ್ಣ *state_home = getenv("XDG_STATE_HOME");
				ಯದಿ (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} ಅನ್ಯಥಾ ಯದಿ ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		ಯದಿ (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		ಯದಿ ((plog = fopen(logfile, "ae"))) {
			ಯದಿ ((log = fopen(tmplog, "re"))) {
				ಯಾವತ್ ((bytes = fread(buffer, ಪರಿಮಾಣ(*buffer), BUFSIZ, log)) > 0) {
					ಯದಿ (fwrite(buffer, ಪರಿಮಾಣ(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						ಭಙ್ಗ;
					}
				}
				fclose(log);
			} ಅನ್ಯಥಾ {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} ಅನ್ಯಥಾ {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			ಯದಿ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		ಮುಕ್ತ(usrlog);

		/* Try to keep the temporary log in case of errors */
		ಯದಿ (!log_error) {
			ಯದಿ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				ಯದಿ (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} ಅನ್ಯಥಾ ಯದಿ (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		ಮುಕ್ತ(tmplog);

		exit(0);
		/* NOTREACHED */

	ಸ್ವಭಾವ:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		ಯದಿ ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		ಯದಿ ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		ಯದಿ (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		ಭಙ್ಗ;
	}
}
