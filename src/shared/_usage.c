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

#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "rc.h"
#অন্তর্ভাৱ "_usage.h"
#অন্তর্ভাৱ "version.h"
#অন্তর্ভাৱ "helpers.h"


শূন্য set_quiet_options(শূন্য)
{
	স্থির পূর্ণাঙ্ক qcount = 0;

	qcount ++;
	ৱিকল্প (qcount) {
	পক্ষ 1:
		setenv ("EINFO_QUIET", "YES", 1);
		ভঙ্গ;
	পক্ষ 2:
		setenv ("EERROR_QUIET", "YES", 1);
		ভঙ্গ;
	}
}

RC_NORETURN শূন্য show_version(শূন্য)
{
	ধ্রুৱ ৱর্ণ *systype = রিক্ত;

	printf("%s (OpenRC", applet);
	যদি ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN শূন্য usage(পূর্ণাঙ্ক exit_status)
{
	ধ্রুৱ ৱর্ণ * ধ্রুৱ has_arg[] = { "", "<arg>", "[arg]" };
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক len;
	ৱর্ণ *lo;
	ৱর্ণ *p;
	ৱর্ণ *token;
	ৱর্ণ val[4] = "-?,";

	যদি (usagestring)
		printf("%s", usagestring);
	অন্যথা
		printf("Usage: %s [options] ", applet);

	যদি (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	বাবে (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		যেতিয়ালৈ ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			যদি (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		মুক্ত(lo);
	}
	exit(exit_status);
}
