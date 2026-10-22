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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <ctype.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <poll.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <termios.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <unistd.h>

#ఒకవేళ defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#లేదావేళ defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#లేకపోతే
#  include <libutil.h>
#అయితే_ముగింపు

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "rc-logger.h"
#అన్తర్భావ "rc.h"
#అన్తర్భావ "misc.h"
#అన్తర్భావ "helpers.h"

#నిర్వచించు DEFAULTLOG "/var/log/rc.log"

స్థిర పూర్ణాంకం signal_pipe[2] = { -1, -1 };
స్థిర పూర్ణాంకం fd_stdout = -1;
స్థిర పూర్ణాంకం fd_stderr = -1;
స్థిర స్థిరరాశి అక్షరం *runlevel = ఖాళీ;
స్థిర bool లోపల_escape = false;
స్థిర bool లోపల_పదం = false;

స్థిర అక్షరం *logbuf = ఖాళీ;
స్థిర size_t logbuf_కొలత = 0;
స్థిర size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
పూర్ణాంకం rc_logger_tty = -1;
bool rc_లోపల_logger = false;

స్థిర శూన్య
write_log(పూర్ణాంకం logfd, స్థిరరాశి అక్షరం *బఫరు, size_t బైట్లు)
{
	స్థిరరాశి అక్షరం *p = బఫరు;

	అయితే ((size_t)(p - బఫరు) < బైట్లు) {
		ఎంపిక (*p) {
		స్థితి '\r':
			వెళ్ళు cont;
		స్థితి '\033':
			లోపల_escape = true;
			లోపల_పదం = false;
			వెళ్ళు cont;
		స్థితి '\n':
			లోపల_escape = లోపల_పదం = false;
			ఆపు;
		స్థితి '[':
			ఒకవేళ (లోపల_escape)
				లోపల_పదం = true;
			ఆపు;
		}

		ఒకవేళ (!లోపల_escape) {
			ఒకవేళ (!isprint((పూర్ణాంకం) *p) && *p != '\n')
				వెళ్ళు cont;
			ఒకవేళ (లిఖ(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			కొనసాగు;
		}

		ఒకవేళ (!లోపల_పదం || అక్షరమా((గుర్తులేని అక్షరం)*p))
			లోపల_escape = లోపల_పదం = false;
cont:
		p++;
	}
}

స్థిర శూన్య
write_సమయం(FILE *f, స్థిరరాశి అక్షరం *s)
{
	time_t now = time(ఖాళీ);
	నిర్మాణం tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

స్థిర శూన్య
mkpath(స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం *dir = xstrdup(మార్గం), *slash = dir;
	అయితే ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		ఒకవేళ (mkdir(మార్గం, 0755) == -1 && errno != EEXIST)
			ఆపు;
		*slash = '/';
	}
	mkdir(మార్గం, 0755);
	విడుదల(dir);
}

శూన్య
rc_logger_close(శూన్య)
{
	పూర్ణాంకం sig = SIGTERM;

	ఒకవేళ (signal_pipe[1] > -1) {
		ఒకవేళ (లిఖ(signal_pipe[1], &sig, పరిమాణం(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	ఒకవేళ (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	ఒకవేళ (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	ఒకవేళ (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

శూన్య
rc_logger_open(స్థిరరాశి అక్షరం *level)
{
	పూర్ణాంకం slave_tty;
	నిర్మాణం termios tt;
	నిర్మాణం winsize ws;
	అక్షరం బఫరు[BUFSIZ];
	నిర్మాణం pollfd fd[2];
	పూర్ణాంకం s = 0;
	size_t బైట్లు;
	FILE *సంవర్గమానం = ఖాళీ;
	FILE *plog = ఖాళీ;
	స్థిరరాశి అక్షరం *logfile;
	అక్షరం *tmplog, *usrlog = ఖాళీ;
	పూర్ణాంకం log_error = 0;

	ఒకవేళ (!rc_conf_yesno("rc_logger"))
		తిరిగి;

	ఒకవేళ (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	ఒకవేళ (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		ఒకవేళ (openpty(&rc_logger_tty, &slave_tty, ఖాళీ, &tt, &ws))
			తిరిగి;
	} లేకపోతే
		ఒకవేళ (openpty(&rc_logger_tty, &slave_tty, ఖాళీ, ఖాళీ, ఖాళీ))
			తిరిగి;

	ఒకవేళ ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	ఒకవేళ ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	ఎంపిక (rc_logger_pid) {
	స్థితి -1:
		eerror("fork: %s", strerror(errno));
		ఆపు;
	స్థితి 0:
		rc_లోపల_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		ఒకవేళ ((సంవర్గమానం = fopen(tmplog, "ae")))
			write_సమయం(సంవర్గమానం, "started");
		లేకపోతే {
			విడుదల(logbuf);
			logbuf_కొలత = BUFSIZ * 10;
			logbuf = xmalloc(పరిమాణం (అక్షరం) * logbuf_కొలత);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		ఒకవేళ (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		కొరకు (;;) {
			ఒకవేళ ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				ఆపు;
			} లేకపోతే ఒకవేళ (s == 0)
				కొనసాగు;

			ఒకవేళ (fd[1].revents & (POLLIN | POLLHUP)) {
				memset(బఫరు, 0, BUFSIZ);
				బైట్లు = పఠ(rc_logger_tty, బఫరు, BUFSIZ);
				ఒకవేళ (లిఖ(STDOUT_FILENO, బఫరు, బైట్లు) == -1)
					eerror("write: %s", strerror(errno));

				ఒకవేళ (సంవర్గమానం)
					write_log(fileno (సంవర్గమానం), బఫరు, బైట్లు);
				లేకపోతే {
					ఒకవేళ (logbuf_కొలత - logbuf_len < బైట్లు) {
						logbuf_కొలత += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    పరిమాణం(అక్షరం ) *
						    logbuf_కొలత);
					}

					జ్ఞాపకం_నకలు(logbuf + logbuf_len,
					    బఫరు, బైట్లు);
					logbuf_len += బైట్లు;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			ఒకవేళ (fd[0].revents & (POLLIN | POLLHUP))
				ఆపు;
		}
		ఒకవేళ (logbuf) {
			ఒకవేళ ((సంవర్గమానం = fopen(tmplog, "ae"))) {
				write_సమయం(సంవర్గమానం, "started");
				write_log(fileno(సంవర్గమానం), logbuf, logbuf_len);
			}
			విడుదల(logbuf);
		}
		ఒకవేళ (సంవర్గమానం) {
			write_సమయం(సంవర్గమానం, "stopped");
			fclose(సంవర్గమానం);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_విలువ("rc_log_path");
		ఒకవేళ (logfile == ఖాళీ) {
			ఒకవేళ (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} లేకపోతే {
				స్థిరరాశి అక్షరం *state_home = getenv("XDG_STATE_HOME");
				ఒకవేళ (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} లేకపోతే ఒకవేళ ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		ఒకవేళ (!strcmp(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		ఒకవేళ ((plog = fopen(logfile, "ae"))) {
			ఒకవేళ ((సంవర్గమానం = fopen(tmplog, "re"))) {
				అయితే ((బైట్లు = fread(బఫరు, పరిమాణం(*బఫరు), BUFSIZ, సంవర్గమానం)) > 0) {
					ఒకవేళ (fwrite(బఫరు, పరిమాణం(*బఫరు), బైట్లు, plog) < బైట్లు) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						ఆపు;
					}
				}
				fclose(సంవర్గమానం);
			} లేకపోతే {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} లేకపోతే {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			ఒకవేళ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		విడుదల(usrlog);

		/* Try to keep the temporary log in case of errors */
		ఒకవేళ (!log_error) {
			ఒకవేళ (errno != EROFS && ((strcmp(level, RC_LEVEL_SHUTDOWN) != 0) && (strcmp(level, RC_LEVEL_SYSINIT) != 0)))
				ఒకవేళ (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} లేకపోతే ఒకవేళ (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		విడుదల(tmplog);

		నిష్క్రమించు(0);
		/* NOTREACHED */

	అప్రమేయ:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		ఒకవేళ ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		ఒకవేళ ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		ఒకవేళ (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		ఆపు;
	}
}
