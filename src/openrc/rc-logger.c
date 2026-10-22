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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/ioctl.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <termios.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <unistd.h>

#જો defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#અથવાજો defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#નહીંતર
#  include <libutil.h>
#endif

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "rc-logger.h"
#અન્તર્ભાવ "rc.h"
#અન્તર્ભાવ "misc.h"
#અન્તર્ભાવ "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

સ્થિર પૂર્ણાંક signal_pipe[2] = { -1, -1 };
સ્થિર પૂર્ણાંક fd_stdout = -1;
સ્થિર પૂર્ણાંક fd_stderr = -1;
સ્થિર અચલ અક્ષર *runlevel = ખાલી;
સ્થિર bool in_escape = false;
સ્થિર bool in_term = false;

સ્થિર અક્ષર *logbuf = ખાલી;
સ્થિર size_t logbuf_size = 0;
સ્થિર size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
પૂર્ણાંક rc_logger_tty = -1;
bool rc_in_logger = false;

સ્થિર શૂન્ય
write_log(પૂર્ણાંક logfd, અચલ અક્ષર *buffer, size_t bytes)
{
	અચલ અક્ષર *p = buffer;

	જ્યારે ((size_t)(p - buffer) < bytes) {
		પસંદગી (*p) {
		સ્થિતિ '\r':
			જાઓ cont;
		સ્થિતિ '\033':
			in_escape = true;
			in_term = false;
			જાઓ cont;
		સ્થિતિ '\n':
			in_escape = in_term = false;
			તોડો;
		સ્થિતિ '[':
			જો (in_escape)
				in_term = true;
			તોડો;
		}

		જો (!in_escape) {
			જો (!isprint((પૂર્ણાંક) *p) && *p != '\n')
				જાઓ cont;
			જો (લિખ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			ચાલુ;
		}

		જો (!in_term || isalpha((unsigned અક્ષર)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

સ્થિર શૂન્ય
write_time(FILE *f, અચલ અક્ષર *s)
{
	time_t now = time(ખાલી);
	સંરચના tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

સ્થિર શૂન્ય
mkpath(અચલ અક્ષર *path)
{
	અક્ષર *dir = xstrdup(path), *slash = dir;
	જ્યારે ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		જો (mkdir(path, 0755) == -1 && errno != EEXIST)
			તોડો;
		*slash = '/';
	}
	mkdir(path, 0755);
	મુક્ત(dir);
}

શૂન્ય
rc_logger_close(શૂન્ય)
{
	પૂર્ણાંક sig = SIGTERM;

	જો (signal_pipe[1] > -1) {
		જો (લિખ(signal_pipe[1], &sig, કદ(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	જો (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	જો (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	જો (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

શૂન્ય
rc_logger_open(અચલ અક્ષર *level)
{
	પૂર્ણાંક slave_tty;
	સંરચના termios tt;
	સંરચના winsize ws;
	અક્ષર buffer[BUFSIZ];
	સંરચના pollfd fd[2];
	પૂર્ણાંક s = 0;
	size_t bytes;
	FILE *log = ખાલી;
	FILE *plog = ખાલી;
	અચલ અક્ષર *logfile;
	અક્ષર *tmplog, *usrlog = ખાલી;
	પૂર્ણાંક log_error = 0;

	જો (!rc_conf_yesno("rc_logger"))
		પાછા;

	જો (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	જો (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		જો (openpty(&rc_logger_tty, &slave_tty, ખાલી, &tt, &ws))
			પાછા;
	} નહીંતર
		જો (openpty(&rc_logger_tty, &slave_tty, ખાલી, ખાલી, ખાલી))
			પાછા;

	જો ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	જો ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	પસંદગી (rc_logger_pid) {
	સ્થિતિ -1:
		eerror("fork: %s", strerror(errno));
		તોડો;
	સ્થિતિ 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		જો ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		નહીંતર {
			મુક્ત(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(કદ (અક્ષર) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		જો (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		માટે (;;) {
			જો ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				તોડો;
			} નહીંતર જો (s == 0)
				ચાલુ;

			જો (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = પઠ(rc_logger_tty, buffer, BUFSIZ);
				જો (લિખ(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				જો (log)
					write_log(fileno (log), buffer, bytes);
				નહીંતર {
					જો (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    કદ(અક્ષર ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			જો (fd[0].revents & (POLLIN | POLLHUP))
				તોડો;
		}
		જો (logbuf) {
			જો ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			મુક્ત(logbuf);
		}
		જો (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_કિંમત("rc_log_path");
		જો (logfile == ખાલી) {
			જો (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} નહીંતર {
				અચલ અક્ષર *state_home = getenv("XDG_STATE_HOME");
				જો (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} નહીંતર જો ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		જો (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		જો ((plog = fopen(logfile, "ae"))) {
			જો ((log = fopen(tmplog, "re"))) {
				જ્યારે ((bytes = fread(buffer, કદ(*buffer), BUFSIZ, log)) > 0) {
					જો (fwrite(buffer, કદ(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						તોડો;
					}
				}
				fclose(log);
			} નહીંતર {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} નહીંતર {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			જો (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		મુક્ત(usrlog);

		/* Try to keep the temporary log in case of errors */
		જો (!log_error) {
			જો (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				જો (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} નહીંતર જો (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		મુક્ત(tmplog);

		exit(0);
		/* NOTREACHED */

	મૂળભૂત:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		જો ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		જો ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		જો (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		તોડો;
	}
}
