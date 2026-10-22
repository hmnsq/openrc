#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>

#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <syslog.h>

#अन्तर्भाव <rc.h>

#अन्तर्भाव "initreq.h"
#अन्तर्भाव "helpers.h"

स्थिर bool init_halt = false;

स्थिर शून्य sysvinit_runlevel(पूर्णांक runlevel)
{
	स्थिरांक अक्षर *cmd;
	पूर्णांक fifo;

	निवड (runlevel) {
	स्थिती '0':
		cmd = init_halt ? "halt" : "poweroff";
		थांबा;
	स्थिती '1':
	स्थिती 'S':
	स्थिती 's':
		cmd = "single";
		थांबा;
	स्थिती '2':
	स्थिती '3':
	स्थिती '4':
	स्थिती '5':
		/* TODO: switch to default runlevel? */
		परत;
	स्थिती '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		थांबा;
	स्थिती 'Q':
	स्थिती 'q':
		परत;
	स्थिती 'U':
	स्थिती 'u':
		cmd = "reexec";
		थांबा;
	}

	जर ((fifo = खोलो(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", म_त्रुटि(त्रुटि_सं));
		परत;
	}

	जर (लिख(fifo, cmd, म_माप(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", म_त्रुटि(त्रुटि_सं));
	बंद(fifo);
}

स्थिर शून्य sysvinit_setenv(अक्षर *data, माप_प्रकार size)
{
	data[size - 1] = '\0';
	साठी (अक्षर *end = data + size; data < end && *data; data += म_माप(data) + 1) {
		/* We only care about the INIT_HALT variable */
		जर (!म_भेद(data, "INIT_HALT=HALT"))
			init_halt = true;
		अन्यथा जर (!म_भेद(data, "INIT_HALT") || !म_भेदन(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

पूर्णांक main(शून्य) {
	पूर्णांक fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	जर ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", म_त्रुटि(त्रुटि_सं));
		परत 1;
	}

	साठी (;;) {
		संरचना init_request req;
		ssize_t मोजणी = पठ(fifo, &req, आकार(req));

		जर (मोजणी == -1) {
			syslog(LOG_ERR, "read: %s", म_त्रुटि(त्रुटि_सं));
			सुरूठेवा;
		}

		जर (मोजणी != आकार(req)) {
			syslog(LOG_ERR, "read: short count");
			सुरूठेवा;
		}

		जर (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			सुरूठेवा;
		}

		निवड (req.cmd) {
		स्थिती INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			थांबा;
		स्थिती INIT_CMD_SETENV:
		स्थिती INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, आकार(req.i.data));
			थांबा;
		}
	}
}
