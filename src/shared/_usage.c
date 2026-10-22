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

#समावेश <ctype.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "rc.h"
#समावेश "_usage.h"
#समावेश "version.h"
#समावेश "helpers.h"


व्योम set_quiet_options(व्योम)
{
	अटल पूर्णांक qcount = 0;

	qcount ++;
	चयन (qcount) {
	हाल 1:
		setenv ("EINFO_QUIET", "YES", 1);
		अवरोध;
	हाल 2:
		setenv ("EERROR_QUIET", "YES", 1);
		अवरोध;
	}
}

RC_NORETURN व्योम show_version(व्योम)
{
	स्थिर अक्षर *systype = शून्य;

	म_लिखो("%s (OpenRC", applet);
	अगर ((systype = rc_sys()))
		म_लिखो(" [%s]", systype);
	म_लिखो(") %s", VERSION);
#अगर_घोषित BRANDING
	म_लिखो(" (%s)", BRANDING);
#पूर्ण_अगर
	म_लिखो("\n");
	निकास(निकास_सफल);
}

RC_NORETURN व्योम usage(पूर्णांक exit_status)
{
	स्थिर अक्षर * स्थिर has_arg[] = { "", "<arg>", "[arg]" };
	पूर्णांक i;
	पूर्णांक len;
	अक्षर *lo;
	अक्षर *p;
	अक्षर *token;
	अक्षर val[4] = "-?,";

	अगर (usagestring)
		म_लिखो("%s", usagestring);
	अन्यथा
		म_लिखो("Usage: %s [options] ", applet);

	अगर (extraopts)
		म_लिखो("%s", extraopts);

	म_लिखो("\n\nOptions: [ %s ]\n", getoptstring);
	क्रम (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = म_लिखो("  %3s --%s %s", है_छाप(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		जबतक ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			अगर (len > 0)
				म_लिखो("%*s", len, "");
			माला_दो(token);
			len = 0;
		}
		मुक्त(lo);
	}
	निकास(exit_status);
}
