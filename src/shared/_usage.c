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

#شامل <ctype.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "rc.h"
#شامل "_usage.h"
#شامل "version.h"
#شامل "helpers.h"


تهی set_quiet_options(تهی)
{
	ایستا صحیح qcount = 0;

	qcount ++;
	انتخاب (qcount) {
	حالت 1:
		setenv ("EINFO_QUIET", "YES", 1);
		بشکن;
	حالت 2:
		setenv ("EERROR_QUIET", "YES", 1);
		بشکن;
	}
}

RC_NORETURN تهی show_version(تهی)
{
	ثابت نویسه *systype = پوچ;

	printf("%s (OpenRC", applet);
	اگر ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN تهی usage(صحیح exit_status)
{
	ثابت نویسه * ثابت has_arg[] = { "", "<arg>", "[arg]" };
	صحیح i;
	صحیح len;
	نویسه *lo;
	نویسه *p;
	نویسه *token;
	نویسه val[4] = "-?,";

	اگر (usagestring)
		printf("%s", usagestring);
	وگرنه
		printf("Usage: %s [options] ", applet);

	اگر (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	برای (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		وقتی ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			اگر (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		آزاد(lo);
	}
	exit(exit_status);
}
