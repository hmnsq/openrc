#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <syslog.h>

#அந்தர்பாவ <rc.h>

#அந்தர்பாவ "initreq.h"
#அந்தர்பாவ "helpers.h"

நிலையான bool init_halt = false;

நிலையான வெறுமை sysvinit_runlevel(முழு runlevel)
{
	மாறிலி எழுத்து *cmd;
	முழு fifo;

	தேர்வு (runlevel) {
	நிலை '0':
		cmd = init_halt ? "halt" : "poweroff";
		நிறுத்து;
	நிலை '1':
	நிலை 'S':
	நிலை 's':
		cmd = "single";
		நிறுத்து;
	நிலை '2':
	நிலை '3':
	நிலை '4':
	நிலை '5':
		/* TODO: switch to default runlevel? */
		திரும்பு;
	நிலை '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		நிறுத்து;
	நிலை 'Q':
	நிலை 'q':
		திரும்பு;
	நிலை 'U':
	நிலை 'u':
		cmd = "reexec";
		நிறுத்து;
	}

	எனில் ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		திரும்பு;
	}

	எனில் (லிக(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

நிலையான வெறுமை sysvinit_setenv(எழுத்து *data, size_t size)
{
	data[size - 1] = '\0';
	க்கு (எழுத்து *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		எனில் (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		இல்லையெனில் எனில் (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

முழு முதன்மை(வெறுமை) {
	முழு fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	எனில் ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		திரும்பு 1;
	}

	க்கு (;;) {
		கட்டமைப்பு init_request req;
		ssize_t மொத்தம் = பட(fifo, &req, அளவு(req));

		எனில் (மொத்தம் == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			தொடர்;
		}

		எனில் (மொத்தம் != அளவு(req)) {
			syslog(LOG_ERR, "read: short count");
			தொடர்;
		}

		எனில் (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			தொடர்;
		}

		தேர்வு (req.cmd) {
		நிலை INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			நிறுத்து;
		நிலை INIT_CMD_SETENV:
		நிலை INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, அளவு(req.i.data));
			நிறுத்து;
		}
	}
}
