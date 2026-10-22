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
	स्थिर पूर्णांक qcount = 0;

	qcount ++;
	निवड (qcount) {
	स्थिती 1:
		setenv ("EINFO_QUIET", "YES", 1);
		थांबा;
	स्थिती 2:
		setenv ("EERROR_QUIET", "YES", 1);
		थांबा;
	}
}

RC_NORETURN शून्य show_version(शून्य)
{
	स्थिरांक अक्षर *systype = रिक्त;

	म_लिखो("%s (OpenRC", applet);
	जर ((systype = rc_sys()))
		म_लिखो(" [%s]", systype);
	म_लिखो(") %s", VERSION);
#ifdef BRANDING
	म_लिखो(" (%s)", BRANDING);
#endif
	म_लिखो("\n");
	निकास(निकास_सफल);
}

RC_NORETURN शून्य usage(पूर्णांक exit_status)
{
	स्थिरांक अक्षर * स्थिरांक has_arg[] = { "", "<arg>", "[arg]" };
	पूर्णांक i;
	पूर्णांक len;
	अक्षर *lo;
	अक्षर *p;
	अक्षर *token;
	अक्षर val[4] = "-?,";

	जर (usagestring)
		म_लिखो("%s", usagestring);
	अन्यथा
		म_लिखो("Usage: %s [options] ", applet);

	जर (extraopts)
		म_लिखो("%s", extraopts);

	म_लिखो("\n\nOptions: [ %s ]\n", getoptstring);
	साठी (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = म_लिखो("  %3s --%s %s", है_छाप(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		पर्यंत ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			जर (len > 0)
				म_लिखो("%*s", len, "");
			माला_दो(token);
			len = 0;
		}
		मुक्त(lo);
	}
	निकास(exit_status);
}
