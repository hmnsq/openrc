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

static void sysvinit_send_cmd(struct init_request *request)
{
	정수 fd;
	char *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	만약 (fd < 0) {
		만약 (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		반환;
	}
	p = (char *) request;
	bytes = sizeof(*request);
	do {
		r = write(fd, p, bytes);
		만약 (r < 0) {
			만약 ((errno == EAGAIN) || (errno == EINTR))
				continue;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			반환;
		}
		p += r;
		bytes -= r;
	} 동안 (bytes > 0);
}

void sysvinit_runlevel(char rl)
{
	struct init_request request;

	만약 (!rl)
		반환;

	request = (struct init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		반환;
}

/*
 *	Set environment variables in the init process.
 */
void sysvinit_setenv(const char *name, const char *value)
{
	struct init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, sizeof(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	만약 (value)
		vl = strlen(value);
아니면
		vl = 0;

	만약 (nl + vl + 3 >= (정수)sizeof(request.i.data))
		반환;

	memcpy(request.i.data, name, nl);
	만약 (value) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, value, vl);
	}
	sysvinit_send_cmd(&request);
	반환;
}
