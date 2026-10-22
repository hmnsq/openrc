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

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

#അന്തര്ഭാവ "einfo.h"
#അന്തര്ഭാവ "initreq.h"
#അന്തര്ഭാവ "sysvinit.h"

സ്ഥിതം ശൂന്യം sysvinit_send_cmd(ഘടന init_request *request)
{
	പൂർണ്ണസംഖ്യ fd;
	അക്ഷരം *p;
	size_t bytes;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	എങ്കിൽ (fd < 0) {
		എങ്കിൽ (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		തിരികെ;
	}
	p = (അക്ഷരം *) request;
	bytes = വലുപ്പം(*request);
	ചെയ്യുക {
		r = ലിഖ(fd, p, bytes);
		എങ്കിൽ (r < 0) {
			എങ്കിൽ ((errno == EAGAIN) || (errno == EINTR))
				തുടരുക;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			തിരികെ;
		}
		p += r;
		bytes -= r;
	} ആകുമ്പോൾ (bytes > 0);
}

ശൂന്യം sysvinit_runlevel(അക്ഷരം rl)
{
	ഘടന init_request request;

	എങ്കിൽ (!rl)
		തിരികെ;

	request = (ഘടന init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		തിരികെ;
}

/*
 *	Set environment variables in the init process.
 */
ശൂന്യം sysvinit_setenv(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *മൂല്യം)
{
	ഘടന init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, വലുപ്പം(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = strlen(name);
	എങ്കിൽ (മൂല്യം)
		vl = strlen(മൂല്യം);
അല്ലെങ്കിൽ
		vl = 0;

	എങ്കിൽ (nl + vl + 3 >= (പൂർണ്ണസംഖ്യ)വലുപ്പം(request.i.data))
		തിരികെ;

	memcpy(request.i.data, name, nl);
	എങ്കിൽ (മൂല്യം) {
		request.i.data[nl] = '=';
		memcpy(request.i.data + nl + 1, മൂല്യം, vl);
	}
	sysvinit_send_cmd(&request);
	തിരികെ;
}
