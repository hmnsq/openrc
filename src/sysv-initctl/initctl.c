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

স্থির শূন্য sysvinit_runlevel(পূর্ণসংখ্যা runlevel)
{
	ধ্রুবক অক্ষর *cmd;
	পূর্ণসংখ্যা fifo;

	নির্বাচন (runlevel) {
	ক্ষেত্র '0':
		cmd = init_halt ? "halt" : "poweroff";
		বিরতি;
	ক্ষেত্র '1':
	ক্ষেত্র 'S':
	ক্ষেত্র 's':
		cmd = "single";
		বিরতি;
	ক্ষেত্র '2':
	ক্ষেত্র '3':
	ক্ষেত্র '4':
	ক্ষেত্র '5':
		/* TODO: switch to default runlevel? */
		ফেরত;
	ক্ষেত্র '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		বিরতি;
	ক্ষেত্র 'Q':
	ক্ষেত্র 'q':
		ফেরত;
	ক্ষেত্র 'U':
	ক্ষেত্র 'u':
		cmd = "reexec";
		বিরতি;
	}

	যদি ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		ফেরত;
	}

	যদি (লিখ(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

স্থির শূন্য sysvinit_setenv(অক্ষর *তথ্য, size_t আকার_2)
{
	তথ্য[আকার_2 - 1] = '\0';
	জন্য (অক্ষর *end = তথ্য + আকার_2; তথ্য < end && *তথ্য; তথ্য += strlen(তথ্য) + 1) {
		/* We only care about the INIT_HALT variable */
		যদি (!strcmp(তথ্য, "INIT_HALT=HALT"))
			init_halt = true;
		অন্যথা যদি (!strcmp(তথ্য, "INIT_HALT") || !strncmp(তথ্য, "INIT_HALT=", 10))
			init_halt = false;
	}
}

পূর্ণসংখ্যা প্রধান(শূন্য) {
	পূর্ণসংখ্যা fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	যদি ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		ফেরত 1;
	}

	জন্য (;;) {
		কাঠামো init_request req;
		ssize_t গুনতি = পঠ(fifo, &req, আকার(req));

		যদি (গুনতি == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			চালু;
		}

		যদি (গুনতি != আকার(req)) {
			syslog(LOG_ERR, "read: short count");
			চালু;
		}

		যদি (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			চালু;
		}

		নির্বাচন (req.cmd) {
		ক্ষেত্র INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			বিরতি;
		ক্ষেত্র INIT_CMD_SETENV:
		ক্ষেত্র INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.তথ্য, আকার(req.i.তথ্য));
			বিরতি;
		}
	}
}
