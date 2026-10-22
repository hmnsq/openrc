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

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "einfo.h"
#include "initreq.h"
#include "sysvinit.h"

static void sysvinit_send_cmd(sa_nracanaa init_request *request)
{
	puur_naa_nka fd;
	var_na *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	ᮚᮦᮔ᮪ (fd < 0) {
		ᮚᮦᮔ᮪ (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ᮝᮜᮤ;
	}
	p = (var_na *) request;
	bytes = parimaa_na(*request);
	do {
		r = write(fd, p, bytes);
		ᮚᮦᮔ᮪ (r < 0) {
			ᮚᮦᮔ᮪ ((errno == EAGAIN) || (errno == EINTR))
				continue;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ᮝᮜᮤ;
		}
		p += r;
		bytes -= r;
	} ᮞᮨᮓᮨᮜ (bytes > 0);
}

void sysvinit_runlevel(var_na rl)
{
	sa_nracanaa init_request request;

	ᮚᮦᮔ᮪ (!rl)
		ᮝᮜᮤ;

	request = (sa_nracanaa init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ᮝᮜᮤ;
}

/*
 *	Set environment variables in the init process.
 */
void sysvinit_setenv(const var_na *name, const var_na *value)
{
	sa_nracanaa init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, parimaa_na(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	ᮚᮦᮔ᮪ (value)
		vl = strlen(value);
ᮜᮤᮚ
		vl = 0;

	ᮚᮦᮔ᮪ (nl + vl + 3 >= (puur_naa_nka)parimaa_na(request.i.data))
		ᮝᮜᮤ;

	memcpy(request.i.data, name, nl);
	ᮚᮦᮔ᮪ (value) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, value, vl);
	}
	sysvinit_send_cmd(&request);
	ᮝᮜᮤ;
}
