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

ساکن خالی sysvinit_send_cmd(ساخت init_request *request)
{
	صحیح fd;
	حرف *p;
	size_t بائٹیں;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	اگر (fd < 0) {
		اگر (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		واپس;
	}
	p = (حرف *) request;
	بائٹیں = حجم(*request);
	کرو {
		r = اكتب(fd, p, بائٹیں);
		اگر (r < 0) {
			اگر ((errno == EAGAIN) || (errno == EINTR))
				جاری;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			واپس;
		}
		p += r;
		بائٹیں -= r;
	} جب (بائٹیں > 0);
}

خالی sysvinit_runlevel(حرف rl)
{
	ساخت init_request request;

	اگر (!rl)
		واپس;

	request = (ساخت init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		واپس;
}

/*
 *	Set environment variables in the init process.
 */
خالی sysvinit_setenv(ثابت حرف *name, ثابت حرف *قدر)
{
	ساخت init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, حجم(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = لڑی_لمبائی(name);
	اگر (قدر)
		vl = لڑی_لمبائی(قدر);
ورنہ
		vl = 0;

	اگر (nl + vl + 3 >= (صحیح)حجم(request.i.ڈیٹا))
		واپس;

	حافظہ_نقل(request.i.ڈیٹا, name, nl);
	اگر (قدر) {
		request.i.ڈیٹا[nl] = '=';
		حافظہ_نقل(request.i.ڈیٹا + nl + 1, قدر, vl);
	}
	sysvinit_send_cmd(&request);
	واپس;
}
