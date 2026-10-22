/*
 * Copyright (c) 2007-2015 The OpenRC Authors.
 * See the Authors file at the top-level directory of this distribution and
 * https://github.com/OpenRC/openrc/blob/HEAD/AUTHORS
 *
 * This file is part of OpenRC. It is subject to the license terms in
 * the LICENSE file found in the top-level directory of this
 * distribution and at https://github.com/OpenRC/openrc/blob/HEAD/LICENSE
 * This file may not be copied, modified, propagated, or distributed
 *    except according to the terms contained in the LICENSE file.
 */

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ "einfo.h"
#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "misc.h"
#அந்தர்பாவ "helpers.h"

மாறிலி எழுத்து *applet = வெற்று;

முழு முதன்மை(முழு argc, எழுத்து **argv)
{
	bool ok = false;
	எழுத்து *svcname = getenv("RC_SVCNAME");
	எழுத்து *service = வெற்று;
	எழுத்து *openrc_pid;
	/* char *mtime; */
	pid_t pid;
	RC_SERVICE bit;
	/* size_t l; */

	applet = basename_c(argv[0]);
	எனில் (argc > 1)
		service = argv[1];
	இல்லையெனில்
		service = svcname;

	எனில் (service == வெற்று || *service == '\0')
		eerrorx("%s: no service specified", applet);

	எனில் (!strncmp(applet, "mark_", 5) &&
	    (bit = lookup_service_state(applet + 5)))
		ok = rc_service_mark(service, bit);
	இல்லையெனில்
		eerrorx("%s: unknown applet", applet);

	/* If we're marking ourselves then we need to inform our parent
	   openrc-run process so they do not mark us based on our exit code */
	/*
	 * FIXME: svcname and service are almost always equal except called from a
	 * shell with just argv[1] - So that doesn't seem to do what Roy initially
	 * expected.
	 * See 20120424041423.GA23657@odin.qasl.de (Tue, 24 Apr 2012 06:14:23 +0200,
	 * openrc@gentoo.org).
	 */
	எனில் (ok && svcname && strcmp(svcname, service) == 0) {
		openrc_pid = getenv("RC_OPENRC_PID");
		எனில் (openrc_pid && sscanf(openrc_pid, "%d", &pid) == 1)
			எனில் (kill(pid, SIGUSR1) != 0)
				eerror("%s: failed to signal parent %d: %s",
				    applet, pid, strerror(errno));

		/* Remove the exclusive time test. This ensures that it's not
		   in control as well */
		/*
		l = strlen(RC_SVCDIR "/exclusive") + strlen(svcname) +
		    strlen(openrc_pid) + 4;
		mtime = xmalloc(l);
		snprintf(mtime, l, RC_SVCDIR "/exclusive/%s.%s",
		    svcname, openrc_pid);
		if (exists(mtime) && unlink(mtime) != 0)
			eerror("%s: unlink: %s", applet, strerror(errno));
		free(mtime);
		*/
	}

	திரும்பு ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
