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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <poll.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/ioctl.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <termios.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <unistd.h>

#यदि defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#नोचेद्यदि defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#अन्यथा
#  include <libutil.h>
#यदि_अन्तः

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc-logger.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "helpers.h"

#परिभाषय DEFAULTLOG "/var/log/rc.log"

स्थिर पूर्णाङ्क signal_pipe[2] = { -1, -1 };
स्थिर पूर्णाङ्क fd_stdout = -1;
स्थिर पूर्णाङ्क fd_stderr = -1;
स्थिर ध्रुव वर्ण *runlevel = रिक्त;
स्थिर bool अन्तर्_escape = false;
स्थिर bool अन्तर्_पदांशः = false;

स्थिर वर्ण *logbuf = रिक्त;
स्थिर size_t logbuf_आकार = 0;
स्थिर size_t logbuf_len = 0;

pid_t rc_logger_pid = -1;
पूर्णाङ्क rc_logger_tty = -1;
bool rc_अन्तर्_logger = false;

स्थिर शून्य
write_log(पूर्णाङ्क logfd, ध्रुव वर्ण *बफर, size_t अष्टकानि)
{
	ध्रुव वर्ण *p = बफर;

	यावत् ((size_t)(p - बफर) < अष्टकानि) {
		विकल्प (*p) {
		पक्ष '\r':
			गच्छ cont;
		पक्ष '\033':
			अन्तर्_escape = true;
			अन्तर्_पदांशः = false;
			गच्छ cont;
		पक्ष '\n':
			अन्तर्_escape = अन्तर्_पदांशः = false;
			भङ्ग;
		पक्ष '[':
			यदि (अन्तर्_escape)
				अन्तर्_पदांशः = true;
			भङ्ग;
		}

		यदि (!अन्तर्_escape) {
			यदि (!isprint((पूर्णाङ्क) *p) && *p != '\n')
				गच्छ cont;
			यदि (लिख(logfd, p++, 1) == -1)
				eerror("write: %s", strerror(errno));
			अनुवर्त;
		}

		यदि (!अन्तर्_पदांशः || अक्षरम्_वा((अचिह्नित वर्ण)*p))
			अन्तर्_escape = अन्तर्_पदांशः = false;
cont:
		p++;
	}
}

स्थिर शून्य
write_time(FILE *f, ध्रुव वर्ण *s)
{
	time_t now = time(रिक्त);
	संरचना tm *tm = localtime(&now);

	fprintf(f, "\nrc %s logging %s at %s\n", runlevel, s, asctime(tm));
	fflush(f);
}

स्थिर शून्य
mkpath(ध्रुव वर्ण *मार्गः)
{
	वर्ण *dir = xstrdup(मार्गः), *slash = dir;
	यावत् ((slash = strchr(slash + 1, '/'))) {
		*slash = '\0';
		यदि (mkdir(मार्गः, 0755) == -1 && errno != EEXIST)
			भङ्ग;
		*slash = '/';
	}
	mkdir(मार्गः, 0755);
	मुक्त(dir);
}

शून्य
rc_logger_close(शून्य)
{
	पूर्णाङ्क sig = SIGTERM;

	यदि (signal_pipe[1] > -1) {
		यदि (लिख(signal_pipe[1], &sig, परिमाण(sig)) == -1)
			eerror("write: %s", strerror(errno));
		close(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	यदि (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	यदि (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	यदि (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

शून्य
rc_logger_open(ध्रुव वर्ण *level)
{
	पूर्णाङ्क slave_tty;
	संरचना termios tt;
	संरचना winsize ws;
	वर्ण बफर[BUFSIZ];
	संरचना pollfd fd[2];
	पूर्णाङ्क s = 0;
	size_t अष्टकानि;
	FILE *लघुगणकः = रिक्त;
	FILE *plog = रिक्त;
	ध्रुव वर्ण *logfile;
	वर्ण *tmplog, *usrlog = रिक्त;
	पूर्णाङ्क log_error = 0;

	यदि (!rc_conf_yesno("rc_logger"))
		प्रत्यागम;

	यदि (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", strerror(errno));

	यदि (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		यदि (openpty(&rc_logger_tty, &slave_tty, रिक्त, &tt, &ws))
			प्रत्यागम;
	} अन्यथा
		यदि (openpty(&rc_logger_tty, &slave_tty, रिक्त, रिक्त, रिक्त))
			प्रत्यागम;

	यदि ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	यदि ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = fork();
	विकल्प (rc_logger_pid) {
	पक्ष -1:
		eerror("fork: %s", strerror(errno));
		भङ्ग;
	पक्ष 0:
		rc_अन्तर्_logger = true;
		close(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		यदि ((लघुगणकः = fopen(tmplog, "ae")))
			write_time(लघुगणकः, "started");
		अन्यथा {
			मुक्त(logbuf);
			logbuf_आकार = BUFSIZ * 10;
			logbuf = xmalloc(परिमाण (वर्ण) * logbuf_आकार);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		यदि (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		प्रति (;;) {
			यदि ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", strerror(errno));
				भङ्ग;
			} अन्यथा यदि (s == 0)
				अनुवर्त;

			यदि (fd[1].revents & (POLLIN | POLLHUP)) {
				स्मृतिं_पूरय(बफर, 0, BUFSIZ);
				अष्टकानि = पठ(rc_logger_tty, बफर, BUFSIZ);
				यदि (लिख(STDOUT_FILENO, बफर, अष्टकानि) == -1)
					eerror("write: %s", strerror(errno));

				यदि (लघुगणकः)
					write_log(fileno (लघुगणकः), बफर, अष्टकानि);
				अन्यथा {
					यदि (logbuf_आकार - logbuf_len < अष्टकानि) {
						logbuf_आकार += BUFSIZ * 10;
						logbuf = xrealloc(logbuf,
						    परिमाण(वर्ण ) *
						    logbuf_आकार);
					}

					स्मृतिं_प्रतिलिख(logbuf + logbuf_len,
					    बफर, अष्टकानि);
					logbuf_len += अष्टकानि;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			यदि (fd[0].revents & (POLLIN | POLLHUP))
				भङ्ग;
		}
		यदि (logbuf) {
			यदि ((लघुगणकः = fopen(tmplog, "ae"))) {
				write_time(लघुगणकः, "started");
				write_log(fileno(लघुगणकः), logbuf, logbuf_len);
			}
			मुक्त(logbuf);
		}
		यदि (लघुगणकः) {
			write_time(लघुगणकः, "stopped");
			fclose(लघुगणकः);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_मानम्("rc_log_path");
		यदि (logfile == रिक्त) {
			यदि (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} अन्यथा {
				ध्रुव वर्ण *state_home = getenv("XDG_STATE_HOME");
				यदि (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} अन्यथा यदि ((state_home = getenv("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		यदि (!वाक्यं_तुलय(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		यदि ((plog = fopen(logfile, "ae"))) {
			यदि ((लघुगणकः = fopen(tmplog, "re"))) {
				यावत् ((अष्टकानि = fread(बफर, परिमाण(*बफर), BUFSIZ, लघुगणकः)) > 0) {
					यदि (fwrite(बफर, परिमाण(*बफर), अष्टकानि, plog) < अष्टकानि) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, strerror(errno));
						भङ्ग;
					}
				}
				fclose(लघुगणकः);
			} अन्यथा {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, strerror(errno));
			}

			fclose(plog);
		} अन्यथा {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			यदि (errno != EROFS && ((वाक्यं_तुलय(level, RC_LEVEL_SHUTDOWN) != 0) && (वाक्यं_तुलय(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, strerror(errno));
			}
		}

		मुक्त(usrlog);

		/* Try to keep the temporary log in case of errors */
		यदि (!log_error) {
			यदि (errno != EROFS && ((वाक्यं_तुलय(level, RC_LEVEL_SHUTDOWN) != 0) && (वाक्यं_तुलय(level, RC_LEVEL_SYSINIT) != 0)))
				यदि (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, strerror(errno));
		} अन्यथा यदि (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		मुक्त(tmplog);

		निर्गच्छ(0);
		/* NOTREACHED */

	स्वभाव:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		यदि ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		यदि ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		यदि (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			close(slave_tty);
		close(signal_pipe[0]);
		signal_pipe[0] = -1;
		भङ्ग;
	}
}
