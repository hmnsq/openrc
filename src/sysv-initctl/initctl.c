#شامل <sys/types.h>
#شامل <sys/stat.h>

#شامل <errno.h>
#شامل <fcntl.h>
#شامل <unistd.h>
#شامل <string.h>
#شامل <stdbool.h>
#شامل <syslog.h>

#شامل <rc.h>

#شامل "initreq.h"
#شامل "helpers.h"

ایستا bool init_halt = false;

ایستا تهی sysvinit_runlevel(صحیح runlevel)
{
	ثابت نویسه *cmd;
	صحیح fifo;

	انتخاب (runlevel) {
	حالت '0':
		cmd = init_halt ? "halt" : "poweroff";
		بشکن;
	حالت '1':
	حالت 'S':
	حالت 's':
		cmd = "single";
		بشکن;
	حالت '2':
	حالت '3':
	حالت '4':
	حالت '5':
		/* TODO: switch to default runlevel? */
		بازگشت;
	حالت '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		بشکن;
	حالت 'Q':
	حالت 'q':
		بازگشت;
	حالت 'U':
	حالت 'u':
		cmd = "reexec";
		بشکن;
	}

	اگر ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		بازگشت;
	}

	اگر (اكتب(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ایستا تهی sysvinit_setenv(نویسه *data, size_t size)
{
	data[size - 1] = '\0';
	برای (نویسه *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		اگر (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		وگرنه اگر (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

صحیح اصلی(تهی) {
	صحیح fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	اگر ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		بازگشت 1;
	}

	برای (;;) {
		ساختار init_request req;
		ssize_t شمار = اقرأ(fifo, &req, اندازه(req));

		اگر (شمار == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			ادامه;
		}

		اگر (شمار != اندازه(req)) {
			syslog(LOG_ERR, "read: short count");
			ادامه;
		}

		اگر (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			ادامه;
		}

		انتخاب (req.cmd) {
		حالت INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			بشکن;
		حالت INIT_CMD_SETENV:
		حالت INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, اندازه(req.i.data));
			بشکن;
		}
	}
}
