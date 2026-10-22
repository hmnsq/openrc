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
#अथवायदि defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#अन्यथा
#  include <libutil.h>
#endif

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc-logger.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "helpers.h"

#define DEFAULTLOG "/var/log/rc.log"

स्थिर पूर्णाङ्क signal_pipe[2] = { -1, -1 };
स्थिर पूर्णाङ्क fd_stdout = -1;
स्थिर पूर्णाङ्क fd_stderr = -1;
स्थिर अचर वर्ण *runlevel = रिक्त;
स्थिर bool in_escape = false;
स्थिर bool in_term = false;

स्थिर वर्ण *logbuf = रिक्त;
स्थिर माप_प्रकार logbuf_size = 0;
स्थिर माप_प्रकार logbuf_len = 0;

pid_t rc_logger_pid = -1;
पूर्णाङ्क rc_logger_tty = -1;
bool rc_in_logger = false;

स्थिर शून्य
write_log(पूर्णाङ्क logfd, अचर वर्ण *buffer, माप_प्रकार bytes)
{
	अचर वर्ण *p = buffer;

	जबसम्म ((माप_प्रकार)(p - buffer) < bytes) {
		विकल्प (*p) {
		पक्ष '\r':
			गच्छ cont;
		पक्ष '\033':
			in_escape = true;
			in_term = false;
			गच्छ cont;
		पक्ष '\n':
			in_escape = in_term = false;
			रोक;
		पक्ष '[':
			यदि (in_escape)
				in_term = true;
			रोक;
		}

		यदि (!in_escape) {
			यदि (!है_छाप((पूर्णाङ्क) *p) && *p != '\n')
				गच्छ cont;
			यदि (लिख(logfd, p++, 1) == -1)
				eerror("write: %s", म_त्रुटि(त्रुटि_सं));
			जारी;
		}

		यदि (!in_term || है_अक्षर((अचिन्हित वर्ण)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

स्थिर शून्य
write_time(खाता *f, अचर वर्ण *s)
{
	समय_प्रकार now = समय(रिक्त);
	संरचना पंचांग *पंचांग = स_स्थानीय(&now);

	ख_लिखो(f, "\nrc %s logging %s at %s\n", runlevel, s, समय_ठीक(पंचांग));
	ख_साफ(f);
}

स्थिर शून्य
mkpath(अचर वर्ण *path)
{
	वर्ण *dir = xstrdup(path), *slash = dir;
	जबसम्म ((slash = म_अक्षर(slash + 1, '/'))) {
		*slash = '\0';
		यदि (सूची_गढ़ो(path, 0755) == -1 && त्रुटि_सं != EEXIST)
			रोक;
		*slash = '/';
	}
	सूची_गढ़ो(path, 0755);
	मुक्त(dir);
}

शून्य
rc_logger_close(शून्य)
{
	पूर्णाङ्क sig = संक_इति;

	यदि (signal_pipe[1] > -1) {
		यदि (लिख(signal_pipe[1], &sig, परिमाण(sig)) == -1)
			eerror("write: %s", म_त्रुटि(त्रुटि_सं));
		बंद(signal_pipe[1]);
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
rc_logger_open(अचर वर्ण *level)
{
	पूर्णाङ्क slave_tty;
	संरचना termios tt;
	संरचना winsize ws;
	वर्ण buffer[बफ_मान];
	संरचना pollfd fd[2];
	पूर्णाङ्क s = 0;
	माप_प्रकार bytes;
	खाता *log = रिक्त;
	खाता *plog = रिक्त;
	अचर वर्ण *logfile;
	वर्ण *tmplog, *usrlog = रिक्त;
	पूर्णाङ्क log_error = 0;

	यदि (!rc_conf_yesno("rc_logger"))
		फर्काऊ;

	यदि (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", म_त्रुटि(त्रुटि_सं));

	यदि (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		यदि (openpty(&rc_logger_tty, &slave_tty, रिक्त, &tt, &ws))
			फर्काऊ;
	} अन्यथा
		यदि (openpty(&rc_logger_tty, &slave_tty, रिक्त, रिक्त, रिक्त))
			फर्काऊ;

	यदि ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	यदि ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = विभाजन();
	विकल्प (rc_logger_pid) {
	पक्ष -1:
		eerror("fork: %s", म_त्रुटि(त्रुटि_सं));
		रोक;
	पक्ष 0:
		rc_in_logger = true;
		बंद(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		यदि ((log = ख_खोलो(tmplog, "ae")))
			write_time(log, "started");
		अन्यथा {
			मुक्त(logbuf);
			logbuf_size = बफ_मान * 10;
			logbuf = xmalloc(परिमाण (वर्ण) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		यदि (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		लागि (;;) {
			यदि ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", म_त्रुटि(त्रुटि_सं));
				रोक;
			} अन्यथा यदि (s == 0)
				जारी;

			यदि (fd[1].revents & (POLLIN | POLLHUP)) {
				स_रखो(buffer, 0, बफ_मान);
				bytes = पठ(rc_logger_tty, buffer, बफ_मान);
				यदि (लिख(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", म_त्रुटि(त्रुटि_सं));

				यदि (log)
					write_log(fileno (log), buffer, bytes);
				अन्यथा {
					यदि (logbuf_size - logbuf_len < bytes) {
						logbuf_size += बफ_मान * 10;
						logbuf = xrealloc(logbuf,
						    परिमाण(वर्ण ) *
						    logbuf_size);
					}

					स_नकल(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			यदि (fd[0].revents & (POLLIN | POLLHUP))
				रोक;
		}
		यदि (logbuf) {
			यदि ((log = ख_खोलो(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			मुक्त(logbuf);
		}
		यदि (log) {
			write_time(log, "stopped");
			ख_बंद(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_मान("rc_log_path");
		यदि (logfile == रिक्त) {
			यदि (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} अन्यथा {
				अचर वर्ण *state_home = दो_पर्या("XDG_STATE_HOME");
				यदि (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} अन्यथा यदि ((state_home = दो_पर्या("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		यदि (!म_भेद(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		यदि ((plog = ख_खोलो(logfile, "ae"))) {
			यदि ((log = ख_खोलो(tmplog, "re"))) {
				जबसम्म ((bytes = ख_पढ़ो(buffer, परिमाण(*buffer), बफ_मान, log)) > 0) {
					यदि (ख_डालो(buffer, परिमाण(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, म_त्रुटि(त्रुटि_सं));
						रोक;
					}
				}
				ख_बंद(log);
			} अन्यथा {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", tmplog, म_त्रुटि(त्रुटि_सं));
			}

			ख_बंद(plog);
		} अन्यथा {
			/*
			 * logfile or its basedir may be read-only during sysinit and
			 * shutdown so skip the error in this case
			 */
			यदि (त्रुटि_सं != EROFS && ((म_भेद(level, RC_LEVEL_SHUTDOWN) != 0) && (म_भेद(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, म_त्रुटि(त्रुटि_सं));
			}
		}

		मुक्त(usrlog);

		/* Try to keep the temporary log in case of errors */
		यदि (!log_error) {
			यदि (त्रुटि_सं != EROFS && ((म_भेद(level, RC_LEVEL_SHUTDOWN) != 0) && (म_भेद(level, RC_LEVEL_SYSINIT) != 0)))
				यदि (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, म_त्रुटि(त्रुटि_सं));
		} अन्यथा यदि (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		मुक्त(tmplog);

		निकास(0);
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
			बंद(slave_tty);
		बंद(signal_pipe[0]);
		signal_pipe[0] = -1;
		रोक;
	}
}
