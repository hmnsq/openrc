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

#شامل <errno.h>
#شامل <ctype.h>
#شامل <fcntl.h>
#شامل <poll.h>
#شامل <signal.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/types.h>
#شامل <sys/ioctl.h>
#شامل <sys/wait.h>
#شامل <termios.h>
#شامل <time.h>
#شامل <unistd.h>

#اگر defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#وگرنهاگر defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#وگرنه
#  include <libutil.h>
#endif

#شامل "einfo.h"
#شامل "rc-logger.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

ایستا صحیح signal_pipe[2] = { -1, -1 };
ایستا صحیح fd_stdout = -1;
ایستا صحیح fd_stderr = -1;
ایستا ثابت نویسه *runlevel = پوچ;
ایستا bool in_escape = false;
ایستا bool in_term = false;

ایستا نویسه *logbuf = پوچ;
ایستا size_t logbuf_size = 0;
ایستا size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
صحیح rc_logger_tty = -1;
bool rc_in_logger = false;

ایستا تهی
write_log(صحیح logfd, ثابت نویسه *buffer, size_t bytes)
{
	ثابت نویسه *p = buffer;

	وقتی ((size_t)(p - buffer) < bytes) {
		انتخاب (*p) {
		حالت '\r':
			برو cont;
		حالت '\033':
			in_escape = true;
			in_term = false;
			برو cont;
		حالت '\n':
			in_escape = in_term = false;
			بشکن;
		حالت '[':
			اگر (in_escape)
				in_term = true;
			بشکن;
		}

		اگر (!in_escape) {
			اگر (!isprint((صحیح) *p) && *p != '\n')
				برو cont;
			اگر (اكتب(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			ادامه;
		}

		اگر (!in_term || isalpha((unsigned نویسه)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

ایستا تهی
write_time(FILE *f, ثابت نویسه *s)
{
	time_t now = time(پوچ);
	ساختار tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ایستا تهی
mkpath(ثابت نویسه *path)
{
	نویسه *dir = xstrdup(path), *slash = dir;
	وقتی ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		اگر (mkdir(path, 0755) == -1 && errno != EEXIST)
			بشکن;
		*slash = '/';
	}
	mkdir(path, 0755);
	آزاد(dir);
}

تهی
rc_logger_close(تهی)
{
	صحیح sig = SIGTERM;

	اگر (signal_pipe[1] > -1) {
		اگر (اكتب(signal_pipe[1], &sig, اندازه(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	اگر (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	اگر (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	اگر (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

تهی
rc_logger_open(ثابت نویسه *level)
{
	صحیح slave_tty;
	ساختار termios tt;
	ساختار winsize ws;
	نویسه buffer[BUFSIZ];
	ساختار pollfd fd[2];
	صحیح s = 0;
	size_t bytes;
	FILE *log = پوچ;
	FILE *plog = پوچ;
	ثابت نویسه *logfile;
	نویسه *tmplog, *usrlog = پوچ;
	صحیح log_error = 0;

	اگر (!rc_conf_yesno("rc_logger"))
		بازگشت;

	اگر (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	اگر (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		اگر (openpty(&rc_logger_tty, &slave_tty, پوچ, &tt, &ws))
			بازگشت;
	} وگرنه
		اگر (openpty(&rc_logger_tty, &slave_tty, پوچ, پوچ, پوچ))
			بازگشت;

	اگر ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	اگر ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	انتخاب (rc_logger_pid) {
	حالت -1:
		eerror("fork: %s", strerror(errno));
		بشکن;
	حالت 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		اگر ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		وگرنه {
			آزاد(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(اندازه (نویسه) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		اگر (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		برای (;;) {
			اگر ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				بشکن;
			} وگرنه اگر (s == 0)
				ادامه;

			اگر (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = اقرأ(rc_logger_tty, buffer, BUFSIZ);
				اگر (اكتب(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				اگر (log)
					write_log(fileno (log), buffer, bytes);
				وگرنه {
					اگر (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    اندازه(نویسه ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			اگر (fd[0].revents & (POLLIN | POLLHUP))
				بشکن;
		}
		اگر (logbuf) {
			اگر ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			آزاد(logbuf);
		}
		اگر (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_مقدار("rc_log_path");
		اگر (logfile == پوچ) {
			اگر (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} وگرنه {
				ثابت نویسه *state_home = getenv("XDG_STATE_HOME");
				اگر (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} وگرنه اگر ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		اگر (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		اگر ((plog = fopen(logfile, "ae"))) {
			اگر ((log = fopen(tmplog, "re"))) {
				وقتی ((bytes = fread(buffer, اندازه(*buffer), BUFSIZ, log)) > 0) {
					اگر (fwrite(buffer, اندازه(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						بشکن;
					}
				}
				fclose(log);
			} وگرنه {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} وگرنه {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			اگر (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		آزاد(usrlog);

		/* Try to keep the temporary log in case of errors */
		اگر (!log_error) {
			اگر (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				اگر (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} وگرنه اگر (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		آزاد(tmplog);

		exit(0);
		/* NOTREACHED */

	پیشفرض:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		اگر ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		اگر ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		اگر (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		بشکن;
	}
}
