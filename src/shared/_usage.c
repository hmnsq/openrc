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

#ضمن <ctype.h>
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>

#ضمن "rc.h"
#ضمن "_usage.h"
#ضمن "version.h"
#ضمن "helpers.h"


فراغ سېټ_quiet_options(فراغ)
{
	ساكن صحيح qcount = 0;

	qcount ++;
	بدل (qcount) {
	حالة 1:
		setenv ("EINFO_QUIET", "YES", 1);
		اكسر;
	حالة 2:
		setenv ("EERROR_QUIET", "YES", 1);
		اكسر;
	}
}

RC_NORETURN فراغ show_نسخه(فراغ)
{
	ثابت حرف *systype = عدم;

	چاپ_کړه("%s (OpenRC", applet);
	إذا ((systype = rc_sys()))
		چاپ_کړه(" [%s]", systype);
	چاپ_کړه(") %s", نسخه);
#که_تعریف_شوی BRANDING
	چاپ_کړه(" (%s)", BRANDING);
#د_که_پای
	چاپ_کړه("\n");
	وتل(EXIT_SUCCESS);
}

RC_NORETURN فراغ usage(صحيح exit_status)
{
	ثابت حرف * ثابت has_arg[] = { "", "<arg>", "[arg]" };
	صحيح i;
	صحيح len;
	حرف *lo;
	حرف *p;
	حرف *token;
	حرف val[4] = "-?,";

	إذا (usagestring)
		چاپ_کړه("%s", usagestring);
	وإلا
		چاپ_کړه("Usage: %s [options] ", applet);

	إذا (extraopts)
		چاپ_کړه("%s", extraopts);

	چاپ_کړه("\n\nOptions: [ %s ]\n", getoptstring);
	لكل (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = چاپ_کړه("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		طالما ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			إذا (len > 0)
				چاپ_کړه("%*s", len, "");
			puts(token);
			len = 0;
		}
		حرر(lo);
	}
	وتل(exit_status);
}
