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

#ضمن <errno.h>
#ضمن <ctype.h>
#ضمن <fcntl.h>
#ضمن <poll.h>
#ضمن <signal.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/types.h>
#ضمن <sys/ioctl.h>
#ضمن <sys/wait.h>
#ضمن <termios.h>
#ضمن <time.h>
#ضمن <unistd.h>

#إذا defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#وإلاإذا defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#وإلا
#  include <libutil.h>
#endif

#ضمن "einfo.h"
#ضمن "rc-logger.h"
#ضمن "rc.h"
#ضمن "misc.h"
#ضمن "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

ساكن صحيح signal_pipe[2] = { -1, -1 };
ساكن صحيح fd_stdout = -1;
ساكن صحيح fd_stderr = -1;
ساكن ثابت حرف *runlevel = عدم;
ساكن bool in_escape = false;
ساكن bool in_term = false;

ساكن حرف *logbuf = عدم;
ساكن size_t logbuf_size = 0;
ساكن size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
صحيح rc_logger_tty = -1;
bool rc_in_logger = false;

ساكن فراغ
write_log(صحيح logfd, ثابت حرف *buffer, size_t bytes)
{
	ثابت حرف *p = buffer;

	طالما ((size_t)(p - buffer) < bytes) {
		بدل (*p) {
		حالة '\r':
			اذهب cont;
		حالة '\033':
			in_escape = true;
			in_term = false;
			اذهب cont;
		حالة '\n':
			in_escape = in_term = false;
			اكسر;
		حالة '[':
			إذا (in_escape)
				in_term = true;
			اكسر;
		}

		إذا (!in_escape) {
			إذا (!isprint((صحيح) *p) && *p != '\n')
				اذهب cont;
			إذا (اكتب(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			استمر;
		}

		إذا (!in_term || isalpha((unsigned حرف)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

ساكن فراغ
write_time(FILE *f, ثابت حرف *s)
{
	time_t now = time(عدم);
	بنية tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ساكن فراغ
mkpath(ثابت حرف *path)
{
	حرف *dir = xstrdup(path), *slash = dir;
	طالما ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		إذا (mkdir(path, 0755) == -1 && errno != EEXIST)
			اكسر;
		*slash = '/';
	}
	mkdir(path, 0755);
	حرر(dir);
}

فراغ
rc_logger_close(فراغ)
{
	صحيح sig = SIGTERM;

	إذا (signal_pipe[1] > -1) {
		إذا (اكتب(signal_pipe[1], &sig, حجم(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	إذا (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	إذا (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	إذا (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

فراغ
rc_logger_open(ثابت حرف *level)
{
	صحيح slave_tty;
	بنية termios tt;
	بنية winsize ws;
	حرف buffer[BUFSIZ];
	بنية pollfd fd[2];
	صحيح s = 0;
	size_t bytes;
	FILE *log = عدم;
	FILE *plog = عدم;
	ثابت حرف *logfile;
	حرف *tmplog, *usrlog = عدم;
	صحيح log_error = 0;

	إذا (!rc_conf_yesno("rc_logger"))
		أرجع;

	إذا (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	إذا (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		إذا (openpty(&rc_logger_tty, &slave_tty, عدم, &tt, &ws))
			أرجع;
	} وإلا
		إذا (openpty(&rc_logger_tty, &slave_tty, عدم, عدم, عدم))
			أرجع;

	إذا ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	إذا ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	بدل (rc_logger_pid) {
	حالة -1:
		eerror("fork: %s", strerror(errno));
		اكسر;
	حالة 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		إذا ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		وإلا {
			حرر(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(حجم (حرف) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		إذا (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		لكل (;;) {
			إذا ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				اكسر;
			} وإلا إذا (s == 0)
				استمر;

			إذا (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = اقرأ(rc_logger_tty, buffer, BUFSIZ);
				إذا (اكتب(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				إذا (log)
					write_log(fileno (log), buffer, bytes);
				وإلا {
					إذا (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    حجم(حرف ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			إذا (fd[0].revents & (POLLIN | POLLHUP))
				اكسر;
		}
		إذا (logbuf) {
			إذا ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			حرر(logbuf);
		}
		إذا (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_نيلاي("rc_log_path");
		إذا (logfile == عدم) {
			إذا (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} وإلا {
				ثابت حرف *state_home = getenv("XDG_STATE_HOME");
				إذا (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} وإلا إذا ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		إذا (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		إذا ((plog = fopen(logfile, "ae"))) {
			إذا ((log = fopen(tmplog, "re"))) {
				طالما ((bytes = fread(buffer, حجم(*buffer), BUFSIZ, log)) > 0) {
					إذا (fwrite(buffer, حجم(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						اكسر;
					}
				}
				fclose(log);
			} وإلا {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} وإلا {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			إذا (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		حرر(usrlog);

		/* Try to keep the temporary log in case of errors */
		إذا (!log_error) {
			إذا (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				إذا (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} وإلا إذا (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		حرر(tmplog);

		exit(0);
		/* NOTREACHED */

	افتراضي:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		إذا ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		إذا ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		إذا (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		اكسر;
	}
}
