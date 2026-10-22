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

ಸ್ಥಿರ ಶೂನ್ಯ sysvinit_runlevel(ಪೂರ್ಣಾಂಕ runlevel)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd;
	ಪೂರ್ಣಾಂಕ fifo;

	ಆಯ್ಕೆ (runlevel) {
	ಸ್ಥಿತಿ '0':
		cmd = init_halt ? "halt" : "poweroff";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '1':
	ಸ್ಥಿತಿ 'S':
	ಸ್ಥಿತಿ 's':
		cmd = "single";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '2':
	ಸ್ಥಿತಿ '3':
	ಸ್ಥಿತಿ '4':
	ಸ್ಥಿತಿ '5':
		/* TODO: switch to default runlevel? */
		ಹಿಂತಿರುಗಿ;
	ಸ್ಥಿತಿ '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'Q':
	ಸ್ಥಿತಿ 'q':
		ಹಿಂತಿರುಗಿ;
	ಸ್ಥಿತಿ 'U':
	ಸ್ಥಿತಿ 'u':
		cmd = "reexec";
		ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		ಹಿಂತಿರುಗಿ;
	}

	ಆದರೆ (ಲಿಖ(fifo, cmd, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ಸ್ಥಿರ ಶೂನ್ಯ sysvinit_setenv(ಅಕ್ಷರ *ದತ್ತಾಂಶ, size_t ಗಾತ್ರ_2)
{
	ದತ್ತಾಂಶ[ಗಾತ್ರ_2 - 1] = '\0';
	ಗಾಗಿ (ಅಕ್ಷರ *ಕೊನೆ = ದತ್ತಾಂಶ + ಗಾತ್ರ_2; ದತ್ತಾಂಶ < ಕೊನೆ && *ದತ್ತಾಂಶ; ದತ್ತಾಂಶ += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ದತ್ತಾಂಶ) + 1) {
		/* We only care about the INIT_HALT variable */
		ಆದರೆ (!strcmp(ದತ್ತಾಂಶ, "INIT_HALT=HALT"))
			init_halt = true;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!strcmp(ದತ್ತಾಂಶ, "INIT_HALT") || !ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ದತ್ತಾಂಶ, "INIT_HALT=", 10))
			init_halt = false;
	}
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಶೂನ್ಯ) {
	ಪೂರ್ಣಾಂಕ fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	ಆದರೆ ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		ಹಿಂತಿರುಗಿ 1;
	}

	ಗಾಗಿ (;;) {
		ರಚನೆ init_request req;
		ssize_t ಎಣಿಕೆ_2 = ಪಠ(fifo, &req, ಗಾತ್ರ(req));

		ಆದರೆ (ಎಣಿಕೆ_2 == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (ಎಣಿಕೆ_2 != ಗಾತ್ರ(req)) {
			syslog(LOG_ERR, "read: short count");
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			ಮುಂದುವರಿ;
		}

		ಆಯ್ಕೆ (req.cmd) {
		ಸ್ಥಿತಿ INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ INIT_CMD_SETENV:
		ಸ್ಥಿತಿ INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.ದತ್ತಾಂಶ, ಗಾತ್ರ(req.i.ದತ್ತಾಂಶ));
			ನಿಲ್ಲಿಸು;
		}
	}
}
