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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "einfo.h"
#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "misc.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *applet = ಖಾಲಿ;

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	bool ok = false;
	ಅಕ್ಷರ *svcname = getenv("RC_SVCNAME");
	ಅಕ್ಷರ *service = ಖಾಲಿ;
	ಅಕ್ಷರ *openrc_pid;
	/* char *mtime; */
	pid_t pid;
	RC_SERVICE ಬಿಟ್;
	/* size_t l; */

	applet = basename_c(argv[0]);
	ಆದರೆ (argc > 1)
		service = argv[1];
	ಇಲ್ಲದಿದ್ದರೆ
		service = svcname;

	ಆದರೆ (service == ಖಾಲಿ || *service == '\0')
		eerrorx("%s: no service specified", applet);

	ಆದರೆ (!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(applet, "mark_", 5) &&
	    (ಬಿಟ್ = lookup_service_state(applet + 5)))
		ok = rc_service_mark(service, ಬಿಟ್);
	ಇಲ್ಲದಿದ್ದರೆ
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
	ಆದರೆ (ok && svcname && strcmp(svcname, service) == 0) {
		openrc_pid = getenv("RC_OPENRC_PID");
		ಆದರೆ (openrc_pid && ಅಕ್ಷರಸರಣಿ_ಓದು(openrc_pid, "%d", &pid) == 1)
			ಆದರೆ (kill(pid, SIGUSR1) != 0)
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

	ಹಿಂತಿರುಗಿ ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
