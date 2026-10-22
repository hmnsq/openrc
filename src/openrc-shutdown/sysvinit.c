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

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "initreq.h"
#அந்தர்பாவ "sysvinit.h"

நிலையான வெறுமை sysvinit_send_cmd(கட்டமைப்பு init_request *request)
{
	முழு fd;
	எழுத்து *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	எனில் (fd < 0) {
		எனில் (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		திரும்பு;
	}
	p = (எழுத்து *) request;
	bytes = அளவு(*request);
	செய் {
		r = லிக(fd, p, bytes);
		எனில் (r < 0) {
			எனில் ((errno == EAGAIN) || (errno == EINTR))
				தொடர்;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			திரும்பு;
		}
		p += r;
		bytes -= r;
	} போது (bytes > 0);
}

வெறுமை sysvinit_runlevel(எழுத்து rl)
{
	கட்டமைப்பு init_request request;

	எனில் (!rl)
		திரும்பு;

	request = (கட்டமைப்பு init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		திரும்பு;
}

/*
 *	Set environment variables in the init process.
 */
வெறுமை sysvinit_setenv(மாறிலி எழுத்து *name, மாறிலி எழுத்து *மதிப்பு)
{
	கட்டமைப்பு init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, அளவு(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	எனில் (மதிப்பு)
		vl = strlen(மதிப்பு);
இல்லையெனில்
		vl = 0;

	எனில் (nl + vl + 3 >= (முழு)அளவு(request.i.data))
		திரும்பு;

	memcpy(request.i.data, name, nl);
	எனில் (மதிப்பு) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, மதிப்பு, vl);
	}
	sysvinit_send_cmd(&request);
	திரும்பு;
}
