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


خالی سیٹ_quiet_options(خالی)
{
	ساکن صحیح qcount = 0;

	qcount ++;
	انتخاب (qcount) {
	حالت 1:
		setenv ("EINFO_QUIET", "YES", 1);
		توڑو;
	حالت 2:
		setenv ("EERROR_QUIET", "YES", 1);
		توڑو;
	}
}

RC_NORETURN خالی show_نسخہ(خالی)
{
	ثابت حرف *systype = تہی;

	لکھو("%s (OpenRC", applet);
	اگر ((systype = rc_sys()))
		لکھو(" [%s]", systype);
	لکھو(") %s", نسخہ);
#اگر_تعریف_شدہ BRANDING
	لکھو(" (%s)", BRANDING);
#اگر_ختم
	لکھو("\n");
	باہر_نکلو(EXIT_SUCCESS);
}

RC_NORETURN خالی usage(صحیح exit_status)
{
	ثابت حرف * ثابت has_arg[] = { "", "<arg>", "[arg]" };
	صحیح i;
	صحیح len;
	حرف *lo;
	حرف *p;
	حرف *token;
	حرف val[4] = "-?,";

	اگر (usagestring)
		لکھو("%s", usagestring);
	ورنہ
		لکھو("Usage: %s [options] ", applet);

	اگر (extraopts)
		لکھو("%s", extraopts);

	لکھو("\n\nOptions: [ %s ]\n", getoptstring);
	برائے (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = لکھو("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		جب ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			اگر (len > 0)
				لکھو("%*s", len, "");
			puts(token);
			len = 0;
		}
		آزاد(lo);
	}
	باہر_نکلو(exit_status);
}
