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

#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "rc.h"
#அந்தர்பாவ "_usage.h"
#அந்தர்பாவ "version.h"
#அந்தர்பாவ "helpers.h"


வெறுமை set_quiet_options(வெறுமை)
{
	நிலையான முழு qcount = 0;

	qcount ++;
	தேர்வு (qcount) {
	நிலை 1:
		setenv ("EINFO_QUIET", "YES", 1);
		நிறுத்து;
	நிலை 2:
		setenv ("EERROR_QUIET", "YES", 1);
		நிறுத்து;
	}
}

RC_NORETURN வெறுமை show_version(வெறுமை)
{
	மாறிலி எழுத்து *systype = வெற்று;

	printf("%s (OpenRC", applet);
	எனில் ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN வெறுமை usage(முழு exit_status)
{
	மாறிலி எழுத்து * மாறிலி has_arg[] = { "", "<arg>", "[arg]" };
	முழு i;
	முழு len;
	எழுத்து *lo;
	எழுத்து *p;
	எழுத்து *token;
	எழுத்து val[4] = "-?,";

	எனில் (usagestring)
		printf("%s", usagestring);
	இல்லையெனில்
		printf("Usage: %s [options] ", applet);

	எனில் (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	க்கு (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		போது ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			எனில் (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		விடுவி(lo);
	}
	exit(exit_status);
}
