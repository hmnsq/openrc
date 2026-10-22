#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <syslog.h>

#અન્તર્ભાવ <rc.h>

#અન્તર્ભાવ "initreq.h"
#અન્તર્ભાવ "helpers.h"

સ્થિર bool init_halt = false;

સ્થિર શૂન્ય sysvinit_runlevel(પૂર્ણાંક runlevel)
{
	અચલ અક્ષર *cmd;
	પૂર્ણાંક fifo;

	પસંદગી (runlevel) {
	સ્થિતિ '0':
		cmd = init_halt ? "halt" : "poweroff";
		તોડો;
	સ્થિતિ '1':
	સ્થિતિ 'S':
	સ્થિતિ 's':
		cmd = "single";
		તોડો;
	સ્થિતિ '2':
	સ્થિતિ '3':
	સ્થિતિ '4':
	સ્થિતિ '5':
		/* TODO: switch to default runlevel? */
		પાછા;
	સ્થિતિ '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		તોડો;
	સ્થિતિ 'Q':
	સ્થિતિ 'q':
		પાછા;
	સ્થિતિ 'U':
	સ્થિતિ 'u':
		cmd = "reexec";
		તોડો;
	}

	જો ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		પાછા;
	}

	જો (લિખ(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

સ્થિર શૂન્ય sysvinit_setenv(અક્ષર *data, size_t size)
{
	data[size - 1] = '\0';
	માટે (અક્ષર *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		જો (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		નહીંતર જો (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

પૂર્ણાંક મુખ્ય(શૂન્ય) {
	પૂર્ણાંક fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	જો ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		પાછા 1;
	}

	માટે (;;) {
		સંરચના init_request req;
		ssize_t ગણતરી = પઠ(fifo, &req, કદ(req));

		જો (ગણતરી == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			ચાલુ;
		}

		જો (ગણતરી != કદ(req)) {
			syslog(LOG_ERR, "read: short count");
			ચાલુ;
		}

		જો (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			ચાલુ;
		}

		પસંદગી (req.cmd) {
		સ્થિતિ INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			તોડો;
		સ્થિતિ INIT_CMD_SETENV:
		સ્થિતિ INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, કદ(req.i.data));
			તોડો;
		}
	}
}
