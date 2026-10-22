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

#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "rc.h"
#অন্তর্ভাব "_usage.h"
#অন্তর্ভাব "version.h"
#অন্তর্ভাব "helpers.h"


শূন্য set_quiet_options(শূন্য)
{
	স্থির পূর্ণসংখ্যা qcount = 0;

	qcount ++;
	নির্বাচন (qcount) {
	ক্ষেত্র 1:
		setenv ("EINFO_QUIET", "YES", 1);
		বিরতি;
	ক্ষেত্র 2:
		setenv ("EERROR_QUIET", "YES", 1);
		বিরতি;
	}
}

RC_NORETURN শূন্য show_version(শূন্য)
{
	ধ্রুবক অক্ষর *systype = ফাঁকা;

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

RC_NORETURN শূন্য usage(পূর্ণসংখ্যা exit_status)
{
	ধ্রুবক অক্ষর * ধ্রুবক has_arg[] = { "", "<arg>", "[arg]" };
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা len;
	অক্ষর *lo;
	অক্ষর *p;
	অক্ষর *token;
	অক্ষর val[4] = "-?,";

	যদি (usagestring)
		printf("%s", usagestring);
	অন্যথা
		printf("Usage: %s [options] ", applet);

	যদি (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	জন্য (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		যখন ((token = strsep(&p, "\n"))) {
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
