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

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

#અન્તર્ભાવ "einfo.h"
#અન્તર્ભાવ "initreq.h"
#અન્તર્ભાવ "sysvinit.h"

સ્થિર શૂન્ય sysvinit_send_cmd(સંરચના init_request *request)
{
	પૂર્ણાંક fd;
	અક્ષર *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	જો (fd < 0) {
		જો (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		પાછા;
	}
	p = (અક્ષર *) request;
	bytes = કદ(*request);
	કરો {
		r = લિખ(fd, p, bytes);
		જો (r < 0) {
			જો ((errno == EAGAIN) || (errno == EINTR))
				ચાલુ;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			પાછા;
		}
		p += r;
		bytes -= r;
	} જ્યારે (bytes > 0);
}

શૂન્ય sysvinit_runlevel(અક્ષર rl)
{
	સંરચના init_request request;

	જો (!rl)
		પાછા;

	request = (સંરચના init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		પાછા;
}

/*
 *	Set environment variables in the init process.
 */
શૂન્ય sysvinit_setenv(અચલ અક્ષર *name, અચલ અક્ષર *કિંમત)
{
	સંરચના init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, કદ(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	જો (કિંમત)
		vl = strlen(કિંમત);
નહીંતર
		vl = 0;

	જો (nl + vl + 3 >= (પૂર્ણાંક)કદ(request.i.data))
		પાછા;

	memcpy(request.i.data, name, nl);
	જો (કિંમત) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, કિંમત, vl);
	}
	sysvinit_send_cmd(&request);
	પાછા;
}
