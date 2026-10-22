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

#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <signal.h>
#समावेश <string.h>
#समावेश <sys/types.h>

#समावेश "einfo.h"
#समावेश "rc.h"
#समावेश "misc.h"
#समावेश "helpers.h"

स्थिर अक्षर *applet = शून्य;

पूर्णांक मुख्य(पूर्णांक argc, अक्षर **argv)
{
	bool ok = false;
	अक्षर *svcname = दो_पर्या("RC_SVCNAME");
	अक्षर *service = शून्य;
	अक्षर *openrc_pid;
	/* char *mtime; */
	pid_t pid;
	RC_SERVICE bit;
	/* size_t l; */

	applet = basename_c(argv[0]);
	अगर (argc > 1)
		service = argv[1];
	अन्यथा
		service = svcname;

	अगर (service == शून्य || *service == '\0')
		eerrorx("%s: no service specified", applet);

	अगर (!म_भेदन(applet, "mark_", 5) &&
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
	अगर (ok && svcname && म_भेद(svcname, service) == 0) {
		openrc_pid = दो_पर्या("RC_OPENRC_PID");
		अगर (openrc_pid && माला_पूछो(openrc_pid, "%d", &pid) == 1)
			अगर (समाप्त(pid, SIGUSR1) != 0)
				eerror("%s: failed to signal parent %d: %s",
				    applet, pid, म_त्रुटि(त्रुटि_सं));

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

	वापस ok ? निकास_सफल : निकास_त्रुटि;
}
