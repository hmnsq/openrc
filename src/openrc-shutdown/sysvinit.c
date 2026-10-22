/*
 * sysvinit.c
 * Helper to send a runlevel change to sysvinit
 */

/*
 * Copyright (c) 2019 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "initreq.h"
#ಅನ್ತರ್ಭಾವ "sysvinit.h"

ಸ್ಥಿರ ಶೂನ್ಯ sysvinit_send_cmd(ಸಂರಚನಾ init_request *request)
{
	ಪೂರ್ಣಾಙ್ಕ fd;
	ವರ್ಣ *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	ಯದಿ (fd < 0) {
		ಯದಿ (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ಪ್ರತ್ಯಾಗಮ;
	}
	p = (ವರ್ಣ *) request;
	bytes = ಪರಿಮಾಣ(*request);
	ಕುರು {
		r = ಲಿಖ(fd, p, bytes);
		ಯದಿ (r < 0) {
			ಯದಿ ((errno == EAGAIN) || (errno == EINTR))
				ಅನುವರ್ತ;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ಪ್ರತ್ಯಾಗಮ;
		}
		p += r;
		bytes -= r;
	} ಯಾವತ್ (bytes > 0);
}

ಶೂನ್ಯ sysvinit_runlevel(ವರ್ಣ rl)
{
	ಸಂರಚನಾ init_request request;

	ಯದಿ (!rl)
		ಪ್ರತ್ಯಾಗಮ;

	request = (ಸಂರಚನಾ init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ಪ್ರತ್ಯಾಗಮ;
}

/*
 *	Set environment variables in the init process.
 */
ಶೂನ್ಯ sysvinit_setenv(ಧ್ರುವ ವರ್ಣ *name, ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ)
{
	ಸಂರಚನಾ init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, ಪರಿಮಾಣ(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	ಯದಿ (ಮೌಲ್ಯ)
		vl = strlen(ಮೌಲ್ಯ);
ಅನ್ಯಥಾ
		vl = 0;

	ಯದಿ (nl + vl + 3 >= (ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(request.i.data))
		ಪ್ರತ್ಯಾಗಮ;

	memcpy(request.i.data, name, nl);
	ಯದಿ (ಮೌಲ್ಯ) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, ಮೌಲ್ಯ, vl);
	}
	sysvinit_send_cmd(&request);
	ಪ್ರತ್ಯಾಗಮ;
}
