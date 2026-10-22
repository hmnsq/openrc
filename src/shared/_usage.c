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


ಶೂನ್ಯ ಗಣ_quiet_options(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ qcount = 0;

	qcount ++;
	ಆಯ್ಕೆ (qcount) {
	ಸ್ಥಿತಿ 1:
		setenv ("EINFO_QUIET", "YES", 1);
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 2:
		setenv ("EERROR_QUIET", "YES", 1);
		ನಿಲ್ಲಿಸು;
	}
}

RC_NORETURN ಶೂನ್ಯ show_ಆವೃತ್ತಿ(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *systype = ಖಾಲಿ;

	ಬರೆ("%s (OpenRC", applet);
	ಆದರೆ ((systype = rc_sys()))
		ಬರೆ(" [%s]", systype);
	ಬರೆ(") %s", ಆವೃತ್ತಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ BRANDING
	ಬರೆ(" (%s)", BRANDING);
#ಆದರೆ_ಅಂತ್ಯ
	ಬರೆ("\n");
	ನಿರ್ಗಮಿಸು(EXIT_SUCCESS);
}

RC_NORETURN ಶೂನ್ಯ usage(ಪೂರ್ಣಾಂಕ exit_status)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ has_arg[] = { "", "<arg>", "[arg]" };
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ len;
	ಅಕ್ಷರ *lo;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *token;
	ಅಕ್ಷರ val[4] = "-?,";

	ಆದರೆ (usagestring)
		ಬರೆ("%s", usagestring);
	ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ("Usage: %s [options] ", applet);

	ಆದರೆ (extraopts)
		ಬರೆ("%s", extraopts);

	ಬರೆ("\n\nOptions: [ %s ]\n", getoptstring);
	ಗಾಗಿ (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = ಬರೆ("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		ಆಗ ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			ಆದರೆ (len > 0)
				ಬರೆ("%*s", len, "");
			puts(token);
			len = 0;
		}
		ಬಿಡುಗಡೆ(lo);
	}
	ನಿರ್ಗಮಿಸು(exit_status);
}
