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

#अन्तर्भाव <ctype.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "rc.h"
#अन्तर्भाव "_usage.h"
#अन्तर्भाव "version.h"
#अन्तर्भाव "helpers.h"


शून्य set_quiet_options(शून्य)
{
	स्थिर पूर्णाङ्क qcount = 0;

	qcount ++;
	विकल्प (qcount) {
	पक्ष 1:
		setenv ("EINFO_QUIET", "YES", 1);
		भङ्ग;
	पक्ष 2:
		setenv ("EERROR_QUIET", "YES", 1);
		भङ्ग;
	}
}

RC_NORETURN शून्य show_version(शून्य)
{
	ध्रुव वर्ण *systype = रिक्त;

	printf("%s (OpenRC", applet);
	यदि ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN शून्य usage(पूर्णाङ्क exit_status)
{
	ध्रुव वर्ण * ध्रुव has_arg[] = { "", "<arg>", "[arg]" };
	पूर्णाङ्क i;
	पूर्णाङ्क len;
	वर्ण *lo;
	वर्ण *p;
	वर्ण *token;
	वर्ण val[4] = "-?,";

	यदि (usagestring)
		printf("%s", usagestring);
	अन्यथा
		printf("Usage: %s [options] ", applet);

	यदि (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	प्रति (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		यावत् ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			यदि (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		मुक्त(lo);
	}
	exit(exit_status);
}
