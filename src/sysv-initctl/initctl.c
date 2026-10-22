#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>

#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <syslog.h>

#అన్తర్భావ <rc.h>

#అన్తర్భావ "initreq.h"
#అన్తర్భావ "helpers.h"

స్థిర bool init_halt = false;

స్థిర శూన్య sysvinit_runlevel(పూర్ణాంకం runlevel)
{
	స్థిరరాశి అక్షరం *cmd;
	పూర్ణాంకం fifo;

	ఎంపిక (runlevel) {
	స్థితి '0':
		cmd = init_halt ? "halt" : "poweroff";
		ఆపు;
	స్థితి '1':
	స్థితి 'S':
	స్థితి 's':
		cmd = "single";
		ఆపు;
	స్థితి '2':
	స్థితి '3':
	స్థితి '4':
	స్థితి '5':
		/* TODO: switch to default runlevel? */
		తిరిగి;
	స్థితి '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		ఆపు;
	స్థితి 'Q':
	స్థితి 'q':
		తిరిగి;
	స్థితి 'U':
	స్థితి 'u':
		cmd = "reexec";
		ఆపు;
	}

	ఒకవేళ ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		తిరిగి;
	}

	ఒకవేళ (లిఖ(fifo, cmd, అక్షరశ్రేణి_పొడవు(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

స్థిర శూన్య sysvinit_setenv(అక్షరం *దత్తాంశం, size_t కొలత)
{
	దత్తాంశం[కొలత - 1] = '\0';
	కొరకు (అక్షరం *చివర = దత్తాంశం + కొలత; దత్తాంశం < చివర && *దత్తాంశం; దత్తాంశం += అక్షరశ్రేణి_పొడవు(దత్తాంశం) + 1) {
		/* We only care about the INIT_HALT variable */
		ఒకవేళ (!strcmp(దత్తాంశం, "INIT_HALT=HALT"))
			init_halt = true;
		లేకపోతే ఒకవేళ (!strcmp(దత్తాంశం, "INIT_HALT") || !అక్షరశ్రేణి_పోలిక(దత్తాంశం, "INIT_HALT=", 10))
			init_halt = false;
	}
}

పూర్ణాంకం ప్రధాన(శూన్య) {
	పూర్ణాంకం fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	ఒకవేళ ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		తిరిగి 1;
	}

	కొరకు (;;) {
		నిర్మాణం init_request req;
		ssize_t లెక్క = పఠ(fifo, &req, పరిమాణం(req));

		ఒకవేళ (లెక్క == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			కొనసాగు;
		}

		ఒకవేళ (లెక్క != పరిమాణం(req)) {
			syslog(LOG_ERR, "read: short count");
			కొనసాగు;
		}

		ఒకవేళ (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			కొనసాగు;
		}

		ఎంపిక (req.cmd) {
		స్థితి INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			ఆపు;
		స్థితి INIT_CMD_SETENV:
		స్థితి INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.దత్తాంశం, పరిమాణం(req.i.దత్తాంశం));
			ఆపు;
		}
	}
}
