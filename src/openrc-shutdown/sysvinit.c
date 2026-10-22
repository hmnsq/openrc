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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "initreq.h"
#अन्तर्भाव "sysvinit.h"

स्थिर शून्य sysvinit_send_cmd(संरचना init_request *request)
{
	पूर्णाङ्क fd;
	वर्ण *p;
	size_t अष्टकानि;
	ssize_t r;

	fd = उद्घाटय("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	यदि (fd < 0) {
		यदि (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		प्रत्यागम;
	}
	p = (वर्ण *) request;
	अष्टकानि = परिमाण(*request);
	कुरु {
		r = लिख(fd, p, अष्टकानि);
		यदि (r < 0) {
			यदि ((errno == EAGAIN) || (errno == EINTR))
				अनुवर्त;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			प्रत्यागम;
		}
		p += r;
		अष्टकानि -= r;
	} यावत् (अष्टकानि > 0);
}

शून्य sysvinit_runlevel(वर्ण rl)
{
	संरचना init_request request;

	यदि (!rl)
		प्रत्यागम;

	request = (संरचना init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		प्रत्यागम;
}

/*
 *	Set environment variables in the init process.
 */
शून्य sysvinit_setenv(ध्रुव वर्ण *name, ध्रुव वर्ण *मानम्)
{
	संरचना init_request	request;
	size_t nl;
	size_t vl;

	स्मृतिं_पूरय(&request, 0, परिमाण(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = वाक्यदैर्घ्यम्(name);
	यदि (मानम्)
		vl = वाक्यदैर्घ्यम्(मानम्);
अन्यथा
		vl = 0;

	यदि (nl + vl + 3 >= (पूर्णाङ्क)परिमाण(request.i.दत्त))
		प्रत्यागम;

	स्मृतिं_प्रतिलिख(request.i.दत्त, name, nl);
	यदि (मानम्) {
		request.i.दत्त[nl] = '=';
		स्मृतिं_प्रतिलिख(request.i.दत्त + nl + 1, मानम्, vl);
	}
	sysvinit_send_cmd(&request);
	प्रत्यागम;
}
