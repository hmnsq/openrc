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
	माप_प्रकार bytes;
	ssize_t r;

	fd = खोलो("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	यदि (fd < 0) {
		यदि (त्रुटि_सं != ENOENT)
			eerror("Failed to open initctl fifo: %s", म_त्रुटि(त्रुटि_सं));
		फर्काऊ;
	}
	p = (वर्ण *) request;
	bytes = परिमाण(*request);
	गर {
		r = लिख(fd, p, bytes);
		यदि (r < 0) {
			यदि ((त्रुटि_सं == EAGAIN) || (त्रुटि_सं == EINTR))
				जारी;
			eerror("Failed to write to /run/initctl: %s", म_त्रुटि(त्रुटि_सं));
			फर्काऊ;
		}
		p += r;
		bytes -= r;
	} जबसम्म (bytes > 0);
}

शून्य sysvinit_runlevel(वर्ण rl)
{
	संरचना init_request request;

	यदि (!rl)
		फर्काऊ;

	request = (संरचना init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		फर्काऊ;
}

/*
 *	Set environment variables in the init process.
 */
शून्य sysvinit_setenv(अचर वर्ण *name, अचर वर्ण *मान)
{
	संरचना init_request	request;
	माप_प्रकार nl;
	माप_प्रकार vl;

	स_रखो(&request, 0, परिमाण(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = म_माप(name);
	यदि (मान)
		vl = म_माप(मान);
अन्यथा
		vl = 0;

	यदि (nl + vl + 3 >= (पूर्णाङ्क)परिमाण(request.i.data))
		फर्काऊ;

	स_नकल(request.i.data, name, nl);
	यदि (मान) {
		request.i.data[nl] = '=';
		स_नकल(request.i.data + nl + 1, मान, vl);
	}
	sysvinit_send_cmd(&request);
	फर्काऊ;
}
