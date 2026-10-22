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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/ioctl.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <termios.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <unistd.h>

#എങ്കിൽ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#അല്ലെങ്കിലെങ്കിൽ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#അല്ലെങ്കിൽ
#  include <libutil.h>
#endif

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "rc-logger.h"
#അന്തര്ഭാവ "rc.h"
#അന്തര്ഭാവ "misc.h"
#അന്തര്ഭാവ "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

സ്ഥിതം പൂർണ്ണസംഖ്യ signal_pipe[2] = { -1, -1 };
സ്ഥിതം പൂർണ്ണസംഖ്യ fd_stdout = -1;
സ്ഥിതം പൂർണ്ണസംഖ്യ fd_stderr = -1;
സ്ഥിതം സ്ഥിരം അക്ഷരം *runlevel = ശൂന്യ;
സ്ഥിതം bool in_escape = false;
സ്ഥിതം bool in_term = false;

സ്ഥിതം അക്ഷരം *logbuf = ശൂന്യ;
സ്ഥിതം size_t logbuf_size = 0;
സ്ഥിതം size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
പൂർണ്ണസംഖ്യ rc_logger_tty = -1;
bool rc_in_logger = false;

സ്ഥിതം ശൂന്യം
write_log(പൂർണ്ണസംഖ്യ logfd, സ്ഥിരം അക്ഷരം *buffer, size_t bytes)
{
	സ്ഥിരം അക്ഷരം *p = buffer;

	ആകുമ്പോൾ ((size_t)(p - buffer) < bytes) {
		തിരഞ്ഞെടുപ്പ് (*p) {
		അവസ്ഥ '\r':
			പോകുക cont;
		അവസ്ഥ '\033':
			in_escape = true;
			in_term = false;
			പോകുക cont;
		അവസ്ഥ '\n':
			in_escape = in_term = false;
			നിർത്തുക;
		അവസ്ഥ '[':
			എങ്കിൽ (in_escape)
				in_term = true;
			നിർത്തുക;
		}

		എങ്കിൽ (!in_escape) {
			എങ്കിൽ (!isprint((പൂർണ്ണസംഖ്യ) *p) && *p != '\n')
				പോകുക cont;
			എങ്കിൽ (ലിഖ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			തുടരുക;
		}

		എങ്കിൽ (!in_term || isalpha((unsigned അക്ഷരം)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

സ്ഥിതം ശൂന്യം
write_time(FILE *f, സ്ഥിരം അക്ഷരം *s)
{
	time_t now = time(ശൂന്യ);
	ഘടന tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

സ്ഥിതം ശൂന്യം
mkpath(സ്ഥിരം അക്ഷരം *path)
{
	അക്ഷരം *dir = xstrdup(path), *slash = dir;
	ആകുമ്പോൾ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		എങ്കിൽ (mkdir(path, 0755) == -1 && errno != EEXIST)
			നിർത്തുക;
		*slash = '/';
	}
	mkdir(path, 0755);
	സ്വതന്ത്രം(dir);
}

ശൂന്യം
rc_logger_close(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ sig = SIGTERM;

	എങ്കിൽ (signal_pipe[1] > -1) {
		എങ്കിൽ (ലിഖ(signal_pipe[1], &sig, വലുപ്പം(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	എങ്കിൽ (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	എങ്കിൽ (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	എങ്കിൽ (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

ശൂന്യം
rc_logger_open(സ്ഥിരം അക്ഷരം *level)
{
	പൂർണ്ണസംഖ്യ slave_tty;
	ഘടന termios tt;
	ഘടന winsize ws;
	അക്ഷരം buffer[BUFSIZ];
	ഘടന pollfd fd[2];
	പൂർണ്ണസംഖ്യ s = 0;
	size_t bytes;
	FILE *log = ശൂന്യ;
	FILE *plog = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *logfile;
	അക്ഷരം *tmplog, *usrlog = ശൂന്യ;
	പൂർണ്ണസംഖ്യ log_error = 0;

	എങ്കിൽ (!rc_conf_yesno("rc_logger"))
		തിരികെ;

	എങ്കിൽ (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	എങ്കിൽ (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		എങ്കിൽ (openpty(&rc_logger_tty, &slave_tty, ശൂന്യ, &tt, &ws))
			തിരികെ;
	} അല്ലെങ്കിൽ
		എങ്കിൽ (openpty(&rc_logger_tty, &slave_tty, ശൂന്യ, ശൂന്യ, ശൂന്യ))
			തിരികെ;

	എങ്കിൽ ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	എങ്കിൽ ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	തിരഞ്ഞെടുപ്പ് (rc_logger_pid) {
	അവസ്ഥ -1:
		eerror("fork: %s", strerror(errno));
		നിർത്തുക;
	അവസ്ഥ 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		എങ്കിൽ ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		അല്ലെങ്കിൽ {
			സ്വതന്ത്രം(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(വലുപ്പം (അക്ഷരം) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		എങ്കിൽ (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		വേണ്ടി (;;) {
			എങ്കിൽ ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				നിർത്തുക;
			} അല്ലെങ്കിൽ എങ്കിൽ (s == 0)
				തുടരുക;

			എങ്കിൽ (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = പഠ(rc_logger_tty, buffer, BUFSIZ);
				എങ്കിൽ (ലിഖ(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				എങ്കിൽ (log)
					write_log(fileno (log), buffer, bytes);
				അല്ലെങ്കിൽ {
					എങ്കിൽ (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    വലുപ്പം(അക്ഷരം ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			എങ്കിൽ (fd[0].revents & (POLLIN | POLLHUP))
				നിർത്തുക;
		}
		എങ്കിൽ (logbuf) {
			എങ്കിൽ ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			സ്വതന്ത്രം(logbuf);
		}
		എങ്കിൽ (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_മൂല്യം("rc_log_path");
		എങ്കിൽ (logfile == ശൂന്യ) {
			എങ്കിൽ (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} അല്ലെങ്കിൽ {
				സ്ഥിരം അക്ഷരം *state_home = getenv("XDG_STATE_HOME");
				എങ്കിൽ (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} അല്ലെങ്കിൽ എങ്കിൽ ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		എങ്കിൽ (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		എങ്കിൽ ((plog = fopen(logfile, "ae"))) {
			എങ്കിൽ ((log = fopen(tmplog, "re"))) {
				ആകുമ്പോൾ ((bytes = fread(buffer, വലുപ്പം(*buffer), BUFSIZ, log)) > 0) {
					എങ്കിൽ (fwrite(buffer, വലുപ്പം(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						നിർത്തുക;
					}
				}
				fclose(log);
			} അല്ലെങ്കിൽ {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} അല്ലെങ്കിൽ {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			എങ്കിൽ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		സ്വതന്ത്രം(usrlog);

		/* Try to keep the temporary log in case of errors */
		എങ്കിൽ (!log_error) {
			എങ്കിൽ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				എങ്കിൽ (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} അല്ലെങ്കിൽ എങ്കിൽ (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		സ്വതന്ത്രം(tmplog);

		exit(0);
		/* NOTREACHED */

	സ്ഥിരസ്ഥിതി:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		എങ്കിൽ ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		എങ്കിൽ ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		എങ്കിൽ (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		നിർത്തുക;
	}
}
