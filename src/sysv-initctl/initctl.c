#ضمن <sys/types.h>
#ضمن <sys/stat.h>

#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <unistd.h>
#ضمن <string.h>
#ضمن <stdbool.h>
#ضمن <syslog.h>

#ضمن <rc.h>

#ضمن "initreq.h"
#ضمن "helpers.h"

ساكن bool init_halt = false;

ساكن فراغ sysvinit_runlevel(صحيح runlevel)
{
	ثابت حرف *cmd;
	صحيح fifo;

	بدل (runlevel) {
	حالة '0':
		cmd = init_halt ? "halt" : "poweroff";
		اكسر;
	حالة '1':
	حالة 'S':
	حالة 's':
		cmd = "single";
		اكسر;
	حالة '2':
	حالة '3':
	حالة '4':
	حالة '5':
		/* TODO: switch to default runlevel? */
		أرجع;
	حالة '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		اكسر;
	حالة 'Q':
	حالة 'q':
		أرجع;
	حالة 'U':
	حالة 'u':
		cmd = "reexec";
		اكسر;
	}

	إذا ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		أرجع;
	}

	إذا (اكتب(fifo, cmd, لڙي_ڊيگهه(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ساكن فراغ sysvinit_setenv(حرف *معلومات, size_t سائيز)
{
	معلومات[سائيز - 1] = '\0';
	لكل (حرف *پڇاڙي = معلومات + سائيز; معلومات < پڇاڙي && *معلومات; معلومات += لڙي_ڊيگهه(معلومات) + 1) {
		/* We only care about the INIT_HALT variable */
		إذا (!strcmp(معلومات, "INIT_HALT=HALT"))
			init_halt = true;
		وإلا إذا (!strcmp(معلومات, "INIT_HALT") || !لڙي_ڀيٽ(معلومات, "INIT_HALT=", 10))
			init_halt = false;
	}
}

صحيح مکيه(فراغ) {
	صحيح fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	إذا ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		أرجع 1;
	}

	لكل (;;) {
		بنية init_request req;
		ssize_t ڳڻپ = اقرأ(fifo, &req, حجم(req));

		إذا (ڳڻپ == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			استمر;
		}

		إذا (ڳڻپ != حجم(req)) {
			syslog(LOG_ERR, "read: short count");
			استمر;
		}

		إذا (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			استمر;
		}

		بدل (req.cmd) {
		حالة INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			اكسر;
		حالة INIT_CMD_SETENV:
		حالة INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.معلومات, حجم(req.i.معلومات));
			اكسر;
		}
	}
}
