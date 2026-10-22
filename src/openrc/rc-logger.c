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

#ꦪꦼꦤ꧀ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ꦭꦶꦪ
#  include <libutil.h>
#endif

#include "einfo.h"
#include "rc-logger.h"
#include "rc.h"
#include "misc.h"
#include "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

static puur_naa_nka signal_pipe[2] = { -1, -1 };
static puur_naa_nka fd_stdout = -1;
static puur_naa_nka fd_stderr = -1;
static const var_na *runlevel = NULL;
static bool in_escape = false;
static bool in_term = false;

static var_na *logbuf = NULL;
static size_t logbuf_size = 0;
static size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
puur_naa_nka rc_logger_tty = -1;
bool rc_in_logger = false;

static void
write_log(puur_naa_nka logfd, const var_na *buffer, size_t bytes)
{
	const var_na *p = buffer;

	ꦱꦼꦢꦼꦭ ((size_t)(p - buffer) < bytes) {
		switch (*p) {
		pak_sa '\r':
			goto cont;
		pak_sa '\033':
			in_escape = true;
			in_term = false;
			goto cont;
		pak_sa '\n':
			in_escape = in_term = false;
			bha_nga;
		pak_sa '[':
			ꦪꦼꦤ꧀ (in_escape)
				in_term = true;
			bha_nga;
		}

		ꦪꦼꦤ꧀ (!in_escape) {
			ꦪꦼꦤ꧀ (!isprint((puur_naa_nka) *p) && *p != '\n')
				goto cont;
			ꦪꦼꦤ꧀ (write(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			continue;
		}

		ꦪꦼꦤ꧀ (!in_term || isalpha((unsigned var_na)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

static void
write_time(FILE *f, const var_na *s)
{
	time_t now = time(NULL);
	sa_nracanaa tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

static void
mkpath(const var_na *path)
{
	var_na *dir = xstrdup(path), *slash = dir;
	ꦱꦼꦢꦼꦭ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		ꦪꦼꦤ꧀ (mkdir(path, 0755) == -1 && errno != EEXIST)
			bha_nga;
		*slash = '/';
	}
	mkdir(path, 0755);
	free(dir);
}

void
rc_logger_close(void)
{
	puur_naa_nka sig = SIGTERM;

	ꦪꦼꦤ꧀ (signal_pipe[1] > -1) {
		ꦪꦼꦤ꧀ (write(signal_pipe[1], &sig, parimaa_na(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	ꦪꦼꦤ꧀ (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	ꦪꦼꦤ꧀ (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	ꦪꦼꦤ꧀ (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

void
rc_logger_open(const var_na *level)
{
	puur_naa_nka slave_tty;
	sa_nracanaa termios tt;
	sa_nracanaa winsize ws;
	var_na buffer[BUFSIZ];
	sa_nracanaa pollfd fd[2];
	puur_naa_nka s = 0;
	size_t bytes;
	FILE *log = NULL;
	FILE *plog = NULL;
	const var_na *logfile;
	var_na *tmplog, *usrlog = NULL;
	puur_naa_nka log_error = 0;

	ꦪꦼꦤ꧀ (!rc_conf_yesno("rc_logger"))
		ꦮꦭꦶ;

	ꦪꦼꦤ꧀ (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	ꦪꦼꦤ꧀ (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		ꦪꦼꦤ꧀ (openpty(&rc_logger_tty, &slave_tty, NULL, &tt, &ws))
			ꦮꦭꦶ;
	} ꦭꦶꦪ
		ꦪꦼꦤ꧀ (openpty(&rc_logger_tty, &slave_tty, NULL, NULL, NULL))
			ꦮꦭꦶ;

	ꦪꦼꦤ꧀ ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	ꦪꦼꦤ꧀ ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	switch (rc_logger_pid) {
	pak_sa -1:
		eerror("fork: %s", strerror(errno));
		bha_nga;
	pak_sa 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		ꦪꦼꦤ꧀ ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		ꦭꦶꦪ {
			free(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(parimaa_na (var_na) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		ꦪꦼꦤ꧀ (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		ꦏꦁꦒꦺꦴ (;;) {
			ꦪꦼꦤ꧀ ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				bha_nga;
			} ꦭꦶꦪ ꦪꦼꦤ꧀ (s == 0)
				continue;

			ꦪꦼꦤ꧀ (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = read(rc_logger_tty, buffer, BUFSIZ);
				ꦪꦼꦤ꧀ (write(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				ꦪꦼꦤ꧀ (log)
					write_log(fileno (log), buffer, bytes);
				ꦭꦶꦪ {
					ꦪꦼꦤ꧀ (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    parimaa_na(var_na ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			ꦪꦼꦤ꧀ (fd[0].revents & (POLLIN | POLLHUP))
				bha_nga;
		}
		ꦪꦼꦤ꧀ (logbuf) {
			ꦪꦼꦤ꧀ ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			free(logbuf);
		}
		ꦪꦼꦤ꧀ (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_value("rc_log_path");
		ꦪꦼꦤ꧀ (logfile == NULL) {
			ꦪꦼꦤ꧀ (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} ꦭꦶꦪ {
				const var_na *state_home = getenv("XDG_STATE_HOME");
				ꦪꦼꦤ꧀ (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} ꦭꦶꦪ ꦪꦼꦤ꧀ ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		ꦪꦼꦤ꧀ (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		ꦪꦼꦤ꧀ ((plog = fopen(logfile, "ae"))) {
			ꦪꦼꦤ꧀ ((log = fopen(tmplog, "re"))) {
				ꦱꦼꦢꦼꦭ ((bytes = fread(buffer, parimaa_na(*buffer), BUFSIZ, log)) > 0) {
					ꦪꦼꦤ꧀ (fwrite(buffer, parimaa_na(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						bha_nga;
					}
				}
				fclose(log);
			} ꦭꦶꦪ {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} ꦭꦶꦪ {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			ꦪꦼꦤ꧀ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		free(usrlog);

		/* Try to keep the temporary log in case of errors */
		ꦪꦼꦤ꧀ (!log_error) {
			ꦪꦼꦤ꧀ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				ꦪꦼꦤ꧀ (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} ꦭꦶꦪ ꦪꦼꦤ꧀ (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		free(tmplog);

		exit(0);
		/* NOTREACHED */

	default:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		ꦪꦼꦤ꧀ ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		ꦪꦼꦤ꧀ ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		ꦪꦼꦤ꧀ (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		bha_nga;
	}
}
