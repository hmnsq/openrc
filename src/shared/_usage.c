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


فراغ set_quiet_options(فراغ)
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

RC_NORETURN فراغ show_version(فراغ)
{
	ثابت حرف *systype = عدم;

	printf("%s (OpenRC", applet);
	אם ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
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

	אם (usagestring)
		printf("%s", usagestring);
	אחרת
		printf("Usage: %s [options] ", applet);

	אם (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	עבור (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		כלעוד ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			אם (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		حرر(lo);
	}
	exit(exit_status);
}
