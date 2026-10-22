#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <syslog.h>

#ଅନ୍ତର୍ଭାଵ <rc.h>

#ଅନ୍ତର୍ଭାଵ "initreq.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"

ସ୍ଥିର bool init_halt = false;

ସ୍ଥିର ଶୂନ୍ୟ sysvinit_runlevel(ପୂର୍ଣ୍ଣସଂଖ୍ୟା runlevel)
{
	ଅଚଳ ଅକ୍ଷର *cmd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fifo;

	ବାଛ (runlevel) {
	ସ୍ଥିତି '0':
		cmd = init_halt ? "halt" : "poweroff";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '1':
	ସ୍ଥିତି 'S':
	ସ୍ଥିତି 's':
		cmd = "single";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '2':
	ସ୍ଥିତି '3':
	ସ୍ଥିତି '4':
	ସ୍ଥିତି '5':
		/* TODO: switch to default runlevel? */
		ଫେରନ୍ତୁ;
	ସ୍ଥିତି '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 'Q':
	ସ୍ଥିତି 'q':
		ଫେରନ୍ତୁ;
	ସ୍ଥିତି 'U':
	ସ୍ଥିତି 'u':
		cmd = "reexec";
		ଭାଙ୍ଗ;
	}

	ଯଦି ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		ଫେରନ୍ତୁ;
	}

	ଯଦି (ଲିଖ(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ସ୍ଥିର ଶୂନ୍ୟ sysvinit_setenv(ଅକ୍ଷର *data, size_t size)
{
	data[size - 1] = '\0';
	ପାଇଁ (ଅକ୍ଷର *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		ଯଦି (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		ଅନ୍ୟଥା ଯଦି (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ଶୂନ୍ୟ) {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	ଯଦି ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		ଫେରନ୍ତୁ 1;
	}

	ପାଇଁ (;;) {
		ସଂରଚନା init_request req;
		ssize_t ଗଣନା_2 = ପଠ(fifo, &req, ଆକାର(req));

		ଯଦି (ଗଣନା_2 == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			ଜାରି;
		}

		ଯଦି (ଗଣନା_2 != ଆକାର(req)) {
			syslog(LOG_ERR, "read: short count");
			ଜାରି;
		}

		ଯଦି (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			ଜାରି;
		}

		ବାଛ (req.cmd) {
		ସ୍ଥିତି INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି INIT_CMD_SETENV:
		ସ୍ଥିତି INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, ଆକାର(req.i.data));
			ଭାଙ୍ଗ;
		}
	}
}
