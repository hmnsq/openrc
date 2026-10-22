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

#అన్తర్భావ <ctype.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "rc.h"
#అన్తర్భావ "_usage.h"
#అన్తర్భావ "version.h"
#అన్తర్భావ "helpers.h"


శూన్య సమితి_quiet_options(శూన్య)
{
	స్థిర పూర్ణాంకం qcount = 0;

	qcount ++;
	ఎంపిక (qcount) {
	స్థితి 1:
		setenv ("EINFO_QUIET", "YES", 1);
		ఆపు;
	స్థితి 2:
		setenv ("EERROR_QUIET", "YES", 1);
		ఆపు;
	}
}

RC_NORETURN శూన్య show_సంస్కరణ(శూన్య)
{
	స్థిరరాశి అక్షరం *systype = ఖాళీ;

	రాయి("%s (OpenRC", applet);
	ఒకవేళ ((systype = rc_sys()))
		రాయి(" [%s]", systype);
	రాయి(") %s", సంస్కరణ);
#నిర్వచితమైతే BRANDING
	రాయి(" (%s)", BRANDING);
#అయితే_ముగింపు
	రాయి("\n");
	నిష్క్రమించు(EXIT_SUCCESS);
}

RC_NORETURN శూన్య usage(పూర్ణాంకం exit_status)
{
	స్థిరరాశి అక్షరం * స్థిరరాశి has_arg[] = { "", "<arg>", "[arg]" };
	పూర్ణాంకం i;
	పూర్ణాంకం len;
	అక్షరం *lo;
	అక్షరం *p;
	అక్షరం *token;
	అక్షరం val[4] = "-?,";

	ఒకవేళ (usagestring)
		రాయి("%s", usagestring);
	లేకపోతే
		రాయి("Usage: %s [options] ", applet);

	ఒకవేళ (extraopts)
		రాయి("%s", extraopts);

	రాయి("\n\nOptions: [ %s ]\n", getoptstring);
	కొరకు (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = రాయి("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		అయితే ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			ఒకవేళ (len > 0)
				రాయి("%*s", len, "");
			puts(token);
			len = 0;
		}
		విడుదల(lo);
	}
	నిష్క్రమించు(exit_status);
}
