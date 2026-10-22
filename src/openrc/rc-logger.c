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

#만약 defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#아니면
#  include <libutil.h>
#endif

#include "einfo.h"
#include "rc-logger.h"
#include "rc.h"
#include "misc.h"
#include "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

static 정수 signal_pipe[2] = { -1, -1 };
static 정수 fd_stdout = -1;
static 정수 fd_stderr = -1;
static const char *runlevel = NULL;
static bool in_escape = false;
static bool in_term = false;

static char *logbuf = NULL;
static size_t logbuf_size = 0;
static size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
정수 rc_logger_tty = -1;
bool rc_in_logger = false;

static void
write_log(정수 logfd, const char *buffer, size_t bytes)
{
	const char *p = buffer;

	동안 ((size_t)(p - buffer) < bytes) {
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
			만약 (in_escape)
				in_term = true;
			break;
		}

		만약 (!in_escape) {
			만약 (!isprint((정수) *p) && *p != '\n')
				goto cont;
			만약 (write(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			continue;
		}

		만약 (!in_term || isalpha((unsigned char)*p))
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
	동안 ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		만약 (mkdir(path, 0755) == -1 && errno != EEXIST)
			break;
		*slash = '/';
	}
	mkdir(path, 0755);
	free(dir);
}

void
rc_logger_close(void)
{
	정수 sig = SIGTERM;

	만약 (signal_pipe[1] > -1) {
		만약 (write(signal_pipe[1], &sig, sizeof(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	만약 (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	만약 (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	만약 (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

void
rc_logger_open(const char *level)
{
	정수 slave_tty;
	struct termios tt;
	struct winsize ws;
	char buffer[BUFSIZ];
	struct pollfd fd[2];
	정수 s = 0;
	size_t bytes;
	FILE *log = NULL;
	FILE *plog = NULL;
	const char *logfile;
	char *tmplog, *usrlog = NULL;
	정수 log_error = 0;

	만약 (!rc_conf_yesno("rc_logger"))
		반환;

	만약 (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	만약 (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		만약 (openpty(&rc_logger_tty, &slave_tty, NULL, &tt, &ws))
			반환;
	} 아니면
		만약 (openpty(&rc_logger_tty, &slave_tty, NULL, NULL, NULL))
			반환;

	만약 ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	만약 ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
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
		만약 ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		아니면 {
			free(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(sizeof (char) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		만약 (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		위해 (;;) {
			만약 ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				break;
			} 아니면 만약 (s == 0)
				continue;

			만약 (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = read(rc_logger_tty, buffer, BUFSIZ);
				만약 (write(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				만약 (log)
					write_log(fileno (log), buffer, bytes);
				아니면 {
					만약 (logbuf_size - logbuf_len < bytes) {
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
			만약 (fd[0].revents & (POLLIN | POLLHUP))
				break;
		}
		만약 (logbuf) {
			만약 ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			free(logbuf);
		}
		만약 (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_value("rc_log_path");
		만약 (logfile == NULL) {
			만약 (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} 아니면 {
				const char *state_home = getenv("XDG_STATE_HOME");
				만약 (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} 아니면 만약 ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		만약 (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		만약 ((plog = fopen(logfile, "ae"))) {
			만약 ((log = fopen(tmplog, "re"))) {
				동안 ((bytes = fread(buffer, sizeof(*buffer), BUFSIZ, log)) > 0) {
					만약 (fwrite(buffer, sizeof(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						break;
					}
				}
				fclose(log);
			} 아니면 {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} 아니면 {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			만약 (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		free(usrlog);

		/* Try to keep the temporary log in case of errors */
		만약 (!log_error) {
			만약 (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				만약 (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} 아니면 만약 (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		free(tmplog);

		exit(0);
		/* NOTREACHED */

	default:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		만약 ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		만약 ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		만약 (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		break;
	}
}
