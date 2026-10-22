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

#شامل <errno.h>
#شامل <fcntl.h>
#شامل <stddef.h>
#شامل <string.h>
#شامل <sys/stat.h>
#شامل <unistd.h>

#شامل "einfo.h"
#شامل "initreq.h"
#شامل "sysvinit.h"

ایستا تهی sysvinit_send_cmd(ساختار init_request *request)
{
	صحیح fd;
	نویسه *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	اگر (fd < 0) {
		اگر (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		بازگشت;
	}
	p = (نویسه *) request;
	bytes = اندازه(*request);
	انجامبده {
		r = اكتب(fd, p, bytes);
		اگر (r < 0) {
			اگر ((errno == EAGAIN) || (errno == EINTR))
				ادامه;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			بازگشت;
		}
		p += r;
		bytes -= r;
	} وقتی (bytes > 0);
}

تهی sysvinit_runlevel(نویسه rl)
{
	ساختار init_request request;

	اگر (!rl)
		بازگشت;

	request = (ساختار init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		بازگشت;
}

/*
 *	Set environment variables in the init process.
 */
تهی sysvinit_setenv(ثابت نویسه *name, ثابت نویسه *مقدار)
{
	ساختار init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, اندازه(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	اگر (مقدار)
		vl = strlen(مقدار);
وگرنه
		vl = 0;

	اگر (nl + vl + 3 >= (صحیح)اندازه(request.i.data))
		بازگشت;

	memcpy(request.i.data, name, nl);
	اگر (مقدار) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, مقدار, vl);
	}
	sysvinit_send_cmd(&request);
	بازگشت;
}
