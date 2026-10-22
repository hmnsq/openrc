#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <syslog.h>

#অন্তর্ভাব <rc.h>

#অন্তর্ভাব "initreq.h"
#অন্তর্ভাব "helpers.h"

স্থির bool init_halt = false;

স্থির শূন্য sysvinit_runlevel(পূর্ণাঙ্ক runlevel)
{
	ধ্রুব বর্ণ *cmd;
	পূর্ণাঙ্ক fifo;

	বিকল্প (runlevel) {
	পক্ষ '0':
		cmd = init_halt ? "halt" : "poweroff";
		ভঙ্গ;
	পক্ষ '1':
	পক্ষ 'S':
	পক্ষ 's':
		cmd = "single";
		ভঙ্গ;
	পক্ষ '2':
	পক্ষ '3':
	পক্ষ '4':
	পক্ষ '5':
		/* TODO: switch to default runlevel? */
		প্রত্যাগম;
	পক্ষ '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		ভঙ্গ;
	পক্ষ 'Q':
	পক্ষ 'q':
		প্রত্যাগম;
	পক্ষ 'U':
	পক্ষ 'u':
		cmd = "reexec";
		ভঙ্গ;
	}

	যদি ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		প্রত্যাগম;
	}

	যদি (লিখ(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

স্থির শূন্য sysvinit_setenv(বর্ণ *data, size_t size)
{
	data[size - 1] = '\0';
	প্রতি (বর্ণ *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		যদি (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		অন্যথা যদি (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

পূর্ণাঙ্ক main(শূন্য) {
	পূর্ণাঙ্ক fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	যদি ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		প্রত্যাগম 1;
	}

	প্রতি (;;) {
		সংরচনা init_request req;
		ssize_t গুনতি = পঠ(fifo, &req, পরিমাণ(req));

		যদি (গুনতি == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			অনুবর্ত;
		}

		যদি (গুনতি != পরিমাণ(req)) {
			syslog(LOG_ERR, "read: short count");
			অনুবর্ত;
		}

		যদি (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			অনুবর্ত;
		}

		বিকল্প (req.cmd) {
		পক্ষ INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			ভঙ্গ;
		পক্ষ INIT_CMD_SETENV:
		পক্ষ INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, পরিমাণ(req.i.data));
			ভঙ্গ;
		}
	}
}
