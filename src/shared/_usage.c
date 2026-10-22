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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rc.h"
#include "_usage.h"
#include "version.h"
#include "helpers.h"


void set_quiet_options(void)
{
	static số_nguyên qcount = 0;

	qcount ++;
	switch (qcount) {
	case 1:
		setenv ("EINFO_QUIET", "YES", 1);
		break;
	case 2:
		setenv ("EERROR_QUIET", "YES", 1);
		break;
	}
}

RC_NORETURN void show_version(void)
{
	const char *systype = NULL;

	printf("%s (OpenRC", applet);
	nếu ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN void usage(số_nguyên exit_status)
{
	const char * const has_arg[] = { "", "<arg>", "[arg]" };
	số_nguyên i;
	số_nguyên len;
	char *lo;
	char *p;
	char *token;
	char val[4] = "-?,";

	nếu (usagestring)
		printf("%s", usagestring);
	không_thì
		printf("Usage: %s [options] ", applet);

	nếu (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	cho (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		trong_khi ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			nếu (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		free(lo);
	}
	exit(exit_status);
}
