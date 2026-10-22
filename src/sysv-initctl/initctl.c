#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <syslog.h>

#അന്തര്ഭാവ <rc.h>

#അന്തര്ഭാവ "initreq.h"
#അന്തര്ഭാവ "helpers.h"

സ്ഥിതം bool init_halt = false;

സ്ഥിതം ശൂന്യം sysvinit_runlevel(പൂർണ്ണസംഖ്യ runlevel)
{
	സ്ഥിരം അക്ഷരം *cmd;
	പൂർണ്ണസംഖ്യ fifo;

	തിരഞ്ഞെടുപ്പ് (runlevel) {
	അവസ്ഥ '0':
		cmd = init_halt ? "halt" : "poweroff";
		നിർത്തുക;
	അവസ്ഥ '1':
	അവസ്ഥ 'S':
	അവസ്ഥ 's':
		cmd = "single";
		നിർത്തുക;
	അവസ്ഥ '2':
	അവസ്ഥ '3':
	അവസ്ഥ '4':
	അവസ്ഥ '5':
		/* TODO: switch to default runlevel? */
		തിരികെ;
	അവസ്ഥ '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		നിർത്തുക;
	അവസ്ഥ 'Q':
	അവസ്ഥ 'q':
		തിരികെ;
	അവസ്ഥ 'U':
	അവസ്ഥ 'u':
		cmd = "reexec";
		നിർത്തുക;
	}

	എങ്കിൽ ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		തിരികെ;
	}

	എങ്കിൽ (ലിഖ(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

സ്ഥിതം ശൂന്യം sysvinit_setenv(അക്ഷരം *data, size_t size)
{
	data[size - 1] = '\0';
	വേണ്ടി (അക്ഷരം *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		എങ്കിൽ (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		അല്ലെങ്കിൽ എങ്കിൽ (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

പൂർണ്ണസംഖ്യ പ്രധാന(ശൂന്യം) {
	പൂർണ്ണസംഖ്യ fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	എങ്കിൽ ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		തിരികെ 1;
	}

	വേണ്ടി (;;) {
		ഘടന init_request req;
		ssize_t എണ്ണം = പഠ(fifo, &req, വലുപ്പം(req));

		എങ്കിൽ (എണ്ണം == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			തുടരുക;
		}

		എങ്കിൽ (എണ്ണം != വലുപ്പം(req)) {
			syslog(LOG_ERR, "read: short count");
			തുടരുക;
		}

		എങ്കിൽ (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			തുടരുക;
		}

		തിരഞ്ഞെടുപ്പ് (req.cmd) {
		അവസ്ഥ INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			നിർത്തുക;
		അവസ്ഥ INIT_CMD_SETENV:
		അവസ്ഥ INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, വലുപ്പം(req.i.data));
			നിർത്തുക;
		}
	}
}
