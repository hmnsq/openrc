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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <poll.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/ioctl.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <termios.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <unistd.h>

#எனில் defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#அல்லதெனில் defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#இல்லையெனில்
#  include <libutil.h>
#endif

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "rc-logger.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

நிலையான முழு signal_pipe[2] = { -1, -1 };
நிலையான முழு fd_stdout = -1;
நிலையான முழு fd_stderr = -1;
நிலையான மாறிலி எழுத்து *runlevel = வெற்று;
நிலையான bool in_escape = false;
நிலையான bool in_term = false;

நிலையான எழுத்து *logbuf = வெற்று;
நிலையான size_t logbuf_size = 0;
நிலையான size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
முழு rc_logger_tty = -1;
bool rc_in_logger = false;

நிலையான வெறுமை
write_log(முழு logfd, மாறிலி எழுத்து *buffer, size_t bytes)
{
	மாறிலி எழுத்து *p = buffer;

	போது ((size_t)(p - buffer) < bytes) {
		தேர்வு (*p) {
		நிலை '\r':
			செல் cont;
		நிலை '\033':
			in_escape = true;
			in_term = false;
			செல் cont;
		நிலை '\n':
			in_escape = in_term = false;
			நிறுத்து;
		நிலை '[':
			எனில் (in_escape)
				in_term = true;
			நிறுத்து;
		}

		எனில் (!in_escape) {
			எனில் (!isprint((முழு) *p) && *p != '\n')
				செல் cont;
			எனில் (லிக(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			தொடர்;
		}

		எனில் (!in_term || isalpha((unsigned எழுத்து)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

நிலையான வெறுமை
write_time(FILE *f, மாறிலி எழுத்து *s)
{
	time_t now = time(வெற்று);
	கட்டமைப்பு tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

நிலையான வெறுமை
mkpath(மாறிலி எழுத்து *path)
{
	எழுத்து *dir = xstrdup(path), *slash = dir;
	போது ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		எனில் (mkdir(path, 0755) == -1 && errno != EEXIST)
			நிறுத்து;
		*slash = '/';
	}
	mkdir(path, 0755);
	விடுவி(dir);
}

வெறுமை
rc_logger_close(வெறுமை)
{
	முழு sig = SIGTERM;

	எனில் (signal_pipe[1] > -1) {
		எனில் (லிக(signal_pipe[1], &sig, அளவு(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	எனில் (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	எனில் (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	எனில் (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

வெறுமை
rc_logger_open(மாறிலி எழுத்து *level)
{
	முழு slave_tty;
	கட்டமைப்பு termios tt;
	கட்டமைப்பு winsize ws;
	எழுத்து buffer[BUFSIZ];
	கட்டமைப்பு pollfd fd[2];
	முழு s = 0;
	size_t bytes;
	FILE *log = வெற்று;
	FILE *plog = வெற்று;
	மாறிலி எழுத்து *logfile;
	எழுத்து *tmplog, *usrlog = வெற்று;
	முழு log_error = 0;

	எனில் (!rc_conf_yesno("rc_logger"))
		திரும்பு;

	எனில் (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	எனில் (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		எனில் (openpty(&rc_logger_tty, &slave_tty, வெற்று, &tt, &ws))
			திரும்பு;
	} இல்லையெனில்
		எனில் (openpty(&rc_logger_tty, &slave_tty, வெற்று, வெற்று, வெற்று))
			திரும்பு;

	எனில் ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	எனில் ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	தேர்வு (rc_logger_pid) {
	நிலை -1:
		eerror("fork: %s", strerror(errno));
		நிறுத்து;
	நிலை 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		எனில் ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		இல்லையெனில் {
			விடுவி(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(அளவு (எழுத்து) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		எனில் (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		க்கு (;;) {
			எனில் ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				நிறுத்து;
			} இல்லையெனில் எனில் (s == 0)
				தொடர்;

			எனில் (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = பட(rc_logger_tty, buffer, BUFSIZ);
				எனில் (லிக(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				எனில் (log)
					write_log(fileno (log), buffer, bytes);
				இல்லையெனில் {
					எனில் (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    அளவு(எழுத்து ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			எனில் (fd[0].revents & (POLLIN | POLLHUP))
				நிறுத்து;
		}
		எனில் (logbuf) {
			எனில் ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			விடுவி(logbuf);
		}
		எனில் (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_மதிப்பு("rc_log_path");
		எனில் (logfile == வெற்று) {
			எனில் (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} இல்லையெனில் {
				மாறிலி எழுத்து *state_home = getenv("XDG_STATE_HOME");
				எனில் (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} இல்லையெனில் எனில் ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		எனில் (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		எனில் ((plog = fopen(logfile, "ae"))) {
			எனில் ((log = fopen(tmplog, "re"))) {
				போது ((bytes = fread(buffer, அளவு(*buffer), BUFSIZ, log)) > 0) {
					எனில் (fwrite(buffer, அளவு(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						நிறுத்து;
					}
				}
				fclose(log);
			} இல்லையெனில் {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} இல்லையெனில் {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			எனில் (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		விடுவி(usrlog);

		/* Try to keep the temporary log in case of errors */
		எனில் (!log_error) {
			எனில் (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				எனில் (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} இல்லையெனில் எனில் (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		விடுவி(tmplog);

		exit(0);
		/* NOTREACHED */

	இயல்பு:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		எனில் ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		எனில் ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		எனில் (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		நிறுத்து;
	}
}
