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

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "einfo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "initreq.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sysvinit.h"

ਸਥਿਰ ਖਾਲੀ sysvinit_send_cmd(ਬਣਤਰ init_request *request)
{
	ਪੂਰਨ fd;
	ਅੱਖਰ *p;
	size_t ਬਾਈਟਾਂ;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	ਜੇ (fd < 0) {
		ਜੇ (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ਵਾਪਸ;
	}
	p = (ਅੱਖਰ *) request;
	ਬਾਈਟਾਂ = ਆਕਾਰ(*request);
	ਕਰੋ {
		r = ਲਿਖ(fd, p, ਬਾਈਟਾਂ);
		ਜੇ (r < 0) {
			ਜੇ ((errno == EAGAIN) || (errno == EINTR))
				ਜਾਰੀ;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ਵਾਪਸ;
		}
		p += r;
		ਬਾਈਟਾਂ -= r;
	} ਜਦੋਂ (ਬਾਈਟਾਂ > 0);
}

ਖਾਲੀ sysvinit_runlevel(ਅੱਖਰ rl)
{
	ਬਣਤਰ init_request request;

	ਜੇ (!rl)
		ਵਾਪਸ;

	request = (ਬਣਤਰ init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ਵਾਪਸ;
}

/*
 *	Set environment variables in the init process.
 */
ਖਾਲੀ sysvinit_setenv(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *ਮੁੱਲ)
{
	ਬਣਤਰ init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, ਆਕਾਰ(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = ਲੜੀ_ਲੰਬਾਈ(name);
	ਜੇ (ਮੁੱਲ)
		vl = ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ);
ਨਹੀਂਤਾਂ
		vl = 0;

	ਜੇ (nl + vl + 3 >= (ਪੂਰਨ)ਆਕਾਰ(request.i.ਅੰਕੜੇ))
		ਵਾਪਸ;

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(request.i.ਅੰਕੜੇ, name, nl);
	ਜੇ (ਮੁੱਲ) {
		request.i.ਅੰਕੜੇ[nl] = '=';
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(request.i.ਅੰਕੜੇ + nl + 1, ਮੁੱਲ, vl);
	}
	sysvinit_send_cmd(&request);
	ਵਾਪਸ;
}
