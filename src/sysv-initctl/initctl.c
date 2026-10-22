#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <syslog.h>

#ಅನ್ತರ್ಭಾವ <rc.h>

#ಅನ್ತರ್ಭಾವ "initreq.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರ bool init_halt = false;

ಸ್ಥಿರ ಶೂನ್ಯ sysvinit_runlevel(ಪೂರ್ಣಾಙ್ಕ runlevel)
{
	ಧ್ರುವ ವರ್ಣ *cmd;
	ಪೂರ್ಣಾಙ್ಕ fifo;

	ವಿಕಲ್ಪ (runlevel) {
	ಪಕ್ಷ '0':
		cmd = init_halt ? "halt" : "poweroff";
		ಭಙ್ಗ;
	ಪಕ್ಷ '1':
	ಪಕ್ಷ 'S':
	ಪಕ್ಷ 's':
		cmd = "single";
		ಭಙ್ಗ;
	ಪಕ್ಷ '2':
	ಪಕ್ಷ '3':
	ಪಕ್ಷ '4':
	ಪಕ್ಷ '5':
		/* TODO: switch to default runlevel? */
		ಪ್ರತ್ಯಾಗಮ;
	ಪಕ್ಷ '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		ಭಙ್ಗ;
	ಪಕ್ಷ 'Q':
	ಪಕ್ಷ 'q':
		ಪ್ರತ್ಯಾಗಮ;
	ಪಕ್ಷ 'U':
	ಪಕ್ಷ 'u':
		cmd = "reexec";
		ಭಙ್ಗ;
	}

	ಯದಿ ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಯದಿ (ಲಿಖ(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ಸ್ಥಿರ ಶೂನ್ಯ sysvinit_setenv(ವರ್ಣ *data, size_t size)
{
	data[size - 1] = '\0';
	ಪ್ರತಿ (ವರ್ಣ *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		ಯದಿ (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		ಅನ್ಯಥಾ ಯದಿ (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

ಪೂರ್ಣಾಙ್ಕ main(ಶೂನ್ಯ) {
	ಪೂರ್ಣಾಙ್ಕ fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	ಯದಿ ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	ಪ್ರತಿ (;;) {
		ಸಂರಚನಾ init_request req;
		ssize_t ಎಣಿಕೆ = ಪಠ(fifo, &req, ಪರಿಮಾಣ(req));

		ಯದಿ (ಎಣಿಕೆ == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			ಅನುವರ್ತ;
		}

		ಯದಿ (ಎಣಿಕೆ != ಪರಿಮಾಣ(req)) {
			syslog(LOG_ERR, "read: short count");
			ಅನುವರ್ತ;
		}

		ಯದಿ (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			ಅನುವರ್ತ;
		}

		ವಿಕಲ್ಪ (req.cmd) {
		ಪಕ್ಷ INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			ಭಙ್ಗ;
		ಪಕ್ಷ INIT_CMD_SETENV:
		ಪಕ್ಷ INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, ಪರಿಮಾಣ(req.i.data));
			ಭಙ್ಗ;
		}
	}
}
