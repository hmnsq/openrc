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

#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <stddef.h>
#ضمن <string.h>
#ضمن <sys/stat.h>
#ضمن <unistd.h>

#ضمن "einfo.h"
#ضمن "initreq.h"
#ضمن "sysvinit.h"

ساكن فراغ sysvinit_send_cmd(بنية init_request *request)
{
	صحيح fd;
	حرف *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	إذا (fd < 0) {
		إذا (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		أرجع;
	}
	p = (حرف *) request;
	bytes = حجم(*request);
	افعل {
		r = اكتب(fd, p, bytes);
		إذا (r < 0) {
			إذا ((errno == EAGAIN) || (errno == EINTR))
				استمر;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			أرجع;
		}
		p += r;
		bytes -= r;
	} طالما (bytes > 0);
}

فراغ sysvinit_runlevel(حرف rl)
{
	بنية init_request request;

	إذا (!rl)
		أرجع;

	request = (بنية init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		أرجع;
}

/*
 *	Set environment variables in the init process.
 */
فراغ sysvinit_setenv(ثابت حرف *name, ثابت حرف *نيلاي)
{
	بنية init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, حجم(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	إذا (نيلاي)
		vl = strlen(نيلاي);
وإلا
		vl = 0;

	إذا (nl + vl + 3 >= (صحيح)حجم(request.i.data))
		أرجع;

	memcpy(request.i.data, name, nl);
	إذا (نيلاي) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, نيلاي, vl);
	}
	sysvinit_send_cmd(&request);
	أرجع;
}
