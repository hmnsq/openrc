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

#ଅନ୍ତର୍ଭାଵ <ctype.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "rc.h"
#ଅନ୍ତର୍ଭାଵ "_usage.h"
#ଅନ୍ତର୍ଭାଵ "version.h"
#ଅନ୍ତର୍ଭାଵ "helpers.h"


ଶୂନ୍ୟ set_quiet_options(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା qcount = 0;

	qcount ++;
	ବାଛ (qcount) {
	ସ୍ଥିତି 1:
		setenv ("EINFO_QUIET", "YES", 1);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 2:
		setenv ("EERROR_QUIET", "YES", 1);
		ଭାଙ୍ଗ;
	}
}

RC_NORETURN ଶୂନ୍ୟ show_version(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *systype = ଖାଲି;

	printf("%s (OpenRC", applet);
	ଯଦି ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN ଶୂନ୍ୟ usage(ପୂର୍ଣ୍ଣସଂଖ୍ୟା exit_status)
{
	ଅଚଳ ଅକ୍ଷର * ଅଚଳ has_arg[] = { "", "<arg>", "[arg]" };
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ଅକ୍ଷର *lo;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *token;
	ଅକ୍ଷର val[4] = "-?,";

	ଯଦି (usagestring)
		printf("%s", usagestring);
	ଅନ୍ୟଥା
		printf("Usage: %s [options] ", applet);

	ଯଦି (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	ପାଇଁ (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		ଯେତେବେଳେ ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			ଯଦି (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		ମୁକ୍ତ(lo);
	}
	exit(exit_status);
}
