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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "einfo.h"
#অন্তর্ভাৱ "initreq.h"
#অন্তর্ভাৱ "sysvinit.h"

স্থির শূন্য sysvinit_send_cmd(সংরচনা init_request *request)
{
	পূর্ণাঙ্ক fd;
	ৱর্ণ *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	যদি (fd < 0) {
		যদি (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ঘূৰাই;
	}
	p = (ৱর্ণ *) request;
	bytes = পরিমাণ(*request);
	কৰা {
		r = লিখ(fd, p, bytes);
		যদি (r < 0) {
			যদি ((errno == EAGAIN) || (errno == EINTR))
				অনুৱর্ত;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ঘূৰাই;
		}
		p += r;
		bytes -= r;
	} যেতিয়ালৈ (bytes > 0);
}

শূন্য sysvinit_runlevel(ৱর্ণ rl)
{
	সংরচনা init_request request;

	যদি (!rl)
		ঘূৰাই;

	request = (সংরচনা init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ঘূৰাই;
}

/*
 *	Set environment variables in the init process.
 */
শূন্য sysvinit_setenv(ধ্রুৱ ৱর্ণ *name, ধ্রুৱ ৱর্ণ *মান)
{
	সংরচনা init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, পরিমাণ(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	যদি (মান)
		vl = strlen(মান);
অন্যথা
		vl = 0;

	যদি (nl + vl + 3 >= (পূর্ণাঙ্ক)পরিমাণ(request.i.data))
		ঘূৰাই;

	memcpy(request.i.data, name, nl);
	যদি (মান) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, মান, vl);
	}
	sysvinit_send_cmd(&request);
	ঘূৰাই;
}
