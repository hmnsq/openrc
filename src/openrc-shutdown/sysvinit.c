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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "einfo.h"
#অন্তর্ভাব "initreq.h"
#অন্তর্ভাব "sysvinit.h"

স্থির শূন্য sysvinit_send_cmd(কাঠামো init_request *request)
{
	পূর্ণসংখ্যা fd;
	অক্ষর *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	যদি (fd < 0) {
		যদি (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ফেরত;
	}
	p = (অক্ষর *) request;
	bytes = আকার(*request);
	করো {
		r = লিখ(fd, p, bytes);
		যদি (r < 0) {
			যদি ((errno == EAGAIN) || (errno == EINTR))
				চালু;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ফেরত;
		}
		p += r;
		bytes -= r;
	} যখন (bytes > 0);
}

শূন্য sysvinit_runlevel(অক্ষর rl)
{
	কাঠামো init_request request;

	যদি (!rl)
		ফেরত;

	request = (কাঠামো init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ফেরত;
}

/*
 *	Set environment variables in the init process.
 */
শূন্য sysvinit_setenv(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *মান)
{
	কাঠামো init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, আকার(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	যদি (মান)
		vl = strlen(মান);
অন্যথা
		vl = 0;

	যদি (nl + vl + 3 >= (পূর্ণসংখ্যা)আকার(request.i.তথ্য))
		ফেরত;

	memcpy(request.i.তথ্য, name, nl);
	যদি (মান) {
		request.i.তথ্য[nl] = '=';
		memcpy(request.i.তথ্য + nl + 1, মান, vl);
	}
	sysvinit_send_cmd(&request);
	ফেরত;
}
