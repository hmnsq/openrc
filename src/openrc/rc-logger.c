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
#وگرنہ_اگر defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#ورنہ
#  include <libutil.h>
#اگر_ختم

#شامل "einfo.h"
#شامل "rc-logger.h"
#شامل "rc.h"
#شامل "misc.h"
#شامل "helpers.h"

#تعریف_کرو DEFAULTLOG "/var/log/rc.log"

ساکن صحیح signal_pipe[2] = { -1, -1 };
ساکن صحیح fd_stdout = -1;
ساکن صحیح fd_stderr = -1;
ساکن ثابت حرف *runlevel = تہی;
ساکن bool اندر_escape = false;
ساکن bool اندر_رکن = false;

ساکن حرف *logbuf = تہی;
ساکن size_t logbuf_سائز = 0;
ساکن size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
صحیح rc_logger_tty = -1;
bool rc_اندر_logger = false;

ساکن خالی
write_log(صحیح logfd, ثابت حرف *بفر, size_t بائٹیں)
{
	ثابت حرف *p = بفر;

	جب ((size_t)(p - بفر) < بائٹیں) {
		انتخاب (*p) {
		حالت '\r':
			جاؤ cont;
		حالت '\033':
			اندر_escape = true;
			اندر_رکن = false;
			جاؤ cont;
		حالت '\n':
			اندر_escape = اندر_رکن = false;
			توڑو;
		حالت '[':
			اگر (اندر_escape)
				اندر_رکن = true;
			توڑو;
		}

		اگر (!اندر_escape) {
			اگر (!isprint((صحیح) *p) && *p != '\n')
				جاؤ cont;
			اگر (اكتب(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			جاری;
		}

		اگر (!اندر_رکن || حرف_ہے((غیرعلامتی حرف)*p))
			اندر_escape = اندر_رکن = false;
cont:
		p++;
	}
}

ساکن خالی
write_وقت(FILE *f, ثابت حرف *s)
{
	time_t now = time(تہی);
	ساخت tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

ساکن خالی
mkpath(ثابت حرف *راستہ)
{
	حرف *dir = xstrdup(راستہ), *slash = dir;
	جب ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		اگر (mkdir(راستہ, 0755) == -1 && errno != EEXIST)
			توڑو;
		*slash = '/';
	}
	mkdir(راستہ, 0755);
	آزاد(dir);
}

خالی
rc_logger_close(خالی)
{
	صحیح sig = SIGTERM;

	اگر (signal_pipe[1] > -1) {
		اگر (اكتب(signal_pipe[1], &sig, حجم(sig)) == -1)
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

خالی
rc_logger_open(ثابت حرف *level)
{
	صحیح slave_tty;
	ساخت termios tt;
	ساخت winsize ws;
	حرف بفر[BUFSIZ];
	ساخت pollfd fd[2];
	صحیح s = 0;
	size_t بائٹیں;
	FILE *لوگارتھم = تہی;
	FILE *plog = تہی;
	ثابت حرف *logfile;
	حرف *tmplog, *usrlog = تہی;
	صحیح log_error = 0;

	اگر (!rc_conf_yesno("rc_logger"))
		واپس;

	اگر (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	اگر (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		اگر (openpty(&rc_logger_tty, &slave_tty, تہی, &tt, &ws))
			واپس;
	} ورنہ
		اگر (openpty(&rc_logger_tty, &slave_tty, تہی, تہی, تہی))
			واپس;

	اگر ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	اگر ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	انتخاب (rc_logger_pid) {
	حالت -1:
		eerror("fork: %s", strerror(errno));
		توڑو;
	حالت 0:
		rc_اندر_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		اگر ((لوگارتھم = fopen(tmplog, "ae")))
			write_وقت(لوگارتھم, "started");
		ورنہ {
			آزاد(logbuf);
			logbuf_سائز = BUFSIZ * 10;
			logbuf = xmalloc(حجم (حرف) * logbuf_سائز);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		اگر (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		برائے (;;) {
			اگر ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				توڑو;
			} ورنہ اگر (s == 0)
				جاری;

			اگر (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(بفر, 0, BUFSIZ);
				بائٹیں = اقرأ(rc_logger_tty, بفر, BUFSIZ);
				اگر (اكتب(STDOUT_FILENO, بفر, بائٹیں) == -1)
					eerror("write: %s", strerror(errno));

				اگر (لوگارتھم)
					write_log(fileno (لوگارتھم), بفر, بائٹیں);
				ورنہ {
					اگر (logbuf_سائز - logbuf_len < بائٹیں) {
						logbuf_سائز += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    حجم(حرف ) *
						    logbuf_سائز);
					}

					حافظہ_نقل(logbuf + logbuf_len,
					    بفر, بائٹیں);
					logbuf_len += بائٹیں;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			اگر (fd[0].revents & (POLLIN | POLLHUP))
				توڑو;
		}
		اگر (logbuf) {
			اگر ((لوگارتھم = fopen(tmplog, "ae"))) {
				write_وقت(لوگارتھم, "started");
				write_log(fileno(لوگارتھم), logbuf, logbuf_len);
			}
			آزاد(logbuf);
		}
		اگر (لوگارتھم) {
			write_وقت(لوگارتھم, "stopped");
			fclose(لوگارتھم);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_قدر("rc_log_path");
		اگر (logfile == تہی) {
			اگر (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} ورنہ {
				ثابت حرف *state_home = getenv("XDG_STATE_HOME");
				اگر (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} ورنہ اگر ((state_home = getenv("HOME"))) {
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
			اگر ((لوگارتھم = fopen(tmplog, "re"))) {
				جب ((بائٹیں = fread(بفر, حجم(*بفر), BUFSIZ, لوگارتھم)) > 0) {
					اگر (fwrite(بفر, حجم(*بفر), بائٹیں, plog) < بائٹیں) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						توڑو;
					}
				}
				fclose(لوگارتھم);
			} ورنہ {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} ورنہ {
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
		} ورنہ اگر (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		آزاد(tmplog);

		باہر_نکلو(0);
		/* NOTREACHED */

	طےشدہ:
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
		توڑو;
	}
}
