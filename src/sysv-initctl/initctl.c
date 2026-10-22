#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include <rc.h>

#include "initreq.h"
#include "helpers.h"

static bool init_halt = false;

static void sysvinit_runlevel(целое runlevel)
{
	const char *cmd;
	целое fifo;

	switch (runlevel) {
	case '0':
		cmd = init_halt ? "halt" : "poweroff";
		break;
	case '1':
	case 'S':
	case 's':
		cmd = "single";
		break;
	case '2':
	case '3':
	case '4':
	case '5':
		/* TODO: switch to default runlevel? */
		вернуть;
	case '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		break;
	case 'Q':
	case 'q':
		вернуть;
	case 'U':
	case 'u':
		cmd = "reexec";
		break;
	}

	если ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		вернуть;
	}

	если (write(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

static void sysvinit_setenv(char *data, size_t size)
{
	data[size - 1] = '\0';
	для (char *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		если (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		иначе если (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

целое основной(void) {
	целое fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	если ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		вернуть 1;
	}

	для (;;) {
		struct init_request req;
		ssize_t count = read(fifo, &req, sizeof(req));

		если (count == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			continue;
		}

		если (count != sizeof(req)) {
			syslog(LOG_ERR, "read: short count");
			continue;
		}

		если (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			continue;
		}

		switch (req.cmd) {
		case INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			break;
		case INIT_CMD_SETENV:
		case INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, sizeof(req.i.data));
			break;
		}
	}
}
