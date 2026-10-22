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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <poll.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/ioctl.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <termios.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଯଦି defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#ନଚେତଯଦି defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ଅନ୍ୟଥା
#  include <libutil.h>
#endif

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "rc-logger.h"
#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "misc.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା signal_pipe[2] = { -1, -1 };
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd_stdout = -1;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd_stderr = -1;
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *runlevel = ଖାଲି;
ସ୍ଥିର bool in_escape = false;
ସ୍ଥିର bool in_term = false;

ସ୍ଥିର ଅକ୍ଷର *logbuf = ଖାଲି;
ସ୍ଥିର size_t logbuf_size = 0;
ସ୍ଥିର size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc_logger_tty = -1;
bool rc_in_logger = false;

ସ୍ଥିର ଶୂନ୍ୟ
write_log(ପୂର୍ଣ୍ଣସଂଖ୍ୟା logfd, ଅଚଳ ଅକ୍ଷର *buffer, size_t bytes)
{
	ଅଚଳ ଅକ୍ଷର *p = buffer;

	ଯେତେବେଳେ ((size_t)(p - buffer) < bytes) {
		ବାଛ (*p) {
		ସ୍ଥିତି '\r':
			ଯାଅ cont;
		ସ୍ଥିତି '\033':
			in_escape = true;
			in_term = false;
			ଯାଅ cont;
		ସ୍ଥିତି '\n':
			in_escape = in_term = false;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '[':
			ଯଦି (in_escape)
				in_term = true;
			ଭାଙ୍ଗ;
		}

		ଯଦି (!in_escape) {
			ଯଦି (!isprint((ପୂର୍ଣ୍ଣସଂଖ୍ୟା) *p) && *p != '\n')
				ଯାଅ cont;
			ଯଦି (ଲିଖ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			ଜାରି;
		}

		ଯଦି (!in_term || isalpha((unsigned ଅକ୍ଷର)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ
write_time(FILE *f, ଅଚଳ ଅକ୍ଷର *s)
{
	time_t now = time(ଖାଲି);
	ସଂରଚନା tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ସ୍ଥିର ଶୂନ୍ୟ
mkpath(ଅଚଳ ଅକ୍ଷର *path)
{
	ଅକ୍ଷର *dir = xstrdup(path), *slash = dir;
	ଯେତେବେଳେ ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		ଯଦି (mkdir(path, 0755) == -1 && errno != EEXIST)
			ଭାଙ୍ଗ;
		*slash = '/';
	}
	mkdir(path, 0755);
	ମୁକ୍ତ(dir);
}

ଶୂନ୍ୟ
rc_logger_close(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = SIGTERM;

	ଯଦି (signal_pipe[1] > -1) {
		ଯଦି (ଲିଖ(signal_pipe[1], &sig, ଆକାର(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	ଯଦି (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	ଯଦି (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	ଯଦି (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

ଶୂନ୍ୟ
rc_logger_open(ଅଚଳ ଅକ୍ଷର *level)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା slave_tty;
	ସଂରଚନା termios tt;
	ସଂରଚନା winsize ws;
	ଅକ୍ଷର buffer[BUFSIZ];
	ସଂରଚନା pollfd fd[2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା s = 0;
	size_t bytes;
	FILE *log = ଖାଲି;
	FILE *plog = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *logfile;
	ଅକ୍ଷର *tmplog, *usrlog = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା log_error = 0;

	ଯଦି (!rc_conf_yesno("rc_logger"))
		ଫେରନ୍ତୁ;

	ଯଦି (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	ଯଦି (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		ଯଦି (openpty(&rc_logger_tty, &slave_tty, ଖାଲି, &tt, &ws))
			ଫେରନ୍ତୁ;
	} ଅନ୍ୟଥା
		ଯଦି (openpty(&rc_logger_tty, &slave_tty, ଖାଲି, ଖାଲି, ଖାଲି))
			ଫେରନ୍ତୁ;

	ଯଦି ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	ଯଦି ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	ବାଛ (rc_logger_pid) {
	ସ୍ଥିତି -1:
		eerror("fork: %s", strerror(errno));
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 0:
		rc_in_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		ଯଦି ((log = fopen(tmplog, "ae")))
			write_time(log, "started");
		ଅନ୍ୟଥା {
			ମୁକ୍ତ(logbuf);
			logbuf_size = BUFSIZ * 10;
			logbuf = xmalloc(ଆକାର (ଅକ୍ଷର) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		ଯଦି (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		ପାଇଁ (;;) {
			ଯଦି ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା ଯଦି (s == 0)
				ଜାରି;

			ଯଦି (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(buffer, 0, BUFSIZ);
				bytes = ପଠ(rc_logger_tty, buffer, BUFSIZ);
				ଯଦି (ଲିଖ(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", strerror(errno));

				ଯଦି (log)
					write_log(fileno (log), buffer, bytes);
				ଅନ୍ୟଥା {
					ଯଦି (logbuf_size - logbuf_len < bytes) {
						logbuf_size += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    ଆକାର(ଅକ୍ଷର ) *
						    logbuf_size);
					}

					memcpy(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			ଯଦି (fd[0].revents & (POLLIN | POLLHUP))
				ଭାଙ୍ଗ;
		}
		ଯଦି (logbuf) {
			ଯଦି ((log = fopen(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			ମୁକ୍ତ(logbuf);
		}
		ଯଦି (log) {
			write_time(log, "stopped");
			fclose(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_ମୂଲ୍ୟ("rc_log_path");
		ଯଦି (logfile == ଖାଲି) {
			ଯଦି (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} ଅନ୍ୟଥା {
				ଅଚଳ ଅକ୍ଷର *state_home = getenv("XDG_STATE_HOME");
				ଯଦି (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} ଅନ୍ୟଥା ଯଦି ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		ଯଦି (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		ଯଦି ((plog = fopen(logfile, "ae"))) {
			ଯଦି ((log = fopen(tmplog, "re"))) {
				ଯେତେବେଳେ ((bytes = fread(buffer, ଆକାର(*buffer), BUFSIZ, log)) > 0) {
					ଯଦି (fwrite(buffer, ଆକାର(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						ଭାଙ୍ଗ;
					}
				}
				fclose(log);
			} ଅନ୍ୟଥା {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} ଅନ୍ୟଥା {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			ଯଦି (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		ମୁକ୍ତ(usrlog);

		/* Try to keep the temporary log in case of errors */
		ଯଦି (!log_error) {
			ଯଦି (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				ଯଦି (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} ଅନ୍ୟଥା ଯଦି (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		ମୁକ୍ତ(tmplog);

		exit(0);
		/* NOTREACHED */

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		ଯଦି ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		ଯଦି ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		ଯଦି (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		ଭାଙ୍ଗ;
	}
}
