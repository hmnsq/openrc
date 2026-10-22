#समावेश <sys/types.h>
#समावेश <sys/stat.h>

#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <unistd.h>
#समावेश <string.h>
#समावेश <stdbool.h>
#समावेश <syslog.h>

#समावेश <rc.h>

#समावेश "initreq.h"
#समावेश "helpers.h"

अटल bool init_halt = false;

अटल व्योम sysvinit_runlevel(पूर्णांक runlevel)
{
	स्थिर अक्षर *cmd;
	पूर्णांक fifo;

	चयन (runlevel) {
	हाल '0':
		cmd = init_halt ? "halt" : "poweroff";
		अवरोध;
	हाल '1':
	हाल 'S':
	हाल 's':
		cmd = "single";
		अवरोध;
	हाल '2':
	हाल '3':
	हाल '4':
	हाल '5':
		/* TODO: switch to default runlevel? */
		वापस;
	हाल '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		अवरोध;
	हाल 'Q':
	हाल 'q':
		वापस;
	हाल 'U':
	हाल 'u':
		cmd = "reexec";
		अवरोध;
	}

	अगर ((fifo = खोलो(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", म_त्रुटि(त्रुटि_सं));
		वापस;
	}

	अगर (ग_लिखो(fifo, cmd, म_माप(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", म_त्रुटि(त्रुटि_सं));
	बंद(fifo);
}

अटल व्योम sysvinit_setenv(अक्षर *तथ्य, माप_प्रकार आकार)
{
	तथ्य[आकार - 1] = '\0';
	क्रम (अक्षर *end = तथ्य + आकार; तथ्य < end && *तथ्य; तथ्य += म_माप(तथ्य) + 1) {
		/* We only care about the INIT_HALT variable */
		अगर (!म_भेद(तथ्य, "INIT_HALT=HALT"))
			init_halt = true;
		अन्यथा अगर (!म_भेद(तथ्य, "INIT_HALT") || !म_भेदन(तथ्य, "INIT_HALT=", 10))
			init_halt = false;
	}
}

पूर्णांक मुख्य(व्योम) {
	पूर्णांक fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	अगर ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", म_त्रुटि(त्रुटि_सं));
		वापस 1;
	}

	क्रम (;;) {
		काष्ठा init_request req;
		ssize_t गिनती = पढ़ो(fifo, &req, माप(req));

		अगर (गिनती == -1) {
			syslog(LOG_ERR, "read: %s", म_त्रुटि(त्रुटि_सं));
			जारी;
		}

		अगर (गिनती != माप(req)) {
			syslog(LOG_ERR, "read: short count");
			जारी;
		}

		अगर (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			जारी;
		}

		चयन (req.cmd) {
		हाल INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			अवरोध;
		हाल INIT_CMD_SETENV:
		हाल INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.तथ्य, माप(req.i.तथ्य));
			अवरोध;
		}
	}
}
