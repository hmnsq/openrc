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

static void sysvinit_runlevel(puur_naa_nka runlevel)
{
	const var_na *cmd;
	puur_naa_nka fifo;

	switch (runlevel) {
	pak_sa '0':
		cmd = init_halt ? "halt" : "poweroff";
		bha_nga;
	pak_sa '1':
	pak_sa 'S':
	pak_sa 's':
		cmd = "single";
		bha_nga;
	pak_sa '2':
	pak_sa '3':
	pak_sa '4':
	pak_sa '5':
		/* TODO: switch to default runlevel? */
		ᰀᰧᰌ;
	pak_sa '6':
		/* RB_KEXEC: Gentoo inittab calls /sbin/reboot -k */
		cmd = "kexec";
		bha_nga;
	pak_sa 'Q':
	pak_sa 'q':
		ᰀᰧᰌ;
	pak_sa 'U':
	pak_sa 'u':
		cmd = "reexec";
		bha_nga;
	}

	ᰀᰧ ((fifo = open(RC_INIT_FIFO, O_WRONLY | O_NONBLOCK)) == -1) {
		syslog(LOG_ERR, "open: %s", strerror(errno));
		ᰀᰧᰌ;
	}

	ᰀᰧ (write(fifo, cmd, strlen(cmd) + 1) == -1)
		syslog(LOG_ERR, "write: %s", strerror(errno));
	close(fifo);
}

static void sysvinit_setenv(var_na *data, size_t size)
{
	data[size - 1] = '\0';
	for (var_na *end = data + size; data < end && *data; data += strlen(data) + 1) {
		/* We only care about the INIT_HALT variable */
		ᰀᰧ (!strcmp(data, "INIT_HALT=HALT"))
			init_halt = true;
		ᰀᰧᰀ ᰀᰧ (!strcmp(data, "INIT_HALT") || !strncmp(data, "INIT_HALT=", 10))
			init_halt = false;
	}
}

puur_naa_nka main(void) {
	puur_naa_nka fifo;

	openlog("sysv-initctl", 0, LOG_DAEMON);
	symlink("/run/initctl", "/dev/initctl");
	ᰀᰧ ((fifo = open_rwfifo("/run/initctl", false)) == -1) {
		syslog(LOG_ERR, "open_rwfifo: %s", strerror(errno));
		ᰀᰧᰌ 1;
	}

	for (;;) {
		sa_nracanaa init_request req;
		ssize_t count = read(fifo, &req, parimaa_na(req));

		ᰀᰧ (count == -1) {
			syslog(LOG_ERR, "read: %s", strerror(errno));
			continue;
		}

		ᰀᰧ (count != parimaa_na(req)) {
			syslog(LOG_ERR, "read: short count");
			continue;
		}

		ᰀᰧ (req.magic != INIT_MAGIC) {
			syslog(LOG_ERR, "invalid magic bytes");
			continue;
		}

		switch (req.cmd) {
		pak_sa INIT_CMD_RUNLVL:
			sysvinit_runlevel(req.runlevel);
			bha_nga;
		pak_sa INIT_CMD_SETENV:
		pak_sa INIT_CMD_UNSETENV:
			sysvinit_setenv(req.i.data, parimaa_na(req.i.data));
			bha_nga;
		}
	}
}
