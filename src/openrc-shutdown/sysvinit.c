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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "einfo.h"
#ଅନ୍ତର୍ଭାଵ "initreq.h"
#ଅନ୍ତର୍ଭାଵ "sysvinit.h"

ସ୍ଥିର ଶୂନ୍ୟ sysvinit_send_cmd(ସଂରଚନା init_request *request)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ଅକ୍ଷର *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	ଯଦି (fd < 0) {
		ଯଦି (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ଫେରନ୍ତୁ;
	}
	p = (ଅକ୍ଷର *) request;
	bytes = ଆକାର(*request);
	କର {
		r = ଲିଖ(fd, p, bytes);
		ଯଦି (r < 0) {
			ଯଦି ((errno == EAGAIN) || (errno == EINTR))
				ଜାରି;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ଫେରନ୍ତୁ;
		}
		p += r;
		bytes -= r;
	} ଯେତେବେଳେ (bytes > 0);
}

ଶୂନ୍ୟ sysvinit_runlevel(ଅକ୍ଷର rl)
{
	ସଂରଚନା init_request request;

	ଯଦି (!rl)
		ଫେରନ୍ତୁ;

	request = (ସଂରଚନା init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ଫେରନ୍ତୁ;
}

/*
 *	Set environment variables in the init process.
 */
ଶୂନ୍ୟ sysvinit_setenv(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ)
{
	ସଂରଚନା init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, ଆକାର(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	ଯଦି (ମୂଲ୍ୟ)
		vl = strlen(ମୂଲ୍ୟ);
ଅନ୍ୟଥା
		vl = 0;

	ଯଦି (nl + vl + 3 >= (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(request.i.data))
		ଫେରନ୍ତୁ;

	memcpy(request.i.data, name, nl);
	ଯଦି (ମୂଲ୍ୟ) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, ମୂଲ୍ୟ, vl);
	}
	sysvinit_send_cmd(&request);
	ଫେରନ୍ତୁ;
}
