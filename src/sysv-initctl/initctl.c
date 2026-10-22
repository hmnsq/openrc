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

ساکن bool init_halt = false;

ساکن خالی sysvinit_runlevel(صحیح runlevel)
{
	ثابت حرف *cmd;
	صحیح fifo;

	انتخاب (runlevel) {
	حالت '0':
		cmd = init_halt ? "halt" : "poweroff";
		توڑو;
	حالت '1':
	حالت 'S':
	حالت 's':
		cmd = "single";
		توڑو;
	حالت '2':
	حالت '3':
	حالت '4':
	حالت '5':
		/* TODO: switch to default runlevel? */
		واپس;
	حالت '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		توڑو;
	حالت 'Q':
	حالت 'q':
		واپس;
	حالت 'U':
	حالت 'u':
		cmd = "reexec";
		توڑو;
	}

	اگر ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		واپس;
	}

	اگر (اكتب(fifo, cmd, لڑی_لمبائی(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ساکن خالی sysvinit_setenv(حرف *ڈیٹا, size_t سائز)
{
	ڈیٹا[سائز - 1] = '\0';
	برائے (حرف *انتہا = ڈیٹا + سائز; ڈیٹا < انتہا && *ڈیٹا; ڈیٹا += لڑی_لمبائی(ڈیٹا) + 1) {
		/* We only care about the INIT_HALT variable */
		اگر (!strcmp(ڈیٹا, "INIT_HALT=HALT"))
			init_halt = true;
		ورنہ اگر (!strcmp(ڈیٹا, "INIT_HALT") || !لڑی_موازنہ(ڈیٹا, "INIT_HALT=", 10))
			init_halt = false;
	}
}

صحیح مین(خالی) {
	صحیح fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	اگر ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		واپس 1;
	}

	برائے (;;) {
		ساخت init_request req;
		ssize_t گنتی = اقرأ(fifo, &req, حجم(req));

		اگر (گنتی == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			جاری;
		}

		اگر (گنتی != حجم(req)) {
			syslog(LOG_ERR, "read: short count");
			جاری;
		}

		اگر (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			جاری;
		}

		انتخاب (req.cmd) {
		حالت INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			توڑو;
		حالت INIT_CMD_SETENV:
		حالت INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.ڈیٹا, حجم(req.i.ڈیٹا));
			توڑو;
		}
	}
}
