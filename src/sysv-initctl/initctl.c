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

स्थिर शून्य sysvinit_runlevel(पूर्णाङ्क runlevel)
{
	ध्रुव वर्ण *cmd;
	पूर्णाङ्क fifo;

	विकल्प (runlevel) {
	पक्ष '0':
		cmd = init_halt ? "halt" : "poweroff";
		भङ्ग;
	पक्ष '1':
	पक्ष 'S':
	पक्ष 's':
		cmd = "single";
		भङ्ग;
	पक्ष '2':
	पक्ष '3':
	पक्ष '4':
	पक्ष '5':
		/* TODO: switch to default runlevel? */
		प्रत्यागम;
	पक्ष '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		भङ्ग;
	पक्ष 'Q':
	पक्ष 'q':
		प्रत्यागम;
	पक्ष 'U':
	पक्ष 'u':
		cmd = "reexec";
		भङ्ग;
	}

	यदि ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		प्रत्यागम;
	}

	यदि (लिख(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

स्थिर शून्य sysvinit_setenv(वर्ण *data, size_t size)
{
	data[size - 1] = '\0';
	प्रति (वर्ण *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		यदि (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		अन्यथा यदि (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

पूर्णाङ्क main(शून्य) {
	पूर्णाङ्क fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	यदि ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		प्रत्यागम 1;
	}

	प्रति (;;) {
		संरचना init_request req;
		ssize_t मोजणी = पठ(fifo, &req, परिमाण(req));

		यदि (मोजणी == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			अनुवर्त;
		}

		यदि (मोजणी != परिमाण(req)) {
			syslog(LOG_ERR, "read: short count");
			अनुवर्त;
		}

		यदि (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			अनुवर्त;
		}

		विकल्प (req.cmd) {
		पक्ष INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			भङ्ग;
		पक्ष INIT_CMD_SETENV:
		पक्ष INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, परिमाण(req.i.data));
			भङ्ग;
		}
	}
}
