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
	पूर्णांक fd;
	अक्षर *p;
	माप_प्रकार bytes;
	ssize_t r;

	fd = खोलो("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	जर (fd < 0) {
		जर (त्रुटि_सं != ENOENT)
			eerror("Failed to open initctl fifo: %s", म_त्रुटि(त्रुटि_सं));
		परत;
	}
	p = (अक्षर *) request;
	bytes = आकार(*request);
	करा {
		r = लिख(fd, p, bytes);
		जर (r < 0) {
			जर ((त्रुटि_सं == EAGAIN) || (त्रुटि_सं == EINTR))
				सुरूठेवा;
			eerror("Failed to write to /run/initctl: %s", म_त्रुटि(त्रुटि_सं));
			परत;
		}
		p += r;
		bytes -= r;
	} पर्यंत (bytes > 0);
}

शून्य sysvinit_runlevel(अक्षर rl)
{
	संरचना init_request request;

	जर (!rl)
		परत;

	request = (संरचना init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		परत;
}

/*
 *	Set environment variables in the init process.
 */
शून्य sysvinit_setenv(स्थिरांक अक्षर *name, स्थिरांक अक्षर *मूल्य)
{
	संरचना init_request	request;
	माप_प्रकार nl;
	माप_प्रकार vl;

	स_रखो(&request, 0, आकार(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = म_माप(name);
	जर (मूल्य)
		vl = म_माप(मूल्य);
अन्यथा
		vl = 0;

	जर (nl + vl + 3 >= (पूर्णांक)आकार(request.i.data))
		परत;

	स_नकल(request.i.data, name, nl);
	जर (मूल्य) {
		request.i.data[nl] = '=';
		स_नकल(request.i.data + nl + 1, मूल्य, vl);
	}
	sysvinit_send_cmd(&request);
	परत;
}
