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

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#nếu defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#không_thì
#  include <libutil.h>
#endif

#include "einfo.h"
#include "rc-logger.h"
#include "rc.h"
#include "misc.h"
#include "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

static số_nguyên signal_pipe[2] = { -1, -1 };
static số_nguyên fd_stdout = -1;
static số_nguyên fd_stderr = -1;
static const char *runlevel = NULL;
static bool in_escape = false;
static bool in_term = false;

static char *logbuf = NULL;
static size_t logbuf_size = 0;
static size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
số_nguyên rc_logger_tty = -1;
bool rc_in_logger = false;

static void
write_log(số_nguyên logfd, const char *buffer, size_t bytes)
{
	const char *p = buffer;

	trong_khi ((size_t)(p - buffer) < bytes) {
		switch (*p) {
		case '\r':
			goto cont;
		case '\033':
			in_escape = true;
			in_term = false;
			goto cont;
		case '\n':
			in_escape = in_term = false;
			break;
		case '[':
			nếu (in_escape)
				in_term = true;
			break;
		}

		nếu (!in_escape) {
			nếu (!isprint((số_nguyên) *p) && *p != '\n')
				goto cont;
			nếu (write(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			continue;
		}

		nếu (!in_term || isalpha((unsigned char)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

static void
write_time(FILE *f, const char *s)
{
	time_t now = time(NULL);
	struct tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

static void
mkpath(const char *path)
{
	char *dir = xstrdup(path), *slash = dir;
	trong_khi ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		nếu (mkdir(path, 0755) == -1 && errno != EEXIST)
			break;
		*slash = '/';
	}
	mkdir(path, 0755);
	free(dir);
}

void
rc_logger_close(void)
{
	số_nguyên sig = SIGTERM;

	nếu (signal_pipe[1] > -1) {
		nếu (write(signal_pipe[1], &sig, sizeof(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	nếu (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	nếu (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	nếu (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

void
rc_logger_open(const char *level)
{
	số_nguyên slave_tty;
	struct termios tt;
	struct winsize ws;
	char buffer[BUFSIZ];
	struct pollfd fd[2];
	số_nguyên s = 0;
	size_t bytes;
	FILE *log = NULL;
	FILE *plog = NULL;
	const char *logfile;
	char *tmplog, *usrlog = NULL;
	số_nguyên log_error = 0;

	nếu (!rc_conf_yesno("rc_logger"))
		trả_về;

	nếu (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	nếu (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		nếu (openpty(&rc_logger_tty, &slave_tty, NULL, &tt, &ws))
			trả_về;
	} không_thì
		nếu (openpty(&rc_logger_tty, &slave_tty, NULL, NULL, NULL))
			trả_về;

	nếu ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	nếu ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	switch (rc_logger_pid) {
	case -1:
		eerror("fork: %s", strerror(errno));
		break;
	case 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		nếu ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		không_thì {
			free(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(sizeof (char) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		nếu (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		cho (;;) {
			nếu ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				break;
			} không_thì nếu (s == 0)
				continue;

			nếu (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = read(rc_logger_tty, buffer, BUFSIZ);
				nếu (write(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				nếu (log)
					write_log(fileno (log), buffer, bytes);
				không_thì {
					nếu (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    sizeof(char ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			nếu (fd[0].revents & (POLLIN | POLLHUP))
				break;
		}
		nếu (logbuf) {
			nếu ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			free(logbuf);
		}
		nếu (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_value("rc_log_path");
		nếu (logfile == NULL) {
			nếu (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} không_thì {
				const char *state_home = getenv("XDG_STATE_HOME");
				nếu (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} không_thì nếu ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		nếu (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		nếu ((plog = fopen(logfile, "ae"))) {
			nếu ((log = fopen(tmplog, "re"))) {
				trong_khi ((bytes = fread(buffer, sizeof(*buffer), BUFSIZ, log)) > 0) {
					nếu (fwrite(buffer, sizeof(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						break;
					}
				}
				fclose(log);
			} không_thì {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} không_thì {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			nếu (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		free(usrlog);

		/* Try to keep the temporary log in case of errors */
		nếu (!log_error) {
			nếu (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				nếu (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} không_thì nếu (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		free(tmplog);

		exit(0);
		/* NOTREACHED */

	default:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		nếu ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		nếu ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		nếu (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		break;
	}
}
