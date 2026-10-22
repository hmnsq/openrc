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

#ಆದರೆ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ಅಥವಾಆದರೆ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ಇಲ್ಲದಿದ್ದರೆ
#  include <libutil.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "rc-logger.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULTLOG "/var/log/rc.log"

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ signal_pipe[2] = { -1, -1 };
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fd_stdout = -1;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fd_stderr = -1;
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *runlevel = ಖಾಲಿ;
ಸ್ಥಿರ bool ಒಳ_escape = false;
ಸ್ಥಿರ bool ಒಳ_ಪದ = false;

ಸ್ಥಿರ ಅಕ್ಷರ *logbuf = ಖಾಲಿ;
ಸ್ಥಿರ size_t logbuf_ಗಾತ್ರ = 0;
ಸ್ಥಿರ size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
ಪೂರ್ಣಾಂಕ rc_logger_tty = -1;
bool rc_ಒಳ_logger = false;

ಸ್ಥಿರ ಶೂನ್ಯ
write_log(ಪೂರ್ಣಾಂಕ logfd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಫರ್, size_t ಬೈಟುಗಳು)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = ಬಫರ್;

	ಆಗ ((size_t)(p - ಬಫರ್) < ಬೈಟುಗಳು) {
		ಆಯ್ಕೆ (*p) {
		ಸ್ಥಿತಿ '\r':
			ಹೋಗು cont;
		ಸ್ಥಿತಿ '\033':
			ಒಳ_escape = true;
			ಒಳ_ಪದ = false;
			ಹೋಗು cont;
		ಸ್ಥಿತಿ '\n':
			ಒಳ_escape = ಒಳ_ಪದ = false;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '[':
			ಆದರೆ (ಒಳ_escape)
				ಒಳ_ಪದ = true;
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (!ಒಳ_escape) {
			ಆದರೆ (!isprint((ಪೂರ್ಣಾಂಕ) *p) && *p != '\n')
				ಹೋಗು cont;
			ಆದರೆ (ಲಿಖ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (!ಒಳ_ಪದ || ಅಕ್ಷರವೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*p))
			ಒಳ_escape = ಒಳ_ಪದ = false;
cont:
		p++;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ
write_ಸಮಯ(FILE *f, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	time_t now = time(ಖಾಲಿ);
	ರಚನೆ tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ಸ್ಥಿರ ಶೂನ್ಯ
mkpath(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ)
{
	ಅಕ್ಷರ *dir = xstrdup(ಮಾರ್ಗ), *slash = dir;
	ಆಗ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		ಆದರೆ (mkdir(ಮಾರ್ಗ, 0755) == -1 && errno != EEXIST)
			ನಿಲ್ಲಿಸು;
		*slash = '/';
	}
	mkdir(ಮಾರ್ಗ, 0755);
	ಬಿಡುಗಡೆ(dir);
}

ಶೂನ್ಯ
rc_logger_close(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ sig = SIGTERM;

	ಆದರೆ (signal_pipe[1] > -1) {
		ಆದರೆ (ಲಿಖ(signal_pipe[1], &sig, ಗಾತ್ರ(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	ಆದರೆ (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	ಆದರೆ (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	ಆದರೆ (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

ಶೂನ್ಯ
rc_logger_open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *level)
{
	ಪೂರ್ಣಾಂಕ slave_tty;
	ರಚನೆ termios tt;
	ರಚನೆ winsize ws;
	ಅಕ್ಷರ ಬಫರ್[BUFSIZ];
	ರಚನೆ pollfd fd[2];
	ಪೂರ್ಣಾಂಕ s = 0;
	size_t ಬೈಟುಗಳು;
	FILE *ಲಘುಗಣಕ = ಖಾಲಿ;
	FILE *plog = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *logfile;
	ಅಕ್ಷರ *tmplog, *usrlog = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ log_error = 0;

	ಆದರೆ (!rc_conf_yesno("rc_logger"))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	ಆದರೆ (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		ಆದರೆ (openpty(&rc_logger_tty, &slave_tty, ಖಾಲಿ, &tt, &ws))
			ಹಿಂತಿರುಗಿ;
	} ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (openpty(&rc_logger_tty, &slave_tty, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ))
			ಹಿಂತಿರುಗಿ;

	ಆದರೆ ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	ಆದರೆ ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	ಆಯ್ಕೆ (rc_logger_pid) {
	ಸ್ಥಿತಿ -1:
		eerror("fork: %s", strerror(errno));
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 0:
		rc_ಒಳ_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		ಆದರೆ ((ಲಘುಗಣಕ = fopen(tmplog, "ae")))
			write_ಸಮಯ(ಲಘುಗಣಕ, "started");
		ಇಲ್ಲದಿದ್ದರೆ {
			ಬಿಡುಗಡೆ(logbuf);
			logbuf_ಗಾತ್ರ = BUFSIZ * 10;
			logbuf = xmalloc(ಗಾತ್ರ (ಅಕ್ಷರ) * logbuf_ಗಾತ್ರ);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		ಆದರೆ (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		ಗಾಗಿ (;;) {
			ಆದರೆ ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s == 0)
				ಮುಂದುವರಿ;

			ಆದರೆ (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(ಬಫರ್, 0, BUFSIZ);
				ಬೈಟುಗಳು = ಪಠ(rc_logger_tty, ಬಫರ್, BUFSIZ);
				ಆದರೆ (ಲಿಖ(STDOUT_FILENO, ಬಫರ್, ಬೈಟುಗಳು) == -1)
					eerror("write: %s", strerror(errno));

				ಆದರೆ (ಲಘುಗಣಕ)
					write_log(fileno (ಲಘುಗಣಕ), ಬಫರ್, ಬೈಟುಗಳು);
				ಇಲ್ಲದಿದ್ದರೆ {
					ಆದರೆ (logbuf_ಗಾತ್ರ - logbuf_len < ಬೈಟುಗಳು) {
						logbuf_ಗಾತ್ರ += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    ಗಾತ್ರ(ಅಕ್ಷರ ) *
						    logbuf_ಗಾತ್ರ);
					}

					ಸ್ಮರಣೆ_ನಕಲು(logbuf + logbuf_len,
					    ಬಫರ್, ಬೈಟುಗಳು);
					logbuf_len += ಬೈಟುಗಳು;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			ಆದರೆ (fd[0].revents & (POLLIN | POLLHUP))
				ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (logbuf) {
			ಆದರೆ ((ಲಘುಗಣಕ = fopen(tmplog, "ae"))) {
				write_ಸಮಯ(ಲಘುಗಣಕ, "started");
				write_log(fileno(ಲಘುಗಣಕ), logbuf, logbuf_len);
			}
			ಬಿಡುಗಡೆ(logbuf);
		}
		ಆದರೆ (ಲಘುಗಣಕ) {
			write_ಸಮಯ(ಲಘುಗಣಕ, "stopped");
			fclose(ಲಘುಗಣಕ);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_ಮೌಲ್ಯ("rc_log_path");
		ಆದರೆ (logfile == ಖಾಲಿ) {
			ಆದರೆ (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *state_home = getenv("XDG_STATE_HOME");
				ಆದರೆ (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		ಆದರೆ (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		ಆದರೆ ((plog = fopen(logfile, "ae"))) {
			ಆದರೆ ((ಲಘುಗಣಕ = fopen(tmplog, "re"))) {
				ಆಗ ((ಬೈಟುಗಳು = fread(ಬಫರ್, ಗಾತ್ರ(*ಬಫರ್), BUFSIZ, ಲಘುಗಣಕ)) > 0) {
					ಆದರೆ (fwrite(ಬಫರ್, ಗಾತ್ರ(*ಬಫರ್), ಬೈಟುಗಳು, plog) < ಬೈಟುಗಳು) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						ನಿಲ್ಲಿಸು;
					}
				}
				fclose(ಲಘುಗಣಕ);
			} ಇಲ್ಲದಿದ್ದರೆ {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			ಆದರೆ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		ಬಿಡುಗಡೆ(usrlog);

		/* Try to keep the temporary log in case of errors */
		ಆದರೆ (!log_error) {
			ಆದರೆ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				ಆದರೆ (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		ಬಿಡುಗಡೆ(tmplog);

		ನಿರ್ಗಮಿಸು(0);
		/* NOTREACHED */

	ಪೂರ್ವನಿಯೋಜಿತ:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		ಆದರೆ ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		ಆದರೆ ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		ಆದರೆ (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		ನಿಲ್ಲಿಸು;
	}
}
