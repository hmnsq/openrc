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

#जर defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#किंवाजर defined(__NetBSD__) || defined(__OpenBSD__)
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

स्थिर पूर्णांक signal_pipe[2] = { -1, -1 };
स्थिर पूर्णांक fd_stdout = -1;
स्थिर पूर्णांक fd_stderr = -1;
स्थिर स्थिरांक अक्षर *runlevel = रिक्त;
स्थिर bool in_escape = false;
स्थिर bool in_term = false;

स्थिर अक्षर *logbuf = रिक्त;
स्थिर माप_प्रकार logbuf_size = 0;
स्थिर माप_प्रकार logbuf_len = 0;

pid_t rc_logger_pid = -1;
पूर्णांक rc_logger_tty = -1;
bool rc_in_logger = false;

स्थिर शून्य
write_log(पूर्णांक logfd, स्थिरांक अक्षर *buffer, माप_प्रकार bytes)
{
	स्थिरांक अक्षर *p = buffer;

	पर्यंत ((माप_प्रकार)(p - buffer) < bytes) {
		निवड (*p) {
		स्थिती '\r':
			जा cont;
		स्थिती '\033':
			in_escape = true;
			in_term = false;
			जा cont;
		स्थिती '\n':
			in_escape = in_term = false;
			थांबा;
		स्थिती '[':
			जर (in_escape)
				in_term = true;
			थांबा;
		}

		जर (!in_escape) {
			जर (!है_छाप((पूर्णांक) *p) && *p != '\n')
				जा cont;
			जर (लिख(logfd, p++, 1) == -1)
				eerror("write: %s", म_त्रुटि(त्रुटि_सं));
			सुरूठेवा;
		}

		जर (!in_term || है_अक्षर((अचिन्हित अक्षर)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

स्थिर शून्य
write_time(खाता *f, स्थिरांक अक्षर *s)
{
	समय_प्रकार now = समय(रिक्त);
	संरचना पंचांग *पंचांग = स_स्थानीय(&now);

	ख_लिखो(f, "\nrc %s logging %s at %s\n", runlevel, s, समय_ठीक(पंचांग));
	ख_साफ(f);
}

स्थिर शून्य
mkpath(स्थिरांक अक्षर *path)
{
	अक्षर *dir = xstrdup(path), *slash = dir;
	पर्यंत ((slash = म_अक्षर(slash + 1, '/'))) {
		*slash = '\0';
		जर (सूची_गढ़ो(path, 0755) == -1 && त्रुटि_सं != EEXIST)
			थांबा;
		*slash = '/';
	}
	सूची_गढ़ो(path, 0755);
	मुक्त(dir);
}

शून्य
rc_logger_close(शून्य)
{
	पूर्णांक sig = संक_इति;

	जर (signal_pipe[1] > -1) {
		जर (लिख(signal_pipe[1], &sig, आकार(sig)) == -1)
			eerror("write: %s", म_त्रुटि(त्रुटि_सं));
		बंद(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	जर (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	जर (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	जर (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

शून्य
rc_logger_open(स्थिरांक अक्षर *level)
{
	पूर्णांक slave_tty;
	संरचना termios tt;
	संरचना winsize ws;
	अक्षर buffer[बफ_मान];
	संरचना pollfd fd[2];
	पूर्णांक s = 0;
	माप_प्रकार bytes;
	खाता *log = रिक्त;
	खाता *plog = रिक्त;
	स्थिरांक अक्षर *logfile;
	अक्षर *tmplog, *usrlog = रिक्त;
	पूर्णांक log_error = 0;

	जर (!rc_conf_yesno("rc_logger"))
		परत;

	जर (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", म_त्रुटि(त्रुटि_सं));

	जर (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		जर (openpty(&rc_logger_tty, &slave_tty, रिक्त, &tt, &ws))
			परत;
	} अन्यथा
		जर (openpty(&rc_logger_tty, &slave_tty, रिक्त, रिक्त, रिक्त))
			परत;

	जर ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	जर ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = विभाजन();
	निवड (rc_logger_pid) {
	स्थिती -1:
		eerror("fork: %s", म_त्रुटि(त्रुटि_सं));
		थांबा;
	स्थिती 0:
		rc_in_logger = true;
		बंद(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		जर ((log = ख_खोलो(tmplog, "ae")))
			write_time(log, "started");
		अन्यथा {
			मुक्त(logbuf);
			logbuf_size = बफ_मान * 10;
			logbuf = xmalloc(आकार (अक्षर) * logbuf_size);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		जर (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		साठी (;;) {
			जर ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", म_त्रुटि(त्रुटि_सं));
				थांबा;
			} अन्यथा जर (s == 0)
				सुरूठेवा;

			जर (fd[1].revents & (POLLIN | POLLHUP)) {
				स_रखो(buffer, 0, बफ_मान);
				bytes = पठ(rc_logger_tty, buffer, बफ_मान);
				जर (लिख(STDOUT_FILENO, buffer, bytes) == -1)
					eerror("write: %s", म_त्रुटि(त्रुटि_सं));

				जर (log)
					write_log(fileno (log), buffer, bytes);
				अन्यथा {
					जर (logbuf_size - logbuf_len < bytes) {
						logbuf_size += बफ_मान * 10;
						logbuf = xrealloc(logbuf,
						    आकार(अक्षर ) *
						    logbuf_size);
					}

					स_नकल(logbuf + logbuf_len,
					    buffer, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			जर (fd[0].revents & (POLLIN | POLLHUP))
				थांबा;
		}
		जर (logbuf) {
			जर ((log = ख_खोलो(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			मुक्त(logbuf);
		}
		जर (log) {
			write_time(log, "stopped");
			ख_बंद(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_मूल्य("rc_log_path");
		जर (logfile == रिक्त) {
			जर (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} अन्यथा {
				स्थिरांक अक्षर *state_home = दो_पर्या("XDG_STATE_HOME");
				जर (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} अन्यथा जर ((state_home = दो_पर्या("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		जर (!म_भेद(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		जर ((plog = ख_खोलो(logfile, "ae"))) {
			जर ((log = ख_खोलो(tmplog, "re"))) {
				पर्यंत ((bytes = ख_पढ़ो(buffer, आकार(*buffer), बफ_मान, log)) > 0) {
					जर (ख_डालो(buffer, आकार(*buffer), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, म_त्रुटि(त्रुटि_सं));
						थांबा;
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
			जर (त्रुटि_सं != EROFS && ((म_भेद(level, RC_LEVEL_SHUTDOWN) != 0) && (म_भेद(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, म_त्रुटि(त्रुटि_सं));
			}
		}

		मुक्त(usrlog);

		/* Try to keep the temporary log in case of errors */
		जर (!log_error) {
			जर (त्रुटि_सं != EROFS && ((म_भेद(level, RC_LEVEL_SHUTDOWN) != 0) && (म_भेद(level, RC_LEVEL_SYSINIT) != 0)))
				जर (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, म_त्रुटि(त्रुटि_सं));
		} अन्यथा जर (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		मुक्त(tmplog);

		निकास(0);
		/* NOTREACHED */

	पूर्वनिर्धारित:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		जर ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		जर ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		जर (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			बंद(slave_tty);
		बंद(signal_pipe[0]);
		signal_pipe[0] = -1;
		थांबा;
	}
}
