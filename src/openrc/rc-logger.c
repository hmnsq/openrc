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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <poll.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <termios.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <unistd.h>

#যদি defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#নোচেদ্যদি defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#অন্যথা
#  include <libutil.h>
#endif

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "rc-logger.h"
#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "misc.h"
#অন্তর্ভাব "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

স্থির পূর্ণাঙ্ক signal_pipe[2] = { -1, -1 };
স্থির পূর্ণাঙ্ক fd_stdout = -1;
স্থির পূর্ণাঙ্ক fd_stderr = -1;
স্থির ধ্রুব বর্ণ *runlevel = রিক্ত;
স্থির bool in_escape = false;
স্থির bool in_term = false;

স্থির বর্ণ *logbuf = রিক্ত;
স্থির size_t logbuf_size = 0;
স্থির size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
পূর্ণাঙ্ক rc_logger_tty = -1;
bool rc_in_logger = false;

স্থির শূন্য
write_log(পূর্ণাঙ্ক logfd, ধ্রুব বর্ণ *buffer, size_t bytes)
{
	ধ্রুব বর্ণ *p = buffer;

	যাবত্ ((size_t)(p - buffer) < bytes) {
		বিকল্প (*p) {
		পক্ষ '\r':
			গচ্ছ cont;
		পক্ষ '\033':
			in_escape = true;
			in_term = false;
			গচ্ছ cont;
		পক্ষ '\n':
			in_escape = in_term = false;
			ভঙ্গ;
		পক্ষ '[':
			যদি (in_escape)
				in_term = true;
			ভঙ্গ;
		}

		যদি (!in_escape) {
			যদি (!isprint((পূর্ণাঙ্ক) *p) && *p != '\n')
				গচ্ছ cont;
			যদি (লিখ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			অনুবর্ত;
		}

		যদি (!in_term || isalpha((unsigned বর্ণ)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

স্থির শূন্য
write_time(FILE *f, ধ্রুব বর্ণ *s)
{
	time_t now = time(রিক্ত);
	সংরচনা tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

স্থির শূন্য
mkpath(ধ্রুব বর্ণ *path)
{
	বর্ণ *dir = xstrdup(path), *slash = dir;
	যাবত্ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		যদি (mkdir(path, 0755) == -1 && errno != EEXIST)
			ভঙ্গ;
		*slash = '/';
	}
	mkdir(path, 0755);
	মুক্ত(dir);
}

শূন্য
rc_logger_close(শূন্য)
{
	পূর্ণাঙ্ক sig = SIGTERM;

	যদি (signal_pipe[1] > -1) {
		যদি (লিখ(signal_pipe[1], &sig, পরিমাণ(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	যদি (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	যদি (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	যদি (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

শূন্য
rc_logger_open(ধ্রুব বর্ণ *level)
{
	পূর্ণাঙ্ক slave_tty;
	সংরচনা termios tt;
	সংরচনা winsize ws;
	বর্ণ buffer[BUFSIZ];
	সংরচনা pollfd fd[2];
	পূর্ণাঙ্ক s = 0;
	size_t bytes;
	FILE *log = রিক্ত;
	FILE *plog = রিক্ত;
	ধ্রুব বর্ণ *logfile;
	বর্ণ *tmplog, *usrlog = রিক্ত;
	পূর্ণাঙ্ক log_error = 0;

	যদি (!rc_conf_yesno("rc_logger"))
		প্রত্যাগম;

	যদি (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	যদি (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		যদি (openpty(&rc_logger_tty, &slave_tty, রিক্ত, &tt, &ws))
			প্রত্যাগম;
	} অন্যথা
		যদি (openpty(&rc_logger_tty, &slave_tty, রিক্ত, রিক্ত, রিক্ত))
			প্রত্যাগম;

	যদি ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	যদি ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	বিকল্প (rc_logger_pid) {
	পক্ষ -1:
		eerror("fork: %s", strerror(errno));
		ভঙ্গ;
	পক্ষ 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		যদি ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		অন্যথা {
			মুক্ত(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(পরিমাণ (বর্ণ) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		যদি (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		প্রতি (;;) {
			যদি ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				ভঙ্গ;
			} অন্যথা যদি (s == 0)
				অনুবর্ত;

			যদি (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = পঠ(rc_logger_tty, buffer, BUFSIZ);
				যদি (লিখ(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				যদি (log)
					write_log(fileno (log), buffer, bytes);
				অন্যথা {
					যদি (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    পরিমাণ(বর্ণ ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			যদি (fd[0].revents & (POLLIN | POLLHUP))
				ভঙ্গ;
		}
		যদি (logbuf) {
			যদি ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			মুক্ত(logbuf);
		}
		যদি (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_মান("rc_log_path");
		যদি (logfile == রিক্ত) {
			যদি (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} অন্যথা {
				ধ্রুব বর্ণ *state_home = getenv("XDG_STATE_HOME");
				যদি (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} অন্যথা যদি ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		যদি (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		যদি ((plog = fopen(logfile, "ae"))) {
			যদি ((log = fopen(tmplog, "re"))) {
				যাবত্ ((bytes = fread(buffer, পরিমাণ(*buffer), BUFSIZ, log)) > 0) {
					যদি (fwrite(buffer, পরিমাণ(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						ভঙ্গ;
					}
				}
				fclose(log);
			} অন্যথা {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} অন্যথা {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			যদি (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		মুক্ত(usrlog);

		/* Try to keep the temporary log in case of errors */
		যদি (!log_error) {
			যদি (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				যদি (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} অন্যথা যদি (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		মুক্ত(tmplog);

		exit(0);
		/* NOTREACHED */

	স্বভাব:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		যদি ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		যদি ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		যদি (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		ভঙ্গ;
	}
}
