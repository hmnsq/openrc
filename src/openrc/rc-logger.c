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

#समावेश <errno.h>
#समावेश <ctype.h>
#समावेश <fcntl.h>
#समावेश <poll.h>
#समावेश <signal.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/types.h>
#समावेश <sys/ioctl.h>
#समावेश <sys/wait.h>
#समावेश <termios.h>
#समावेश <time.h>
#समावेश <unistd.h>

#अगर defined(__linux__) || (defined(__FreeBSD_kernel__) && defined(__GLIBC__)) \
	|| defined(__GNU__)
#  include <pty.h>
#या_अगर defined(__NetBSD__) || defined(__OpenBSD__)
#  include <util.h>
#अन्यथा
#  include <libutil.h>
#पूर्ण_अगर

#समावेश "einfo.h"
#समावेश "rc-logger.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "helpers.h"

#घोषणा DEFAULTLOG "/var/log/rc.log"

अटल पूर्णांक signal_pipe[2] = { -1, -1 };
अटल पूर्णांक fd_stdout = -1;
अटल पूर्णांक fd_stderr = -1;
अटल स्थिर अक्षर *runlevel = शून्य;
अटल bool in_escape = false;
अटल bool in_term = false;

अटल अक्षर *logbuf = शून्य;
अटल माप_प्रकार logbuf_आकार = 0;
अटल माप_प्रकार logbuf_len = 0;

pid_t rc_logger_pid = -1;
पूर्णांक rc_logger_tty = -1;
bool rc_in_logger = false;

अटल व्योम
write_log(पूर्णांक logfd, स्थिर अक्षर *बफर, माप_प्रकार bytes)
{
	स्थिर अक्षर *p = बफर;

	जबतक ((माप_प्रकार)(p - बफर) < bytes) {
		चयन (*p) {
		हाल '\r':
			जाओ cont;
		हाल '\033':
			in_escape = true;
			in_term = false;
			जाओ cont;
		हाल '\n':
			in_escape = in_term = false;
			अवरोध;
		हाल '[':
			अगर (in_escape)
				in_term = true;
			अवरोध;
		}

		अगर (!in_escape) {
			अगर (!है_छाप((पूर्णांक) *p) && *p != '\n')
				जाओ cont;
			अगर (ग_लिखो(logfd, p++, 1) == -1)
				eerror("write: %s", म_त्रुटि(त्रुटि_सं));
			जारी;
		}

		अगर (!in_term || है_अक्षर((अचिन्हित अक्षर)*p))
			in_escape = in_term = false;
cont:
		p++;
	}
}

अटल व्योम
write_time(खाता *f, स्थिर अक्षर *s)
{
	समय_प्रकार now = समय(शून्य);
	काष्ठा पंचांग *पंचांग = स_स्थानीय(&now);

	ख_लिखो(f, "\nrc %s logging %s at %s\n", runlevel, s, समय_ठीक(पंचांग));
	ख_साफ(f);
}

अटल व्योम
mkpath(स्थिर अक्षर *path)
{
	अक्षर *dir = xstrdup(path), *slash = dir;
	जबतक ((slash = म_अक्षर(slash + 1, '/'))) {
		*slash = '\0';
		अगर (सूची_गढ़ो(path, 0755) == -1 && त्रुटि_सं != EEXIST)
			अवरोध;
		*slash = '/';
	}
	सूची_गढ़ो(path, 0755);
	मुक्त(dir);
}

व्योम
rc_logger_close(व्योम)
{
	पूर्णांक sig = संक_इति;

	अगर (signal_pipe[1] > -1) {
		अगर (ग_लिखो(signal_pipe[1], &sig, माप(sig)) == -1)
			eerror("write: %s", म_त्रुटि(त्रुटि_सं));
		बंद(signal_pipe[1]);
		signal_pipe[1] = -1;
	}

	अगर (rc_logger_pid > 0)
		waitpid(rc_logger_pid, 0, 0);

	अगर (fd_stdout > -1)
		dup2(fd_stdout, STDOUT_FILENO);
	अगर (fd_stderr > -1)
		dup2(fd_stderr, STDERR_FILENO);
}

व्योम
rc_logger_open(स्थिर अक्षर *level)
{
	पूर्णांक slave_tty;
	काष्ठा termios tt;
	काष्ठा winsize ws;
	अक्षर बफर[बफ_मान];
	काष्ठा pollfd fd[2];
	पूर्णांक s = 0;
	माप_प्रकार bytes;
	खाता *log = शून्य;
	खाता *plog = शून्य;
	स्थिर अक्षर *logfile;
	अक्षर *tmplog, *usrlog = शून्य;
	पूर्णांक log_error = 0;

	अगर (!rc_conf_yesno("rc_logger"))
		वापस;

	अगर (pipe2(signal_pipe, O_CLOEXEC) == -1)
		eerrorx("pipe2: %s", म_त्रुटि(त्रुटि_सं));

	अगर (isatty(STDOUT_FILENO)) {
		tcgetattr(STDOUT_FILENO, &tt);
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
		अगर (openpty(&rc_logger_tty, &slave_tty, शून्य, &tt, &ws))
			वापस;
	} अन्यथा
		अगर (openpty(&rc_logger_tty, &slave_tty, शून्य, शून्य, शून्य))
			वापस;

	अगर ((s = fcntl(rc_logger_tty, F_GETFD, 0)) == 0)
		fcntl(rc_logger_tty, F_SETFD, s | FD_CLOEXEC);

	अगर ((s = fcntl(slave_tty, F_GETFD, 0)) == 0)
		fcntl(slave_tty, F_SETFD, s | FD_CLOEXEC);

	rc_logger_pid = विभाजन();
	चयन (rc_logger_pid) {
	हाल -1:
		eerror("fork: %s", म_त्रुटि(त्रुटि_सं));
		अवरोध;
	हाल 0:
		rc_in_logger = true;
		बंद(signal_pipe[1]);
		signal_pipe[1] = -1;

		runlevel = level;
		xasprintf(&tmplog, "%s/rc.log", rc_svcdir());
		अगर ((log = ख_खोलो(tmplog, "ae")))
			write_time(log, "started");
		अन्यथा {
			मुक्त(logbuf);
			logbuf_आकार = बफ_मान * 10;
			logbuf = xmalloc(माप (अक्षर) * logbuf_आकार);
			logbuf_len = 0;
		}

		fd[0].fd = signal_pipe[0];
		fd[0].events = fd[1].events = POLLIN;
		fd[0].revents = fd[1].revents = 0;
		अगर (rc_logger_tty >= 0)
			fd[1].fd = rc_logger_tty;
		क्रम (;;) {
			अगर ((s = poll(fd,
				    rc_logger_tty >= 0 ? 2 : 1, -1)) == -1)
			{
				eerror("poll: %s", म_त्रुटि(त्रुटि_सं));
				अवरोध;
			} अन्यथा अगर (s == 0)
				जारी;

			अगर (fd[1].revents & (POLLIN | POLLHUP)) {
				स_रखो(बफर, 0, बफ_मान);
				bytes = पढ़ो(rc_logger_tty, बफर, बफ_मान);
				अगर (ग_लिखो(STDOUT_FILENO, बफर, bytes) == -1)
					eerror("write: %s", म_त्रुटि(त्रुटि_सं));

				अगर (log)
					write_log(fileno (log), बफर, bytes);
				अन्यथा {
					अगर (logbuf_आकार - logbuf_len < bytes) {
						logbuf_आकार += बफ_मान * 10;
						logbuf = xrealloc(logbuf,
						    माप(अक्षर ) *
						    logbuf_आकार);
					}

					स_नकल(logbuf + logbuf_len,
					    बफर, bytes);
					logbuf_len += bytes;
				}
			}

			/* Only SIGTERMS signals come down this pipe */
			अगर (fd[0].revents & (POLLIN | POLLHUP))
				अवरोध;
		}
		अगर (logbuf) {
			अगर ((log = ख_खोलो(tmplog, "ae"))) {
				write_time(log, "started");
				write_log(fileno(log), logbuf, logbuf_len);
			}
			मुक्त(logbuf);
		}
		अगर (log) {
			write_time(log, "stopped");
			ख_बंद(log);
		}

		/* Append the temporary log to the real log */
		logfile = rc_conf_मूल्य("rc_log_path");
		अगर (logfile == शून्य) {
			अगर (!rc_is_user()) {
				logfile = DEFAULTLOG;
			} अन्यथा {
				स्थिर अक्षर *state_home = दो_पर्या("XDG_STATE_HOME");
				अगर (state_home) {
					xasprintf(&usrlog, "%s/rc.log", state_home);
				} अन्यथा अगर ((state_home = दो_पर्या("HOME"))) {
					xasprintf(&usrlog, "%s/.local/state/rc.log", state_home);
					state_home = ".local/state";
				}
				mkpath(state_home);
				logfile = usrlog;
			}
		}
		अगर (!म_भेद(logfile, tmplog)) {
			eerror("Cowardly refusing to concatenate a logfile into itself.");
			eerrorx("Please change rc_log_path to something other than %s to get rid of this message", tmplog);
		}

		अगर ((plog = ख_खोलो(logfile, "ae"))) {
			अगर ((log = ख_खोलो(tmplog, "re"))) {
				जबतक ((bytes = ख_पढ़ो(बफर, माप(*बफर), बफ_मान, log)) > 0) {
					अगर (ख_डालो(बफर, माप(*बफर), bytes, plog) < bytes) {
						log_error = 1;
						eerror("Error: write(%s) failed: %s", logfile, म_त्रुटि(त्रुटि_सं));
						अवरोध;
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
			अगर (त्रुटि_सं != EROFS && ((म_भेद(level, RC_LEVEL_SHUTDOWN) != 0) && (म_भेद(level, RC_LEVEL_SYSINIT) != 0))) {
				log_error = 1;
				eerror("Error: fopen(%s) failed: %s", logfile, म_त्रुटि(त्रुटि_सं));
			}
		}

		मुक्त(usrlog);

		/* Try to keep the temporary log in case of errors */
		अगर (!log_error) {
			अगर (त्रुटि_सं != EROFS && ((म_भेद(level, RC_LEVEL_SHUTDOWN) != 0) && (म_भेद(level, RC_LEVEL_SYSINIT) != 0)))
				अगर (unlink(tmplog) == -1)
					eerror("Error: unlink(%s) failed: %s", tmplog, म_त्रुटि(त्रुटि_सं));
		} अन्यथा अगर (access(tmplog, F_OK) == 0) {
			eerrorx("Warning: temporary logfile left behind: %s", tmplog);
		}

		मुक्त(tmplog);

		निकास(0);
		/* NOTREACHED */

	शेष:
		setpgid(rc_logger_pid, 0);
		fd_stdout = dup(STDOUT_FILENO);
		fd_stderr = dup(STDERR_FILENO);
		अगर ((s = fcntl(fd_stdout, F_GETFD, 0)) == 0)
			fcntl(fd_stdout, F_SETFD, s | FD_CLOEXEC);

		अगर ((s = fcntl(fd_stderr, F_GETFD, 0)) == 0)
			fcntl(fd_stderr, F_SETFD, s | FD_CLOEXEC);
		dup2(slave_tty, STDOUT_FILENO);
		dup2(slave_tty, STDERR_FILENO);
		अगर (slave_tty != STDIN_FILENO &&
		    slave_tty != STDOUT_FILENO &&
		    slave_tty != STDERR_FILENO)
			बंद(slave_tty);
		बंद(signal_pipe[0]);
		signal_pipe[0] = -1;
		अवरोध;
	}
}
