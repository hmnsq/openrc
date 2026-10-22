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

#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "einfo.h"
#అన్తర్భావ "initreq.h"
#అన్తర్భావ "sysvinit.h"

స్థిర శూన్య sysvinit_send_cmd(నిర్మాణం init_request *request)
{
	పూర్ణాంకం fd;
	అక్షరం *p;
	size_t బైట్లు;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	ఒకవేళ (fd < 0) {
		ఒకవేళ (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		తిరిగి;
	}
	p = (అక్షరం *) request;
	బైట్లు = పరిమాణం(*request);
	చేయి {
		r = లిఖ(fd, p, బైట్లు);
		ఒకవేళ (r < 0) {
			ఒకవేళ ((errno == EAGAIN) || (errno == EINTR))
				కొనసాగు;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			తిరిగి;
		}
		p += r;
		బైట్లు -= r;
	} అయితే (బైట్లు > 0);
}

శూన్య sysvinit_runlevel(అక్షరం rl)
{
	నిర్మాణం init_request request;

	ఒకవేళ (!rl)
		తిరిగి;

	request = (నిర్మాణం init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		తిరిగి;
}

/*
 *	Set environment variables in the init process.
 */
శూన్య sysvinit_setenv(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *విలువ)
{
	నిర్మాణం init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, పరిమాణం(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = అక్షరశ్రేణి_పొడవు(name);
	ఒకవేళ (విలువ)
		vl = అక్షరశ్రేణి_పొడవు(విలువ);
లేకపోతే
		vl = 0;

	ఒకవేళ (nl + vl + 3 >= (పూర్ణాంకం)పరిమాణం(request.i.దత్తాంశం))
		తిరిగి;

	జ్ఞాపకం_నకలు(request.i.దత్తాంశం, name, nl);
	ఒకవేళ (విలువ) {
		request.i.దత్తాంశం[nl] = '=';
		జ్ఞాపకం_నకలు(request.i.దత్తాంశం + nl + 1, విలువ, vl);
	}
	sysvinit_send_cmd(&request);
	తిరిగి;
}
