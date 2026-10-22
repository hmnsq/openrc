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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "initreq.h"
#ಅನ್ತರ್ಭಾವ "sysvinit.h"

ಸ್ಥಿರ ಶೂನ್ಯ sysvinit_send_cmd(ರಚನೆ init_request *request)
{
	ಪೂರ್ಣಾಂಕ fd;
	ಅಕ್ಷರ *p;
	size_t ಬೈಟುಗಳು;
	ssize_t r;

	fd = open("/run/initctl", O_WRONLY|O_NONBLOCK|O_CLOEXEC|O_NOCTTY);
	ಆದರೆ (fd < 0) {
		ಆದರೆ (errno != ENOENT)
			eerror("Failed to open initctl fifo: %s", strerror(errno));
		ಹಿಂತಿರುಗಿ;
	}
	p = (ಅಕ್ಷರ *) request;
	ಬೈಟುಗಳು = ಗಾತ್ರ(*request);
	ಮಾಡು {
		r = ಲಿಖ(fd, p, ಬೈಟುಗಳು);
		ಆದರೆ (r < 0) {
			ಆದರೆ ((errno == EAGAIN) || (errno == EINTR))
				ಮುಂದುವರಿ;
			eerror("Failed to write to /run/initctl: %s", strerror(errno));
			ಹಿಂತಿರುಗಿ;
		}
		p += r;
		ಬೈಟುಗಳು -= r;
	} ಆಗ (ಬೈಟುಗಳು > 0);
}

ಶೂನ್ಯ sysvinit_runlevel(ಅಕ್ಷರ rl)
{
	ರಚನೆ init_request request;

	ಆದರೆ (!rl)
		ಹಿಂತಿರುಗಿ;

	request = (ರಚನೆ init_request) {
		.magic = INIT_MAGIC,
		.sleeptime = 0,
		.cmd = INIT_CMD_RUNLVL,
		.runlevel = rl,
	};
	sysvinit_send_cmd(&request);
		ಹಿಂತಿರುಗಿ;
}

/*
 *	Set environment variables in the init process.
 */
ಶೂನ್ಯ sysvinit_setenv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
	ರಚನೆ init_request	request;
	size_t nl;
	size_t vl;

	memset(&request, 0, ಗಾತ್ರ(request));
	request.magic = INIT_MAGIC;
	request.cmd = INIT_CMD_SETENV;
	nl = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
	ಆದರೆ (ಮೌಲ್ಯ)
		vl = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ);
ಇಲ್ಲದಿದ್ದರೆ
		vl = 0;

	ಆದರೆ (nl + vl + 3 >= (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(request.i.ದತ್ತಾಂಶ))
		ಹಿಂತಿರುಗಿ;

	ಸ್ಮರಣೆ_ನಕಲು(request.i.ದತ್ತಾಂಶ, name, nl);
	ಆದರೆ (ಮೌಲ್ಯ) {
		request.i.ದತ್ತಾಂಶ[nl] = '=';
		ಸ್ಮರಣೆ_ನಕಲು(request.i.ದತ್ತಾಂಶ + nl + 1, ಮೌಲ್ಯ, vl);
	}
	sysvinit_send_cmd(&request);
	ಹಿಂತಿರುಗಿ;
}
