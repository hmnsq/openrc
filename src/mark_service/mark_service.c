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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "einfo.h"
#अन्तर्भाव "rc.h"
#अन्तर्भाव "misc.h"
#अन्तर्भाव "helpers.h"

ध्रुव वर्ण *applet = रिक्त;

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण **argv)
{
	bool ok = false;
	वर्ण *svcname = getenv("RC_SVCNAME");
	वर्ण *service = रिक्त;
	वर्ण *openrc_pid;
	/* char *mtime; */
	pid_t pid;
	RC_SERVICE bit;
	/* size_t l; */

	applet = basename_c(argv[0]);
	यदि (argc > 1)
		service = argv[1];
	अन्यथा
		service = svcname;

	यदि (service == रिक्त || *service == '\0')
		eerrorx("%s: no service specified", applet);

	यदि (!strncmp(applet, "mark_", 5) &&
	    (bit = lookup_service_state(applet + 5)))
		ok = rc_service_mark(service, bit);
	अन्यथा
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
	यदि (ok && svcname && strcmp(svcname, service) == 0) {
		openrc_pid = getenv("RC_OPENRC_PID");
		यदि (openrc_pid && sscanf(openrc_pid, "%d", &pid) == 1)
			यदि (kill(pid, SIGUSR1) != 0)
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

	प्रत्यागम ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
