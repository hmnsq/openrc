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

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "rc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "_usage.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "version.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "helpers.h"


ਖਾਲੀ ਸੈੱਟ_quiet_options(ਖਾਲੀ)
{
	ਸਥਿਰ ਪੂਰਨ qcount = 0;

	qcount ++;
	ਚੋਣ (qcount) {
	ਹਾਲਤ 1:
		setenv ("EINFO_QUIET", "YES", 1);
		ਰੋਕੋ;
	ਹਾਲਤ 2:
		setenv ("EERROR_QUIET", "YES", 1);
		ਰੋਕੋ;
	}
}

RC_NORETURN ਖਾਲੀ show_ਸੰਸਕਰਣ(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *systype = ਨਿਰਾ;

	ਲਿਖੋ("%s (OpenRC", applet);
	ਜੇ ((systype = rc_sys()))
		ਲਿਖੋ(" [%s]", systype);
	ਲਿਖੋ(") %s", ਸੰਸਕਰਣ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ BRANDING
	ਲਿਖੋ(" (%s)", BRANDING);
#ਜੇ_ਅੰਤ
	ਲਿਖੋ("\n");
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
}

RC_NORETURN ਖਾਲੀ usage(ਪੂਰਨ exit_status)
{
	ਅਚਲ ਅੱਖਰ * ਅਚਲ has_arg[] = { "", "<arg>", "[arg]" };
	ਪੂਰਨ i;
	ਪੂਰਨ len;
	ਅੱਖਰ *lo;
	ਅੱਖਰ *p;
	ਅੱਖਰ *token;
	ਅੱਖਰ val[4] = "-?,";

	ਜੇ (usagestring)
		ਲਿਖੋ("%s", usagestring);
	ਨਹੀਂਤਾਂ
		ਲਿਖੋ("Usage: %s [options] ", applet);

	ਜੇ (extraopts)
		ਲਿਖੋ("%s", extraopts);

	ਲਿਖੋ("\n\nOptions: [ %s ]\n", getoptstring);
	ਲਈ (i = 0; longopts[i].name; ++i) {
		val[1] = longopts[i].val;
		len = ਲਿਖੋ("  %3s --%s %s", isprint(longopts[i].val) ? val : "",
		    longopts[i].name, has_arg[longopts[i].has_arg]);

		lo = p = xstrdup(longopts_help[i]);
		ਜਦੋਂ ((token = strsep(&p, "\n"))) {
			len = 36 - len;
			ਜੇ (len > 0)
				ਲਿਖੋ("%*s", len, "");
			puts(token);
			len = 0;
		}
		ਮੁਕਤ(lo);
	}
	ਬਾਹਰ_ਨਿਕਲੋ(exit_status);
}
