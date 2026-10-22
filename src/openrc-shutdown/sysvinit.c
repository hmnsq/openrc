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

#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <stddef.h>
#समावेश <string.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

#समावेश "einfo.h"
#समावेश "initreq.h"
#समावेश "sysvinit.h"

अटल व्योम sysvinit_send_cmd(काष्ठा init_request *request)
{
	पूर्णांक fd;
	अक्षर *p;
	माप_प्रकार bytes;
	ssize_t r;

	fd = खोलो("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	अगर (fd < 0) {
		अगर (त्रुटि_सं != ENOENT)
			eerror("Failed to open initctl fifo: %s", म_त्रुटि(त्रुटि_सं));
		वापस;
	}
	p = (अक्षर *) request;
	bytes = माप(*request);
	करो {
		r = ग_लिखो(fd, p, bytes);
		अगर (r < 0) {
			अगर ((त्रुटि_सं == EAGAIN) || (त्रुटि_सं == EINTR))
				जारी;
			eerror("Failed to write to /run/initctl: %s", म_त्रुटि(त्रुटि_सं));
			वापस;
		}
		p += r;
		bytes -= r;
	} जबतक (bytes > 0);
}

व्योम sysvinit_runlevel(अक्षर rl)
{
	काष्ठा init_request request;

	अगर (!rl)
		वापस;

	request = (काष्ठा init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		वापस;
}

/*
 *	Set environment variables in the init process.
 */
व्योम sysvinit_setenv(स्थिर अक्षर *name, स्थिर अक्षर *मूल्य)
{
	काष्ठा init_request	request;
	माप_प्रकार nl;
	माप_प्रकार vl;

	स_रखो(&request, 0, माप(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = म_माप(name);
	अगर (मूल्य)
		vl = म_माप(मूल्य);
अन्यथा
		vl = 0;

	अगर (nl + vl + 3 >= (पूर्णांक)माप(request.i.तथ्य))
		वापस;

	स_नकल(request.i.तथ्य, name, nl);
	अगर (मूल्य) {
		request.i.तथ्य[nl] = '=';
		स_नकल(request.i.तथ्य + nl + 1, मूल्य, vl);
	}
	sysvinit_send_cmd(&request);
	वापस;
}
