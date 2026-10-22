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

#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "rc.h"
#ಅನ್ತರ್ಭಾವ "_usage.h"
#ಅನ್ತರ್ಭಾವ "version.h"
#ಅನ್ತರ್ಭಾವ "helpers.h"


ಶೂನ್ಯ set_quiet_options(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ qcount = 0;

	qcount ++;
	ವಿಕಲ್ಪ (qcount) {
	ಪಕ್ಷ 1:
		setenv ("EINFO_QUIET", "YES", 1);
		ಭಙ್ಗ;
	ಪಕ್ಷ 2:
		setenv ("EERROR_QUIET", "YES", 1);
		ಭಙ್ಗ;
	}
}

RC_NORETURN ಶೂನ್ಯ show_version(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *systype = ರಿಕ್ತ;

	printf("%s (OpenRC", applet);
	ಯದಿ ((systype = rc_sys()))
		printf(" [%s]", systype);
	printf(") %s", VERSION);
#ifdef BRANDING
	printf(" (%s)", BRANDING);
#endif
	printf("\n");
	exit(EXIT_SUCCESS);
}

RC_NORETURN ಶೂನ್ಯ usage(ಪೂರ್ಣಾಙ್ಕ exit_status)
{
	ಧ್ರುವ ವರ್ಣ * ಧ್ರುವ has_arg[] = { "", "<arg>", "[arg]" };
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ len;
	ವರ್ಣ *lo;
	ವರ್ಣ *p;
	ವರ್ಣ *token;
	ವರ್ಣ val[4] = "-?,";

	ಯದಿ (usagestring)
		printf("%s", usagestring);
	ಅನ್ಯಥಾ
		printf("Usage: %s [options] ", applet);

	ಯದಿ (extraopts)
		printf("%s", extraopts);

	printf("\n\nOptions: [ %s ]\n", getoptstring);
	ಪ್ರತಿ (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = printf("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		ಯಾವತ್ ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			ಯದಿ (len > 0)
				printf("%*s", len, "");
			puts(token);
			len = 0;
		}
		ಮುಕ್ತ(lo);
	}
	exit(exit_status);
}
