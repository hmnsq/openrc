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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <termios.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਜੇ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ਜਾਂਜੇ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ਨਹੀਂਤਾਂ
#  include <libutil.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc-logger.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "misc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULTLOG "/var/log/rc.log"

ਸਥਿਰ ਪੂਰਨ signal_pipe[2] = { -1, -1 };
ਸਥਿਰ ਪੂਰਨ fd_stdout = -1;
ਸਥਿਰ ਪੂਰਨ fd_stderr = -1;
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *runlevel = ਨਿਰਾ;
ਸਥਿਰ bool ਵਿੱਚ_escape = false;
ਸਥਿਰ bool ਵਿੱਚ_ਪਦ = false;

ਸਥਿਰ ਅੱਖਰ *logbuf = ਨਿਰਾ;
ਸਥਿਰ size_t logbuf_ਮਾਪ = 0;
ਸਥਿਰ size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
ਪੂਰਨ rc_logger_tty = -1;
bool rc_ਵਿੱਚ_logger = false;

ਸਥਿਰ ਖਾਲੀ
write_log(ਪੂਰਨ logfd, ਅਚਲ ਅੱਖਰ *ਬਫਰ, size_t ਬਾਈਟਾਂ)
{
	ਅਚਲ ਅੱਖਰ *p = ਬਫਰ;

	ਜਦੋਂ ((size_t)(p - ਬਫਰ) < ਬਾਈਟਾਂ) {
		ਚੋਣ (*p) {
		ਹਾਲਤ '\r':
			ਜਾਓ cont;
		ਹਾਲਤ '\033':
			ਵਿੱਚ_escape = true;
			ਵਿੱਚ_ਪਦ = false;
			ਜਾਓ cont;
		ਹਾਲਤ '\n':
			ਵਿੱਚ_escape = ਵਿੱਚ_ਪਦ = false;
			ਰੋਕੋ;
		ਹਾਲਤ '[':
			ਜੇ (ਵਿੱਚ_escape)
				ਵਿੱਚ_ਪਦ = true;
			ਰੋਕੋ;
		}

		ਜੇ (!ਵਿੱਚ_escape) {
			ਜੇ (!isprint((ਪੂਰਨ) *p) && *p != '\n')
				ਜਾਓ cont;
			ਜੇ (ਲਿਖ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			ਜਾਰੀ;
		}

		ਜੇ (!ਵਿੱਚ_ਪਦ || ਅੱਖਰ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*p))
			ਵਿੱਚ_escape = ਵਿੱਚ_ਪਦ = false;
cont:
		p++;
	}
}

ਸਥਿਰ ਖਾਲੀ
write_ਸਮਾਂ(FILE *f, ਅਚਲ ਅੱਖਰ *s)
{
	time_t now = time(ਨਿਰਾ);
	ਬਣਤਰ tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ਸਥਿਰ ਖਾਲੀ
mkpath(ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ *dir = xstrdup(ਰਸਤਾ), *slash = dir;
	ਜਦੋਂ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		ਜੇ (mkdir(ਰਸਤਾ, 0755) == -1 && errno != EEXIST)
			ਰੋਕੋ;
		*slash = '/';
	}
	mkdir(ਰਸਤਾ, 0755);
	ਮੁਕਤ(dir);
}

ਖਾਲੀ
rc_logger_close(ਖਾਲੀ)
{
	ਪੂਰਨ sig = SIGTERM;

	ਜੇ (signal_pipe[1] > -1) {
		ਜੇ (ਲਿਖ(signal_pipe[1], &sig, ਆਕਾਰ(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	ਜੇ (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	ਜੇ (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	ਜੇ (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

ਖਾਲੀ
rc_logger_open(ਅਚਲ ਅੱਖਰ *level)
{
	ਪੂਰਨ slave_tty;
	ਬਣਤਰ termios tt;
	ਬਣਤਰ winsize ws;
	ਅੱਖਰ ਬਫਰ[BUFSIZ];
	ਬਣਤਰ pollfd fd[2];
	ਪੂਰਨ s = 0;
	size_t ਬਾਈਟਾਂ;
	FILE *ਲਘੂਗਣਕ = ਨਿਰਾ;
	FILE *plog = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *logfile;
	ਅੱਖਰ *tmplog, *usrlog = ਨਿਰਾ;
	ਪੂਰਨ log_error = 0;

	ਜੇ (!rc_conf_yesno("rc_logger"))
		ਵਾਪਸ;

	ਜੇ (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	ਜੇ (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		ਜੇ (openpty(&rc_logger_tty, &slave_tty, ਨਿਰਾ, &tt, &ws))
			ਵਾਪਸ;
	} ਨਹੀਂਤਾਂ
		ਜੇ (openpty(&rc_logger_tty, &slave_tty, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ))
			ਵਾਪਸ;

	ਜੇ ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	ਜੇ ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	ਚੋਣ (rc_logger_pid) {
	ਹਾਲਤ -1:
		eerror("fork: %s", strerror(errno));
		ਰੋਕੋ;
	ਹਾਲਤ 0:
		rc_ਵਿੱਚ_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		ਜੇ ((ਲਘੂਗਣਕ = fopen(tmplog, "ae")))
			write_ਸਮਾਂ(ਲਘੂਗਣਕ, "started");
		ਨਹੀਂਤਾਂ {
			ਮੁਕਤ(logbuf);
			logbuf_ਮਾਪ = BUFSIZ * 10;
			logbuf = xmalloc(ਆਕਾਰ (ਅੱਖਰ) * logbuf_ਮਾਪ);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		ਜੇ (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		ਲਈ (;;) {
			ਜੇ ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ ਜੇ (s == 0)
				ਜਾਰੀ;

			ਜੇ (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(ਬਫਰ, 0, BUFSIZ);
				ਬਾਈਟਾਂ = ਪਠ(rc_logger_tty, ਬਫਰ, BUFSIZ);
				ਜੇ (ਲਿਖ(STDOUT_FILENO, ਬਫਰ, ਬਾਈਟਾਂ) == -1)
					eerror("write: %s", strerror(errno));

				ਜੇ (ਲਘੂਗਣਕ)
					write_log(fileno (ਲਘੂਗਣਕ), ਬਫਰ, ਬਾਈਟਾਂ);
				ਨਹੀਂਤਾਂ {
					ਜੇ (logbuf_ਮਾਪ - logbuf_len < ਬਾਈਟਾਂ) {
						logbuf_ਮਾਪ += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    ਆਕਾਰ(ਅੱਖਰ ) *
						    logbuf_ਮਾਪ);
					}

					ਸਮ੍ਰਿਤੀ_ਨਕਲ(logbuf + logbuf_len,
					    ਬਫਰ, ਬਾਈਟਾਂ);
					logbuf_len += ਬਾਈਟਾਂ;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			ਜੇ (fd[0].revents & (POLLIN | POLLHUP))
				ਰੋਕੋ;
		}
		ਜੇ (logbuf) {
			ਜੇ ((ਲਘੂਗਣਕ = fopen(tmplog, "ae"))) {
				write_ਸਮਾਂ(ਲਘੂਗਣਕ, "started");
				write_log(fileno(ਲਘੂਗਣਕ), logbuf, logbuf_len);
			}
			ਮੁਕਤ(logbuf);
		}
		ਜੇ (ਲਘੂਗਣਕ) {
			write_ਸਮਾਂ(ਲਘੂਗਣਕ, "stopped");
			fclose(ਲਘੂਗਣਕ);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_ਮੁੱਲ("rc_log_path");
		ਜੇ (logfile == ਨਿਰਾ) {
			ਜੇ (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} ਨਹੀਂਤਾਂ {
				ਅਚਲ ਅੱਖਰ *state_home = getenv("XDG_STATE_HOME");
				ਜੇ (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} ਨਹੀਂਤਾਂ ਜੇ ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		ਜੇ (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		ਜੇ ((plog = fopen(logfile, "ae"))) {
			ਜੇ ((ਲਘੂਗਣਕ = fopen(tmplog, "re"))) {
				ਜਦੋਂ ((ਬਾਈਟਾਂ = fread(ਬਫਰ, ਆਕਾਰ(*ਬਫਰ), BUFSIZ, ਲਘੂਗਣਕ)) > 0) {
					ਜੇ (fwrite(ਬਫਰ, ਆਕਾਰ(*ਬਫਰ), ਬਾਈਟਾਂ, plog) < ਬਾਈਟਾਂ) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						ਰੋਕੋ;
					}
				}
				fclose(ਲਘੂਗਣਕ);
			} ਨਹੀਂਤਾਂ {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} ਨਹੀਂਤਾਂ {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			ਜੇ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		ਮੁਕਤ(usrlog);

		/* Try to keep the temporary log in case of errors */
		ਜੇ (!log_error) {
			ਜੇ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				ਜੇ (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} ਨਹੀਂਤਾਂ ਜੇ (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		ਮੁਕਤ(tmplog);

		ਬਾਹਰ_ਨਿਕਲੋ(0);
		/* NOTREACHED */

	ਮੂਲ:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		ਜੇ ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		ਜੇ ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		ਜੇ (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		ਰੋਕੋ;
	}
}
