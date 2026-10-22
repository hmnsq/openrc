#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <syslog.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <rc.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "initreq.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"

ਸਥਿਰ bool init_halt = false;

ਸਥਿਰ ਖਾਲੀ sysvinit_runlevel(ਪੂਰਨ runlevel)
{
	ਅਚਲ ਅੱਖਰ *cmd;
	ਪੂਰਨ fifo;

	ਚੋਣ (runlevel) {
	ਹਾਲਤ '0':
		cmd = init_halt ? "halt" : "poweroff";
		ਰੋਕੋ;
	ਹਾਲਤ '1':
	ਹਾਲਤ 'S':
	ਹਾਲਤ 's':
		cmd = "single";
		ਰੋਕੋ;
	ਹਾਲਤ '2':
	ਹਾਲਤ '3':
	ਹਾਲਤ '4':
	ਹਾਲਤ '5':
		/* TODO: switch to default runlevel? */
		ਵਾਪਸ;
	ਹਾਲਤ '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		ਰੋਕੋ;
	ਹਾਲਤ 'Q':
	ਹਾਲਤ 'q':
		ਵਾਪਸ;
	ਹਾਲਤ 'U':
	ਹਾਲਤ 'u':
		cmd = "reexec";
		ਰੋਕੋ;
	}

	ਜੇ ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		ਵਾਪਸ;
	}

	ਜੇ (ਲਿਖ(fifo, cmd, ਲੜੀ_ਲੰਬਾਈ(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

ਸਥਿਰ ਖਾਲੀ sysvinit_setenv(ਅੱਖਰ *ਅੰਕੜੇ, size_t ਮਾਪ)
{
	ਅੰਕੜੇ[ਮਾਪ - 1] = '\0';
	ਲਈ (ਅੱਖਰ *ਸਿਰਾ = ਅੰਕੜੇ + ਮਾਪ; ਅੰਕੜੇ < ਸਿਰਾ && *ਅੰਕੜੇ; ਅੰਕੜੇ += ਲੜੀ_ਲੰਬਾਈ(ਅੰਕੜੇ) + 1) {
		/* We only care about the INIT_HALT variable */
		ਜੇ (!strcmp(ਅੰਕੜੇ, "INIT_HALT=HALT"))
			init_halt = true;
		ਨਹੀਂਤਾਂ ਜੇ (!strcmp(ਅੰਕੜੇ, "INIT_HALT") || !ਲੜੀ_ਤੁਲਨਾ(ਅੰਕੜੇ, "INIT_HALT=", 10))
			init_halt = false;
	}
}

ਪੂਰਨ ਮੁੱਖ(ਖਾਲੀ) {
	ਪੂਰਨ fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	ਜੇ ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		ਵਾਪਸ 1;
	}

	ਲਈ (;;) {
		ਬਣਤਰ init_request req;
		ssize_t ਗਿਣਤੀ_2 = ਪਠ(fifo, &req, ਆਕਾਰ(req));

		ਜੇ (ਗਿਣਤੀ_2 == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			ਜਾਰੀ;
		}

		ਜੇ (ਗਿਣਤੀ_2 != ਆਕਾਰ(req)) {
			syslog(LOG_ERR, "read: short count");
			ਜਾਰੀ;
		}

		ਜੇ (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			ਜਾਰੀ;
		}

		ਚੋਣ (req.cmd) {
		ਹਾਲਤ INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			ਰੋਕੋ;
		ਹਾਲਤ INIT_CMD_SETENV:
		ਹਾਲਤ INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.ਅੰਕੜੇ, ਆਕਾਰ(req.i.ਅੰਕੜੇ));
			ਰੋਕੋ;
		}
	}
}
